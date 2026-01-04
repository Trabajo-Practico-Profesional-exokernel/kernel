#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "vnode.h"



uint32_t kbd_init(void);
int kbd_get_vnode(vnode_t *out);
long kgetchar(void);
void keyboard_handle_interrupt(void);

#endif /* KEYBOARD_H */
