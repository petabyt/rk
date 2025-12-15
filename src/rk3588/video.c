#include <string.h>
#include <main.h>
#include "rk3588.h"

//static void write32(uintptr_t addr, uint32_t v) {
//	((volatile uint32_t *)addr)[0] = v;
//}

void rk3588_setup_video(void) {
	rk_clr_set_bits((void *)(PMU + 0x8150), 15, 0, 0); // ??
    // CRU_MODE_CON00
    // clk_npll_mode = clk_npll
    // clk_gpll_mode = clk_gpll
    // clk_v0pll_mode = clk_v0pll
    // clk_aupll_mode = clk_aupll
    // clk_cpll_mode = clk_cpll
	rk_clr_set_bits((void *)(CRU + 0x280), 15, 0, 0x155);
	rk_clr_set_bits((void *)(CRU + 0x4c0), 4, 0, 7); // dclk_vp2_src_div = 7
	rk_clr_set_bits((void *)(CRU + 0x4c0), 6, 5, 2); // clksel = clk_v0pll_mux	

	vop2_init(VOP);
}

void rk3588_setup_video_edp1(uintptr_t framebuffer, uint32_t width, uint32_t height) {
	rk3588_setup_video();
	vop2_init(VOP);

	//memset((void *)framebuffer, 123456, width * height * 4);

	rk_clr_set_bits((void *)(VO1_GRF + 4), 0, 0, 1); // enable edp1
	rk_clr_set_bits((void *)(VOP_GRF + 8), 4, 3, 1); // disable hdmi1, enable edp1

	rk3588_setup_v0pll();

	vop2_config_edp1(VOP);
	vop2_setup_display(VOP);
	config_smart_layer(VOP, framebuffer, width, height);
	vop2_start_video(VOP);

	hdptx_phy_init(1);
	hdptx_phy_configure_edp(2, 0xa8c);

	edp_init(EDP1);
	edp_enable(EDP1, 10, 2);
}

void dwhdmi_setup(int hdmi_id);

void rk3588_setup_video_hdmi0(uintptr_t framebuffer, uint32_t width, uint32_t height) {
	rk3588_setup_video();

    // Polarity inversion (hdmitx0_hsync_pol, hdmitx0_vsync_pol)
    rk_clr_set_bits((void *)(VO1_GRF), 6, 5, 3);

	rk_clr_set_bits((void *)(VOP_GRF + 8), 1, 1, 1);

	vop2_config_hdmi(VOP);
	vop2_setup_display(VOP);
	config_smart_layer(VOP, framebuffer, width, height);
	vop2_start_video(VOP);

	dwhdmi_setup(0);
}
