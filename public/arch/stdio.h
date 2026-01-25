#ifndef INC_STDIO
#define INC_STDIO

#include "types.h"

void putchar(char ch);
long getchar(void);

// Mueve el cursor
void move_cursor(uint16_t pos);

// Limpia la pantalla
void clear(void);

#endif /* !*/
