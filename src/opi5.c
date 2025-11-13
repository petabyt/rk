#include <string.h>
#include <stdint.h>
#include <rk3588.h>
#include "main.h"
#include "firmware.h"

uint64_t plat_process_firmware_call(uint64_t p1, uint64_t p2, uint64_t p3) {
	return FU_ERROR;
}

int c_entry(void) {
	char ttbl_buffer[0x2000];
	plat_setup_mmu(ttbl_buffer);
	puts("Hello, World");
	return 0;
}
