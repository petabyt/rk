// Code enters here from bootrom, on core 0.

#include "main.h"

.global _start
_start:
	// Setup exception tables
	adr x0, exception_table
	msr VBAR_EL3, x0

	msr daifclr, #0x7

	// Allow ICH_* register access
	mov x0, #0x1
	msr ICC_SRE_EL3, x0

	// enable timer
	mov x0, #0x1
	msr CNTV_CTL_EL0, x0

	// Allow floating point registers
	msr CPTR_EL3, xzr

	// Setup stack pointer
	mov x0, #STACK_TOP
	mov sp, x0

	// Secure configuration register
	mov x0, xzr
	orr x0, x0, #(1 << 11) // ST
	orr x0, x0, #(1 << 10) // We will be in arm64
	orr x0, x0, #(1 << 8) // Enable hypervisor call
	orr x0, x0, #(1 << 0) // EL0 and EL1 in non secure state
	msr SCR_EL3, x0
	isb

	// Reset sctlr_el3
	mov x1, #0x30C50000
	movk x1, #0x0838
	msr SCTLR_EL3, x1

	.extern c_entry
	bl c_entry

	ret // return to bootrom
