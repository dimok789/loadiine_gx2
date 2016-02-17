OUTPUT(loadiine.elf);

/* Tell linker where our application entry is so the garbage collect can work correct */
ENTRY(__entry_menu);

SECTIONS {
	. = 0x00802000;
	.text : {
		*(.kernel_code*);
		*(.text*);
		/* Tell linker to not garbage collect this section as it is not referenced anywhere */
		KEEP(*(.kernel_code*));
	}
	.rodata : {
		*(.rodata*);
	}
	.data : {
		*(.data*);
		
		__sdata_start = .;
		*(.sdata*);
		__sdata_end = .;
		
		__sdata2_start = .;
		*(.sdata2*);
		__sdata2_end = .;
	}
	.bss : {
		__bss_start = .;
		*(.bss*);
		*(.sbss*);
		*(COMMON);
		__bss_end = .;
	}
	__CODE_END = .;
	
	/DISCARD/ : {
		*(*);
	}
}

/******************************************************** FS ********************************************************/
/* coreinit.rpl difference in addresses 0xFE3C00 */
