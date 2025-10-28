#include <string.h>
#include <stdint.h>
#include "os.h"
#include "rk3588.h"
#include "firmware.h"

struct FuScreenList screens;

void usleep(int ticks) {
	while (ticks--) {
		nop_sleep_short();
	}
}

struct FuMemory mem = {
	.start_addr = DUMMY_ALLOC_BASE,
	.end_addr = DUMMY_ALLOC_BASE + 0x40000000,
};

int edk2_hdptx_phy_configure_edp(int lanes, int linkrate_mbps);
int edk2_hdptx_phy_init(int id);

uint64_t plat_process_firmware_call(uint64_t p1, uint64_t p2, uint64_t p3) {
	switch (p1) {
	case FU_GET_SCREEN_LIST:
		screens.length = 0;
		return (uintptr_t)&screens;
	case FU_GET_MEM_CHUNK:
		return (uintptr_t)&mem;
	case 0xf00ba000:
		edk2_hdptx_phy_init(1);
		edk2_hdptx_phy_configure_edp(2, 0xa8c);
		return 0;
	}

	return 0;
}

int c_entry(void) {
	uint64_t tcr = 0x351c | (1 << 0x10);

	// attr0: ngnrne device memory
	// attr1: ngnre device memory
	// attr2: NonCacheable
	// attr3: WriteBack_NonTransient_ReadWriteAlloc
	uint64_t mair = 0xFF440400;

	setup_tt_el3(tcr, mair, (uintptr_t)ttb0_base);
	enable_mmu_el3();

	puts("Hello, World");

	rk3588_sgrf_init();

	jump_to_payload();

	return 0;
}
