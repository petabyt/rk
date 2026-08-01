/* SPDX-License-Identifier: Apache-2.0 */
#include <assert.h>
#include <stdint.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chainload/chainload.h"
#include "chainload/sha256.h"
#include "libfdt.h"

static const struct ChainAddressRange bl31_ranges[] = {
	{ 0x00040000, 0x00200000 },
	{ 0xfdcc0000, 0xfdcf0000 },
};

static const struct ChainPlatform platform = {
	.board = "yy3568",
	.soc = "rk3568",
	.fit_stage_start = 0x08000000,
	.fit_stage_end = 0x08400000,
	.params_addr = 0x00100000,
	.expected_bl31_entry = 0x00040000,
	.expected_bl33_entry = 0x00a00000,
	.bl33_limit = 0x00c00000,
	.handoff_protocol = CHAIN_HANDOFF_TFA_V1_BL33_EL2,
	.bl31_ranges = bl31_ranges,
	.bl31_range_count = 2,
	.expected_bl31_segments = 2,
};

static const struct ChainPlatform rock3a_platform = {
	.board = "rock3a",
	.soc = "rk3568",
	.fit_stage_start = 0x08000000,
	.fit_stage_end = 0x08400000,
	.params_addr = 0x00100000,
	.expected_bl31_entry = 0x00040000,
	.expected_bl33_entry = 0x00800000,
	.bl33_limit = 0x03f00000,
	.handoff_protocol = CHAIN_HANDOFF_TFA_V1_BL33_EL2,
	.bl31_ranges = bl31_ranges,
	.bl31_range_count = 2,
	.expected_bl31_segments = 2,
};

uint64_t asm_get_mpidr(void) {
	return 0x80000001;
}

static jmp_buf recovery_jump;
static const char *recovery_stage;
static const char *recovery_reason;

void debug(const char *text, uint64_t value) {
	(void)text;
	(void)value;
}

void dcache_clean(uintptr_t start, uintptr_t end) {
	(void)start;
	(void)end;
}

CHAIN_NORETURN void chain_jump_bl31(uintptr_t entry, void *params) {
	(void)entry;
	(void)params;
	abort();
}

static void test_recover(const char *stage, const char *reason) {
	recovery_stage = stage;
	recovery_reason = reason;
	longjmp(recovery_jump, 1);
}

static void require_ok(int result, const char *reason) {
	if (result) {
		fprintf(stderr, "unexpected parser failure: %s\n", reason);
		abort();
	}
}

static int add_hashed_image(void *fit, const char *name, const char *type,
		const char *os, uintptr_t load, uintptr_t entry,
		const uint8_t *data, int size) {
	uint8_t digest[CHAIN_SHA256_SIZE];
	int images = fdt_path_offset(fit, "/images");
	int image = fdt_add_subnode(fit, images, name);
	int hash;
	assert(image >= 0);
	assert(!fdt_setprop_string(fit, image, "arch", "arm64"));
	assert(!fdt_setprop_string(fit, image, "compression", "none"));
	assert(!fdt_setprop_string(fit, image, "type", type));
	if (os)
		assert(!fdt_setprop_string(fit, image, "os", os));
	if (load != UINTPTR_MAX)
		assert(!fdt_setprop_u32(fit, image, "load", (uint32_t)load));
	if (entry != UINTPTR_MAX)
		assert(!fdt_setprop_u32(fit, image, "entry", (uint32_t)entry));
	assert(!fdt_setprop(fit, image, "data", data, size));
	chain_sha256(data, (size_t)size, digest);
	hash = fdt_add_subnode(fit, image, "hash");
	assert(hash >= 0);
	assert(!fdt_setprop_string(fit, hash, "algo", "sha256"));
	assert(!fdt_setprop(fit, hash, "value", digest, sizeof(digest)));
	return image;
}

