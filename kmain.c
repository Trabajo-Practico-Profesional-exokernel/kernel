#define UNUSED_ARGUMENT(x) (void) x

#define FB_MEMORY 0xB8000
#define FB_NUM_COLS 80
#define FB_NUM_ROWS 25

#define FB_CURSOR_DATA_PORT 0x3D5
#define FB_CURSOR_INDEX_PORT 0x3D4

#define FB_HIGH_BYTE 14
#define FB_LOW_BYTE 15

#define BLACK_ON_WHITE 0x0F

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

static uint8_t *fb = (uint8_t *) FB_MEMORY;

void fb_write(uint8_t c, uint32_t row, uint32_t col)
{
    uint8_t *cell = fb + 2 * (row*FB_NUM_COLS + col);
    cell[0] = c;
    cell[1] = BLACK_ON_WHITE;
}

void fb_clear()
{
    uint8_t i, j;
    for (i = 0; i < FB_NUM_ROWS; ++i) {
        for (j = 0; j < FB_NUM_COLS; ++j) {
            fb_write(' ', i, j);
        }
    }
}

int kmain(void *mboot, unsigned int magic_number)
{
    UNUSED_ARGUMENT(mboot);
    UNUSED_ARGUMENT(magic_number);
    fb_clear();
    fb_write('h', 0, 0);
    fb_write('e', 0, 1);
    fb_write('l', 0, 2);
    fb_write('l', 0, 3);
    fb_write('o', 0, 4);
    return 0xDEADBEEF;
}
