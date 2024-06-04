// Bare minimum UART code
#include <string.h>
#include <stdint.h>

#include "io.h"
#include "os.h"

void uart_init() {
	volatile struct Uart *uart = (volatile struct Uart *)UART2_START;

	grf_gpio_iomux_set(IOMUX_4C, 7, 6, 0b10); // gpio4c3_sel = uart2dbgc_sin
	grf_gpio_iomux_set(IOMUX_4C, 9, 8, 0b01); // gpio4c4_sel = uart2dbgc_sout

	// Set all reset register bits
	uart->srr = (1 << 0) | (1 << 1) | (1 << 2);

	// Disable interrupts
	uart->ier = 0;

	// Modem control register
	uart->mcr = 0;

	// Line control register
	uart->lcr = 0x3;

	// Close latch
	uart->lcr |= (1 << 7);

	// Set baud rate 1500000
	uart->rbr = 0x1;

	// Clear latch
	uart->lcr &= ~(1 << 7);
	
	uart->sfe = 1;
	uart->srt = 1;
	uart->stet = 1;
}

int switch_uart_bmp = 0;

void uart_chr(char c) {
	bmp_print_char(c);

	volatile struct Uart *uart = (volatile struct Uart *)UART2_START;

	// Wait until FIFO transmit not full
	while (!(uart->usr & (1 << 1)));

	uart->rbr = (uint32_t)c;
}

int putchar(int c) {
	uart_chr((char)c);
	return 0;
}
