/* SPDX-License-Identifier: GPL-2.0+ */
/* Board-scoped U-Boot policy for the rk YY3568 chainloader. */
#ifndef __CONFIGS_YY3568_H
#define __CONFIGS_YY3568_H

#include <configs/rk3568_common.h>

#ifndef CONFIG_SPL_BUILD

#undef ROCKCHIP_DEVICE_SETTINGS
#define ROCKCHIP_DEVICE_SETTINGS \
	"stdin=serial,usbkbd\0" \
	"stdout=serial,vidconsole\0" \
	"stderr=serial,vidconsole\0"

#define CONFIG_SYS_MMC_ENV_DEV 0

/* Board aliases: mmc1 is the SD slot, mmc0 is eMMC, and mmc2 is SDIO. */
#undef CONFIG_BOOTCOMMAND
#define CONFIG_BOOTCOMMAND \
	"setenv boot_targets nvme0 nvme1 mmc1 usb0 mmc0; run distro_bootcmd"

#endif
#endif
