/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * SHA-256 implementation based on Olivier Gay's BSD-licensed implementation
 * as carried by upstream U-Boot in lib/avb/libavb/avb_sha256.c.
 * Copyright (C) 2005, 2007 Olivier Gay <olivier.gay@a3.epfl.ch>
 */
#include "sha256.h"

#define ROR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
#define CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define S0(x) (ROR((x), 2) ^ ROR((x), 13) ^ ROR((x), 22))
#define S1(x) (ROR((x), 6) ^ ROR((x), 11) ^ ROR((x), 25))
#define G0(x) (ROR((x), 7) ^ ROR((x), 18) ^ ((x) >> 3))
#define G1(x) (ROR((x), 17) ^ ROR((x), 19) ^ ((x) >> 10))

static const uint32_t initial[8] = {
	0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
	0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19,
};

static const uint32_t constants[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
	0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
	0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
	0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
	0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
	0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
	0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
	0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
	0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
};

static void copy_bytes(uint8_t *dst, const uint8_t *src, size_t len) {
	while (len--)
		*dst++ = *src++;
}

static void transform(struct ChainSha256 *ctx, const uint8_t *data) {
	uint32_t w[64], a, b, c, d, e, f, g, h;
	for (unsigned int i = 0; i < 16; i++) {
		w[i] = ((uint32_t)data[i * 4] << 24) |
			((uint32_t)data[i * 4 + 1] << 16) |
			((uint32_t)data[i * 4 + 2] << 8) | data[i * 4 + 3];
	}
	for (unsigned int i = 16; i < 64; i++)
		w[i] = G1(w[i - 2]) + w[i - 7] + G0(w[i - 15]) + w[i - 16];
	a = ctx->h[0]; b = ctx->h[1]; c = ctx->h[2]; d = ctx->h[3];
	e = ctx->h[4]; f = ctx->h[5]; g = ctx->h[6]; h = ctx->h[7];
	for (unsigned int i = 0; i < 64; i++) {
		uint32_t t1 = h + S1(e) + CH(e, f, g) + constants[i] + w[i];
		uint32_t t2 = S0(a) + MAJ(a, b, c);
		h = g; g = f; f = e; e = d + t1;
		d = c; c = b; b = a; a = t1 + t2;
	}
	ctx->h[0] += a; ctx->h[1] += b; ctx->h[2] += c; ctx->h[3] += d;
	ctx->h[4] += e; ctx->h[5] += f; ctx->h[6] += g; ctx->h[7] += h;
}

void chain_sha256_init(struct ChainSha256 *ctx) {
	for (unsigned int i = 0; i < 8; i++)
		ctx->h[i] = initial[i];
	ctx->total = 0;
	ctx->used = 0;
}

void chain_sha256_update(struct ChainSha256 *ctx, const void *data_, size_t len) {
	const uint8_t *data = data_;
	ctx->total += len;
	while (len) {
		size_t amount = 64 - ctx->used;
		if (amount > len)
			amount = len;
		copy_bytes(ctx->block + ctx->used, data, amount);
		ctx->used += amount;
		data += amount;
		len -= amount;
		if (ctx->used == 64) {
			transform(ctx, ctx->block);
			ctx->used = 0;
		}
	}
}

void chain_sha256_final(struct ChainSha256 *ctx, uint8_t digest[CHAIN_SHA256_SIZE]) {
	uint64_t bits = ctx->total * 8;
	ctx->block[ctx->used++] = 0x80;
	if (ctx->used > 56) {
		while (ctx->used < 64)
			ctx->block[ctx->used++] = 0;
		transform(ctx, ctx->block);
		ctx->used = 0;
	}
	while (ctx->used < 56)
		ctx->block[ctx->used++] = 0;
	for (unsigned int i = 0; i < 8; i++)
		ctx->block[56 + i] = bits >> (56 - i * 8);
	transform(ctx, ctx->block);
	for (unsigned int i = 0; i < 8; i++) {
		digest[i * 4] = ctx->h[i] >> 24;
		digest[i * 4 + 1] = ctx->h[i] >> 16;
		digest[i * 4 + 2] = ctx->h[i] >> 8;
		digest[i * 4 + 3] = ctx->h[i];
	}
}

void chain_sha256(const void *data, size_t len, uint8_t digest[CHAIN_SHA256_SIZE]) {
	struct ChainSha256 ctx;
	chain_sha256_init(&ctx);
	chain_sha256_update(&ctx, data, len);
	chain_sha256_final(&ctx, digest);
}
