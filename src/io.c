// IO, register, and pin related functions
#include "pins.h"
#include "io.h"
#include "os.h"
#include "stdint.h"

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
	return g->ext_porta & (1 << pin) != 0;
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
