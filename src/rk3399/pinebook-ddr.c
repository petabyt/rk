#include <string.h>
#include <stdint.h>
#include <main.h>
#include "rk3399.h"

void ddr_shim(void) {
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
}

/* DMC Setup (Dynamic Memory Controller)
An attempt to get a DDR image working for pinebook pro/rk3399

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
	return 0;
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

	reset_timer0();
	setup_cru();

	rk3399_cpu_clock_start();

	volatile struct Cru *cru = (volatile struct Cru *)CRU_BASE;
	volatile struct PmuSGrfRegs *sgrf = (volatile struct PmuSGrfRegs *)PMUSGRF_BASE;

	// Kever Yang says this fixes "IC ECO bug"
	sgrf->ddr_rgn_con[16] = 0xc000c000;

	// 48mhz
	clock_set_pll(&cru->dpll_con[0], 1, 12, 3, 2);

	extern int rk3399_ddr_entry();
	rk3399_ddr_entry();

	// MMU is off, try to write to RAM
	((uint32_t *)0x10000000)[0] = 0x1234567;

	// Red led off - if still on then DDR image has crashed
	gpio_set_dir(0, RK_PIN_A2, 0);
	gpio_set_pin(0, RK_PIN_A2, 0);
}

