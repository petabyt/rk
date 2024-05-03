#include <string.h>
#include <stdint.h>

#include "pins.h"
#include "io.h"
#include "os.h"

void clock_setup_vop(); // clock.c

static void gpio_set_backlight() {
	puts("Turning on backlight");

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

int init_vop() {
	puts("Setting up VOP");

	volatile struct Vop *vop = (volatile struct Vop *)VOP_LIT_BASE;

	clock_setup_vop();

#define STANDBY_MODE (1 << 22)

	// Exit standby mode
	vop->sys_ctrl &= ~STANDBY_MODE;

#define OUT_MIPI (1 << 15)
#define OUT_EDP (1 << 14)
#define OUT_HDMI (1 << 13)
#define OUT_RGB (1 << 12)

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

		vop->win0_yrgb_mst = FB_ADDR;

		vop->reg_cfg_done = 0x1;
	}

	return 0;
}

int init_edp() {
#define SSC_FUNC_EN_N				(0x1 << 7)
#define AUX_FUNC_EN_N				(0x1 << 2)
#define SERDES_FIFO_FUNC_EN_N			(0x1 << 1)
#define LS_CLK_DOMAIN_FUNC_EN_N			(0x1 << 0)

	volatile struct eDpRegs *edp = (volatile struct eDpRegs *)EDP_BASE;

	// edp_ref_clk_sel = 1 (select clock)
	rk_clr_set_bits((uint32_t *)(GRF_BASE + GRF_SOC_CON25), 11, 11, 1);

	// Invert clock phase
	//rk_clr_set_bits((uint32_t *)(GRF_BASE + GRF_SOC_CON20), 9, 9, 1);

	// Set edp_lcdc_sel = vop little
	rk_clr_set_bits((uint32_t *)(GRF_BASE + GRF_SOC_CON20), 5, 5, 1);

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

	gpio_set_backlight();

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

// Controls to start transaction between the DisplayPort transmitter and reciever
// This is used when the aux registers are already set, and set for a transaction
int edp_aux_start_transaction(volatile struct eDpRegs *edp) {
#define AUX_EN					(0x1 << 0)
#define RPLY_RECEIV				(0x1 << 1)
#define AUX_ERR					(0x1 << 0)
#define AUX_STATUS_MASK			(0xf << 0)

	// Enable AUX CH operation
	edp->aux_ch_ctl_2 |= AUX_EN;

	int tm = 10000;
	while (edp->aux_ch_ctl_2 & AUX_EN) {
		usleep(1); tm--;
		if (tm == 0) {
			puts("AUX CH enable timeout");
			return 1;
		}
	}

	tm = 10;
	while (!(edp->dp_int_sta & RPLY_RECEIV)) {
		usleep(1); tm--;
		if (tm == 0) {
			puts("AUX CH command reply timeout");
			return 1;
		}
	}
	edp->dp_int_sta = RPLY_RECEIV;

	// Check AUX err bit
	if (edp->dp_int_sta & AUX_ERR) {
		debug("AUX CH Access Error: ", edp->dp_int_sta);
		// Register must be cleared
		edp->dp_int_sta = AUX_ERR;
		msleep(50);
		return 1;
	}

	if (edp->dp_int_sta & AUX_STATUS_MASK) {
		debug("AUX CH Error: ", edp->dp_int_sta);
		return 1;
	}

	return 0;
}

// Read or write data in the DPCD (DisplayPort Configuration Data)
// This is basically used to get the displayport's info like revision, maximum rates, capabilities, etc
// Some info on page 86 DisplayPort 1.1
// dir: 0 = read, 1 = write
int edp_dpcd_transfer(volatile struct eDpRegs *edp, uint32_t addr, uint8_t *buf, int len, int dir) {
#define AUX_LENGTH(x)				(((x - 1) & 0xf) << 4)
#define AUX_TX_COMM_DP_TRANSACTION		(0x1 << 3)
#define AUX_TX_COMM_WRITE			(0x0 << 0)
#define AUX_TX_COMM_READ			(0x1 << 0)
#define BUF_CLR					(0x1 << 7)

	int cnt = 10;
	while (1) {
		cnt--;
		if (cnt == 0) {
			puts("DPCD: Failed to send command");
			return 1;
		}

		while (len) {
			int max = 16;
			if (max > len) max = len;

			edp->buf_data_ctl = BUF_CLR; // clear AUX CH data buffer

			// Most unusual way to write an address to a register...
			edp->aux_addr_7_0 = ((addr >> 0) & 0xff);
			edp->aux_addr_15_8 = ((addr >> 8) & 0xff);
			edp->aux_addr_19_16 = ((addr >> 16) & 0xff);

			if (dir == 1) {
				// Write data into edp temp buffer
				for (int i = 0; i < max; i++) {
					edp->buf_data[i] = (uint32_t)(*buf);
					buf++;
				}

				// Prepare AUX CH command
				edp->aux_ch_ctl_1 = AUX_LENGTH(max) | AUX_TX_COMM_DP_TRANSACTION | AUX_TX_COMM_WRITE;
			} else {
				// Read command
				edp->aux_ch_ctl_1 = AUX_LENGTH(max) | AUX_TX_COMM_DP_TRANSACTION | AUX_TX_COMM_READ;
			}

			// Start the command
			if (edp_aux_start_transaction(edp)) {
				continue;
			}

			for (int i = 0; i < len; i++) {
				*buf = (uint8_t)edp->buf_data[i];
				buf++;
			}

			len -= max;
		}

		return 0;
	}

	return 0;
}

#define DPCD_TRANSFER_MAX 10

int edp_dpcd_read(volatile struct eDpRegs *edp, int addr, void *buf, int size) {
	int i;
	for (i = 0; i < DPCD_TRANSFER_MAX; i++) {
		if (!edp_dpcd_transfer(edp, addr, buf, size, 0)) {
			break;
		}
	}

	return i == (DPCD_TRANSFER_MAX - 1);
}

int edp_dpcd_write(volatile struct eDpRegs *edp, int addr, void *buf, int size) {
	int i;
	for (i = 0; i < DPCD_TRANSFER_MAX; i++) {
		if (!edp_dpcd_transfer(edp, addr, buf, size, 1)) {
			break;
		}
	}

	return i == (DPCD_TRANSFER_MAX - 1);
}

// (Unused)
int edp_dpcd_debug(volatile struct eDpRegs *edp) {
	// We have 0.5% down spread
	uint8_t max_downspread;
	edp_dpcd_read(edp, 3, &max_downspread, 1);
	debug("Downspread: ", max_downspread);

#define NO_AUX_HANDSHAKE_LINK_TRAINING (1 << 6)
	if (max_downspread & NO_AUX_HANDSHAKE_LINK_TRAINING) {
		puts("Doesn't require AUX CH handshake");
	} else {
		puts("Requires AUX CH handshake");
	}

	uint8_t power_state;
	edp_dpcd_read(edp, DPCD_SINK_CONTROL_FIELD, &power_state, 1);
	debug("Power state: ", power_state);

	// Set SET_POWER_STATE to D0
	// Set local sink and all downstream sinks to D0 (normal operation mode)
	power_state = 0b01;
	edp_dpcd_write(edp, DPCD_SINK_CONTROL_FIELD, &power_state, 1);

#define DPCD_LINK_CONTROL_FIELD 0x100
	uint8_t config[2];
	edp_dpcd_read(edp, DPCD_LINK_CONTROL_FIELD, config, 2);
	debug("Config: ", config[0]);

#define LANE0_1_STATUS 0x202
	uint8_t status;
	edp_dpcd_read(edp, DPCD_LINK_CONTROL_FIELD, &status, 1);
	debug("Link status: ", status);

	edp->dp_hw_link_training = 1;
	debug("Training status: ", edp->dp_hw_link_training);

	return 0;
}

int edp_link_training(volatile struct eDpRegs *edp) {
	/*

	Look at page 112 DisplayPort v1.1 spec for DPCD (DisplayPort Config Data)
	official address mapping

	*/

	uint8_t revision_info[3];

	edp_dpcd_read(edp, 0, revision_info, 3);
	debug("revision: ", revision_info[0]);
	debug("link rate: ", revision_info[1]);
	debug("lane count: ", revision_info[2]);

	/*
	Link training:
	- Negotiate max speed of connection
	- Setup link lanes (eDP has 2)
	- If failed:
	- Clock recovery, reduce rate

	We don't even need to do this, just give it the right numbers and move on.
	*/

	// Set SET_POWER_STATE (600h) to "local sink and all downstream sinks to D0 (normal operationmode)"
	uint8_t power_state = 0b01;
	edp_dpcd_write(edp, 0x600, &power_state, 1);

	msleep(1);

	// Write settings to the link config field (100h)
	uint8_t config[2] = {revision_info[1], revision_info[2]};
	edp_dpcd_write(edp, 0x100, config, sizeof(config));

	// Write settings to some eDP registers
	edp->link_bw_set = revision_info[1];
	edp->lane_count_set = revision_info[2];

	return 0;
}

void edp_shutdown() {
	volatile struct eDpRegs *edp = (volatile struct eDpRegs *)EDP_BASE;

	puts("Shutting down eDP");
	edp->video_ctl_1 = 1 << 6;
	edp->sys_ctl_3 = 0x0;
	edp->func_en_1 = 0x61;

	// TODO: clear PLL and GPIO
}

extern int blob_link_training(volatile void *ptr);

int enable_edp() {
	volatile struct eDpRegs *edp = (volatile struct eDpRegs *)EDP_BASE;

    // Enable all function and video modes
	edp->func_en_1 = 0;
    // SSC to normal mode, AUX channel module to normal
	edp->func_en_2 = 0;

	if (edp_link_training(edp)) {
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
	
	// Now start enabling and setting up the eDP

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

uint32_t *sys_get_framebuffer() {
	return (uint32_t *)FB_ADDR;
}

void sys_turn_on_screen() {
	puts("Turning on eDP");
	if (init_edp()) {
		puts("init edp failed");
		return;
	}

	if (init_vop()) {
		puts("failed to init vopl");
		return;
	}

	if (enable_edp()) {
		puts("enable edp failed");
		return;
	}

	puts("Screen initialized");
}
