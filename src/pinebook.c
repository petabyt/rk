#include <string.h>
#include <stdint.h>
#include <rk3399.h>
#include "rk.h"
#include "os.h"

int c_entry(void) {
	enable_uart();
	uart_init();

	reset_timer0();
	setup_cru();

	sys_soc_setup();

	bmp_clear();
	puts("------------- RK Bootloader -------------");
	puts("Copyright FUTO 2023");

	sys_turn_on_screen();

	return 0;
}
