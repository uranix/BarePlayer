#include "mm.h"
#include "intrin.h"

#define PAGES_LOW 2048
#define PAGES_TOTAL (1024*1024)

u8 page_allocated[(PAGES_TOTAL >> 3)] = {0};

u32 alloc_page_high() {
	u32 i;
	u8 *p = page_allocated;
	for (i=(PAGES_LOW >> 3); i< (PAGES_TOTAL >> 3); i++, p++) {
		if (*p != 0xff) {
			u8 v = *p;
			int o = 1, s = 0;
			for (; v & o; o <<= 1, s++);
			*p |= o;
			return i << 3 | s;
		}
	}
	return 0xffffffff;
}

/* alloc page from first 8MB, which is mapped 1 to 1*/
u32 alloc_page_mapped() {
	u32 i;
	u8 *p = page_allocated;
	for (i=0; i< (PAGES_LOW >> 3); i++, p++) {
		if (*p != 0xff) {
			u8 v = *p;
			int o = 1, s = 0;
			for (; v & o; o <<= 1, s++);
			*p |= o;
			return i << 3 | s;
		}
	}
	return 0xffffffff;
}

/* alloc mapped & zero */
u32 alloc_page_zeroed() {
	u32 page = alloc_page_mapped();
	if (page != 0xffffffff) {
		u8 *p = (u8 *)(page << 12);
		zero_page(p);
	}
	return page;
}

void free_page(u32 idx) {
	u32 i = idx >> 3;
	u32 s = idx & 7;
	
	page_allocated[i] &= ~(1 << s); /* TODO: check double-free*/
}

void use_page(u32 idx) {
	u32 i = idx >> 3;
	u32 s = idx & 7;
	
	page_allocated[i] |= (1 << s); 
}
