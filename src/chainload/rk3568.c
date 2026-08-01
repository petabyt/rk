/* SPDX-License-Identifier: Apache-2.0 */
#include <stdint.h>
#include <string.h>

#include "board_config.h"
#include "chainload.h"
#include "main.h"

#define PMUGRF_BASE 0xfdc20000UL
#define CPU_GRF_BASE 0xfdc30000UL
#define GRF_BASE 0xfdc60000UL
#define USBPHY_U3_GRF 0xfdca0000UL
#define USBPHY_U2_GRF 0xfdca8000UL
#define PMUCRU_BASE 0xfdd00000UL
#define SGRF_BASE 0xfdd18000UL
#define CRU_BASE 0xfdd20000UL
#define PMU_BASE 0xfdd90000UL
#define UART2_BASE 0xfe660000UL
#define EBC_PRIORITY_REG 0xfe158008UL
#define BROM_BOOTSOURCE_ID_ADDR 0xfdcc0010UL
#define BROM_BOOTSOURCE_MASK 0x0fU

extern char _fit_start[];

static uint64_t page_tables[3][512] __attribute__((aligned(4096)));

static inline volatile uint32_t *reg(uintptr_t address) {
	return (volatile uint32_t *)address;
}

static inline void write32(uintptr_t address, uint32_t value) {
	*reg(address) = value;
}

volatile void *plat_get_uart_base(void) {
	return (void *)UART2_BASE;
}

void enable_uart(void) {
	/* RK3568 UART2 M0: GPIO0_D0 RX, GPIO0_D1 TX, xin24m clock. */
	rk_clr_set_bits(reg(PMUGRF_BASE + 0x18), 2, 0, 1);
	rk_clr_set_bits(reg(PMUGRF_BASE + 0x18), 6, 4, 1);
	rk_clr_set_bits(reg(GRF_BASE + 0x30c), 11, 10, 0);
	rk_clr_set_bits(reg(CRU_BASE + 0x100 + 54 * 4), 13, 12, 2);
	rk_clr_set_bits(reg(CRU_BASE + 0x300 + 26 * 4), 1, 1, 0);
	rk_clr_set_bits(reg(CRU_BASE + 0x300 + 28 * 4), 0, 0, 0);
	rk_clr_set_bits(reg(CRU_BASE + 0x300 + 28 * 4), 3, 3, 0);
	rk_clr_set_bits(reg(CRU_BASE + 0x400 + 25 * 4), 1, 0, 0);
}

void plat_setup_mmu(void *unused) {
	uint8_t *l1 = (uint8_t *)page_tables[0];
	uint8_t *low = (uint8_t *)page_tables[1];
	uint8_t *high = (uint8_t *)page_tables[2];
	(void)unused;
	memset(page_tables, 0, sizeof(page_tables));
	ttbl_table_entry(l1, (uintptr_t)low);
	ttbl_block_1gb(l1 + 8, 0x40000000, 3);
	ttbl_block_1gb(l1 + 16, 0x80000000, 3);
	ttbl_table_entry(l1 + 24, (uintptr_t)high);
	for (unsigned int i = 0; i < 512; i++)
		ttbl_block_2mb(low + i * 8, (uint64_t)i << 21, 3);
	for (unsigned int i = 0; i < 512; i++) {
		uint64_t address = 0xc0000000ULL + ((uint64_t)i << 21);
		ttbl_block_2mb(high + i * 8, address,
			address >= 0xf0000000ULL ? 0 : 3);
	}
	setup_tt_el3(0x3520, 0xeeff440400ULL, (uintptr_t)l1);
	enable_mmu_el3();
}

static int wait_clear(uintptr_t address, uint32_t mask) {
	for (unsigned int timeout = 0; timeout < 1000; timeout++) {
		if (!(*reg(address) & mask))
			return 0;
		usleep(1);
	}
	return -1;
}

