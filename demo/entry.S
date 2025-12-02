.extern entry
.extern exception_table
.global _start
_start:
	adr x20, exception_table
	msr vbar_el2, x20
	adr x1, _start
	b entry
	.int 0x0
	.int 0x0
	.int 0x0
	.int 0x0
	.int 0x0
	.int 0x0

	.int 0x08008135 // magic
	.int 0x1 // version
	.int 0x1 // flags: relocate
	.int 200000 // img_size: guess 200kb for now
	.int 0xa00000; .int 0x0 // relocation_addr

.global fw_handler
fw_handler:
	smc #0
	ret
