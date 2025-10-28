#include <stdint.h>
#include <string.h>
#include "os.h"

// Rockchip register design: in order to write bit x, bit x + 16 must be 1.
// If bit x is 1 but x + 16 is 0, the write will be denied.
// In this func: rk_clr_set_bits(&a, 5, 0, 0x0); == sets bits [5:0] of ptr a
void rk_clr_set_bits(uint32_t *d, int bit_end, int bit_start, int v) {
	uint32_t temp = 0;

	// Write the requested bits at their location
	temp |= (v << (bit_start));

	// Write the corrosponding bits in the write_bit field (31:16)
	// So we generate a mask of 1s, and shift it over to the corrosponding bits
	temp |= ((1 << (bit_end - bit_start + 1)) - 1) << (16 + bit_start);

	d[0] = temp;
}

void nop_sleep(void) {
	for (int i = 0; i < 300000; i++) {
		asm("nop");
	}
}

void nop_sleep_short(void) {
	for (int i = 0; i < 100000; i++) {
		asm("nop");
	}
}

void halt(void) {
	while (1) {
		asm("wfi");
	}
}

static uintptr_t alloc_base = DUMMY_ALLOC_BASE;
void *malloc(long unsigned int size) {
	void *mem = (void *)alloc_base;
	alloc_base += size;
	return mem;
}
void *memalign(unsigned long alignment, unsigned long size) {
	uintptr_t new = (alloc_base + alignment - 1) & ~(alignment - 1);
	alloc_base = new + size;
	memset((void *)new, 0x0, size);
	return (void *)new;
}
void free(void *x) {
	(void)x;
}

void int_handler(void) {
	puts("Handling an interrupt");
}

uint64_t panic_handler(uint64_t p1, uint64_t p2, uint64_t p3, uint64_t p4) {
	uint64_t esr_el3, elr_el3;
	asm volatile("mrs %0, ESR_EL3" : "=r" (esr_el3));
	asm volatile("mrs %0, elr_el3" : "=r" (elr_el3));

	if (esr_el3 == 0x5E000000) { // smc call
		return process_firmware_call(p1, p2, p3);
	}

	puts("!!!!! Exception !!!!!");
	debug("we are in EL", asm_get_el() >> 2);
	debug("esr_el3: ", esr_el3);
	debug("elr_el3: ", elr_el3);
	debug("instr: ", ((uint32_t *)elr_el3)[0]);
	puts("Please reset the board");
	halt();
	return 0;
}

void fail(char *reason, int code) {
	while (1) {
		debug(reason, code);
		nop_sleep();
	}
}

// Generate aarch64 branch instruction - fairly similar to aarch32
void generate_branch(void *base, void *to, void *buffer) {
	((uint32_t *)buffer)[0] = (((to - base - 8) >> 2) & 0x00ffffff) + 2;
	((uint8_t *)buffer)[3] = 0x17;
}

void generate_call(void *base, void *to, void *buffer) {
	generate_branch(base, to, buffer);
	((uint8_t *)buffer)[3] = 0x97;
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
