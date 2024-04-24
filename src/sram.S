// Loaded into "INTMEM" as TPL (very early setup)
.global _start
_start:
	// rk33
	.int 0x33334b52

	// bootrom has stack and vectors already setup.

	isb
	dsb sy

	// Set our own vectors for debugging.
	adr x0, exception_table
	msr VBAR_EL3, x0

	// Whatever bootrom set up, we'll keep it
	adr x0, reg_stores
	mov x1, x30
	str w1, [x0]
	mov x1, x29
	str w1, [x0, #0x4]
	mov x1, x28
	str w1, [x0, #0x8]
	mov x1, sp
	str w1, [x0, #0xc]

	// dmc.c
	.extern dmc_entry
	bl dmc_entry

	// Restore registers
	mov x1, xzr
	adr x0, reg_stores
	ldr w1, [x0]
	mov x30, x1
	ldr w1, [x0, #0x4]
	mov x29, x1
	ldr w1, [x0, #0x8]
	mov x28, x1
	ldr w1, [x0, #0xc]
	mov sp, x1
	mov x0, #0x0
	ret

.global reg_stores
reg_stores:
.int 0x0
.int 0x0
.int 0x0
.int 0x0

//#include "vectors.S"
