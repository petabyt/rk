#include <stdint.h>
#include "rk356x.h"

void rk356x_setup_security(void) {
	volatile uint32_t *soc_con4 = (volatile uint32_t *)(RK356X_SGRF + 0x10);
	uint32_t value = *soc_con4;

	/* Match the RK3568 early-boot policy: make eMMC/SDMMC0 transactions NS. */
	value &= ~((3U << 11) | (1U << 4));
	*soc_con4 = value;
}
