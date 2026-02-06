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
    // ICW1: initialize command
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    // ICW2 - vector offsets
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    // ICW3 - chaining master/slave
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    // ICW4
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    // UNMASK (enables every picirq interrupt)
	// TODO: config lapic
    //outb(0x21, 0x00);
    //outb(0xA1, 0x00);

	outb(0x20, 0x68);             /* clear specific mask */
	outb(0x20, 0x0a);             /* read IRR by default */

	outb(0xA0, 0x68);               /* OCW3 */
	outb(0xA0, 0x0a);               /* OCW3 */

}
