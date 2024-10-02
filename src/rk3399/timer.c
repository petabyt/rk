// Basic timer code
#include <os.h>
#include "io.h"

volatile struct Timer *rk3399_get_timer(int t) {
	if (t > 5) {
		return (volatile struct Timer *)(TIMER6_START + ((uintptr_t)t * 0x20));
	}
	return (volatile struct Timer *)(TIMER0_START + ((uintptr_t)t * 0x20));
}

int timer_check_int(int t) {
	volatile struct Timer *reg = rk3399_get_timer(t);
	uint32_t s = reg->int_status;
	reg->int_status = 1;
	return s;
}

void reset_timer(int t, uint64_t start, uint64_t limit) {
	volatile struct Timer *reg = rk3399_get_timer(t);
	reg->ctrl_reg = 0x0;
	reg->load_count0 = (limit & 0xffffffff);
	reg->load_count1 = (limit >> 32);
	reg->load_count2 = (start & 0xffffffff);
	reg->load_count3 = (start >> 32);
	reg->ctrl_reg = 0b101;
	reg->int_status = 0x1;
}

void reset_timer0() {
	reset_timer(0, 0x0, 0xffffffff);
}

uint32_t timer_get32(int t) {
	volatile struct Timer *reg = rk3399_get_timer(t);
	return reg->curr_value0;
}

uint32_t timer0_get_val() {
	struct Timer *t = (struct Timer *)TIMER0_START;
	return t->curr_value0; // return low
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
