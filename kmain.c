#include "io.h"
#include "fb.h"


int kmain(void *mboot, unsigned int magic_number)
{
    UNUSED_ARGUMENT(mboot);
    UNUSED_ARGUMENT(magic_number);
    fb_clear();

    const uint8_t msg[] = "hello";
    k_write(msg, 5, 0);
    return 0xDEADBEEF;
}
