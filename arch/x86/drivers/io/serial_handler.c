#include "drivers/io.h"

#define SERIAL_COM1_PORT 0x3F8 
// COM1 base port... definido por separado para claridad por las dudas.

void serial_init() {
    outb(SERIAL_COM1_PORT + 1, 0x00); // Disable all interrupts
    outb(SERIAL_COM1_PORT + 3, 0x80); // Enable DLAB
    outb(SERIAL_COM1_PORT + 0, 0x03); // Set divisor to 3 (38400 baud)
    outb(SERIAL_COM1_PORT + 1, 0x00);
    outb(SERIAL_COM1_PORT + 3, 0x03); // 8 bits, no parity, one stop bit
    outb(SERIAL_COM1_PORT + 2, 0xC7); // Enable FIFO, clear with 14-byte threshold
    outb(SERIAL_COM1_PORT + 4, 0x0B); // IRQs enabled, RTS/DSR set
}
int serial_is_transmit_ready() {
    return inb(SERIAL_COM1_PORT + 5) & 0x20;
}

void wait_serial_init(){
    while (!serial_is_transmit_ready());    
}

void serial_putchar(char c) {
    outb(SERIAL_COM1_PORT, c);
}
