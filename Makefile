DEFINES=-D_DEBUG -D__ELF__
CFLAGS=-m32 -Wall -Wextra -Werror -nostdlib -nostartfiles -nodefaultlibs $(DEFINES)
RCFLAGS=-O1 --omit-frame-pointer -m32 -Wall -Wextra -Werror -nostdlib -nostartfiles -nodefaultlibs $(DEFINES)
ASMFLAGS=-x -f elf32 -Ox -Wall
LDFLAGS=-melf_i386 -n

CC=gcc
NASM=/home/uranix/nasm-2.09.10/nasm
CSRC=kernel.c output.c idt.c gdt.c pic.c pit.c mm.c paging.c vm8086.c
SSRC=
ASMSRC=v86.asm boot.asm switch.asm
OBJS=$(CSRC:.c=.o) $(SSRC:.S=.o) $(ASMSRC:.asm=.o)
DEPS=$(CSRC:.c=.dep) $(SSRC:.S=.dep)

.SUFFIXES : .c .s .asm .dep .o 

all: kernel

include Makefile.deps

%.o : %.asm
	$(NASM) $(ASMFLAGS) -o $@ $<

kernel: $(OBJS) linker.ld floppy.img
	$(LD) $(LDFLAGS) -T linker.ld -o kernel $(OBJS)
	cp kernel /mnt/fd/kernel
	sync

clean: 
	rm -f *.o kernel

send: kernel
	/bin/sh -c "cd /home/uranix/os/; ./sync send"

deps: $(DEPS)
	mv Makefile.tmp Makefile.deps

%.dep: %.c
	gcc $(CFALGS) -M $< | sed 's/\.o:\ \w*\.c/.c:/' >> Makefile.tmp
	/bin/echo -e '\ttouch -c $<' >> Makefile.tmp

%.dep: %.S
	gcc $(CFALGS) -M $< | sed 's/\.o:\ \w*\.S/.S:/' >> Makefile.tmp
	/bin/echo -e '\ttouch -c $<'>> Makefile.tmp
