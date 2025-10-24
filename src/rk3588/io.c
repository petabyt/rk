#include "rk3588.h"

volatile void *plat_get_uart_base(void) {
	return (volatile void *)UART2;
}
