#ifndef __VM8086_H__
#define __VM8086_H__

#include "types.h"
#include "idt.h"

#define FLAGS_CONST 0x0002
#define FLAGS_CF 0x0001
#define FLAGS_PF 0x0004
#define FLAGS_AF 0x0010
#define FLAGS_ZF 0x0040
#define FLAGS_SF 0x0080
#define FLAGS_TF 0x0100
#define FLAGS_IF 0x0200
#define FLAGS_DF 0x0400
#define FLAGS_OF 0x0800
#define FLAGS_IOPL_LO 0x1000
#define FLAGS_IOPL_HI 0x2000
#define FLAGS_IOPL_0 0
#define FLAGS_IOPL_1 (FLAGS_IOPL_LO)
#define FLAGS_IOPL_2 (FLAGS_IOPL_HI)
#define FLAGS_IOPL_3 (FLAGS_IOPL_LO | FLAGS_IOPL_HI)
#define FLAGS_NT 0x4000
#define EFLAGS_RF  0x0010000
#define EFLAGS_VM  0x0020000
#define EFLAGS_AC  0x0040000
#define EFLAGS_VIF 0x0080000
#define EFLAGS_VIP 0x0100000
#define EFLAGS_ID  0x0200000

static inline u32 LINEAR(u32 seg, u32 offs) {
	return (seg << 4) + offs;
}

static inline u16 SEG(u32 lin) {
	return (lin & 0xf0000) >> 4;
}

static inline u16 OFFS(u32 lin) {
	return lin & 0xffff;
}

static inline void PUSHW(u32 *esp, u16 v) {
	*esp -= 2;
	*(u16 *)(*esp) = v;
}

static inline void PUSHD(u32 *esp, u32 v) {
	*esp -= 4;
	*(u32 *)(*esp) = v;
}

static inline u16 POPW(u32 *esp) {
	u16 ret = *(u16 *)(*esp);
	*esp += 2;
	return ret;
}

static inline u32 POPD(u32 *esp) {
	u32 ret = *(u32 *)(*esp);
	*esp += 4;
	return ret;
}

u32 vm86_patcher(frame *argv);

#endif
