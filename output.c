#include "output.h"
#include "intrin.h"
#include "types.h"

static int xpos;
static int ypos;
static volatile u16 *video;

void cls(void)
{
	int i;

	video = (u16 *) VIDEO;

	for (i = 0; i < COLUMNS * LINES; i++)
		video[i] = 0;

	xpos = 0;
	ypos = 0;
}

static char digit[37] = "0123456789abcdefGHIJKLMNOPQRSTUVWXYZ";

void utoa(char *buf, u32 val, int base, int width, char fill) {
	char out[33], *q, *p, empty = 1;
	q = buf;
	p = out;
	if (width > 32)
		width = 32;
	while (val || empty) {
		empty = 0;
		*p++ = digit[val % base];
		val /= base;
	}
	while (p < (out+width))
		*p++ = fill;
	do {
		*q++ = *--p;
	} while (p > out);
	*q = 0;
}

void itoa(char *buf, int val, int base, int width, char fill) {
	char *p = buf;
	if (val < 0) {
		val = -val;
		*p++ = '-';
	}
	utoa(buf, (u32)val, base, width, fill);
}

void kputchar (u16 s)
{
	char c = (char)s;
	if (c == '\n')
	{
		xpos = 0;
		ypos++;
		goto check2;
	}
	if (c == '\r')
	{
		ypos++;
		goto check2;
	}
	if (c == '\t')
	{
		do {
			video[xpos + ypos * COLUMNS] = (s & 0xff00) | ' ';
			xpos++;
		} while (xpos & 7);
		goto check;
	}

	video[xpos + ypos * COLUMNS] = s; xpos++;

check:	
	if (xpos >= COLUMNS)
		kputchar('\n');
check2:
	if (ypos >= LINES)
		cls();
}

static inline void _kputchar(char c, u8 d) {
	return kputchar((d << 8) | (u8)c);
}

void kprintf (const char *format, ...)
{
	u32 *arg = (u32 *) &format;
	int width;
	char fill;

	char c;
	u8 d = 7;
	char buf[33];

	arg++;

	while ((c = *format++) != 0)
	{
		if (c != '%')
			_kputchar (c, d);
		else
		{
			char *p;

			c = *format++;
			fill = ' ';
			width = 0;
			if (c == '.') {
				fill = '0';
				c = *format++;
			}
			if (c == ' ') {
				fill = ' ';
				c = *format++;
			}
			while ((c >= '0') && (c <= '9')) {
				width = 10*width + (c - '0');
				c = *format++;
			}

			switch (c)
			{
				case 'N':
					d = 0x7;
					break;
				case 'I':
					d = 0x70;
					break;
				case 'd':
					itoa (buf, *((int*)arg++), 10, width, fill);
					p = buf;
					goto string;
				case 'u':
					utoa (buf, *((int *)arg++), 10, width, fill);
					p = buf;
					goto string;
				case 'p':
					p = buf;
					*p++ = '0'; *p++ = 'x';
					utoa (p, *((u32 *)arg++), 0x10, 8, '0');
					p = buf;
					goto string;
				case 'x':
					utoa (buf, *((u32 *)arg++), 0x10, width, fill);
					p = buf;
					goto string;
				case 'o':
					utoa (buf, *((u32 *)arg++), 010, width, fill);
					p = buf;
					goto string;
				case 'b':
					utoa (buf, *((u32 *)arg++), 2, width, fill);
					p = buf;
					goto string;
				case 's':
					p = *((char **)arg++);
					if (! p)
						p = "(null)";

string:
					while (*p)
						_kputchar (*p++, d);
					break;

				case 'c':
					_kputchar (*((char *) arg++), d);
					break;
				default:
					p = "#arg#";
					arg++;
					goto string;
			}
		}
	}
}

void getpos(int *x, int *y) {
	*x = xpos;
	*y = ypos;
}

void setpos(int x, int y) {
	xpos = x;
	ypos = y;
}

void disable_cursor() {
	outw(0x3d4, 0x070e);
	outw(0x3d4, 0xd00f);
}

