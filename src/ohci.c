#include <stdint.h>
#include <string.h>
#include "main.h"
#include "usb.h"
#include "ohci.h"
#include "hid_keyboard.h"

#define MAX_CONTROLLERS 2
#define MAX_CONFIG_SIZE 512
#define BIT(x)            (1U << (x))
#define TD_CC            0xf0000000U
#define TD_DATA0         0x02000000U
#define TD_DATA1         0x03000000U
#define TD_ROUND         0x00040000U
#define TD_SETUP         0x00000000U
#define TD_OUT           0x00080000U
#define TD_IN            0x00100000U
#define ED_LOWSPEED      (1U << 13)
#define ED_SKIP          (1U << 14)

#define CTRL_PLE         (1U << 2)
#define CTRL_CLE         (1U << 4)
#define CTRL_HCFS        (3U << 6)
#define CTRL_OPERATIONAL (2U << 6)
#define CMD_RESET        (1U << 0)
#define CMD_CLF          (1U << 1)

struct OhciRegs {
	uint32_t revision, control, command_status, interrupt_status;
	uint32_t interrupt_enable, interrupt_disable, hcca;
	uint32_t period_current_ed, control_head_ed, control_current_ed;
	uint32_t bulk_head_ed, bulk_current_ed, done_head;
	uint32_t frame_interval, frame_remaining, frame_number;
	uint32_t periodic_start, ls_threshold, rh_descriptor_a;
	uint32_t rh_descriptor_b, rh_status, rh_port_status[15];
};

struct OhciHcca {
	uint32_t interrupt_table[32];
	uint16_t frame_number;
	uint16_t pad;
	uint32_t done_head;
	uint8_t reserved[120];
} __attribute__((packed, aligned(256)));

struct OhciEd {
	uint32_t info, tail, head, next;
} __attribute__((aligned(16)));

struct OhciTd {
	uint32_t info, cbp, next, be;
} __attribute__((aligned(16)));

struct ControllerDma {
	struct OhciHcca hcca;
	struct OhciEd control_ed;
	struct OhciTd control_td[4];
	struct OhciEd interrupt_ed;
	struct OhciTd interrupt_td;
	struct OhciTd interrupt_tail;
	uint8_t setup[8] __attribute__((aligned(16)));
	uint8_t data[MAX_CONFIG_SIZE] __attribute__((aligned(16)));
	uint8_t report[8] __attribute__((aligned(16)));
} __attribute__((aligned(256)));

struct Controller {
	volatile struct OhciRegs *regs;
	struct ControllerDma *dma;
	uint64_t deadline;
	uint64_t retry_after;
	uint8_t address;
	uint8_t port;
	uint8_t enum_state;
	uint8_t configured;
	uint8_t low_speed;
	uint8_t control_packet;
	uint8_t endpoint;
	uint8_t interval;
	uint16_t max_packet;
	uint8_t interface_number;
};

enum EnumState {
	ENUM_SCAN,
	ENUM_POWER_WAIT,
	ENUM_RESET_WAIT,
	ENUM_GET_DEVICE,
	ENUM_SET_ADDRESS,
	ENUM_ADDRESS_WAIT,
	ENUM_GET_DEVICE_FULL,
	ENUM_GET_CONFIG_HEAD,
	ENUM_GET_CONFIG,
	ENUM_SET_CONFIG,
	ENUM_SET_PROTOCOL,
	ENUM_SET_IDLE,
};

static uintptr_t dma_start, dma_cursor, dma_end;
static struct Controller controllers[MAX_CONTROLLERS];
static unsigned int controller_count;

void ohci_dma_configure(uintptr_t start, uintptr_t end) {
	dma_start = dma_cursor = start;
	dma_end = end;
	controller_count = 0;
	memset(controllers, 0, sizeof(controllers));
}

