// Rockchip clock-related drivers
// CRU is Clock and Reset Unit, see page 65 TRM
// PLL: https://en.wikipedia.org/wiki/Phase-locked_loop
#include <string.h>
#include <stdint.h>
#include <main.h>
#include "rk3399.h"

// 24Mhz
#define MHZ_24_OSC 24000000
#define KHZ_24_OSC 24000
#define VCO_MAX_KHZ 3200000
#define OUT_MAX_KHZ 3200000

/*
Page 46 TRM
If DSMPD = 1 (DSM is disabled, "integer mode")
  FOUTVCO = FREF / REFDIV * FBDIV
  FOUTPOSTDIV = FOUTVCO / POSTDIV1 / POSTDIV2
  If DSMPD = 0 (DSM is enabled, "fractional mode")
  FOUTVCO = FREF / REFDIV * (FBDIV + FRAC / 224)
  FOUTPOSTDIV = FOUTVCO / POSTDIV1 / POSTDIV2
Where:
  FOUTVCO = Fractional PLL non-divided output frequency
  FOUTPOSTDIV = Fractional PLL divided output frequency (output of second post divider)
  FREF = Fractional PLL input reference frequency
  REFDIV = Fractional PLL input reference clock divider
  FVCO = Frequency of internal VCO
  FBDIV = Integer value programmed into feedback divide
  FRAC = Fractional value programmed into DSM
*/

// Manually set PLL from CRU clksel regs. We have 8 PLLs in the Pinebook Pro, all controlled by one 24mhz oscillator.
void clock_set_pll(uint32_t *cons, uint32_t refdiv, uint32_t fbdiv, uint32_t postdiv1, uint32_t postdiv2) {
	//debug("Setting clock @ ", (uint64_t)cons);
	// Fractional PLL non-divided output frequency
	uint32_t foutvco = KHZ_24_OSC * fbdiv / refdiv; // TODO: bad

	// Fractional PLL divided output frequency (output of second post divider)
	uint32_t foutpostdiv = foutvco / postdiv1 / postdiv2;

	//debug("Setting PLL @ ", (uint64_t)cons);
	//debug("Output khz = ", foutpostdiv);

	if (foutvco > VCO_MAX_KHZ) {
		debug("foutvco exceeded max khz: ", foutvco);
		abort();
		return;
	}

	if (foutpostdiv > OUT_MAX_KHZ) {
		debug("foutpostdiv exceeded max khz: ", foutpostdiv);
		abort();
		return;
	}

	/*    Page 72 TRM
	cons[0]		11:0	fbdiv

	cons[1]		14:12	postdiv2
	cons[1]		10:8	postdiv1
	cons[1]		5:0		refdiv

	cons[2]		31		pll lock status unlock/lock
	cons[2]		23:0	fracdiv

	cons[3]		9:8		slow/deep/normal mode (1 << 3) (0 << 3)
	cons[3]		6		Power down 4-phase clocks and 2X, 3X, 4X clocks
				5		power down buffered vco clock
				4		Power down all outputs except for buffered VCO clock
				3		PLL saturation behavior enable
				2		Power down quantization noise cancellation DAC
				1		PLL Bypass. FREF bypasses PLL to FOUTPOSTDIV
				0		Global power down
	*/

	// con3 8:9 'pll_work_mode' to 0b00 - slow mode
	rk_clr_set_bits(&cons[3], 9, 8, 0b00);

	// con3 3:3 'dsmpd' to 0b1 - integer mode
	rk_clr_set_bits(&cons[3], 3, 3, 0b1);

	// con0 3:3 - (modulator is disabled, "integer mode")
	rk_clr_set_bits(&cons[0], 11, 0, fbdiv);

	// Set our settings
	rk_clr_set_bits(&cons[1], 14, 0, (postdiv2 << 12) | (postdiv1 << 8) | refdiv);

	// Wait for the PLL to lock
	int waits = 0;
	while (!(cons[2] & (1 << 31))) {
		timer0_usleep(1);
		waits++;
		if (waits > 1000) {
			debug("Waited too long: ", cons[2]);
			abort();
			return;
		}
	}

	// 'pll_work_mode' to 0b01 - back to normal mode
	rk_clr_set_bits(&cons[3], 9, 8, 0b01);	
}

// Configure one of the 8 PLLs in the CRU map
void clock_set_pll_div(uint32_t *cons, uint32_t div_hz, uint32_t refdiv, uint32_t postdiv1, uint32_t postdiv2) {
	clock_set_pll(
		cons,
		refdiv,
		div_hz * refdiv * postdiv1 * postdiv2 / MHZ_24_OSC,
		postdiv1,
		postdiv2
	);
}

void clock_setup_vop(void) {
	struct Cru *cru = (struct Cru *)CRU_BASE;

	rk_clr_set_bits(&cru->clksel_con[48], 4, 0, 0b0); // set divider to zero
	rk_clr_set_bits(&cru->clksel_con[48], 7, 6, 0b01); // Set clock to CPLL

	clock_set_pll(&cru->vpll_con[0], 0x14, 0x2c3, 6, 1);

	rk_clr_set_bits(&cru->clksel_con[50], 15, 0, 0x0); // select VPLL, dclk_vop_divout

	debug("VOP1 Clock status: ", cru->clksel_con[48]); // 0x140
}

void setup_cru(void) {
	// set gpio4c2_sel = gpio
	grf_gpio_iomux_set(IOMUX_4C, 5, 4, 0);
}

void sdmmc_setup_clock(void) {
	// Quick clock setup (copied from bootrom pretty much)
	struct Cru *cru = (struct Cru *)CRU_BASE;
	cru->clksel_con[13] = 0x9F008300;
	cru->clksel_con[16] = 0x077F051F;
//	cru->clksel_con[16] = 0x077F0500;
	cru->sdmmc_con[0] = 0x60004;
}
