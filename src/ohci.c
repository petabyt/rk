#include <stdint.h>
#include <string.h>
#include "main.h"
#include "usb.h"

// TODO: Remove hardcoded magic address
static uintptr_t noncache_memory_start = 0xf0000000;

// HostControllerFunctionalState
#define USBRESET 0b00
#define USBRESUME 0b01
#define USBOPERATIONAL 0b10
#define USBSUSPEND 0b11

struct __attribute__((packed)) OhciHC {
	uint32_t revision;
	uint32_t control;
	uint32_t cmdstatus;
	uint32_t intrstatus;
	uint32_t intrenable;
	uint32_t intrdisable;
	uint32_t hcca;
	uint32_t ed_periodcurrent;
	uint32_t ed_controlhead;
	uint32_t ed_controlcurrent;
	uint32_t ed_bulkhead;
	uint32_t ed_bulkcurrent;
	uint32_t donehead;
	uint32_t fminterval;
	uint32_t fmremaining;
	uint32_t fmnumber;
	uint32_t periodicstart;
	uint32_t lsthresh;
	uint32_t desc_a;
	uint32_t desc_b;
	uint32_t status;
	uint32_t portstatus [15];
};

struct __attribute__((packed)) OhciHcca {
	uint32_t int_table[32];
	uint32_t frame_no;
	uint32_t done_head;
};

struct __attribute__((packed)) OhciTD {
	uint32_t info;
	uint32_t cbp; // current buffer pointer
	uint32_t next;
	uint32_t be; // buffer end
};

struct __attribute__((packed)) OhciED {
	uint32_t info;
	uint32_t tail;
	uint32_t head;
	uint32_t next;
};

#define OHCI_CTRL_CBSR	(3 << 0)	/* control/bulk service ratio */
#define OHCI_CTRL_PLE	(1 << 2)	/* periodic list enable */
#define OHCI_CTRL_IE	(1 << 3)	/* isochronous enable */
#define OHCI_CTRL_CLE	(1 << 4)	/* control list enable */
#define OHCI_CTRL_BLE	(1 << 5)	/* bulk list enable */
#define OHCI_CTRL_HCFS	(3 << 6)	/* host controller functional state */
#define OHCI_CTRL_IR	(1 << 8)	/* interrupt routing */
#define OHCI_CTRL_RWC	(1 << 9)	/* remote wakeup connected */
#define OHCI_CTRL_RWE	(1 << 10)	/* remote wakeup enable */

/* pre-shifted values for HCFS */
#define OHCI_USB_RESET	(0 << 6)
#define OHCI_USB_RESUME	(1 << 6)
#define OHCI_USB_OPER	(2 << 6)
#define OHCI_USB_SUSPEND (3 << 6)

#define HCFS_USB_OPERATIONAL (USBOPERATIONAL << 6)
#define HCFS_PLE (1 << 2)
#define HCFS_IE (1 << 3)
#define HCFS_CLE (1 << 4)
#define HCFS_BLE (1 << 5)

#define OHCI_INTR_SO	(1 << 0)	/* scheduling overrun */
#define OHCI_INTR_WDH	(1 << 1)	/* writeback of done_head */
#define OHCI_INTR_SF	(1 << 2)	/* start frame */
#define OHCI_INTR_RD	(1 << 3)	/* resume detect */
#define OHCI_INTR_UE	(1 << 4)	/* unrecoverable error */
#define OHCI_INTR_FNO	(1 << 5)	/* frame number overflow */
#define OHCI_INTR_RHSC	(1 << 6)	/* root hub status change */
#define OHCI_INTR_OC	(1 << 30)	/* ownership change */
#define OHCI_INTR_MIE	(1 << 31)	/* master interrupt enable */

static inline void *ptr32(uint32_t x) {
	return (void *)(uintptr_t)x;
}

