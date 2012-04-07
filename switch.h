typedef struct _taskstate {
	u32 eax;
	u32 ebx;
	u32 ecx;
	u32 edx;
	u32 esi;
	u32 edi;
	u32 esp;
	u32 ebp;
	u32 eflags;
	u32 cr4;
	u32 ds;
	u32 es;
	u32 fs;
	u32 gs;
	u32 ss;
	u32 ret;
	frame *v86;
} taskstate;

extern taskstate TS;

void storestate(u32 toret); 
void restorestate() __attribute__((noreturn));
