#include <string.h>
#include <stdint.h>
#include <rk3588.h>
#include "main.h"
#include "firmware.h"
#include "rk3588/genbook.dtb.out.h"

static uint8_t *dtb_addr;
static uint8_t *shared_mem;

uint64_t plat_process_firmware_call(uint64_t p1, uint64_t p2, uint64_t p3, uint64_t p4) {
	uint8_t *buf = shared_mem;
	struct FuScreenList *screens = (void *)(buf); buf += 0x40;
	struct FuDeviceInfo *info = (void *)(buf); buf += 0x40;
	struct FuMemoryMap *map = (void *)(buf);
	switch (p1) {
	case FU_GET_SCREEN_LIST:
		screens->length = 1;
		screens->screens[0].framebuffer_addr = plat_get_framebuffer();
		screens->screens[0].width = 1920;
		screens->screens[0].height = 1080;
		screens->screens[0].stride = 1920 * 4;
		screens->screens[0].id = 0;
		return (uintptr_t)screens;
	case FU_GET_MEM_CHUNK:
		plat_get_mem_map(map);
		return (uintptr_t)&map->items[2];
	case FU_GET_MEM_MAP:
		plat_get_mem_map(map);
		return (uintptr_t)map;
	case FU_GET_DEVICE_INFO:
		strcpy(info->vendor, "Cool-Pi");
		strcpy(info->product, "Genbook");
		return (uintptr_t)info;
	case FU_GET_DTB:
		return (uintptr_t)dtb_addr;
	}

	return FU_ERROR;
}

int c_entry(void) {
	char ttbl_buffer[0x2000];
	plat_setup_mmu(ttbl_buffer);

	// GPIO0_C4_1V8_D lcd pwr on
	gpio_set_dir(0, RK_PIN_C4, 1); gpio_set_pin(0, RK_PIN_C4, 1);
	// GPIO4_A3_3V3_D blen
	gpio_set_dir(4, RK_PIN_A3, 1); gpio_set_pin(4, RK_PIN_A3, 1);
	// typec5v_pwren
	gpio_set_dir(0, RK_PIN_A0, 1); gpio_set_pin(0, RK_PIN_A0, 1);
    // vcc5v0_host1_en
	gpio_set_dir(1, RK_PIN_D5, 1); gpio_set_pin(1, RK_PIN_D5, 1);
    // keyboard_en
	gpio_set_dir(1, RK_PIN_A7, 1); gpio_set_pin(1, RK_PIN_A7, 1);

	// Setup backlight
	// Set gpio4c1 function to pwm6
	volatile struct BusIoc *busioc = (volatile struct BusIoc *)BUS_IOC;
	rk_clr_set_bits(&busioc->gpio4c_iomux_sel_l, 7, 4, 0xb);
	pwm_setup_continuous(6, 0x400, 0x100);

	// cw2015 battery
	rk3588_set_pin_func(1, RK_PIN_A3, 9);
	rk3588_set_pin_func(1, RK_PIN_A2, 9);

	rk3588_sgrf_init();
	rk3588_init_power_domains();

	rk3588_setup_video_edp1(plat_get_framebuffer(), 1920, 1080);

	uint8_t buffer[1000];
	shared_mem = buffer;

	uint8_t dtb_data_copy[sizeof(dtb_data)];
	memcpy(dtb_data_copy, dtb_data, sizeof(dtb_data));
	dtb_addr = dtb_data_copy;

	jump_to_payload();

	return 0;
}
