#include <string.h>
#include "main.h"
#include "rk356x.h"

#define BIT(x) (1U << (x))

static inline volatile uint32_t *cru(unsigned int offset) {
	return (volatile uint32_t *)(RK356X_CRU + offset);
}

static inline volatile uint32_t *vop(unsigned int offset) {
	return (volatile uint32_t *)(RK356X_VOP2 + offset);
}

static int set_vpll(uint32_t rate_khz) {
	volatile uint32_t *pll = cru(40 * 4);
	uint32_t post1 = 0, post2 = 0;
	uint64_t vco = 0, best_delta = ~(uint64_t)0;

	/* Keep the VCO close to 1.2 GHz, as in Rockchip's 74.25/297 MHz rows. */
	for (uint32_t p1 = 1; p1 <= 7; p1++) {
		for (uint32_t p2 = 1; p2 <= p1; p2++) {
			uint64_t candidate = (uint64_t)rate_khz * p1 * p2;
			uint64_t delta;
			if (candidate < 800000 || candidate > 3200000)
				continue;
			delta = candidate > 1200000 ? candidate - 1200000 :
				1200000 - candidate;
			if (delta < best_delta) {
				best_delta = delta;
				post1 = p1;
				post2 = p2;
				vco = candidate;
			}
		}
	}
	if (!post1)
		return -1;

	uint32_t fbdiv = (uint32_t)(vco / 24000);
	uint32_t frac = (uint32_t)((((vco % 24000) << 24) + 12000) / 24000);
	if (frac == 0x1000000) {
		fbdiv++;
		frac = 0;
	}
	if (fbdiv < 16 || fbdiv > 0xfff)
		return -1;

	/* RK3328-style VPLL: mode bit 12, RK3036-compatible CON0..CON2. */
	rk_clr_set_bits(cru(0xc0), 12, 12, 0);
	rk_clr_set_bits(&pll[0], 14, 0, (post1 << 12) | fbdiv);
	rk_clr_set_bits(&pll[1], 5, 0, 1);
	rk_clr_set_bits(&pll[1], 8, 6, post2);
	rk_clr_set_bits(&pll[1], 12, 12, frac ? 0 : 1);
	pll[2] = (pll[2] & ~0xffffffU) | frac;
	for (unsigned int timeout = 0; timeout < 24000; timeout++) {
		if (pll[1] & BIT(10)) {
			rk_clr_set_bits(cru(0xc0), 12, 12, 1);
			return 0;
		}
	}
	rk_clr_set_bits(cru(0xc0), 12, 12, 1);
	return -1;
}

static void release_reset(unsigned int id) {
	rk_clr_set_bits(cru(0x400 + (id / 16) * 4), id % 16, id % 16, 0);
}

int rk356x_display_clocks(uint32_t pixel_clock_khz) {
	if (rk356x_enable_vo_domain())
		return -1;
	/* VO roots, VOP0, HDMI host and HDMI SFR clocks. */
	rk_clr_set_bits(cru(0x300 + 20 * 4), 2, 0, 0);
	rk_clr_set_bits(cru(0x300 + 20 * 4), 6, 6, 0);
	rk_clr_set_bits(cru(0x300 + 20 * 4), 10, 8, 0);
	rk_clr_set_bits(cru(0x300 + 21 * 4), 4, 3, 0);
	for (unsigned int id = 256; id <= 265; id++)
		release_reset(id);
	release_reset(270);
	release_reset(271);
	if (set_vpll(pixel_clock_khz))
		return -1;
	/* ACLK_VOP_PRE = GPLL / 4; DCLK_VOP0 = VPLL / 1. */
	rk_clr_set_bits(cru(0x100 + 38 * 4), 7, 0, (1 << 6) | 3);
	rk_clr_set_bits(cru(0x100 + 39 * 4), 11, 0, (1 << 10));
	return 0;
}

void rk356x_vop2_setup(const struct VideoMode *mode, uint32_t stride) {
	uint32_t htotal = mode->hactive + mode->hfront_porch + mode->hsync_len +
		mode->hback_porch;
	uint32_t vtotal = mode->vactive + mode->vfront_porch + mode->vsync_len +
		mode->vback_porch;
	uint32_t hact_st = htotal - (mode->hactive + mode->hfront_porch);
	uint32_t vact_st = vtotal - (mode->vactive + mode->vfront_porch);
	uint32_t polarity = 0;

	if (mode->flags & VIDEO_FLAG_HSYNC_HIGH) polarity |= BIT(4);
	if (mode->flags & VIDEO_FLAG_VSYNC_HIGH) polarity |= BIT(5);

	*vop(0x008) = 0;
	*vop(0x028) = BIT(1);             /* HDMI from VP0. */
	*vop(0x030) = polarity | BIT(28); /* Immediate interface update. */
	*vop(0x600) = 0;
	*vop(0x604) = 0x76543102;         /* Layer 0 is eSmart0. */
	*vop(0x608) = 0x00000880;         /* One layer on VP0; VP1/2 disabled. */
	*vop(0x6e0) = 42U << 24;
	*vop(0x6f8) = 20;

	*vop(0xc00 + 0x00) = 0;          /* P888, progressive, leave standby. */
	*vop(0xc00 + 0x2c) = 0;
	*vop(0xc00 + 0x30) = ((42 + (mode->hactive >> 1) - 1) << 16) |
		mode->hsync_len;
	*vop(0xc00 + 0x34) = (hact_st << 16) | (hact_st + mode->hactive);
	*vop(0xc00 + 0x38) = (vact_st << 16) | (vact_st + mode->vactive);
	*vop(0xc00 + 0x3c) = 0x10001000;
	*vop(0xc00 + 0x40) = 0;
	*vop(0xc00 + 0x48) = (htotal << 16) | mode->hsync_len;
	*vop(0xc00 + 0x4c) = (hact_st << 16) | (hact_st + mode->hactive);
	*vop(0xc00 + 0x50) = (vtotal << 16) | mode->vsync_len;
	*vop(0xc00 + 0x54) = (vact_st << 16) | (vact_st + mode->vactive);

	*vop(0x1800 + 0x10) = 1;         /* ARGB/XRGB8888, region enabled. */
	*vop(0x1800 + 0x14) = RK356X_FB_START;
	*vop(0x1800 + 0x1c) = stride / 4;
	*vop(0x1800 + 0x20) = ((mode->vactive - 1) << 16) |
		(mode->hactive - 1);
	*vop(0x1800 + 0x24) = ((mode->vactive - 1) << 16) |
		(mode->hactive - 1);
	*vop(0x1800 + 0x28) = 0;
	*vop(0x000) = BIT(15) | BIT(0);
}
