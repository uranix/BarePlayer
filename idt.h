#ifndef __IDT_H__
#define __IDT_H__

#include "types.h"

#define TYPE_PRESENT 0x80
#define TYPE_INT_R0	(0 << 5)
#define TYPE_INT_R1	(1 << 5)
#define TYPE_INT_R2	(2 << 5)
#define TYPE_INT_R3	(3 << 5)
#define TYPE_TASK_GATE_32	0x05
#define TYPE_INT_GATE_16	0x06
#define TYPE_TRAP_GATE_16	0x07
#define TYPE_INT_GATE_32	0x0E
#define TYPE_TRAP_GATE_32	0x0F

typedef struct _nearret {
	u32 eip;
	u32 cs;
	u32 eflags;
	u32 kernstack[0];
	u32 _padd[3];
} __attribute__((packed)) nearret;

typedef struct _farret {
	u32 eip;
	u32 cs;
	u32 eflags;
	u32 esp;
	u32 ss;
	u32 _padd;
} __attribute__((packed)) farret;

typedef struct _efarret {
	u32 error;
	u32 eip;
	u32 cs;
	u32 eflags;
	u32 esp;
	u32 ss;
} __attribute__((packed)) efarret;

typedef struct _vm8086 {
	u32 es;
	u32 ds;
	u32 fs;
	u32 gs;
} __attribute__((packed)) vm8086;

typedef struct _frame {
	u32 edi;
	u32 esi;
	u32 ebp;
	u32 unused_esp;
	u32 ebx;
	u32 edx;
	u32 ecx;
	u32 eax;
	u32 gs;
	u32 fs;
	u32 es;
	u32 ds;
	union {
		nearret pm;
		farret noerr;
		efarret werr;
	} u;
	vm8086 vm;
} __attribute__((packed)) frame;

typedef u32 (*handler)(u32, frame *);

void setup_idt();
u32 check_ints();
inline void unmask_irq(u8 irq);
inline void mask_irq(u8 irq);
handler register_handler(u32 intr, handler hdl);

#endif
