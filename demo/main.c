#include <stdint.h>
#include "../src/firmware.h"
#include "main.h"

void itoa(uint64_t n, char *buffer, int base) {
	int i = 12;

	char hex[] = "0123456789ABCDEF";

	// Backwards read into buffer
	do {
		buffer[i] = hex[n % base];
		i--;
		n = n / base;
	} while(n > 0);

	// Shift the chars down
	int j = 0;
	while (++i < 13) {
		buffer[j++] = buffer[i];
	}

	buffer[j] = '\0';
}

char *strcat(char *dst, const char *src) {
	char *d = dst;
	while (*d) d++;
	while ((*d++ = *src++));
	return dst;
}

char *strcpy(char *dst, const char *src) {
	char *d = dst;
	while ((*d++ = *src++));
	return dst;
}

int puts(const char *s) {
	fw_handler(FU_PRINT_STR, (uintptr_t)s, 0, 0);
	fw_handler(FU_PRINT_STR, (uintptr_t)"\r\n", 0, 0);
	return 0;
}

int entry(uintptr_t firmware_function, uintptr_t _start) {
	puts("Hello World from Payload");

	char buf2[8];
	char buf1[64];

	uint64_t el;
	asm volatile("mrs %0, CurrentEl" : "=r"(el));

	if (!bmp_setup()) {
		bmp_clear();
		bmp_print("Hello, World from bmp\r\n");

		strcpy(buf1, "Hello From EL");
		itoa(el >> 2, buf2, 10);
		strcat(buf1, buf2);
		strcat(buf1, "\r\n");
		bmp_print(buf1);
	}

	for (int i = 0x10000000; i != 0; i--) {
		__asm__("nop");
	}

	puts("Turning off...");
	fw_handler(PSCI_SYSTEM_OFF, 0, 0, 0);
	return 0;
}