static size_t make_fit_for(uint8_t *fit, size_t capacity, uintptr_t bl33_load,
		const char *compatible) {
	static const uint8_t atf0[32] = { 0x31, 0x00, 0x40 };
	static const uint8_t atf1[16] = { 0x31, 0x01, 0x5a };
	static const uint8_t uboot[64] = { 0x55, 0x42, 0x4f, 0x4f, 0x54 };
	static const char loadables[] = "uboot\0atf1";
	uint8_t dtb[1024];
	int dtb_size;
	int configurations, config;
	assert(!fdt_create_empty_tree(dtb, sizeof(dtb)));
	assert(!fdt_setprop_string(dtb, 0, "compatible", compatible));
	assert(!fdt_pack(dtb));
	dtb_size = fdt_totalsize(dtb);
	assert(!fdt_create_empty_tree(fit, (int)capacity));
	assert(fdt_add_subnode(fit, 0, "images") >= 0);
	assert(fdt_add_subnode(fit, 0, "configurations") >= 0);
	add_hashed_image(fit, "atf0", "firmware", "arm-trusted-firmware",
		0x00040000, 0x00040000, atf0, sizeof(atf0));
	add_hashed_image(fit, "atf1", "firmware", "arm-trusted-firmware",
		0xfdcc1000, UINTPTR_MAX, atf1, sizeof(atf1));
	add_hashed_image(fit, "uboot", "standalone", "U-Boot",
		bl33_load, UINTPTR_MAX, uboot, sizeof(uboot));
	add_hashed_image(fit, "fdt", "flat_dt", NULL,
		UINTPTR_MAX, UINTPTR_MAX, dtb, dtb_size);
	configurations = fdt_path_offset(fit, "/configurations");
	assert(!fdt_setprop_string(fit, configurations, "default", "conf"));
	config = fdt_add_subnode(fit, configurations, "conf");
	assert(config >= 0);
	assert(!fdt_setprop_string(fit, config, "firmware", "atf0"));
	assert(!fdt_setprop(fit, config, "loadables", loadables,
		(int)sizeof(loadables)));
	assert(!fdt_setprop_string(fit, config, "fdt", "fdt"));
	return (size_t)fdt_totalsize(fit);
}

static size_t make_fit(uint8_t *fit, size_t capacity) {
	return make_fit_for(fit, capacity, 0x00a00000, "youyeetoo,yy3568");
}

static void test_mainline_fit_policy(void) {
	uint8_t fit[16384];
	struct ChainFitPlan plan;
	const char *reason = NULL;
	int fdt;
	size_t size = make_fit_for(fit, sizeof(fit), 0x00800000, "radxa,rock3a");
	/* Mainline's rockchip-u-boot.dtsi omits arch from flat_dt images. */
	fdt = fdt_path_offset(fit, "/images/fdt");
	assert(fdt >= 0 && !fdt_delprop(fit, fdt, "arch"));
	require_ok(chain_fit_parse(fit, size, &rock3a_platform, &plan, &reason), reason);
	assert(plan.bl33_entry == 0x00800000);
	assert(chain_fit_parse(fit, size, &platform, &plan, &reason));
	assert(reason && strstr(reason, "BL33"));
}

static void test_valid_and_load(void) {
	uint8_t fit[16384], uboot_memory[16384], atf0[64], atf1[64];
	struct ChainFitPlan plan;
	const char *reason = NULL;
	size_t size = make_fit(fit, sizeof(fit));
	require_ok(chain_fit_parse(fit, size, &platform, &plan, &reason), reason);
	assert(plan.image_count == 4 && plan.bl31_count == 2);
	assert(plan.bl31_entry == 0x40000 && plan.bl33_entry == 0xa00000);
	for (unsigned int i = 0; i < plan.image_count; i++) {
		if (plan.images[i].role == CHAIN_IMAGE_BL33)
			plan.images[i].load = (uintptr_t)uboot_memory;
		else if (plan.images[i].role == CHAIN_IMAGE_BL31)
			plan.images[i].load = plan.images[i].entry == 0x40000 ?
				(uintptr_t)atf0 : (uintptr_t)atf1;
	}
	plan.control_fdt = (uintptr_t)(uboot_memory + 128);
	plan.control_fdt_capacity = sizeof(uboot_memory) - 128;
	require_ok(chain_fit_load(fit, &plan, &reason), reason);
	assert(!memcmp(uboot_memory, "UBOOT", 5));
	assert(fdt_path_offset((void *)plan.control_fdt, "/fit-images/uboot") >= 0);
	assert(!fdt_getprop((void *)plan.control_fdt,
		fdt_path_offset((void *)plan.control_fdt, "/fit-images/uboot"),
		"entry-point", NULL));
	assert(fdt_path_offset((void *)plan.control_fdt, "/fit-images/atf1") >= 0);
	assert(fdt_path_offset((void *)plan.control_fdt, "/fit-images/atf0") < 0);
}

static void expect_invalid(void (*mutate)(void *), const char *expected) {
	uint8_t fit[16384];
	struct ChainFitPlan plan;
	const char *reason = NULL;
	size_t size = make_fit(fit, sizeof(fit));
	mutate(fit);
	assert(chain_fit_parse(fit, size, &platform, &plan, &reason));
	assert(reason && strstr(reason, expected));
}

static void corrupt_hash(void *fit) {
	int node = fdt_path_offset(fit, "/images/uboot/hash");
	int len;
	uint8_t *value = (uint8_t *)fdt_getprop_w(fit, node, "value", &len);
	assert(value && len == 32);
	value[0] ^= 1;
}

