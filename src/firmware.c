// Hands off control to a payload appended to the end of the image
// And passes to it a function that can be used to do stuff with system/screen/serial/etc
#include <string.h>
#include "main.h"
#include "firmware.h"

// Memory shared between EL2/EL3
static uint8_t *shared_mem;

uint64_t smc_handler(uint64_t p1, uint64_t p2, uint64_t p3, uint64_t p4) {
	return process_firmware_call(p1, p2, p3, p4);
}

static void bsod(void) {
	// No font is included in this minimal binary, so just fill the screen with blue
	uint32_t *fb = (void *)plat_get_framebuffer();
	if (fb != NULL) {
		for (int i = 0; i < (1080 * 1920); i++) {
			fb[i] = 0x0000FF;
		}
	}
	halt();
}

uint64_t process_firmware_call(uint64_t p1, uint64_t p2, uint64_t p3, uint64_t p4) {
	switch (p1) {
	case PSCI_VERSION:
		return 0 | (1 << 16); // PSCIv1.0
	case PSCI_SYSTEM_OFF:
		plat_shutdown();
		return 0;
	case PSCI_SYSTEM_RESET:
		plat_reset();
		return 0;
	case PSCI_FEATURES:
		return -1; // NOT_SUPPORTED
	case FU_PRINT_CHAR:
		putchar((char)p2);
		return 0;
	case FU_PRINT_STR: {
		char *s = (char *)(uintptr_t)p2;
		while (*s != '\0') {
			putchar(*s);
			s++;
		}
		} return 0;
	case FU_GET_CHAR:
	case FU_POLL_CHAR:
		return 0;
	}

	uint64_t rc = plat_process_firmware_call(p1, p2, p3, p4);
	if (rc == FU_ERROR) {
		// Return empty device list for all 'get device' calls
		if ((p1 & 0xff000000) == 0xf1000000) {
			struct FuMmioDeviceList *dev = (struct FuMmioDeviceList *)shared_mem;
			dev->length = 0;
			return (uintptr_t)dev;
		}

		debug("Unknown command: ", p1);
	}

	// Clean cache between calls
	dcache_clean(STACK_TOP - STACK_SIZE, STACK_TOP);

	return rc;
}


typedef void entry(uintptr_t call_fn);

void jump_to_payload(void) {
	struct FuPayloadHeader *header = (struct FuPayloadHeader *)_end_of_image;

	if (header->magic != 0x8008135) {
		puts("Bad payload magic");
		bsod();
	}

	if (header->flags & PAYLOAD_FLAG_REQUIRES_RELOCATION) {
		if ((uintptr_t)header != (uintptr_t)header->relocation_addr) {
			memcpy((void *)header->relocation_addr, header, header->img_size);
			dcache_clean(header->relocation_addr, header->relocation_addr + header->img_size);
			header = (struct FuPayloadHeader *)header->relocation_addr;
		}
	}

	uint8_t buffer[500];
	shared_mem = buffer;

	debug("Calling ", (uintptr_t)header->boot_code);

	entry *fn = (entry *)header->boot_code;
	// Memory below 0xa00000 on RK3588 is accessible from EL3 only
	if ((uintptr_t)header->boot_code >= 0xa00000) {
		puts("Dropping down into EL2");
		start_in_el2((uintptr_t)header->boot_code, STACK2_TOP);
	} else {
		puts("Jumping to payload in EL3");
		fn((uintptr_t)process_firmware_call);
	}

	puts("Returned from payload");

	halt();
}

// Jump to a u-boot.bin payload
void jump_to_uboot(uintptr_t text_addr, uint32_t image_size) {
	memcpy((void *)text_addr, (void *)_end_of_image, image_size);
	dcache_clean(text_addr, text_addr + image_size);
	//disable_mmu_el3();
	typedef void c(void);
	puts("Jumping to u-boot");
	c *x = (c *)text_addr;
	x();
}
