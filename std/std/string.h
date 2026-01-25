#ifndef STRING_H
#define STRING_H

#include "types.h"

int strlen(const char *s);
int strnlen(const char *s, size_t size);
char *strcpy(char *dst, const char *src);
char *strncpy(char *dst, const char *src, size_t size);
char *strcat(char *dst, const char *src);
size_t strlcpy(char *dst, const char *src, size_t size);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t size);
char *strchr(const char *s, char c);
char *strfind(const char *s, char c);


void split_by_once(char* src, char** after_delim, char delimeter);
char* extract_once(char* src, char** extracted_arg, char delimeter);

int parse_num_and_msg(char* input_buffer, char** msg_out);

void *memset(void *dst, int c, size_t len);
void *memcpy(void *dst, const void *src, size_t len);
void *memmove(void *dst, const void *src, size_t len);
int memcmp(const void *s1, const void *s2, size_t len);
void *memfind(const void *s, int c, size_t len);

long strtol(const char *s, char **endptr, int base);
// int atoi(const char *str);
int atoi(const char *s);
void itoa(unsigned int n, char *s);
void itohex(unsigned int n, char *s);
void reverse(char *s);
void int_to_string(int n, char s[]);
int parse_3_args(char *input, char *argv[3]);
int parse_str_int(char *input, char **out_str, int *out_int);
#endif /* not STRING_H */
