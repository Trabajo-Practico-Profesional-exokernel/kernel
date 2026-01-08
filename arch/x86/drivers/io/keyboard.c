#include "inc/types.h"
#include "../../idt.h"
#include "vnode.h"
#include "inc/common.h"
#include "../../interrupt.h"

/*
Hardware facts (x86 PS/2)
    - Data port: 0x60
    - Status port: 0x64
    - IRQ: 1
    - Interrupt fires when a scancode is available
*/

#define KBD_DATA_PORT   0x3F8
#define KBD_BUFFER_SIZE 512

/* Alphabet */
/* Standard Letters */
#define KBD_SC_A        0x1e
#define KBD_SC_B        0x30
#define KBD_SC_C        0x2e
#define KBD_SC_D        0x20
#define KBD_SC_E        0x12
#define KBD_SC_F        0x21
#define KBD_SC_G        0x22
#define KBD_SC_H        0x23
#define KBD_SC_I        0x17
#define KBD_SC_J        0x24
#define KBD_SC_K        0x25
#define KBD_SC_L        0x26
#define KBD_SC_M        0x32
#define KBD_SC_N        0x31
#define KBD_SC_O        0x18
#define KBD_SC_P        0x19
#define KBD_SC_Q        0x10
#define KBD_SC_R        0x13
#define KBD_SC_S        0x1f
#define KBD_SC_T        0x14
#define KBD_SC_U        0x16
#define KBD_SC_V        0x2f
#define KBD_SC_W        0x11
#define KBD_SC_X        0x2d
#define KBD_SC_Y        0x15
#define KBD_SC_Z        0x2c

/* Numeric keys */
#define KBD_SC_1        0x02
#define KBD_SC_2        0x03
#define KBD_SC_3        0x04
#define KBD_SC_4        0x05
#define KBD_SC_5        0x06
#define KBD_SC_6        0x07
#define KBD_SC_7        0x08
#define KBD_SC_8        0x09
#define KBD_SC_9        0x0a
#define KBD_SC_0        0x0b

/* Special keys - Physical Locations */
#define KBD_SC_ENTER    0x1c
#define KBD_SC_SPACE    0x39
#define KBD_SC_BS       0x0e
#define KBD_SC_LSHIFT   0x2a
#define KBD_SC_RSHIFT   0x36
#define KBD_SC_CAPSLOCK 0x3a
#define KBD_SC_TAB      0x0f

/* Layout Specific Mapping (LatAm uses these physical keys differently) */
#define KBD_SC_DASH     0x0c  // Fila Numérica, derecha del 0 (LatAm: ' ?)
#define KBD_SC_EQUALS   0x0d  // Fila Numérica, derecha del ' (LatAm: ¿ ¡)
#define KBD_SC_LBRACKET 0x1a  // Derecha de P (LatAm: ´ ¨)
#define KBD_SC_RBRACKET 0x1b  // Derecha de ´ (LatAm: + *)
#define KBD_SC_SCOLON   0x27  // Derecha de L (LatAm: Ñ)
#define KBD_SC_QUOTE    0x28  // Derecha de Ñ (LatAm: { [)
#define KBD_SC_BSLASH   0x2b  // Izquierda de Backspace o Enter (LatAm: } ])
#define KBD_SC_COMMA    0x33
#define KBD_SC_DOT      0x34
#define KBD_SC_FSLASH   0x35  // Derecha de punto (LatAm: - _)
#define KBD_SC_TILDE    0x29  // Izquierda de 1 (LatAm: | °)
#define KBD_SC_LESS     0x56  // Izquierda de Z (LatAm: < >)

static uint8_t is_lshift_down       = 0;
static uint8_t is_rshift_down       = 0;
static uint8_t is_caps_lock_pressed = 0;

uint8_t get_char_ready = 0;
int actual_char = -1;

struct kbd_buffer {
    uint8_t buffer[KBD_BUFFER_SIZE];
    uint8_t *head;
    uint8_t *tail;
    uint32_t count;
};
typedef struct kbd_buffer kbd_buffer_t;
static kbd_buffer_t kbd_buffer;

