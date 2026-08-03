#include <string.h>
#include "main.h"
#include "edid.h"
#include "rk356x.h"

#define BIT(x) (1U << (x))
#define HDMI(reg) ((volatile uint32_t *)(RK356X_HDMI + ((reg) << 2)))

struct Rk356xVideo rk356x_video;

extern int rk356x_display_clocks(uint32_t pixel_clock_khz);
extern void rk356x_vop2_setup(const struct VideoMode *mode, uint32_t stride);

static uint8_t hdmi_read(unsigned int offset) {
	return (uint8_t)*HDMI(offset);
}

static void hdmi_write(unsigned int offset, uint8_t value) {
	*HDMI(offset) = value;
}

static void hdmi_modify(unsigned int offset, uint8_t mask, uint8_t value) {
	hdmi_write(offset, (hdmi_read(offset) & ~mask) | (value & mask));
}

static int ddc_wait(void) {
	for (unsigned int timeout = 0; timeout < 100000; timeout++) {
		uint8_t status = hdmi_read(0x0105) & 3;
		if (status) {
			hdmi_write(0x0105, status);
			return (status & 2) ? 0 : -1;
		}
	}
	return -1;
}

static void ddc_init(void) {
	hdmi_write(0x7e09, 0);
	hdmi_write(0x7e07, 0);
	hdmi_write(0x7e05, 0x08);
	hdmi_write(0x7e06, 0x88);
	hdmi_write(0x0105, 3);
	hdmi_write(0x7e00, 0x50);
}

static int ddc_read_block(unsigned int block, uint8_t *data) {
	unsigned int segment = block >> 1;
	unsigned int start = (block & 1) ? 128 : 0;
	hdmi_write(0x7e08, 0x30);
	hdmi_write(0x7e0a, segment);
	for (unsigned int i = 0; i < EDID_BLOCK_SIZE; i++) {
		hdmi_write(0x7e01, start + i);
		hdmi_write(0x7e04, segment ? 2 : 1);
		if (ddc_wait())
			return -1;
		data[i] = hdmi_read(0x7e03);
	}
	return 0;
}

static int read_edid(uint8_t *edid, unsigned int *blocks) {
	unsigned int wanted;
	ddc_init();
	if (ddc_read_block(0, edid) || !edid_block_valid(edid))
		return -1;
	wanted = 1 + edid[126];
	if (wanted > EDID_MAX_BLOCKS)
		wanted = EDID_MAX_BLOCKS;
	*blocks = 1;
	for (unsigned int i = 1; i < wanted; i++) {
		if (ddc_read_block(i, edid + i * EDID_BLOCK_SIZE))
			break;
		*blocks = i + 1;
	}
	return 0;
}

static void fc_write16(unsigned int low, uint16_t value) {
	hdmi_write(low, value);
	hdmi_write(low + 1, value >> 8);
}

static uint8_t mode_vic(const struct VideoMode *m) {
	if (m->hactive == 640 && m->vactive == 480 && m->refresh_hz == 60) return 1;
	if (m->hactive == 720 && m->vactive == 480 && m->refresh_hz == 60) return 2;
	if (m->hactive == 1280 && m->vactive == 720 && m->refresh_hz == 60) return 4;
	if (m->hactive == 1920 && m->vactive == 1080 && m->refresh_hz == 60) return 16;
	if (m->hactive == 1280 && m->vactive == 720 && m->refresh_hz == 50) return 19;
	if (m->hactive == 1920 && m->vactive == 1080 && m->refresh_hz == 50) return 31;
	if (m->hactive == 1920 && m->vactive == 1080 && m->refresh_hz == 24) return 32;
	if (m->hactive == 1920 && m->vactive == 1080 && m->refresh_hz == 25) return 33;
	if (m->hactive == 1920 && m->vactive == 1080 && m->refresh_hz == 30) return 34;
	if (m->hactive == 3840 && m->vactive == 2160 && m->refresh_hz == 24) return 93;
	if (m->hactive == 3840 && m->vactive == 2160 && m->refresh_hz == 25) return 94;
	if (m->hactive == 3840 && m->vactive == 2160 && m->refresh_hz == 30) return 95;
	return 0;
}

