#include "types.h"
#include "constants.h"

#include "arch/stdio.h"
#include "drivers/io.h"
#include "drivers/io/serial_handler.h"

#include "drivers/io/keyboard.h"


/* The I/O ports */
#define FB_COMMAND_PORT 0x3D4
#define FB_DATA_PORT    0x3D5

#define KBD_DATA_PORT   0x60
#define KBD_STATUS_PORT 0x64

/* The I/O port commands */
#define FB_HIGH_BYTE_COMMAND 14
#define FB_LOW_BYTE_COMMAND  15

/* Framebuffer memory address */
#define FB_MEMORY      0xB8000
#define FB_NUM_COLS    80
#define FB_NUM_ROWS    25
#define BLACK_ON_WHITE 0x0F  // atributo de color: texto negro, fondo blanco
#define SERIAL_PORT 0x3F8   // COM1




static uint8_t *fb = (uint8_t *) FB_MEMORY;

/* Escribe un caracter en (row, col) */
void fb_write(uint8_t c, uint32_t row, uint32_t col)
{
    uint8_t *cell = fb + 2 * (row * FB_NUM_COLS + col);
    cell[0] = c;
    cell[1] = BLACK_ON_WHITE;
}

/* Limpia la pantalla */
void clear(void)
{
    for (uint32_t i = 0; i < FB_NUM_ROWS; i++) {
        for (uint32_t j = 0; j < FB_NUM_COLS; j++) {
            fb_write(' ', i, j);
        }
    }
}

/* Mueve el cursor a la celda número `pos` */
void move_cursor(uint16_t pos)
{
    outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
    outb(FB_DATA_PORT, (pos >> 8) & 0x00FF);
    outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
    outb(FB_DATA_PORT, pos & 0x00FF);
}


//devuelve la posicion actual del cursor
uint16_t get_cursor_position(){
    uint16_t position = 0;

    //obtengo los bytes mas altos de la posicion actual del cursos a traves del puerto VGA
    outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
    position = ((uint16_t)inb(FB_DATA_PORT)) << 8;

    //obtengo los bytes mas bajos de la posicion actual del cursos a traves del puerto VGA
    outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
    position |= (uint16_t)inb(FB_DATA_PORT);
    return position;
}

// TODO: check if it works

/* Takes a single input character from standard input.
Talks directly with the keyboard hardware.
Keyboard gives scan codes which we convert to ASCII. */
long getchar(void) {
    // PANIC("en getchar");
    return kgetchar();
}

void putchar(char ch) {
    serial_putchar(ch);
    uint16_t pos = get_cursor_position();
    uint32_t row = pos / FB_NUM_COLS;
    uint32_t col = pos % FB_NUM_COLS;

    if (ch == '\n') {
        col = 0;
        row++;
    } else if (ch == '\r') {
        col = 0;
    } else {
        fb_write(ch, row, col);
        col++;
        if (col >= FB_NUM_COLS) {
            col = 0;
            row++;
        }
    }

    if (row >= FB_NUM_ROWS) {
        for (uint32_t r = 1; r < FB_NUM_ROWS; r++) {
            for (uint32_t c = 0; c < FB_NUM_COLS; c++) {
                uint8_t *src = fb + 2 * (r * FB_NUM_COLS + c);
                uint8_t *dst = fb + 2 * ((r - 1) * FB_NUM_COLS + c);
                dst[0] = src[0];
                dst[1] = src[1];
            }
        }
        for (uint32_t c = 0; c < FB_NUM_COLS; c++) {
            fb_write(' ', FB_NUM_ROWS - 1, c);
        }
        row = FB_NUM_ROWS - 1;
    }

    move_cursor(row * FB_NUM_COLS + col);
}

/* Escribe una cadena de texto a partir de la celda indicada */
int k_write(const uint8_t *buf, int len, uint32_t cell)
{
    uint32_t row = cell / FB_NUM_COLS;
    uint32_t col = cell % FB_NUM_COLS;
    uint32_t current_cell = cell+1;
    

    int written = 0;

    for (int i = 0; i < len; i++) {
        if (buf[i] == '\0') {   // fin de string
            break;
        }

        fb_write((uint8_t)buf[i], row, col);
        move_cursor(current_cell++);
        written++;

        col++;
        if (col >= FB_NUM_COLS) {
            col = 0;
            row++;
        }
    }
    return written;
}
