// Tool to create sd bootable images for rk3399 and rk3588
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

	// init size must be padded to 4kb
	unsigned int init_size = (((file_size(ddr_file) / 512) / 4) + 1) * 4;
	unsigned int os_size = (file_size(main_file) / 512) + 2;

	struct RkHeaderV1 hdr;
	memset(&hdr, '\0', sizeof(hdr));
	hdr.signature = RK33_V1_MAGIC;
	hdr.disable_rc4 = 1;
	hdr.init_offset = 4;
	hdr.init_size = init_size;
	hdr.init_boot_size = init_size + os_size;

	rc4_encode((uint8_t *)&hdr, sizeof(hdr), rockchip_key);

	fwrite(&hdr, 1, 512, o);

	// TPL @ 0x8800, sets up sdram
	fseek(o, 0x8800, SEEK_SET);
	write_file(o, ddr_file, 0);

	// Main OS image directly after DDR image
	fseek(o, (init_size * 512) + 0x8800, SEEK_SET);
	write_file(o, main_file, 0);

	printf("Finished writing to %s\n", out_file);
	fclose(o);

	return 0;
}

int make_v2(const char *out_file, const char *ddr_file, const char *main_file) {
	FILE *o = fopen(out_file, "wb");

	unsigned int init_size = (file_size(ddr_file) / 512) + 1;
	unsigned int os_size = (file_size(main_file) / 512) + 1;

	// not necessary
	os_size = ((os_size / 8) + 1) * 8;

	struct RkHeaderV2 hdr;
	memset(&hdr, '\0', sizeof(hdr));
	hdr.signature = RKNS_V2_MAGIC;
	hdr.hash_field_offset = 0x180;
	hdr.n_images = 2;
	hdr.boot_flag = RK_NO_CHECKSUM;
	hdr.images[0].offset = 4; // 4 sectors after header
	hdr.images[0].size = init_size;
	hdr.images[1].offset = init_size + 4;
	hdr.images[1].size = os_size;

	fseek(o, 0x8000, SEEK_SET);
	fwrite(&hdr, 1, sizeof(hdr), o);

	fseek(o, 0x8800, SEEK_SET);
	write_file(o, ddr_file, 0);

	fseek(o, (init_size * 512) + 0x8800, SEEK_SET);
	write_file(o, main_file, 0);

	// pad file, xrock flash write didn't work if not padded
	fseek(o, (init_size * 512) + (os_size * 512) + 0x8800 - 4, SEEK_SET);
	uint32_t bit = 0x0;
	fwrite(&bit, 1, 4, o);

	printf("Finished writing to %s\n", out_file);
	fclose(o);

	return 0;
}

int decode(const char *image) {
	void *header_buf = malloc(2048);
	FILE *f = fopen(image, "rb");
	if (f == NULL) return -1;

	{
		fseek(f, 0x8000, SEEK_SET);
		fread(header_buf, 1, 512, f);

		struct RkHeaderV1 *hdr = header_buf;
		rc4_encode((uint8_t *)hdr, 512, rockchip_key);
		if (hdr->signature == RK33_V1_MAGIC) {
			printf("signature: %X\n", hdr->signature);
			printf("disable_rc4: %X\n", hdr->disable_rc4);
			printf("init_offset: %X\n", hdr->init_offset);
			printf("init_size: %X\n", hdr->init_size);
			printf("init_boot_size: %X\n", hdr->init_boot_size);
			printf("reserved: %X\n", hdr->reserved);
			printf("reserved: %X\n", hdr->reserved1[100]);
			return 0;
		}
	}
	{
		fseek(f, 0x8000, SEEK_SET);
		fread(header_buf, 1, 2048, f);

		struct RkHeaderV2 *hdr = header_buf;
		if (hdr->signature == RKNS_V2_MAGIC) {
			printf("hash_field_offset: %X\n", hdr->hash_field_offset);
			printf("boot_flag: %X\n", hdr->boot_flag);
			for (unsigned int i = 0; i < hdr->n_images; i++) {
				printf("Image #%u: offset: %u, size: %u\n", i, hdr->images[i].offset, hdr->images[i].size);
			}
			return 0;
		}
	}

	printf("Didn't recognize image\n");

	fclose(f);
	free(header_buf);

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