uint32_t usb_alloc(int size, int alignment) {
	uint32_t new = (noncache_memory_start + alignment - 1) & ~(alignment - 1); // alignment trick
	noncache_memory_start = new + size;
	memset((void *)((uintptr_t)new), 0x0, size);
	return new;
}

int interrupt_handler(volatile struct OhciHC *ohci) {
	uint32_t intr = ohci->intrstatus;

	if (intr & OHCI_INTR_UE) {
		puts("! Unrecoverable error");
		abort();
	}
	if (intr & OHCI_INTR_RHSC) {
		puts("! Root hub status changed");
	}
	if (intr & OHCI_INTR_WDH) {
		ohci->intrdisable = OHCI_INTR_WDH;

		volatile struct OhciHcca *hcca = (volatile struct OhciHcca *)(uintptr_t)ohci->hcca;
		debug("Donehead: ", hcca->done_head);

		struct OhciTD *td = (struct OhciTD *)(uintptr_t)hcca->done_head;
		if (((td->info >> 24) & 0b11) != 0) { // check data toggle bits (T)
			debug("Successful transaction: ", td->info);
		}

		puts("! Processing finished");
		ohci->intrenable = OHCI_INTR_WDH;

		ohci->intrstatus = intr;

		return 1;
	}

	ohci->intrstatus = intr;

	return 0;
}

uint32_t new_td_out(int length, void *data, uint32_t next) {
	uint32_t td = usb_alloc(sizeof(struct OhciTD), 16);
	volatile struct OhciTD *td_ = ptr32(td);
	td_->info = (0b1111 << 28) // ConditionCode not accessed
		| (0b00 << 24) // toggle 0b01
		| (0b01 << 19); // OUT, to endpoint

	td_->cbp = (uint32_t)(uintptr_t)data;
	td_->be = td_->cbp + length - 1;
	td_->next = next;

	return td;
}

uint32_t new_td_in(int length, void *data, uint32_t next) {
	uint32_t td = usb_alloc(sizeof(struct OhciTD), 16);
	volatile struct OhciTD *td_ = ptr32(td);
	td_->info = (0b1111 << 28) // ConditionCode not accessed
		| (0b10 << 19) // IN, from endpoint
		| (0b00 << 24); // toggle 0b11

	td_->cbp = (uint32_t)(uintptr_t)data;
	td_->be = td_->cbp + length;
	td_->next = next;

	return td;
}

uint32_t new_td_control(void *data, int length, uint32_t next) {
	uint32_t td = usb_alloc(sizeof(struct OhciTD), 16);
	volatile struct OhciTD *td_ = ptr32(td);
	td_->info = (0b1111 << 28) // ConditionCode not accessed
		| (0b00 << 24) // toggle 0b01
		| (0b00 << 19); // SETUP, to endpoint

	uint32_t buffer = usb_alloc(length, 1);
	memcpy(ptr32(buffer), data, length);
	td_->cbp = (uint32_t)(uintptr_t)data;
	td_->be = td_->cbp + length - 1;
	td_->next = next;

	return td;
}

static void print_td(struct OhciTD *td, uint32_t tail) {
	debug("TD at ", (uintptr_t)td);
	debug("TD ", td->info);
	debug("TD ", td->cbp);
	debug("TD ", td->next);
	debug("TD ", td->be);
	if ((uint32_t)(uintptr_t)td == tail) {
		return;
	}

	struct OhciTD *td2 = (struct OhciTD *)(uintptr_t)td->next;
	print_td(td2, tail);
}

uint32_t new_ed(uint32_t info, uint32_t head, uint32_t tail) {
	uint32_t ed = usb_alloc(sizeof(struct OhciED), 16);
	volatile struct OhciED *ed_ = ptr32(ed);
	ed_->info = info;
	ed_->head = head;
	ed_->tail = tail;
	ed_->next = 0x0;
	return ed;
}

