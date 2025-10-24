.global rk33xx_cpu_entry
rk3399_cpu_entry:
	mrs x0, mpidr_el1
	and x0, x0, #0xffffffff // mask since we only store lower 32 bits of mpidr
	ldr w1, core_to_wake_mpidr

	// Unset deadbeaf from sram+4
	ldr w2, sram_pos
	mov w3, #0x0
	str w3, [x2, #0x4]

	// Put location of this into sram+8
	adr x3, rk3399_cpu_entry
	str w3, [x2, #0x8]

	cmp w0, w1
	b.eq core_start

	wfe
	b rk3399_cpu_entry

	core_start:
	ldr w0, core_start_pos
	blr x0

core_to_wake_mpidr: .int 0x0
core_start_pos: .int 0x0
sram_pos: .int 0xff8c0000
magic: .int 0xdeadbeaf

.global rk33xx_setup_smp
rk3399_setup_smp:
	adr x0, rk3399_cpu_entry
	ldr w1, [sram_pos]
	str w0, [x1, #0x8] // sram+8 = boot_base

// w0 = target CPU mpidr
// w1 = target base address for execution
.global rk3399_wake_up_core
rk3399_wake_up_core:
	adr x4, core_to_wake_mpidr
	str w0, [x4] // set target mpidr
	adr x4, core_start_pos
	str w1, [x4] // set core_start_pos

	ldr w0, magic
	ldr w4, sram_pos
	str w0, [w4, #0x4] // sram+4 = 0xdeadbeaf

	dsb sy
	sev // send event to all cores

	ret
