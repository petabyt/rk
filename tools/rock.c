#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libusb-1.0/libusb.h>
#include "main.h"

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

#define RK_SEND_DDR 0x471
#define RK_SEND_IMG 0x472
#define RK_MAX 0x1000

int send_blob(libusb_device *dev, int cmd, const char *filename, int do_rc4) {
	unsigned int crc = 0xffff;
	FILE *f = fopen(filename, "rb");
	if (!f) {
		printf("%s not found\n", filename);
		return -1;
	}
	fseek(f, 0, SEEK_END);
	long file_size = ftell(f);
	fseek(f, 0, SEEK_SET);

	libusb_device_handle *handle;
	if (libusb_open(dev, &handle)) {
		printf("libusb_open\n");
		return -1;
	}

	struct Rc4Encoder r;
	setup_rc4_encoder(&r, rockchip_key);
	unsigned int total_read = 0;
	while (1) {
		uint8_t chunk[0x1004];
		unsigned int max = 0x1000;
		unsigned int read = fread(chunk, 1, 0x1000, f);
		total_read += read;
		if (do_rc4) {
			rc4_encode_chunk(&r, chunk, read);
		}
		crc = crc_sum_16(crc, chunk, read);
		if (read != 0x1000) {
			printf("CRC: %04x\n", crc);
			chunk[read] = crc >> 8;
			chunk[read + 1] = crc & 0xff;
			read += 2;
		}

		int rc = libusb_control_transfer(handle, 0x40, 0xc, 0x0, cmd, chunk, read, 500);
		if (rc < 0) {
			printf("libusb_control_transfer: '%s'\n", libusb_strerror(rc));
			return -1;
		} else {
			printf("Sent %u bytes\n", rc);
		}
		if (read != 0x1000) break;
	}

	libusb_close(handle);
	fclose(f);
	return 0;
}

int main(int argc, char **argv) {
	libusb_context *ctx;
	libusb_init(&ctx);

	int version = 1;
	const char *ddr_file = "ddr.bin";
	const char *main_file = "os.bin";

	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--v2")) {
			version = 2;
		} else if (!strcmp(argv[i], "--ddr")) {
			ddr_file = argv[i + 1];
			i++;
		} else if (!strcmp(argv[i], "--os")) {
			main_file = argv[i + 1];
			i++;
		} else {
			printf("Unknown arg '%s'\n", argv[i]);
		}
	}

	// discover devices
	libusb_device **list;
	libusb_device *found = NULL;
	ssize_t cnt = libusb_get_device_list(ctx, &list);
	ssize_t i = 0;
	int err = 0;
	if (cnt < 0) {
		printf("Error getting device list\n");
		return -1;
	}

	for (i = 0; i < cnt; i++) {
		libusb_device *device = list[i];
		struct libusb_device_descriptor desc;
		int rc = libusb_get_device_descriptor(device, &desc);
		if (rc) return -1;
		if (desc.idVendor == 0x2207 && desc.idProduct == 0x330c) {
			// RK3399
			rc = send_blob(device, RK_SEND_DDR, ddr_file, 1);
			if (rc) return rc;
			usleep(1000);
			send_blob(device, RK_SEND_IMG, main_file, 1);
			if (rc) return rc;
			goto exit;
		} else if (desc.idVendor == 0x2207 && desc.idProduct == 0x350b) {
			// RK3588
			rc = send_blob(device, RK_SEND_DDR, ddr_file, 0);
			if (rc) return rc;
			usleep(1000);
			send_blob(device, RK_SEND_IMG, main_file, 0);
			if (rc) return rc;
			goto exit;
		}
	}

	printf("No rockchip devices found.\n");
	return -1;

	exit:;
	return 0;
}