void show_mbi (multiboot_info_t *mbi)
{
	kprintf ("flags = 0x%x\n", (unsigned) mbi->flags);

	if (mbi->flags & MULTIBOOT_INFO_MEMORY)
		kprintf ("mem_lower = %uKB, mem_upper = %uKB\n",
				(unsigned) mbi->mem_lower, (unsigned) mbi->mem_upper);

	if (mbi->flags & MULTIBOOT_INFO_BOOTDEV)
		kprintf ("boot_device = 0x%x\n", (unsigned) mbi->boot_device);

	if (mbi->flags & MULTIBOOT_INFO_CMDLINE)
		kprintf ("cmdline = %s\n", (char *) mbi->cmdline);

	if (mbi->flags & MULTIBOOT_INFO_MODS)
	{
		multiboot_module_t *mod;
		unsigned i;

		kprintf ("mods_count = %d, mods_addr = 0x%x\n",
				(int) mbi->mods_count, (int) mbi->mods_addr);
		for (i = 0, mod = (multiboot_module_t *) mbi->mods_addr;
				i < mbi->mods_count;
				i++, mod++)
			kprintf (" mod_start = 0x%x, mod_end = 0x%x, cmdline = %s\n",
					(unsigned) mod->mod_start,
					(unsigned) mod->mod_end,
					(char *) mod->cmdline);
	}

	if ((mbi->flags & MULTIBOOT_INFO_AOUT_SYMS) &&
		(mbi->flags & MULTIBOOT_INFO_ELF_SHDR))
	{
		kprintf ("Both bits 4 and 5 are set.\n");
		return;
	}

	if (mbi->flags & MULTIBOOT_INFO_AOUT_SYMS)
	{
		multiboot_aout_symbol_table_t *multiboot_aout_sym = &(mbi->u.aout_sym);

		kprintf ("multiboot_aout_symbol_table: tabsize = 0x%0x, "
				"strsize = 0x%x, addr = 0x%x\n",
				(unsigned) multiboot_aout_sym->tabsize,
				(unsigned) multiboot_aout_sym->strsize,
				(unsigned) multiboot_aout_sym->addr);
	}

	if (mbi->flags & MULTIBOOT_INFO_ELF_SHDR)
	{
		multiboot_elf_section_header_table_t *multiboot_elf_sec = &(mbi->u.elf_sec);

		kprintf ("multiboot_elf_sec: num = %u, size = 0x%x,"
				" addr = 0x%x, shndx = 0x%x\n",
				(unsigned) multiboot_elf_sec->num, (unsigned) multiboot_elf_sec->size,
				(unsigned) multiboot_elf_sec->addr, (unsigned) multiboot_elf_sec->shndx);
	}

	if (mbi->flags & MULTIBOOT_INFO_MEM_MAP)
	{
		multiboot_memory_map_t *mmap;

		kprintf ("mmap_addr = 0x%x, mmap_length = 0x%x\n",
				(unsigned) mbi->mmap_addr, (unsigned) mbi->mmap_length);
		for (mmap = (multiboot_memory_map_t *) mbi->mmap_addr;
				(unsigned long) mmap < mbi->mmap_addr + mbi->mmap_length;
				mmap = (multiboot_memory_map_t *) ((unsigned long) mmap
					+ mmap->size + sizeof (mmap->size)))
			kprintf (" size = 0x%x, base_addr = 0x%x:%x,"
					" length = 0x%x:%x, type = 0x%x\n",
					(unsigned) mmap->size,
					mmap->addr >> 32,
					mmap->addr & 0xffffffff,
					mmap->len >> 32,
					mmap->len & 0xffffffff,
					(unsigned) mmap->type);
	}

	if (mbi->flags & MULTIBOOT_INFO_DRIVE_INFO)
	{
		multiboot_drive_t *drive;
		unsigned i;

		kprintf ("drives_count = %d, drives_addr = 0x%x\n",
				(int) mbi->drives_length, (unsigned) mbi->drives_addr);
		for (i = 0, drive = (multiboot_drive_t *) mbi->drives_addr;
				i < mbi->drives_length;
				i++, drive = NEXT_DRIVE(drive)) {
			kprintf (" drive_number = 0x%x, drive_mode = %s, C:H:S=%d:%d:%d\n",
					(unsigned) drive->drive_number,
					drive->drive_mode ? "LBA" : "CHS",
					(unsigned) drive->drive_cylinders,
					(unsigned) drive->drive_heads,
					(unsigned) drive->drive_sectors);
			kprintf (" ports = ");
			u16 *p = &drive->drive_ports[0];
			for (;*p;p++) 
				kprintf("%xh%s",*p,p[1]?",":"");
			kprintf("\n");
		}
	}

	if (mbi->flags & MULTIBOOT_INFO_CONFIG_TABLE)
	{
		kprintf ("config_table = TODO\n");
	}

	if (mbi->flags & MULTIBOOT_INFO_BOOT_LOADER_NAME)
	{
		kprintf ("boot_loader_name = %s\n", mbi->boot_loader_name);
	}

	if (mbi->flags & MULTIBOOT_INFO_APM_TABLE)
	{
		kprintf ("apm_table = TODO\n");
	}

	if (mbi->flags & MULTIBOOT_INFO_VIDEO_INFO)
	{
		kprintf ("video_info = TODO\n");
	}
}
