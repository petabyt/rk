//#include <string.h>
#include <stdint.h>
#include "main.h"
#include "rk3588.h"
#include "firmware.h"

static uint8_t *shared_mem;

uint64_t plat_process_firmware_call(uint64_t p1, uint64_t p2, uint64_t p3, uint64_t p4) {
	uint8_t *buf = shared_mem;
	struct FuScreenList *screens = (void *)(buf);
	buf += 0x40;
	struct FuMmioDeviceList *ohci = (void *)(buf);
	buf += 0x40;
	struct FuMmioDeviceList *i2c = (void *)(buf);
	buf += 0x40;
	struct FuMmioGic *gic = (void *)(buf);
	buf += 0x40;
	struct FuDeviceInfo *info = (void *)(buf);
	buf += 0x40;
	struct FuMemoryMap *map = (void *)(buf);
	switch (p1) {
	case FU_GET_SCREEN_LIST:
		screens->type = FU_DEV_TYPE_SCREEN;
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
	case FU_GET_OHCI_LIST:
		ohci->type = FU_DEV_TYPE_MMIO_DEVICE;
		ohci->length = 1;
		ohci->devices[0].address = USB2HOST_0_OHCI;
		ohci->devices[0].n_interrupts = 0;
		return (uintptr_t)ohci;
	case FU_GET_MEM_MAP:
		plat_get_mem_map(map);
		return (uintptr_t)map;
	case FU_GET_GIC:
		gic->type = FU_DEV_TYPE_GIC;
		gic->exists = 0;
		return (uintptr_t)gic;
	case FU_GET_RKI2C_LIST:
		i2c->type = FU_DEV_TYPE_MMIO_DEVICE;
		i2c->length = 1;
		i2c->devices[0].address = I2C4;
		return (uintptr_t)i2c;
	case FU_GET_DEVICE_INFO:
		strcpy(info->vendor, "Orange-Pi");
		strcpy(info->product, "Orange-Pi 5");
		return (uintptr_t)info;
	}

	return FU_ERROR;
}

int c_entry(void) {
	char ttbl_buffer[0x2000];
	plat_setup_mmu(ttbl_buffer);

	rk3588_sgrf_init();
	rk3588_init_power_domains();

	rk3588_set_pin_func(1, RK_PIN_A5, 5);
	rk3588_set_pin_func(3, RK_PIN_D4, 5);
	rk3588_set_pin_func(3, RK_PIN_C7, 5);
	rk3588_set_pin_func(4, RK_PIN_C1, 5);
	rk3588_set_pin_func(4, RK_PIN_B7, 5);
	rk3588_set_pin_func(4, RK_PIN_C0, 5);
	rk3588_setup_video_hdmi0(plat_get_framebuffer(), 1920, 1080);

	uint8_t buffer[1000];
	shared_mem = buffer;

	jump_to_payload();

	return 0;
}
