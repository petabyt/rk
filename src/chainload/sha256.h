/* SPDX-License-Identifier: BSD-3-Clause */
#ifndef RK_CHAINLOAD_SHA256_H
#define RK_CHAINLOAD_SHA256_H

#include <stddef.h>
#include <stdint.h>

#define CHAIN_SHA256_SIZE 32

struct ChainSha256 {
	uint32_t h[8];
	uint64_t total;
	size_t used;
	uint8_t block[128];
};

void chain_sha256_init(struct ChainSha256 *ctx);
void chain_sha256_update(struct ChainSha256 *ctx, const void *data, size_t len);
void chain_sha256_final(struct ChainSha256 *ctx, uint8_t digest[CHAIN_SHA256_SIZE]);
void chain_sha256(const void *data, size_t len, uint8_t digest[CHAIN_SHA256_SIZE]);

#endif
