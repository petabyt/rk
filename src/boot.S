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

	.extern c_entry
	bl c_entry

	ret // return to bootrom


