// Basic string routines.  Not hardware optimized, but not shabby.

#include "std/string.h"

// Using assembly for memset/memmove
// makes some difference on real hardware,
#define ASM 0 // 0 == no ASM for now.. since ASM its for x86 for now too 

int
strlen(const char *s)
{
	int n;

	for (n = 0; *s != '\0'; s++)
		n++;
	return n;
}

int
strnlen(const char *s, size_t size)
{
	int n;

	for (n = 0; size > 0 && *s != '\0'; s++, size--)
		n++;
	return n;
}

char *
strcpy(char *dst, const char *src)
{
	char *ret;

	ret = dst;
	while ((*dst++ = *src++) != '\0')
		/* do nothing */;
	return ret;
}

char *
strcat(char *dst, const char *src)
{
	int len = strlen(dst);
	strcpy(dst + len, src);
	return dst;
}

char *
strncpy(char *dst, const char *src, size_t size)
{
	size_t i;
	char *ret;

	ret = dst;
	for (i = 0; i < size; i++) {
		*dst++ = *src;
		// If strlen(src) < size, null-pad 'dst' out to 'size' chars
		if (*src != '\0')
			src++;
	}
	return ret;
}

size_t
strlcpy(char *dst, const char *src, size_t size)
{
	char *dst_in;

	dst_in = dst;
	if (size > 0) {
		while (--size > 0 && *src != '\0')
			*dst++ = *src++;
		*dst = '\0';
	}
	return dst - dst_in;
}

int
strcmp(const char *p, const char *q)
{
	while (*p && *p == *q)
		p++, q++;
	return (int) ((unsigned char) *p - (unsigned char) *q);
}

int
strncmp(const char *p, const char *q, size_t n)
{
	while (n > 0 && *p && *p == *q)
		n--, p++, q++;
	if (n == 0)
		return 0;
	else
		return (int) ((unsigned char) *p - (unsigned char) *q);
}

// Return a pointer to the first occurrence of 'c' in 's',
// or a null pointer if the string has no 'c'.
char *
strchr(const char *s, char c)
{
	for (; *s; s++)
		if (*s == c)
			return (char *) s;
	return 0;
}

// Return a pointer to the first occurrence of 'c' in 's',
// or a pointer to the string-ending null character if the string has no 'c'.
char *
strfind(const char *s, char c)
{
	for (; *s; s++)
		if (*s == c)
			break;
	return (char *) s;
}

void split_by_once(char* src, char** after_delim, char delimeter){
    char* space_char = strchr(src, delimeter);
    if (space_char == 0){
        *after_delim = NULL; //Replace args to an empty string
    } else {
        // Next char is start of args...
        *after_delim = space_char+1;
        *space_char =0; //Replace value by 0 so that input_buf ends here for strncmp!
    }
}

char* extract_once(char* src, char** extracted_arg, char delimeter){
    char* space_char = strchr(src, delimeter);
	*extracted_arg = src;
    if (space_char == 0){
		return NULL;
	}
	*space_char =0; //Replace value by 0 so that input_buf ends here for strncmp!
	return space_char+1;
}


#define ERR_PARSE_CODE -1

int parse_num_and_msg(char* input_buffer, char** msg_out) {
    char* endptr;
    long number = strtol(input_buffer, &endptr, 10);

    if (endptr == input_buffer) {
        return ERR_PARSE_CODE; 
    }

    while (*endptr == ' ') {
        endptr++;
    }

    *msg_out = endptr;
    
    return (int)number;
}

#if ASM
void *
memset(void *v, int c, size_t n)
{
	char *p = v;

	if (n == 0)
		return v;
	if ((int) v % 4 == 0 && n % 4 == 0) {
		c &= 0xFF;
		c = (c << 24) | (c << 16) | (c << 8) | c;
		__asm__ volatile("cld; rep stosl\n"
		             : "=D"(p), "=c"(n)
		             : "D"(p), "a"(c), "c"(n / 4)
		             : "cc", "memory");
	} else
		__asm__ volatile("cld; rep stosb\n"
		             : "=D"(p), "=c"(n)
		             : "0"(p), "a"(c), "1"(n)
		             : "cc", "memory");
	return v;
}

