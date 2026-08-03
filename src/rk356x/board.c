#include <string.h>
#include "main.h"
#include "firmware.h"
#include "input.h"
#include "ohci.h"
#include "rk356x.h"

const struct Rk356xBoard *rk356x_board;
static void *dtb_addr;

extern uint64_t rk356x_ram_size(void);

void rk356x_set_dtb(const void *data, unsigned int size) {
	if (size > RK356X_SHARED - RK356X_DTB)
		size = RK356X_SHARED - RK356X_DTB;
	memcpy((void *)RK356X_DTB, data, size);
	dtb_addr = (void *)RK356X_DTB;
}

static struct FuMemoryMapItem *largest_free(struct FuMemoryMap *map) {
	struct FuMemoryMapItem *best = 0;
	for (uint32_t i = 0; i < map->length; i++) {
		if (!(map->items[i].flags & FU_MEM_ATTR_UNUSED))
			continue;
		if (!best || map->items[i].end_addr - map->items[i].start_addr >
			best->end_addr - best->start_addr)
			best = &map->items[i];
	}
	return best;
}

uint64_t plat_process_firmware_call(uint64_t p1, uint64_t p2,
		uint64_t p3, uint64_t p4) {
	uint8_t *shared = (uint8_t *)RK356X_SHARED;
	struct FuScreenList *screens = (void *)shared;
	struct FuDeviceInfo *info = (void *)(shared + 0x80);
	struct FuMemoryMap *map = (void *)(shared + 0x100);
	(void)p2; (void)p3; (void)p4;
	switch (p1) {
	case FU_GET_SCREEN_LIST:
		memset(screens, 0, 0x80);
		screens->type = FU_SCREEN_XRGB8888;
		if (rk356x_video.active) {
			screens->length = 1;
			screens->screens[0].framebuffer_addr = RK356X_FB_START;
			screens->screens[0].width = rk356x_video.mode.hactive;
			screens->screens[0].height = rk356x_video.mode.vactive;
			screens->screens[0].stride = rk356x_video.stride;
		}
		return (uintptr_t)screens;
	case FU_GET_MEM_CHUNK:
		plat_get_mem_map(map);
		return (uintptr_t)largest_free(map);
	case FU_GET_MEM_MAP:
		plat_get_mem_map(map);
		return (uintptr_t)map;
	case FU_GET_DEVICE_INFO:
		memset(info, 0, sizeof(*info));
		strcpy(info->vendor, rk356x_board->vendor);
		strcpy(info->product, rk356x_board->product);
		return (uintptr_t)info;
	case FU_GET_DTB:
		return (uintptr_t)dtb_addr;
	default:
		return FU_ERROR;
	}
}

int rk356x_board_entry(const struct Rk356xBoard *board,
		const void *dtb, unsigned int dtb_size) {
	rk356x_board = board;
	asm_set_cnt_freq(24000000);
	enable_uart();
	uart_init(1500000);
	puts("RK356x bare-metal firmware");
	puts(board->name);
	puts(board->soc);

	rk356x_setup_security();
	plat_setup_mmu(0);
	debug("DDR bytes: ", rk356x_ram_size());
	rk356x_set_dtb(dtb, dtb_size);
	for (unsigned int i = 0; i < 2; i++)
		rk356x_gpio_output(board->usb_vbus[i], 1);
	for (unsigned int i = 0; i < 2; i++)
		rk356x_gpio_output(board->leds[i], 1);

	if (rk356x_display_init())
		puts(rk356x_video.hpd ? "HDMI: setup failed" : "HDMI: headless");
	else {
		puts(rk356x_video.edid_valid ? "HDMI: EDID mode" :
			"HDMI: 720p60 fallback");
		debug("HDMI width: ", rk356x_video.mode.hactive);
		debug("HDMI height: ", rk356x_video.mode.vactive);
	}

	input_reset();
	rk356x_usb_init();
	puts("Transitioning payload to EL2");
	jump_to_payload();
	return 0;
}
