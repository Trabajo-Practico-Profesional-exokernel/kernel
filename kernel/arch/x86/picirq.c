/*
==================================================
  PIC remap
==================================================
	cmd master: 0x20
	data master: 0x21
	cmd slave : 0xA0
	data slave: 0xA1
*/
void pic_init(void) {
    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);
}

