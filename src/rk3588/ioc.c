#include <main.h>
#include "rk3588.h"

void rk3588_iomux_uart2(void) {
	volatile struct BusIoc *busioc = (volatile struct BusIoc *)BUS_IOC;
	rk_clr_set_bits(&busioc->gpio0b_iomux_sel_h, 7, 4, 10);	
	rk_clr_set_bits(&busioc->gpio0b_iomux_sel_h, 10, 8, 10);	
}

void rk3588_iomux_emmc(void) {
	volatile struct BusIoc *busioc = (volatile struct BusIoc *)BUS_IOC;
	rk_clr_set_bits(&busioc->gpio2a_iomux_sel_l, 15, 0, 1111);	
	rk_clr_set_bits(&busioc->gpio2d_iomux_sel_l, 15, 0, 1111);	
	rk_clr_set_bits(&busioc->gpio2d_iomux_sel_h, 15, 0, 1111);	
}

// hdmi:
//  x  := . ! ! bus_ioc . gpio1a_iomux_sel_h $ ! #Boot/Rockchip !reg_mask(7, 4, 5) // gpio1_a5
//
//   x := . ! ! bus_ioc . gpio3d_iomux_sel_h $ ! #Boot/Rockchip !reg_mask(3, 0, 3) // gpio3_d4
// x   := . ! ! bus_ioc . gpio3c_iomux_sel_h $ ! #Boot/Rockchip !reg_mask(0f, 0c, 5) // gpio3_c7
// x   := . ! ! bus_ioc . gpio3d_iomux_sel_h $ ! #Boot/Rockchip !reg_mask(3, 0, 5) // gpio3_d4
//
//    := . ! ! bus_ioc . gpio4c_iomux_sel_l $ ! #Boot/Rockchip !reg_mask(7, 4, 5) // gpio4_c1
//    := . ! ! bus_ioc . gpio4b_iomux_sel_h $ ! #Boot/Rockchip !reg_mask(0f, 0c, 5) // gpio4_b7
//    := . ! ! bus_ioc . gpio4c_iomux_sel_l $ ! #Boot/Rockchip !reg_mask(3, 0, 5) // gpio4_c0

void rk3588_set_pin_func(int bank, int pin, int func) {
	volatile struct BusIoc *busioc = (volatile struct BusIoc *)BUS_IOC;
	volatile struct Pmu2Ioc *pmu2ioc = (volatile struct Pmu2Ioc *)PMU2_IOC;
	if (bank == 0) {
		if (pin == RK_PIN_B5) rk_clr_set_bits(&busioc->gpio0b_iomux_sel_h, 7, 4, func);
		if (pin == RK_PIN_B6) rk_clr_set_bits(&busioc->gpio0b_iomux_sel_h, 10, 8, func);
		if (pin == RK_PIN_C0) rk_clr_set_bits(&busioc->gpio4c_iomux_sel_l, 3, 0, func);
		if (pin == RK_PIN_C1) rk_clr_set_bits(&busioc->gpio4c_iomux_sel_l, 7, 4, func);
		if (pin == RK_PIN_C2) rk_clr_set_bits(&busioc->gpio4c_iomux_sel_l, 11, 8, func);
		if (pin == RK_PIN_C2) rk_clr_set_bits(&busioc->gpio4c_iomux_sel_l, 15, 12, func);
		if (pin == RK_PIN_D0) rk_clr_set_bits(&busioc->gpio0d_iomux_sel_l, 3, 0, func);
		if (pin == RK_PIN_D1) rk_clr_set_bits(&busioc->gpio0d_iomux_sel_l, 7, 4, func);
		if (pin == RK_PIN_D4) rk_clr_set_bits(&busioc->gpio0d_iomux_sel_h, 3, 0, func);
		if (pin == RK_PIN_D5) {
			// Rockchip is stupid for doing this BS
			if (func > 2) {
				rk_clr_set_bits(&pmu2ioc->gpio0d_iomux_sel_h, 7, 4, 8);
				rk_clr_set_bits(&busioc->gpio0d_iomux_sel_h, 7, 4, func);
			} else {
				rk_clr_set_bits(&pmu2ioc->gpio0d_iomux_sel_h, 7, 4, func);				
			}
		}
	}
	if (bank == 1) {
		if (pin == RK_PIN_A2) rk_clr_set_bits(&busioc->gpio1a_iomux_sel_l, 11, 8, func);
		if (pin == RK_PIN_A3) rk_clr_set_bits(&busioc->gpio1a_iomux_sel_l, 15, 12, func);
		if (pin == RK_PIN_A5) rk_clr_set_bits(&busioc->gpio1a_iomux_sel_h, 7, 4, func);
		if (pin == RK_PIN_A6) rk_clr_set_bits(&busioc->gpio1a_iomux_sel_h, 11, 8, func);
	}
	if (bank == 3) {
		if (pin == RK_PIN_B7) rk_clr_set_bits(&busioc->gpio4b_iomux_sel_h, 15, 12, func);
		if (pin == RK_PIN_C0) rk_clr_set_bits(&busioc->gpio4c_iomux_sel_l, 3, 0, func);
		if (pin == RK_PIN_C1) rk_clr_set_bits(&busioc->gpio4c_iomux_sel_l, 7, 4, func);
		if (pin == RK_PIN_C7) rk_clr_set_bits(&busioc->gpio3c_iomux_sel_h, 15, 12, func);
		if (pin == RK_PIN_D4) rk_clr_set_bits(&busioc->gpio3d_iomux_sel_h, 3, 0, func);
	}

	// This driver is a pain to write, I'm stoppig here
}
