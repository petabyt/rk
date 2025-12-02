#include <stdint.h>
#include "../src/firmware.h"
#include "main.h"

void exception_handler(uintptr_t a0, uintptr_t sp) {
	puts("Exception triggered");
	while (1);
}

static int bmp_status = 1;

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
	if (!bmp_status) {
		bmp_print(s);
		bmp_print("\r\n");
	}
	return 0;
}

int entry(uintptr_t firmware_function, uintptr_t _start) {
	puts("Hello World from Payload");

	char buf1[64];
	char buf2[20];

	uint64_t el;
	asm volatile("mrs %0, CurrentEl" : "=r"(el));

	bmp_status = bmp_setup();

	struct FuDeviceInfo *info = (struct FuDeviceInfo *)fw_handler(FU_GET_DEVICE_INFO, 0, 0, 0);

	if (!bmp_status) bmp_clear();

	strcpy(buf1, "FUTO Bootloader payload binary, running on '");
	strcat(buf1, info->product);
	strcat(buf1, "'");
	puts(buf1);

	strcpy(buf1, "We are in EL");
	itoa(el >> 2, buf2, 10);
	strcat(buf1, buf2);
	puts(buf1);

	puts("Memory description map:");
	struct FuMemoryMap *map = (struct FuMemoryMap *)fw_handler(FU_GET_MEM_MAP, 0, 0, 0);
	for (unsigned int i = 0; i < map->length; i++) {
		strcpy(buf1, "Range: 0x");
		itoa(map->items[i].start_addr, buf2, 16);
		strcat(buf1, buf2);
		strcat(buf1, "-0x");
		itoa(map->items[i].end_addr, buf2, 16);
		strcat(buf1, buf2);
		puts(buf1);
	}

	for (int i = 0x10000000; i != 0; i--) {
		__asm__("nop");
	}

	puts("Turning off...");
	fw_handler(PSCI_SYSTEM_OFF, 0, 0, 0);
	return 0;
}
