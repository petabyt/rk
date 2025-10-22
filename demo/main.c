#include <stdint.h>
#include "../src/firmware.h"
#include "main.h"

func *fw_handler = NULL;

int puts(const char *s) {
	fw_handler(FU_PRINT_STR, (uintptr_t)s, 0, 0);
	fw_handler(FU_PRINT_STR, (uintptr_t)"\r\n", 0, 0);
	return 0;
}

int entry(uintptr_t firmware_function, uintptr_t _start) {
	fw_handler = (func *)firmware_function;
	puts("Hello, World");

	if (!bmp_setup()) {
		bmp_clear();
		bmp_print("Hello, World from BMP");
	}

	fw_handler(PSCI_SYSTEM_OFF, 0, 0, 0);
	return 0;
}
