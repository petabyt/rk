#ifndef RK_EDID_H
#define RK_EDID_H

#include <stdint.h>

#define EDID_BLOCK_SIZE 128
#define EDID_MAX_BLOCKS 5

#define VIDEO_FLAG_HSYNC_HIGH (1u << 0)
#define VIDEO_FLAG_VSYNC_HIGH (1u << 1)

struct VideoMode {
	uint32_t pixel_clock_khz;
	uint16_t hactive;
	uint16_t hfront_porch;
	uint16_t hsync_len;
	uint16_t hback_porch;
	uint16_t vactive;
	uint16_t vfront_porch;
	uint16_t vsync_len;
	uint16_t vback_porch;
	uint16_t refresh_hz;
	uint16_t flags;
};

int edid_block_valid(const uint8_t block[EDID_BLOCK_SIZE]);
int edid_select_mode(const uint8_t *edid, unsigned int blocks,
		struct VideoMode *mode);
void edid_fallback_mode(struct VideoMode *mode);

#endif
