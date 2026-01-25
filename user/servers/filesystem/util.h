/* util.h */

#ifndef UTIL_INCLUDED
#define UTIL_INCLUDED

#include "common.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "stdio.h"
#include "console/debug.h"

// Funciones de conversión


// Mapeo a funciones estándar del kernel (std/string.h)
// int strlen(const char *s); // Ya definida en std/string.h

// Adaptación de funciones de memoria legacy
#define bcopy(src, dest, size) memcpy(dest, src, size)
#define bzero(addr, size) memset(addr, 0, size)

// Helper de comparación (simple-linux-fs espera TRUE si son iguales)
#define same_string(s1, s2) (strcmp(s1, s2) == 0)

// Funciones de pantalla (Eliminadas - Sin acceso a hardware de video)
// void clear_screen(int minx, int miny, int maxx, int maxy);
// void scroll(int minx, int miny, int maxx, int maxy);
// int peek_screen(int x, int y);

// Funciones de tiempo (Eliminadas - Sin acceso a timer hardware)
// void ms_delay(uint32_t msecs);
// uint64_t get_timer(void);

// Adaptación de funciones de impresión para usar printf (ignoran fila/columna)
#define print_char(l, c, ch) debug_printf("%c", ch)
#define print_int(l, c, num) debug_printf("%d", num)
#define print_hex(l, c, num) debug_printf("%x", num)
#define print_str(l, c, str) debug_printf("%s", str)

// Debug print
#define dprint(str) debug_printf("%s\n", str)

#endif