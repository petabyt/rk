#include <main.h>
#include "rk3588.h"

#define VCO_MIN_HZ 2250000000
#define VCO_MAX_HZ 205032704
#define FOUT_MIN_HZ 0x37000000
#define FOUT_MAX_HZ0 0x205032704

enum RkPll {
	PPLL,
	HPLL,
	APLL,
	DPLL,
	GPLL,
	CPLL,
	NPLL,
	V0PLL,
	B0PLL,
	B1PLL,
};

struct PllConfig {
	enum RkPll id;
	uintptr_t con0;
	uintptr_t con1;
	uintptr_t con2;
	uintptr_t con6;
	uintptr_t mode;
	uintptr_t mode_b1;
	uintptr_t mode_b2;
};

struct __attribute__((packed)) Cru {
	uint8_t pad[0x300];
	uint32_t clksel[0x1b];
};

#define PLL_MODE_XIN_OSC0_FUNC 0
#define PLL_MODE_NORMAL 1
#define PLL_MODE_DEEPSLOW 2

struct PllConfig get_config(enum RkPll pll) {
	switch (pll) {
	case V0PLL:
		return (struct PllConfig){
			.id = pll,
			.con0 = CRU + 0x160,
			.con1 = CRU + 0x164,
			.con2 = CRU + 0x168,
			.con6 = CRU + 0x178,
			.mode = CRU + 0x280,
			.mode_b1 = 5,
			.mode_b2 = 4,
		};
	case PPLL:
		return (struct PllConfig){
			.id = pll,
			.con0 = CRU + 0x200,
			.con1 = CRU + 0x204,
			.con2 = CRU + 0x208,
			.con6 = CRU + 0x218,
			.mode = 0x0, // No mode register
		};
	default: return (struct PllConfig){};
	}
}

int rk3588_set_pll(struct PllConfig *cfg, int p, int m, int s) {
	// Connect PLL to deep slow clock while it's set up
	if (cfg->mode) {
		rk_clr_set_bits((void *)cfg->mode, cfg->mode_b1, cfg->mode_b2, PLL_MODE_DEEPSLOW);
	}

    // Power down
    // > 'power down mode is enabled and all digital blocks are reset.'
	rk_clr_set_bits((void *)cfg->con1, 13, 13, 1);
	rk_clr_set_bits((void *)cfg->con1, 5, 0, p);
	rk_clr_set_bits((void *)cfg->con1, 8, 6, s);
	rk_clr_set_bits((void *)cfg->con0, 9, 0, m);

	// power up
	rk_clr_set_bits((void *)cfg->con1, 13, 13, 0);

	// Set PLL to normal mode
	if (cfg->mode) {
		rk_clr_set_bits((void *)cfg->mode, cfg->mode_b1, cfg->mode_b2, PLL_MODE_NORMAL);
	}

	// Wait for PLL to lock
	int timeout = 0x1000;
	while (timeout--) {
		if (((volatile uint32_t *)cfg->con6)[0] & (1 << 15)) break;
		udelay(1);
	}
	if (timeout == 0) {
		puts("PLL didn't lock in time");
	}

	return 0;
}

int rk3588_pll_set_freq(enum RkPll pll, uint32_t freq) {
	struct PllConfig cfg = get_config(pll);
	if (freq == 148500000) rk3588_set_pll(&cfg, 2, 0xc6, 1);
	return 0;
}

void rk3588_setup_v0pll(void) {
	rk3588_pll_set_freq(V0PLL, 148500000);
}

void rk3588_reset_hdptx(void) {
	rk_clr_set_bits((void *)(PMU1CRU + 0x0a0c), 15, 15, 1);
	rk_clr_set_bits((void *)(PMU1CRU + 0x0a10), 0, 0, 1);
	rk_clr_set_bits((void *)(PMU1CRU + 0x0a10), 1, 1, 1);
	// bias_en = 1, bgr_en = 1
	rk_clr_set_bits((void *)(HDPTXPHY0_GRF), 6, 5, 3);
	udelay(10);
	// release init_rstn (rk devs didn't do this)
	rk_clr_set_bits((void *)(PMU1CRU + 0xa0c), 15, 15, 1);
	udelay(10);
	// pll_en = 1
	rk_clr_set_bits((void *)(HDPTXPHY0_GRF), 7, 7, 1);
	udelay(10);
	// release cmn_rstn
	rk_clr_set_bits((void *)(PMU1CRU + 0xa10), 0, 0, 1);
}
