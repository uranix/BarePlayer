#include "paging.h"

u32 *PD = 0;

void init_mapping() {
	// Map page 0 to non present
	// Map first 8MB 1 to 1 
	u32 i; 
	u32 *PT1 = (u32 *)(alloc_page_zeroed() << 12);
	u32 *PT2 = (u32 *)(alloc_page_zeroed() << 12);
	u32 *p = PT1;

	PD = (u32 *)(alloc_page_zeroed() << 12);
	*p++ = PAGE_RW;
	for (i = 1; i < 0x400; i++) {
		*p++ = (i << 12) | (PAGE_RW | PAGE_PRESENT);
	}
	p = PT2;
	for (i = 0x400; i < 0x800; i++) {
		*p++ = (i << 12) | (PAGE_RW | PAGE_PRESENT);
	}
	PD[0x000] = ((u32)PT1) | (PAGE_USER | PAGE_RW | PAGE_PRESENT); 
	PD[0x001] = ((u32)PT2) | (PAGE_USER | PAGE_RW | PAGE_PRESENT);

	asm (	"movl %0, %%cr3\n"
			"movl %%cr0, %%eax\n"
			"or $0x80010000, %%eax\n"
			"movl %%eax, %%cr0\n"
		:
		: "a"(PD)
		); 
}
