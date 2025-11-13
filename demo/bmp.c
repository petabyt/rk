#include <stdint.h>
#include "font.h"
#include "../src/firmware.h"
#include "main.h"

#define BACKGROUND_COLOR 0x4398D7
static uintptr_t fb_addr = 0x0;
static uint32_t screen_width = 1920;
static uint32_t screen_height = 1080;
static int last_x = 10;
static int last_y = 50;

static inline void font_draw_pixel(int x, int y, int col) {
	x *= 2; y *= 2;
	((uint64_t *)(&((uint32_t *)fb_addr)[y * screen_width + x]))[0] = (uint64_t)col << 32 | (uint64_t)col;
	((uint64_t *)(&((uint32_t *)fb_addr)[(y + 1) * screen_width + x]))[0] = (uint64_t)col << 32 | (uint64_t)col;
}

int font_print_char(int x, int y, char c) {
	// Loop to "null terminator character"
	int match = 0;
	for (int l = 0; font[l].letter != 0; l++) {
		if (font[l].letter == c) {
			match = l;
			break;
		}
	}

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

int font_print_string(int x, int y, char *string) {
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
		if (cx > screen_width - 20 && string[c] == ' ') {
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

	uint64_t *framebuffer = (uint64_t *)fb_addr;

	for (int i = 0; i < screen_height * screen_width / 2; i++) {
		framebuffer[i] = (uint64_t)BACKGROUND_COLOR | ((uint64_t)BACKGROUND_COLOR << 32);
	}
}

int bmp_print_char(char c) {
	if (c == ' ') {
		last_x += 5;
	} else if (c == '\n') {
		last_y += 10;
	} else if (c == '\r') {
		last_x = 10;
	} else {
		last_x += font_print_char(last_x, last_y, c) + 3;
	}

	return 1;
}

int bmp_print(char *s) {
	for (int i = 0; s[i] != '\0'; i++) {
		bmp_print_char(s[i]);
	}
	return 0;
}

int bmp_setup(void) {
	struct FuScreenList *list = (struct FuScreenList *)fw_handler(FU_GET_SCREEN_LIST, 0, 0, 0);
	// list is in secure memory, can't access

	if (list->length != 0) {
		fb_addr = list->screens[0].framebuffer_addr;
		screen_width = list->screens[0].width;
		screen_height = list->screens[0].height;
		return 0;
	}
	return 1;
}
