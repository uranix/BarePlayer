#ifndef __PAGING_H__
#define __PAGING_H__

#include "types.h"
#include "mm.h"

#define PAGE_4KB			0x00
#define PAGE_4MB			0x80
#define PAGE_ACCESSED 		0x20
#define PAGE_CACHE_DISABLED	0x10
#define PAGE_WRITE_THROUGH 	0x08
#define PAGE_SUPERVISOR		0x00
#define PAGE_USER 			0x04
#define PAGE_RW				0x02
#define PAGE_PRESENT		0x01

extern u32 *PD;

static inline void remap_page(u32 page, u32 virt, u8 flags) {
	u32 dir = (virt >> 22) & 0x3ff;
	u32 table = (virt >> 12) & 0x3ff;
	
	u32 *PT = (u32 *)(PD[dir] & 0xfffff000);
	asm ("invlpg %0":: "m"(virt));
	PT[table] = (page << 12) | (flags & 0x1f);
}

static inline void map_page(u32 page, u32 virt, u8 flags) {
	u32 dir = (virt >> 22) & 0x3ff;
	u32 table = (virt >> 12) & 0x3ff;
	
	if (PD[dir]) {
		u32 *PT = (u32 *)(PD[dir] & 0xfffff000);
		if (PT[table]) {
			asm ("invlpg %0":: "m"(virt));
		}
		PT[table] = (page << 12) | (flags & 0x1f);
	} else {
		u32 *PT = (u32 *)(alloc_page_zeroed() << 12);
		PD[dir] = ((u32)PT) | (PAGE_USER | PAGE_RW | PAGE_PRESENT);
		if (PT[table]) {
			asm ("invlpg %0":: "m"(virt));
		}
		PT[table] = (page << 12) | (flags & 0x1f);
	}
}

void init_mapping();

#endif
