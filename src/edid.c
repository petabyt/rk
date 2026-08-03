#include <string.h>
#include "edid.h"

struct VicMode {
	uint8_t vic;
	struct VideoMode mode;
};

#define MODE(v, p, ha, hf, hs, hb, va, vf, vs, vb, r, f) \
	{ v, { p, ha, hf, hs, hb, va, vf, vs, vb, r, f } }

static const struct VicMode vic_modes[] = {
	MODE(1, 25175, 640, 16, 96, 48, 480, 10, 2, 33, 60, 0),
	MODE(2, 27000, 720, 16, 62, 60, 480, 9, 6, 30, 60, 0),
	MODE(4, 74250, 1280, 110, 40, 220, 720, 5, 5, 20, 60,
		VIDEO_FLAG_HSYNC_HIGH | VIDEO_FLAG_VSYNC_HIGH),
	MODE(16, 148500, 1920, 88, 44, 148, 1080, 4, 5, 36, 60,
		VIDEO_FLAG_HSYNC_HIGH | VIDEO_FLAG_VSYNC_HIGH),
	MODE(19, 74250, 1280, 440, 40, 220, 720, 5, 5, 20, 50,
		VIDEO_FLAG_HSYNC_HIGH | VIDEO_FLAG_VSYNC_HIGH),
	MODE(31, 148500, 1920, 528, 44, 148, 1080, 4, 5, 36, 50,
		VIDEO_FLAG_HSYNC_HIGH | VIDEO_FLAG_VSYNC_HIGH),
	MODE(32, 74250, 1920, 638, 44, 148, 1080, 4, 5, 36, 24,
		VIDEO_FLAG_HSYNC_HIGH | VIDEO_FLAG_VSYNC_HIGH),
	MODE(33, 74250, 1920, 528, 44, 148, 1080, 4, 5, 36, 25,
		VIDEO_FLAG_HSYNC_HIGH | VIDEO_FLAG_VSYNC_HIGH),
	MODE(34, 74250, 1920, 88, 44, 148, 1080, 4, 5, 36, 30,
		VIDEO_FLAG_HSYNC_HIGH | VIDEO_FLAG_VSYNC_HIGH),
	MODE(93, 297000, 3840, 1276, 88, 296, 2160, 8, 10, 72, 24,
		VIDEO_FLAG_HSYNC_HIGH | VIDEO_FLAG_VSYNC_HIGH),
	MODE(94, 297000, 3840, 1056, 88, 296, 2160, 8, 10, 72, 25,
		VIDEO_FLAG_HSYNC_HIGH | VIDEO_FLAG_VSYNC_HIGH),
	MODE(95, 297000, 3840, 176, 88, 296, 2160, 8, 10, 72, 30,
		VIDEO_FLAG_HSYNC_HIGH | VIDEO_FLAG_VSYNC_HIGH),
};

int edid_block_valid(const uint8_t block[EDID_BLOCK_SIZE]) {
	uint8_t sum = 0;
	for (unsigned int i = 0; i < EDID_BLOCK_SIZE; i++)
		sum = (uint8_t)(sum + block[i]);
	return sum == 0;
}

static int mode_supported(const struct VideoMode *m) {
	if (!m->hactive || !m->vactive || !m->pixel_clock_khz)
		return 0;
	if (m->hactive > 3840 || m->vactive > 2160 || m->pixel_clock_khz > 297000)
		return 0;
	if (m->refresh_hz > 60)
		return 0;
	if ((m->hactive > 1920 || m->vactive > 1080) && m->refresh_hz > 30)
		return 0;
	return 1;
}