void *
memmove(void *dst, const void *src, size_t n)
{
	const char *s;
	char *d;

	s = src;
	d = dst;
	if (s < d && s + n > d) {
		s += n;
		d += n;
		if ((int) s % 4 == 0 && (int) d % 4 == 0 && n % 4 == 0)
			__asm__ volatile("std; rep movsl\n" ::"D"(d - 4),
			             "S"(s - 4),
			             "c"(n / 4)
			             : "cc", "memory");
		else
			__asm__ volatile("std; rep movsb\n" ::"D"(d - 1),
			             "S"(s - 1),
			             "c"(n)
			             : "cc", "memory");
		// Some versions of GCC rely on DF being clear
		__asm__ volatile("cld" ::: "cc");
	} else {
		if ((int) s % 4 == 0 && (int) d % 4 == 0 && n % 4 == 0)
			__asm__ volatile("cld; rep movsl\n" ::"D"(d), "S"(s), "c"(n / 4)
			             : "cc", "memory");
		else
			__asm__ volatile("cld; rep movsb\n" ::"D"(d), "S"(s), "c"(n)
			             : "cc", "memory");
	}
	return dst;
}

#else

void *
memset(void *v, int c, size_t n)
{
	char *p;
	int m;

	p = v;
	m = n;
	while (--m >= 0)
		*p++ = c;

	return v;
}

void *
memmove(void *dst, const void *src, size_t n)
{
	const char *s;
	char *d;

	s = src;
	d = dst;
	if (s < d && s + n > d) {
		s += n;
		d += n;
		while (n-- > 0)
			*--d = *--s;
	} else
		while (n-- > 0)
			*d++ = *s++;

	return dst;
}
#endif

void *
memcpy(void *dst, const void *src, size_t n)
{
	return memmove(dst, src, n);
}

int
memcmp(const void *v1, const void *v2, size_t n)
{
	const uint8_t *s1 = (const uint8_t *) v1;
	const uint8_t *s2 = (const uint8_t *) v2;

	while (n-- > 0) {
		if (*s1 != *s2)
			return (int) *s1 - (int) *s2;
		s1++, s2++;
	}

	return 0;
}

void *
memfind(const void *s, int c, size_t n)
{
	const void *ends = (const char *) s + n;
	for (; s < ends; s++)
		if (*(const unsigned char *) s == (unsigned char) c)
			break;
	return (void *) s;
}

long
strtol(const char *s, char **endptr, int base)
{
	int neg = 0;
	long val = 0;

	// gobble initial whitespace
	while (*s == ' ' || *s == '\t')
		s++;

	// plus/minus sign
	if (*s == '+')
		s++;
	else if (*s == '-')
		s++, neg = 1;

	// hex or octal base prefix
	if ((base == 0 || base == 16) && (s[0] == '0' && s[1] == 'x'))
		s += 2, base = 16;
	else if (base == 0 && s[0] == '0')
		s++, base = 8;
	else if (base == 0)
		base = 10;

	// digits
	while (1) {
		int dig;

		if (*s >= '0' && *s <= '9')
			dig = *s - '0';
		else if (*s >= 'a' && *s <= 'z')
			dig = *s - 'a' + 10;
		else if (*s >= 'A' && *s <= 'Z')
			dig = *s - 'A' + 10;
		else
			break;
		if (dig >= base)
			break;
		s++, val = (val * base) + dig;
		// we don't properly detect overflow!
	}

	if (endptr)
		*endptr = (char *) s;
	return (neg ? -val : val);
}


// int atoi(const char *str) {
//     int sign = 1;
//     long long result = 0; // Use long long to handle intermediate sums and check for overflow
//     int i = 0;

//     // 1. Ignore leading whitespace
//     while (str[i] == ' ' || str[i] == '\t' || str[i] == '\n') {
//         i++;
//     }

//     // 2. Check for an optional sign character
//     if (str[i] == '-' || str[i] == '+') {
//         if (str[i] == '-') {
//             sign = -1;
//         }
//         i++;
//     }

//     // 3. Convert the digits
//     while (str[i] >= '0' && str[i] <= '9') {
//         int digit = str[i] - '0';
        
//         // 4. Handle potential overflow/underflow
//         if (result > INT_MAX / 10 || (result == INT_MAX / 10 && digit > INT_MAX % 10)) {
//             return (sign == 1) ? INT_MAX : INT_MIN;
//         }

//         result = result * 10 + digit;
//         i++;
//     }

//     // 5. Apply the sign and return the final result
//     return (int)(result * sign);
// }

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
