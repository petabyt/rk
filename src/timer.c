// Basic timer code
#include "io.h"
#include "os.h"

void reset_timer0() {
	struct Timer *t = (struct Timer *)TIMER0_START;
	t->timers[0].ctrl_reg = 0x0;
	t->timers[0].load_count0 = 0xffffffff;
	t->timers[0].load_count1 = 0xffffffff;
	t->timers[0].load_count2 = 0x0;
	t->timers[0].load_count3 = 0x0;
	t->timers[0].ctrl_reg = 0x1;

	t->timers[0].int_status = 0x1;
}

uint32_t timer0_get_val() {
	struct Timer *t = (struct Timer *)TIMER0_START;
	// return low
	return t->timers[0].curr_value0;
}

void usleep(int ticks) {
	uint32_t then = timer0_get_val();

	ticks *= 24; // 24mhz

	// TODO: this will overflow -> get stuck
	while (1) {
		uint32_t now = timer0_get_val();
		if (now < then) {
			return;
		}
		if (now - then > (uint32_t)ticks) {
			return;
		}			
	}
}

void msleep(int ms) {
	usleep(ms * 1000);
}
