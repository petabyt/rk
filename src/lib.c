#include <stdint.h>
#include <string.h>
#include "os.h"
#include "pins.h"
#include "io.h"

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

void exception(int64_t of, uint64_t lr) {
	uart_init();
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

// Red flashing emergency mode, also print reason over uart
void fail(char *reason, int code) {
	gpio_set_pin(0, RK_PIN_B3, 0);
	gpio_set_dir(0, RK_PIN_A2, 1);

	while (1) {
		gpio_set_dir(0, RK_PIN_A2, 1);
		gpio_set_pin(0, RK_PIN_A2, 1);

		debug(reason, code);
		nop_sleep();

		gpio_set_pin(0, RK_PIN_A2, 0);

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
