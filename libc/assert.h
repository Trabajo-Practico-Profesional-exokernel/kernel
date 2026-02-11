#ifndef _LIBC_ASSERT_H
#define _LIBC_ASSERT_H

#include "constants.h"

#define assert(x)		\
	do { if (!(x)) PANIC("assertion failed: %s", #x); } while (0)

	// static_assert(x) will generate a compile-time error if 'x' is false.
#define static_assert(x)	switch (x) case 0: case (x):

#endif 
