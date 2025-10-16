#define UNUSED_ARGUMENT(x) (void) x

#define FB_MEMORY 0xB8000
#define FB_NUM_COLS 80
#define FB_NUM_ROWS 25

#define FB_CURSOR_DATA_PORT 0x3D5
#define FB_CURSOR_INDEX_PORT 0x3D4

#define FB_HIGH_BYTE 14
#define FB_LOW_BYTE 15

#define BLACK_ON_WHITE 0x0F

#ifndef INCLUDE_IO_H
#define INCLUDE_IO_H

/** outb:
 *  Sends the given data to the given I/O port. Defined in io.s
 *
 *  @param port The I/O port to send the data to
 *  @param data The data to send to the I/O port
 */
void outb(unsigned short port, unsigned char data);
unsigned short inb(unsigned short port);

#endif /* INCLUDE_IO_H */