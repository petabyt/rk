#include "main.h"
#include "rk356x.h"

static inline volatile uint32_t *reg(uintptr_t base, unsigned int offset) {
	return (volatile uint32_t *)(base + offset);
}

void rk356x_enable_uart(void) {
	/* UART2 M0: GPIO0_D0 RX and GPIO0_D1 TX. */
	rk_clr_set_bits(reg(RK356X_PMUGRF, 0x18), 2, 0, 1);
	rk_clr_set_bits(reg(RK356X_PMUGRF, 0x18), 6, 4, 1);
	/* Select the M0 rather than M1 pin group. */
	rk_clr_set_bits(reg(RK356X_GRF, 0x30c), 11, 10, 0);

	/* Select xin24m directly, enable PCLK/SCLK, and release both resets. */
	rk_clr_set_bits(reg(RK356X_CRU, 0x100 + 54 * 4), 13, 12, 2);
	rk_clr_set_bits(reg(RK356X_CRU, 0x300 + 26 * 4), 1, 1, 0);
	rk_clr_set_bits(reg(RK356X_CRU, 0x300 + 28 * 4), 0, 0, 0);
	rk_clr_set_bits(reg(RK356X_CRU, 0x300 + 28 * 4), 3, 3, 0);
	rk_clr_set_bits(reg(RK356X_CRU, 0x400 + 25 * 4), 1, 0, 0);
}

void enable_uart(void) {
	rk356x_enable_uart();
}

int rk356x_enable_vo_domain(void) {
	/* PMU power-domain control: clear PD_VO to request power-up. */
	*reg(RK356X_PMU, 0xa0) = (1U << (16 + 7));
	for (unsigned int i = 0; i < 100000; i++) {
		if (!(*reg(RK356X_PMU, 0x98) & (1U << 7)))
			return 0;
	}
	return -1;
}
