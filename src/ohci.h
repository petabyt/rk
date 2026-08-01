#ifndef RK_OHCI_H
#define RK_OHCI_H

#include <stdint.h>

struct UsbBootKeyboard {
	uint8_t interface_number;
	uint8_t endpoint;
	uint8_t interval;
	uint16_t max_packet;
};

void ohci_dma_configure(uintptr_t start, uintptr_t end);
void *ohci_dma_alloc(unsigned long size, unsigned long alignment);
int ohci_dma_contains(uintptr_t address, unsigned long size);
int usb_find_boot_keyboard(const uint8_t *config, unsigned int length,
		struct UsbBootKeyboard *keyboard);
int ohci_add_controller(uintptr_t base);
int setup_ohci(uintptr_t base);
void ohci_poll_all(void);

#endif
