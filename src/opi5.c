#include <string.h>
#include <stdint.h>
#include <rk3588.h>
#include "main.h"
#include "firmware.h"

static uint8_t *shared_mem;

uint64_t plat_process_firmware_call(uint64_t p1, uint64_t p2, uint64_t p3, uint64_t p4) {
	uint8_t *buf = shared_mem;
	struct FuScreenList *screens = (void *)(buf); buf += 0x40;
	struct FuDeviceInfo *info = (void *)(buf); buf += 0x40;
	switch (p1) {
	case FU_GET_SCREEN_LIST:
		screens->length = 0;
		return (uintptr_t)screens;
	case FU_GET_DEVICE_INFO:
		strcpy(info->vendor, "Orange Pi");
		strcpy(info->product, "5");
		return (uintptr_t)info;
	}

	return FU_ERROR;
}

int c_entry(void) {
	uint8_t buffer[1000];
	shared_mem = buffer;

	char ttbl_buffer[0x2000];
	plat_setup_mmu(ttbl_buffer);
	puts("Hello, World");
	return 0;
}