static void gzip_image(void *fit) {
	int node = fdt_path_offset(fit, "/images/uboot");
	assert(!fdt_setprop_string(fit, node, "compression", "gzip"));
}

static void external_image(void *fit) {
	int node = fdt_path_offset(fit, "/images/uboot");
	assert(!fdt_setprop_u32(fit, node, "data-offset", 0));
}

static void unsafe_atf(void *fit) {
	int node = fdt_path_offset(fit, "/images/atf1");
	assert(!fdt_setprop_u32(fit, node, "load", 0x20000000));
}

static void overlapping_atf(void *fit) {
	int node = fdt_path_offset(fit, "/images/atf1");
	assert(!fdt_setprop_u32(fit, node, "load", 0x00040010));
}

static void wrong_uboot_entry(void *fit) {
	int node = fdt_path_offset(fit, "/images/uboot");
	assert(!fdt_setprop_u32(fit, node, "entry", 0x00a00100));
}

static void wrong_uboot_arch(void *fit) {
	int node = fdt_path_offset(fit, "/images/uboot");
	assert(!fdt_setprop_string(fit, node, "arch", "arm"));
}

static void wrong_fdt_arch(void *fit) {
	int node = fdt_path_offset(fit, "/images/fdt");
	assert(!fdt_setprop_string(fit, node, "arch", "arm"));
}

static void missing_hash(void *fit) {
	int node = fdt_path_offset(fit, "/images/uboot/hash");
	assert(!fdt_del_node(fit, node));
}

static void optee(void *fit) {
	int node = fdt_path_offset(fit, "/images/atf1");
	assert(!fdt_setprop_string(fit, node, "os", "op-tee"));
}

static void missing_split_segment(void *fit) {
	int node = fdt_path_offset(fit, "/configurations/conf");
	assert(!fdt_setprop_string(fit, node, "loadables", "uboot"));
}

static void test_params(void) {
	_Alignas(16) unsigned char buffer[1024];
	uint8_t *params = chain_build_bl31_params((uintptr_t)buffer, 0x40000,
		0x2345, 0xa00000, 0xa00000, 0x1234);
	assert(params == buffer);
	assert(params[0] == 3 && params[1] == 1);
	/* The BL33 entry-point pointer is the fourth pointer after the header. */
	uintptr_t ep = ((uintptr_t *)(buffer + 8))[3];
	assert(ep >= (uintptr_t)buffer && ep < (uintptr_t)(buffer + sizeof(buffer)));
}

static void test_recovery(void) {
	struct ChainPlatform recovering = platform;
	uint8_t invalid[64] = { 0 };
	recovering.recover = test_recover;
	if (!setjmp(recovery_jump))
		chainload_run(&recovering, invalid);
	assert(!strcmp(recovery_stage, "stage"));
	assert(strstr(recovery_reason, "header"));
}

int main(void) {
	uint8_t fit[16384];
	struct ChainFitPlan plan;
	struct ChainPlatform bad_platform;
	const char *reason = NULL;
	size_t size;
	uint8_t digest[32];
	chain_sha256("abc", 3, digest);
	assert(!memcmp(digest,
		"\xba\x78\x16\xbf\x8f\x01\xcf\xea\x41\x41\x40\xde\x5d\xae\x22\x23"
		"\xb0\x03\x61\xa3\x96\x17\x7a\x9c\xb4\x10\xff\x61\xf2\x00\x15\xad", 32));
	test_valid_and_load();
	test_mainline_fit_policy();
	expect_invalid(corrupt_hash, "SHA-256");
	expect_invalid(gzip_image, "compression");
	expect_invalid(external_image, "external");
	expect_invalid(unsafe_atf, "address policy");
	expect_invalid(overlapping_atf, "overlap");
	expect_invalid(wrong_uboot_entry, "BL33");
	expect_invalid(wrong_uboot_arch, "not ARM64");
	expect_invalid(wrong_fdt_arch, "incompatible architecture");
	expect_invalid(missing_hash, "SHA-256");
	expect_invalid(optee, "BL32/OP-TEE");
	expect_invalid(missing_split_segment, "split BL31");
	size = make_fit(fit, sizeof(fit));
	assert(chain_fit_parse(fit, size - 1, &platform, &plan, &reason));
	bad_platform = platform;
	bad_platform.params_addr = 0x00a00010;
	assert(chain_fit_parse(fit, size, &bad_platform, &plan, &reason));
	assert(strstr(reason, "parameters"));
	test_params();
	test_recovery();
	puts("chainloader unit tests passed");
	return 0;
}
