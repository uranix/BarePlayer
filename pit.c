#include "pit.h"
#include "intrin.h"
#include "types.h"

void set_pit_freq(int freq) {
	u16 reload = 1193182 / freq;
	outb(0x43, 0x34);
	outb(0x40, reload & 0xff);
	outb(0x40, (reload >> 8) & 0xff);
}

