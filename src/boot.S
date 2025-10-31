// Code enters here from bootrom, on core 0.

#include "main.h"

.global _start
_start:
	// Setup exception tables
	adr x0, exception_table
	msr VBAR_EL3, x0
	msr VBAR_EL2, x0
	msr VBAR_EL1, x0

	// Secure configuration register
	mov x0, xzr
	orr x0, x0, #(1 << 1) // IRQs will be taken to EL3
	orr x0, x0, #(1 << 2) // FIQs will be taken to EL3
	orr x0, x0, #(1 << 3) // EA
	orr x0, x0, #(1 << 10) // RW
	orr x0, x0, #(1 << 11) // ST
	msr SCR_EL3, x0
	isb

	mov x0, #0x1
	msr ICC_SRE_EL3, x0 // allow interrupts

	mov x0, #0x1
	msr CNTV_CTL_EL0, x0 // enable timer

	// Allow floating point registers
	msr CPTR_EL3, xzr
	msr CPTR_EL2, xzr

	// Init system control registers
	msr HCR_EL2, xzr
	mov x1, #0x30C50000
	movk x1, #0x0838
	msr SCTLR_EL3, x1
	msr SCTLR_EL2, x1

	// Setup stack pointer
	mov x0, #STACK_BASE
	mov sp, x0

	.extern c_entry
	bl c_entry

	ret // return to bootrom
