//#include "strutil.h"
//#include "string.h"
//#include "stdlib.h"
#include "types.h"
#include "constants.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"

uint8_t *strfind(const uint8_t *s, int32_t c) {
    for (; *s; s++)
        if (*s == (uint8_t)c)
            break;
    return (uint8_t *) s;
}

void split_by_once(uint8_t* src, uint8_t** after_delim, int32_t delimeter) {
    uint8_t* space_char = strchr(src, delimeter);
    if (space_char == NULL) {
        *after_delim = NULL; 
    } else {
        *after_delim = space_char + 1;
        *space_char = 0;
    }
}

uint8_t* extract_once(uint8_t* src, uint8_t** extracted_arg, int32_t delimeter) {
    uint8_t* space_char = strchr(src, delimeter);
    *extracted_arg = src;
    if (space_char == NULL) {
        return NULL;
    }
    *space_char = 0; 
    return space_char + 1;
}

int32_t parse_num_and_msg(uint8_t* input_buffer, uint8_t** msg_out) {
    uint8_t* endptr;
    int32_t number = strtol(input_buffer, &endptr, 10);

    if (endptr == input_buffer) {
        return ERROR; 
    }

    while (*endptr == ' ') {
        endptr++;
    }

    *msg_out = endptr;
    
    return number;
}

extern uint8_t *strtok(uint8_t *str, const uint8_t *delim);

int32_t parse_3_args(uint8_t *input, uint8_t *argv[3]) {
    int32_t count = 0;
    uint8_t *token = strtok(input, (const uint8_t*)" "); 

    while (token != NULL && count < 3) {
        argv[count++] = token;
        token = strtok(NULL, (const uint8_t*)" ");
    }
    
    return count;
}

int32_t parse_str_int(uint8_t *input, uint8_t **out_str, int32_t *out_int) {
    uint8_t *token = strtok(input, (const uint8_t*)" ");
    if (token == NULL) {
        return -1; 
    }
    *out_str = token;

    token = strtok(NULL, (const uint8_t*)" ");
    if (token == NULL) {
        return -1; 
    }
    
    *out_int = strtol(token, NULL, 8); 

    return 0;
}

void join_strings(char *dest, const char *s1, const char *s2) {
    int i = 0;
    int j = 0;

    while(s1[j] != '\0') {
        dest[i++] = s1[j++];
    }

    dest[i++] = ' ';
    
    j = 0;
    while(s2[j] != '\0') { 
        dest[i++] = s2[j++];
    }
    
    dest[i] = '\0';
}

char* split_at_first_space(char* str) {
    int i = 0;
    while(str[i] != '\0') {
        if (str[i] == ' ') {
            str[i] = '\0';
            return &str[i + 1];
        }
        i++;
    }
    return (char *)0;
}

static char* split_arg(char* str) {
    char* next = (char*)strchr((const uint8_t*)str, ' ');
    if (next) {
        *next = 0;
        next++;
        while(*next == ' ') next++;
    }
    return next;
}
