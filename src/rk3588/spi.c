#include "rk3588.h"

int do_transfer(uintptr_t base) {
	//struct Spi *spi = (struct Spi *)base;
	// TODO: Finish driver

	return 0;
}

int rkspi_init(uintptr_t base) {
	struct Spi *spi = (struct Spi *)base;
	spi->baudr = 0x0;
	spi->imr = 0xff;

	return 0;
}
