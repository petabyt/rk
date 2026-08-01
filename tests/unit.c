#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "edid.h"
#include "hid_keyboard.h"
#include "input.h"
#include "ohci.h"
#include "usb.h"

static void checksum(uint8_t block[128]) {
	uint8_t sum = 0;
	block[127] = 0;
	for (unsigned int i = 0; i < 127; i++) sum += block[i];
	block[127] = (uint8_t)(0 - sum);
}

static void base_edid(uint8_t block[128]) {
	static const uint8_t header[8] =
		{ 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00 };
	memset(block, 0, 128);
	memcpy(block, header, sizeof(header));
	block[18] = 1;
	block[19] = 4;
}

static void dtd(uint8_t *d, uint32_t clock_khz, uint16_t width,
		uint16_t hblank, uint16_t hfront, uint16_t hsync,
		uint16_t height, uint16_t vblank, uint16_t vfront, uint16_t vsync,
		uint8_t flags) {
	uint16_t clock = (uint16_t)(clock_khz / 10);
	memset(d, 0, 18);
	d[0] = (uint8_t)clock; d[1] = (uint8_t)(clock >> 8);
	d[2] = (uint8_t)width; d[3] = (uint8_t)hblank;
	d[4] = (uint8_t)(((width >> 8) << 4) | (hblank >> 8));
	d[5] = (uint8_t)height; d[6] = (uint8_t)vblank;
	d[7] = (uint8_t)(((height >> 8) << 4) | (vblank >> 8));
	d[8] = (uint8_t)hfront; d[9] = (uint8_t)hsync;
	d[10] = (uint8_t)((vfront << 4) | vsync);
	d[11] = (uint8_t)(((hfront >> 8) << 6) | ((hsync >> 8) << 4) |
		((vfront >> 4) << 2) | (vsync >> 4));
	d[17] = flags;
}

static void test_edid(void) {
	uint8_t edid[256];
	struct VideoMode mode;
	base_edid(edid);
	dtd(edid + 54, 148500, 1920, 280, 88, 44, 1080, 45, 4, 5, 0x1e);
	checksum(edid);
	assert(edid_block_valid(edid));
	assert(edid_select_mode(edid, 1, &mode));
	assert(mode.hactive == 1920 && mode.vactive == 1080 && mode.refresh_hz == 60);

	base_edid(edid);
	edid[126] = 1;
	checksum(edid);
	memset(edid + 128, 0, 128);
	edid[128] = 0x02; edid[129] = 0x03; edid[130] = 6;
	edid[132] = 0x41; edid[133] = 0x80 | 95;
	checksum(edid + 128);
	assert(edid_select_mode(edid, 2, &mode));
	assert(mode.hactive == 3840 && mode.vactive == 2160 && mode.refresh_hz == 30);

	/* 4K60 is unsupported and a Y420-only block does not become RGB. */
	edid[133] = 97;
	checksum(edid + 128);
	assert(!edid_select_mode(edid, 2, &mode));
	edid[132] = (7 << 5) | 2; edid[133] = 0x0e; edid[134] = 95;
	edid[130] = 7;
	checksum(edid + 128);
	assert(!edid_select_mode(edid, 2, &mode));

	/* A native CTA DTD wins over a larger non-native DTD. */
	base_edid(edid);
	edid[126] = 1;
	checksum(edid);
	memset(edid + 128, 0, 128);
	edid[128] = 0x02; edid[129] = 0x03; edid[130] = 4; edid[131] = 1;
	dtd(edid + 132, 74250, 1280, 370, 110, 40, 720, 30, 5, 5, 0x1e);
	dtd(edid + 150, 148500, 1920, 280, 88, 44, 1080, 45, 4, 5, 0x1e);
	checksum(edid + 128);
	assert(edid_select_mode(edid, 2, &mode));
	assert(mode.hactive == 1280 && mode.vactive == 720);

	/* Non-native choices rank by pixel area and then refresh rate. */
	memset(edid + 128, 0, 128);
	edid[128] = 0x02; edid[129] = 0x03; edid[130] = 7;
	edid[132] = 0x42; edid[133] = 32; edid[134] = 16;
	checksum(edid + 128);
	assert(edid_select_mode(edid, 2, &mode));
	assert(mode.hactive == 1920 && mode.vactive == 1080 && mode.refresh_hz == 60);
	edid[130] = 8; edid[132] = 0x43; edid[135] = 93;
	checksum(edid + 128);
	assert(edid_select_mode(edid, 2, &mode));
	assert(mode.hactive == 3840 && mode.vactive == 2160 && mode.refresh_hz == 24);

	base_edid(edid);
	dtd(edid + 54, 74250, 1920, 280, 88, 44, 1080, 45, 4, 5, 0x80);
	checksum(edid);
	assert(!edid_select_mode(edid, 1, &mode));
	edid[20] ^= 1;
	assert(!edid_select_mode(edid, 1, &mode));
	edid_fallback_mode(&mode);
	assert(mode.hactive == 1280 && mode.vactive == 720);
}