static vnode_t kbd_vnode;
static vnodeops_t kbd_vnodeops;

// TODO: refactor
struct idt_info {
	uint32_t idt_index;
	uint32_t error_code;
} __attribute__((packed));
typedef struct idt_info idt_info_t;

struct cpu_state {
	uint32_t edi;
	uint32_t esi;
	uint32_t ebp;
	uint32_t edx;
	uint32_t ecx;
	uint32_t ebx;
	uint32_t eax;
	uint32_t esp;
} __attribute__((packed));
typedef struct cpu_state cpu_state_t;

struct stack_state {
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t user_esp; /* not always safe to derefence! */
    uint32_t user_ss;  /* not always safe to derefence! */
} __attribute__((packed));
typedef struct stack_state stack_state_t;

/* function declarations */
static char kbd_scan_code_to_ascii(uint8_t sc);
static uint8_t kbd_read_scan_code(void);

#define SYSCALL_INT_IDX 0xAE

typedef void (*interrupt_handler_t)(cpu_state_t state,
                                    idt_info_t info,
                                    stack_state_t exec);

static interrupt_handler_t interrupt_handlers[IDT_NUM_ENTRIES];

uint32_t register_interrupt_handler(uint32_t interrupt,
                                    interrupt_handler_t handler)
{
    if (interrupt > 255) {
        return 1;
    }
    if (interrupt == SYSCALL_INT_IDX) {
        return 1;
    }
    if (interrupt_handlers[interrupt] != NULL) {
        return 1;
    }

    interrupt_handlers[interrupt] = handler;
    return 0;
}

void interrupt_handler(cpu_state_t state, idt_info_t info, stack_state_t exec)
{
    if (interrupt_handlers[info.idt_index] != NULL) {
        interrupt_handlers[info.idt_index](state, info, exec);
    }
}

//por el momento mantenemos el handle de la interrupcion 36 (entrada de teclado con QEMU -nographic)
//para QEMU sin el flag "-nographic" con una ventana aparte hay que volver a la implementacion anterior
//de este handleo, y llamar a esta funcion para la interrupcion 33 en lugar de la interrupcion 36
void keyboard_handle_interrupt(void)
{
    uint8_t scan_code = inb(KBD_DATA_PORT);
    //printf("scan_code hex: [%x] - scan_code [%c]", scan_code, scan_code); 
    if (get_char_ready){
        if (actual_char == -1) {
           //actual_char = kbd_scan_code_to_ascii(scan_code);
           actual_char = scan_code;
           //printf("actual char: [%d]", actual_char); 
        }
    }
    pic_acknowledge(1);
    enable_interrupts();
}

static int kbd_open(vnode_t *n)
{
    UNUSED_ARGUMENT(n);

    return 0;
}

static int kbd_lookup(vnode_t *d, char const *n, vnode_t *o)
{
    UNUSED_ARGUMENT(d);
    UNUSED_ARGUMENT(n);
    UNUSED_ARGUMENT(o);

    return -1;
}

static int kbd_read(vnode_t *n, void *buf, size_t count)
{
    UNUSED_ARGUMENT(n);

    /* DO NOT MODIFY kbd_buffer.tail or kbd_buffer.count in this function,
     * it will introcude race conditions!
     */

    char ch;
    int i = 0;
    char *b = buf;

    while (count > 0) {
        while (kbd_buffer.head != kbd_buffer.tail) {
            ch = kbd_scan_code_to_ascii(*kbd_buffer.head++);
            if (kbd_buffer.head == kbd_buffer.buffer + KBD_BUFFER_SIZE) {
                kbd_buffer.head = kbd_buffer.buffer;
            }
            if (ch != -1) {
                b[i] = ch;
                ++i;
                --count;
            }
        }
    }

    return i;
}

static int kbd_write(vnode_t *n, char const *name, size_t count)
{
    UNUSED_ARGUMENT(n);
    UNUSED_ARGUMENT(name);
    UNUSED_ARGUMENT(count);

    return -1;
}

