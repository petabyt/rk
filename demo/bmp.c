#include <stdint.h>
#include "font.h"
#include "../src/firmware.h"
#include "main.h"

#define BACKGROUND_COLOR 0x4398D7
static uintptr_t fb_addr = 0x0;
static uint32_t screen_width;
static uint32_t screen_height;
static uint32_t screen_stride;
static int last_x = 10;
static int last_y = 50;

static inline void font_draw_pixel(int x, int y, int col) {
	x *= 2; y *= 2;
	if ((unsigned int)(x + 1) >= screen_width ||
		(unsigned int)(y + 1) >= screen_height)
		return;
	uint32_t stride_pixels = screen_stride / 4;
	((uint64_t *)(&((uint32_t *)fb_addr)[y * stride_pixels + x]))[0] =
		(uint64_t)col << 32 | (uint64_t)col;
	((uint64_t *)(&((uint32_t *)fb_addr)[(y + 1) * stride_pixels + x]))[0] =
		(uint64_t)col << 32 | (uint64_t)col;
}

int font_print_char(int x, int y, char c) {
	// Loop to "null terminator character"
	int match = -1;
	for (int l = 0; font[l].letter != 0; l++) {
		if (font[l].letter == c) {
			match = l;
			break;
		}
	}
	if (match < 0)
		return 0;

	// Loop through 7 high 5 wide monochrome font
	int maxLength = 0;
	for (int py = 0; py < 7; py++) {
		for (int px = 0; px < 5; px++) {
			if (font[match].code[py][px] == '#') {
				font_draw_pixel(x + px, y + py, 0xffffff);

				// Check width of characters for better spacing
				if (px > maxLength) {
					maxLength = px;
				}
			}
		}
	}

	return maxLength;
}

int font_print_string(int x, int y, const char *string) {
	int cx = x;
	int cy = y;

	for (int c = 0; string[c] != '\0'; c++) {
		int length;
		if (string[c] == ' ') {
			length = 5;
		} else {
			length = font_print_char(cx, cy, string[c]) + 3;
		}

		cx += length;

		// wrapping
		if ((unsigned int)(cx * 2) > screen_width - 20 && string[c] == ' ') {
			cx = x;
			cy += 8;
		}
	}

	return cy;
}

void bmp_draw_rect(int x, int y, int w, int h, int col) {
	for (int y2 = y; y2 < h; y2++) {
		for (int x2 = x; x2 < w; x2++) {
			font_draw_pixel(x2, y2, col);
		}
	}
}

void bmp_clear(void) {
	last_x = 10;
	last_y = 10;

	uint32_t *framebuffer = (uint32_t *)fb_addr;
	for (uint32_t y = 0; y < screen_height; y++)
		for (uint32_t x = 0; x < screen_width; x++)
			framebuffer[y * (screen_stride / 4) + x] = BACKGROUND_COLOR;
}

int bmp_print_char(char c) {
	if (c == ' ') {
		last_x += 5;
	} else if (c == '\t') {
		last_x = 10 + (((last_x - 10) / 32) + 1) * 32;
	} else if (c == '\b') {
		if (last_x > 10)
			last_x = last_x > 17 ? last_x - 7 : 10;
		for (int y = 0; y < 8; y++)
			for (int x = 0; x < 7; x++)
				font_draw_pixel(last_x + x, last_y + y,
					BACKGROUND_COLOR);
	} else if (c == '\n') {
		last_y += 10;
	} else if (c == '\r') {
		last_x = 10;
	} else if (c >= 0x20 && c <= 0x7e) {
		last_x += font_print_char(last_x, last_y, c) + 3;
	}
	if ((unsigned int)(last_x * 2 + 14) >= screen_width) {
		last_x = 10;
		last_y += 10;
	}

	return 1;
}

int bmp_print(const char *s) {
	for (int i = 0; s[i] != '\0'; i++) {
		bmp_print_char(s[i]);
	}
	return 0;
}

int bmp_setup(void) {
	struct FuScreenList *list = (struct FuScreenList *)fw_handler(FU_GET_SCREEN_LIST, 0, 0, 0);
	if (list != (struct FuScreenList *)FU_ERROR && list->length != 0 &&
		list->type == FU_SCREEN_XRGB8888) {
		fb_addr = list->screens[0].framebuffer_addr;
		screen_width = list->screens[0].width;
		screen_height = list->screens[0].height;
		screen_stride = list->screens[0].stride;
		return 0;
	}
	return 1;
}
