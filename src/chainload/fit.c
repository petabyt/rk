/* SPDX-License-Identifier: Apache-2.0 */
#include <stdint.h>
#include <string.h>

#include "chainload.h"
#include "sha256.h"
#include "libfdt.h"

static int string_equal(const char *a, const char *b) {
	while (*a && *a == *b) {
		a++;
		b++;
	}
	return *a == *b;
}

static const char *string_prop(const void *fit, int node, const char *name) {
	int len;
	const char *value = fdt_getprop(fit, node, name, &len);
	if (!value || len < 1 || !memchr(value, 0, (size_t)len))
		return NULL;
	return value;
}

static int address_prop(const void *fit, int node, const char *name,
		uintptr_t *value, int required) {
	int len;
	const fdt32_t *cells = fdt_getprop(fit, node, name, &len);
	if (!cells)
		return required ? -1 : 1;
	if (len == 4) {
		*value = fdt32_to_cpu(cells[0]);
		return 0;
	}
	if (len == 8) {
		uint64_t v = ((uint64_t)fdt32_to_cpu(cells[0]) << 32) |
			fdt32_to_cpu(cells[1]);
		if (v > UINTPTR_MAX)
			return -1;
		*value = (uintptr_t)v;
		return 0;
	}
	return -1;
}

static int checked_end(uintptr_t start, size_t size, uintptr_t *end) {
	if (size > UINTPTR_MAX - start)
		return -1;
	*end = start + size;
	return 0;
}

static int in_range(uintptr_t start, uintptr_t end,
		const struct ChainAddressRange *range) {
	return start >= range->start && end > start && end <= range->end;
}

static int bl31_address_allowed(const struct ChainPlatform *platform,
		uintptr_t start, uintptr_t end) {
	for (unsigned int i = 0; i < platform->bl31_range_count; i++)
		if (in_range(start, end, &platform->bl31_ranges[i]))
			return 1;
	return 0;
}

static int validate_hash(const void *fit, int image, const void *data,
		size_t size) {
	uint8_t digest[CHAIN_SHA256_SIZE];
	int hash;
	int valid_hashes = 0;
	fdt_for_each_subnode(hash, fit, image) {
		const char *algo = string_prop(fit, hash, "algo");
		int len;
		const uint8_t *expected;
		if (!algo || !string_equal(algo, "sha256"))
			continue;
		expected = fdt_getprop(fit, hash, "value", &len);
		if (!expected || len != CHAIN_SHA256_SIZE)
			return -1;
		chain_sha256(data, size, digest);
		if (memcmp(expected, digest, sizeof(digest)))
			return -1;
		valid_hashes++;
	}
	return valid_hashes == 1 ? 0 : -1;
}

