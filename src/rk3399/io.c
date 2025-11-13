#include <main.h>
#include <firmware.h>
#include "rk3399.h"

void plat_setup_mmu(void *buffer) {
	buffer = (void *)align_to_4kb((uintptr_t)buffer);

	// 4kb page size
	// 32 bit output address
	uint64_t tcr = 0x3520;
	// attr0: ngnrne device memory
	// attr1: ngnre device memory
	// attr2: NonCacheable
	// attr3: WriteBack_NonTransient_ReadWriteAlloc
	uint64_t mair = 0xff440400;

	uint8_t *tbl = buffer;
	tbl += ttbl_block_1gb(tbl, 0x00000000, 3);
	tbl += ttbl_block_1gb(tbl, 0x40000000, 3);
	tbl += ttbl_block_1gb(tbl, 0x80000000, 3);
	tbl += ttbl_block_1gb(tbl, 0xc0000000, 0);

	setup_tt_el3(tcr, mair, (uintptr_t)buffer);
	enable_mmu_el3();
}

void plat_get_mem_map(void *buffer) {
	struct FuMemoryMap *map = buffer;
	map->length = 4;
	map->items[0].flags = FU_MEM_ATTR_RESERVED;
	map->items[0].start_addr = 0x0;
	map->items[0].end_addr = (uintptr_t)_end_of_image;
	map->items[1].flags = FU_MEM_ATTR_PAYLOAD;
	map->items[1].start_addr = (uintptr_t)_end_of_image;
	map->items[1].end_addr = 0x10000000;
	map->items[2].flags = FU_MEM_ATTR_UNUSED;
	map->items[2].start_addr = 0x10000000;
	map->items[2].end_addr = 0xc0000000;
	map->items[3].flags = FU_MEM_ATTR_MMIO;
	map->items[3].start_addr = 0xc0000000;
	map->items[3].end_addr = 0x100000000;
}

volatile void *plat_get_uart_base(void) {
	return (volatile void *)UART2_START; 
}

uintptr_t plat_get_framebuffer(void) {
	return 0xf7800000;
}

void enable_uart(void) {
	grf_gpio_iomux_set(IOMUX_4C, 9, 8, 0b1); // gpio4c4_sel = uart2dbgc_sout
	grf_gpio_iomux_set(IOMUX_4C, 7, 6, 0b1); // gpio4c3_sel = uart2dbgc_sin

	rk_clr_set_bits((uint32_t *)(GRF_BASE + 0xe21c), 11, 10, 0b10);
}

void plat_reset(void) {
	gpio_set_dir(1, 6, 1);
	gpio_set_pin(1, 6, 1);
}

void plat_shutdown(void) {
	plat_reset();
}
