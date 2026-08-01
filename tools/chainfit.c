/* SPDX-License-Identifier: Apache-2.0 */
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chainload/chainload.h"

#define MAX_BL31_RANGES 8

static int parse_number(const char *text, uintptr_t *result) {
	char *end;
	unsigned long long value;
	errno = 0;
	value = strtoull(text, &end, 0);
	if (errno || !*text || *end || value > UINTPTR_MAX)
		return -1;
	*result = (uintptr_t)value;
	return 0;
}

int main(int argc, char **argv) {
	struct ChainFitPlan plan;
	const char *reason;
	unsigned char *data;
	FILE *file;
	long size;
	struct ChainPlatform platform = { 0 };
	struct ChainAddressRange ranges[MAX_BL31_RANGES];
	uintptr_t segments;
	unsigned int range_count;
	if (argc < 12 || ((argc - 10) & 1)) {
		fprintf(stderr, "usage: %s BOARD FIT FIT_START FIT_END PARAMS "
			"BL31_ENTRY BL33_ENTRY BL33_LIMIT SEGMENTS RANGE_START RANGE_END [...]\n",
			argv[0]);
		return 2;
	}
	range_count = (unsigned int)(argc - 10) / 2;
	if (range_count > MAX_BL31_RANGES ||
		parse_number(argv[3], &platform.fit_stage_start) ||
		parse_number(argv[4], &platform.fit_stage_end) ||
		parse_number(argv[5], &platform.params_addr) ||
		parse_number(argv[6], &platform.expected_bl31_entry) ||
		parse_number(argv[7], &platform.expected_bl33_entry) ||
		parse_number(argv[8], &platform.bl33_limit) ||
		parse_number(argv[9], &segments) || segments > UINT32_MAX) {
		fprintf(stderr, "invalid manifest-derived address policy\n");
		return 2;
	}
	for (unsigned int index = 0; index < range_count; index++) {
		if (parse_number(argv[10 + index * 2], &ranges[index].start) ||
			parse_number(argv[11 + index * 2], &ranges[index].end)) {
			fprintf(stderr, "invalid manifest-derived BL31 range\n");
			return 2;
		}
	}
	platform.board = argv[1];
	platform.soc = "rk3568";
	platform.handoff_protocol = CHAIN_HANDOFF_TFA_V1_BL33_EL2;
	platform.bl31_ranges = ranges;
	platform.bl31_range_count = range_count;
	platform.expected_bl31_segments = (uint32_t)segments;
	file = fopen(argv[2], "rb");
	if (!file) {
		perror(argv[2]);
		return 2;
	}
	if (fseek(file, 0, SEEK_END) || (size = ftell(file)) <= 0 ||
		fseek(file, 0, SEEK_SET)) {
		fprintf(stderr, "cannot size FIT\n");
		return 2;
	}
	data = malloc((size_t)size);
	if (!data || fread(data, 1, (size_t)size, file) != (size_t)size) {
		fprintf(stderr, "cannot read FIT\n");
		return 2;
	}
	fclose(file);
	if (chain_fit_parse(data, (size_t)size, &platform, &plan, &reason)) {
		fprintf(stderr, "invalid FIT: %s\n", reason);
		return 1;
	}
	printf("board=%s bl31_segments=%u bl31_entry=0x%lx "
		"bl33_entry=0x%lx control_fdt=0x%lx\n", argv[1], plan.bl31_count,
		(unsigned long)plan.bl31_entry, (unsigned long)plan.bl33_entry,
		(unsigned long)plan.control_fdt);
	free(data);
	return 0;
}