static int kbd_getattr(vnode_t *n, vattr_t *a)
{
    UNUSED_ARGUMENT(n);
    UNUSED_ARGUMENT(a);

    return -1;
}

uint32_t kbd_init(void)
{
    register_interrupt_handler(KBD_INT_IDX, keyboard_handle_interrupt);

    kbd_buffer.count = 0;
    kbd_buffer.head = kbd_buffer.buffer;
    kbd_buffer.tail = kbd_buffer.buffer;

    kbd_vnodeops.vn_open = &kbd_open;
    kbd_vnodeops.vn_lookup = &kbd_lookup;
    kbd_vnodeops.vn_read = &kbd_read;
    kbd_vnodeops.vn_write = &kbd_write;
    kbd_vnodeops.vn_getattr = &kbd_getattr;

    kbd_vnode.v_op = &kbd_vnodeops;
    kbd_vnode.v_data = 0;

    return 0;
}

int kbd_get_vnode(vnode_t *out)
{
    out->v_op = kbd_vnode.v_op;
    out->v_data = kbd_vnode.v_data;

    return 0;
}

static void toggle_left_shift(void)
{
    is_lshift_down = is_lshift_down ? 0 : 1;
}

static void toggle_right_shift(void)
{
    is_rshift_down = is_rshift_down ? 0 : 1;
}

static void toggle_caps_lock(void)
{
    is_caps_lock_pressed = is_caps_lock_pressed ? 0 : 1;
}

static char handle_caps_lock(uint8_t ch)
{
    if (ch >= 'a' && ch <= 'z') {
        return ch + 'A' - 'a';
    }
    return ch;
}

static char handle_shift(uint8_t ch)
{
    /* Alphabetic characters */
    if (ch >= 'a' && ch <= 'z') {
        return ch + 'A' - 'a';
    }

    /* Number characters */
    switch (ch) {
        case '0':
            return ')';
        case '1':
            return '!';
        case '2':
            return '@';
        case '3':
            return '#';
        case '4':
            return '$';
        case '5':
            return '%';
        case '6':

            return '^';
        case '7':
            return '&';
        case '8':
            return '*';
        case '9':
            return '(';
        default:
            break;
    }

    /* Special charachters */
    switch (ch) {
        case '-':
            return '_';
        case '=':
            return '+';
        case '[':
            return '{';
        case ']':
            return '}';
        case '\\':
            return '|';
        case ';':
            return ':';
        case '\'':
            return '\"';
        case ',':
            return '<';
        case '.':
            return '>';
        case '/':
            return '?';
        case '`':
            return '~';
    }

    return ch;
}

uint8_t kbd_read_scan_code(void)
{
    return inb(KBD_DATA_PORT);
}

