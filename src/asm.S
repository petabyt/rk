.global asm_enable_smp_cache_coherency
asm_enable_smp_cache_coherency:
	// https://developer.arm.com/documentation/ddi0500/e/level-1-memory-system/cache-behavior/data-cache-coherency
	// CPU Extended Control Register, EL1
	// IIRC, this is supported on A53/A52/A72/others, but not A55
	mrs x0, s3_1_c15_c2_1
	orr x0, x0, #0x40
	msr s3_1_c15_c2_1, x0
	ret

.global asm_set_cnt_freq
asm_set_cnt_freq:
	msr CNTFRQ_EL0, x0
	ret

.global asm_dc_civac
asm_dc_civac:
	dc civac, x0
	ret

.global asm_get_cpu_timer
asm_get_cpu_timer:
	mrs x0, CNTFRQ_EL0
	mov x1, #0x4240
	movk x1, #0xf, lsl #16
	udiv x0, x0, x1 // Divide by 1mhz
	mrs x1, CNTVCT_EL0
	udiv x0, x1, x0
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

// setup_tt_el3(tcr, mair, ttbr0);
.global setup_tt_el3
setup_tt_el3:
	msr TCR_EL3, x0
	msr MAIR_EL3, x1
	msr TTBR0_el3, x2
	ret

// enable_mmu(void);
.global enable_mmu_el3
enable_mmu_el3:
	str x0, [sp, #-0x10]!

	// Enable caches and the MMU.
	mrs x0, SCTLR_EL3
	orr x0, x0, #(1 << 2) // The C bit (data cache).
	orr x0, x0, #(1 << 12) // The I bit (instruction cache).
	orr x0, x0, #(1 << 0) // The M bit (MMU).
	and x0, x0, #~(1 << 1) // Disable alignment check
	msr SCTLR_EL3, x0

	dsb sy
	isb

	ldr x0, [sp], 0x10
	ret

// disable_mmu(void);
.global disable_mmu_el3
disable_mmu_el3:
	str x0, [sp, #-0x10]!
	mrs x0, SCTLR_EL3
	and x0, x0, #~(1 << 2) // The C bit (data cache).
	and x0, x0, #~(1 << 12) // The I bit (instruction cache).
	and x0, x0, #~(1 << 0) // The M bit (MMU).
	msr SCTLR_EL3, x0

	dsb sy
	isb

	ldr x0, [sp], 0x10
	ret


// https://developer.arm.com/documentation/102437/0100/Changing-Exception-levels
// http://classweb.ece.umd.edu/enee447.S2021/baremetal_boot_code_for_ARMv8_A_processors.pdf

// The processor can only change execution state on the change of an exception level. We
// are in bare metal, so this means we need we must go to a lower exception level in order
// to switch from arm64 to arm32.
// - Note that on reset, we are always in EL3.
// - In order to get arm32, we need to configure all lower exception levels as 32 bit (required by armv8a)
// - You can only get to a lower EL through an exception (we have to generate one)
.global start_in_el2_32bit
start_in_el2_32bit:
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
	mov x0, #0b1001 // Hyp mode
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

.global start_in_el2
start_in_el2:
	// x0 = entry address
	msr ELR_EL3, x0

	mov x0, sp
	sub x0, x0, #0x500
    msr SP_EL2, x0

	mrs x0, ttbr0_el3
	msr ttbr0_el2, x0

	mrs x0, tcr_el3
	msr tcr_el2, x0

	mrs x0, mair_el3
	msr mair_el2, x0

	msr cptr_el3, xzr
	mov x0, #0x33ff
	msr cptr_el2, x0

	mov x0, xzr
	orr x0, x0, #(1 << 23) // SPAN
	orr x0, x0, #(1 << 22) // EIS
	orr x0, x0, #(1 << 19)
	orr x0, x0, #(1 << 18)
	orr x0, x0, #(1 << 16)
	orr x0, x0, #(1 << 13)
	orr x0, x0, #(1 << 12) // I
	orr x0, x0, #(1 << 11) // EOS
	orr x0, x0, #(1 << 5) // CP15BEN (enable CP15 instructions)
	orr x0, x0, #(1 << 4)
	orr x0, x0, #(1 << 3)
	orr x0, x0, #(1 << 2) // C (data cache)
	orr x0, x0, #(1 << 0) // M (MMU)
	msr SCTLR_EL3, x0

	// Init EL2 system control registers
	mov x0, #0x0830
	movk x0, #0x30C5, lsl #16
	orr x0, x0, #(1 << 0) // M
	orr x0, x0, #(1 << 2) // C
	orr x0, x0, #(1 << 12) // I
	msr sctlr_el2,x0

	mov x0, xzr
	orr x0, x0, #(1 << 1)
	msr hcr_el2, x0

	// Set EL2 execution state from EL3
	mov x0, xzr
	orr x0, x0, #(1 << 10) // We will be in arm64
	orr x0, x0, #(1 << 8) // Enable hypervisor call
	orr x0, x0, #(1 << 9)
	orr x0, x0, #(1 << 5)
	orr x0, x0, #(1 << 4)
	orr x0, x0, #(1 << 3)
	orr x0, x0, #(1 << 0) // EL0 and EL1 in non secure state
	msr SCR_EL3, x0

	// Saved program status register
	mov x0, #0b1001 // Hyp mode
	orr x0, x0, #(1 << 6) // FIQ interrupt mask
	orr x0, x0, #(1 << 7) // IRQ interrupt mask
	orr x0, x0, #(1 << 8) // SError interrupt mask
	orr x0, x0, #(1 << 9) // SError interrupt mask
	msr SPSR_EL3, x0

	dsb sy
	isb

	eret

.global start_in_el1
start_in_el1:
	// x0 = entry address
	msr ELR_EL3, x0

	mov x0, sp
    msr SP_EL1, x0

	mrs x0, ttbr0_el3
	msr ttbr0_el1, x0

	mrs x0, vbar_el3
	msr vbar_el1, x0

	msr cptr_el3, xzr

	mov x0, xzr
	orr x0, x0, #(1 << 31)
	msr hcr_el2, x0

	// Init EL2 system control registers
	mov x0, xzr
	//orr x0, x0, (1 << 5) 
	msr SCTLR_EL1, xzr

	// Set EL2 execution state from EL3
	mov x0, xzr
	orr x0, x0, #(1 << 10) // We will be in arm64
	orr x0, x0, #(1 << 8) // Enable hypervisor call
	orr x0, x0, #(1 << 0) // EL0 and EL1 in non secure state
	msr SCR_EL3, x0

	// Saved program status register
	mov x0, #0b1110 // Hyp mode
	orr x0, x0, #(1 << 6) // FIQ interrupt mask
	orr x0, x0, #(1 << 7) // IRQ interrupt mask
	orr x0, x0, #(1 << 8) // SError interrupt mask
	orr x0, x0, #(1 << 9) // SError interrupt mask
	msr SPSR_EL3, x0

	dsb sy
	isb

	eret

