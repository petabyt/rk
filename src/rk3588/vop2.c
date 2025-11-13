#include <main.h>
#include "rk3588.h"

struct __attribute__((packed)) Vop2SysCtrl {
	uint32_t reg_cfg_done;
	uint32_t version_info;
	uint32_t auto_gating_ctrl_imd;
	uint32_t win_reg_cfg_done;
	uint32_t axi_ctrl0_imd;
	uint32_t axi_hurry_ctrl0_imd;
	uint32_t axi_hurry_ctrl1_imd;
	uint32_t axi_outstanding_ctrl0_imd;
	char pad1[0x4];
	uint32_t axi_lut_ctrl_imd;
	uint32_t dsp_inface_en;
	uint32_t dsp_inface_ctrl;
	uint32_t dsp_inface_pol;
	uint32_t power_ctrl;
	uint32_t var_freq_ctrl_imd;
	uint32_t mmu_raddr_range;
	uint32_t wb_ctrl0;
	uint32_t wb_xspd_factor;
	uint32_t wb_yrgb_mst;
	uint32_t wb_cbr_mst;
	uint32_t otp_win_en_imd;
	uint32_t otp_mirr_ctrl_imd;
	uint32_t lut_port_sel;
	uint32_t power_stable_ctrl;
	uint32_t status0;
	uint32_t status1;
	uint32_t status2;
	uint32_t status3;
	uint32_t line_flag0;
	uint32_t line_flag1;
	uint32_t line_flag2;
	uint32_t line_flag3;
	uint32_t intr_en0;
	uint32_t intr_clr0;
	uint32_t intr_status0;
	uint32_t intr_raw_status0;
	uint32_t intr_en1;
	uint32_t intr_clr_sy1;
	uint32_t intr_status1;
	uint32_t intr_raw_status1;
	uint32_t port0_intr_en;
	uint32_t port0_intr_clr;
	uint32_t port0_intr_status;
	uint32_t port0_intr_raw_status;
	uint32_t port1_intr_en;
	uint32_t port1_intr_clr;
	uint32_t port1_intr_status;
	uint32_t port1_intr_raw_status;
	uint32_t port2_intr_en;
	uint32_t port2_intr_clr;
	uint32_t port2_intr_status;
	uint32_t port2_intr_raw_status;
	uint32_t port3_intr_en;
	uint32_t port3_intr_clr;
	uint32_t port3_intr_status;
	uint32_t port3_intr_raw_status;
	uint32_t afbcd_intr_en0;
	uint32_t afbcd_intr_clr0;
	uint32_t afbcd_intr_status0;
	uint32_t afbcd_intr_raw_status0;
	uint32_t afbcd_intr_en1;
	uint32_t afbcd_intr_clr1;
	uint32_t afbcd_intr_status1;
	uint32_t afbcd_intr_raw_status1;
	uint32_t sec_drm_ctrl;
	uint32_t sec_drm_layer_sel;
	uint32_t sec_drm_port_mux;
	uint32_t sec_drm_inface_mux;
	uint32_t sec_axi_rid_prot;
};

