//#include <string.h>
#include <stdint.h>
#include "main.h"
#include "rk3588.h"
#include "firmware.h"

struct FuMemory mem = {
	.start_addr = DUMMY_ALLOC_BASE,
	.end_addr = DUMMY_ALLOC_BASE + 0x40000000,
};
struct FuScreenList screens;

int hdptx_phy_configure_edp(int lanes, int linkrate_mbps);
int hdptx_phy_init(int id);

uint64_t plat_process_firmware_call(uint64_t p1, uint64_t p2, uint64_t p3) {
	switch (p1) {
	case FU_GET_SCREEN_LIST:
		screens.length = 1;
		screens.screens[0].framebuffer_addr = 0xd0000000;
		screens.screens[0].width = 1920;
		screens.screens[0].height = 1080;
		screens.screens[0].stride = 1920 * 4;
		return (uintptr_t)&screens;
	case FU_GET_MEM_CHUNK:
		return (uintptr_t)&mem;
	case 0xf00ba000:
		hdptx_phy_init(1);
		hdptx_phy_configure_edp(2, 0xa8c);
		return 0;
	}

	return 0;
}

void start_in_el2(uintptr_t addr);
void start_in_el2_x(uintptr_t addr);
void start_in_el1(uintptr_t addr);

void testcode(void) {
	puts("Hello from el2");
}

int c_entry(void) {
	uint64_t tcr = 0x351c | (1 << 0x10);

	// attr0: ngnrne device memory
	// attr1: ngnre device memory
	// attr2: NonCacheable
	// attr3: WriteBack_NonTransient_ReadWriteAlloc
	uint64_t mair = 0xFF440400;

	setup_tt_el3(tcr, mair, (uintptr_t)ttb0_base);
	//enable_mmu_el3();

	// GPIO0_C4_1V8_D lcd pwr on
	gpio_set_dir(0, RK_PIN_C4, 1); gpio_set_pin(0, RK_PIN_C4, 1);
	// GPIO4_A3_3V3_D blen
	gpio_set_dir(4, RK_PIN_A3, 1); gpio_set_pin(4, RK_PIN_A3, 1);
	// typec5v_pwren
	gpio_set_dir(0, RK_PIN_A0, 1); gpio_set_pin(0, RK_PIN_A0, 1);
    // vcc5v0_host1_en
	gpio_set_dir(1, RK_PIN_D5, 1); gpio_set_pin(1, RK_PIN_D5, 1);
    // keyboard_en
	gpio_set_dir(1, RK_PIN_A7, 1); gpio_set_pin(1, RK_PIN_A7, 1);

	// Setup backlight
	// Set gpio4c1 function to pwm6
	volatile struct BusIoc *busioc = (volatile struct BusIoc *)BUS_IOC;
	rk_clr_set_bits(&busioc->gpio4c_iomux_sel_l, 7, 4, 0xb);
	pwm_setup_continuous(6, 0x400, 0x100);

	// cw2015 battery
	rk3588_set_pin_func(1, RK_PIN_A3, 9);
	rk3588_set_pin_func(1, RK_PIN_A2, 9);

	rk3588_sgrf_init();
	rk3588_init_power_domains();

	// SPSR_el3: 0x600003CD
	// SPSR_el2: 0x52DF9AC9

	uint64_t v;
	asm volatile("mrs %0, spsr_el2" : "=r"(v));
	debug("spsr_el2: ", v);

	puts("Jumping to EL2");
	start_in_el1((uintptr_t)&testcode);
	puts("end");
	halt();

	rk3588_setup_video_edp1(0xd0000000, 1920, 1080);

	jump_to_payload();

	return 0;
}
