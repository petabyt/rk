#include <stdint.h>
#include "main.h"

#define DPCD_SINK_CONTROL_FIELD		0x600
struct AnalogixEdp {
	char	res0[0x10];
	uint32_t	dp_tx_version;
	char	res1[0x4];
	uint32_t	func_en_1;
	uint32_t	func_en_2;
	uint32_t	video_ctl_1;
	uint32_t	video_ctl_2;
	uint32_t	video_ctl_3;
	uint32_t	video_ctl_4;
	char	res2[0xc];
	uint32_t	video_ctl_8;
	char	res3[0x4];
	uint32_t	video_ctl_10;
	uint32_t	total_line_l;
	uint32_t	total_line_h;
	uint32_t	active_line_l;
	uint32_t	active_line_h;
	uint32_t	v_f_porch; // vertical front porch
	uint32_t	vsync;
	uint32_t	v_b_porch;
	uint32_t	total_pixel_l;
	uint32_t	total_pixel_h;
	uint32_t	active_pixel_l;
	uint32_t	active_pixel_h;
	uint32_t	h_f_porch_l;
	uint32_t	h_f_porch_h;
	uint32_t	hsync_l;
	uint32_t	hysnc_h;
	uint32_t	h_b_porch_l;
	uint32_t	h_b_porch_h;
	uint32_t	vid_status;
	uint32_t	total_line_sta_l;
	uint32_t	total_line_sta_h;
	uint32_t	active_line_sta_l;
	uint32_t	active_line_sta_h;
	uint32_t	v_f_porch_sta;
	uint32_t	vsync_sta;
	uint32_t	v_b_porch_sta;
	uint32_t	total_pixel_sta_l;
	uint32_t	total_pixel_sta_h;
	uint32_t	active_pixel_sta_l;
	uint32_t	active_pixel_sta_h;
	uint32_t	h_f_porch_sta_l;
	uint32_t	h_f_porch_sta_h;
	uint32_t	hsync_sta_l;
	uint32_t	hsync_sta_h;
	uint32_t	h_b_porch_sta_l;
	uint32_t	h_b_porch__sta_h;
	char      res4[0x28];
	uint32_t	pll_reg_1;
	char	res5[4];
	uint32_t	ssc_reg;
	char	res6[0xc];
	uint32_t	tx_common;
	uint32_t	tx_common2;
	char	res7[0x4];
	uint32_t	dp_aux;
	uint32_t	dp_bias;
	uint32_t	dp_test;
	uint32_t	dp_pd;
	uint32_t	dp_reserv1;
	uint32_t	dp_reserv2;
	char	res8[0x224];
	uint32_t	lane_map;
	char	res9[0x14];
	uint32_t	analog_ctl_2;
	char	res10[0x48];
	uint32_t	int_state;
	uint32_t	common_int_sta_1;
	uint32_t	common_int_sta_2;
	uint32_t	common_int_sta_3;
	uint32_t	common_int_sta_4;
	uint32_t	spdif_biphase_int_sta;
	char	res11[0x4];
	uint32_t	dp_int_sta;
	uint32_t	common_int_mask_1;
	uint32_t	common_int_mask_2;
	uint32_t	common_int_mask_3;
	uint32_t	common_int_mask_4;
	char	res12[0x08];
	uint32_t	int_sta_mask;
	uint32_t	int_ctl;
	char	res13[0x200];
	uint32_t	sys_ctl_1;
	uint32_t	sys_ctl_2;
	uint32_t	sys_ctl_3;
	uint32_t	sys_ctl_4;
	uint32_t	dp_vid_ctl;
	char	res14[0x4];
	uint32_t	dp_aud_ctl;
	char	res15[0x24];
	uint32_t	pkt_send_ctl;
	char	res16[0x4];
	uint32_t	dp_hdcp_ctl;
	char	res17[0x34];
	uint32_t	link_bw_set;
	uint32_t	lane_count_set;
	uint32_t	dp_training_ptn_set;
	uint32_t	ln_link_trn_ctl[4];
	char	res18[0x4];
	uint32_t	dp_hw_link_training;
	char	res19[0x1c];
	uint32_t	dp_debug_ctl;
	uint32_t	hpd_deglitch_l;
	uint32_t	hpd_deglitch_h;
	char	res20[0x14];
	uint32_t	dp_link_debug_ctl;
	char	res21[0x1c];
	uint32_t	m_vid_0;
	uint32_t	m_vid_1;
	uint32_t	m_vid_2;
	uint32_t	n_vid_0;
	uint32_t	n_vid_1;
	uint32_t	n_vid_2;
	uint32_t	m_vid_mon;
	char	res22[0x14];
	uint32_t	dp_video_fifo_thrd;
	char	res23[0x8];
	uint32_t	dp_audio_margin;
	char	res24[0x20];
	uint32_t	dp_m_cal_ctl;
	uint32_t	m_vid_gen_filter_th;
	char	res25[0x10];
	uint32_t	m_aud_gen_filter_th;
	char	res26[0x4];
	uint32_t	aux_ch_sta;
	uint32_t	aux_err_num;
	uint32_t	aux_ch_defer_dtl;
	uint32_t	aux_rx_comm;
	uint32_t	buf_data_ctl;
	uint32_t	aux_ch_ctl_1;
	uint32_t	aux_addr_7_0;
	uint32_t	aux_addr_15_8;
	uint32_t	aux_addr_19_16;
	uint32_t	aux_ch_ctl_2;
	char	res27[0x18];
	uint32_t	buf_data[16];
	uint32_t	soc_general_ctl;
	char	res29[0x1e0];
	uint32_t	pll_reg_2;
	uint32_t	pll_reg_3;
	uint32_t	pll_reg_4;
	char	res30[0x10];
	uint32_t	pll_reg_5;
};

