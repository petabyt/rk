/* SPDX-License-Identifier: Apache-2.0 */
#include <stdint.h>
#include <string.h>

#include "chainload.h"
#include "libfdt.h"

int puts(const char *text);
void debug(const char *text, uint64_t value);
void dcache_clean(uintptr_t start, uintptr_t end);

static CHAIN_NORETURN void fail(const struct ChainPlatform *platform,
		const char *stage, const char *reason) {
	platform->recover(stage, reason);
	for (;;)
		;
}

void chainload_run(const struct ChainPlatform *platform, const void *appended_fit) {
	struct ChainFitPlan plan;
	const char *reason;
	void *params;
	size_t bl31_bytes = 0;
	size_t stage_capacity;
	size_t fit_size;

	if (platform->fit_stage_end <= platform->fit_stage_start)
		fail(platform, "policy", "invalid FIT staging arena");
	if (platform->handoff_protocol != CHAIN_HANDOFF_TFA_V1_BL33_EL2)
		fail(platform, "policy", "unsupported BL31/BL33 handoff protocol");
	stage_capacity = platform->fit_stage_end - platform->fit_stage_start;
	puts("chainload: validating appended FIT");
	if (fdt_check_header(appended_fit))
		fail(platform, "stage", "invalid appended FIT header");
	fit_size = (size_t)fdt_totalsize(appended_fit);
	if (fit_size < sizeof(struct fdt_header) || fit_size > stage_capacity)
		fail(platform, "stage", "FIT exceeds board staging arena");
	memcpy((void *)platform->fit_stage_start, appended_fit, fit_size);
	if (chain_fit_parse((void *)platform->fit_stage_start, fit_size,
			platform, &plan, &reason))
		fail(platform, "validate", reason);
	debug("chainload: FIT bytes ", fit_size);
	debug("chainload: BL31 segments ", plan.bl31_count);
	if (platform->prepare_handoff && platform->prepare_handoff())
		fail(platform, "prepare", "platform pre-BL31 preparation failed");
	if (chain_fit_load((void *)platform->fit_stage_start, &plan, &reason))
		fail(platform, "load", reason);
	for (unsigned int i = 0; i < plan.image_count; i++)
		if (plan.images[i].role == CHAIN_IMAGE_BL31)
			bl31_bytes += plan.images[i].size;
	params = chain_build_bl31_params(platform->params_addr, plan.bl31_entry,
		bl31_bytes, plan.bl33_entry,
		plan.images[plan.bl33_index].load,
		plan.images[plan.bl33_index].size);

	for (unsigned int i = 0; i < plan.image_count; i++) {
		uintptr_t end = plan.images[i].load + plan.images[i].size;
		if (plan.images[i].role != CHAIN_IMAGE_FDT &&
			(!platform->range_is_cacheable ||
			 platform->range_is_cacheable(plan.images[i].load, end)))
			dcache_clean(plan.images[i].load,
				end);
	}
	dcache_clean(plan.control_fdt,
		plan.control_fdt + plan.control_fdt_capacity);
	dcache_clean(platform->params_addr,
		platform->params_addr + CHAIN_PARAMS_RESERVE);
	puts("chainload: entering BL31");
	chain_jump_bl31(plan.bl31_entry, params);
}
