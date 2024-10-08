// Bare minimum UART code
#include <string.h>
#include <stdint.h>

#include "rk.h"
#include "os.h"

void uart_init() {
	volatile struct Uart *uart = (volatile struct Uart *)get_uart_base();
	enable_uart();

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

	// Set baud rate 115200
	uart->rbr = 0xd;

	// Clear latch
	uart->lcr &= ~(1 << 7);
	
	uart->sfe = 1;
	uart->srt = 1;
	uart->stet = 1;
}

void uart_chr(char c) {
	bmp_print_char(c);

	volatile struct Uart *uart = (volatile struct Uart *)get_uart_base();

	// Wait until FIFO transmit not full
	while (!(uart->lsr & (1 << 5)));

	uart->rbr = (uint32_t)c;
}

int uart_in() {
	volatile struct Uart *uart = (volatile struct Uart *)get_uart_base();
	while (!(uart->usr & (1 << 3)));
	return uart->rbr;
}

int putchar(int c) {
	uart_chr((char)c);
	return 0;
}
