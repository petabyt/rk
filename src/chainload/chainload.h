/* SPDX-License-Identifier: Apache-2.0 */
#ifndef RK_CHAINLOAD_H
#define RK_CHAINLOAD_H

#include <stddef.h>
#include <stdint.h>

#define CHAIN_MAX_IMAGES 8
#define CHAIN_DTB_SLACK 8192U
#define CHAIN_PARAMS_RESERVE 1024U

#ifdef _MSC_VER
#define CHAIN_NORETURN __declspec(noreturn)
#else
#define CHAIN_NORETURN __attribute__((noreturn))
#endif

enum ChainImageRole {
	CHAIN_IMAGE_BL31,
	CHAIN_IMAGE_BL33,
	CHAIN_IMAGE_FDT,
};

enum ChainHandoffProtocol {
	CHAIN_HANDOFF_TFA_V1_BL33_EL2 = 1,
};

struct ChainAddressRange {
	uintptr_t start;
	uintptr_t end;
};

struct ChainImage {
	const char *name;
	const void *data;
	size_t size;
	uintptr_t load;
	uintptr_t entry;
	const char *type;
	const char *os;
	enum ChainImageRole role;
	uint8_t record_in_control_fdt;
	uint8_t entry_explicit;
};

struct ChainFitPlan {
	struct ChainImage images[CHAIN_MAX_IMAGES];
	unsigned int image_count;
	unsigned int bl31_count;
	unsigned int bl33_index;
	unsigned int fdt_index;
	uintptr_t bl31_entry;
	uintptr_t bl33_entry;
	uintptr_t control_fdt;
	size_t control_fdt_capacity;
};

struct ChainPlatform {
	const char *board;
	const char *soc;
	uintptr_t fit_stage_start;
	uintptr_t fit_stage_end;
	uintptr_t params_addr;
	uintptr_t expected_bl31_entry;
	uintptr_t expected_bl33_entry;
	uintptr_t bl33_limit;
	enum ChainHandoffProtocol handoff_protocol;
	const struct ChainAddressRange *bl31_ranges;
	unsigned int bl31_range_count;
	unsigned int expected_bl31_segments;
	const char *(*get_boot_source)(void);
	int (*prepare_handoff)(void);
	int (*range_is_cacheable)(uintptr_t start, uintptr_t end);
	void (*recover)(const char *stage, const char *reason);
};

int chain_fit_parse(const void *fit, size_t available,
		const struct ChainPlatform *platform, struct ChainFitPlan *plan,
		const char **reason);
int chain_fit_load(const void *fit, struct ChainFitPlan *plan,
		const char **reason);
void *chain_build_bl31_params(uintptr_t address, uintptr_t bl31_entry,
		size_t bl31_size, uintptr_t bl33_entry, uintptr_t bl33_base,
		size_t bl33_size);
CHAIN_NORETURN void chainload_run(const struct ChainPlatform *platform,
	const void *appended_fit);
CHAIN_NORETURN void chain_jump_bl31(uintptr_t entry, void *params);

#endif
