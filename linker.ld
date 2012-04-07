ENTRY (_start)

PHDRS {
	headers PT_PHDR FILEHDR PHDRS ;
	text PT_LOAD ;
	rodata PT_LOAD ;
	data PT_LOAD ;
	bss PT_LOAD ;
	text86 PT_LOAD ;
	data86 PT_LOAD ;
	stack86 PT_LOAD ;
}

SECTIONS{
	. = 0x00100000;
	_skernel = ALIGN(0x1000);

	.text : {
		*(.ldtext)
		*(.text)
	} :text

	.rodata ALIGN (0x1000) : {
		*(.rodata)
	} :rodata

	.data ALIGN(0x1000) : {
		*(.data)
	} :data

	.bss : {
		sbss = .;
		*(COMMON)
		*(.bss)
		ebss = .;
	} :bss

	.text86 ALIGN(0x1000) : {
		_scode86 = .;
		*(.text86)
		_ecode86 = .;
	} :text86

	.data86 ALIGN(0x1000) : {
		_sdata86 = .;
		*(.data86)
		_edata86 = .;
	} :data86

	.stack86 ALIGN(0x1000) : {
		_sstack86 = .;
		*(.stack86)
		_estack86 = .;
	} :stack86
	
	_ekernel = ALIGN(0x1000);
}
