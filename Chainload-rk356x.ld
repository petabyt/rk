ENTRY(_start)

SECTIONS {
	. = 0x00000000;
	. = ALIGN(16);
	.text : {
		_text_start = .;
		*(.text .text.*)
		_text_end = .;
	}
	. = ALIGN(16);
	.rodata : {
		_rodata_start = .;
		*(.rodata .rodata.*)
		_rodata_end = .;
	}
	. = ALIGN(16);
	.data : {
		_data_start = .;
		*(.data .data.*)
		_data_end = .;
	}
	. = ALIGN(4096);
	.bss : {
		_bss_start = .;
		*(.bss .bss.* COMMON)
		_bss_end = .;
	}
	. = ALIGN(16);
	.got : { *(.got .got.*) }
	. = ALIGN(16);
	_end_of_image = .;
	_fit_start = .;
	/DISCARD/ : {
		*(.comment .comment.*)
		*(.note .note.*)
		*(.eh_frame .eh_frame.*)
	}
	ASSERT(_end_of_image < 0x00040000,
		"RK356x chainloader exceeds the first BL31 load address")
}
