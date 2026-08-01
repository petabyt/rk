#include <stdint.h>
#include "../src/firmware.h"
#include "main.h"

static int bmp_status = 1;

static uint64_t timer_count(void) {
	uint64_t value;
	asm volatile("mrs %0, cntpct_el0" : "=r"(value));
	return value;
}

static uint64_t timer_frequency(void) {
	uint64_t value;
	asm volatile("mrs %0, cntfrq_el0" : "=r"(value));
	return value;
}

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

void exception_handler(uintptr_t a0, uintptr_t sp) {
	puts("Exception triggered");
	uint64_t esr_el3, elr_el3;
	asm volatile("mrs %0, esr_el2" : "=r" (esr_el3));
	asm volatile("mrs %0, elr_el2" : "=r" (elr_el3));
	char buffer[100];
	char buffer2[16];
	strcpy(buffer, "esr_el2: ");
	itoa(esr_el3, buffer2, 16);
	strcat(buffer, buffer2);
	strcat(buffer, "\n");
	puts(buffer);
	while (1);
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

	char buf1[96];
	char buf2[20];

	uint64_t el;
	asm volatile("mrs %0, CurrentEl" : "=r"(el));

	bmp_status = bmp_setup();
	struct FuScreenList *screens = (struct FuScreenList *)
		fw_handler(FU_GET_SCREEN_LIST, 0, 0, 0);

	struct FuDeviceInfo *info = (struct FuDeviceInfo *)fw_handler(FU_GET_DEVICE_INFO, 0, 0, 0);

	if (!bmp_status) bmp_clear();

	strcpy(buf1, "FUTO Bootloader payload binary, running on '");
	strcat(buf1, info->product);
	strcat(buf1, "'");
	puts(buf1);
	if (screens != (struct FuScreenList *)FU_ERROR && screens->length) {
		strcpy(buf1, "Video mode: ");
		itoa(screens->screens[0].width, buf2, 10);
		strcat(buf1, buf2);
		strcat(buf1, "x");
		itoa(screens->screens[0].height, buf2, 10);
		strcat(buf1, buf2);
		puts(buf1);
	} else {
		puts("Video mode: headless");
	}

	strcpy(buf1, "We are in EL");
	itoa(el >> 2, buf2, 10);
	strcat(buf1, buf2);
	puts(buf1);

	uint32_t *dtb = (uint32_t *)fw_handler(FU_GET_DTB, 0, 0, 0);
	if (dtb == (uint32_t *)FU_ERROR) {
		puts("No DTB available");
	} else {
		if (dtb[0] == 0x0edfe0dd0) {
			puts("Valid DTB present");
		} else {
			puts("Invalid DTB");
		}
	}

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

	puts("Keyboard input (30 seconds):");
	uint64_t start = timer_count();
	uint64_t duration = timer_frequency() * 30;
	while (timer_count() - start < duration) {
		if (!fw_handler(FU_POLL_CHAR, 0, 0, 0))
			continue;
		char c = (char)fw_handler(FU_GET_CHAR, 0, 0, 0);
		if (!c)
			continue;
		if (c == '\b') {
			fw_handler(FU_PRINT_CHAR, '\b', 0, 0);
			fw_handler(FU_PRINT_CHAR, ' ', 0, 0);
			fw_handler(FU_PRINT_CHAR, '\b', 0, 0);
		} else {
			fw_handler(FU_PRINT_CHAR, c, 0, 0);
		}
		if (!bmp_status)
			bmp_print_char(c);
		if (c == '\r') {
			fw_handler(FU_PRINT_CHAR, '\n', 0, 0);
			if (!bmp_status) bmp_print_char('\n');
		}
	}

	puts("Turning off...");
	fw_handler(PSCI_SYSTEM_OFF, 0, 0, 0);
	return 0;
}
