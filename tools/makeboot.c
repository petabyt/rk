#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "main.h"

static unsigned int file_size(const char *filename) {
	FILE *f = fopen(filename, "rb");
	if (!f) {
		printf("%s not found\n", filename);
		exit(1);
	}
	fseek(f, 0, SEEK_END);
	long file_size = ftell(f);
	fclose(f);
	return file_size;
}

static int write_file(FILE *f, const char *filename, int offset) {
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

int make_v1(const char *out_file, const char *ddr_file, const char *main_file) {
	FILE *o = fopen(out_file, "wb");

	// write rc4 encrpted data @ 0x8000
	fseek(o, 0x8000, SEEK_SET);

	unsigned int init_size = (file_size(ddr_file) / 512) + 1;
	unsigned int os_size = (file_size(main_file) / 512) + 1;

	struct RkHeaderV1 hdr;
	memset(&hdr, '\0', sizeof(hdr));
	hdr.signature = 0x0ff0aa55;
	hdr.disable_rc4 = 1;
	hdr.init_offset = 4;
	hdr.init_size = init_size;
	hdr.init_boot_size = os_size;

	rc4_encode((uint8_t *)&hdr, sizeof(hdr), rockchip_key);

	fwrite(&hdr, 1, 512, o);

	// "RK33" required signature
	uint32_t rk33 = RK33_V1_MAGIC;

	// TPL @ 0x8800, sets up sdram
	fseek(o, 0x8800, SEEK_SET);
	fwrite(&rk33, 1, 4, o);
	write_file(o, ddr_file, 4);

	// Main OS image directly after DDR image
	fseek(o, (os_size * 512) + 0x8800, SEEK_SET);
	write_file(o, main_file, 0);

	printf("Finished writing to %s\n", out_file);
	fclose(o);

	return 0;
}

int make_v2(const char *out_file, const char *ddr_file, const char *main_file) {
	FILE *o = fopen(out_file, "wb");

	unsigned int init_size = (file_size(ddr_file) / 512) + 1;
	unsigned int os_size = (file_size(main_file) / 512) + 1;

	struct RkHeaderV2 hdr;
	memset(&hdr, '\0', sizeof(hdr));
	hdr.signature = RKNS_V2_MAGIC;
	hdr.hash_field_offset = 0x180;
	hdr.n_images = 2;
	hdr.boot_flag = 0; // no checksums
	hdr.images[0].offset = 4; // 4 sectors after header
	hdr.images[0].size = init_size;
	hdr.images[1].offset = init_size + 4;
	hdr.images[1].size = os_size;

	fseek(o, 0x8000, SEEK_SET);
	fwrite(&hdr, 1, sizeof(hdr), o);

	fseek(o, 0x8800, SEEK_SET);
	write_file(o, ddr_file, 0);

	fseek(o, (os_size * 512) + 0x8800, SEEK_SET);
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

	rc4_encode((uint8_t *)hdr, 512, rockchip_key);

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
	int version = 1;
	const char *out_file = "output.img";
	const char *ddr_file = "ddr.bin";
	const char *main_file = "os.bin";

	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--v2")) {
			version = 2;
		} else if (!strcmp(argv[i], "--v1")) {
			version = 1;
		} else if (!strcmp(argv[i], "--ddr")) {
			ddr_file = argv[i + 1];
			i++;
		} else if (!strcmp(argv[i], "--os")) {
			main_file = argv[i + 1];
			i++;
		} else if (!strcmp(argv[i], "--decode")) {
			return decode(argv[i + 1]);
		} else if (!strcmp(argv[i], "-o")) {
			out_file = argv[i + 1];
			i++;
		} else {
			printf("Unknown arg '%s'\n", argv[i]);
		}
	}

	if (version == 1) {
		return make_v1(out_file, ddr_file, main_file);
	} else {
		return make_v2(out_file, ddr_file, main_file);
	}
}
