// Typical exception vector table code.

panic_func:
	stp xzr, x30, [x29, #-0x10]!
	stp x27, x28, [x29, #-0x10]!
	stp x25, x26, [x29, #-0x10]!
	stp x23, x24, [x29, #-0x10]!
	stp x21, x22, [x29, #-0x10]!
	stp x19, x20, [x29, #-0x10]!
	stp x17, x18, [x29, #-0x10]!
	stp x15, x16, [x29, #-0x10]!
	stp x13, x14, [x29, #-0x10]!
	stp x11, x12, [x29, #-0x10]!
	stp x9, x10, [x29, #-0x10]!
	stp x7, x8, [x29, #-0x10]!
	stp x5, x6, [x29, #-0x10]!
	stp x3, x4, [x29, #-0x10]!
	stp x1, x2, [x29, #-0x10]!
	stp xzr, x0, [x29, #-0x10]!
	bl panic_handler
	ldp xzr, x0, [x29], #0x10
	ldp x1, x2, [x29], #0x10
	ldp x3, x4, [x29], #0x10
	ldp x5, x6, [x29], #0x10
	ldp x7, x8, [x29], #0x10
	ldp x9, x10, [x29], #0x10
	ldp x11, x12, [x29], #0x10
	ldp x13, x14, [x29], #0x10
	ldp x15, x16, [x29], #0x10
	ldp x17, x18, [x29], #0x10
	ldp x19, x20, [x29], #0x10
	ldp x21, x22, [x29], #0x10
	ldp x23, x24, [x29], #0x10
	ldp x25, x26, [x29], #0x10
	ldp x27, x28, [x29], #0x10
	ldp xzr, x30, [x29], #0x10
	eret

int_handler_func:
	stp xzr, x30, [x29, #-0x10]!
	stp x27, x28, [x29, #-0x10]!
	stp x25, x26, [x29, #-0x10]!
	stp x23, x24, [x29, #-0x10]!
	stp x21, x22, [x29, #-0x10]!
	stp x19, x20, [x29, #-0x10]!
	stp x17, x18, [x29, #-0x10]!
	stp x15, x16, [x29, #-0x10]!
	stp x13, x14, [x29, #-0x10]!
	stp x11, x12, [x29, #-0x10]!
	stp x9, x10, [x29, #-0x10]!
	stp x7, x8, [x29, #-0x10]!
	stp x5, x6, [x29, #-0x10]!
	stp x3, x4, [x29, #-0x10]!
	stp x1, x2, [x29, #-0x10]!
	stp xzr, x0, [x29, #-0x10]!
	bl panic_handler
	ldp xzr, x0, [x29], #0x10
	ldp x1, x2, [x29], #0x10
	ldp x3, x4, [x29], #0x10
	ldp x5, x6, [x29], #0x10
	ldp x7, x8, [x29], #0x10
	ldp x9, x10, [x29], #0x10
	ldp x11, x12, [x29], #0x10
	ldp x13, x14, [x29], #0x10
	ldp x15, x16, [x29], #0x10
	ldp x17, x18, [x29], #0x10
	ldp x19, x20, [x29], #0x10
	ldp x21, x22, [x29], #0x10
	ldp x23, x24, [x29], #0x10
	ldp x25, x26, [x29], #0x10
	ldp x27, x28, [x29], #0x10
	ldp xzr, x30, [x29], #0x10
	eret

nothing_func:
	eret

.balign 0x800
.global exception_table
exception_table:

// The exception handler for the synchronous exception from the current EL using SP0.
curr_el_sp0_sync:
b panic_func

// The exception handler for the IRQ exception from the current EL using SP0.
.balign 0x80
curr_el_sp0_irq:
b panic_func

// The exception handler for the FIQ exception from the current EL using SP0.
.balign 0x80
curr_el_sp0_fiq:
b panic_func

// The exception handler for the system error exception from the current EL using SP0.
.balign 0x80
curr_el_sp0_serror:
b panic_func

// The exception handler for the synchronous exception from the current EL using the current SP.
.balign 0x80
curr_el_spx_sync:
b panic_func

.balign 0x80
curr_el_spx_irq: // The exception handler for IRQ exception from the current EL using the current SP.
b int_handler_func

.balign 0x80
curr_el_spx_fiq: // The exception handler for the FIQ exception from the current EL using the current SP.
b int_handler_func

.balign 0x80
curr_el_spx_serror: // The exception handler for the system error exception from the current EL using the current SP.
b panic_func

// ----- 0x400 -----

.balign 0x80
lower_el_aarch64_sync: // The exception handler for the synchronous exception from a lower EL (AArch64).
b panic_func
.balign 0x80
lower_el_aarch64_irq: // The exception handler for the IRQ exception from a lower EL (AArch64).
b panic_func
.balign 0x80
lower_el_aarch64_fiq: // The exception handler for the FIQ exception from a lower EL (AArch64).
b panic_func
.balign 0x80
lower_el_aarch64_serror: // The exception handler for the system error exception from a lower EL(AArch64).
b panic_func
.balign 0x80
lower_el_aarch32_sync: // The exception handler for the synchronous exception from a lower EL(AArch32).
b panic_func
.balign 0x80
lower_el_aarch32_irq: // The exception handler for the IRQ exception from a lower EL (AArch32).
b panic_func
.balign 0x80
lower_el_aarch32_fiq: // The exception handler for the FIQ exception from a lower EL (AArch32).
b panic_func
.balign 0x80
lower_el_aarch32_serror: // The exception handler for the system error exception from a lower EL(AArch32).
b panic_func
