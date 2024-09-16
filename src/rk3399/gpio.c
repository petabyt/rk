// IO, register, and pin related functions
#include "pins.h"
#include "io.h"
#include "os.h"
#include "stdint.h"

// Set the function of one of the GPIOs from the GRF pin multiplexing
void grf_gpio_iomux_set(int gpio, int bit1, int bit2, int func) {
	struct GrfGpioIomux *regs = (struct GrfGpioIomux *)GRF_IOMUX_BASE;
	rk_clr_set_bits(&regs->regs[gpio], bit1, bit2, func);
}

void pmugrf_gpio_iomux_set(int gpio, int bit1, int bit2, int func) {
	uint32_t *regs = (uint32_t *)PMUGRF_BASE;
	rk_clr_set_bits(&regs[gpio], bit1, bit2, func);
}

// page 680
struct Gpio {
	uint32_t data_reg;
	uint32_t data_dir;
	uint8_t res0[0x2c-4];
	uint32_t int_enable_reg;
	uint32_t int_mask_reg;
	uint32_t int_type_level;
	uint32_t int_polarity;
	uint32_t int_status;
	uint32_t int_raw_status;
	uint32_t debounce;
	uint32_t porta_eoi;
	uint32_t ext_porta;
	uint8_t res1[8];
	uint32_t ls_sync;
};

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

void gpio_set_dir(int gpio, int pin, int bit) {
	volatile struct Gpio *g = gpio_get(gpio);
	if (bit) {
		g->data_dir |= (1 << pin);
	} else {
		g->data_dir &= ~(1 << pin);
	}
}

int gpio_get_pin(int gpio, int pin) {
	volatile struct Gpio *g = gpio_get(gpio);
	return (g->ext_porta & (1 << pin)) != 0;
}

void gpio_pin_mask_int(int gpio, int pin) {
	volatile struct Gpio *g = gpio_get(gpio);
	g->int_mask_reg |= (1 << pin);
	g->int_enable_reg |= (1 << pin);
}

void gpio_set_pin(int gpio, int pin, int bit) {
	volatile struct Gpio *g = gpio_get(gpio);
	if (bit) {
		g->data_reg |= (1 << pin);
	} else {
		g->data_reg &= ~(1 << pin);
	}
}

