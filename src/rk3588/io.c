#include <main.h>
#include "rk3588.h"

volatile void *plat_get_uart_base(void) {
	return (volatile void *)UART2;
}

uint32_t *plat_get_framebuffer(void) {
	return (uint32_t *)0x0;
}

void rk3588_setup_mmu(void) {
	uint64_t tcr = 0x351c | (1 << 0x10);

	// attr0: ngnrne device memory
	// attr1: ngnre device memory
	// attr2: NonCacheable
	// attr3: WriteBack_NonTransient_ReadWriteAlloc
	uint64_t mair = 0xFF440400;

	setup_tt_el3(tcr, mair, (uintptr_t)ttb0_base);
	enable_mmu_el3();
}

void plat_reboot(void) {
	gpio_set_dir(0, RK_PIN_C4, 0);
	gpio_set_dir(4, RK_PIN_A3, 0);
	gpio_set_dir(0, RK_PIN_A0, 0);
	gpio_set_dir(1, RK_PIN_D5, 0);
	gpio_set_dir(1, RK_PIN_A7, 0);

	gpio_set_pin(0, RK_PIN_C4, 0);
	gpio_set_pin(4, RK_PIN_A3, 0);
	gpio_set_pin(0, RK_PIN_A0, 0);
	gpio_set_pin(1, RK_PIN_D5, 0);
	gpio_set_pin(1, RK_PIN_A7, 0);

	((volatile uint32_t *)0xfd588080)[0] = 0xef08a53c; // force boot into otg maskrom
	((volatile uint32_t *)0xfd58a004)[0] = 0xffff0000; // reboot
	((volatile uint32_t *)0xfd7c0c08)[0] = 0xfdb9; // ???

	halt();
}

void plat_shutdown(void) {
	plat_reboot();
}
