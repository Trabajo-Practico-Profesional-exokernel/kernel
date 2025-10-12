#include "../drivers/io.h"
#include "../drivers/fb.h"

#include "common.h"

int kmain(void *mboot, unsigned int magic_number)
{
    UNUSED_ARGUMENT(mboot);
    UNUSED_ARGUMENT(magic_number);
    fb_clear();
    fb_move_cursor(0);
    printf("HOLIS");
    return 0xDEADBEEF;
}
