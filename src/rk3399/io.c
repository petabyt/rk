#include <main.h>
#include <firmware.h>
#include "rk3399.h"

struct FuMemoryMap rk3399_map = {
	.length = 1,
	.items = {
		{
			.flags = FU_MEM_ATTR_RESERVED,
			.start_addr = 0x0,
			.end_addr = (uintptr_t)_end_of_image,
		},
		{
			.flags = FU_MEM_ATTR_PAYLOAD,
			.start_addr = (uintptr_t)_end_of_image,
			.end_addr = 0x10000000,
		},
		{
			.flags = FU_MEM_ATTR_UNUSED,
			.start_addr = 0x10000000,
			.end_addr = 0xc0000000,
		},
		{
			.flags = FU_MEM_ATTR_MMIO,
			.start_addr = 0xc0000000,
			.end_addr = 0x100000000,
		},
	}
};

volatile void *plat_get_uart_base(void) {
	return (volatile void *)UART2_START; 
}

uintptr_t plat_get_framebuffer(void) {
	return 0xf7800000;
}

void enable_uart(void) {
	grf_gpio_iomux_set(IOMUX_4C, 9, 8, 0b1); // gpio4c4_sel = uart2dbgc_sout
	grf_gpio_iomux_set(IOMUX_4C, 7, 6, 0b1); // gpio4c3_sel = uart2dbgc_sin

	rk_clr_set_bits((uint32_t *)(GRF_BASE + 0xe21c), 11, 10, 0b10);
}

void plat_reset(void) {
	gpio_set_dir(1, 6, 1);
	gpio_set_pin(1, 6, 1);
}

void plat_shutdown(void) {
	plat_reset();
}
