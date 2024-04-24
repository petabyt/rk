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

	/*
	 * OHCI defines u16 frame_no, followed by u16 zero pad.
	 * Since some processors can't do 16 bit bus accesses,
	 * portable access must be a 32 bits wide.
	 */
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

// Temporary allocator
uint32_t usb_alloc(int size, int alignment) {
	static uint32_t start = 0xf4000000;
	uint32_t new = (start + alignment - 1) & ~(alignment - 1); // alignment trick
	start = new + size;
	memset((void *)((uintptr_t)new), 0x0, size);
	return new;
}

int interrupt_handler(volatile struct ohci_regs *ohci) {
	uint32_t intr = ohci->intrstatus;
	if (intr & OHCI_INTR_SF) {
		puts("! New frames");
	}
	if (intr & OHCI_INTR_UE) {
		fail("! Unrecoverable error", 0);
	}
	if (intr & OHCI_INTR_RHSC) {
		puts("! Root hub status changed");
	}
	if (intr & OHCI_INTR_WDH) {
		ohci->intrdisable = OHCI_INTR_WDH;
		puts("! Processing finished");
		ohci->intrenable = OHCI_INTR_WDH;
	}

	debug("ed_periodhead: ", ohci->ed_controlhead);
	debug("ed_periodcurrent: ", ohci->ed_controlcurrent);

	volatile struct ohci_hcca *hcca = (volatile struct ohci_hcca *)(uintptr_t)ohci->hcca;
	debug("frame number: ", hcca->frame_no);
	debug("Done: ", hcca->done_head);

//	for (int i = 0; i < 30; i++) {
//		volatile struct ohci_ed *ed = ptr32(hcca->int_table[i]);
//		if (ed == 0) break;
//		debug("Checking ED ", i);
//		volatile struct ohci_td *td = ptr32(ed->hwHeadP);
//		if (td == 0) break;
//		debug("Checking TD: ", td->hwINFO);
//	}

	return 0;
}

uint32_t new_td_out(int length) {
	uint32_t td = usb_alloc(sizeof(struct ohci_td), 16);
	volatile struct ohci_td *td_ = ptr32(td);
	td_->hwINFO = (0b1111 << 28); // ConditionCode
	td_->hwINFO |= (0b01 << 19); // Data out

	uint32_t buffer = usb_alloc(length, 1);
	td_->hwCBP = buffer;
	td_->hwBE = buffer + length;
	td_->hwNextTD = 0x0;

	return td;
}

uint32_t new_td_in(int length) {
	uint32_t td = usb_alloc(sizeof(struct ohci_td), 16);
	volatile struct ohci_td *td_ = ptr32(td);
	td_->hwINFO = (0b1111 << 28); // ConditionCode
	td_->hwINFO |= (0b10 << 19); // Data in

	uint32_t buffer = usb_alloc(length, 1);
	td_->hwCBP = buffer;
	td_->hwBE = buffer + length;
	td_->hwNextTD = 0x0;

	return td;
}

uint32_t new_td_control(void *data, int length) {
	uint32_t td = usb_alloc(sizeof(struct ohci_td), 16);
	volatile struct ohci_td *td_ = ptr32(td);
	td_->hwINFO = (0b1111 << 28); // ConditionCode
	td_->hwINFO |= (0b00 << 19); // Control out

	uint32_t buffer = usb_alloc(length, 1);
	memcpy(ptr32(buffer), data, length);
	td_->hwCBP = buffer;
	td_->hwBE = buffer + length;
	td_->hwNextTD = 0x0;

	return td;
}

uint32_t new_td_dummy() {
	uint32_t td = usb_alloc(sizeof(struct ohci_td), 16);
	return td;
}

static void add_td(uint32_t parent, uint32_t child) {
	volatile struct ohci_td *td_ = ptr32(parent);
	td_->hwNextTD = child;
}

uint32_t new_td_chain(struct ohci_ed *ed, void *data, int length, int in_length) {
	uint32_t td = new_td_control(data, length);
	uint32_t resp = new_td_in(in_length);
	uint32_t dummy = new_td_dummy();
	add_td(td, resp);
	add_td(resp, dummy);

	ed->hwHeadP = td;
	ed->hwTailP = dummy;

	return td;
}

uint32_t new_dummy_ed() {
	uint32_t ed = usb_alloc(sizeof(struct ohci_ed), 16);
	struct ohci_ed *ed_ = (struct ohci_ed *)(uintptr_t)ed;
	ed_->hwINFO |= (1 << 14); // set skip bit
	ed_->hwTailP = usb_alloc(sizeof(struct ohci_ed), 16);
	ed_->hwHeadP = ed_->hwTailP;
	return ed;
}

