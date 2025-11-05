#include "rk3588.h"

static void write32(uintptr_t addr, uint32_t v) {
	((volatile uint32_t *)addr)[0] = v;
}
static uint32_t read32(uintptr_t addr) {
	return ((volatile uint32_t *)addr)[0];
}

void unknown_setup(void) {
	// setup done by bootrom for emmc boot
	write32(0xfd5f8040, 0xffff0000);
	write32(0xfd5f8058, 0xffff0000);
	write32(0xfd5f805c, 0xffff0000);
	write32(0xfe0380a8, read32(0xfe0380a8) | 0xffff0000);
	write32(0xfe0300a8, read32(0xfe0300a8) | 0xffff0000);
}

void rk3588_sgrf_init(void) {
	write32(0xfe0300f0, read32(0xfe0300f0) & 0xffff0001);
	write32(0xfe0100f0, read32(0xfe0100f0) & 0xffff0001);
	write32(0xfe0100f4, read32(0xfe0100f4) & 0xffff0001);
	write32(0xfe0100f8, read32(0xfe0100f8) & 0xffff0001);
	write32(0xfe0100fc, read32(0xfe0100fc) & 0xffff0001);
	write32(0xfe0380f0, read32(0xfe0380f0) & 0xffff0001);

	write32(0xfe030044, 0xffff);
	write32(0xfe030078, 0x0);
	write32(0xfe0300d0, 0xffff0000);

	write32(0x0fe030040 + (1 * 4), 0xffffffff);
	write32(0x0fe030040 + (0xe * 4), 0xffffffff);
	write32(0x0fe030040 + (0x24 * 4), 0xffffffff);

	write32(0x0fe03808c, 0xff);
	write32(0x0fe0380d8, 0xff);
	write32(0x0fe0380e4, 0x0);

	write32(0x0fe038040 + (1 * 4), 0xff00ff);
	write32(0x0fe038040 + (0x26 * 4), 0xff00ff);
	write32(0x0fe038040 + (0x29 * 4), 0xff00ff);

	write32(0xfe010040, 0xffffffff);
	write32(0xfe010044, 0x0);

	__asm__(
		"dsb sy\n"
		"isb"
	);

	write32(0xfe010000, 0x0);

	// DSU secure region
    write32(0xfe0100f0, read32(0xfe0100f0) | 1);
    write32(0xfe0100f4, read32(0xfe0100f4) | 1);
    write32(0xfe0100f8, read32(0xfe0100f8) | 1);
    write32(0xfe0100fc, read32(0xfe0100fc) | 1);
	// DDR secure region
	write32(0xfe030000, 0);
	write32(0xfe0300f0, read32(0xfe0300f0) | 1);
	write32(0xfe038000, 0);
	write32(0xfe03801c, 0xff00f0);
	write32(0xfe0380f0, read32(0xfe0380f0) | 0x81);
	// Mark SRAM secure
	write32(0xfd586038, 0x1f0011);
	write32(0xfd58603c, 0xffffffff);
	// Mark MMIO as secure
    write32(0xfd586040, 0x3ff03ff);
    write32(0xfd586250, 0xffff2000);
    write32(0xfd586254, 0xffff0060);
    write32(0xfd5862a0, 0xffff0fbf);
    write32(0xfd5862a4, 0xffff03ff);

	write32(0xfd586240 + (4 * 4), 0xffff0000);
	write32(0xfd586240 + (4 * 5), 0xffff0000);
	write32(0xfd586240 + (4 * 18), 0xffff0000);
	write32(0xfd586240 + (4 * 19), 0xffff0000);

    write32(0xfd582000, 0x180010);
    write32(0xfd582004, 0x0efbe6020);
    write32(0xfd582008, 0x1f900c0);
    write32(0xfd586034, 0x180018);
    write32(0xfd586044, 0x11111111);
    write32(0xfd586048, 0x11111111);
    write32(0xfd58604c, 0x110011);

	// configs copied from ATF
    write32(0xfd7c0c10, 0x0ffdf);
    write32(0xfd586008, 0x0c000c00);
    write32(0xfd58a004, 0x0ffffffff);
    write32(0xfd582000, 0x70007);
    write32(0xfd58a01c, 0x30003000);
    write32(0xfd582038, 0x200020);
    write32(0xfd5f0000, 0x0f00020);

	unknown_setup();
}