static int parse_dtd(const uint8_t *d, struct VideoMode *m) {
	uint32_t hblank, vblank, htotal, vtotal;
	if (!d[0] && !d[1])
		return 0;
	memset(m, 0, sizeof(*m));
	m->pixel_clock_khz = ((uint32_t)d[1] << 8 | d[0]) * 10;
	m->hactive = d[2] | ((uint16_t)(d[4] & 0xf0) << 4);
	hblank = d[3] | ((uint16_t)(d[4] & 0x0f) << 8);
	m->vactive = d[5] | ((uint16_t)(d[7] & 0xf0) << 4);
	vblank = d[6] | ((uint16_t)(d[7] & 0x0f) << 8);
	m->hfront_porch = d[8] | ((uint16_t)(d[11] & 0xc0) << 2);
	m->hsync_len = d[9] | ((uint16_t)(d[11] & 0x30) << 4);
	m->vfront_porch = (d[10] >> 4) | ((uint16_t)(d[11] & 0x0c) << 2);
	m->vsync_len = (d[10] & 0x0f) | ((uint16_t)(d[11] & 0x03) << 4);
	if (hblank < (uint32_t)m->hfront_porch + m->hsync_len ||
		vblank < (uint32_t)m->vfront_porch + m->vsync_len || vblank > 255 ||
		m->vfront_porch > 255 || m->vsync_len > 255 || (d[17] & 0x80))
		return 0;
	m->hback_porch = (uint16_t)(hblank - m->hfront_porch - m->hsync_len);
	m->vback_porch = (uint16_t)(vblank - m->vfront_porch - m->vsync_len);
	if ((d[17] & 0x18) == 0x18) {
		if (d[17] & 0x02) m->flags |= VIDEO_FLAG_HSYNC_HIGH;
		if (d[17] & 0x04) m->flags |= VIDEO_FLAG_VSYNC_HIGH;
	}
	htotal = m->hactive + hblank;
	vtotal = m->vactive + vblank;
	m->refresh_hz = (uint16_t)(((uint64_t)m->pixel_clock_khz * 1000 +
		(uint64_t)htotal * vtotal / 2) / ((uint64_t)htotal * vtotal));
	return mode_supported(m);
}

static int vic_mode(uint8_t vic, struct VideoMode *mode) {
	for (unsigned int i = 0; i < sizeof(vic_modes) / sizeof(vic_modes[0]); i++) {
		if (vic_modes[i].vic == vic) {
			*mode = vic_modes[i].mode;
			return 1;
		}
	}
	return 0;
}

static uint64_t mode_score(const struct VideoMode *m) {
	return (uint64_t)m->hactive * m->vactive * 100 + m->refresh_hz;
}

void edid_fallback_mode(struct VideoMode *mode) {
	(void)vic_mode(4, mode);
}

int edid_select_mode(const uint8_t *edid, unsigned int blocks,
		struct VideoMode *mode) {
	static const uint8_t header[8] = { 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00 };
	struct VideoMode candidate, best = { 0 };
	struct VideoMode native_best = { 0 };
	uint64_t best_score = 0;
	uint64_t native_score = 0;

	if (!edid || !mode || !blocks || blocks > EDID_MAX_BLOCKS ||
		memcmp(edid, header, sizeof(header)) || !edid_block_valid(edid))
		return 0;

	/* The first base-block DTD is the EDID preferred timing. */
	if (parse_dtd(edid + 54, &candidate)) {
		*mode = candidate;
		return 1;
	}
	for (unsigned int off = 72; off <= 108; off += 18) {
		if (parse_dtd(edid + off, &candidate) && mode_score(&candidate) > best_score) {
			best = candidate;
			best_score = mode_score(&candidate);
		}
	}

	for (unsigned int b = 1; b < blocks; b++) {
		const uint8_t *ext = edid + b * EDID_BLOCK_SIZE;
		unsigned int end;
		unsigned int native_dtds;
		if (!edid_block_valid(ext) || ext[0] != 0x02)
			continue;
		end = ext[2] ? ext[2] : 127;
		if (end < 4 || end > 127)
			continue;
		native_dtds = ext[3] & 0x0f;
		for (unsigned int i = 4; i < end;) {
			unsigned int tag = ext[i] >> 5;
			unsigned int len = ext[i] & 0x1f;
			if (i + len >= 127) break;
			if (tag == 2) {
				for (unsigned int j = 1; j <= len; j++) {
					uint8_t svd = ext[i + j];
					if (vic_mode(svd & 0x7f, &candidate)) {
						if ((svd & 0x80) && mode_supported(&candidate)) {
							if (mode_score(&candidate) > native_score) {
								native_best = candidate;
								native_score = mode_score(&candidate);
							}
						}
						if (mode_supported(&candidate) && mode_score(&candidate) > best_score) {
							best = candidate;
							best_score = mode_score(&candidate);
						}
					}
				}
			}
			i += len + 1;
		}
		for (unsigned int off = end, dtd_index = 0;
			off + 17 < 127; off += 18, dtd_index++) {
			if (parse_dtd(ext + off, &candidate)) {
				uint64_t score = mode_score(&candidate);
				if (dtd_index < native_dtds && score > native_score) {
					native_best = candidate;
					native_score = score;
				} else if (dtd_index >= native_dtds && score > best_score) {
					best = candidate;
					best_score = score;
				}
			}
		}
	}
	if (native_score) {
		*mode = native_best;
		return 1;
	}
	if (!best_score)
		return 0;
	*mode = best;
	return 1;
}
