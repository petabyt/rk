#include <string.h>
#include <stdint.h>
#include "os.h"

int demo_futo();

void sys_shutdown() {
	gpio_set_dir(1, 6, 1);
	gpio_set_pin(1, 6, 1);
}

void int_handler() {
	
}

int c_entry() {
	uart_init();
	reset_timer0();
	setup_cru();

	sys_soc_setup();

	puts("------------- RK3399 Bootloader -------------");
	puts("Copyright FUTO " __DATE__);

	return 0;
}
