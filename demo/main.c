#include <stdint.h>
#include "../src/firmware.h"
#include "main.h"

int puts(const char *s) {
	fw_handler(FU_PRINT_STR, (uintptr_t)s, 0, 0);
	fw_handler(FU_PRINT_STR, (uintptr_t)"\r\n", 0, 0);
	return 0;
}

int entry(uintptr_t firmware_function, uintptr_t _start) {
	puts("Hello World from Payload");

	uint64_t el;
	asm volatile("mrs %0, CurrentEl" : "=r"(el));
	if ((el >> 2) == 2) {
		puts("Hello from EL2");
	} else {
		puts("Hello from somewhere else");
	}

	if (!bmp_setup()) {
		bmp_clear();
		bmp_print("Hello, World from bmp");
	}

	for (int i = 0x10000000; i != 0; i--) {
		__asm__("nop");
	}

	puts("Turning off...");
	fw_handler(PSCI_SYSTEM_OFF, 0, 0, 0);
	return 0;
}
