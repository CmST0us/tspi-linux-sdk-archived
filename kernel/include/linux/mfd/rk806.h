/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2021 Rockchip Electronics Co., Ltd.
 */

#ifndef __LINUX_REGULATOR_RK806_H
#define __LINUX_REGULATOR_RK806_H

#include <linux/of.h>
#include <linux/regmap.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>

#define RK806_POWER_EN0			0x0
#define RK806_POWER_EN1			0x1
#define RK806_POWER_EN2			0x2
#define RK806_POWER_EN3			0x3
#define RK806_POWER_EN4			0x4
#define RK806_POWER_EN5			0x5
#define RK806_POWER_SLP_EN0		0x6
#define RK806_POWER_SLP_EN1		0x7
#define RK806_POWER_SLP_EN2		0x8
#define RK806_POWER_DISCHRG_EN0		0x9
#define RK806_POWER_DISCHRG_EN1		0xA
#define RK806_POWER_DISCHRG_EN2		0xB
#define RK806_BUCK_FB_CONFIG		0xC
#define RK806_SLP_LP_CONFIG		0xD
#define RK806_POWER_FPWM_EN0		0xE
#define RK806_POWER_FPWM_EN1		0xF
#define RK806_BUCK1_CONFIG		0x10
#define RK806_BUCK2_CONFIG		0x11
#define RK806_BUCK3_CONFIG		0x12
#define RK806_BUCK4_CONFIG		0x13
#define RK806_BUCK5_CONFIG		0x14
#define RK806_BUCK6_CONFIG		0x15
#define RK806_BUCK7_CONFIG		0x16
#define RK806_BUCK8_CONFIG		0x17
#define RK806_BUCK9_CONFIG		0x18
#define RK806_BUCK10_CONFIG		0x19
#define RK806_BUCK1_ON_VSEL		0x1A
#define RK806_BUCK2_ON_VSEL		0x1B
#define RK806_BUCK3_ON_VSEL		0x1C
#define RK806_BUCK4_ON_VSEL		0x1D
#define RK806_BUCK5_ON_VSEL		0x1E
#define RK806_BUCK6_ON_VSEL		0x1F
#define RK806_BUCK7_ON_VSEL		0x20
#define RK806_BUCK8_ON_VSEL		0x21
#define RK806_BUCK9_ON_VSEL		0x22
#define RK806_BUCK10_ON_VSEL		0x23
#define RK806_BUCK1_SLP_VSEL		0x24
#define RK806_BUCK2_SLP_VSEL		0x25
#define RK806_BUCK3_SLP_VSEL		0x26
#define RK806_BUCK4_SLP_VSEL		0x27
#define RK806_BUCK5_SLP_VSEL		0x28
#define RK806_BUCK6_SLP_VSEL		0x29
#define RK806_BUCK7_SLP_VSEL		0x2A
#define RK806_BUCK8_SLP_VSEL		0x2B
#define RK806_BUCK9_SLP_VSEL		0x2D
#define RK806_BUCK10_SLP_VSEL		0x2E
#define RK806_BUCK_DEBUG1		0x30
#define RK806_BUCK_DEBUG2		0x31
#define RK806_BUCK_DEBUG3		0x32
#define RK806_BUCK_DEBUG4		0x33
#define RK806_BUCK_DEBUG5		0x34
#define RK806_BUCK_DEBUG6		0x35
#define RK806_BUCK_DEBUG7		0x36
#define RK806_BUCK_DEBUG8		0x37
#define RK806_BUCK_DEBUG9		0x38
#define RK806_BUCK_DEBUG10		0x39
#define RK806_BUCK_DEBUG11		0x3A
#define RK806_BUCK_DEBUG12		0x3B
#define RK806_BUCK_DEBUG13		0x3C
#define RK806_BUCK_DEBUG14		0x3D
#define RK806_BUCK_DEBUG15		0x3E
#define RK806_BUCK_DEBUG16		0x3F
#define RK806_BUCK_DEBUG17		0x40
#define RK806_BUCK_DEBUG18		0x41
#define RK806_NLDO_IMAX			0x42
#define RK806_NLDO1_ON_VSEL		0x43
#define RK806_NLDO2_ON_VSEL		0x44
#define RK806_NLDO3_ON_VSEL		0x45
#define RK806_NLDO4_ON_VSEL		0x46
#define RK806_NLDO5_ON_VSEL		0x47
#define RK806_NLDO1_SLP_VSEL		0x48
#define RK806_NLDO2_SLP_VSEL		0x49
#define RK806_NLDO3_SLP_VSEL		0x4A
#define RK806_NLDO4_SLP_VSEL		0x4B
#define RK806_NLDO5_SLP_VSEL		0x4C
#define RK806_PLDO_IMAX			0x4D
#define RK806_PLDO1_ON_VSEL		0x4E
#define RK806_PLDO2_ON_VSEL		0x4F
#define RK806_PLDO3_ON_VSEL		0x50
#define RK806_PLDO4_ON_VSEL		0x51
#define RK806_PLDO5_ON_VSEL		0x52
#define RK806_PLDO6_ON_VSEL		0x53
#define RK806_PLDO1_SLP_VSEL		0x54
#define RK806_PLDO2_SLP_VSEL		0x55
#define RK806_PLDO3_SLP_VSEL		0x56
#define RK806_PLDO4_SLP_VSEL		0x57
#define RK806_PLDO5_SLP_VSEL		0x58
#define RK806_PLDO6_SLP_VSEL		0x59
#define RK806_CHIP_NAME			0x5A
#define RK806_CHIP_VER			0x5B
#define RK806_OTP_VER			0x5C
#define RK806_SYS_STS			0x5D
#define RK806_SYS_CFG0			0x5E
#define RK806_SYS_CFG1			0x5F
#define RK806_SYS_OPTION		0x61
#define RK806_SLEEP_CONFIG0		0x62
#define RK806_SLEEP_CONFIG1		0x63
#define RK806_SLEEP_CTR_SEL0		0x64
#define RK806_SLEEP_CTR_SEL1		0x65
#define RK806_SLEEP_CTR_SEL2		0x66
#define RK806_SLEEP_CTR_SEL3		0x67
#define RK806_SLEEP_CTR_SEL4		0x68
#define RK806_SLEEP_CTR_SEL5		0x69
#define RK806_DVS_CTRL_SEL0		0x6A
#define RK806_DVS_CTRL_SEL1		0x6B
#define RK806_DVS_CTRL_SEL2		0x6C
#define RK806_DVS_CTRL_SEL3		0x6D
#define RK806_DVS_CTRL_SEL4		0x6E
#define RK806_DVS_CTRL_SEL5		0x6F
#define RK806_DVS_START_CTRL		0x70
#define RK806_SLEEP_GPIO		0x71
#define RK806_SYS_CFG3			0x72
#define RK806_ON_SOURCE			0x74
#define RK806_OFF_SOURCE		0x75
#define RK806_PWRON_KEY			0x76
#define RK806_INT_STS0			0x77
#define RK806_INT_MSK0			0x78
#define RK806_INT_STS1			0x79
#define RK806_INT_MSK1			0x7A
#define RK806_GPIO_INT_CONFIG		0x7B
#define RK806_DATA_REG0			0x7C
#define RK806_DATA_REG1			0x7D
#define RK806_DATA_REG2			0x7E
#define RK806_DATA_REG3			0x7F
#define RK806_DATA_REG4			0x80
#define RK806_DATA_REG5			0x81
#define RK806_DATA_REG6			0x82
#define RK806_DATA_REG7			0x83
#define RK806_DATA_REG8			0x84
#define RK806_DATA_REG9			0x85
#define RK806_DATA_REG10		0x86
#define RK806_DATA_REG11		0x87
#define RK806_DATA_REG12		0x88
#define RK806_DATA_REG13		0x89
#define RK806_DATA_REG14		0x8A
#define RK806_DATA_REG15		0x8B
#define RK806_TM_REG			0x8C
#define RK806_OTP_EN_REG		0x8D
#define RK806_FUNC_OTP_EN_REG		0x8E
#define RK806_TEST_REG1			0x8F
#define RK806_TEST_REG2			0x90
#define RK806_TEST_REG3			0x91
#define RK806_TEST_REG4			0x92
#define RK806_TEST_REG5			0x93
#define RK806_BUCK_VSEL_OTP_REG0	0x94
#define RK806_BUCK_VSEL_OTP_REG1	0x95
#define RK806_BUCK_VSEL_OTP_REG2	0x96
#define RK806_BUCK_VSEL_OTP_REG3	0x97
#define RK806_BUCK_VSEL_OTP_REG4	0x98
#define RK806_BUCK_VSEL_OTP_REG5	0x99
#define RK806_BUCK_VSEL_OTP_REG6	0x9A
#define RK806_BUCK_VSEL_OTP_REG7	0x9B
#define RK806_BUCK_VSEL_OTP_REG8	0x9C
#define RK806_BUCK_VSEL_OTP_REG9	0x9D
#define RK806_NLDO1_VSEL_OTP_REG0	0x9E
#define RK806_NLDO1_VSEL_OTP_REG1	0x9F
#define RK806_NLDO1_VSEL_OTP_REG2	0xA0
#define RK806_NLDO1_VSEL_OTP_REG3	0xA1
#define RK806_NLDO1_VSEL_OTP_REG4	0xA2
#define RK806_PLDO_VSEL_OTP_REG0	0xA3
#define RK806_PLDO_VSEL_OTP_REG1	0xA4
#define RK806_PLDO_VSEL_OTP_REG2	0xA5
#define RK806_PLDO_VSEL_OTP_REG3	0xA6
#define RK806_PLDO_VSEL_OTP_REG4	0xA7
#define RK806_PLDO_VSEL_OTP_REG5	0xA8
#define RK806_BUCK_EN_OTP_REG1		0xA9
#define RK806_NLDO_EN_OTP_REG1		0xAA
#define RK806_PLDO_EN_OTP_REG1		0xAB
#define RK806_BUCK_FB_RES_OTP_REG1	0xAC
#define RK806_OTP_RESEV_REG0		0xAD
#define RK806_OTP_RESEV_REG1		0xAE
#define RK806_OTP_RESEV_REG2		0xAF
#define RK806_OTP_RESEV_REG3		0xB0
#define RK806_OTP_RESEV_REG4		0xB1
#define RK806_BUCK_SEQ_REG0		0xB2
#define RK806_BUCK_SEQ_REG1		0xB3
#define RK806_BUCK_SEQ_REG2		0xB4
#define RK806_BUCK_SEQ_REG3		0xB5
#define RK806_BUCK_SEQ_REG4		0xB6
#define RK806_BUCK_SEQ_REG5		0xB7
#define RK806_BUCK_SEQ_REG6		0xB8
#define RK806_BUCK_SEQ_REG7		0xB9
#define RK806_BUCK_SEQ_REG8		0xBA
#define RK806_BUCK_SEQ_REG9		0xBB
#define RK806_BUCK_SEQ_REG10		0xBC
#define RK806_BUCK_SEQ_REG11		0xBD
#define RK806_BUCK_SEQ_REG12		0xBE
#define RK806_BUCK_SEQ_REG13		0xBF
#define RK806_BUCK_SEQ_REG14		0xC0
#define RK806_BUCK_SEQ_REG15		0xC1
#define RK806_BUCK_SEQ_REG16		0xC2
#define RK806_BUCK_SEQ_REG17		0xC3
#define RK806_HK_TRIM_REG1		0xC4
#define RK806_HK_TRIM_REG2		0xC5
#define RK806_BUCK_REF_TRIM_REG1	0xC6
#define RK806_BUCK_REF_TRIM_REG2	0xC7
#define RK806_BUCK_REF_TRIM_REG3	0xC8
#define RK806_BUCK_REF_TRIM_REG4	0xC9
#define RK806_BUCK_REF_TRIM_REG5	0xCA
#define RK806_BUCK_OSC_TRIM_REG1	0xCB
#define RK806_BUCK_OSC_TRIM_REG2	0xCC
#define RK806_BUCK_OSC_TRIM_REG3	0xCD
#define RK806_BUCK_OSC_TRIM_REG4	0xCE
#define RK806_BUCK_OSC_TRIM_REG5	0xCF
#define RK806_BUCK_TRIM_ZCDIOS_REG1	0xD0
#define RK806_BUCK_TRIM_ZCDIOS_REG2	0xD1
#define RK806_NLDO_TRIM_REG1		0xD2
#define RK806_NLDO_TRIM_REG2		0xD3
#define RK806_NLDO_TRIM_REG3		0xD4
#define RK806_PLDO_TRIM_REG1		0xD5
#define RK806_PLDO_TRIM_REG2		0xD6
#define RK806_PLDO_TRIM_REG3		0xD7
#define RK806_TRIM_ICOMP_REG1		0xD8
#define RK806_TRIM_ICOMP_REG2		0xD9
#define RK806_EFUSE_CONTROL_REGH	0xDA
#define RK806_FUSE_PROG_REG		0xDB
#define RK806_MAIN_FSM_STS_REG		0xDD
#define RK806_FSM_REG			0xDE
#define RK806_TOP_RESEV_OFFR		0xEC
#define RK806_TOP_RESEV_POR		0xED
#define RK806_BUCK_VRSN_REG1		0xEE
#define RK806_BUCK_VRSN_REG2		0xEF
#define RK806_NLDO_RLOAD_SEL_REG1	0xF0
#define RK806_PLDO_RLOAD_SEL_REG1	0xF1
#define RK806_PLDO_RLOAD_SEL_REG2	0xF2
#define RK806_BUCK_CMIN_MX_REG1		0xF3
#define RK806_BUCK_CMIN_MX_REG2		0xF4
#define RK806_BUCK_FREQ_SET_REG1	0xF5
#define RK806_BUCK_FREQ_SET_REG2	0xF6
#define RK806_BUCK_RS_MEABS_REG1	0xF7
#define RK806_BUCK_RS_MEABS_REG2	0xF8
#define RK806_BUCK_RS_ZDLEB_REG1	0xF9
#define RK806_BUCK_RS_ZDLEB_REG2	0xFA
#define RK806_BUCK_RSERVE_REG1		0xFB
#define RK806_BUCK_RSERVE_REG2		0xFC
#define RK806_BUCK_RSERVE_REG3		0xFD
#define RK806_BUCK_RSERVE_REG4		0xFE
#define RK806_BUCK_RSERVE_REG5		0xFF

