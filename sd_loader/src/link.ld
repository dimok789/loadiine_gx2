OUTPUT(sd_loader.elf);

ENTRY(_start);

SECTIONS {
	. = 0x00800000;
	.text : {
		*(.kernel_code*);
		*(.text*);
		/* Tell linker to not garbage collect this section as it is not referenced anywhere */
		KEEP(*(.kernel_code*));
	}
	.data : {
		*(.rodata*);
		*(.data*);
	}
	/DISCARD/ : {
		*(*);
	}
}

ASSERT((SIZEOF(.text) + SIZEOF(.data)) < 0x1000, "Memory overlapping with main elf.");
