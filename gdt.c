#include "gdt.h"

static void fill_gdt_entry(u8 *entry, u32 base, u32 limit, u8 access) 
{
	entry[6] = FLAG_32_BIT;
	if (limit > 0xfffff) {
		limit >>= 12;
		entry[6] |= FLAG_GRAN;
	}
	*(u16 *)entry = (limit & 0xffff); /* 0-1 */
	*(u16 *)&entry[2] = (base & 0xffff); /* 2-3 */
	entry[4] = (base & 0xff0000) >> 16;
	entry[5] = access;
	entry[6] |= (limit & 0xf0000) >> 16;
	entry[7] = (base & 0xff000000) >> 24;
}

static u8 GDT[6*8];
static struct {
	u16 size;
	u32 offset;
} __attribute__((packed)) GDTD = {.size = sizeof(GDT), .offset = (u32)GDT};
struct _TSS_seg{
	u32 TSS[0x1a];
	u8 intmap[32];
	u8 portmap[0x2008];
} __attribute__((packed)) TSS_seg = {.TSS = {0}, .intmap = {0}, .portmap = {0}};
u8 TSS_stack[0x1000] __attribute__((aligned(0x10)));

void setup_gdt() 
{
	fill_gdt_entry(GDT + KERNEL_NU , 0, 0, 0);
	fill_gdt_entry(GDT + KERNEL_CS , 0, 0xffffffff, ACCESS_XR | ACCESS_EX | ACCESS_CODE_OR_DATA | ACCESS_PRIV_R0 | ACCESS_PR );
	fill_gdt_entry(GDT + KERNEL_DS , 0, 0xffffffff, ACCESS_RW | ACCESS_CODE_OR_DATA | ACCESS_PRIV_R0 | ACCESS_PR );
	fill_gdt_entry(GDT + (USER_CS & 0xf8), 0, 0xffffffff, ACCESS_XR | ACCESS_EX | ACCESS_CODE_OR_DATA | ACCESS_PRIV_R3 | ACCESS_PR );
	fill_gdt_entry(GDT + (USER_DS & 0xf8), 0, 0xffffffff, ACCESS_RW | ACCESS_CODE_OR_DATA | ACCESS_PRIV_R3 | ACCESS_PR );
	fill_gdt_entry(GDT + KERNEL_TSS, (u32)&TSS_seg, sizeof(TSS_seg)-1, ACCESS_AC | ACCESS_EX | ACCESS_PR );
	TSS_seg.TSS[1] = ((u32)TSS_stack) + sizeof(TSS_stack);
	TSS_seg.TSS[2] = KERNEL_DS;
	TSS_seg.TSS[25] = ((u32)(&TSS_seg.portmap)-(u32)(&TSS_seg)) << 16;
	asm (	"lgdt %0\n"
			"ljmp %1,$.L_cs\n"
			".L_cs: movw %2, %%ax\n"
			"movw %%ax, %%ds\n"
			"movw %%ax, %%es\n"
			"movw %%ax, %%fs\n"
			"movw %%ax, %%gs\n"
			"movw %%ax, %%ss\n"
			"movw %3, %%ax\n"
			"ltr %%ax\n"
		:: "m"(GDTD), "i"(KERNEL_CS), "i"(KERNEL_DS), "i"(KERNEL_TSS)
		: "%ax");
}
