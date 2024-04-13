/*
 * (C) Copyright 2017 Rockchip Electronics Co., Ltd
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <syscon.h>
#include <asm/arch/clock.h>

static const struct udevice_id rk322x_syscon_ids[] = {
	{ .compatible = "rockchip,rk3228-grf", .data = ROCKCHIP_SYSCON_GRF },
	{ .compatible = "rockchip,rk3228-msch", .data = ROCKCHIP_SYSCON_MSCH },
	{ }
};

U_BOOT_DRIVER(syscon_rk322x) = {
	.name = "rk322x_syscon",
	.id = UCLASS_SYSCON,
	.bind = dm_scan_fdt_dev,
	.of_match = rk322x_syscon_ids,
};
