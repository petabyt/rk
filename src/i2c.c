#include "io.h"
#include "os.h"

int rk_clock_i2c(int i2c, int hz) {
	struct Cru *cru = (struct Cru *)CRU_BASE;
	rk_clr_set_bits(&cru->clksel_con[61], 4, 0, 0b0);

	return 0;
}

int rk_i2c_test(uintptr_t base) {
	volatile struct RkI2C *i2c = (volatile struct RkI2C *)base;

	debug("rki2c version: ", i2c->con >> 16);

	i2c->con |= 0b1; // i2c module enable

	return 0;
}
