#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "vnode.h"



uint32_t kbd_init(void);
int kbd_get_vnode(vnode_t *out);
void kbd_hw_enable(void);
void keyboard_handle_interrupt(void);
uint8_t kgetchar(void);

#endif /* KEYBOARD_H */
