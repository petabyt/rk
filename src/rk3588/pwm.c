#include <main.h>
#include "rk3588.h"

struct __attribute__((packed)) Pwm {
	struct __attribute__((packed)) PwmChannel {
		uint32_t cnt;
		uint32_t period_hpr;
		uint32_t duty_lpr;
		uint32_t ctrl;
	}channels[4];
	uint32_t inststs;
	uint32_t int_en;
	uint32_t res3[2];
	uint32_t fifo_ctrl;
	uint32_t fifo_inststs;
	uint32_t fifo_toutthr;
	uint32_t version_id;
	uint32_t fifo;
	uint32_t res0[7];
	uint32_t pwrmatch_ctrl;
	uint32_t pwrmatch_lpre;
	uint32_t pwrmatch_hpre;
	uint32_t pwrmatch_ld;
	uint32_t pwrmatch_hd_zero;
	uint32_t pwrmatch_hd_on;
	uint32_t pwrmatch_value0;
	uint32_t pwrmatch_value1;
	uint32_t pwrmatch_value2;
	uint32_t pwrmatch_value3;
	uint32_t pwrmatch_value4;
	uint32_t pwrmatch_value5;
	uint32_t pwrmatch_value6;
	uint32_t pwrmatch_value7;
	uint32_t pwrmatch_value8;
	uint32_t pwrmatch_value9;
	uint8_t res1[0xc];
	uint32_t pwm3_pwrcaptur_e_value;
	uint32_t channel_io_ctrl;
};

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