void *ohci_dma_alloc(unsigned long size, unsigned long alignment) {
	uintptr_t address;
	if (!alignment || (alignment & (alignment - 1)))
		return 0;
	address = (dma_cursor + alignment - 1) &
		~((uintptr_t)alignment - 1);
	if (address < dma_cursor || address > dma_end || size > dma_end - address)
		return 0;
	dma_cursor = address + size;
	memset((void *)address, 0, size);
	return (void *)address;
}

int ohci_dma_contains(uintptr_t address, unsigned long size) {
	return address >= dma_start && address <= dma_end && size <= dma_end - address;
}

static uint32_t ptr(const void *p) {
	return (uint32_t)(uintptr_t)p;
}

static int wait_clear(volatile uint32_t *value, uint32_t mask,
		unsigned int timeout_ms) {
	uint64_t limit = asm_get_cpu_timer() + (uint64_t)timeout_ms * 1000;
	while (*value & mask)
		if (asm_get_cpu_timer() >= limit)
			return -1;
	return 0;
}

static void td_fill(struct OhciTd *td, uint32_t info, void *buffer,
		unsigned int length, struct OhciTd *next) {
	td->info = TD_CC | info;
	td->cbp = length ? ptr(buffer) : 0;
	td->be = length ? ptr(buffer) + length - 1 : 0;
	td->next = ptr(next);
}

static void control_submit(struct Controller *c, uint8_t address,
		uint16_t max_packet, const struct UsbRequest *request,
		void *buffer, unsigned int length, int low_speed) {
	struct ControllerDma *d = c->dma;
	struct OhciTd *setup = &d->control_td[0];
	struct OhciTd *data = &d->control_td[1];
	struct OhciTd *status = &d->control_td[2];
	struct OhciTd *tail = &d->control_td[3];
	struct OhciTd *after_setup = length ? data : status;
	int input = (request->requesttype & USB_DIR_IN) != 0;

	memset(&d->control_ed, 0, sizeof(d->control_ed));
	memset(d->control_td, 0, sizeof(d->control_td));
	memcpy(d->setup, request, sizeof(*request));
	td_fill(setup, TD_SETUP | TD_DATA0, d->setup, sizeof(*request), after_setup);
	if (length)
		td_fill(data, (input ? TD_IN : TD_OUT) | TD_DATA1 | TD_ROUND,
			buffer, length, status);
	td_fill(status, (input ? TD_OUT : TD_IN) | TD_DATA1, 0, 0, tail);
	d->control_ed.info = address | (low_speed ? ED_LOWSPEED : 0) |
		((uint32_t)max_packet << 16);
	d->control_ed.head = ptr(setup);
	d->control_ed.tail = ptr(tail);
	c->regs->control_head_ed = ptr(&d->control_ed);
	c->regs->control |= CTRL_CLE;
	c->regs->command_status = CMD_CLF;
	c->deadline = asm_get_cpu_timer() + 100000;
}

/* Zero means pending, one means success, and -1 means failed or timed out. */
static int control_status(struct Controller *c) {
	struct OhciTd *status = &c->dma->control_td[2];
	uint32_t condition = status->info >> 28;
	if (condition == 0xf && asm_get_cpu_timer() < c->deadline)
		return 0;
	if (condition == 0xf)
		condition = 1;
	c->dma->control_ed.info |= ED_SKIP;
	return condition == 0 ? 1 : -1;
}

static void request_submit(struct Controller *c, uint8_t address, uint8_t type,
		uint8_t command, uint16_t value, uint16_t index, void *buffer,
		uint16_t length, uint16_t max_packet, int low_speed) {
	struct UsbRequest req = {
		.requesttype = type, .request = command, .value = value,
		.index = index, .length = length,
	};
	control_submit(c, address, max_packet, &req, buffer, length, low_speed);
}

