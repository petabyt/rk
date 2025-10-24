#include "rk3588.h"

volatile struct Gpio *gpio_get(int gpio) {
	uint32_t ptrs[] = {
		GPIO0_START,
		GPIO1_START,
		GPIO2_START,
		GPIO3_START,
		GPIO4_START,
	};
	return (volatile struct Gpio *)((uintptr_t)ptrs[gpio]);
}

void gen_pin_mask(int pin, int on, uint32_t *l, uint32_t *h) {
	*l = 0;
	*h = 0;
	if (on) {
		if (pin > 0xf) {
			*h = 1 << (pin - 0x10);
			*h |= (1 << pin);
		} else {
			*l = 1 << pin;
			*l |= 1 << (pin + 0x10);
		}
	} else {
		if (pin > 0xf) {
			*h = 1 << pin;
		} else {
			*l = 1 << (pin + 0x10);
		}
	}
}

void gpio_set_dir(int gpio, int pin, int bit) {
	volatile struct Gpio *g = gpio_get(gpio);
	uint32_t l, h;
	gen_pin_mask(pin, bit, &l, &h);
	g->swport_dr_l = l;
	g->swport_dr_h = h;
}

void gpio_set_pin(int gpio, int pin, int bit) {
	volatile struct Gpio *g = gpio_get(gpio);
	uint32_t l, h;
	gen_pin_mask(pin, bit, &l, &h);
	g->swport_ddr_l = l;
	g->swport_ddr_h = h;
}