static int rk3568_prepare_handoff(void) {
	/* Ported from U-Boot rk3568 arch_cpu_init()/qos_priority_init(). */
	write32(PMU_BASE + 0x70, 0xffffffff);
	write32(PMU_BASE + 0x74, 0x000f000f);
	write32(SGRF_BASE + 0x10, ((0x3U << 11 | 0x1U << 4) << 16));
	write32(CPU_GRF_BASE + 0x10, 0x00ff002b);
	write32(CRU_BASE + 0x470, 0x02a002a0);
	write32(USBPHY_U3_GRF + 0x04, 0x01ff01d1);
	write32(USBPHY_U2_GRF + 0x00, 0x01ff01d1);
	write32(USBPHY_U2_GRF + 0x04, 0x01ff01d1);

	/* Power every domain except GPU and NPU, then release their NOC idle. */
	write32(PMU_BASE + 0xa0, 0xfffc0000);
	if (wait_clear(PMU_BASE + 0x98, 0xfffffffc))
		return -1;
	write32(PMU_BASE + 0x50, 0xfff90000);
	if (wait_clear(PMU_BASE + 0x60, 0xfffffff9) ||
		wait_clear(PMU_BASE + 0x68, 0xfffffff9))
		return -1;
	write32(EBC_PRIORITY_REG, 0x303);
	return 0;
}

static int rk3568_range_is_cacheable(uintptr_t start, uintptr_t end) {
	(void)start;
	return end <= 0xf0000000UL;
}

static const char *rk3568_boot_source(void) {
	switch (*reg(BROM_BOOTSOURCE_ID_ADDR) & BROM_BOOTSOURCE_MASK) {
	case 1: return "source=nand";
	case 2: return "source=emmc";
	case 3: return "source=spi-nor";
	case 4: return "source=spi-nand";
	case 5: return "source=sd";
	case 10: return "source=usb";
	default: return "source=unknown";
	}
}

static void rk3568_recover(const char *stage, const char *reason) {
	puts("chainload: fatal error");
	puts("board: " CHAIN_BOARD_NAME);
	puts("stage:");
	puts(stage);
	puts("reason:");
	puts(reason ? reason : "unknown error");
	/* BootROM download marker followed by the RK3568 global reset. */
	write32(PMUGRF_BASE + 0x200, 0xef08a53c);
	__asm__ volatile("dsb sy");
	write32(CRU_BASE + 0xd4, 0xfdb9);
	halt();
}

static const struct ChainAddressRange bl31_ranges[] = {
	CHAIN_BL31_RANGE_INITIALIZER
};

_Static_assert(sizeof(bl31_ranges) / sizeof(bl31_ranges[0]) ==
	CHAIN_BL31_RANGE_COUNT, "generated BL31 range count mismatch");

static const struct ChainPlatform board_platform = {
	.board = CHAIN_BOARD_NAME,
	.soc = CHAIN_SOC_NAME,
	.fit_stage_start = CHAIN_FIT_STAGE_START,
	.fit_stage_end = CHAIN_FIT_STAGE_END,
	.params_addr = CHAIN_PARAMS_ADDR,
	.expected_bl31_entry = CHAIN_EXPECTED_BL31_ENTRY,
	.expected_bl33_entry = CHAIN_EXPECTED_BL33_ENTRY,
	.bl33_limit = CHAIN_BL33_LIMIT,
	.handoff_protocol = CHAIN_HANDOFF_TFA_V1_BL33_EL2,
	.bl31_ranges = bl31_ranges,
	.bl31_range_count = CHAIN_BL31_RANGE_COUNT,
	.expected_bl31_segments = CHAIN_EXPECTED_BL31_SEGMENTS,
	.get_boot_source = rk3568_boot_source,
	.prepare_handoff = rk3568_prepare_handoff,
	.range_is_cacheable = rk3568_range_is_cacheable,
	.recover = rk3568_recover,
};

void c_entry(void) {
	asm_set_cnt_freq(24000000);
	enable_uart();
	uart_init(1500000);
	puts("rk chainloader");
	puts(board_platform.board);
	puts(board_platform.soc);
	puts(board_platform.get_boot_source());
	plat_setup_mmu(NULL);
	chainload_run(&board_platform, _fit_start);
}