uint32_t new_dummy_td() {
	uint32_t td = usb_alloc(sizeof(struct OhciTD), 16);
	return td;
}

uint32_t control_request(volatile struct OhciHC *ohci, uint32_t dev, struct UsbRequest *req, void *buffer, int length) {
	uint32_t td3 = new_dummy_td(); // we need an empty dummy packet as tail
	uint32_t td2 = new_td_in(length, buffer, td3); // data in packet
	uint32_t td1 = new_td_control(req, length, td2); // data out (SETUP) packet

	uint32_t ed = new_ed(dev | (1 << 0x0d) | (8 << 0x10), td1, td3);

	ohci->ed_controlhead = ed;

	ohci->control |= 1 << 4;
	ohci->cmdstatus = 1 << 1;

	msleep(10);

	while (!interrupt_handler(ohci));

	ohci->control |= 1 << 4;

	return 0;
}

int setup_ohci(uintptr_t base) {
	volatile struct OhciHC *ohci = (volatile struct OhciHC *)base;

	debug("revision: ", ohci->revision);

	if (ohci->revision != 0x10) {
		puts("Driver only for OHCI v1.0");
		return 1;
	}

	ohci->intrdisable = 1 << 31; // disable interrupts

	ohci->control = 0;
	ohci->cmdstatus = (1 << 0); // Reset controller
	while (ohci->cmdstatus & (1 << 0)); // Wait until completed

	if (((ohci->control >> 6) & 0b11) != USBSUSPEND) {
		debug("USB not suspended", ohci->control);
		abort();
	}

	uint32_t hcca32 = usb_alloc(256, 256);
	memset((void *)(uintptr_t)hcca32, 0, 256);
	ohci->ed_controlhead = 0x0;
	ohci->ed_bulkhead = 0x0;
	ohci->hcca = hcca32;

	// Set our magic numbers
	ohci->fminterval = (((6 * (11999 - 210)) / 7) << 16) | 11999;
	ohci->fminterval ^= (1 << 31);
	ohci->periodicstart = (11999 * 9) / 10;
	ohci->lsthresh = 0x628;

	// Enable all interrupts
	ohci->intrdisable = 0b1111111 | (1 << 30) | (1 << 31);
	ohci->intrstatus = 0b1111111 | (1 << 30);
	ohci->intrenable = 0b1111111 | (1 << 30);

	ohci->control = HCFS_USB_OPERATIONAL | OHCI_CTRL_PLE | OHCI_CTRL_IE | 0b11;

	uint32_t num_ports = ohci->desc_a & 0xff;
	debug("Number of ports: ", num_ports);
	for (uint32_t i = 0; i < num_ports; i++) {
		debug("portstatus: ", ohci->portstatus[i]);

		ohci->portstatus[i] = 1 << 8;
		ohci->portstatus[i] = 1 << 16;

		ohci->portstatus[i] = (1 << 4); // reset port
		while (ohci->portstatus[i] & (1 << 4)); // wait for reset over
		// 'USB Address' for this device is now zero

		if (!(ohci->portstatus[i] & (1 << 0))) {
			debug("Device not connected on port ", i);
			continue;
		}

		uint32_t stat = ohci->portstatus[i];
		if (stat & (1 << 1)) puts("Port is enabled");
		else puts("Port is disabled");
		if (stat & (1 << 9)) puts("Low speed device attached");
		if (stat & (1 << 8)) puts("Port is powered on");
		else puts("Port is powered off");
		if (stat & (1 << 0)) puts("Device connected");
		else puts("No device connected");

		// Our first request will be SET_ADDRESS
		struct UsbRequest *req = (void *)(uintptr_t)usb_alloc(8, 16);
		req->requesttype = 0x0;
		req->request = USB_REQ_SET_ADDRESS;
		req->value = 0x1;
		req->index = 0x0;
		req->length = 0x0;

		control_request(ohci, 0, req, NULL, 0);
	}

	return 0;
}
