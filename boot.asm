BITS 32

global _start
extern kprintf
extern kmain

MAGIC equ 0x1BADB002
FLAGS equ 0x00000003

section .ldtext
_start:
jmp	entry

align 4, db 0
dd MAGIC
dd FLAGS
dd -(MAGIC+FLAGS)

entry:
	mov esp, stackend

	push 0
	popf

	push ebx
	push eax
	call kmain

	push halted
	call kprintf
	add esp, 0x0c
	
	cli
	hlt

halted db "%IHalted.", 0x0a, 0

section .bss nobits align=4
	stack resb 0x2000
	stackend equ $
