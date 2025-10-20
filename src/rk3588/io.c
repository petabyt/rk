#include "rk3588.h"

volatile void *get_uart_base(void) {
	return (volatile void *)UART2;
}
