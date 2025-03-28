#include <string.h>
#include <stdint.h>
#include "os.h"

int demo_futo();

void int_handler() {
	puts("Handling an interrupt");
}

int c_entry() {
	uart_init();
	reset_timer0();
	setup_cru();

	sys_soc_setup();

	bmp_clear();
	puts("------------- RK Bootloader -------------");
	puts("Copyright FUTO 2023");

	//sys_turn_on_screen();

	return 0;
}
