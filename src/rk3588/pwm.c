#include <main.h>
#include "rk3588.h"

volatile struct PwmChannel *get_channel(int idx) {
	uintptr_t list[] = {PWM0, PWM1, PWM2, PWM3};
	volatile struct Pwm *pwm = (volatile struct Pwm *)list[idx / 4];
	return &pwm->channels[idx % 4];	
}

void pwm_setup_continuous(int idx, int period, int duty) {
	volatile struct PwmChannel *c = get_channel(idx);
	c->ctrl &= ~(1);
	c->cnt = 0;
	c->period_hpr = period;
	c->duty_lpr = duty;
	c->ctrl = 0b11; // pwn_en and cont mode
}

void pwm_set_peroid(int idx, int period) {
	volatile struct PwmChannel *c = get_channel(idx);
	c->ctrl &= ~(1);
	c->period_hpr = period;	
	c->ctrl = 0b11;
}

void pwm_set_duty(int idx, int duty) {
	volatile struct PwmChannel *c = get_channel(idx);
	c->ctrl &= ~(1);
	c->duty_lpr = duty;	
	c->ctrl = 0b11;
}