static void report(uint8_t modifier, uint8_t usage) {
	uint8_t data[8] = { modifier, 0, usage, 0, 0, 0, 0, 0 };
	hid_keyboard_report(data);
}

static void release_keys(void) {
	uint8_t data[8] = { 0 };
	hid_keyboard_report(data);
}

static void test_hid(void) {
	input_reset(); hid_keyboard_reset();
	report(0, 0x04); assert(input_get_char() == 'a');
	report(0, 0x04); assert(!input_available());
	release_keys(); report(0x02, 0x04); assert(input_get_char() == 'A');
	release_keys(); report(0, 0x39); release_keys();
	report(0, 0x04); assert(input_get_char() == 'A');
	release_keys(); report(0x02, 0x1e); assert(input_get_char() == '!');
	release_keys(); report(0, 0x28); assert(input_get_char() == '\r');
	release_keys(); report(0, 0x2a); assert(input_get_char() == '\b');
	release_keys(); report(0, 0x2b); assert(input_get_char() == '\t');
	release_keys(); report(0, 0x29); assert(input_get_char() == 0x1b);
	release_keys(); report(0, 0x4f); assert(!input_available());
	release_keys(); report(0, 0x3a); assert(!input_available());
	release_keys(); report(0x04, 0x04); assert(!input_available());
	release_keys();
	uint8_t rollover[8] = { 0, 0, 1, 1, 1, 1, 1, 1 };
	hid_keyboard_report(rollover); assert(!input_available());

	input_reset();
	for (unsigned int i = 0; i < 63; i++) assert(input_enqueue('x'));
	assert(!input_enqueue('y'));
	for (unsigned int i = 0; i < 32; i++) assert(input_get_char() == 'x');
	for (unsigned int i = 0; i < 31; i++) assert(input_enqueue('z'));
	for (unsigned int i = 0; i < 31; i++) assert(input_get_char() == 'x');
	for (unsigned int i = 0; i < 31; i++) assert(input_get_char() == 'z');
	assert(!input_available());
}

static void test_usb(void) {
	uint8_t composite[] = {
		9, USB_DT_CONFIG, 34, 0, 2, 1, 0, 0x80, 50,
		9, USB_DT_INTERFACE, 0, 0, 1, 8, 6, 80, 0,
		9, USB_DT_INTERFACE, 2, 0, 1, 3, 1, 1, 0,
		7, USB_DT_ENDPOINT, 0x83, 3, 8, 0, 10,
	};
	struct UsbBootKeyboard keyboard;
	assert(!usb_find_boot_keyboard(composite, sizeof(composite), &keyboard));
	assert(keyboard.interface_number == 2 && keyboard.endpoint == 3);
	assert(keyboard.max_packet == 8 && keyboard.interval == 10);
	composite[9 + 9 + 9 + 6] = 0;
	assert(usb_find_boot_keyboard(composite, sizeof(composite), &keyboard));
	composite[9 + 9 + 9 + 6] = 10;
	composite[9 + 9 + 3] = 1;
	assert(usb_find_boot_keyboard(composite, sizeof(composite), &keyboard));
	composite[9 + 9 + 3] = 0;
	composite[9 + 9 + 7] = 2;
	assert(usb_find_boot_keyboard(composite, sizeof(composite), &keyboard));

	static uint8_t arena[1024 + 256];
	uintptr_t start = (uintptr_t)arena;
	ohci_dma_configure(start, start + sizeof(arena));
	void *a = ohci_dma_alloc(17, 16);
	void *b = ohci_dma_alloc(256, 256);
	assert(a && b && !((uintptr_t)a & 15) && !((uintptr_t)b & 255));
	assert(ohci_dma_contains((uintptr_t)a, 17));
	assert(!ohci_dma_contains(start - 1, 1));
	assert(ohci_dma_contains(start + sizeof(arena) - 1, 1));
	assert(!ohci_dma_contains(start + sizeof(arena) - 1, 2));
	assert(!ohci_dma_alloc(sizeof(arena), 16));
}

int main(void) {
	test_edid();
	test_hid();
	test_usb();
	puts("RK356x host unit tests passed");
	return 0;
}
