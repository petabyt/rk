#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define RK33_V1_MAGIC 0x33334b52
#define RKNS_V2_MAGIC 0x534e4b52

// Rockchip's encryption key
static const unsigned char rc4_key[16] = {
	124, 78, 3, 4, 85, 5, 9, 7, 45, 44, 123, 56, 23, 13, 23, 17
};

struct __attribute__((packed)) RkHeaderV1 {
	uint32_t signature;
	uint32_t reserved;
	uint32_t disable_rc4;
	uint16_t init_offset;
	uint8_t reserved1[492];
	uint16_t init_size;
	uint16_t init_boot_size;
	uint16_t reserved2;
	// (512 bytes)
};

struct __attribute__((packed)) RkHeaderV2 {
	uint32_t signature;
	uint32_t res1;
	uint16_t hash_field_offset;
	uint16_t n_images;
	uint32_t boot_flag;
	uint8_t res2[0x68];
	struct ImageEntry {
		uint16_t offset;
		uint16_t size;
		uint16_t address;
		uint32_t flag;
		uint32_t counter;
		uint8_t res[12];
		uint8_t hash[0x40];
	}images[4];
};

// Generic RC4 encode algorithm
void rc4_encode(uint8_t *buf, size_t len, const uint8_t key[16]) {
    uint8_t S[256];
    for (int i = 0; i < 256; i++) {
        S[i] = i;
    }

    int j = 0;
    for (int i = 0; i < 256; i++) {
        j = (j + S[i] + key[i % 16]) % 256;
        uint8_t temp = S[i];
        S[i] = S[j];
        S[j] = temp;
    }

    int i = 0;
    j = 0;

    for (size_t k = 0; k < len; k++) {
        i = (i + 1) % 256;
        j = (j + S[i]) % 256;
        uint8_t temp = S[i];
        S[i] = S[j];
        S[j] = temp;

        uint8_t key_byte = S[(S[i] + S[j]) % 256];
        buf[k] ^= key_byte;
    }
}

int write_file(FILE *f, const char *filename, int offset) {
	FILE *source_file = fopen(filename, "rb");
	if (!source_file) {
		printf("%s not found\n", filename);
		exit(1);
	}

	fseek(source_file, 0, SEEK_END);
	long file_size = ftell(source_file) - offset;
	fseek(source_file, offset, SEEK_SET);

	char *buffer = malloc(file_size);

	fread(buffer, 1, file_size, source_file);
	fwrite(buffer, 1, file_size, f);

	fclose(source_file);
	free(buffer);

	return 0;
}

int make(const char *out_file, const char *tpl_file, const char *main_file) {
	FILE *o = fopen(out_file, "wb");

	// write rc4 encrpted data @ 0x8000
	fseek(o, 0x8000, SEEK_SET);

	// Size of ddr image
	#define INIT_SIZE (90000 / 512 + 1) 

	// Size of main sdram image
	#define OS_SIZE (0x400)

	#define DDR_OFFSET 0x8800

	struct RkHeaderV1 hdr;
	memset(&hdr, '\0', sizeof(hdr));
	hdr.signature = 0x0ff0aa55;
	hdr.disable_rc4 = 1;
	hdr.init_offset = 4;
	hdr.init_size = INIT_SIZE; // 71kb / 512 + padding
	hdr.init_boot_size = OS_SIZE; // approx size of os.bin

	rc4_encode((uint8_t *)&hdr, sizeof(hdr), rc4_key);

	fwrite(&hdr, 1, 512, o);

	// "RK33" required signature
	uint32_t rk33 = 0x33334b52;

	// TPL @ 0x8800, sets up sdram
	fseek(o, DDR_OFFSET, SEEK_SET);
	fwrite(&rk33, 1, 4, o);
	write_file(o, tpl_file, 4);

	int secondary_pos = (INIT_SIZE * 512) + DDR_OFFSET;

	fseek(o, secondary_pos, SEEK_SET);
	write_file(o, main_file, 0);

	printf("Finished writing to %s\n", out_file);
	fclose(o);

	return 0;
}

int decode(const char *image) {
	FILE *f = fopen(image, "rb");

	fseek(f, 0x8000, SEEK_SET);

	struct RkHeaderV1 *hdr = malloc(512);

	fread(hdr, 1, 512, f);
	fclose(f);

	rc4_encode((uint8_t *)hdr, 512, rc4_key);

	printf("signature: %X\n", hdr->signature);
	printf("disable_rc4: %X\n", hdr->disable_rc4);
	printf("init_offset: %X\n", hdr->init_offset);
	printf("init_size: %X\n", hdr->init_size);
	printf("init_boot_size: %X\n", hdr->init_boot_size);
	printf("reserved: %X\n", hdr->reserved);
	printf("reserved: %X\n", hdr->reserved1[100]);

	return 0;
}

int main(int argc, char *argv[]) {
	const char *out_file = "pine.img";
	const char *tpl_file = "ddr.bin";
	const char *main_file = "os.bin";

	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-m")) {
			out_file = argv[i + 1];
			tpl_file = argv[i + 2];
		} else if (!strcmp(argv[i], "-d")) {
			out_file = argv[i + 1];
			return decode(out_file);
		}
	}

	return make(out_file, tpl_file, main_file);
}
