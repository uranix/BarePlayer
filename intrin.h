#ifndef __INTRIN_H__
#define __INTRIN_H__

#include "types.h"

static inline void outb( u16 port, u8 val )
{
    asm volatile( "outb %0, %1"
                  : : "a"(val), "Nd"(port) );
}

static inline u8 inb( u16 port )
{
	unsigned char val;
    asm volatile( "inb %1, %0"
                  : "=a"(val): "Nd"(port) );
	return val;
}

static inline void outw( u16 port, u16 val )
{
    asm volatile( "outw %0, %1"
                  : : "a"(val), "Nd"(port) );
}

static inline u16 inw( u16 port )
{
	unsigned char val;
    asm volatile( "inw %1, %0"
                  : "=a"(val): "Nd"(port) );
	return val;
}

static inline void io_wait()
{
	asm volatile(	"jmp 1f\n"
					"1: jmp 2f\n"
					"2: nop\n"
				);
}

static inline void zero_page(u8 *p) {
	asm (	"xorl %%eax, %%eax\n"
			"movl $0x400, %%ecx\n"
			"rep stosl\n"
		:
		: "D"(p)
		: "%eax", "%ecx");
}

static inline void kmemcpy(void *dst, void *src, u8 sz) {
	asm (	
			"push %%es\n"
			"push %%ds\n"
			"pop %%es\n"
			"rep movsb\n"
			"pop %%es\n"
		:
		: "D"(dst), "S"(src), "c"(sz));
}

static inline void kmemcpy4(void *dst, void *src, u8 sz) {
	asm (	
			"push %%es\n"
			"push %%ds\n"
			"pop %%es\n"
			"rep movsl\n"
			"pop %%es\n"
		:
		: "D"(dst), "S"(src), "c"(sz));
}

#endif
