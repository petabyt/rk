#include <stdint.h>
#include <string.h>
#include "os.h"
#include "usb.h"

#pragma pack(push, 1)

// HostControllerFunctionalState
#define USBRESET 0b00
#define USBRESUME 0b01
#define USBOPERATIONAL 0b10
#define USBSUSPEND 0b11

// structs copied from tinyusb
struct ohci_regs {
	/* control and status registers (section 7.1) */
	uint32_t	revision;
	uint32_t	control;
	uint32_t	cmdstatus;
	uint32_t	intrstatus;
	uint32_t	intrenable;
	uint32_t	intrdisable;

	/* memory pointers (section 7.2) */
	uint32_t	hcca;
	uint32_t	ed_periodcurrent;
	uint32_t	ed_controlhead;
	uint32_t	ed_controlcurrent;
	uint32_t	ed_bulkhead;
	uint32_t	ed_bulkcurrent;
	uint32_t	donehead;

	/* frame counters (section 7.3) */
	uint32_t	fminterval;
	uint32_t	fmremaining;
	uint32_t	fmnumber;
	uint32_t	periodicstart;
	uint32_t	lsthresh;

	/* Root hub ports (section 7.4) */
	uint32_t	desc_a;
	uint32_t	desc_b;
	uint32_t	status;
#define MAX_ROOT_PORTS	15	/* maximum OHCI root hub ports (RH_A_NDP) */
	uint32_t	portstatus [MAX_ROOT_PORTS];
};

struct ohci_hcca {
#define NUM_INTS 32
	uint32_t	int_table [NUM_INTS];	/* periodic schedule */
	uint32_t	frame_no;		/* current frame number */
	uint32_t	done_head;		/* info returned for an interrupt */
	uint8_t	reserved_for_hc [116];
	uint8_t	what [4];		/* spec only identifies 252 bytes :) */
};

struct ohci_td {
	uint32_t hwINFO;
	uint32_t hwCBP;		/* Current Buffer Pointer */
	uint32_t hwNextTD;		/* Next TD Pointer */
	uint32_t hwBE;		/* Memory Buffer End Pointer */
};

struct ohci_ed {
	uint32_t hwINFO;
	uint32_t hwTailP;
	uint32_t hwHeadP;
	uint32_t hwNextED;
};

struct usbreq {
	uint8_t	requesttype;
	uint8_t	request;
	uint16_t	value;
	uint16_t	index;
	uint16_t	length;
};

#pragma pack(pop)

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

static inline uint32_t phy32(void *x) {
	return (uint32_t)(uintptr_t)x;
}

// Allocate memory in nGnRnE attributed memory
uint32_t usb_alloc(int size, int alignment) {
	static uint32_t start = 0xf0000000;
	uint32_t new = (start + alignment - 1) & ~(alignment - 1); // alignment trick
	start = new + size;
	memset((void *)((uintptr_t)new), 0x0, size);
	return new;
}

int interrupt_handler(volatile struct ohci_regs *ohci) {
	uint32_t intr = ohci->intrstatus;

	if (intr & OHCI_INTR_UE) {
		fail("! Unrecoverable error", 0);
	}
	if (intr & OHCI_INTR_RHSC) {
		puts("! Root hub status changed");
	}
	if (intr & OHCI_INTR_WDH) {
		ohci->intrdisable = OHCI_INTR_WDH;

		volatile struct ohci_hcca *hcca = (volatile struct ohci_hcca *)(uintptr_t)ohci->hcca;
		debug("Donehead: ", hcca->done_head);

		struct ohci_td *td = (struct ohci_td *)(uintptr_t)hcca->done_head;
		if (((td->hwINFO >> 24) & 0b11) != 0) { // check data toggle bits (T)
			debug("Successful transaction: ", td->hwINFO);
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
	uint32_t td = usb_alloc(sizeof(struct ohci_td), 16);
	volatile struct ohci_td *td_ = ptr32(td);
	td_->hwINFO = (0b1111 << 28) // ConditionCode not accessed
		| (0b00 << 24) // toggle 0b01
		| (0b01 << 19); // OUT, to endpoint

	td_->hwCBP = (uint32_t)(uintptr_t)data;
	td_->hwBE = td_->hwCBP + length - 1;
	td_->hwNextTD = next;

	return td;
}

uint32_t new_td_in(int length, void *data, uint32_t next) {
	uint32_t td = usb_alloc(sizeof(struct ohci_td), 16);
	volatile struct ohci_td *td_ = ptr32(td);
	td_->hwINFO = (0b1111 << 28) // ConditionCode not accessed
		| (0b10 << 19) // IN, from endpoint
		| (0b00 << 24); // toggle 0b11

	td_->hwCBP = (uint32_t)(uintptr_t)data;
	td_->hwBE = td_->hwCBP + length;
	td_->hwNextTD = next;

	return td;
}

uint32_t new_td_control(void *data, int length, uint32_t next) {
	uint32_t td = usb_alloc(sizeof(struct ohci_td), 16);
	volatile struct ohci_td *td_ = ptr32(td);
	td_->hwINFO = (0b1111 << 28) // ConditionCode not accessed
		| (0b00 << 24) // toggle 0b01
		| (0b00 << 19); // SETUP, to endpoint

	uint32_t buffer = usb_alloc(length, 1);
	memcpy(ptr32(buffer), data, length);
	td_->hwCBP = (uint32_t)(uintptr_t)data;
	td_->hwBE = td_->hwCBP + length - 1;
	td_->hwNextTD = next;

	return td;
}

static void print_td(struct ohci_td *td, uint32_t tail) {
	debug("TD at ", (uintptr_t)td);
	debug("TD ", td->hwINFO);
	debug("TD ", td->hwCBP);
	debug("TD ", td->hwNextTD);
	debug("TD ", td->hwBE);
	if ((uint32_t)(uintptr_t)td == tail) {
		return;
	}

	struct ohci_td *td2 = (struct ohci_td *)(uintptr_t)td->hwNextTD;
	print_td(td2, tail);
}

uint32_t new_ed(uint32_t info, uint32_t head, uint32_t tail) {
	uint32_t ed = usb_alloc(sizeof(struct ohci_ed), 16);
	volatile struct ohci_ed *ed_ = ptr32(ed);
	ed_->hwINFO = info;
	ed_->hwHeadP = head;
	ed_->hwTailP = tail;
	ed_->hwNextED = 0x0;
	return ed;
}

uint32_t new_dummy_td() {
	uint32_t td = usb_alloc(sizeof(struct ohci_td), 16);
	return td;
}

uint32_t control_request(volatile struct ohci_regs *ohci, uint32_t dev, struct usbreq *req, void *buffer, int length) {
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
	volatile struct ohci_regs *ohci = (volatile struct ohci_regs *)base;

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
		fail("USB not suspended", ohci->control);
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
		struct usbreq *req = (void *)(uintptr_t)usb_alloc(8, 16);
		req->requesttype = 0x0;
		req->request = USB_REQ_SET_ADDRESS;
		req->value = 0x1;
		req->index = 0x0;
		req->length = 0x0;

		control_request(ohci, 0, req, NULL, 0);
	}

	return 0;
}