static int add_image(const void *fit, int images, const char *name,
		int primary, const struct ChainPlatform *platform, struct ChainFitPlan *plan,
		const char **reason) {
	struct ChainImage *out;
	const char *arch, *compression, *type, *os;
	const void *data;
	uintptr_t end;
	int node, len, entry_status;

	if (plan->image_count == CHAIN_MAX_IMAGES) {
		*reason = "too many FIT images";
		return -1;
	}
	node = fdt_subnode_offset(fit, images, name);
	if (node < 0) {
		*reason = "configuration references a missing image";
		return -1;
	}
	if (fdt_getprop(fit, node, "data-position", NULL) ||
		fdt_getprop(fit, node, "data-offset", NULL)) {
		*reason = "external FIT data is not supported";
		return -1;
	}
	arch = string_prop(fit, node, "arch");
	compression = string_prop(fit, node, "compression");
	type = string_prop(fit, node, "type");
	os = string_prop(fit, node, "os");
	if (!compression || !string_equal(compression, "none")) {
		*reason = "FIT compression is not supported";
		return -1;
	}
	data = fdt_getprop(fit, node, "data", &len);
	if (!data || len <= 0) {
		*reason = "FIT image has no inline data";
		return -1;
	}
	if (validate_hash(fit, node, data, (size_t)len)) {
		*reason = "FIT SHA-256 verification failed";
		return -1;
	}

	out = &plan->images[plan->image_count];
	memset(out, 0, sizeof(*out));
	out->name = name;
	out->data = data;
	out->size = (size_t)len;
	out->entry = UINTPTR_MAX;
	out->type = type;
	out->os = os;
	out->record_in_control_fdt = primary ? 0 : 1;
	entry_status = address_prop(fit, node, "entry", &out->entry, 0);
	if (entry_status < 0) {
		*reason = "invalid FIT entry point";
		return -1;
	}
	out->entry_explicit = entry_status == 0;

	if (type && string_equal(type, "flat_dt")) {
		/*
		 * Mainline U-Boot's Rockchip binman template intentionally omits
		 * the architecture property from flat_dt images.  A control DTB is
		 * data rather than executable code, so accept that canonical form;
		 * if an architecture is supplied, still reject a conflicting one.
		 */
		if (arch && !string_equal(arch, "arm64")) {
			*reason = "control DTB has an incompatible architecture";
			return -1;
		}
		if (fdt_getprop(fit, node, "load", NULL) ||
			out->entry != UINTPTR_MAX) {
			*reason = "control DTB must not carry a load or entry address";
			return -1;
		}
		out->role = CHAIN_IMAGE_FDT;
		out->record_in_control_fdt = 0;
		if (plan->fdt_index != UINT32_MAX) {
			*reason = "multiple control DTBs are not supported";
			return -1;
		}
		plan->fdt_index = plan->image_count++;
		return 0;
	}

	/* BL31 and BL33 are executable AArch64 images. */
	if (!arch || !string_equal(arch, "arm64")) {
		*reason = "FIT executable image is not ARM64";
		return -1;
	}

	if (address_prop(fit, node, "load", &out->load, 1) ||
		checked_end(out->load, out->size, &end)) {
		*reason = "invalid FIT load range";
		return -1;
	}
	if (os && (string_equal(os, "tee") || string_equal(os, "op-tee"))) {
		*reason = "BL32/OP-TEE is not supported";
		return -1;
	}
	if (os && (string_equal(os, "U-Boot") || string_equal(os, "u-boot"))) {
		if (!type || !string_equal(type, "standalone")) {
			*reason = "BL33 is not a standalone U-Boot image";
			return -1;
		}
		if (out->entry == UINTPTR_MAX)
			out->entry = out->load;
		out->role = CHAIN_IMAGE_BL33;
		if (plan->bl33_index != UINT32_MAX ||
			out->load != platform->expected_bl33_entry ||
			out->entry != platform->expected_bl33_entry ||
			end > platform->bl33_limit) {
			*reason = "unsafe or duplicate BL33 image";
			return -1;
		}
		plan->bl33_index = plan->image_count;
		plan->bl33_entry = out->entry;
	} else if (os && string_equal(os, "arm-trusted-firmware")) {
		if (!type || !string_equal(type, "firmware")) {
			*reason = "BL31 segment is not firmware";
			return -1;
		}
		if (primary && out->entry == UINTPTR_MAX)
			out->entry = out->load;
		out->role = CHAIN_IMAGE_BL31;
		if (!bl31_address_allowed(platform, out->load, end)) {
			*reason = "BL31 segment violates board address policy";
			return -1;
		}
		if (out->entry != UINTPTR_MAX) {
			if (out->entry != platform->expected_bl31_entry || plan->bl31_entry) {
				*reason = "invalid BL31 entry point";
				return -1;
			}
			plan->bl31_entry = out->entry;
		}
		plan->bl31_count++;
	} else {
		*reason = "unsupported FIT operating system";
		return -1;
	}
	plan->image_count++;
	return 0;
}

static int images_overlap(const struct ChainImage *a, const struct ChainImage *b) {
	uintptr_t a_end, b_end;
	if (a->role == CHAIN_IMAGE_FDT || b->role == CHAIN_IMAGE_FDT)
		return 0;
	if (checked_end(a->load, a->size, &a_end) || checked_end(b->load, b->size, &b_end))
		return 1;
	return a->load < b_end && b->load < a_end;
}

