BITS 16

section .text86 exec align=1
mov ax, 0x4f00
push ds
pop es
mov di, vi
int 0x10
int3

section .data86 write align=4
vi times 0x1000 db 0

section .stack86 nobits align=4
resb 0x2000
