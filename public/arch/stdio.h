#ifndef INC_STDIO
#define INC_STDIO

#include "inc/types.h"

void putchar(char ch);

// Mueve el cursor
void move_cursor(uint16_t pos);

// Limpia la pantalla
void clear(void);

#endif /* !*/
