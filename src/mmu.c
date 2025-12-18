#include <stdint.h>
#include "main.h"

// AF: 1
// SH: Inner Shareable
// NS: 0
// AP: 1
#define FLAG 0x740
#define ATTR(mair_i) (FLAG | (mair_i << 2))

static void write64(void *addr, uint64_t v) {
	((volatile uint64_t *)addr)[0] = v;
}

int ttbl_block_1gb(uint8_t *buf, uint64_t oa, uint64_t mair_i) {
	write64(buf, (oa & 0xffffffffc0000000) | ATTR(mair_i) | 1);
	return 8;
}
int ttbl_block_2mb(uint8_t *buf, uint64_t oa, uint64_t mair_i) {
	write64(buf, (oa & 0xffffffffffe00000) | ATTR(mair_i) | 1);
	return 8;
}
int ttbl_table_entry(uint8_t *buf, uint64_t oa) {
	write64(buf, oa | 3);
	return 8;
}

void dcache_clean(uintptr_t start_addr, uintptr_t end_addr) {
	start_addr = ((start_addr) - 0x40) & ~(uint64_t)0x3f;
	end_addr = ((end_addr) + 0x40) & ~(uint64_t)0x3f;
	__asm__("dsb sy");
	while (start_addr < end_addr) {
		asm_dc_civac(start_addr);
		start_addr += 0x40;
	}
	asm_dc_civac(start_addr);
	__asm__("dsb sy");
}
