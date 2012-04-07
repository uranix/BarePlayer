#include "multiboot.h"
#include "output.h"
#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "pit.h"
#include "mm.h"
#include "paging.h"
#include "vm8086.h"
#include "switch.h"

#include "intrin.h"

static void register_handlers();

#define LINKER_SET(x) extern u32 _ ## x; u32 x = (u32)&(_ ## x)

LINKER_SET(skernel);
LINKER_SET(ekernel);

static u32 clock = 0;
volatile u32 spin = 1;

LINKER_SET(scode86);
LINKER_SET(ecode86);
LINKER_SET(sdata86);
LINKER_SET(edata86);
LINKER_SET(sstack86);
LINKER_SET(estack86);

void run_8086(u32 scode, u32 ecode, u32 sdata, u32 edata, u32 sstack, u32 estack, frame *ret)
{
	asm ( 
			"pushl %6\n"
			"pushl %5\n"
			"pushl %4\n"
			"pushl %3\n"
			"pushl %2\n"
			"pushl %1\n"
			"pushl %0\n"
			"int   $0x7f\n"
			"addl  $0x1c, %%esp\n"
		::
		"g"(scode),"g"(ecode),
		"g"(sdata),"g"(edata),
		"g"(sstack),"g"(estack),"g"(ret));
}

typedef struct _vbeInfo {
	u8  signature[4];             // == "VESA"
	u16 version;                 // == 0x0300 for VBE 3.0
	u16 oemString[2];            // isa vbeFarPtr
	u8 capabilities[4];
	u16 videomodes[2];           // isa vbeFarPtr
	u16 totalMemory;             // as # of 64KB blocks
} __attribute__((packed)) vbeInfo;

void kmain(unsigned long magic, unsigned long addr)
{
	u32 i;
	frame ret;

	cls();
	disable_cursor();

	if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
	{
		kprintf ("Invalid magic number: 0x%x\n", (unsigned) magic);
		return;
	}
	(void)addr;
	/* show_mbi((multiboot_info_t *) addr); */

	/* mark 1MB as used */
	for (i=0; i < 0x100; i++)
		use_page(i);
	/* mark kernel mem as used */
	for (i=skernel >> 12; i < ekernel >> 12; i++)
		use_page(i);

	kprintf("Kernel mapped to %p - %p (4K page boundary rounded)\n", skernel, ekernel);

	kprintf("Interrupts %sabled\n", check_ints()?"en":"dis");
	setup_gdt();
	kprintf("GDT set\n");
	remap_pic(0x20, 0x28);
	kprintf("PIC remapped\n");
	setup_idt();
	kprintf("IDT set\n");
	set_pit_freq(24);
	kprintf("PIT freq set to 24Hz\n");
	unmask_irq(0);
	unmask_irq(1);
	kprintf("IRQ0-1 unmasked\n");
	register_handlers();
	kprintf("Going to enable interrupts...");
	asm( "sti" );
	kprintf("done\n");
	init_mapping();
	kprintf("Mapping OK\n");
	kprintf("To VM86...\n");
	for (i = 0; i < 0x100; i++)
		remap_page(i, i<<12 , PAGE_RW | PAGE_USER | PAGE_PRESENT);
	run_8086(scode86, ecode86, sdata86, edata86, sstack86, estack86, &ret);
	kprintf("VM ret: eax = %p\n", ret.eax);
	if ((ret.eax & 0xffff) == 0x4f) {
		vbeInfo *vi = (vbeInfo *)sdata86;
		kprintf("Vesa: %c%c%c%c 0x%.4x\n", 
				vi->signature[0], vi->signature[1],
				vi->signature[2], vi->signature[3],
				vi->version	);
	}
}


