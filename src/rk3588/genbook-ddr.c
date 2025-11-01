#include <main.h>
#include "rk3588.h"

void ddr_shim(void) {
	// GPIO0_C4_1V8_D lcd pwr on
	gpio_set_dir(0, RK_PIN_C4, 1); gpio_set_pin(0, RK_PIN_C4, 1);
	// GPIO4_A3_3V3_D blen
	gpio_set_dir(4, RK_PIN_A3, 1); gpio_set_pin(4, RK_PIN_A3, 1);
	// typec5v_pwren
	gpio_set_dir(0, RK_PIN_A0, 1); gpio_set_pin(0, RK_PIN_A0, 1);
    // vcc5v0_host1_en
	gpio_set_dir(1, RK_PIN_D5, 1); gpio_set_pin(1, RK_PIN_D5, 1);
    // keyboard_en
	gpio_set_dir(1, RK_PIN_A7, 1); gpio_set_pin(1, RK_PIN_A7, 1);
}