/* INT_STS Register field definitions */
#define RK806_INT_STS_PWRON_FALL	BIT(0)
#define RK806_INT_STS_PWRON_RISE	BIT(1)
#define RK806_INT_STS_PWRON		BIT(2)
#define RK806_INT_STS_PWRON_LP		BIT(3)
#define RK806_INT_STS_HOTDIE		BIT(4)
#define RK806_INT_STS_VDC_RISE		BIT(5)
#define RK806_INT_STS_VDC_FALL		BIT(6)
#define RK806_INT_STS_VB_LO		BIT(7)
#define RK806_INT_STS_REV0		BIT(0)
#define RK806_INT_STS_REV1		BIT(1)
#define RK806_INT_STS_REV2		BIT(2)
#define RK806_INT_STS_CRC_ERROR		BIT(3)
#define RK806_INT_STS_SLP3_GPIO		BIT(4)
#define RK806_INT_STS_SLP2_GPIO		BIT(5)
#define RK806_INT_STS_SLP1_GPIO		BIT(6)
#define RK806_INT_STS_WDT		BIT(7)

/* spi command */
#define RK806_CMD_READ			0
#define RK806_CMD_WRITE			BIT(7)
#define RK806_CMD_CRC_EN		BIT(6)
#define RK806_CMD_CRC_DIS		0
#define RK806_CMD_LEN_MSK		0x0f
#define RK806_REG_H			0x00

