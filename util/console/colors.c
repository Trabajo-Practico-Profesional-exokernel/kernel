
#include "colors.h"
#include "types.h"
//#include "stdio.h"
#include "stdio.h"


void printGreen(const uint8_t* text) {
  printf((const uint8_t*)"\033[0;32m%s\033[0m", text);
}

void printRed(const uint8_t* text) {
  printf((const uint8_t*)"\033[0;31m%s\033[0m", text);
}

void printYellow(const uint8_t* text) {
  printf((const uint8_t*)"\033[0;33m%s\033[0m", text);
}

void printPurple(const uint8_t* text) {
  printf((const uint8_t*)"\033[0;35m%s\033[0m", text);
}

void printBlue(const uint8_t* text) {
  printf((const uint8_t*)"\033[0;34m%s\033[0m", text);
}
