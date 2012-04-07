#include "idt.h"
#include "pic.h"
#include "intrin.h"
#include "output.h"

static u8 IDT[8*0x100];
struct {
	u16 size;
	u32 offset;
} __attribute__((packed)) IDTD = {.size = sizeof(IDT)-1, .offset = (u32)IDT};
static handler IDT_handlers[0x100] = {0};
static u8 idt_handlers_raw[0x100*33];

static void int_gate(u32 intr, ...);
static u32 int_gate_dup = (u32)int_gate;
static u32 int_gate_addr = (u32)&int_gate_dup;

handler register_handler(u32 intr, handler hdl) {
	register handler old = hdl;
	asm volatile (	"pushf\n"
					"cli\n"
					"xchgl (%3,%2,0x4), %0\n"
					"popf"
					: "=a"(old)
					: "0"(old), "r"(intr), "r"(IDT_handlers));
	return old;
}

u32 check_ints()
{
	u32 res;
	asm (	"pushfl\n"
			"popl %%eax\n"
			"shrl $9, %%eax\n"
			"andl $1, %%eax\n"
		: "=a"(res));
	return res;
}

void mask_irq(u8 irq) {
	if (irq < 8) {
		u8 state = inb(PIC1_DATA);
		state |= (1 << irq);
		outb(PIC1_DATA, state);
	} else {
		u8 state = inb(PIC2_DATA);
		state |= (1 << irq) >> 8;
		outb(PIC2_DATA, state);
	}
}

void unmask_irq(u8 irq) {
	if (irq < 8) {
		u8 state = inb(PIC1_DATA);
		state &= (~(1 << irq));
		outb(PIC1_DATA, state);
	} else {
		u8 state = inb(PIC2_DATA);
		state &= (~((1 << irq) >> 8));
		outb(PIC2_DATA, state);
	}
}

static void int_gate(u32 intr, ...)
{
	frame *args = (frame *)((u32 *)&intr+1);
	u32 handled = 0;

	if (IDT_handlers[intr]) {
		handled = IDT_handlers[intr](intr, args);
	}
	
	if ((intr < 0x20) && !handled) {
		asm ( "cli; hlt" );
	}

	/* drop IRQ signal from PIC */
	if (intr < 0x30) {
		if (intr >= 0x28)
			outb(PIC2_COMMAND, PIC_EOI);
		outb(PIC1_COMMAND, PIC_EOI);
	}
}

static void fill_idt_entry(u8 *entry, u16 selector, u32 offset, u8 type)
{
	*(u16 *)entry = offset & 0xffff;
	*(u16 *)(&entry[2]) = selector;
	entry[4] = 0;
	entry[5] = type;
	*(u16 *)(&entry[6]) = (offset & 0xffff0000) >> 16;
}

void setup_idt()
{
	u32 i; 
	u8 *p = idt_handlers_raw, *q;

	for (i=0, q=p; i<0x100; i++, q=p) {
		/*
00000000  1E                push ds
00000001  06                push es
00000002  0FA0              push fs
00000004  0FA8              push gs
00000006  60                pushad
00000007  16                push ss
00000008  1F                pop ds
00000009  68FE000000        push dword 0xfe
0000000E  FF1534120000      call dword near [dword 0x1234]
00000014  58                pop eax
00000015  61                popad
00000016  0FA9              pop gs
00000018  0FA1              pop fs
0000001A  07                pop es
0000001B  1F                pop ds
0000001C  83C400            add esp,byte +0x0
0000001F  CF                iretd
		 */
		*(u32 *)p = 0xa00f061e; p+=4;
		*(u32 *)p = 0x1660a80f; p+=4;
		*(u16 *)p = 0x681f; p+= 2;
		*(u32 *)p = i; p += 4;
		*(u16 *)p = 0x15ff; p+= 2;
		*(u32 *)p = int_gate_addr; p += 4;
		*p++ = 0x58;
		*(u32 *)p = 0x0fa90f61; p+=4;
		*(u32 *)p = 0x831f07a1; p+=4;
		*p++ = 0xc4;

		switch (i) {
			case 8: /* #DF */
			case 10: /* #TS */
			case 11: /* #NP */
			case 12: /* #SS */
			case 13: /* #GP */
			case 14: /* #PF */
			case 17: /* #AC */
				*p++ = 4;
				break;
			default:
				*p++ = 0;
		}
		*p++ = 0xcf;

		fill_idt_entry(IDT+8*i, 0x8, (u32)q, TYPE_PRESENT | (i < 0x80 ? TYPE_INT_R0 : TYPE_INT_R3) 
				| TYPE_INT_GATE_32);
	}
	asm (	"lidt %0\n"
		:: "m"(IDTD));
}

