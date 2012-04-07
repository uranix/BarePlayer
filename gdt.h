#ifndef __GDT_H__
#define __GDT_H__

#include "types.h"

#define FLAG_GRAN	0x80
#define FLAG_32_BIT 0x40
#define FLAG_AVL	0x10

#define ACCESS_AC	0x01
#define ACCESS_RW	0x02
/* just an alias */
#define ACCESS_XR	0x02
#define ACCESS_DC	0x04
#define ACCESS_EX	0x08
#define ACCESS_CODE_OR_DATA 0x10
#define ACCESS_PRIV_R0 (0 << 5)
#define ACCESS_PRIV_R1 (1 << 5)
#define ACCESS_PRIV_R2 (2 << 5)
#define ACCESS_PRIV_R3 (3 << 5)
#define ACCESS_PR	0x80

#define KERNEL_NU 0x00
#define KERNEL_CS 0x08
#define KERNEL_DS 0x10
#define USER_CS (0x18 | 0x03)
#define USER_DS (0x20 | 0x03)
#define KERNEL_TSS	0x28

extern u32 TSS[0x1a];
void setup_gdt();

#endif