static char kbd_scan_code_to_ascii(uint8_t scan_code)
{
    char ch = -1;

    if (scan_code & 0x80) {
        scan_code &= 0x7F; /* clear the bit set by key break */

        switch (scan_code) {
            case KBD_SC_LSHIFT:
                toggle_left_shift();
                break;
            case KBD_SC_RSHIFT:
                toggle_right_shift();
                break;
            case KBD_SC_CAPSLOCK:
                toggle_caps_lock();
                break;
            default:
                break;
        }

        return -1;
    }

    switch (scan_code) {
        case KBD_SC_A: ch = 'a'; break;
        case KBD_SC_B: ch = 'b'; break;
        case KBD_SC_C: ch = 'c'; break;
        case KBD_SC_D: ch = 'd'; break;
        case KBD_SC_E: ch = 'e'; break;
        case KBD_SC_F: ch = 'f'; break;
        case KBD_SC_G: ch = 'g'; break;
        case KBD_SC_H: ch = 'h'; break;
        case KBD_SC_I: ch = 'i'; break;
        case KBD_SC_J: ch = 'j'; break;
        case KBD_SC_K: ch = 'k'; break;
        case KBD_SC_L: ch = 'l'; break;
        case KBD_SC_M: ch = 'm'; break;
        case KBD_SC_N: ch = 'n'; break;
        case KBD_SC_O: ch = 'o'; break;
        case KBD_SC_P: ch = 'p'; break;
        case KBD_SC_Q: ch = 'q'; break;
        case KBD_SC_R: ch = 'r'; break;
        case KBD_SC_S: ch = 's'; break;
        case KBD_SC_T: ch = 't'; break;
        case KBD_SC_U: ch = 'u'; break;
        case KBD_SC_V: ch = 'v'; break;
        case KBD_SC_W: ch = 'w'; break;
        case KBD_SC_X: ch = 'x'; break;
        case KBD_SC_Y: ch = 'y'; break;
        case KBD_SC_Z: ch = 'z'; break;
        
        case KBD_SC_0: ch = '0'; break;
        case KBD_SC_1: ch = '1'; break;
        case KBD_SC_2: ch = '2'; break;
        case KBD_SC_3: ch = '3'; break;
        case KBD_SC_4: ch = '4'; break;
        case KBD_SC_5: ch = '5'; break;
        case KBD_SC_6: ch = '6'; break;
        case KBD_SC_7: ch = '7'; break;
        case KBD_SC_8: ch = '8'; break;
        case KBD_SC_9: ch = '9'; break;
        
        case KBD_SC_ENTER: ch = '\r'; break;
        case KBD_SC_SPACE: ch = ' '; break;
        case KBD_SC_BS:    ch = 8; break; // Backspace
        case KBD_SC_TAB:   ch = '\t'; break;
        
        case KBD_SC_DASH:     ch = '\''; break; 

        case KBD_SC_EQUALS:   ch = 168; break; 
        
        case KBD_SC_LBRACKET: ch = '\''; break; // Representado simple
        
        case KBD_SC_RBRACKET: ch = '+'; break; 
        
        case KBD_SC_BSLASH:   ch = '}'; break; 
        
        case KBD_SC_SCOLON:   ch = 164; break; 
        
        case KBD_SC_QUOTE:    ch = '{'; break; 
        
        case KBD_SC_COMMA:    ch = ','; break; 
        
        case KBD_SC_DOT:      ch = '.'; break; 
        
        case KBD_SC_FSLASH:   ch = '-'; break; 
        
        case KBD_SC_TILDE:    ch = '|'; break;
        
        case KBD_SC_LESS:     ch = '<'; break;

        case KBD_SC_LSHIFT:
            toggle_left_shift();
            break;
        case KBD_SC_RSHIFT:
            toggle_right_shift();
            break;
        default:
            return -1;
    }

    if (is_caps_lock_pressed) {
        ch = handle_caps_lock(ch);
    }

    if (is_lshift_down || is_rshift_down) {
        ch = handle_shift(ch);
    }

    return ch;
}

/// ADDED:
static inline void io_wait(void) {
    outb(0x80, 0);
}

void kbd_hw_enable(void)
{
    // Enable PS/2 keyboard port
    outb(0x64, 0xAE);
    io_wait();

    // Optional but recommended: enable IRQ1 in controller config
    outb(0x64, 0x20);      // read controller command byte
    io_wait();
    uint8_t status = inb(0x60);
    status |= 0x01;        // enable IRQ1
    outb(0x64, 0x60);
    io_wait();
    outb(0x60, status);
}

static void enable_get_char(){
    get_char_ready = 1;
}

static void disable_get_char(){
    get_char_ready = 0;
    actual_char = -1;
}

static void enable_only_keyboard_interrupt(void) {
    outb(0x21, 0xFD);
}

void disable_timer_interrupt(void) {
    uint8_t mask = inb(0x21);
    mask = mask | 0x01;
    outb(0x21, mask);
}

void enable_timer_interrupt(void) {
    uint8_t mask = inb(0x21);
    mask = mask & ~0x01;
    outb(0x21, mask);
}

uint8_t kgetchar(void)
{
    enable_interrupts();
    disable_timer_interrupt();
    enable_get_char();
    for (;;) {

        if (actual_char != -1) {
            disable_interrupts();
            char new_char = actual_char;
            disable_get_char();
            enable_interrupts();
            enable_timer_interrupt();
            return (uint8_t)new_char;
        }
    }
}