int usb_find_boot_keyboard(const uint8_t *config, unsigned int length,
		struct UsbBootKeyboard *keyboard) {
	int candidate = 0;
	memset(keyboard, 0, sizeof(*keyboard));
	for (unsigned int offset = 0; offset + 2 <= length;) {
		uint8_t dlen = config[offset];
		uint8_t type = config[offset + 1];
		if (dlen < 2 || offset + dlen > length)
			return -1;
		if (type == USB_DT_INTERFACE && dlen >= 9) {
			candidate = config[offset + 3] == 0 &&
				config[offset + 5] == 3 &&
				config[offset + 6] == 1 && config[offset + 7] == 1;
			if (candidate)
				keyboard->interface_number = config[offset + 2];
		} else if (candidate && type == USB_DT_ENDPOINT && dlen >= 7 &&
			(config[offset + 2] & 0x80) && (config[offset + 3] & 3) == 3) {
			keyboard->endpoint = config[offset + 2] & 0x0f;
			keyboard->max_packet = config[offset + 4] |
				((uint16_t)config[offset + 5] << 8);
			keyboard->interval = config[offset + 6];
			if (keyboard->interval && keyboard->max_packet >= 8 &&
				keyboard->max_packet <= 64)
				return 0;
			return -1;
		}
		offset += dlen;
	}
	return -1;
}

static void periodic_stop(struct Controller *c) {
	c->dma->interrupt_ed.info |= ED_SKIP;
	c->regs->control &= ~CTRL_PLE;
	memset(c->dma->hcca.interrupt_table, 0,
		sizeof(c->dma->hcca.interrupt_table));
	c->configured = 0;
	c->enum_state = ENUM_SCAN;
	c->retry_after = asm_get_cpu_timer() + 500000;
	hid_keyboard_reset();
}

static void enumeration_fail(struct Controller *c) {
	c->dma->control_ed.info |= ED_SKIP;
	c->enum_state = ENUM_SCAN;
	c->address = 0;
	c->retry_after = asm_get_cpu_timer() + 500000;
}

static int periodic_start(struct Controller *c) {
	struct ControllerDma *d = c->dma;
	unsigned int period = 1;
	while (period < c->interval && period < 32)
		period <<= 1;
	memset(d->report, 0, sizeof(d->report));
	memset(&d->interrupt_ed, 0, sizeof(d->interrupt_ed));
	memset(&d->interrupt_td, 0, sizeof(d->interrupt_td));
	memset(&d->interrupt_tail, 0, sizeof(d->interrupt_tail));
	td_fill(&d->interrupt_td, TD_IN | TD_ROUND, d->report, 8,
		&d->interrupt_tail);
	d->interrupt_ed.info = c->address | ((uint32_t)c->endpoint << 7) |
		(2U << 11) | (c->low_speed ? ED_LOWSPEED : 0) |
		((uint32_t)c->max_packet << 16);
	d->interrupt_ed.head = ptr(&d->interrupt_td);
	d->interrupt_ed.tail = ptr(&d->interrupt_tail);
	for (unsigned int i = 0; i < 32; i++)
		d->hcca.interrupt_table[i] = (i & (period - 1)) ? 0 :
			ptr(&d->interrupt_ed);
	c->regs->control |= CTRL_PLE;
	c->configured = 1;
	return 0;
}

/* Advance at most one enumeration transition per call. This keeps FUEFI input
 * polling nonblocking even while a device is attaching or misbehaving. */
