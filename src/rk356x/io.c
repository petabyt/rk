#include <string.h>
#include "main.h"
#include "firmware.h"
#include "rk356x.h"

static uint64_t dram_size;
static uint32_t page_tables[3][1024] __attribute__((aligned(4096)));

static uint64_t payload_end(void) {
	const struct FuPayloadHeader *header = (const void *)_end_of_image;
	uint64_t end = RK356X_PAYLOAD;
	if (header->magic == 0x08008135 && header->img_size >= sizeof(*header) &&
		header->img_size < RK356X_STACK_BOTTOM - RK356X_PAYLOAD)
		end += header->img_size;
	return (end + 0xfff) & ~0xfffULL;
}

void plat_setup_mmu(void *unused) {
	uint8_t *l1 = (uint8_t *)page_tables[0];
	uint8_t *low = (uint8_t *)page_tables[1];
	uint8_t *high = (uint8_t *)page_tables[2];
	(void)unused;
	memset(page_tables, 0, sizeof(page_tables));

	ttbl_table_entry(l1 + 0 * 8, (uintptr_t)low);
	ttbl_block_1gb(l1 + 1 * 8, 0x40000000, 3);
	ttbl_block_1gb(l1 + 2 * 8, 0x80000000, 3);
	ttbl_table_entry(l1 + 3 * 8, (uintptr_t)high);
	for (unsigned int i = 0; i < 512; i++) {
		uint64_t address = (uint64_t)i << 21;
		uint64_t attr = 3;
		if ((address >= RK356X_DMA_START && address < RK356X_DMA_END) ||
			(address >= RK356X_FB_START && address < RK356X_FB_END))
			attr = 2;
		ttbl_block_2mb(low + i * 8, address, attr);
	}
	for (unsigned int i = 0; i < 512; i++) {
		uint64_t address = 0xc0000000ULL + ((uint64_t)i << 21);
		ttbl_block_2mb(high + i * 8, address,
			address >= RK356X_MMIO_START ? 0 : 3);
	}
	/* 4 KiB granule, 32-bit VA/PA; device, non-cacheable and WB attributes. */
	setup_tt_el3(0x3520, 0xeeff440400ULL, (uintptr_t)l1);
	enable_mmu_el3();
}

static void map_add(struct FuMemoryMap *map, uint64_t start, uint64_t end,
		uint32_t flags) {
	if (end <= start)
		return;
	map->items[map->length].start_addr = start;
	map->items[map->length].end_addr = end;
	map->items[map->length].flags = flags;
	map->items[map->length].pad2 = 0;
	map->length++;
}

void plat_get_mem_map(void *buffer) {
	struct FuMemoryMap *map = buffer;
	uint64_t ram_end = dram_size ? dram_size : rk356x_detect_dram();
	uint64_t p_end = payload_end();
	if (ram_end > RK356X_MMIO_START)
		ram_end = RK356X_MMIO_START;
	map->length = 0;
	map->pad = 0;
	map_add(map, 0, 0x00200000, FU_MEM_ATTR_RESERVED);
	map_add(map, 0x00200000, RK356X_PAYLOAD, FU_MEM_ATTR_UNUSED);
	map_add(map, RK356X_PAYLOAD, p_end, FU_MEM_ATTR_PAYLOAD);
	map_add(map, p_end, RK356X_STACK_BOTTOM, FU_MEM_ATTR_UNUSED);
	map_add(map, RK356X_STACK_BOTTOM, RK356X_STACK_TOP, FU_MEM_ATTR_RESERVED);
	map_add(map, RK356X_DMA_START, RK356X_DMA_END, FU_MEM_ATTR_RESERVED);
	map_add(map, RK356X_DMA_END, RK356X_FB_START, FU_MEM_ATTR_UNUSED);
	map_add(map, RK356X_FB_START, RK356X_FB_END, FU_MEM_ATTR_FRAMEBUFFER);
	map_add(map, RK356X_FB_END, ram_end, FU_MEM_ATTR_UNUSED);
	map_add(map, RK356X_MMIO_START, 0x100000000ULL, FU_MEM_ATTR_MMIO);
}

volatile void *plat_get_uart_base(void) {
	return (volatile void *)RK356X_UART2;
}

uintptr_t plat_get_framebuffer(void) {
	return rk356x_video.active ? RK356X_FB_START : 0;
}

void plat_get_screen(uint32_t *width, uint32_t *height, uint32_t *stride) {
	if (rk356x_video.active) {
		*width = rk356x_video.mode.hactive;
		*height = rk356x_video.mode.vactive;
		*stride = rk356x_video.stride;
	} else {
		*width = *height = *stride = 0;
	}
}

uint64_t rk356x_ram_size(void) {
	if (!dram_size)
		dram_size = rk356x_detect_dram();
	return dram_size;
}

void plat_reset(void) {
	*(volatile uint32_t *)(RK356X_PMUGRF + 0x200) = 0xef08a53c;
	__asm__ volatile("dsb sy");
	*(volatile uint32_t *)(RK356X_CRU + 0xd4) = 0xfdb9;
	halt();
}

void plat_shutdown(void) {
	plat_reset();
}
