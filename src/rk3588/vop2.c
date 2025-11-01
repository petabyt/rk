#include <main.h>
#include "rk3588.h"

struct LayerConfig {
	uint32_t width;
	uint32_t height;
	uint64_t framebuffer;
};

struct TimingConfig {
	int dclk_freq;
	int dclk_core;
	int dclk_out;
};

enum Layer {
	Cluster0,
	Cluster1,
	Esmart0,
	Esmart1,
	Cluster2,
	Cluster3,
	Esmart2,
	Esmart3,
};

struct TimingConfig init_clock(void) {
	return (struct TimingConfig){
		.dclk_freq = 0,
		.dclk_core = 2,
		.dclk_out = 0,
	};
}

static volatile struct Vop2Overlay *get_overlay(uintptr_t base) {
	return (volatile struct Vop2Overlay *)(base + 0x600);
}

static volatile struct Vop2ESmart *get_esmart(uintptr_t base, int i) {
	return (volatile struct Vop2ESmart *)((base + 0x1800) + (i * 0x200));
}

static volatile struct Vop2Post *get_post(uintptr_t base, int i) {
	return (volatile struct Vop2Post *)((base + 0xc00) + (i * 0x100));
}

static void set_layer_port(uintptr_t base, int layer, int port) {
	volatile struct Vop2Overlay *overlay = get_overlay(base);
	int bs = 0;
	overlay->layer_sel |= (port << bs);
}

void config_smart_layer(uintptr_t base, uintptr_t framebuffer_addr, uint32_t width, uint32_t height) {
	// TODO:
	volatile struct Vop2ESmart *smart = get_esmart(base, 2);
	// Disable mst
	smart->region0_mst_ctl = 0;
	smart->region0_scl_factor_yrgb = 0;
	// region0_yrgb_yscl_mode = 1
	// region0_yrgb_xscl_mode = 1
	smart->region0_scl_ctrl = 0x44;
	// esmart_yrgb_rid = 0xc
	// esmart_cbcr_rid = 0xd
	smart->ctrl1 = 0xd0c0;
	smart->region0_vir = width;
	// region0_act_heigth = vertical size - 1, region0_act_width = horizontal size - 1
	smart->region0_act_info = ((height - 1) << 0x10) | (width - 1);
	// Set the same for dsp info.
	smart->region0_dsp_info = ((height - 1) << 0x10) | (width - 1);
	// no offset.
	smart->region0_dsp_offset = 0x0;
	smart->region0_mst_yrgb = framebuffer_addr;
	// esmart_frm_resetn_en = Disable
	// esmart_csc_mode = BT709_L
	smart->ctrl0 = 4;
	// Enable mst
	// region0_data_fmt = ARGB8888
	smart->region0_mst_ctl = 1;
}

#define RGB888 0
#define YUV422 3
#define YUV420 0xe
#define RGB101010 0xf
void config_post(uintptr_t base) {
	volatile struct Vop2Post *post = get_post(base, 2);

	// Dclk_core = dclk / 4
	// Dclk_out = dclk
	post->clk_ctrl = 2;
	// Display standby, choose output format
	post->dsp_ctrl = RGB101010 | (1 << 0x1f);
	post->dsp_ctrl = 0x0;
	post->dsp_ctrl = RGB101010;
	post->dsp_htotal_hs_end = 20 | (2200 << 0x10);
	post->dsp_hact_st_end = 2152 | (232 << 0x10);
	post->dsp_vact_st_end = 1122 | (42 << 0x10);
	post->dsp_vtotal_vs_end = 10 | (1125 << 0x10);
	post->dual_display_ctrl = 0x0;
	post->dsp_bg = 0x0;
	post->scl_ctrl = 0x0;
	// Magic numbers
	post->pre_scan_htiming0 = 0x3f30020;
	post->pre_scan_timing1 = 0xe80868;
	post->pre_scan_timing2 = 0x2a0462;
	post->pre_scan_timing3 = 0x10001000;
}

void vop2_config_hdmi(uintptr_t base) {
	volatile struct Vop2SysCtrl *sys = (volatile struct Vop2SysCtrl *)base;
	// hdmiedp0_mode = Enable & HDMI mode
	sys->dsp_inface_en |= (2 << 2);
	// hdmiedp0_port_sel = Video output 2
	sys->dsp_inface_en |= (2 << 0x10);
	// hdmiedp0_dclk_sel = dclk N / 4
	sys->dsp_inface_ctrl |= (2 << 0x10);
	// hdmiedp0_pix_clk_sel = dclk N / 2
	sys->dsp_inface_ctrl |= (1 << 0x12);
}

void vop2_config_edp0(uintptr_t base) {
	volatile struct Vop2SysCtrl *sys = (volatile struct Vop2SysCtrl *)base;
	// hdmiedp0_mode = Enable & eDP mode
	sys->dsp_inface_en |= (1 << 2);
	sys->dsp_inface_en |= (1 << 0);
	// hdmiedp0_port_sel = Video output 2
	sys->dsp_inface_en |= (1 << 0xe);
}

void vop2_config_edp1(uintptr_t base) {
	volatile struct Vop2SysCtrl *sys = (volatile struct Vop2SysCtrl *)base;
	// TODO: de-magicify
	sys->dsp_inface_en = 0x100a0018;
	sys->dsp_inface_ctrl = 0x60000;
}

void vop2_init(uintptr_t base) {
	volatile struct Vop2SysCtrl *sys = (volatile struct Vop2SysCtrl *)base;
	volatile struct Vop2Overlay *overlay = get_overlay(base);
	// Power on
	sys->power_ctrl = 0x0;
	udelay(1000);
	// Interface on
	overlay->ctrl |= (1 << 0x1c);
	sys->dsp_inface_pol |= (1 << 0x1c);

	// TODO: Unmagicify numbers
	// set_layer_port(base, 0, 0);
	// set_layer_port(base, 1, 1);
	// set_layer_port(base, 2, 2);
	// set_layer_port(base, 3, 3);
	// set_layer_port(base, 4, 4);
	// set_layer_port(base, 5, 5);
	// set_layer_port(base, 6, 6);
	// set_layer_port(base, 7, 7);
	overlay->layer_sel = 0x76543210;
	overlay->port_sel = 0xa5a47738;

	overlay->dp2_bg_mix_ctrl = 0x34000000;
    // All clocks are already enabled on reset...
}

void vop2_setup_display(uintptr_t base) {
	volatile struct Vop2SysCtrl *sys = (volatile struct Vop2SysCtrl *)base;
	config_post(base);
	// line flag mode of video output2
	sys->line_flag2 = 0x462; // 0~ DSP_VTOTAL-1
}

void vop2_start_video(uintptr_t base) {
	volatile struct Vop2SysCtrl *sys = (volatile struct Vop2SysCtrl *)base;
	volatile struct Vop2Post *post = get_post(base, 2);
	post->dsp_ctrl &= ~(1 << 0x1f);		
	// Set DONE bits
	sys->reg_cfg_done = 0x48004;

	puts("VOP video is active");
}
