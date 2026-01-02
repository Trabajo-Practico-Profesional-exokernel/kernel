/* util.c */

#include "common.h"
#include "util.h"

/* Funciones de utilidad para manipulación de strings y conversión numérica.
   Las funciones de impresión y memoria (bcopy, bzero, etc.) han sido
   reemplazadas por macros en util.h que utilizan la librería estándar del kernel.
*/

void reverse(char *s)
{
    int c, i, j;

    for (i = 0, j = strlen(s) - 1; i < j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

void itoa(unsigned int n, char *s) {
    int i = 0;
    
    /* Nota: La implementación original recibía 'unsigned int' pero intentaba 
       verificar signos negativos (n < 0), lo cual siempre es falso para unsigned.
       Se asume conversión directa de unsigned a string decimal.
       Si se necesita imprimir negativos, usar printf con %d.
    */
    
    do {
        s[i++] = n % 10 + '0';
    } while((n /= 10) > 0);
    
    s[i++] = 0;
    reverse(s);
}

int atoi(const char *s) {
    int n = 0, sign = 1;
    
    if (*s == '-') {
        sign = -1;
        s++;
    }
    while (*s) {
        n = n * 10 + (*s) - '0';
        s++;
    }
    return sign * n;
}

void itohex(unsigned int n, char *s)
{
    int i, d;

    i = 0;
    do {
        d = n % 16;
        if (d < 10) {
            s[i++] = d + '0';
        } else {
            s[i++] = d - 10 + 'a';
        }
    } while ((n /= 16) > 0);
    s[i++] = 0;
    reverse(s);
}