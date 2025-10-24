.global asm_enable_smp_cache_coherency
asm_enable_smp_cache_coherency:
	// https://developer.arm.com/documentation/ddi0500/e/level-1-memory-system/cache-behavior/data-cache-coherency
	// CPU Extended Control Register, EL1
	// IIRC, this is supported on A53/A52/A72/others, but not A55
	mrs x0, s3_1_c15_c2_1
	orr x0, x0, #0x40
	msr s3_1_c15_c2_1, x0
	ret

.global asm_get_mpidr
asm_get_mpidr:
	mrs x0, mpidr_el1
	ret

.global asm_enable_ints
asm_enable_ints:
	msr DAIFClr, #0b11
	ret

.global asm_disable_ints
asm_disable_ints:
	msr DAIFSet, #0b1111
	ret

.global asm_enable_int_groups
asm_enable_int_groups:
	// int group 0 enable
	mov x0, #0x1
	msr ICC_IGRPEN0_EL1, x0
	// Int group 1 enable
	mrs x0, ICC_IGRPEN1_EL1
	orr x0, x0, #0x1
	msr ICC_IGRPEN1_EL1, x0
	// Int group 1 EL3 
	mrs x0, ICC_IGRPEN1_EL3
	orr w0, w0, #0x1
	msr ICC_IGRPEN1_EL3, x0

	ret

.global asm_get_cpu_rev
asm_get_cpu_rev:
	mrs x1,midr_el1
	ubfx x0, x1, #0x10, #0x8
	bfxil x0, x1, #0x0, #0x4
	ret

// Get the current exception level
.global asm_get_el
asm_get_el:
	mrs x0, CurrentEL
	ret

// Get reset management register
.global asm_get_el_es
asm_get_el_es:
	mrs x0, RMR_EL3
	ret

.global test_gen
test_gen:

// https://developer.arm.com/documentation/102437/0100/Changing-Exception-levels
// http://classweb.ece.umd.edu/enee447.S2021/baremetal_boot_code_for_ARMv8_A_processors.pdf

// The processor can only change execution state on the change of an exception level. We
// are in bare metal, so this means we need we must go to a lower exception level in order
// to switch from arm64 to arm32.
// - Note that on reset, we are always in EL3.
// - In order to get arm32, we need to configure all lower exception levels as 32 bit (required by armv8a)
// - You can only get to a lower EL through an exception (we have to generate one)
.global run_32_bit
run_32_bit:
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
	orr x0, x0, (1 << 12) // I (instr cache)
	orr x0, x0, (1 << 23) // SPAN
	orr x0, x0, (1 << 29) // LSMAOE
	orr x0, x0, (1 << 30) // EnIB
	//orr x0, x0, #(1 << 2) // C (data cache)
	//orr x0, x0, #(1 << 0) // M (MMU)
	msr SCTLR_EL2, x0

	// Set EL2 execution state from EL3
	mov x0, xzr
	orr x0, x0, #(1 << 0) // lower level are not in secure state
	orr x0, x0, #(1 << 8) // Enable hypervisor call
	msr SCR_EL3, x0

	// Saved program status register
	mov x0, #0b1010 // Hyp mode
	orr x0, x0, #(1 << 4) // 32bit execution state
	orr x0, x0, #(1 << 6) // FIQ interrupt mask
	orr x0, x0, #(1 << 7) // IRQ interrupt mask
	orr x0, x0, #(1 << 8) // SError interrupt mask
	msr SPSR_EL3, x0

	// Set return addr
	msr ELR_EL3, x4

	dsb sy
	isb

	eret