int edp_init(uintptr_t edp_addr) {
#define SSC_FUNC_EN_N				(0x1 << 7)
#define AUX_FUNC_EN_N				(0x1 << 2)
#define SERDES_FIFO_FUNC_EN_N			(0x1 << 1)
#define LS_CLK_DOMAIN_FUNC_EN_N			(0x1 << 0)

	volatile struct AnalogixEdp *edp = (volatile struct AnalogixEdp *)edp_addr;

	// Check for HPD_STATUS (hot plug detect)
	for (int i = 0; i < 10; i++) {
		if (edp->sys_ctl_3 & (1 << 6)) {
			debug("HPD status OK: ", edp->sys_ctl_3);
			break;
		} else {
			if (i < 9) {
				debug("Forcing HPD: ", edp->sys_ctl_3);
				edp->sys_ctl_3 |= ((1 << 4) | (1 << 5));
			}
		}

		usleep(1);
	}

	// Enable software function
#define SW_FUNC_EN_N	(0x1 << 0)
	edp->func_en_1 &= ~SW_FUNC_EN_N;

	// Lock the PLL
	edp->dp_pd = 0;
#define PLL_LOCK_CHG	(0x1 << 6)
	edp->common_int_sta_1 = PLL_LOCK_CHG;

#define PLL_LOCK (1 << 4)

	edp->dp_debug_ctl &= ~(0b1100); // clear bits F_PLL_LOCK | PLL_LOCK_CTRL
	for (int i = 100; i != 0; i--) {
		if ((edp->dp_debug_ctl & PLL_LOCK)) {
			puts("PLL is locked");
			break;
		} else {
			debug("Waiting for PLL to be locked... ", edp->dp_debug_ctl);
		}

		if (i == 1) {
			puts("Timeout unable to lock PLL");
			return -1;
		}
		usleep(1);
	}

	return 0;
}

int edp_link_training(volatile struct AnalogixEdp *edp, uint32_t link_rate, uint32_t lane_count) {
	// Write settings to some eDP registers
	edp->link_bw_set = link_rate;
	edp->lane_count_set = lane_count;

	// Thanks to colten for this bit
	edp->dp_training_ptn_set = 0x21;
	usleep(500);
	edp->dp_training_ptn_set = 0x22;
	usleep(500);
	edp->dp_training_ptn_set = 0x0;

	return 0;
}

void edp_shutdown(volatile struct AnalogixEdp *edp) {
	puts("Shutting down eDP");
	edp->video_ctl_1 = 1 << 6;
	edp->sys_ctl_3 = 0x0;
	edp->func_en_1 = 0x61;
}

int edp_enable(uintptr_t edp_addr, uint32_t link_rate, uint32_t lane_count) {
	volatile struct AnalogixEdp *edp = (volatile struct AnalogixEdp *)edp_addr;

    // Enable all function and video modes
	edp->func_en_1 = 0;
    // SSC to normal mode, AUX channel module to normal
	edp->func_en_2 = 0;

	if (edp_link_training(edp, link_rate, lane_count)) {
		puts("link training failed\n");
	}

	// Make sure PLL is still locked
	if (!(edp->dp_debug_ctl & PLL_LOCK)) {
		puts("PLL isn't locked");
		return 1;
	}

#define DET_STA			(0x1 << 2)
#define CHA_STA			(0x1 << 2)

	puts("Checking clock...");
	for (int i = 100; i != 0; i--) {
		usleep(1);
		if (i == 1) {
			return 1;
		}
	
		uint32_t val = edp->sys_ctl_1;
		edp->sys_ctl_1 = val;
		val = edp->sys_ctl_1;
		if (!(val & DET_STA)) {
			continue;
		}

		val = edp->sys_ctl_2;
		edp->sys_ctl_2 = val;
		val = edp->sys_ctl_2;

		if (!(val & CHA_STA)) {
			debug("Waiting on STA: ", val);
			break;
		}
	}
	
	// Disable video mute
#define VIDEO_MUTE (1 << 6)
	edp->video_ctl_1 &= ~VIDEO_MUTE;

	// Enable video output
#define VIDEO_ENABLE (1 << 7)
	edp->video_ctl_1 |= VIDEO_ENABLE;

	// Wait for video stream to come on
#define STRM_VALID (1 << 2)
	for (int i = 100; i != 0; i--) {
		uint32_t val = edp->sys_ctl_3;
		edp->sys_ctl_3 = val;
		val = edp->sys_ctl_3;
		debug("Waiting for video to turn on... ", val);
		if (!(val & STRM_VALID)) {
			puts("enabled video output");
			break;
		}
		if (i == 1) {
			puts("Timeout unable to turn video signal on");
			return 1;
		}

		usleep(1);
	}

	return 0;
}
