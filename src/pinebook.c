#include <string.h>
#include <stdint.h>
#include <rk3399.h>
#include "rk.h"
#include "os.h"

int c_entry(void) {
	// 4GB space 4KB granularity
	// Normal Inner and Outer Cacheable.
	setup_tt_el3(0x3520, 0xFF440400, (uintptr_t)ttb0_base);
	enable_mmu_el3();

	enable_uart();
	uart_init();

	// LCDVCC
	gpio_set_dir(1, RK_PIN_C6, 1);
	gpio_set_pin(1, RK_PIN_C6, 1);

	// LCD_EN
	gpio_set_dir(1, RK_PIN_A0, 1);
	gpio_set_pin(1, RK_PIN_A0, 1);

	// LCD_BL_PWM
	gpio_set_dir(4, RK_PIN_C2, 1);
	gpio_set_pin(4, RK_PIN_C2, 1);

	reset_timer0();
	setup_cru();

	sys_soc_setup();

	bmp_clear();
	puts("------------- RK Bootloader -------------");
	puts("Copyright FUTO 2023");

	sys_turn_on_screen();

	return 0;
}
