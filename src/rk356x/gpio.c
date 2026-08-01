#include "main.h"
#include "rk356x.h"

struct RkGpio {
	uint32_t dr_l;
	uint32_t dr_h;
	uint32_t ddr_l;
	uint32_t ddr_h;
};

static volatile struct RkGpio *gpio_get(int bank) {
	static const uintptr_t bases[] = {
		0xfdd60000, 0xfe740000, 0xfe750000, 0xfe760000, 0xfe770000,
	};
	if ((unsigned int)bank >= sizeof(bases) / sizeof(bases[0]))
		return 0;
	return (volatile struct RkGpio *)bases[bank];
}

static void gpio_enable(int bank) {
	if (bank == 0) {
		rk_clr_set_bits((volatile void *)(RK356X_PMUCRU + 0x184), 9, 9, 0);
		rk_clr_set_bits((volatile void *)(RK356X_PMUCRU + 0x200), 10, 9, 0);
	} else if ((unsigned int)bank <= 4) {
		unsigned int gate = 2U * (unsigned int)bank;
		unsigned int reset = 344U + 2U * (unsigned int)bank;
		rk_clr_set_bits((volatile void *)(RK356X_CRU + 0x300 + 31 * 4),
			gate, gate, 0);
		rk_clr_set_bits((volatile void *)(RK356X_CRU + 0x400 +
			(reset / 16) * 4), reset % 16 + 1, reset % 16, 0);
	}
}

static void gpio_set_mux(int bank, int pin) {
	uintptr_t base;
	unsigned int group = (unsigned int)pin / 8;
	unsigned int offset;
	unsigned int shift = ((unsigned int)pin % 4) * 4;
	if ((unsigned int)bank > 4 || (unsigned int)pin > 31)
		return;
	base = bank ? RK356X_GRF : RK356X_PMUGRF;
	offset = bank ? ((unsigned int)bank - 1) * 0x20 : 0;
	offset += group * 8 + (((unsigned int)pin & 4) ? 4 : 0);
	rk_clr_set_bits((volatile void *)(base + offset), shift + 3, shift, 0);
}

static void masked_pin_write(volatile uint32_t *low, volatile uint32_t *high,
		int pin, int value) {
	unsigned int bit = (unsigned int)pin & 15;
	uint32_t value_and_mask = (1U << (bit + 16)) | ((value != 0) << bit);
	if (pin < 16)
		*low = value_and_mask;
	else
		*high = value_and_mask;
}

void gpio_set_dir(int gpio, int pin, int bit) {
	volatile struct RkGpio *g = gpio_get(gpio);
	if (g)
		masked_pin_write(&g->ddr_l, &g->ddr_h, pin, bit);
}

void gpio_set_pin(int gpio, int pin, int bit) {
	volatile struct RkGpio *g = gpio_get(gpio);
	if (g)
		masked_pin_write(&g->dr_l, &g->dr_h, pin, bit);
}

int gpio_get_pin(int gpio, int pin) {
	volatile uint32_t *ext_port = (volatile uint32_t *)
		((uintptr_t)gpio_get(gpio) + 0x70);
	return gpio_get(gpio) ? ((*ext_port >> pin) & 1) : 0;
}

void gpio_pin_mask_int(int gpio, int pin) {
	volatile uint32_t *low = (volatile uint32_t *)
		((uintptr_t)gpio_get(gpio) + 0x18);
	if (gpio_get(gpio))
		masked_pin_write(low, low + 1, pin, 1);
}

void rk356x_gpio_output(struct Rk356xGpioPin pin, int asserted) {
	if (!pin.valid)
		return;
	gpio_enable(pin.bank);
	gpio_set_mux(pin.bank, pin.pin);
	gpio_set_pin(pin.bank, pin.pin, asserted == pin.active_high);
	gpio_set_dir(pin.bank, pin.pin, 1);
}