struct ohci_ed *add_hcca_ed(volatile struct ohci_hcca *hcca, uint32_t info) {
	int i;
	for (i = 0; i < NUM_INTS; i++) {
		if (hcca->int_table[i] == 0x0) break;
	}

	uint32_t ed = usb_alloc(sizeof(struct ohci_ed), 16);
	struct ohci_ed *ed_ = (struct ohci_ed *)(uintptr_t)ed;
	info |= (1 << 14);
	ed_->hwINFO = info;

	// Try and get manufacturer string
	static struct usbreq req;
	req.requesttype = USB_RECIP_DEVICE | USB_DIR_OUT;
	req.request = USB_REQ_GET_DESCRIPTOR;
	req.value = USB_DT_STRING << 8;
	req.index = 0x1;
	req.length = 16;

	// Link this endpoint to the chain
	new_td_chain(ed_, &req, 8, 16);

	hcca->int_table[i] = ed;

	return ed_;
}

static void insert_ed(struct ohci_ed *ed) {
	// Insert ed into queue
}

int setup_ohci(uintptr_t base) {
	volatile struct ohci_regs *ohci = (volatile struct ohci_regs *)base;

	debug("revision: ", ohci->revision);

	if (ohci->revision != 0x10) {
		puts("Driver only for OHCI v1.0");
		return 1;
	}

	ohci->cmdstatus = (1 << 0); // Reset controller
	while (ohci->cmdstatus & (1 << 0)); // Wait until completed

	if (((ohci->control >> 6) & 0b11) != USBSUSPEND) {
		fail("USB not suspended", ohci->control);
	}

	ohci->hcca = usb_alloc(256, 256);
	ohci->ed_controlhead = new_dummy_ed();
	ohci->ed_bulkhead = new_dummy_ed();

	volatile struct ohci_hcca *hcca = (volatile struct ohci_hcca *)(uintptr_t)ohci->hcca;

	for (int i = 0; i < NUM_INTS; i++) {
		hcca->int_table[i] = ohci->ed_controlhead;
	}

	/* TODO:
	MPS 1000 // max packet size (8)
	F   0
	K   0
	S   1 // low speed
	D   00 // get direction from TD
	EN  0000 // endpoint number
	FA  0000010 // USB address TODO: is this wrong
	*/
	struct ohci_ed *test_transaction = add_hcca_ed(hcca, 0x82000);

	// Set our magic numbers
	ohci->fminterval |= 10104 << 16;
	ohci->fminterval ^= (1 << 31); // toggle update bit
	ohci->periodicstart = 0x3E67; // 90% of fminterval, as per spec
	ohci->lsthresh = 0x628;

	// Enable all interrupts
	ohci->intrenable = 0b111111 | (1 << 30) | (1 << 31);
	ohci->intrdisable = 0x0;

	ohci->control = HCFS_USB_OPERATIONAL | HCFS_PLE | HCFS_IE | HCFS_CLE | HCFS_BLE;

	uint32_t num_ports = ohci->desc_a & 0xff;
	debug("Number of ports: ", num_ports);
	for (uint32_t i = 0; i < num_ports; i++) {
		debug("portstatus: ", ohci->portstatus[i]);

		ohci->portstatus[i] |= (1 << 4); // reset port
		while (ohci->portstatus[i] & (1 << 4)); // wait for reset over
		// 'USB Address' for this device is now zero

		if (!(ohci->portstatus[i] & (1 << 0))) {
			debug("Device not connected on port ", i);
			continue;
		}

		ohci->portstatus[i] |= (1 << 1); // Enable port

		debug("portstatus: ", ohci->portstatus[i]);
		uint32_t stat = ohci->portstatus[i];
		debug("Port status: ", stat);
		if (stat & (1 << 9)) puts("Low speed device attached");
		if (stat & (1 << 8)) puts("Port is powered on");
		else puts("Port is powered off");
		if (stat & (1 << 1)) puts("Port is enabled");
		else puts("Port is disabled");
		if (stat & (1 << 0)) puts("Device connected");
		else puts("No device connected");

		interrupt_handler(ohci);
	}

	if (test_transaction->hwINFO & (1 << 14)) {
		puts("Enabling test transaction");
		test_transaction->hwINFO ^= (1 << 14);
	}

	return 0;
}