#define VERSION_AB		0x01

enum rk806_reg_id {
	RK806_ID_DCDC1 = 0,
	RK806_ID_DCDC2,
	RK806_ID_DCDC3,
	RK806_ID_DCDC4,
	RK806_ID_DCDC5,
	RK806_ID_DCDC6,
	RK806_ID_DCDC7,
	RK806_ID_DCDC8,
	RK806_ID_DCDC9,
	RK806_ID_DCDC10,

	RK806_ID_NLDO1,
	RK806_ID_NLDO2,
	RK806_ID_NLDO3,
	RK806_ID_NLDO4,
	RK806_ID_NLDO5,

	RK806_ID_PLDO1,
	RK806_ID_PLDO2,
	RK806_ID_PLDO3,
	RK806_ID_PLDO4,
	RK806_ID_PLDO5,
	RK806_ID_PLDO6,
	RK806_ID_END,
};

/* Define the rk806 IRQ numbers */
enum rk806_irqs {
	/* INT_STS0 registers */
	RK806_IRQ_PWRON_FALL,
	RK806_IRQ_PWRON_RISE,
	RK806_IRQ_PWRON,
	RK806_IRQ_PWRON_LP,
	RK806_IRQ_HOTDIE,
	RK806_IRQ_VDC_RISE,
	RK806_IRQ_VDC_FALL,
	RK806_IRQ_VB_LO,