int chain_fit_parse(const void *fit, size_t available,
		const struct ChainPlatform *platform, struct ChainFitPlan *plan,
		const char **reason) {
	const char *name;
	int configurations, configuration, images, count, len;
	size_t total;

	*reason = "invalid FIT header";
	if (!fit || !platform || fdt_check_header(fit))
		return -1;
	total = (size_t)fdt_totalsize(fit);
	if (total > available || total < sizeof(struct fdt_header))
		return -1;
	memset(plan, 0, sizeof(*plan));
	plan->bl33_index = UINT32_MAX;
	plan->fdt_index = UINT32_MAX;
	configurations = fdt_path_offset(fit, "/configurations");
	images = fdt_path_offset(fit, "/images");
	if (configurations < 0 || images < 0) {
		*reason = "FIT lacks images or configurations";
		return -1;
	}
	name = string_prop(fit, configurations, "default");
	configuration = name ? fdt_subnode_offset(fit, configurations, name) : -1;
	if (configuration < 0) {
		*reason = "FIT has no valid default configuration";
		return -1;
	}
	name = fdt_stringlist_get(fit, configuration, "firmware", 0, &len);
	if (!name || add_image(fit, images, name, 1, platform, plan, reason))
		return -1;
	count = fdt_stringlist_count(fit, configuration, "loadables");
	if (count < 1) {
		*reason = "FIT has no loadables";
		return -1;
	}
	for (int i = 0; i < count; i++) {
		name = fdt_stringlist_get(fit, configuration, "loadables", i, &len);
		if (!name || add_image(fit, images, name, 0, platform, plan, reason))
			return -1;
	}
	name = fdt_stringlist_get(fit, configuration, "fdt", 0, &len);
	if (!name || add_image(fit, images, name, 0, platform, plan, reason))
		return -1;
	if (!plan->bl31_count || !plan->bl31_entry ||
		plan->bl33_index == UINT32_MAX || plan->fdt_index == UINT32_MAX) {
		*reason = "FIT is missing BL31, BL33, or its control DTB";
		return -1;
	}
	if (platform->expected_bl31_segments &&
		plan->bl31_count != platform->expected_bl31_segments) {
		*reason = "unexpected number of split BL31 segments";
		return -1;
	}
	for (unsigned int i = 0; i < plan->image_count; i++)
		for (unsigned int j = i + 1; j < plan->image_count; j++)
			if (images_overlap(&plan->images[i], &plan->images[j])) {
				*reason = "FIT load ranges overlap";
				return -1;
			}
	{
		struct ChainImage *uboot = &plan->images[plan->bl33_index];
		struct ChainImage *dtb = &plan->images[plan->fdt_index];
		uintptr_t dtb_end, params_end;
		plan->control_fdt = uboot->load + uboot->size;
		plan->control_fdt_capacity = dtb->size + CHAIN_DTB_SLACK;
		if (checked_end(plan->control_fdt, plan->control_fdt_capacity, &dtb_end) ||
			dtb_end > platform->bl33_limit) {
			*reason = "U-Boot plus control DTB exceeds its initial stack limit";
			return -1;
		}
		if (checked_end(platform->params_addr, CHAIN_PARAMS_RESERVE, &params_end)) {
			*reason = "invalid BL31 parameter range";
			return -1;
		}
		for (unsigned int i = 0; i < plan->image_count; i++) {
			uintptr_t image_end;
			if (plan->images[i].role == CHAIN_IMAGE_FDT)
				continue;
			if (checked_end(plan->images[i].load, plan->images[i].size, &image_end) ||
				(plan->images[i].load < platform->fit_stage_end &&
				 platform->fit_stage_start < image_end) ||
				(plan->images[i].load < params_end &&
				 platform->params_addr < image_end)) {
				*reason = "loaded image overlaps staging or BL31 parameters";
				return -1;
			}
		}
		if ((plan->control_fdt < platform->fit_stage_end &&
			 platform->fit_stage_start < dtb_end) ||
			(plan->control_fdt < params_end && platform->params_addr < dtb_end)) {
			*reason = "control DTB overlaps staging or BL31 parameters";
			return -1;
		}
	}
	*reason = NULL;
	return 0;
}

static int record_image(void *dtb, const struct ChainImage *image) {
	int parent = fdt_subnode_offset(dtb, 0, "fit-images");
	int node;
	if (parent == -FDT_ERR_NOTFOUND)
		parent = fdt_add_subnode(dtb, 0, "fit-images");
	if (parent < 0)
		return parent;
	node = fdt_subnode_offset(dtb, parent, image->name);
	if (node == -FDT_ERR_NOTFOUND)
		node = fdt_add_subnode(dtb, parent, image->name);
	if (node < 0)
		return node;
	if (fdt_setprop_u32(dtb, node, "load-addr", (uint32_t)image->load) ||
		fdt_setprop_u32(dtb, node, "size", (uint32_t)image->size))
		return -1;
	if (image->entry_explicit &&
		fdt_setprop_u32(dtb, node, "entry-point", (uint32_t)image->entry))
		return -1;
	if (image->type && fdt_setprop_string(dtb, node, "type", image->type))
		return -1;
	if (image->os && fdt_setprop_string(dtb, node, "os", image->os))
		return -1;
	return 0;
}

int chain_fit_load(const void *fit, struct ChainFitPlan *plan,
		const char **reason) {
	(void)fit;
	struct ChainImage *dtb = &plan->images[plan->fdt_index];
	for (unsigned int i = 0; i < plan->image_count; i++) {
		struct ChainImage *image = &plan->images[i];
		if (image->role != CHAIN_IMAGE_FDT)
			memcpy((void *)image->load, image->data, image->size);
	}
	if (fdt_open_into(dtb->data, (void *)plan->control_fdt,
			(int)plan->control_fdt_capacity)) {
		*reason = "cannot expand U-Boot control DTB";
		return -1;
	}
	for (unsigned int i = 0; i < plan->image_count; i++) {
		if (plan->images[i].record_in_control_fdt &&
			record_image((void *)plan->control_fdt, &plan->images[i])) {
			*reason = "cannot create /fit-images metadata";
			return -1;
		}
	}
	*reason = NULL;
	return 0;
}