static void enumerate_step(struct Controller *c) {
	uint8_t *data = c->dma->data;
	struct UsbBootKeyboard keyboard;
	uint16_t total;
	uint8_t configuration;
	int status;
	uint64_t now = asm_get_cpu_timer();
	volatile uint32_t *port_status;

	if (c->enum_state == ENUM_SCAN) {
		unsigned int ports;
		if (now < c->retry_after)
			return;
		ports = c->regs->rh_descriptor_a & 0xff;
		if (ports > 15) ports = 15;
		for (unsigned int port = 0; port < ports; port++) {
			if (!(c->regs->rh_port_status[port] & BIT(0)))
				continue;
			c->port = (uint8_t)port;
			c->regs->rh_port_status[port] = BIT(8);
			c->deadline = now + 20000;
			c->enum_state = ENUM_POWER_WAIT;
			return;
		}
		return;
	}

	port_status = &c->regs->rh_port_status[c->port];
	if (!(*port_status & BIT(0))) {
		c->enum_state = ENUM_SCAN;
		c->retry_after = 0;
		return;
	}

	switch (c->enum_state) {
	case ENUM_POWER_WAIT:
		if (now < c->deadline) return;
		*port_status = BIT(4);
		c->deadline = now + 100000;
		c->enum_state = ENUM_RESET_WAIT;
		return;
	case ENUM_RESET_WAIT:
		if (*port_status & BIT(4)) {
			if (now >= c->deadline) enumeration_fail(c);
			return;
		}
		*port_status = BIT(20) | BIT(17) | BIT(16);
		if ((*port_status & (BIT(0) | BIT(1))) != (BIT(0) | BIT(1))) {
			enumeration_fail(c);
			return;
		}
		c->low_speed = (*port_status & BIT(9)) != 0;
		c->control_packet = 8;
		request_submit(c, 0, 0x80, USB_REQ_GET_DESCRIPTOR,
			USB_DT_DEVICE << 8, 0, data, 8, 8, c->low_speed);
		c->enum_state = ENUM_GET_DEVICE;
		return;
	case ENUM_GET_DEVICE:
		status = control_status(c);
		if (!status) return;
		if (status < 0 || (data[7] != 8 && data[7] != 16 &&
			data[7] != 32 && data[7] != 64)) {
			enumeration_fail(c);
			return;
		}
		c->control_packet = data[7];
		c->address = (uint8_t)(c - controllers + 1);
		request_submit(c, 0, 0, USB_REQ_SET_ADDRESS, c->address, 0,
			0, 0, c->control_packet, c->low_speed);
		c->enum_state = ENUM_SET_ADDRESS;
		return;
	case ENUM_SET_ADDRESS:
		status = control_status(c);
		if (!status) return;
		if (status < 0) { enumeration_fail(c); return; }
		c->deadline = now + 2000;
		c->enum_state = ENUM_ADDRESS_WAIT;
		return;
	case ENUM_ADDRESS_WAIT:
		if (now < c->deadline) return;
		request_submit(c, c->address, 0x80, USB_REQ_GET_DESCRIPTOR,
			USB_DT_DEVICE << 8, 0, data, 18, c->control_packet,
			c->low_speed);
		c->enum_state = ENUM_GET_DEVICE_FULL;
		return;
	case ENUM_GET_DEVICE_FULL:
		status = control_status(c);
		if (!status) return;
		if (status < 0 || data[0] < 18 || data[1] != USB_DT_DEVICE) {
			enumeration_fail(c);
			return;
		}
		debug("USB VID: ", data[8] | ((uint16_t)data[9] << 8));
		debug("USB PID: ", data[10] | ((uint16_t)data[11] << 8));
		request_submit(c, c->address, 0x80, USB_REQ_GET_DESCRIPTOR,
			USB_DT_CONFIG << 8, 0, data, 9, c->control_packet,
			c->low_speed);
		c->enum_state = ENUM_GET_CONFIG_HEAD;
		return;
	case ENUM_GET_CONFIG_HEAD:
		status = control_status(c);
		if (!status) return;
		total = data[2] | ((uint16_t)data[3] << 8);
		if (status < 0 || total < 9 || total > MAX_CONFIG_SIZE) {
			enumeration_fail(c);
			return;
		}
		request_submit(c, c->address, 0x80, USB_REQ_GET_DESCRIPTOR,
			USB_DT_CONFIG << 8, 0, data, total, c->control_packet,
			c->low_speed);
		c->enum_state = ENUM_GET_CONFIG;
		return;
	case ENUM_GET_CONFIG:
		status = control_status(c);
		if (!status) return;
		total = data[2] | ((uint16_t)data[3] << 8);
		if (status < 0 || total < 9 || total > MAX_CONFIG_SIZE ||
			usb_find_boot_keyboard(data, total, &keyboard)) {
			enumeration_fail(c);
			return;
		}
		configuration = data[5];
		if (!configuration) {
			enumeration_fail(c);
			return;
		}
		c->endpoint = keyboard.endpoint;
		c->interval = keyboard.interval;
		c->max_packet = keyboard.max_packet;
		c->interface_number = keyboard.interface_number;
		request_submit(c, c->address, 0, USB_REQ_SET_CONFIGURATION,
			configuration, 0, 0, 0, c->control_packet, c->low_speed);
		c->enum_state = ENUM_SET_CONFIG;
		return;
	case ENUM_SET_CONFIG:
		status = control_status(c);
		if (!status) return;
		if (status < 0) { enumeration_fail(c); return; }
		request_submit(c, c->address, 0x21, 0x0b, 0,
			c->interface_number, 0, 0, c->control_packet, c->low_speed);
		c->enum_state = ENUM_SET_PROTOCOL;
		return;
	case ENUM_SET_PROTOCOL:
		status = control_status(c);
		if (!status) return;
		if (status < 0) { enumeration_fail(c); return; }
		request_submit(c, c->address, 0x21, 0x0a, 0,
			c->interface_number, 0, 0, c->control_packet, c->low_speed);
		c->enum_state = ENUM_SET_IDLE;
		return;
	case ENUM_SET_IDLE:
		status = control_status(c);
		if (!status) return;
		if (status < 0) { enumeration_fail(c); return; }
		puts("USB HID boot keyboard ready");
		periodic_start(c);
		return;
	default:
		enumeration_fail(c);
	}
}

