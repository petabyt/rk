// --- Boilerplate armv8a code from rockchip --

// Invalidate all caches in the CPU
.global invalidate_cache
invalidate_cache:
	// Disable L1 Caches
	mrs x0, SCTLR_EL3 // Read SCTLR_EL3.
	bic x0, x0, #(1 << 2) // Disable D Cache.
	bic x0, x0, #(1 << 12) // Disable instruction cache (reset value)
	bic x0, x0, #(1 << 0) // Disable MMU (reset value)
	msr SCTLR_EL3, x0 // Write SCTLR_EL3.

	// Invalidate Data cache to make the code general purpose.
	// Calculate the cache size first and loop through each set +
	// way.
	mov x0, #0x0 // x0 = Cache level
	msr CSSELR_EL1, x0 // 0x0 for L1 Dcache 0x2 for L2 Dcache.

	mrs x4, CCSIDR_EL1 // Read Cache Size ID.
	and x1, x4, #0x7
	add x1, x1, #0x4 // x1 = Cache Line Size.
	mov x3, 0x7FFF
	and x2, x3, x4, LSR #13 // x2 = Cache Set Number – 1.
	mov x3, 0x3FF
	and x3, x3, x4, LSR #3 // x3 = Cache Associativity Number – 1.
	clz W4, W3 // x4 = way position in the CISW instruction.
	mov x5, #0 // x5 = way counter way_loop.
	way_loop:
	mov X6, #0 // X6 = set counter set_loop.
	set_loop:
	lsl X7, x5, x4

	orr X7, x0, X7 // Set way.
	lsl X8, X6, x1
	orr X7, X7, X8 // Set set.
	dc cisw, X7 // Clean and Invalidate cache line.
	add X6, X6, #1 // Increment set counter.
	cmp X6, x2 // Last set reached yet?
	ble set_loop // If not, iterate set_loop,
	add x5, x5, #1 // else, next way.
	cmp x5, x3 // Last way reached yet?
	ble way_loop // If not, iterate way_loop.

	ret

// Setup TTBR0
.global setup_tt_el3
setup_tt_el3:
	// Initialize translation table control registers
	mov x1, 0x3520 // 4GB space 4KB granularity
	// Inner-shareable.
	msr TCR_EL3, x1 // Normal Inner and Outer Cacheable.

	mov x1, #0xFF440000
	movk x1, #0x0400
	msr MAIR_EL3, x1 // ATTR2 Normal Non-Cacheable. 

	// Invalidate cache inst
	tlbi alle3

	// ATTR3 Normal Cacheable.
	adr x0, ttb0_base // ttb0_base must be a 4KB-aligned address.
	msr TTBR0_EL3, x0

	ret

// enable_mmu(temp)
.global enable_mmu_el3
enable_mmu_el3:
	// Enable data coherency with other cores
	mrs x0, S3_1_C15_C2_1 // CPU Extended Control Register, EL1
	orr x0, x0, #(0x1 << 6) // The SMP bit.
	msr S3_1_C15_C2_1, x0

	// Enable caches and the MMU.
	mrs x0, SCTLR_EL3
	orr x0, x0, #(1 << 2) // The C bit (data cache).
	orr x0, x0, #(1 << 12) // The I bit (instruction cache).
	orr x0, x0, #(1 << 0) // The M bit (MMU).
	msr SCTLR_EL3, x0

	dsb sy
	isb

	ret

// disable_mmu(temp)
.global disable_mmu_el3
disable_mmu_el3:
	// Enable caches and the MMU.
	mrs x0, SCTLR_EL3
	and x0, x0, #~(1 << 2) // The C bit (data cache).
	and x0, x0, #~(1 << 12) // The I bit (instruction cache).
	and x0, x0, #~(1 << 0) // The M bit (MMU).
	msr SCTLR_EL3, x0

	dsb sy
	isb

	ret

#include "tt.S"
