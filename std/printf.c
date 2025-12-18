#include "std/printf.h"

static char digits[] = "0123456789ABCDEF";

void putchar(char ch);

static void
printint(int32_t xx, int base, int sgn)
{
  char buf[20];
  int i, neg;
  uint32_t x;

  neg = 0;
  if(sgn && xx < 0){
    neg = 1;
    x = -xx;
  } else {
    x = xx;
  }

  i = 0;
  do{
    buf[i++] = digits[x % base];
  }while((x /= base) != 0);
  if(neg)
    buf[i++] = '-';
  while(--i >= 0)
    putchar(buf[i]);
}

static void
printptr(uintptr_t x) {
  uintptr_t i;
  putchar('0');
  putchar('x');
  for (i = 0; i < (sizeof(uintptr_t) * 2); i++, x <<= 4)
    putchar(digits[x >> (sizeof(uintptr_t) * 8 - 4)]);
}

void
vprintf(const char *fmt, va_list ap)
{
  char *s;
  int c0, c1, c2, i, state;

  state = 0;
  for(i = 0; fmt[i]; i++){
    c0 = fmt[i] & 0xff;
    if(state == 0){
      if(c0 == '%'){
        state = '%';
      } else {
        putchar(c0);
      }
    } else if(state == '%'){
      c1 = c2 = 0;
      if(c0) c1 = fmt[i+1] & 0xff;
      if(c1) c2 = fmt[i+2] & 0xff;
      if(c0 == 'd'){
        printint(va_arg(ap, int), 10, 1);
      } else if(c0 == 'l' && c1 == 'd'){
        printint(va_arg(ap, uint64_t), 10, 1);
        i += 1;
      } else if(c0 == 'l' && c1 == 'l' && c2 == 'd'){
        printint(va_arg(ap, uint64_t), 10, 1);
        i += 2;
      } else if(c0 == 'u'){
        printint(va_arg(ap, uint32_t), 10, 0);
      } else if(c0 == 'l' && c1 == 'u'){
        printint(va_arg(ap, uint64_t), 10, 0);
        i += 1;
      } else if(c0 == 'l' && c1 == 'l' && c2 == 'u'){
        printint(va_arg(ap, uint64_t), 10, 0);
        i += 2;
      } else if(c0 == 'x'){
        printint(va_arg(ap, uint32_t), 16, 0);
      } else if(c0 == 'l' && c1 == 'x'){
        printint(va_arg(ap, uint64_t), 16, 0);
        i += 1;
      } else if(c0 == 'l' && c1 == 'l' && c2 == 'x'){
        printint(va_arg(ap, uint64_t), 16, 0);
        i += 2;
      } else if(c0 == 'p'){
        printptr(va_arg(ap, uint32_t)); //////////// ADDED: CHANGED FROM 64 TO 32
      } else if(c0 == 'c'){
        putchar(va_arg(ap, uint32_t));
      } else if(c0 == 's'){
        if((s = va_arg(ap, char*)) == 0)
          s = "(null)";
        for(; *s; s++)
          putchar(*s);
      } else if(c0 == '%'){
        putchar('%');
      } else {
        // Unknown % sequence.  Print it to draw attention.
        putchar('%');
        putchar(c0);
      }

      state = 0;
    }
  }
}


void printf(const char *fmt, ...) {
    va_list vargs;
    va_start(vargs, fmt);
    vprintf(fmt, vargs);
    va_end(vargs);
}
