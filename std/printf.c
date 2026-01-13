#include "std/printf.h"

static char digits[] = "0123456789ABCDEF";

void putchar(char ch);

enum DebugPrintMode actual_debug_print_mode = ENABLE;

void debug_printf(const char *fmt, ...){
  if (actual_debug_print_mode != ENABLE) return;

  char buf[256]; 
  va_list args;
  va_start(args, fmt);
  
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);
  printPurple("[DEBUG] ");
  printf("%s", buf); 
}

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

void printGreen(const char* text) {
  printf("\033[0;32m%s\033[0m", text);
}

void printRed(const char* text) {
  printf("\033[0;31m%s\033[0m", text);
}

void printYellow(const char* text) {
  printf("\033[0;33m%s\033[0m", text);
}

void printPurple(const char* text) {
  printf("\033[0;35m%s\033[0m", text);
}

void printBlue(const char* text) {
  printf("\033[0;34m%s\033[0m", text);
}

void disable_debug_print(void){
  actual_debug_print_mode = DISABLE;
}

void enable_debug_print(void){
  actual_debug_print_mode = ENABLE;
}

static int string_print_num(char *buf, size_t size, size_t *pos, long value, int base, bool sign) {
  char tmp[32];
  int i = 0;
  unsigned long uval = value;

  if (sign && (long)value < 0) {
      uval = -value;
      if (*pos < size - 1) buf[(*pos)++] = '-';
  }

  if (uval == 0) {
      tmp[i++] = '0';
  } else {
      while (uval > 0) {
          int digit = uval % base;
          tmp[i++] = (digit < 10) ? (digit + '0') : (digit - 10 + 'a');
          uval /= base;
      }
  }

  int chars_written = 0;
  while (i > 0) {
      i--;
      if (*pos < size - 1) {
          buf[(*pos)++] = tmp[i];
          chars_written++;
      }
  }
  return chars_written;
}

int vsnprintf(char *buf, size_t size, const char *fmt, va_list args) {
  size_t pos = 0;
  
  if (size == 0) return 0;

  for (const char *p = fmt; *p != '\0'; p++) {
      if (*p != '%') {
          if (pos < size - 1) buf[pos++] = *p;
          continue;
      }

      p++; 
      
      switch (*p) {
          case 'd': 
          case 'i': {
              int val = va_arg(args, int);
              string_print_num(buf, size, &pos, val, 10, true);
              break;
          }
          case 'u': { 
              unsigned int val = va_arg(args, unsigned int);
              string_print_num(buf, size, &pos, val, 10, false);
              break;
          }
          case 'x': 
          case 'p': {
              unsigned long val = va_arg(args, unsigned long);
              string_print_num(buf, size, &pos, val, 16, false);
              break;
          }
          case 's': { 
              const char *s = va_arg(args, const char *);
              if (!s) s = "(null)";
              while (*s) {
                  if (pos < size - 1) buf[pos++] = *s;
                  s++;
              }
              break;
          }
          case 'c': { 
              int c = va_arg(args, int);
              if (pos < size - 1) buf[pos++] = (char)c;
              break;
          }
          case '%': { 
              if (pos < size - 1) buf[pos++] = '%';
              break;
          }
          default:
              if (pos < size - 1) buf[pos++] = '%';
              if (pos < size - 1) buf[pos++] = *p;
              break;
      }
  }

  buf[pos] = '\0';
  return pos;
}

int snprintf(char *buf, size_t size, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  int ret = vsnprintf(buf, size, fmt, args);
  va_end(args);
  return ret;
}
