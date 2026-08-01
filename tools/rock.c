// Tool to boot Rockchip devices through maskrom (OTG boot) mode.
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <libusb-1.0/libusb.h>
#include "main.h"

#define RK_SEND_DDR 0x471
#define RK_SEND_IMG 0x472
#define RK_MAX 0x1000

static int send_blob(libusb_device *dev, int cmd, const char *filename,
	int do_rc4) {
	printf("Sending '%s'...\n", filename);
	unsigned int crc = 0xffff;
	FILE *f = fopen(filename, "rb");
	if (!f) {
		printf("%s not found\n", filename);
		return -1;
	}
	libusb_device_handle *handle = NULL;
	int result = -1;
	int open_result = libusb_open(dev, &handle);
	if (open_result) {
		printf("libusb_open: '%s'\n", libusb_strerror(open_result));
		goto out_file;
	}

	struct Rc4Encoder r;
	setup_rc4_encoder(&r, rockchip_key);
	while (1) {
		uint8_t chunk[0x1004];
		size_t payload_length = fread(chunk, 1, RK_MAX, f);
		int final_chunk = payload_length != RK_MAX;
		if (ferror(f)) {
			printf("Error reading '%s'.\n", filename);
			goto out_handle;
		}
		if (do_rc4) {
			rc4_encode_chunk(&r, chunk, payload_length);
		}
		crc = crc_sum_16(crc, chunk, payload_length);
		size_t transfer_length = payload_length;
		if (final_chunk) {
			chunk[transfer_length++] = crc >> 8;
			chunk[transfer_length++] = crc & 0xff;
		}
		int rc = libusb_control_transfer(handle, 0x40, 0xc, 0x0, cmd,
			chunk, (uint16_t)transfer_length, 0);
		if (rc < 0) {
			printf("libusb_control_transfer: '%s'\n", libusb_strerror(rc));
			goto out_handle;
		}
		if ((size_t)rc != transfer_length) {
			printf("Short USB transfer: sent %d of %zu bytes.\n", rc,
				transfer_length);
			goto out_handle;
		}
		if (final_chunk) {
			result = 0;
			break;
		}
	}

	out_handle:
	libusb_close(handle);
	out_file:
	fclose(f);
	return result;
}

int main(int argc, char **argv) {
	int version = 1;
	const char *ddr_file = "ddr.bin";
	const char *main_file = "os.bin";

	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--v2")) {
			version = 2;
		} else if (!strcmp(argv[i], "--v1")) {
			version = 1;
		} else if (!strcmp(argv[i], "--ddr")) {
			if (i + 1 >= argc) {
				printf("--ddr requires a file name.\n");
				return -1;
			}
			ddr_file = argv[i + 1];
			i++;
		} else if (!strcmp(argv[i], "--os")) {
			if (i + 1 >= argc) {
				printf("--os requires a file name.\n");
				return -1;
			}
			main_file = argv[i + 1];
			i++;
		} else {
			printf("Unknown arg '%s'\n", argv[i]);
			return -1;
		}
	}

	libusb_context *ctx = NULL;
	int init_result = libusb_init(&ctx);
	if (init_result < 0) {
		printf("libusb_init: '%s'\n", libusb_strerror(init_result));
		return -1;
	}

	// Discover first, then act. Sending to the first device is unsafe when
	// two boards are in MaskROM at the same time.
	libusb_device **list;
	libusb_device *found = NULL;
	ssize_t cnt = libusb_get_device_list(ctx, &list);
	uint16_t found_pid = 0;
	int rockchip_count = 0;
	if (cnt < 0) {
		printf("Error getting device list: '%s'\n",
			libusb_strerror((int)cnt));
		libusb_exit(ctx);
		return -1;
	}

	for (ssize_t i = 0; i < cnt; i++) {
		libusb_device *device = list[i];
		struct libusb_device_descriptor desc;
		int rc = libusb_get_device_descriptor(device, &desc);
		if (rc) {
			libusb_free_device_list(list, 1);
			libusb_exit(ctx);
			return -1;
		}
		if (desc.idVendor != 0x2207) continue;
		rockchip_count++;
		found = device;
		found_pid = desc.idProduct;
	}
	if (rockchip_count > 1) {
		printf("Multiple Rockchip MaskROM devices found; connect exactly one.\n");
		libusb_free_device_list(list, 1);
		libusb_exit(ctx);
		return -1;
	}
	if (!rockchip_count) {
		printf("No Rockchip devices found.\n");
		libusb_free_device_list(list, 1);
		libusb_exit(ctx);
		return -1;
	}

	int do_rc4;
	const char *soc;
	switch (found_pid) {
	case 0x330c: do_rc4 = 1; soc = "RK3399"; break;
	case 0x350a: do_rc4 = 0; soc = "RK356x"; break;
	case 0x350b: do_rc4 = 0; soc = "RK3588"; break;
	default:
		printf("Unsupported Rockchip PID 0x%04x.\n", found_pid);
		libusb_free_device_list(list, 1);
		libusb_exit(ctx);
		return -1;
	}
	printf("Found %s (PID 0x%04x, RC4 %s, transfer profile v%d).\n",
		soc, found_pid, do_rc4 ? "on" : "off", version);
	int rc = send_blob(found, RK_SEND_DDR, ddr_file, do_rc4);
	if (!rc) {
		usleep(10000);
		rc = send_blob(found, RK_SEND_IMG, main_file, do_rc4);
	}
	libusb_free_device_list(list, 1);
	libusb_exit(ctx);
	return rc;
}
