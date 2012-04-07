#include "vm8086.h"
#include "output.h"
#include "switch.h"
#include "intrin.h"

u32 vm86_patcher(frame *argv) 
{
	u32 eip = LINEAR(argv->u.werr.cs, argv->u.werr.eip);
	u32 esp = LINEAR(argv->u.werr.ss, argv->u.werr.esp);
	u32 i;
	
	if (*(u8 *)eip == 0xcc) {
		kmemcpy(TS.v86, argv, sizeof(frame));
		dprintf("VM86: return to PM code\n");
		restorestate();
	}
	
	dprintf("VM86: Unhandled VM86 case\n");
	dprintf("VM86: cs:ip = %.4x:%.4x ss:sp = %.4x:%.4x eflags = %x\n", 
			(u16)argv->u.werr.cs, 
			argv->u.werr.eip, 
			(u16)argv->u.werr.ss, 
			argv->u.werr.esp,
			argv->u.werr.eflags);
	dprintf("VM86: cs:ip dump: ");
	for (i=0; i<16; i++)
		dprintf("%.2x ", *(u8 *)(eip + i));
	dprintf("\n");

	return 0;

/*handled:*/
	argv->u.werr.cs = SEG(eip);
	argv->u.werr.eip = OFFS(eip);
	argv->u.werr.ss = SEG(esp);
	argv->u.werr.esp = OFFS(esp);

	return 1;
}
