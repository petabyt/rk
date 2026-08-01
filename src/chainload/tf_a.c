/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * TF-A v1 parameter structures are derived from upstream U-Boot's
 * include/atf_common.h (ARM Limited, Rockchip, and Theobroma Systems).
 */
#include <stdint.h>
#include <string.h>

#include "chainload.h"

uint64_t asm_get_mpidr(void);

#define ATF_PARAM_EP 0x01
#define ATF_PARAM_IMAGE_BINARY 0x02
#define ATF_PARAM_BL31 0x03
#define ATF_VERSION_1 0x01
#define ATF_EP_NON_SECURE 0x01
#define SPSR_EL2H_MASKED 0x3c9U

struct ParamHeader {
	uint8_t type;
	uint8_t version;
	uint16_t size;
	uint32_t attr;
};

struct Aapcs64Params {
	unsigned long arg[8];
};

struct EntryPointInfo {
	struct ParamHeader h;
	uintptr_t pc;
	uint32_t spsr;
	uint32_t pad;
	struct Aapcs64Params args;
};

struct ImageInfo {
	struct ParamHeader h;
	uintptr_t image_base;
	uint32_t image_size;
	uint32_t pad;
};

struct Bl31Params {
	struct ParamHeader h;
	struct ImageInfo *bl31_image_info;
	struct EntryPointInfo *bl32_ep_info;
	struct ImageInfo *bl32_image_info;
	struct EntryPointInfo *bl33_ep_info;
	struct ImageInfo *bl33_image_info;
};

struct Bl31ParamMemory {
	struct Bl31Params params;
	struct ImageInfo bl31_image;
	struct ImageInfo bl33_image;
	struct EntryPointInfo bl33_ep;
};

static void header(struct ParamHeader *h, uint8_t type, uint16_t size,
		uint32_t attr) {
	h->type = type;
	h->version = ATF_VERSION_1;
	h->size = size;
	h->attr = attr;
}

void *chain_build_bl31_params(uintptr_t address, uintptr_t bl31_entry,
		size_t bl31_size, uintptr_t bl33_entry, uintptr_t bl33_base,
		size_t bl33_size) {
	struct Bl31ParamMemory *memory = (void *)address;
	memset(memory, 0, sizeof(*memory));
	header(&memory->params.h, ATF_PARAM_BL31, sizeof(memory->params), 0);
	header(&memory->bl31_image.h, ATF_PARAM_IMAGE_BINARY,
		sizeof(memory->bl31_image), 0);
	header(&memory->bl33_image.h, ATF_PARAM_IMAGE_BINARY,
		sizeof(memory->bl33_image), 0);
	header(&memory->bl33_ep.h, ATF_PARAM_EP, sizeof(memory->bl33_ep),
		ATF_EP_NON_SECURE);
	memory->bl31_image.image_base = bl31_entry;
	memory->bl31_image.image_size = (uint32_t)bl31_size;
	memory->bl33_image.image_base = bl33_base;
	memory->bl33_image.image_size = (uint32_t)bl33_size;
	memory->bl33_ep.pc = bl33_entry;
	memory->bl33_ep.spsr = SPSR_EL2H_MASKED;
	memory->bl33_ep.args.arg[0] = asm_get_mpidr() & 0xffff;
	memory->params.bl31_image_info = &memory->bl31_image;
	memory->params.bl32_ep_info = NULL;
	memory->params.bl32_image_info = NULL;
	memory->params.bl33_ep_info = &memory->bl33_ep;
	memory->params.bl33_image_info = &memory->bl33_image;
	return &memory->params;
}
