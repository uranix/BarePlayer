#ifndef __OUTPUT_H__
#define __OUTPUT_H__

#include "multiboot.h"

#define COLUMNS                 80
#define LINES                   24
#define ATTRIBUTE               7
#define VIDEO                   0xB8000

#ifdef _DEBUG
#define dprintf(...) kprintf("%I" __VA_ARGS__)
#else
#define dprintf(...) do { __nullfunc(__VA_ARGS__); } while (0)
static inline void __nullfunc(const char *fmt, ...) {
	(void)fmt;
}
#endif


void cls (void);
void kputchar (u16 c);
void kprintf (const char *format, ...);
void getpos(int *x, int *y);
void setpos(int x, int y);
void disable_cursor();
void show_mbi (multiboot_info_t *mbi);

#endif
