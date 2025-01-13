// Since we replace ddr image and arm-trusted-firmware, we can act as a bootloader for u-boot.
#include <string.h>
#include "os.h"

extern int ext_bin_size;
extern char ext_bin_start[];
__asm__(
".align 4\n"
".global ext_bin_size\n"
"ext_bin_size: .int ext_bin_end - ext_bin_start\n"
".global ext_bin_start\n"
"ext_bin_start:\n"
".incbin \"/home/daniel/Pulled/coolpi-loader/u-boot.bin\"\n"
"ext_bin_end:\n"
);

void boot_uboot() {
	// U-boot turns MMU off if MMU enabled (?)
	disable_mmu_el3(0);

	puts("Copying memory");
	memcpy((void *)0x200000, ext_bin_start, ext_bin_size);

	puts("Jumping to image");
	debug("Int: ", ((uint32_t *)0x200000)[0]);
	__asm__(
		"mov x0, 0x200000\n"
		"br x0\n"
	);
}