static void hdmi_video_setup(const struct VideoMode *m) {
	uint16_t hblank = m->hfront_porch + m->hsync_len + m->hback_porch;
	uint8_t vblank = m->vfront_porch + m->vsync_len + m->vback_porch;
	uint8_t vic = mode_vic(m);
	uint8_t aspect = 0;
	uint8_t invidconf = 0x18; /* HDMI, active-high DE, progressive RGB. */
	if (vic == 1 || vic == 2 || (uint32_t)m->hactive * 3 ==
		(uint32_t)m->vactive * 4)
		aspect = 0x10;
	else if (vic || (uint32_t)m->hactive * 9 ==
		(uint32_t)m->vactive * 16)
		aspect = 0x20;
	if (m->flags & VIDEO_FLAG_HSYNC_HIGH) invidconf |= 0x20;
	if (m->flags & VIDEO_FLAG_VSYNC_HIGH) invidconf |= 0x40;
	hdmi_write(0x1000, invidconf);
	fc_write16(0x1001, m->hactive);
	fc_write16(0x1003, hblank);
	fc_write16(0x1005, m->vactive);
	hdmi_write(0x1007, vblank);
	fc_write16(0x1008, m->hfront_porch);
	fc_write16(0x100a, m->hsync_len);
	hdmi_write(0x100c, m->vfront_porch);
	hdmi_write(0x100d, m->vsync_len);

	/* RGB888 video sampler and a bypassed 8-bpc packetizer. */
	hdmi_write(0x0200, 0x01);
	hdmi_write(0x0201, 0x07);
	for (unsigned int r = 0x0202; r <= 0x0207; r++) hdmi_write(r, 0);
	hdmi_write(0x0801, 0x40);
	hdmi_write(0x0802, 0x27);
	hdmi_write(0x0803, 0);
	hdmi_write(0x0804, 0x47);

	/* RGB AVI infoframe with the timing's advertised picture aspect. */
	hdmi_write(0x1019, 0);
	hdmi_write(0x101a, aspect);
	hdmi_write(0x101b, 0x08);
	hdmi_write(0x101c, vic);
	hdmi_write(0x1011, 12);
	hdmi_write(0x1012, 32);
	hdmi_write(0x1013, 1);
	hdmi_write(0x1014, 0x0b);
	hdmi_write(0x1015, 0x16);
	hdmi_write(0x1016, 0x21);
	hdmi_write(0x4004, 0); /* Feed-through, no CSC. */
	hdmi_write(0x4001, 0x7e);
	hdmi_write(0x4001, 0x7c);
}

struct PhyRate {
	uint32_t max_khz;
	uint16_t cpce;
	uint16_t gmp;
	uint16_t sym;
	uint16_t term;
	uint16_t vlev;
};

static const struct PhyRate phy_rates[] = {
	{ 30666, 0x00b3, 0x0000, 0x8009, 0x0004, 0x0272 },
	{ 74250, 0x0072, 0x0001, 0x8009, 0x0004, 0x0272 },
	{ 165000, 0x0051, 0x0002, 0x802b, 0x0004, 0x0209 },
	{ 184000, 0x0051, 0x0002, 0x8039, 0x0005, 0x028d },
	{ 340000, 0x0040, 0x0003, 0x8039, 0x0005, 0x028d },
};

static int phy_i2c_write(uint8_t address, uint16_t value) {
	hdmi_write(0x0108, 3);
	hdmi_write(0x3021, address);
	hdmi_write(0x3022, value >> 8);
	hdmi_write(0x3023, value);
	hdmi_write(0x3026, 0x10);
	for (unsigned int timeout = 0; timeout < 100000; timeout++) {
		uint8_t status = hdmi_read(0x0108) & 3;
		if (status) {
			hdmi_write(0x0108, status);
			return (status & 2) ? 0 : -1;
		}
	}
	return -1;
}

