#include <string.h>
#include <stdint.h>
#include "pins.h"
#include "io.h"
#include "os.h"

int demo_futo();

void int_handler() {
	timer_check_int(1);

	uint64_t esr_el3, elr_el3;
	asm volatile("mrs %0, ESR_EL3" : "=r" (esr_el3));
	asm volatile("mrs %0, elr_el3" : "=r" (elr_el3));

	bmp_draw_rect(600, 100, 600 + 100, 100 + 100, 0x0);

	char buffer[64];
	font_print_string(600, 100, "Interrupt!");
	debugf(buffer, "IRQ ID: ", gic_get_int());
	font_print_string(600, 100 + 10, buffer);
	debugf(buffer, "LR: ", elr_el3);
	font_print_string(600, 100 + 20, buffer);
}

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

	debug("asm_get_mpidr: ", asm_get_mpidr());

	gic_init();

	asm_enable_int_groups(0xdead);
	asm_enable_ints();

	//gic_enable_irq(46);
	//gpio_pin_mask_int(0, RK_PIN_A5);

	gic_enable_irq(114);
	reset_timer(1, 0, 0xffffff);

	while (1) {
		puts("wfi...");
		__asm__("wfi");
	}

	setup_ohci(USB2_HOST0_START + 0x20000);

	return 0;
}
