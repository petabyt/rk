#include <main.h>
#include <firmware.h>
#include "rk3588.h"

void plat_get_mem_map(void *buffer) {
	struct FuMemoryMap *map = buffer;
	map->length = 4;
	map->items[0].flags = FU_MEM_ATTR_RESERVED;
	map->items[0].start_addr = 0x0;
	map->items[0].end_addr = (uintptr_t)_end_of_image;
	map->items[1].flags = FU_MEM_ATTR_PAYLOAD;
	map->items[1].start_addr = (uintptr_t)_end_of_image;
	map->items[1].end_addr = 0x10000000;
	map->items[2].flags = FU_MEM_ATTR_UNUSED;
	map->items[2].start_addr = 0x10000000;
	map->items[2].end_addr = 0xc0000000;
	map->items[3].flags = FU_MEM_ATTR_MMIO;
	map->items[3].start_addr = 0xc0000000;
	map->items[3].end_addr = 0x100000000;
}

volatile void *plat_get_uart_base(void) {
	return (volatile void *)UART2;
}

uintptr_t plat_get_framebuffer(void) {
	return 0xd0000000;
}

void rk3588_setup_mmu(void) {
	uint64_t tcr = 0x351c | (1 << 0x10);
	setup_tt_el3(tcr, mair_reg_base, (uintptr_t)ttb0_base);
	enable_mmu_el3();
}

void plat_reset(void) {
	// turn off some pinebook pins
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
	plat_reset();
}
