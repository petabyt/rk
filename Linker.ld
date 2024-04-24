ENTRY(_start)

SECTIONS {
	. = 0x00000000;

	. = ALIGN(4);
	.text : {
		_text_start = .;
		*(.text)
		_text_end = .;
	}

	. = ALIGN(4);
	.rodata : {
		_rodata_start = .;
		*(.rodata*)
		_rodata_end = .;
	}

	. = ALIGN(4);
	.bss : {
		_bss_start = .;
		*(.bss*)
		_bss_end = .;
	}

	. = ALIGN(4);
	.got : {
		_got_start = .;
		*(.got*)
		_got_end = .;
	}

	. = ALIGN(4);
	.data : {
		_data_start = .;
		*(.data*)
		. = ALIGN(10);
		_data_end = .;
	}

	. = 0x40000;
	.bl31 : {
		_bl31_start = .;
		*(.bl31)
		_bl31_end = .;
	}

	_end_of_image = .;
}
