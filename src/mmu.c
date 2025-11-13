#include <stdint.h>
#include "main.h"

#define ATTR(mair_i) (0x740 | (mair_i << 2))

static void write32(void *addr, uint32_t v) {
	((volatile uint32_t *)addr)[0] = v;
}

int ttbl_block_1gb(uint8_t *buf, uint64_t oa, uint64_t mair_i) {
	write32(buf, (oa & 0xc0000000) | ATTR(mair_i) | 1);
	write32(buf + 4, 0);
	return 8;
}
int ttbl_block_2mb(uint8_t *buf, uint64_t oa, uint64_t mair_i) {
	write32(buf, (oa & 0xffe00000) | ATTR(mair_i) | 1);
	write32(buf + 4, 0);
	return 8;
}
int ttbl_table_entry(uint8_t *buf, uint64_t oa) {
	write32(buf, oa + 3);
	write32(buf + 4, 0);
	return 8;
}

void dcache_clean(uintptr_t start_addr, uintptr_t end_addr) {
	start_addr = ((start_addr) - 0x40) & ~0x3f;
	end_addr = ((end_addr) + 0x40) & ~0x3f;
	__asm__("dsb sy");
	while (start_addr < end_addr) {
		asm_dc_civac(start_addr);
		start_addr += 0x40;
	}
	__asm__("dsb sy");
}
