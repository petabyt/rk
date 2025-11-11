#include <string.h>
#include <stdint.h>
#include <rk3588.h>
#include "main.h"
#include "firmware.h"

uint64_t plat_process_firmware_call(uint64_t p1, uint64_t p2, uint64_t p3) {
	return FU_ERROR;
}

int c_entry(void) {
	rk3588_setup_mmu();
	puts("Hello, World");
	return 0;
}