	/* INT_STS0 registers */
	RK806_IRQ_REV0,
	RK806_IRQ_REV1,
	RK806_IRQ_REV2,
	RK806_IRQ_CRC_ERROR,
	RK806_IRQ_SLP3_GPIO,
	RK806_IRQ_SLP2_GPIO,
	RK806_IRQ_SLP1_GPIO,
	RK806_IRQ_WDT,
};

/* VCC1 low voltage threshold */
enum rk806_lv_sel {
	VB_LO_SEL_2800,
	VB_LO_SEL_2900,
	VB_LO_SEL_3000,
	VB_LO_SEL_3100,
	VB_LO_SEL_3200,
	VB_LO_SEL_3300,
	VB_LO_SEL_3400,
	VB_LO_SEL_3500,
};

/* system shut down voltage select */
enum rk806_uv_sel {
	VB_UV_SEL_2700,
	VB_UV_SEL_2800,
	VB_UV_SEL_2900,
	VB_UV_SEL_3000,
	VB_UV_SEL_3100,
	VB_UV_SEL_3200,
	VB_UV_SEL_3300,
	VB_UV_SEL_3400,
};

/* pin function */
enum rk806_pwrctrl_fun {
	PWRCTRL_NULL_FUN,
	PWRCTRL_SLP_FUN,
	PWRCTRL_POWOFF_FUN,
	PWRCTRL_RST_FUN,
	PWRCTRL_DVS_FUN,
	PWRCTRL_GPIO_FUN,
};

