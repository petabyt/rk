/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2021 Rockchip Electronics Co., Ltd
 *
 * ROCK 3A uses mainline's TARGET_EVB_RK3568 board target.  This file exists
 * only in the board-isolated chainload source snapshot and narrows the common
 * RK3568 default boot targets before rk3568_common.h builds the environment.
 */

#ifndef __EVB_RK3568_H
#define __EVB_RK3568_H

#define ROCKCHIP_DEVICE_SETTINGS \
			"stdout=serial,vidconsole\0" \
			"stderr=serial,vidconsole\0"

#define BOOT_TARGETS "nvme mmc1 usb mmc0"

#include <configs/rk3568_common.h>

#endif
