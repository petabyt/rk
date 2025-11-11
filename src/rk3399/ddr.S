// Loaded into "INTMEM" as TPL (very early setup)
.global _start
_start:
	.int 0x33334b52 // rk33

	// bootrom has stack and vectors already setup.

	isb
	dsb sy

	// Set our own vectors for debugging.
	adr x0, exception_table
	msr VBAR_EL3, x0

	stp x0, x1, [sp, #-0x10]!
	stp x2, x3, [sp, #-0x10]!
	stp x4, x5, [sp, #-0x10]!
	stp x6, x7, [sp, #-0x10]!
	stp x8, x9, [sp, #-0x10]!
	stp x10, x11, [sp, #-0x10]!
	stp x12, x13, [sp, #-0x10]!
	stp x14, x15, [sp, #-0x10]!
	stp x16, x17, [sp, #-0x10]!
	stp x18, x30, [sp, #-0x10]!
	.extern ddr_entry
	bl ddr_entry
	ldp x18, x30, [sp], #0x10
	ldp x16, x17, [sp], #0x10
	ldp x14, x15, [sp], #0x10
	ldp x12, x13, [sp], #0x10
	ldp x10, x11, [sp], #0x10
	ldp x8, x9, [sp], #0x10
	ldp x6, x7, [sp], #0x10
	ldp x4, x5, [sp], #0x10
	ldp x2, x3, [sp], #0x10
	ldp x0, x1, [sp], #0x10
	mov x0, #0x0
	ret

