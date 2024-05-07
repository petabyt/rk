#include <string.h>
#include <stdint.h>
#include "pins.h"
#include "io.h"
#include "os.h"

int demo_futo();

void sys_shutdown() {
	gpio_set_dir(1, 6, 1);
	gpio_set_pin(1, 6, 1);
}

int c_entry() {
	uint32_t ticks = timer0_get_val();

	uart_init();
	reset_timer0();
	setup_cru();

	sys_soc_setup();
	bmp_clear();

	puts("------------- RK3399 Bootloader -------------");
	puts("Copyright FUTO " __DATE__);
	debug("Boot time: ", ticks / 24 / 1000);

	sys_turn_on_screen();

	puts("Hello, World");

	return 0;
}
