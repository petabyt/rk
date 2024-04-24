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

void uart_str(char *x) {
	for (int i = 0; x[i] != '\0'; i++) {
		uart_chr(x[i]);
	}
}

// sprintf needs alloc, so this will do for now
void cheap_itoa(uint64_t n, char *buffer) {
	int i = 12;

	char hex[] = "0123456789ABCDEF";

	// Backwards read into buffer
	do {
		buffer[i] = hex[n % 16];
		i--;
		n = n / 16;
	} while(n > 0);

	// Shift the chars down
	int j = 0;
	while (++i < 13) {
		buffer[j++] = buffer[i];
	}

	buffer[j] = '\0';
}

void cheap_memdump(uint8_t *addr, int n) {
	char buffer[16];

	for (int i = 0; i < n; i++) {
		cheap_itoa(addr[i], buffer);
		uart_str(buffer);
		uart_chr(' ');
	}
}

void print_bits(uint64_t ptr) {
	if (ptr == 0) {
		uart_chr('0');
		return;		
	}

	int i = 31;
	while (((ptr >> i) & 1) == 0) i--;

	for (; i != -1; i--) {
		int bit = ((ptr >> i) & 1);
		uart_chr('0' + bit);
	}
}

void debug(char *str, uint64_t reg) {
	uart_str("[UART] ");

	uart_str(str);

	char buffer[32];
	cheap_itoa(reg, buffer);
	uart_str(buffer);

	uart_str(" (0b");

	print_bits(reg);
	uart_chr(')');

	uart_chr('\n');
	uart_chr('\r');
}

int puts(const char *str) {
	uart_str("[UART] ");

	uart_str((char *)str);

	uart_chr('\n');
	uart_chr('\r');	

	return 0;
}