/* pin pol */
enum rk806_pin_level {
	POL_LOW,
	POL_HIGH,
};

enum rk806_vsel_ctr_sel {
	CTR_BY_NO_EFFECT,
	CTR_BY_PWRCTRL1,
	CTR_BY_PWRCTRL2,
	CTR_BY_PWRCTRL3,
};

enum rk806_dvs_ctr_sel {
	CTR_SEL_NO_EFFECT,
	CTR_SEL_DVS_START1,
	CTR_SEL_DVS_START2,
	CTR_SEL_DVS_START3,
};

enum rk806_pin_dr_sel {
	RK806_PIN_INPUT,
	RK806_PIN_OUTPUT,
};

enum rk806_int_pol {
	RK806_INT_POL_LOW,
	RK806_INT_POL_HIGH,
};

enum rk806_int_fun {
	RK806_INT_ONLY,
	RK806_INT_ADN_WKUP,
};

enum rk806_dvs_mode {
	RK806_DVS_NOT_SUPPORT,
	RK806_DVS_START1,
	RK806_DVS_START2,
	RK806_DVS_START3,
	RK806_DVS_PWRCTRL1,
	RK806_DVS_PWRCTRL2,
	RK806_DVS_PWRCTRL3,
	RK806_DVS_START_PWRCTR1,
	RK806_DVS_START_PWRCTR2,
	RK806_DVS_START_PWRCTR3,
	RK806_DVS_END,
};

