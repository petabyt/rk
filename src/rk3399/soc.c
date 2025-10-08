#include <stdint.h>
#include <os.h>
#include <rk.h>
#include "io.h"

/* Low-level init routines

arm-trusted-firmware is a reference implementation of the SoC boot process, so here's an
attempt to boil everything down

Some very early basic setup:
- Timers
- UART
- GPIO
- Setup translation tables and memory attributes for EL3

Next, SoC Setup:
1. Setup SGRF: Includes most general security setup
2. Trusted RAM setup: security options for RAM
3. SoC soft reset init: Setup PMU regs for rebooting
4. Generic GPIO setup: ...
5. Cortex-M0 Init: setup secure config & PMU regs
6. SDRAM Setup: stride, type, channels, timing
7. SDRAM power management setup: DFS (dynamic frequency scaling) configs controller to
reduce speed under-utilized DRAM channels or devices
DRAM: Enters low-power mode
DRAM: setup training

PMU init:
CPU Core stuff
A lot of ondocumented registers
*/

extern uint64_t asm_get_cpu_rev(int nop, int nop2);

static void sgrf_init() {
	puts("Setting up regs in secure GRF");
	// Security config for master
	((uint32_t *)0xff33c240)[0] = 0x30000;
	((uint32_t *)0xff33c244)[0] = 0xffff0100;
	((uint32_t *)0xff33e014)[0] = 0xffffffff;

	// Security config for slave
	((uint32_t *)0xff33e018)[0] = 0xffffffff;
	((uint32_t *)0xff33e01c)[0] = 0xffffffff;
	((uint32_t *)0xff33e3c0)[0] = 0xffff0000;
	((uint32_t *)0xff33e3c4)[0] = 0xffff0000;
	((uint32_t *)0xff33e3c8)[0] = 0xffff0000;
	((uint32_t *)0xff33e3cc)[0] = 0xffff0000;
	((uint32_t *)0xff33e3d0)[0] = 0xffff2000;

	// DDR region address start (mask)
	((uint32_t *)0xff330000)[0] = 0xfff0000;
	// DDR region address end
	((uint32_t *)0xff330020)[0] = 0xfff0000;
	((uint32_t *)0xff330040)[0] = 0x2000000;
}

static void soft_reset_setup() {
	((uint32_t *)0xff750124)[0] = 0x400000; // presetn_sgrf_hold = 0
	((uint32_t *)0xff760510)[0] = ((uint32_t *)0xff760510)[0] & 0xffffffe3; // Reset CRU_GLB_RST_CON
}

static void setup_m0() {
	puts("Setting up Cortex-M0");

	// PMUSGRF secure registers for m0
	((uint32_t *)0xff33c100)[0] = 0x800000; // PMU_CON0
	((uint32_t *)0xff33e018)[0] = 0x10000000; // SOC_CON0

	// PMUCRU Setup for cm0s (Clock M0 secure?)
	((uint32_t *)0xff750080)[0] = 0x9f008000;
	((uint32_t *)0xff750108)[0] = 0x200000;
	((uint32_t *)0xff750130)[0] = ((uint32_t *)0xff750130)[0] | 2;
}

int sys_soc_setup(void) {
	//debug("CPU Rev: ", asm_get_cpu_rev(0, 0));

	sgrf_init();

	soft_reset_setup();	

	setup_m0();

	debug("CPU ID: ", asm_get_mpidr() & 0xff);

	uint32_t el = (asm_get_el() >> 2) & 3;
	debug("We are in EL", el);

	return 0;
}

/* Saw this in uboot, will probably need it:
	 * Disable DDR and SRAM security regions.
	 *
	 * As we are entered from the BootROM, the region from
	 * 0x0 through 0xfffff (i.e. the first MB of memory) will
	 * be protected. This will cause issues with the DW_MMC
	 * driver, which tries to DMA from/to the stack (likely)
	 * located in this range.
	sgrf = syscon_get_first_range(ROCKCHIP_SYSCON_PMUSGRF);
	rk_clrsetreg(&sgrf->ddr_rgn_con[16], 0x1ff, 0);
	rk_clrreg(&sgrf->slv_secure_con4, 0x2000);

	* eMMC clock generator: disable the clock multipilier 
	grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	rk_clrreg(&grf->emmccore_con[11], 0x0ff);
*/
