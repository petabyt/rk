#include <rk.h>
#include <os.h>
#include "rk3399.h"

volatile void *plat_get_uart_base(void) {
	return (volatile void *)UART2_START; 
}

uint32_t *plat_get_framebuffer(void) {
	return (uint32_t *)FB_ADDR;
}

void enable_uart(void) {
	grf_gpio_iomux_set(IOMUX_4C, 9, 8, 0b1); // gpio4c4_sel = uart2dbgc_sout
	grf_gpio_iomux_set(IOMUX_4C, 7, 6, 0b1); // gpio4c3_sel = uart2dbgc_sin

	rk_clr_set_bits((uint32_t *)(GRF_BASE + 0xe21c), 11, 10, 0b10);
}

void sys_shutdown(void) {
	// RK3399 only
	gpio_set_dir(1, 6, 1);
	gpio_set_pin(1, 6, 1);
}

void blink_loop( void) {
	gpio_set_pin(0, RK_PIN_B3, 0);
	gpio_set_dir(0, RK_PIN_A2, 1);

	while (1) {
		gpio_set_dir(0, RK_PIN_A2, 1);
		gpio_set_pin(0, RK_PIN_A2, 1);

		nop_sleep();

		gpio_set_pin(0, RK_PIN_A2, 0);

		nop_sleep();
	}
}