static void periodic_poll(struct Controller *c) {
	struct ControllerDma *d = c->dma;
	uint32_t cc = d->interrupt_td.info >> 28;
	if (cc == 0xf)
		return;
	if (cc != 0) {
		periodic_stop(c);
		return;
	}
	hid_keyboard_report(d->report);
	d->interrupt_ed.info |= ED_SKIP;
	d->hcca.done_head = 0;
	memset(d->report, 0, sizeof(d->report));
	td_fill(&d->interrupt_td, TD_IN | TD_ROUND, d->report, 8,
		&d->interrupt_tail);
	d->interrupt_ed.head = ptr(&d->interrupt_td) |
		(d->interrupt_ed.head & 2);
	d->interrupt_ed.tail = ptr(&d->interrupt_tail);
	d->interrupt_ed.info &= ~ED_SKIP;
}

void ohci_poll_all(void) {
	for (unsigned int n = 0; n < controller_count; n++) {
		struct Controller *c = &controllers[n];
		if (c->configured) {
			if (!(c->regs->rh_port_status[c->port] & BIT(0)))
				periodic_stop(c);
			else
				periodic_poll(c);
			continue;
		}
		enumerate_step(c);
	}
}

int ohci_add_controller(uintptr_t base) {
	struct Controller *c;
	if (controller_count >= MAX_CONTROLLERS)
		return -1;
	c = &controllers[controller_count];
	memset(c, 0, sizeof(*c));
	c->regs = (volatile struct OhciRegs *)base;
	if ((c->regs->revision & 0xff) != 0x10)
		return -1;
	c->dma = ohci_dma_alloc(sizeof(*c->dma), 256);
	if (!c->dma)
		return -1;
	c->regs->interrupt_disable = 0xffffffff;
	c->regs->control = 0;
	c->regs->command_status = CMD_RESET;
	if (wait_clear(&c->regs->command_status, CMD_RESET, 20))
		return -1;
	c->regs->hcca = ptr(&c->dma->hcca);
	c->regs->control_head_ed = 0;
	c->regs->bulk_head_ed = 0;
	c->regs->frame_interval = (1U << 31) |
		(((6U * (11999U - 210U)) / 7U) << 16) | 11999U;
	c->regs->periodic_start = (0x2edfU * 9) / 10;
	c->regs->ls_threshold = 0x628;
	c->regs->interrupt_status = 0xffffffff;
	c->regs->rh_status = BIT(16); /* Global power. */
	c->regs->control = CTRL_OPERATIONAL | 3;
	controller_count++;
	return 0;
}

int setup_ohci(uintptr_t base) {
	return ohci_add_controller(base);
}