u32 exception_handler(u32 intr, frame *argv) {
	u32 cr2, i;
	u8 *p;
	/* u32 *args = argv;
	u32 *cs, *ss, *ip, *sp, *eflags, *ds, *es, *fs, *gs,  */
	u32 eip = 0;
	int haserr = 1;
	/* args += 12;  pusha + 4 * push ds */
	switch (intr) {
		case 8: /* #DF */
		case 10: /* #TS */
		case 11: /* #NP */
		case 12: /* #SS */
		case 17: /* #AC */
			/* err = *args++; */
			dprintf("Exception 0x%.2x handler executed. Error: 0x%x\n", intr, argv->u.werr.error);
			break;
		case 13: /* #GP */
			/* err = *args++;
			ip = args++;
			cs = args++;
			eflags = args++;
			sp = args++;
			ss = args++; */

			if (argv->u.werr.eflags & 0x20000)
			{
				/* es = args++;
				ds = args++;
				fs = args++;
				gs = args++; */
				return vm86_patcher(argv);
			}
			dprintf("Unhandled #GP(0x%x) at", argv->u.werr.error);
			break;
		case 14: /* #PF */
			asm ( "movl %%cr2, %0" : "=r"(cr2));
			dprintf("#PF. ", cr2);
			eip = (argv->u.werr.eflags & 0x20000) ? 
				LINEAR(argv->u.werr.cs, argv->u.werr.eip) : 
				argv->u.werr.eip;
			i = argv->u.werr.error;
			dprintf("cannot %s memory %p (%s) in %s code at %p\n", 
					i & 2?"write to":"read", cr2, 
					i & 1? "no access" : "no page",
					i & 4 ?"user":"super", eip);
			dprintf("dump %p :\n\t", eip);
			p = (u8 *)eip;
			for (i=0; i<16; i++)
				dprintf("%.2x ", p[i]);
			dprintf("\n");
		default:
			haserr = 0;
	}
	if (haserr) {
		if (argv->u.werr.eflags & 0x20000)
			dprintf("cs:ip = %.4x:%.4x ss:sp = %.4x:%.4x eflags = %x\n", 
					(u16)argv->u.werr.cs, 
					argv->u.werr.eip, 
					(u16)argv->u.werr.ss, 
					argv->u.werr.esp,
					argv->u.werr.eflags);
		else
			dprintf("[cs]:eip = [%.4x]:%p [ss]:esp = [%.4x]:%p eflags = %x\n", 
					(u16)argv->u.werr.cs, 
					argv->u.werr.eip, 
					(u16)argv->u.werr.ss, 
					argv->u.werr.esp,
					argv->u.werr.eflags);
	} else {
		if (argv->u.noerr.eflags & 0x20000)
			dprintf("cs:ip = %.4x:%.4x ss:sp = %.4x:%.4x eflags = %x\n", 
					(u16)argv->u.noerr.cs, 
					argv->u.noerr.eip, 
					(u16)argv->u.noerr.ss, 
					argv->u.noerr.esp,
					argv->u.noerr.eflags);
		else
			dprintf("[cs]:eip = [%.4x]:%p [ss]:esp = [%.4x]:%p eflags = %x\n", 
					(u16)argv->u.noerr.cs, 
					argv->u.noerr.eip, 
					(u16)argv->u.noerr.ss, 
					argv->u.noerr.esp,
					argv->u.noerr.eflags);
	}
	return 0; /* halt */
}

u32 spawn8086_handler(u32 intr, frame *args) {
	u32 usercode  = 0x10000;
	u32 userstack = 0x80000;
	u32 userdata  = 0x40000;
	u32 scode, ecode, sdata, edata, sstack, estack, ret;
	u32 *kernstack = &(args->u.pm.kernstack[0]);

	u32 i, j, stacksz;
	(void)intr;
	scode  = kernstack[0];
	ecode  = kernstack[1];
	sdata  = kernstack[2];
	edata  = kernstack[3];
	sstack = kernstack[4];
	estack = kernstack[5];
	ret    = kernstack[6];
	for (i = scode >> 12, j = 0; i < ((ecode+0xfff) >> 12); i++, j += 0x1000)
		map_page(i, usercode + j, PAGE_USER | PAGE_PRESENT);
	for (i = sdata >> 12, j = 0; i < ((edata+0xfff) >> 12); i++, j += 0x1000)
		map_page(i, userdata + j, PAGE_USER | PAGE_RW | PAGE_PRESENT);
	for (i = sstack >> 12, j = 0; i < ((estack+0xfff) >> 12); i++, j += 0x1000)
		map_page(i, userstack + j, PAGE_USER | PAGE_RW | PAGE_PRESENT);

	stacksz = estack - sstack;

	asm volatile ( 
			"pushl %0\n"
			"pushl $ret_from_8086\n"
			"call  *%3\n"
			"addl  $8, %%esp\n"
			"pushl $0\n" /* GS */
			"pushl $0\n" /* FS */
			"pushl $0x4000\n" /* DS */
			"pushl $0\n" /* ES */
			"pushl $0x8000\n" /* SS */
			"pushl %1\n" /* SP */
			"pushl $0xa0202\n" /* EFALGS = VIF|VM|IF*/
			"pushl $0x1000\n" /* CS */
			"pushl %2\n" /* IP */
			"movl %%cr4, %%eax\n"
			"or   $1, %%eax\n"
			"movl %%eax, %%cr4\n"
			"iretl\n"
			"ret_from_8086: "
			:: "a"(ret), "b"(stacksz), "d"(scode & 0xfff), "c"(storestate));

	return 0;
}

u32 timer_handler(u32 intr, frame *args) {
	int x,y;
	(void)args;
	(void)intr;
	clock ++;
	if (clock > 50)
		spin = 0;
	getpos(&x, &y);
	setpos(70, 0);
	kprintf("%d", clock);
	setpos(x,y);
	return 0; /* no one checks it */
}

u32 keybd_handler(u32 intr, frame *args) {
	int x,y;
	(void)args;
	(void)intr;
	u8 sc = inb(0x60);
	getpos(&x, &y);
	setpos(0, 24);
	kprintf("[%.2x %c]", (u32)sc & 0x7f, sc & 0x80?'U':'D');
	setpos(x, y);
	return 0; /* no one checks it */
}

static void register_handlers() {
	u32 i;
	for (i=0; i<32; i++)
		register_handler(i, exception_handler);
	register_handler(0x20, timer_handler);
	register_handler(0x21, keybd_handler);
	register_handler(0x7f, spawn8086_handler);
}
