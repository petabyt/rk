#include <stdint.h>
#include <rk3399.h>
#include "main.h"
#include "firmware.h"

void blink_loop(void) {
	gpio_set_pin(0, RK_PIN_B3, 0);
	gpio_set_dir(0, RK_PIN_A2, 1);
	while (1) {
		gpio_set_dir(0, RK_PIN_A2, 1);
		gpio_set_pin(0, RK_PIN_A2, 1);
		nop_sleep();
		gpio_set_pin(0, RK_PIN_A2, 0);
		nop_sleep();
	}
}

struct FuScreenList screens;

uint64_t plat_process_firmware_call(uint64_t p1, uint64_t p2, uint64_t p3) {
	switch (p1) {
	case FU_GET_SCREEN_LIST:
		screens.length = 1;
		screens.screens[0].framebuffer_addr = FB_ADDR;
		screens.screens[0].width = 1920;
		screens.screens[0].height = 1080;
		screens.screens[0].stride = 1920 * 4;
		return (uintptr_t)&screens;
	}

	return 0;
}

int c_entry(void) {
	asm_set_cnt_freq(24000000); // 24mhz

	// 4kb page size
	// 32 bit output address
	uint64_t tcr = 0x3520;
	setup_tt_el3(tcr, mair_reg_base, (uintptr_t)ttb0_base);
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

	// SDMMC0_PWR_H
	gpio_set_dir(0, RK_PIN_A1, 1);
	gpio_set_pin(0, RK_PIN_A1, 1);
	// gpio4b5_sel = sdmmc_cmd
	// gpio4b4_sel = sdmmc_clkout
	// gpio4b3_sel = sdmmc_data3
	// gpio4b2_sel = sdmmc_data2
	// gpio4b1_sel = sdmmc_data1
	// gpio4b0_sel = sdmmc_data0
	grf_gpio_iomux_set(IOMUX_4B, 11, 0, 0b10101010101);

	reset_timer0();
	setup_cru();

	sys_soc_setup();

	// edp_ref_clk_sel = 1 (select clock)
	rk_clr_set_bits((uint32_t *)(GRF_BASE + GRF_SOC_CON25), 11, 11, 1);

	// Set edp_lcdc_sel = vop little
	rk_clr_set_bits((uint32_t *)(GRF_BASE + GRF_SOC_CON20), 5, 5, 1);

	edp_init(EDP_BASE);
	rk3399_init_vop(VOP_LIT_BASE, FB_ADDR);
	edp_enable(EDP_BASE, 10, 2);

	jump_to_payload();

	return 0;
}
