// Hands off control to a payload appended to the end of the image
// And passes to it a function that can be used to do stuff with system/screen/serial/etc
#include <string.h>
#include "main.h"
#include "firmware.h"

__attribute__((weak))
void int_handler(void) {
	puts("TODO: Implement smc call");
}

static void bsod(void) {
	// No font is included in this minimal binary, so just fill the screen with blue
	uint32_t *fb = plat_get_framebuffer();
	if (fb != NULL) {
		for (int i = 0; i < (1080 * 1920); i++) {
			fb[i] = 0x0000FF;
		}
	}
	halt();
}

uint64_t process_firmware_call(uint64_t p1, uint64_t p2, uint64_t p3) {
	switch (p1) {
	case PSCI_VERSION:
		return 0;
	case PSCI_SYSTEM_OFF:
		plat_shutdown();
		return 0;
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
	default:
		return plat_process_firmware_call(p1, p2, p3);
	}
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
			puts("TODO: Requires relocation");
			// TODO: relocate
			bsod();
		}
	}

	// TODO: Drop down into EL2/EL1. 

	debug("Calling ", (uintptr_t)header->boot_code);

	entry *fn = (entry *)header->boot_code;
	fn((uintptr_t)process_firmware_call);

	puts("Returned from payload");

	halt();
}

// Jumping to u-boot.bin is possible.
void jump_to_uboot(uintptr_t text_addr, uint32_t image_size) {
	memcpy((void *)text_addr, (void *)_end_of_image, image_size);
	dcache_clean(text_addr, image_size);
	disable_mmu_el3();
	typedef void c(void);
	puts("Jumping to u-boot");
	c *x = (c *)text_addr;
	x();
}
