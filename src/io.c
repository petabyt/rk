// IO, register, and pin related functions
#include "pins.h"
#include "io.h"
#include "os.h"
#include "stdint.h"

// All these methods are slow and will be optimized in the future
// But it's fine for now

// Rockchip register design: in order to write bit x, bit x + 16 must be 1.
// If bit x + 16 is 0, the write will be denied.
// In this func: rk_clr_set_bits(&a, 5, 0, 0x0); == sets bits [5:0] of ptr a
void rk_clr_set_bits(uint32_t *d, int bit_end, int bit_start, int v) {
	uint32_t temp = 0;

	// Write the requested bits at their location
	temp |= (v << (bit_start));

	// Write the corrosponding bits in the write_bit field (31:16)
	// So we generate a mask of 1s, and shift it over to the corrosponding bits
	temp |= ((1 << (bit_end - bit_start + 1)) - 1) << (16 + bit_start);

	d[0] = temp;
}

// Set the function of one of the GPIOs from the GRF pin multiplexing
void grf_gpio_iomux_set(int gpio, int bit1, int bit2, int func) {
	struct GrfGpioIomux *regs = (struct GrfGpioIomux *)GRF_IOMUX_BASE;
	rk_clr_set_bits(&regs->regs[gpio], bit1, bit2, func);
}

void pmugrf_gpio_iomux_set(int gpio, int bit1, int bit2, int func) {
	uint32_t *regs = (uint32_t *)PMUGRF_BASE;
	rk_clr_set_bits(&regs[gpio], bit1, bit2, func);
}

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
	return g->ext_porta;
}

void gpio_set_pin(int gpio, int pin, int bit) {
	volatile struct Gpio *g = gpio_get(gpio);
	if (bit) {
		g->data_reg |= (1 << pin);
	} else {
		g->data_reg &= ~(1 << pin);
	}
}
