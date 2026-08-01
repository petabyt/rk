#include "main.h"
#include "input.h"
#include "ohci.h"
#include "rk356x.h"

static void set_reset(unsigned int id, int asserted) {
	rk_clr_set_bits((volatile void *)(RK356X_CRU + 0x400 + (id / 16) * 4),
		id % 16, id % 16, asserted);
}

static void usb2phy1_enable(void) {
	volatile uint32_t *phy = (volatile uint32_t *)0xfe8b0000;
	/* Enable the 480 MHz output and return both UTMI ports to normal mode. */
	phy[0x008 / 4] = (1U << (16 + 4));
	phy[0x000 / 4] = (0x1ffU << 16) | 0x1d1;
	phy[0x004 / 4] = (0x1ffU << 16) | 0x1d1;
}

void rk356x_usb_init(void) {
	puts(rk356x_board->connector_notes);
	/* 200 MHz ACLK, 100 MHz HCLK/PCLK, plus both OHCI companion gates. */
	rk_clr_set_bits((volatile void *)(RK356X_CRU + 0x100 + 32 * 4),
		7, 0, (1 << 4) | (1 << 2) | 1);
	rk_clr_set_bits((volatile void *)(RK356X_CRU + 0x300 + 16 * 4),
		2, 0, 0);
	rk_clr_set_bits((volatile void *)(RK356X_CRU + 0x300 + 16 * 4),
		15, 12, 0);
	/* xin24m reference for USB2PHY1. */
	rk_clr_set_bits((volatile void *)(RK356X_PMUCRU + 0x180 + 2 * 4),
		2, 2, 0);
	for (unsigned int id = 224; id <= 233; id++)
		set_reset(id, 1);
	set_reset(459, 1);
	for (unsigned int id = 467; id <= 469; id++)
		set_reset(id, 1);
	usleep(10);
	for (unsigned int id = 224; id <= 233; id++)
		set_reset(id, 0);
	set_reset(459, 0);
	for (unsigned int id = 467; id <= 469; id++)
		set_reset(id, 0);
	usb2phy1_enable();
	msleep(2);

	ohci_dma_configure(RK356X_DMA_START, RK356X_DMA_LIMIT);
	if (rk356x_board->ohci_mask & 1) {
		if (ohci_add_controller(RK356X_OHCI0)) puts("OHCI0 unavailable");
		else puts("OHCI0 ready");
	}
	if (rk356x_board->ohci_mask & 2) {
		if (ohci_add_controller(RK356X_OHCI1)) puts("OHCI1 unavailable");
		else puts("OHCI1 ready");
	}
	input_set_poller(ohci_poll_all);
	ohci_poll_all();
}
