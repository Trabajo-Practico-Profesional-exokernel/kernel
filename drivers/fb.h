#include "io.h"

// Limpia la pantalla
void fb_clear();

// Escribe una unica letra en la fila row y columna col
// Fila va de 0 a 79
void fb_write(uint8_t c, uint32_t row, uint32_t col);

// Mueve el cursor
void fb_move_cursor(uint16_t pos);

//obtiene la posicion actual del cursor
int get_cursor_position();

void putchar(char ch);

// Escribe una unica letra en una celda
int k_write(const uint8_t buf[], int len, uint32_t cell);
