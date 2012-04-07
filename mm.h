#ifndef __MM_H__
#define __MM_H__

#include "types.h"

u32 alloc_page_high();
u32 alloc_page_mapped();
u32 alloc_page_zeroed();
void free_page(u32 idx);
void use_page(u32 idx);

#endif
