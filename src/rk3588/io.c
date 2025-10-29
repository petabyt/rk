#include <main.h>
#include "rk3588.h"

volatile void *plat_get_uart_base(void) {
	return (volatile void *)UART2;
}

uint32_t *plat_get_framebuffer(void) {
	return (uint32_t *)0x0;
}

void plat_shutdown(void) {
	puts("TODO: Shutdown");
	halt();
}
