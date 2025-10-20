// Mostly from https://github.com/petabyt/font
#include <stdint.h>
#include <string.h>
#include "os.h"
#include "font.h"

static inline void font_draw_pixel(int x, int y, int col) {
	x *= 2; y *= 2;
	((uint32_t *)FB_ADDR)[y * 1920 + x] = col;
	((uint32_t *)FB_ADDR)[(y + 1) * 1920 + x] = col;
	((uint32_t *)FB_ADDR)[(y + 1) * 1920 + x + 1] = col;
	((uint32_t *)FB_ADDR)[y * 1920 + x + 1] = col;
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

#ifdef SCREEN_LENGTH
		if (cx > SCREEN_WIDTH - 20 && string[c] == ' ') {
			cx = x;
			cy += 8;
		}
#endif
	}

	return cy;
}

static int last_x = 10;
static int last_y = 50;

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

	uint32_t *framebuffer = sys_get_framebuffer();

	for (int i = 0; i < 1080 * 1920; i++) {
		framebuffer[i] = 0x4398D7;
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

	if (last_y * 2 >= 1080) {
		bmp_clear();
	}

	return 1;
}

int putchar(int c) {
	bmp_print_char(c);
	uart_chr((char)c);
	return 0;
}
