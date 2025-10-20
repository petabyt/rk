#include <string.h>
#include <stdint.h>
#include <rk.h>
#include <os.h>
#include "rk3399.h"

/* DMC Setup (Dynamic Memory Controller)

Very early into the setup process, we need to setup the Pinebook's memory controller. Since the memory isn't
setup by the BOOTROM, this code is loaded into the chip's 200kb embedded SRAM.

Ok, so the DMC setup. The RK3399 supports a lot of different memory types (DDR3/DDR3L/LPDDR3/LPDDR4),
but on the Pinebook Pro, we only need to support one (LPDDR4). And we know we will always have 4GB, and it
needs to run at 800mhz.

GRF DDR registers:

Name             Offset    W/R   Reset value  Description
GRF_DDRC0_CON0   0x0e380   (W)   0x00001f81   "ddrc0 control register 0"
GRF_DDRC0_CON1   0x0e384   (W)   0x00000000   "ddrc0 control register 1"
GRF_DDRC1_CON0   0x0e388   (W)   0x00001f81   "ddrc1 control register 0"
GRF_DDRC1_CON1   0x0e38c   (W)   0x00000000   "ddrc1 control register 1"

Lots of IO is accessed here:
PMU
GRF
PMUGRF
PMUSGRF (see also soc.c)
CIC (FF620000)

*/

// RK3399 address mapping
#define CCI500 0xFFB00000
#define SERVICE_NOC3 0xFFA90000
#define SERVICE_NOC2 0xFFA8C000
#define DDRC1 0xFFA88000
#define SERVICE_NOC1 0xFFA84000
#define DDRC0 0xFFA80000
#define SERVICE_NOC0 0xFFA50000

struct MSCHRegs {
	uint32_t coreid;
	uint32_t revisionid;
	uint32_t ddrconf;
	uint32_t ddrsize;
};

int cpu_clock_setup(struct Cru *cru) {

	clock_set_pll(&cru->apll_l_con[0], 0x1, 50, 2, 1); // Cluster L

	cru->clksel_con[0] = 0x1fdf0100;
	cru->clksel_con[1] = 0x1f1f0501;

	clock_set_pll(&cru->apll_b_con[0], 0x1, 50, 2, 1); // Cluster B

	cru->clksel_con[2] = 0x1fdf0140;
	cru->clksel_con[3] = 0x1f1f0501;

	clock_set_pll(&cru->gpll_con[0], 0x2, 99, 2, 1);
	clock_set_pll(&cru->cpll_con[0], 0x1, 64, 2, 2);

	cru->clksel_con[14] = 0x739f3083;
	cru->clksel_con[23] = 0x739f1085;
	cru->clksel_con[25] = 0x79f0085;

	return 0;
}

static int clock_start(void) {
	extern int rk3399_ddr_entry();
	struct Cru *cru = (struct Cru *)CRU_BASE;
	volatile struct PmuSGrfRegs *sgrf = (volatile struct PmuSGrfRegs *)PMUSGRF_BASE;

	cpu_clock_setup(cru);

	{
		// Kever Yang says this fixes "IC ECO bug"
		sgrf->ddr_rgn_con[16] = 0xc000c000;

		// 48mhz
		clock_set_pll(&cru->dpll_con[0], 1, 12, 3, 2);
	}

	return 0;
}

int dram_set_clock(int hz) {
	struct Cru *cru = (struct Cru *)CRU_BASE;

	// Step up clock freq
	if (hz == 400000000) {
		clock_set_pll(&cru->dpll_con[0], 1, 50, 3, 1);
	} else if (hz == 800000000) {
		clock_set_pll(&cru->dpll_con[0], 1, 100, 3, 1);		
	} else {
		puts("Unsupported dram freq");
	}
}

static void rk_stimer(void) {
	__asm__("dmb sy");
	((volatile uint32_t *)0xff8680a0)[0] = 0xffffffff;
	__asm__("dmb sy");
	((volatile uint32_t *)0xff8680a4)[0] = 0xffffffff;
	__asm__("dmb sy");
	((volatile uint32_t *)0xff8680b0)[0] = 0;
	__asm__("dmb sy");
	((volatile uint32_t *)0xff8680b4)[0] = 0;
	__asm__("dmb sy");
	((volatile uint32_t *)0xff8680bc)[0] = 1;
}

void ddr_entry(uint64_t lr) {
	// Red led on
	gpio_set_dir(0, RK_PIN_A2, 1);
	gpio_set_pin(0, RK_PIN_A2, 1);

//	rk_clr_set_bits((uint32_t *)(GRF_BASE + GRF_SOC_CON25), 11, 11, 1);
//	rk_clr_set_bits((uint32_t *)(GRF_BASE + GRF_SOC_CON20), 5, 5, 1);

	// USB5v
	gpio_set_dir(1, RK_PIN_B5, 1);
	gpio_set_pin(1, RK_PIN_B5, 1);

	// OTG/USB3 5v
	gpio_set_dir(4, RK_PIN_D2, 1);
	gpio_set_pin(4, RK_PIN_D2, 1);

	// LCDVCC
	gpio_set_dir(1, RK_PIN_C6, 1);
	gpio_set_pin(1, RK_PIN_C6, 1);

	// LCD_EN
	gpio_set_dir(1, RK_PIN_A0, 1);
	gpio_set_pin(1, RK_PIN_A0, 1);

	// LCD_BL_PWM
	gpio_set_dir(4, RK_PIN_C2, 1);
	gpio_set_pin(4, RK_PIN_C2, 1);

	rk_stimer();
	reset_timer0();
	setup_cru();

	clock_start();

	extern int rk3399_ddr_entry();
	rk3399_ddr_entry();

	// MMU is off, try to write to RAM
	((uint32_t *)0x10000000)[0] = 0x1234567;

	// Red led off
	gpio_set_dir(0, RK_PIN_A2, 0);
	gpio_set_pin(0, RK_PIN_A2, 0);
}
