// Bare minimum UART code
#include <stdint.h>
#include "main.h"

struct Uart {
	uint32_t rbr; // recieve buffer reg
	uint32_t ier; // interrupt enable register
	uint32_t fcr; // FIFO control register
	uint32_t lcr; // line control register
	uint32_t mcr; // modem control register
	uint32_t lsr; // line status register
	uint32_t msr; // modem status register
	uint32_t scr; // scratchpad register
	uint32_t reserved1[(0x30 - 0x20) / 4];
	uint32_t srbr[(0x70 - 0x30) / 4];
	uint32_t far;
	uint32_t tfr;
	uint32_t rfw;
	uint32_t usr; // uart status register
	uint32_t tfl;
	uint32_t rfl;
	uint32_t srr; // software reset register
	uint32_t srts;
	uint32_t sbcr;
	uint32_t sdmam;
	uint32_t sfe;
	uint32_t srt;
	uint32_t stet;
	uint32_t htx;
	uint32_t dmasa;
	uint32_t reserved2[(0xf4 - 0xac) / 4];
	uint32_t cpr;
	uint32_t ucv;
	uint32_t ctr;
};

void uart_init(unsigned int baud_rate) {
	volatile struct Uart *uart = (volatile struct Uart *)plat_get_uart_base();

	// Set all reset register bits
	uart->srr = (1 << 0) | (1 << 1) | (1 << 2);

	// Disable interrupts
	uart->ier = 1;

	// Modem control register
	uart->mcr = 3;

	uart->fcr = 7;

	// Line control register
	uart->lcr = 0x83;

	// Close latch
	uart->lcr |= (1 << 7);

	if (baud_rate == 115200) {
		uart->rbr = 0xd;
	} else if (baud_rate == 1500000) {
		uart->rbr = 0x1;
	}

	// Clear latch
	uart->lcr &= ~(1 << 7);
	
	uart->sfe = 1;
	uart->srt = 1;
	uart->stet = 1;
}

void uart_chr(int c) {
	volatile struct Uart *uart = (volatile struct Uart *)plat_get_uart_base();

	// Wait until FIFO transmit not full
	while (!(uart->lsr & (1 << 5)));

	uart->rbr = (uint32_t)c;
}

int uart_get(void) {
	volatile struct Uart *uart = (volatile struct Uart *)plat_get_uart_base();
	while (!(uart->usr & (1 << 3)));
	return uart->rbr;
}

__attribute__((weak))
int getchar(void) {
	return uart_get();
}

__attribute__((weak))
int putchar(int c) {
	uart_chr((char)c);
	return 0;
}
