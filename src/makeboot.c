#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// https://en.wikipedia.org/wiki/Master_boot_record
struct StandardMbr {
	uint8_t bootstrap_area[0x01B8];
	uint32_t disk_signature; 
	uint16_t write_protection;
	struct PartitionTables {
		uint8_t entry[16];
	}partition_table[4];
	uint16_t boot_magic;
	// (512 bytes)
};

// MBR sector "bootstrap area"
const unsigned char mbr_dummy_boostrap_area[] = {0xfa, 0xb8, 0x0, 0x10, 0x8e, 0xd0, 0xbc, 0x0, 0xb0, 0xb8, 0x0, 0x0, 0x8e, 0xd8, 0x8e, 0xc0, 0xfb,
0xbe, 0x0, 0x7c, 0xbf, 0x0, 0x6, 0xb9, 0x0, 0x2, 0xf3, 0xa4, 0xea, 0x21, 0x6, 0x0, 0x0, 0xbe, 0xbe, 0x7, 0x38, 0x4, 0x75, 0xb, 0x83, 0xc6,
0x10, 0x81, 0xfe, 0xfe, 0x7, 0x75, 0xf3, 0xeb, 0x16, 0xb4, 0x2, 0xb0, 0x1, 0xbb, 0x0, 0x7c, 0xb2, 0x80, 0x8a, 0x74, 0x1, 0x8b, 0x4c, 0x2,
0xcd, 0x13, 0xea, 0x0, 0x7c, 0x0, 0x0, 0xeb, 0xfe};

// Partition 1 info for mbr device
const unsigned char mbr_partition_data[] = {0x0, 0x0, 0x1, 0xc0, 0xe, 0x3, 0x60, 0xff, 0x0, 0x60, 0x0, 0x0, 0x0, 0xa0, 0x0, 0x0, };

// Rockchip's encryption key
const unsigned char rc4_key[16] = {
	124, 78, 3, 4, 85, 5, 9, 7, 45, 44, 123, 56, 23, 13, 23, 17
};


struct RkHeader {
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

	// Start with the DOS/MBR boot sector - not necessary, but helps identify the file as a burnable image
	struct StandardMbr mbr;
	memset(&mbr, 0, sizeof(mbr));
	memcpy(mbr.bootstrap_area, mbr_dummy_boostrap_area, sizeof(mbr_dummy_boostrap_area));
	mbr.disk_signature = 0x2f6a2f0a;
	mbr.write_protection = 0x0;
	memcpy(mbr.partition_table[0].entry, mbr_partition_data, 16);
	mbr.boot_magic = 0xaa55;

	//fwrite(&mbr, 1, 512, o);

	// write rc4 encrpted data @ 0x8000
	fseek(o, 0x8000, SEEK_SET);

	// Size of ddr image
	#define INIT_SIZE (90000 / 512 + 1) 

	// Size of main sdram image
	#define OS_SIZE (0x400)

	#define DDR_OFFSET 0x8800

	struct RkHeader hdr;
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

	struct RkHeader *hdr = malloc(512);

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
