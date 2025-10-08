#include <string.h>
#include <stdint.h>
#include "os.h"

int c_entry(void) {
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
