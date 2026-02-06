#include "types.h"


// Escribe una unica letra en la fila row y columna col
// Fila va de 0 a 79
void fb_write(uint8_t c, uint32_t row, uint32_t col);

//obtiene la posicion actual del cursor
int get_cursor_position();

// Escribe una unica letra en una celda
int k_write(const uint8_t buf[], int len, uint32_t cell);
