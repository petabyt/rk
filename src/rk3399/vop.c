#include <stdint.h>
#include <rk.h>
#include <os.h>
#include "rk3399.h"

#define STANDBY_MODE (1 << 22)

#define OUT_MIPI (1 << 15)
#define OUT_EDP (1 << 14)
#define OUT_HDMI (1 << 13)
#define OUT_RGB (1 << 12)

int rk3399_init_vop(uintptr_t vop_addr, uint32_t fb_addr) {
	volatile struct Vop *vop = (volatile struct Vop *)vop_addr;

	clock_setup_vop();

	// Exit standby mode
	vop->sys_ctrl &= ~STANDBY_MODE;

	// Disable other interfaces
	vop->sys_ctrl &= ~(OUT_MIPI | OUT_HDMI | OUT_RGB);

	// Enable VOP eDP interface
	vop->sys_ctrl |= OUT_EDP;

	{
		// Set VOP mode (precomputed)
		// set porch stuff, vsync, hsync, etc 
		vop->dsp_ctrl0 = 0;
		vop->dsp_htotal_hs_end = 0x85E0020;
		vop->dsp_hact_st_end = 0xAE082E;
		vop->dsp_vtotal_vs_end = 0x44C0006;
		vop->dsp_vact_st_end = 0x110449;
		vop->post_dsp_hact_info = 0xAE082E;
		vop->post_dsp_vact_info = 0x110449;

		vop->reg_cfg_done = 0x1;
	}

	{
		vop->win0_act_info = (1080 - 1) << 16 | (1920 - 1);
		vop->win0_dsp_st = 0x1100AE;
		vop->win0_dsp_info = (1080 - 1) << 16 | (1920 - 1);
		vop->win0_color_key = 0;

		vop->win0_vir = 1920; // Stride

		vop->win0_ctrl0 = (vop->win0_ctrl0 & ~0xEF)
			| 1 // win0_en
			| 0b000 << 1 // ARGB888
			| 0b100 << 5 // win0_lb_mode
			;

		vop->win0_yrgb_mst = fb_addr;

		vop->reg_cfg_done = 0x1;
	}

	return 0;
}