struct __attribute__((packed)) Vop2Cluster {
	uint32_t win0_ctrl0;
	uint32_t win0_ctrl1;
	uint32_t win0_ctrl2;
	char pad1[0x4];
	uint32_t win0_yrgb_mst;
	uint32_t win0_bcr_mst;
	uint32_t win0_vir;
	char pad2[0x4];
	uint32_t win0_act_info;
	uint32_t win0_dsp_info;
	uint32_t win0_dsp_st;
	uint32_t win0_dsp_bg;
	uint32_t win0_scl_factor_yrgb;
	char pad3[0x4];
	uint32_t win0_scl_offset;
	uint32_t win0_transformed_offset;
	char pad4[0x10];
	uint32_t win0_afbcd_output_ctrl;
	uint32_t win0_afbcd_mode;
	uint32_t win0_afbcd_hdr_ptr;
	uint32_t win0_afbcd_vir_width;
	uint32_t win0_afbcd_size;
	uint32_t win0_afbcd_pic_offset;
	uint32_t win0_afbcd_display_offset;
	uint32_t win0_afbcd_ctrl;
	char pad5[0x10];
	uint32_t win1_ctrl0;
	uint32_t win1_ctrl1;
	uint32_t win1_ctrl2;
	char pad6[0x4];
	uint32_t win1_yrgb_mst;
	uint32_t win1_bcr_mst;
	uint32_t win1_vir;
	char pad7[0x4];
	uint32_t win1_act_info;
	uint32_t win1_dsp_info;
	uint32_t win1_dsp_st;
	uint32_t win1_dsp_bg;
	uint32_t win1_scl_factor_yrgb;
	char pad8[0x4];
	uint32_t win1_scl_offset;
	uint32_t win1_transformed_offset;
	char pad9[0x10];
	uint32_t win1_afbcd_output_ctrl;
	uint32_t win1_afbcd_mode;
	uint32_t win1_afbcd_hdr_ptr;
	uint32_t win1_afbcd_vir_width;
	uint32_t win1_afbcd_size;
	uint32_t win1_afbcd_pic_offset;
	uint32_t win1_afbcd_dis_offset;
	uint32_t win1_afbcd_ctrl;
	char pad10[0x10];
	uint32_t ctrl;
	char pad11[0xc];
	uint32_t lg_coe0;
	uint32_t lg_coe1;
	uint32_t lg_coe2;
	char pad12[0x4];
	uint32_t hg_coe0;
	uint32_t hg_coe1;
	uint32_t hg_coe2;
};

struct __attribute__((packed)) Vop2Overlay {
	uint32_t ctrl;
	uint32_t layer_sel;
	uint32_t port_sel;
	uint32_t reserve_lut_mst;
	uint32_t cluster_0_mix_src_color_ctrl;
	uint32_t cluster_0_mix_dst_color_ctrl;
	uint32_t cluster_0_mix_src_alpha_ctrl;
	uint32_t cluster_0_mix_dst_alpha_ctrl;
	uint32_t cluster_1_mix_src_color_ctrl;
	uint32_t cluster_1_mix_dst_color_ctrl;
	uint32_t cluster_1_mix_src_alpha_ctrl;
	uint32_t cluster_1_mix_dst_alpha_ctrl;
	uint32_t cluster_2_mix_src_color_ctrl;
	char pad1[0x4];
	uint32_t cluster_2_mix_src_alpha_ctrl;
	char pad2[0x4];
	uint32_t cluster_3_mix_src_color_ctrl;
	uint32_t cluster_3_mix_dst_color_ctrl;
	uint32_t cluster_3_mix_src_alpha_ctrl;
	char pad3[0x4];
	uint32_t mix0_src_color_ctrl;
	uint32_t mix0_dst_color_ctrl;
	uint32_t mix0_src_alpha_ctrl;
	uint32_t mix0_dst_alpha_ctrl;
	uint32_t mix1_src_color_ctrl;
	uint32_t mix1_dst_color_ctrl;
	uint32_t mix1_src_alpha_ctrl;
	uint32_t mix1_dst_alpha_ctrl;
	uint32_t mix2_src_color_ctrl;
	uint32_t mix2_dst_color_ctrl;
	uint32_t mix2_src_alpha_ctrl;
	uint32_t mix2_dst_alpha_ctrl;
	uint32_t mix3_src_color_ctrl;
	uint32_t mix3_dst_color_ctrl;
	uint32_t mix3_src_alpha_ctrl;
	uint32_t mix3_dst_alpha_ctrl;
	uint32_t mix4_src_color_ctrl;
	uint32_t mix4_dst_color_ctrl;
	uint32_t mix4_src_alpha_ctrl;
	uint32_t mix4_dst_alpha_ctrl;
	uint32_t mix5_src_color_ctrl;
	uint32_t mix5_dst_color_ctrl;
	uint32_t mix5_src_alpha_ctrl;
	uint32_t mix5_dst_alpha_ctrl;
	uint32_t mix6_src_color_ctrl;
	uint32_t mix6_dst_color_ctrl;
	uint32_t mix6_src_alpha_ctrl;
	uint32_t mix6_dst_alpha_ctrl;
	uint32_t hdro_mix_src_color_ctrl;
	uint32_t hdro_mix_dst_color_ctrl;
	uint32_t hdro_mix_src_alpha_ctrl;
	uint32_t hdro_mix_dst_alpha_ctrl;
	uint32_t hdr1_mix_src_color_ctrl;
	uint32_t hdr1_mix_dst_color_ctrl;
	uint32_t hdr1_mix_src_alpha_ctrl;
	uint32_t hdr1_mix_dst_alpha_ctrl;
	uint32_t dp0_bg_mix_ctrl;
	uint32_t dp1_bg_mix_ctrl;
	uint32_t dp2_bg_mix_ctrl;
	uint32_t dp3_bg_mix_ctrl;
	uint32_t cluster_dly_num0;
	uint32_t cluster_dly_num1;
	uint32_t smart_dly_num;
};

