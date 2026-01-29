#ifndef _PARSERS_STRUTIL_H
#define _PARSERS_STRUTIL_H

#include "types.h"

uint8_t *strfind(const uint8_t *s, int32_t c); 

void split_by_once(uint8_t* src, uint8_t** after_delim, int32_t delimeter);
uint8_t* extract_once(uint8_t* src, uint8_t** extracted_arg, int32_t delimeter);

int32_t parse_num_and_msg(uint8_t* input_buffer, uint8_t** msg_out);
int32_t parse_3_args(uint8_t *input, uint8_t *argv[3]);
int32_t parse_str_int(uint8_t *input, uint8_t **out_str, int32_t *out_int);

void join_strings(char *dest, const char *s1, const char *s2);
char* split_at_first_space(char* str);
static char* split_arg(char* str);
#endif /* _PARSERS_STRUTIL_H */