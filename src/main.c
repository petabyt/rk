#include <string.h>
#include <stdint.h>
#include "pins.h"
#include "io.h"
#include "os.h"

int demo_futo();

int c_entry() {
	uint32_t ticks = timer0_get_val();

	// Enable USB 5v
	gpio_set_dir(1, RK_PIN_B5, 1);
	gpio_set_pin(1, RK_PIN_B5, 1);

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