struct __attribute__((packed)) Vop2ESmart {
	uint32_t ctrl0;
	uint32_t ctrl1;
	uint32_t axi_ctrl;
	char pad1[0x4];
	uint32_t region0_mst_ctl;
	uint32_t region0_mst_yrgb;
	uint32_t region0_mst_cbcr;
	uint32_t region0_vir;
	uint32_t region0_act_info;
	uint32_t region0_dsp_info;
	uint32_t region0_dsp_offset;
	char pad2[0x4];
	uint32_t region0_scl_ctrl;
	uint32_t region0_scl_factor_yrgb;
	uint32_t region0_scl_factor_cbcr;
	uint32_t region0_scl_offset;
	uint32_t region1_mst_ctl;
	uint32_t region1_mst_yrgb;
	uint32_t region1_mst_cbcr;
	uint32_t region1_vir;
	uint32_t region1_act_info;
	uint32_t region1_dsp_info;
	uint32_t region1_dsp_offset;
	char pad3[0x4];
	uint32_t region1_scl_ctrl;
	uint32_t region1_scl_factor_yrgb;
	uint32_t region1_scl_factor_cbcr;
	uint32_t region1_scl_offset;
	uint32_t region2_mst_ctl;
	uint32_t region2_mst_yrgb;
	uint32_t region2_mst_cbcr;
	uint32_t region2_vir;
	uint32_t region2_act_info;
	uint32_t region2_dsp_info;
	uint32_t region2_dsp_offset;
	char pad4[0x4];
	uint32_t region2_scl_ctrl;
	uint32_t region2_scl_factor_yrgb;
	uint32_t region2_scl_factor_cbcr;
	uint32_t region2_scl_offset;
	uint32_t region3_mst_ctl;
	uint32_t region3_mst_yrgb;
	uint32_t region3_mst_cbcr;
	uint32_t region3_vir;
	uint32_t region3_act_info;
	uint32_t region3_dsp_info;
	uint32_t region3_dsp_offset;
	char pad5[0x4];
	uint32_t region3_scl_ctrl;
	uint32_t region3_scl_factor_yrgb;
	uint32_t region3_scl_factor_cbcr;
	uint32_t region3_scl_offset;
	uint32_t key_ctrl;
	uint32_t bg_en;
};

struct __attribute__((packed)) Vop2Post {
	uint32_t dsp_ctrl;
	uint32_t dual_display_ctrl;
	uint32_t color_ct_rl;
	uint32_t clk_ctrl;
	uint32_t _3d_lut_ctrl;
	uint32_t _3d_lut_r;
	uint32_t _3d_lut_g;
	uint32_t _3d_lut_b;
	uint32_t _3d_lut_ms_t;
	char pad1[0x8];
	uint32_t dsp_bg;
	uint32_t pre_scan_htiming0;
	uint32_t pre_scan_timing1;
	uint32_t pre_scan_timing2;
	uint32_t pre_scan_timing3;
	uint32_t scl_ctrl;
	uint32_t dsp_vact_info_f1;
	uint32_t dsp_htotal_hs_end;
	uint32_t dsp_hact_st_end;
	uint32_t dsp_vtotal_vs_end;
	uint32_t dsp_vact_st_end;
	uint32_t dsp_vs_st_end_f1;
	uint32_t dsp_vact_st_end_f1;
	uint32_t bcsh_ctrl;
	uint32_t bcsh_bcs;
	uint32_t bcsh_h_color_bar;
	uint32_t bcsh_color_bar;
	char pad2[0x30];
	uint32_t frc_lowe_r01_0;
	uint32_t frc_lowe_r01_1;
	uint32_t frc_lowe_r10_0;
	uint32_t frc_lowe_r10_1;
	uint32_t frc_lowe_r11_0;
	uint32_t frc_lowe_r11_1;
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
	usleep(1000);
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
