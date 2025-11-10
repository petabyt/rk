.global dcache_clean
dcache_clean:
dsb sy
	mrs	x3, ctr_el0
	ubfx	x3, x3, #16, #4
	mov	x2, #4
	lsl	x2, x2, x3		/* cache line size */

	/* x2 <- minimal cache line size in cache system */
	sub	x3, x2, #1
	bic	x0, x0, x3
1:	dc	civac, x0	/* clean & invalidate data or unified cache */
	add	x0, x0, x2
	cmp	x0, x1
	b.lo	1b
	dsb	sy
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
