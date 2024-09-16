#include "io.h"

volatile void *get_gicd_base() {
	return (volatile void *)GICD_BASE;
}

volatile void *get_gicr_base() {
	return (volatile void *)GICR_BASE;
}

volatile void *get_gicc_base() {
	return (volatile void *)GICC_BASE;
}

volatile void *get_uart_base() {
	return (volatile void *)0xfeb50000; 
}

void enable_uart() {
	grf_gpio_iomux_set(IOMUX_4C, 9, 8, 0b1); // gpio4c4_sel = uart2dbgc_sout
	grf_gpio_iomux_set(IOMUX_4C, 7, 6, 0b1); // gpio4c3_sel = uart2dbgc_sin

	rk_clr_set_bits((uint32_t *)(GRF_BASE + 0xe21c), 11, 10, 0b10);
}
