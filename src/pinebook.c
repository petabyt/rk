#include <stdint.h>
#include <rk3399.h>
#include "rk.h"
#include "os.h"

uint64_t plat_process_firmware_call(uint64_t p1, uint64_t p2, uint64_t p3) {
	return 0;
}

int c_entry(void) {
	// 4kb page size
	// 32 bit output address
	uint64_t tcr = 0x3520;

	// attr0: ngnrne device memory
	// attr1: ngnre device memory
	// attr2: NonCacheable
	// attr3: WriteBack_NonTransient_ReadWriteAlloc
	uint64_t mair = 0xFF440400;

	setup_tt_el3(tcr, mair, (uintptr_t)ttb0_base);
	enable_mmu_el3();

	enable_uart();
	uart_init();

	puts("RK3399 bootloader - Copyright FUTO 2023");

	// LCDVCC
	gpio_set_dir(1, RK_PIN_C6, 1);
	gpio_set_pin(1, RK_PIN_C6, 1);

	// LCD_EN
	gpio_set_dir(1, RK_PIN_A0, 1);
	gpio_set_pin(1, RK_PIN_A0, 1);

	// LCD_BL_PWM
	gpio_set_dir(4, RK_PIN_C2, 1);
	gpio_set_pin(4, RK_PIN_C2, 1);

	reset_timer0();
	setup_cru();

	sys_soc_setup();

	rk3399_init_edp(EDP_BASE);
	rk3399_init_vop(VOP_LIT_BASE, FB_ADDR);
	rk3399_enable_edp(EDP_BASE);

	return 0;
}