enum rk806_fields {
	CHIP_NAME_H, CHIP_NAME_L, CHIP_VER, OTP_VER,
	POWER_EN0, POWER_EN1, POWER_EN2, POWER_EN3, POWER_EN4, POWER_EN5,
	BUCK4_EN_MASK, BUCK3_EN_MASK, BUCK2_EN_MASK, BUCK1_EN_MASK,
	BUCK4_EN, BUCK3_EN, BUCK2_EN, BUCK1_EN,
	BUCK8_EN_MASK, BUCK7_EN_MASK, BUCK6_EN_MASK, BUCK5_EN_MASK,
	BUCK8_EN, BUCK7_EN, BUCK6_EN, BUCK5_EN,
	BUCK10_EN_MASK, BUCK9_EN_MASK, BUCK10_EN, BUCK9_EN,
	NLDO4_EN_MASK, NLDO3_EN_MASK, NLDO2_EN_MASK, NLDO1_EN_MASK,
	NLDO4_EN, NLDO3_EN, NLDO2_EN, NLDO1_EN,
	PLDO4_EN_MASK, PLDO3_EN_MASK, PLDO2_EN_MASK, PLDO1_EN_MASK,
	PLDO4_EN, PLDO3_EN, PLDO2_EN, PLDO1_EN,
	NLDO5_EN_MASK, PLDO6_EN_MASK, PLDO5_EN_MASK,
	NLDO5_EN, PLDO6_EN, PLDO5_EN,
	BUCK8_SLP_EN, BUCK7_SLP_EN, BUCK6_SLP_EN, BUCK5_SLP_EN, BUCK4_SLP_EN,
	BUCK3_SLP_EN, BUCK2_SLP_EN, BUCK1_SLP_EN,
	BUCK10_SLP_EN, BUCK9_SLP_EN, NLDO5_SLP_EN, NLDO4_SLP_EN, NLDO3_SLP_EN,
	NLDO2_SLP_EN, NLDO1_SLP_EN,
	PLDO6_SLP_EN, PLDO5_SLP_EN, PLDO4_SLP_EN, PLDO3_SLP_EN,
	PLDO2_SLP_EN, PLDO1_SLP_EN,
	BUCK1_ON_VSEL, BUCK2_ON_VSEL, BUCK3_ON_VSEL, BUCK4_ON_VSEL, BUCK5_ON_VSEL,
	BUCK6_ON_VSEL, BUCK7_ON_VSEL, BUCK8_ON_VSEL, BUCK9_ON_VSEL, BUCK10_ON_VSEL,
	BUCK1_SLP_VSEL, BUCK2_SLP_VSEL, BUCK3_SLP_VSEL, BUCK4_SLP_VSEL, BUCK5_SLP_VSEL,
	BUCK6_SLP_VSEL, BUCK7_SLP_VSEL, BUCK8_SLP_VSEL, BUCK9_SLP_VSEL, BUCK10_SLP_VSEL,
	NLDO1_ON_VSEL, NLDO2_ON_VSEL, NLDO3_ON_VSEL, NLDO4_ON_VSEL, NLDO5_ON_VSEL,
	NLDO1_SLP_VSEL, NLDO2_SLP_VSEL, NLDO3_SLP_VSEL, NLDO4_SLP_VSEL, NLDO5_SLP_VSEL,
	PLDO1_ON_VSEL, PLDO2_ON_VSEL, PLDO3_ON_VSEL, PLDO4_ON_VSEL, PLDO5_ON_VSEL,
	PLDO6_ON_VSEL,
	PLDO1_SLP_VSEL, PLDO2_SLP_VSEL, PLDO3_SLP_VSEL, PLDO4_SLP_VSEL, PLDO5_SLP_VSEL,
	PLDO6_SLP_VSEL,
	BUCK1_RATE, BUCK2_RATE, BUCK3_RATE, BUCK4_RATE, BUCK5_RATE, BUCK6_RATE,
	BUCK7_RATE, BUCK8_RATE, BUCK9_RATE, BUCK10_RATE,
	PWRON_STS, VDC_STS, VB_UV_STSS, VB_LO_STS, HOTDIE_STS, TSD_STS, VB_OV_STS,
	VB_UV_DLY, VB_UV_SEL, VB_LO_ACT, VB_LO_SEL,
	ABNORDET_EN, TSD_TEMP, HOTDIE_TMP, SYS_OV_SD_EN, SYS_OV_SD_DLY_SEL, DLY_ABN_SHORT,
	VCCXDET_DIS, OSC_TC, ENB2_2M, ENB_32K,
	PWRCTRL1_FUN, PWRCTRL2_FUN, PWRCTRL3_FUN,
	PWRCTRL1_POL, PWRCTRL2_POL, PWRCTRL3_POL,
	BUCK1_VSEL_CTR_SEL, BUCK2_VSEL_CTR_SEL, BUCK3_VSEL_CTR_SEL, BUCK4_VSEL_CTR_SEL,
	BUCK5_VSEL_CTR_SEL, BUCK6_VSEL_CTR_SEL, BUCK7_VSEL_CTR_SEL, BUCK8_VSEL_CTR_SEL,
	BUCK9_VSEL_CTR_SEL, BUCK10_VSEL_CTR_SEL,
	NLDO1_VSEL_CTR_SEL, NLDO2_VSEL_CTR_SEL, NLDO3_VSEL_CTR_SEL, NLDO4_VSEL_CTR_SEL,
	NLDO5_VSEL_CTR_SEL,
	PLDO1_VSEL_CTR_SEL, PLDO2_VSEL_CTR_SEL, PLDO3_VSEL_CTR_SEL, PLDO4_VSEL_CTR_SEL,
	PLDO5_VSEL_CTR_SEL, PLDO6_VSEL_CTR_SEL,
	BUCK1_DVS_CTR_SEL, BUCK2_DVS_CTR_SEL, BUCK3_DVS_CTR_SEL, BUCK4_DVS_CTR_SEL,
	BUCK5_DVS_CTR_SEL, BUCK6_DVS_CTR_SEL, BUCK7_DVS_CTR_SEL, BUCK8_DVS_CTR_SEL,
	BUCK9_DVS_CTR_SEL, BUCK10_DVS_CTR_SEL,
	NLDO1_DVS_CTR_SEL, NLDO2_DVS_CTR_SEL, NLDO3_DVS_CTR_SEL, NLDO4_DVS_CTR_SEL,
	NLDO5_DVS_CTR_SEL,
	PLDO1_DVS_CTR_SEL, PLDO2_DVS_CTR_SEL, PLDO3_DVS_CTR_SEL, PLDO4_DVS_CTR_SEL,
	PLDO5_DVS_CTR_SEL, PLDO6_DVS_CTR_SEL,
	DVS_START1, DVS_START2, DVS_START3,
	SLP3_DATA, SLP2_DATA, SLP1_DATA, SLP3_DR, SLP2_DR, SLP1_DR,