static int hdmi_phy_setup(uint32_t clock_khz) {
	const struct PhyRate *rate = 0;
	for (unsigned int i = 0; i < sizeof(phy_rates) / sizeof(phy_rates[0]); i++)
		if (clock_khz <= phy_rates[i].max_khz) { rate = &phy_rates[i]; break; }
	if (!rate)
		return -1;

	/* The Gen2 PHY requires the complete setup sequence twice. */
	for (unsigned int pass = 0; pass < 2; pass++) {
		int locked = 0;
		hdmi_modify(0x3000, 0x18, 0x10); /* TX off, PDDQ asserted. */
		hdmi_modify(0x3000, 0x23, 0x22); /* SVSRET, data polarity, PHY I/F. */
		hdmi_write(0x4005, 1);
		hdmi_write(0x4005, 0);
		hdmi_write(0x4007, 1);
		hdmi_modify(0x3001, 0x20, 0x20);
		hdmi_write(0x3020, 0x69);
		hdmi_modify(0x3001, 0x20, 0);
		if (phy_i2c_write(0x06, rate->cpce) ||
			phy_i2c_write(0x15, rate->gmp) ||
			phy_i2c_write(0x10, 0) || phy_i2c_write(0x13, 0) ||
			phy_i2c_write(0x17, 6) || phy_i2c_write(0x19, rate->term) ||
			phy_i2c_write(0x09, rate->sym) ||
			phy_i2c_write(0x0e, rate->vlev) ||
			phy_i2c_write(0x05, 0x8000))
			return -1;
		hdmi_modify(0x3000, 0x1c, 0x0c); /* TX power, PDDQ clear. */
		for (unsigned int timeout = 0; timeout < 100000; timeout++) {
			if (hdmi_read(0x3004) & 1) {
				locked = 1;
				break;
			}
		}
		if (!locked)
			return -1;
	}
	return 0;
}

int rk356x_display_init(void) {
	uint8_t edid[EDID_BLOCK_SIZE * EDID_MAX_BLOCKS];
	unsigned int blocks = 0;
	memset(&rk356x_video, 0, sizeof(rk356x_video));
	memset(edid, 0, sizeof(edid));

	/* Clear GRF bits that otherwise force HDMI DDC inputs inactive. */
	*(volatile uint32_t *)(RK356X_GRF + 0x364) = (BIT(15) | BIT(14)) << 16;
	if (rk356x_display_clocks(74250))
		return -1;
	if (!(hdmi_read(0x3004) & 2))
		return -1;
	rk356x_video.hpd = 1;
	if (!read_edid(edid, &blocks) && edid_select_mode(edid, blocks,
			&rk356x_video.mode)) {
		rk356x_video.edid_valid = 1;
	} else {
		edid_fallback_mode(&rk356x_video.mode);
	}
	rk356x_video.stride = (rk356x_video.mode.hactive * 4 + 63) & ~63U;
	if ((uint64_t)rk356x_video.stride * rk356x_video.mode.vactive >
		RK356X_FB_END - RK356X_FB_START)
		return -1;
	if (rk356x_display_clocks(rk356x_video.mode.pixel_clock_khz))
		return -1;
	memset((void *)RK356X_FB_START, 0,
		(uint64_t)rk356x_video.stride * rk356x_video.mode.vactive);
	rk356x_vop2_setup(&rk356x_video.mode, rk356x_video.stride);
	hdmi_video_setup(&rk356x_video.mode);
	if (hdmi_phy_setup(rk356x_video.mode.pixel_clock_khz))
		return -1;
	rk356x_video.active = 1;
	return 0;
}
