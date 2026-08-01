#include <stdint.h>
#include "rk356x.h"

#define ATAGS_START 0x001fe000UL
#define ATAGS_END   0x00200000UL
#define ATAG_DDR_MEM 0x54410052U

struct __attribute__((packed)) TagHeader {
	uint32_t size;
	uint32_t magic;
};

struct __attribute__((packed)) TagDdrMem {
	uint32_t count;
	uint32_t version;
	uint64_t bank[20];
	uint32_t flags;
	uint32_t data[2];
	uint32_t hash;
};

static uint32_t js_hash(const void *buffer, uint32_t length) {
	const uint8_t *p = buffer;
	uint32_t hash = 0x47c6a7e6;
	for (uint32_t i = 0; i < length; i++)
		hash ^= (hash << 5) + p[i] + (hash >> 2);
	return hash;
}

static uint64_t dram_from_atags(void) {
	const uint32_t *cursor = (const uint32_t *)ATAGS_START;
	while ((uintptr_t)cursor + sizeof(struct TagHeader) <= ATAGS_END) {
		const struct TagHeader *header = (const void *)cursor;
		const struct TagDdrMem *ddr;
		uint64_t top = 0;
		if (!header->size)
			break;
		if (header->size < 2 || (uintptr_t)(cursor + header->size) > ATAGS_END)
			return 0;
		if (header->magic != ATAG_DDR_MEM) {
			cursor += header->size;
			continue;
		}
		if (header->size < (sizeof(*header) + sizeof(struct TagDdrMem)) / 4)
			return 0;
		ddr = (const void *)((const uint8_t *)&header->magic +
			sizeof(header->magic));
		if (!ddr->count || ddr->count > 10)
			return 0;
		if (ddr->hash && js_hash(header, (header->size - 1) * 4) != ddr->hash)
			return 0;
		for (uint32_t i = 0; i < ddr->count; i++) {
			uint64_t start = ddr->bank[i];
			uint64_t size = ddr->bank[i + ddr->count];
			if (!size || start + size < start)
				return 0;
			if (start + size > top)
				top = start + size;
		}
		return top;
	}
	return 0;
}

static uint64_t dram_from_geometry(void) {
	uint32_t r2 = *(volatile uint32_t *)(RK356X_PMUGRF + 0x208);
	uint32_t r3 = *(volatile uint32_t *)(RK356X_PMUGRF + 0x20c);
	uint32_t channels = 1 + ((r2 >> 12) & 1);
	uint32_t version = r3 >> 28;
	uint32_t type = (r2 >> 13) & 7;
	uint64_t total_mb = 0;
	if (version >= 3)
		type |= ((r3 >> 12) & 3) << 3;
	for (uint32_t ch = 0; ch < channels; ch++) {
		uint32_t rank = 1 + ((r2 >> (11 + ch * 16)) & 1);
		uint32_t col0 = 9 + ((r2 >> (9 + ch * 16)) & 3);
		uint32_t col1 = col0;
		uint32_t banks = type == 9 ? 3 + ((r2 >> (8 + ch * 16)) & 1)
			: 3 - ((r2 >> (8 + ch * 16)) & 1);
		uint32_t row0, row1;
		uint32_t bw = 2 >> ((r2 >> (2 + ch * 16)) & 3);
		uint32_t bg = 0;
		if (version >= 2) {
			uint32_t raw0 = ((r3 >> (5 + ch * 2)) & 1) * 4 +
				((r2 >> (6 + ch * 16)) & 3);
			uint32_t raw1 = ((r3 >> (4 + ch * 2)) & 1) * 4 +
				((r2 >> (4 + ch * 16)) & 3);
			row0 = raw0 == 7 ? 12 : 13 + raw0;
			row1 = raw1 == 7 ? 12 : 13 + raw1;
			col1 = 9 + ((r3 >> (ch * 2)) & 3);
		} else {
			row0 = 13 + ((r2 >> (6 + ch * 16)) & 3);
			row1 = 13 + ((r2 >> (4 + ch * 16)) & 3);
		}
		if (type == 0)
			bg = (((r2 >> (ch * 16)) & 3) == 2) ? 2 : 1;
		if (row0 + col0 + banks + bg + bw < 20 ||
			row0 + col0 + banks + bg + bw > 34)
			return 0;
		uint64_t mb = 1ULL << (row0 + col0 + banks + bg + bw - 20);
		if (rank > 1) {
			int delta = (int)row0 - (int)row1 +
				(int)col0 - (int)col1;
			if (delta < 0 || delta > 8)
				return 0;
			mb += mb >> delta;
		}
		if ((r2 >> (30 + ch)) & 1)
			mb = mb * 3 / 4;
		total_mb += mb;
	}
	if (total_mb < 1024 || total_mb > 32768)
		return 0;
	return total_mb << 20;
}

uint64_t rk356x_detect_dram(void) {
	uint64_t size = dram_from_atags();
	if (!size)
		size = dram_from_geometry();
	if (!size)
		size = 1ULL << 30;
	return size;
}
