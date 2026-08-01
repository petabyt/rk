/* SPDX-License-Identifier: Apache-2.0 */
.global chain_jump_bl31
.type chain_jump_bl31, %function
chain_jump_bl31:
	/* x0 = BL31 entry, x1 = TF-A v1 parameter block. */
	mov x9, x0
	mov x10, x1
	msr DAIFSet, #0xf
	dsb sy
	ic iallu
	dsb sy
	isb
	mrs x11, SCTLR_EL3
	bic x11, x11, #(1 << 0)
	bic x11, x11, #(1 << 2)
	bic x11, x11, #(1 << 12)
	msr SCTLR_EL3, x11
	dsb sy
	isb
	mov x0, x10
	mov x1, xzr
	br x9
	.size chain_jump_bl31, . - chain_jump_bl31