	RST_FUN, DEV_RST, DEV_SLP, SLAVE_RESTART_FUN, DEV_OFF,
	WDT_CLR, WDT_EN, WDT_SET, ON_SOURCE, OFF_SOURCE,
	ON_PWRON, ON_VDC, RESTART_RESETB, RESTART_PWRON_LP, RESTART_SLP,
	RESTART_DEV_RST, RESTART_WDT,
	OFF_SLP, VB_SYS_OV, OFF_TSD, OFF_DEV_OFF, OFF_PWRON_LP, OFF_VB_LO,
	PWRON_ON_TIME, PWRON_LP_ACT, PWRON_LP_OFF_TIME, PWRON_LP_TM_SEL, PWRON_DB_SEL,
	VB_LO_INT, VDC_FALL_INT, VDC_RISE_INT, HOTDIE_INT, PWRON_LP_INT, PWRON_INT,
	PWRON_RISE_INT, PWRON_FALL_INT,
	VB_LO_IM, VDC_FALL_INT_IM, VDC_RISE_IM, HOTDIE_IM, PWRON_LP_IM,
	PWRON_IM, PWRON_RISE_INT_IM, PWRON_FALL_INT_IM,
	WDT_INT, SLP1_GPIO_INT, SLP2_GPIO_INT, SLP3_GPIO_INT,
	WDT_INT_IM, SLP1_GPIO_IM, SLP2_GPIO_IM, SLP3_GPIO_IM,
	INT_FUNCTION, INT_POL, INT_FC_EN,
	LDO_RATE, BUCK1_RATE2, BUCK2_RATE2, BUCK3_RATE2, BUCK4_RATE2,
	BUCK5_RATE2, BUCK6_RATE2, BUCK7_RATE2, BUCK8_RATE2, BUCK9_RATE2,
	BUCK10_RATE2,
	F_MAX_FIELDS
};

struct rk806_platform_data {
	int low_voltage_threshold;
	int shutdown_voltage_threshold;
	int force_shutdown_enable;
	int shutdown_temperture_threshold;
	int hotdie_temperture_threshold;
};

struct rk806_pin_info {
	struct pinctrl *p;
	struct pinctrl_state *default_st;
	struct pinctrl_state *power_off;
	struct pinctrl_state *reset;
	struct pinctrl_state *sleep;
	struct pinctrl_state *dvs;
};

/*
 * struct rk806 - state holder for the rk806 driver
 *
 * Device data may be used to access the rk806 chip
 */
struct rk806 {
	struct device *dev;
	struct regmap *regmap;
	struct regmap_field *rmap_fields[F_MAX_FIELDS];
	/* IRQ Data */
	int irq;
	struct regmap_irq_chip_data *irq_data;
	struct rk806_platform_data *pdata;
	struct rk806_pin_info *pins;
	int vb_lo_irq;
};

extern const struct regmap_config rk806_regmap_config;
extern const struct of_device_id rk806_of_match[];
int rk806_device_init(struct rk806 *rk806);
int rk806_device_exit(struct rk806 *rk806);
int rk806_field_write(struct rk806 *rk806,
		      enum rk806_fields field_id,
		      unsigned int val);
int rk806_field_read(struct rk806 *rk806,
		     enum rk806_fields field_id);
#endif /* __LINUX_REGULATOR_RK806_H */
