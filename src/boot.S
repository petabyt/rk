// Code enters here from bootrom, on core 0 of Cortex-A53.

#include "os.h"

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
	msr ICC_SRE_EL3, x0

	// Disable trapping of NEON and FP registers
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

//	bl setup_tt_el3
//	bl enable_mmu_el3

	.extern c_entry
	bl c_entry

	ret

// Jump back to the bootrom, basically a reset
.global back_to_bootrom
back_to_bootrom:
	mov x0, xzr
	msr SCTLR_EL3, x0
	mov x0, 0xffff0000
	br x0

.global start_in_el2
start_in_el2:
	// x0 = addr to jump to
	mov x4, x0

	msr cptr_el3, xzr
	mov x0, #0x33ff
	msr cptr_el2, x0

	// Init generic timers
	msr	cntvoff_el2, xzr

	// Init EL2 system control registers
	mov x0, xzr
	orr x0, x0, (1 << 5) // CP15BEN (enable CP15 instructions)
	orr x0, x0, (1 << 6) // nAA (unaligned access do not generate fault)
//	orr x0, x0, (1 << 12) // I (instr cache)
	orr x0, x0, (1 << 23) // SPAN
	orr x0, x0, (1 << 29) // LSMAOE
	orr x0, x0, (1 << 30) // EnIB
	//orr x0, x0, #(1 << 2) // C (data cache)
	//orr x0, x0, #(1 << 0) // M (MMU)
	msr SCTLR_EL2, x0

	// Set EL2 execution state from EL3
	mov x0, xzr
	orr x0, x0, #(1 << 0) // lower level are not in secure state
	orr x0, x0, #(1 << 10) // We will be in arm64
	orr x0, x0, #(1 << 8) // Enable hypervisor call
	msr SCR_EL3, x0

	// Saved program status register
	mov x0, #0b01001 // Hyp mode
	orr x0, x0, #(1 << 6) // FIQ interrupt mask
	orr x0, x0, #(1 << 7) // IRQ interrupt mask
	orr x0, x0, #(1 << 8) // SError interrupt mask
	msr SPSR_EL3, x0

	// Set return addr
	msr ELR_EL3, x4

	mov x0, sp
    msr SP_EL2, x0

	dsb sy
	isb

	eret
