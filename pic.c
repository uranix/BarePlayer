#include "pic.h"
#include "intrin.h"

void remap_pic(u8 master, u8 slave) {
	
	outb(PIC1_COMMAND, ICW1_INIT + ICW1_ICW4); io_wait();
	outb(PIC1_DATA, master); io_wait();
	outb(PIC1_DATA, 0x04); io_wait();
	outb(PIC1_DATA, ICW4_8086); io_wait();
	
	outb(PIC2_COMMAND, ICW1_INIT + ICW1_ICW4); io_wait();
	outb(PIC2_DATA, slave); io_wait();
	outb(PIC2_DATA, 0x02); io_wait();
	outb(PIC2_DATA, ICW4_8086); io_wait();
	
	outb(PIC1_DATA, 0xff);
	outb(PIC2_DATA, 0xff);
}

