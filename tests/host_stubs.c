#include <stdint.h>

/* Hardware-only helpers referenced by dead OHCI paths in the host test link. */
uint64_t asm_get_cpu_timer(void) {
	static uint64_t now;
	return ++now;
}

void msleep(unsigned int ms) {
	(void)ms;
}

void debug(const char *label, uint64_t value) {
	(void)label;
	(void)value;
}
