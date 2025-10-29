#include "main.h"
#include "firmware.h"

extern char _end_of_image[];

static void bsod(void) {
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

	return -1; // error
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

	debug("Calling ", (uintptr_t)header->boot_code);

	entry *fn = (entry *)header->boot_code;
	fn((uintptr_t)process_firmware_call);

	puts("Returned");

	halt();
}
