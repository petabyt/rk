#include <stdint.h>
#include <string.h>
#include "os.h"

void nop_sleep() {
	for (int i = 0; i < 300000; i++) {
		asm("nop;");
	}
}

void nop_sleep_short() {
	for (int i = 0; i < 100000; i++) {
		asm("nop;");
	}
}

void halt() {
	while (1) {
		asm("wfi;");
	}
}

// :)
uintptr_t crapalloc_base = CRAP_ALLOC_BASE;
void *malloc(long unsigned int size) {
	void *mem = (void *)crapalloc_base;
	crapalloc_base += size;
	return mem;
}
void *memalign(int alignment, int size) {
	uintptr_t new = (crapalloc_base + alignment - 1) & ~(alignment - 1); // alignment trick
	crapalloc_base = new + size;
	memset((void *)new, 0x0, size);
	return (void *)new;
}
void free(void *x) {
	(void)x; // :)
}

void panic_handler(int64_t of) {
	//uart_init();
	puts("!!!!! Exception !!!!!");
	uint64_t esr_el3, elr_el3;
	asm volatile("mrs %0, ESR_EL3" : "=r" (esr_el3));
	asm volatile("mrs %0, elr_el3" : "=r" (elr_el3));
	debug("we are in EL", asm_get_el() >> 2);
	debug("esr_el3: ", esr_el3);
	debug("elr_el3: ", elr_el3);
	debug("instr: ", ((uint32_t *)elr_el3)[0]);
	debug("offset: ", of);
	puts("Please reset the board");
	halt();
}

void fail(char *reason, int code) {
	//gpio_set_pin(0, RK_PIN_B3, 0);
	//gpio_set_dir(0, RK_PIN_A2, 1);

	while (1) {
		//gpio_set_dir(0, RK_PIN_A2, 1);
		//gpio_set_pin(0, RK_PIN_A2, 1);

		debug(reason, code);
		nop_sleep();

		//gpio_set_pin(0, RK_PIN_A2, 0);

		nop_sleep();
	}
}

// Generate aarch64 branch instruction - fairly similar to aarch32
void generate_branch(void *base, void *to, void *buffer) {
	((uint32_t*)buffer)[0] = (((to - base - 8) >> 2) & 0x00ffffff) + 2;
	((uint8_t*)buffer)[3] = 0x17;
}

void generate_call(void *base, void *to, void *buffer) {
	generate_branch(base, to, buffer);
	((uint8_t*)buffer)[3] = 0x97;
}

void *memset(void *dest, int val, long unsigned int len) {
	unsigned char *ptr = dest;
	while (len-- > 0) {
		*ptr++ = val;
	}
	
	return dest;
}

void *memcpy(void *dest, const void *src, long unsigned int count) {
	char* dst8 = (char*)dest;
	char* src8 = (char*)src;
	while (count--) {
		*dst8++ = *src8++;
	}

	return dest;
}

void uart_str(char *x) {
	for (int i = 0; x[i] != '\0'; i++) {
		putchar(x[i]);
	}
}

void itoa(uint64_t n, char *buffer, int base) {
	int i = 12;

	char hex[] = "0123456789ABCDEF";

	// Backwards read into buffer
	do {
		buffer[i] = hex[n % base];
		i--;
		n = n / base;
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
		itoa(addr[i], buffer, 16);
		uart_str(buffer);
		putchar(' ');
	}
}

void print_bits(uint64_t ptr) {
	if (ptr == 0) {
		putchar('0');
		return;		
	}

	int i = 31;
	while (((ptr >> i) & 1) == 0) i--;

	for (; i != -1; i--) {
		int bit = ((ptr >> i) & 1);
		putchar('0' + bit);
	}
}

void debugf(char *buf, char *str, uint64_t reg) {
	while (str[0] != '\0') {
		buf[0] = str[0];
		buf++; str++;
	}
	itoa(reg, buf, 16);	
}

void debug(char *str, uint64_t reg) {
	uart_str("[UART] ");

	uart_str(str);

	char buffer[32];
	itoa(reg, buffer, 16);
	uart_str(buffer);

	uart_str(" (0b");

	print_bits(reg);
	putchar(')');

	putchar('\n');
	putchar('\r');
}

int puts(const char *str) {
	uart_str("[UART] ");

	uart_str((char *)str);

	putchar('\n');
	putchar('\r');	

	return 0;
}
