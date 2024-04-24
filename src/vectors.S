// Typical exception vector table code.

.macro PANIC
// Push every register onto the stack so we can do stuff
stp x0, x1, [sp, #-0x10]!
stp x2, x3, [sp, #-0x10]!
stp x4, x5, [sp, #-0x10]!
stp x6, x7, [sp, #-0x10]!
stp x8, x9, [sp, #-0x10]!
stp x10, x11, [sp, #-0x10]!
stp x12, x13, [sp, #-0x10]!
stp x14, x15, [sp, #-0x10]!
stp x16, x17, [sp, #-0x10]!
stp x18, x30, [sp, #-0x10]!

mov x0, #0x0
adr x9, .
ldr w8, [x9, #0xc]
blr x8
.int exception

ldp x18, x30, [sp], #0x10
ldp x16, x17, [sp], #0x10
ldp x14, x15, [sp], #0x10
ldp x12, x13, [sp], #0x10
ldp x10, x11, [sp], #0x10
ldp x8, x9, [sp], #0x10
ldp x6, x7, [sp], #0x10
ldp x4, x5, [sp], #0x10
ldp x2, x3, [sp], #0x10
ldp x0, x1, [sp], #0x10
eret
.endm

.macro NOTHING
str x0, [sp, #-0x10]!
mrs x0, elr_el3
add x0, x0, #4 // increment eret addr
msr elr_el3, x0
ldr x0, [sp], #0x10
eret // return back like nothing happened
.endm

// The exception handler for the synchronous exception from the current EL using SP0.
.balign 0x800
.global exception_table
exception_table:
curr_el_sp0_sync:
PANIC

// The exception handler for the IRQ exception from the current EL using SP0.
.balign 0x80
curr_el_sp0_irq:
PANIC

// The exception handler for the FIQ exception from the current EL using SP0.
.balign 0x80
curr_el_sp0_fiq:
PANIC

// The exception handler for the system error exception from the current EL using SP0.
.balign 0x80
curr_el_sp0_serror:
PANIC

// The exception handler for the synchronous exception from the current EL using the current SP.
.balign 0x80
curr_el_spx_sync:
PANIC

.balign 0x80
curr_el_spx_irq: // The exception handler for IRQ exception from the current EL using the current SP.
PANIC

.balign 0x80
curr_el_spx_fiq: // The exception handler for the FIQ exception from the current EL using the current SP.
PANIC

.balign 0x80
curr_el_spx_serror: // The exception handler for the system error exception from the current EL using the current SP.
PANIC

// ----- 0x400 -----

.balign 0x80
lower_el_aarch64_sync: // The exception handler for the synchronous exception from a lower EL (AArch64).
PANIC
.balign 0x80
lower_el_aarch64_irq: // The exception handler for the IRQ exception from a lower EL (AArch64).
PANIC
.balign 0x80
lower_el_aarch64_fiq: // The exception handler for the FIQ exception from a lower EL (AArch64).
PANIC
.balign 0x80
lower_el_aarch64_serror: // The exception handler for the system error exception from a lower EL(AArch64).
PANIC
.balign 0x80
lower_el_aarch32_sync: // The exception handler for the synchronous exception from a lower EL(AArch32).
PANIC
.balign 0x80
lower_el_aarch32_irq: // The exception handler for the IRQ exception from a lower EL (AArch32).
PANIC
.balign 0x80
lower_el_aarch32_fiq: // The exception handler for the FIQ exception from a lower EL (AArch32).
PANIC
.balign 0x80
lower_el_aarch32_serror: // The exception handler for the system error exception from a lower EL(AArch32).
PANIC
