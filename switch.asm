BITS 32
global storestate, restorestate, TS

section .text

storestate: 
mov [TS.eax], eax
mov [TS.ebx], ebx
mov [TS.ecx], ecx
mov [TS.edx], edx
mov [TS.esi], esi
mov [TS.edi], edi
lea eax, [esp+0xc]
mov [TS.esp], eax
mov [TS.ebp], ebp
pushfd
pop eax
mov [TS.eflags], eax
mov eax, cr4
mov [TS.cr4], eax
mov ax, ds
mov [TS.ds], eax
mov ax, es
mov [TS.es], eax
mov ax, fs
mov [TS.fs], eax
mov ax, gs
mov [TS.gs], eax
mov ax, ss
mov [TS.ss], eax
mov eax, [esp+4]
mov [TS.ret], eax
mov eax, [esp+8]
mov [TS.frame], eax
ret

restorestate:
pop eax
mov eax, [TS.ss]
mov ss, ax
mov eax, [TS.gs]
mov gs, ax
mov eax, [TS.fs]
mov fs, ax
mov eax, [TS.es]
mov es, ax
mov eax, [TS.ds]
mov ds, ax
mov eax, [TS.cr4]
mov cr4, eax
mov eax, [TS.eflags]
push eax
popfd
mov ebp, [TS.ebp]
mov esp, [TS.esp]
mov edi, [TS.edi]
mov esi, [TS.esi]
mov edx, [TS.edx]
mov ecx, [TS.ecx]
mov ebx, [TS.ebx]
mov eax, [TS.ret]
push eax
mov eax, [TS.frame]
ret

section .data
TS:
.eax dd 0
.ebx dd 0
.ecx dd 0
.edx dd 0
.esi dd 0
.edi dd 0
.esp dd 0
.ebp dd 0
.eflags dd 0
.cr4 dd 0
.ds  dd 0
.es  dd 0
.fs  dd 0
.gs  dd 0
.ss  dd 0
.ret dd 0
.frame dd 0
