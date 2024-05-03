// Since we replace ddr image and arm-trusted-firmware, we can act as a bootloader for u-boot.
#include <string.h>
#include <stdio.h>
extern int ext_bin_size;
extern char ext_bin_start[];
__asm__(
".align 4\n"
".global ext_bin_size\n"
"ext_bin_size: .int ext_bin_end - ext_bin_start\n"
".global ext_bin_start\n"
"ext_bin_start:\n"
".incbin \"../towboot2/u-boot.bin\"\n"
"ext_bin_end:\n");

void boot_uboot() {
	memcpy((void *)0x200000, ext_bin_start, ext_bin_size);

	puts("Jumping to image");
	__asm__(
		"mov x0, 0x200000\n"
		"br x0\n"
	);
}
