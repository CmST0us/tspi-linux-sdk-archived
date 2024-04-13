/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022 Rockchip Electronics Co. Ltd.
 * Author: Joseph Chen <chenjh@rock-chips.com>
 */

#ifndef _ASM_ARCH_CRU_RK3528_H
#define _ASM_ARCH_CRU_RK3528_H

#define MHz		1000000
#define KHz		1000
#define OSC_HZ		(24 * MHz)

#define CPU_PVTPLL_HZ	(1200 * MHz)
#define APLL_HZ		(600 * MHz)
#define GPLL_HZ		(1188 * MHz)
#define CPLL_HZ		(996 * MHz)
#define PPLL_HZ		(1000 * MHz)

/* RK3528 pll id */
enum rk3528_pll_id {
	APLL,
	CPLL,
	GPLL,
	PPLL,
	DPLL,
	PLL_COUNT,
};

struct rk3528_clk_info {
	unsigned long id;
	char *name;
};

struct rk3528_clk_priv {
	struct rk3528_cru *cru;
	struct rk3528_sysgrf *grf;
	ulong ppll_hz;
	ulong gpll_hz;
	ulong cpll_hz;
	ulong armclk_hz;
	ulong armclk_enter_hz;
	ulong armclk_init_hz;
	bool sync_kernel;
	bool set_armclk_rate;
};

struct rk3528_pll {
	unsigned int con0;
	unsigned int con1;
	unsigned int con2;
	unsigned int con3;
	unsigned int con4;
	unsigned int reserved0[3];
};

struct rk3528_cru {
	uint32_t apll_con[5];
	uint32_t reserved0014[3];
	uint32_t cpll_con[5];
	uint32_t reserved0034[11];
	uint32_t gpll_con[5];
	uint32_t reserved0074[51+32];
	uint32_t reserved01c0[48];
	uint32_t mode_con[1];
	uint32_t reserved0284[31];
	uint32_t clksel_con[91];
	uint32_t reserved046c[229];
	uint32_t gate_con[46];
	uint32_t reserved08b8[82];
	uint32_t softrst_con[47];
	uint32_t reserved0abc[81];
	uint32_t glb_cnt_th;
	uint32_t glb_rst_st;
	uint32_t glb_srst_fst;
	uint32_t glb_srst_snd;
	uint32_t glb_rst_con;
	uint32_t reserved0c14[6];
	uint32_t corewfi_con;
	uint32_t reserved0c30[15604];

	/* pmucru */
	uint32_t reserved10000[192];
	uint32_t pmuclksel_con[3];
	uint32_t reserved1030c[317];
	uint32_t pmugate_con[3];
	uint32_t reserved1080c[125];
	uint32_t pmusoftrst_con[3];
	uint32_t reserved10a08[7550+8191];

	/* pciecru */
	uint32_t reserved20000[32];
	uint32_t ppll_con[5];
	uint32_t reserved20094[155];
	uint32_t pcieclksel_con[2];
	uint32_t reserved20308[318];
	uint32_t pciegate_con;
};
check_member(rk3528_cru, pciegate_con, 0x20800);

struct rk3528_grf_clk_priv {
	struct rk3528_grf *grf;
};

struct pll_rate_table {
	unsigned long rate;
	unsigned int fbdiv;
	unsigned int postdiv1;
	unsigned int refdiv;
	unsigned int postdiv2;
	unsigned int dsmpd;
	unsigned int frac;
};

#define RK3528_PMU_CRU_BASE			0x10000
#define RK3528_PCIE_CRU_BASE			0x20000
#define RK3528_DDRPHY_CRU_BASE			0x28000
#define RK3528_PLL_CON(x)			((x) * 0x4)
#define RK3528_PCIE_PLL_CON(x)			((x) * 0x4 + RK3528_PCIE_CRU_BASE)
#define RK3528_DDRPHY_PLL_CON(x)		((x) * 0x4 + RK3528_DDRPHY_CRU_BASE)
#define RK3528_MODE_CON				0x280
#define RK3528_CLKSEL_CON(x)			((x) * 0x4 + 0x300)
#define RK3528_PMU_CLKSEL_CON(x)		((x) * 0x4 + 0x300 + RK3528_PMU_CRU_BASE)
#define RK3528_PCIE_CLKSEL_CON(x)		((x) * 0x4 + 0x300 + RK3528_PCIE_CRU_BASE)
#define RK3528_DDRPHY_MODE_CON			(0x280 + RK3528_DDRPHY_CRU_BASE)

#define RK3528_DIV_ACLK_M_CORE_MASK		0x1f
#define RK3528_DIV_ACLK_M_CORE_SHIFT		11
#define RK3528_DIV_PCLK_DBG_MASK		0x1f
#define RK3528_DIV_PCLK_DBG_SHIFT		1

enum {
	/* CRU_CLKSEL_CON00 */
	CLK_MATRIX_50M_SRC_DIV_SHIFT             = 2,
	CLK_MATRIX_50M_SRC_DIV_MASK              = 0x1F << CLK_MATRIX_50M_SRC_DIV_SHIFT,
	CLK_MATRIX_100M_SRC_DIV_SHIFT            = 7,
	CLK_MATRIX_100M_SRC_DIV_MASK             = 0x1F << CLK_MATRIX_100M_SRC_DIV_SHIFT,

	/* CRU_CLKSEL_CON01 */
	CLK_MATRIX_150M_SRC_DIV_SHIFT            = 0,
	CLK_MATRIX_150M_SRC_DIV_MASK             = 0x1F << CLK_MATRIX_150M_SRC_DIV_SHIFT,
	CLK_MATRIX_200M_SRC_DIV_SHIFT            = 5,
	CLK_MATRIX_200M_SRC_DIV_MASK             = 0x1F << CLK_MATRIX_200M_SRC_DIV_SHIFT,
	CLK_MATRIX_250M_SRC_DIV_SHIFT            = 10,
	CLK_MATRIX_250M_SRC_DIV_MASK             = 0x1F << CLK_MATRIX_250M_SRC_DIV_SHIFT,
	CLK_MATRIX_250M_SRC_SEL_SHIFT            = 15,
	CLK_MATRIX_250M_SRC_SEL_MASK             = 0x1 << CLK_MATRIX_250M_SRC_SEL_SHIFT,

	/* CRU_CLKSEL_CON02 */
	CLK_MATRIX_300M_SRC_DIV_SHIFT            = 0,
	CLK_MATRIX_300M_SRC_DIV_MASK             = 0x1F << CLK_MATRIX_300M_SRC_DIV_SHIFT,
	CLK_MATRIX_339M_SRC_DIV_SHIFT            = 5,
	CLK_MATRIX_339M_SRC_DIV_MASK             = 0x1F << CLK_MATRIX_339M_SRC_DIV_SHIFT,
	CLK_MATRIX_400M_SRC_DIV_SHIFT            = 10,
	CLK_MATRIX_400M_SRC_DIV_MASK             = 0x1F << CLK_MATRIX_400M_SRC_DIV_SHIFT,

	/* CRU_CLKSEL_CON03 */
	CLK_MATRIX_500M_SRC_DIV_SHIFT            = 6,
	CLK_MATRIX_500M_SRC_DIV_MASK             = 0x1F << CLK_MATRIX_500M_SRC_DIV_SHIFT,
	CLK_MATRIX_500M_SRC_SEL_SHIFT            = 11,
	CLK_MATRIX_500M_SRC_SEL_MASK             = 0x1 << CLK_MATRIX_500M_SRC_SEL_SHIFT,

	/* CRU_CLKSEL_CON04 */
	CLK_MATRIX_600M_SRC_DIV_SHIFT            = 0,
	CLK_MATRIX_600M_SRC_DIV_MASK             = 0x1F << CLK_MATRIX_600M_SRC_DIV_SHIFT,
	CLK_MATRIX_250M_SRC_SEL_CLK_GPLL_MUX     = 0U,
	CLK_MATRIX_250M_SRC_SEL_CLK_CPLL_MUX     = 1U,
	CLK_MATRIX_500M_SRC_SEL_CLK_GPLL_MUX     = 0U,
	CLK_MATRIX_500M_SRC_SEL_CLK_CPLL_MUX     = 1U,

	/* PMUCRU_CLKSEL_CON00 */ 
	CLK_I2C2_SEL_SHIFT                       = 0,
	CLK_I2C2_SEL_MASK                        = 0x3 << CLK_I2C2_SEL_SHIFT,

	/* PCIE_CRU_CLKSEL_CON01 */
	PCIE_CLK_MATRIX_50M_SRC_DIV_SHIFT        = 7,
	PCIE_CLK_MATRIX_50M_SRC_DIV_MASK         = 0x1f << PCIE_CLK_MATRIX_50M_SRC_DIV_SHIFT,
	PCIE_CLK_MATRIX_100M_SRC_DIV_SHIFT       = 11,
	PCIE_CLK_MATRIX_100M_SRC_DIV_MASK        = 0x1f << PCIE_CLK_MATRIX_100M_SRC_DIV_SHIFT,

	/* CRU_CLKSEL_CON32 */
	DCLK_VOP_SRC0_SEL_SHIFT                  = 10,
	DCLK_VOP_SRC0_SEL_MASK                   = 0x1 << DCLK_VOP_SRC0_SEL_SHIFT,
	DCLK_VOP_SRC0_DIV_SHIFT                  = 2,
	DCLK_VOP_SRC0_DIV_MASK                   = 0xFF << DCLK_VOP_SRC0_DIV_SHIFT,

	/* CRU_CLKSEL_CON33 */
	DCLK_VOP_SRC1_SEL_SHIFT                  = 8,
	DCLK_VOP_SRC1_SEL_MASK                   = 0x1 << DCLK_VOP_SRC1_SEL_SHIFT,
	DCLK_VOP_SRC1_DIV_SHIFT                  = 0,
	DCLK_VOP_SRC1_DIV_MASK                   = 0xFF << DCLK_VOP_SRC1_DIV_SHIFT,

	/* CRU_CLKSEL_CON43 */
	CLK_CORE_CRYPTO_SEL_SHIFT                = 14,
	CLK_CORE_CRYPTO_SEL_MASK                 = 0x3 << CLK_CORE_CRYPTO_SEL_SHIFT,
	ACLK_BUS_VOPGL_ROOT_DIV_SHIFT            = 0U,
	ACLK_BUS_VOPGL_ROOT_DIV_MASK             = 0x7U << ACLK_BUS_VOPGL_ROOT_DIV_SHIFT,

	/* CRU_CLKSEL_CON44 */
	CLK_PWM0_SEL_SHIFT                       = 6,
	CLK_PWM0_SEL_MASK                        = 0x3 << CLK_PWM0_SEL_SHIFT,
	CLK_PWM1_SEL_SHIFT                       = 8,
	CLK_PWM1_SEL_MASK                        = 0x3 << CLK_PWM1_SEL_SHIFT,
	CLK_PWM0_SEL_CLK_MATRIX_100M_SRC         = 0U,
	CLK_PWM0_SEL_CLK_MATRIX_50M_SRC          = 1U,
	CLK_PWM0_SEL_XIN_OSC0_FUNC               = 2U,
	CLK_PWM1_SEL_CLK_MATRIX_100M_SRC         = 0U,
	CLK_PWM1_SEL_CLK_MATRIX_50M_SRC          = 1U,
	CLK_PWM1_SEL_XIN_OSC0_FUNC               = 2U,
	CLK_PKA_CRYPTO_SEL_SHIFT                 = 0,
	CLK_PKA_CRYPTO_SEL_MASK                  = 0x3 << CLK_PKA_CRYPTO_SEL_SHIFT,
	CLK_CORE_CRYPTO_SEL_CLK_MATRIX_300M_SRC  = 0U,
	CLK_CORE_CRYPTO_SEL_CLK_MATRIX_200M_SRC  = 1U,
	CLK_CORE_CRYPTO_SEL_CLK_MATRIX_100M_SRC  = 2U,
	CLK_CORE_CRYPTO_SEL_XIN_OSC0_FUNC        = 3U,
	CLK_PKA_CRYPTO_SEL_CLK_MATRIX_300M_SRC   = 0U,
	CLK_PKA_CRYPTO_SEL_CLK_MATRIX_200M_SRC   = 1U,
	CLK_PKA_CRYPTO_SEL_CLK_MATRIX_100M_SRC   = 2U,
	CLK_PKA_CRYPTO_SEL_XIN_OSC0_FUNC         = 3U,

	/* CRU_CLKSEL_CON60 */
	CLK_MATRIX_25M_SRC_DIV_SHIFT             = 2,
	CLK_MATRIX_25M_SRC_DIV_MASK              = 0xff << CLK_MATRIX_25M_SRC_DIV_SHIFT,
	CLK_MATRIX_125M_SRC_DIV_SHIFT            = 10,
	CLK_MATRIX_125M_SRC_DIV_MASK             = 0x1f << CLK_MATRIX_125M_SRC_DIV_SHIFT,

	/* CRU_CLKSEL_CON61 */
	SCLK_SFC_DIV_SHIFT                       = 6,
	SCLK_SFC_DIV_MASK                        = 0x3F << SCLK_SFC_DIV_SHIFT,
	SCLK_SFC_SEL_SHIFT                       = 12,
	SCLK_SFC_SEL_MASK                        = 0x3 << SCLK_SFC_SEL_SHIFT,
	SCLK_SFC_SEL_CLK_GPLL_MUX                = 0U,
	SCLK_SFC_SEL_CLK_CPLL_MUX                = 1U,
	SCLK_SFC_SEL_XIN_OSC0_FUNC               = 2U,

	/* CRU_CLKSEL_CON62 */
	CCLK_SRC_EMMC_DIV_SHIFT                  = 0,
	CCLK_SRC_EMMC_DIV_MASK                   = 0x3F << CCLK_SRC_EMMC_DIV_SHIFT,
	CCLK_SRC_EMMC_SEL_SHIFT                  = 6,
	CCLK_SRC_EMMC_SEL_MASK                   = 0x3 << CCLK_SRC_EMMC_SEL_SHIFT,
	BCLK_EMMC_SEL_SHIFT                      = 8,
	BCLK_EMMC_SEL_MASK                       = 0x3 << BCLK_EMMC_SEL_SHIFT,

	/* CRU_CLKSEL_CON63 */
	CLK_I2C3_SEL_SHIFT                       = 12,
	CLK_I2C3_SEL_MASK                        = 0x3 << CLK_I2C3_SEL_SHIFT,
	CLK_I2C5_SEL_SHIFT                       = 14,
	CLK_I2C5_SEL_MASK                        = 0x3 << CLK_I2C5_SEL_SHIFT,
	CLK_SPI1_SEL_SHIFT                       = 10,
	CLK_SPI1_SEL_MASK                        = 0x3 << CLK_SPI1_SEL_SHIFT,

	/* CRU_CLKSEL_CON64 */
	CLK_I2C6_SEL_SHIFT                       = 0,
	CLK_I2C6_SEL_MASK                        = 0x3 << CLK_I2C6_SEL_SHIFT,

	/* CRU_CLKSEL_CON74 */
	CLK_SARADC_DIV_SHIFT                     = 0,
	CLK_SARADC_DIV_MASK                      = 0x7 << CLK_SARADC_DIV_SHIFT,
	CLK_TSADC_DIV_SHIFT                      = 3,
	CLK_TSADC_DIV_MASK                       = 0x1F << CLK_TSADC_DIV_SHIFT,
	CLK_TSADC_TSEN_DIV_SHIFT                 = 8,
	CLK_TSADC_TSEN_DIV_MASK                  = 0x1F << CLK_TSADC_TSEN_DIV_SHIFT,

	/* CRU_CLKSEL_CON79 */
	CLK_I2C1_SEL_SHIFT                       = 9,
	CLK_I2C1_SEL_MASK                        = 0x3 << CLK_I2C1_SEL_SHIFT,
	CLK_I2C0_SEL_SHIFT                       = 11,
	CLK_I2C0_SEL_MASK                        = 0x3 << CLK_I2C0_SEL_SHIFT,
	CLK_SPI0_SEL_SHIFT                       = 13,
	CLK_SPI0_SEL_MASK                        = 0x3 << CLK_SPI0_SEL_SHIFT,

	/* CRU_CLKSEL_CON83 */
	ACLK_VOP_ROOT_DIV_SHIFT                  = 12,
	ACLK_VOP_ROOT_DIV_MASK                   = 0x7 << ACLK_VOP_ROOT_DIV_SHIFT,
	ACLK_VOP_ROOT_SEL_SHIFT                  = 15,
	ACLK_VOP_ROOT_SEL_MASK                   = 0x1 << ACLK_VOP_ROOT_SEL_SHIFT,

	/* CRU_CLKSEL_CON84 */
	DCLK_VOP0_SEL_SHIFT                      = 0,
	DCLK_VOP0_SEL_MASK                       = 0x1 << DCLK_VOP0_SEL_SHIFT,
	DCLK_VOP_SRC_SEL_CLK_GPLL_MUX            = 0U,
	DCLK_VOP_SRC_SEL_CLK_CPLL_MUX            = 1U,
	ACLK_VOP_ROOT_SEL_CLK_GPLL_MUX           = 0U,
	ACLK_VOP_ROOT_SEL_CLK_CPLL_MUX           = 1U,
	DCLK_VOP0_SEL_DCLK_VOP_SRC0              = 0U,
	DCLK_VOP0_SEL_CLK_HDMIPHY_PIXEL_IO       = 1U,

	/* CRU_CLKSEL_CON85 */
	CLK_I2C4_SEL_SHIFT                       = 13,
	CLK_I2C4_SEL_MASK                        = 0x3 << CLK_I2C4_SEL_SHIFT,
	CLK_I2C7_SEL_SHIFT                       = 0,
	CLK_I2C7_SEL_MASK                        = 0x3 << CLK_I2C7_SEL_SHIFT,
	CLK_I2C3_SEL_CLK_MATRIX_200M_SRC         = 0U,
	CLK_I2C3_SEL_CLK_MATRIX_100M_SRC         = 1U,
	CLK_I2C3_SEL_CLK_MATRIX_50M_SRC          = 2U,
	CLK_I2C3_SEL_XIN_OSC0_FUNC               = 3U,
	CLK_SPI1_SEL_CLK_MATRIX_200M_SRC         = 0U,
	CLK_SPI1_SEL_CLK_MATRIX_100M_SRC         = 1U,
	CLK_SPI1_SEL_CLK_MATRIX_50M_SRC          = 2U,
	CLK_SPI1_SEL_XIN_OSC0_FUNC               = 3U,
	CCLK_SRC_SDMMC0_DIV_SHIFT                = 0,
	CCLK_SRC_SDMMC0_DIV_MASK                 = 0x3F << CCLK_SRC_SDMMC0_DIV_SHIFT,
	CCLK_SRC_SDMMC0_SEL_SHIFT                = 6,
	CCLK_SRC_SDMMC0_SEL_MASK                 = 0x3 << CCLK_SRC_SDMMC0_SEL_SHIFT,
	CCLK_SRC_EMMC_SEL_CLK_GPLL_MUX           = 0U,
	CCLK_SRC_EMMC_SEL_CLK_CPLL_MUX           = 1U,
	CCLK_SRC_EMMC_SEL_XIN_OSC0_FUNC          = 2U,
	BCLK_EMMC_SEL_CLK_MATRIX_200M_SRC        = 0U,
	BCLK_EMMC_SEL_CLK_MATRIX_100M_SRC        = 1U,
	BCLK_EMMC_SEL_CLK_MATRIX_50M_SRC         = 2U,
	BCLK_EMMC_SEL_XIN_OSC0_FUNC              = 3U,
	CCLK_SRC_SDMMC0_SEL_CLK_GPLL_MUX         = 0U,
	CCLK_SRC_SDMMC0_SEL_CLK_CPLL_MUX         = 1U,
	CCLK_SRC_SDMMC0_SEL_XIN_OSC0_FUNC        = 2U,

	/* CRU_CLKSEL_CON04 */
	CLK_UART0_SRC_DIV_SHIFT                  = 5,
	CLK_UART0_SRC_DIV_MASK                   = 0x1F << CLK_UART0_SRC_DIV_SHIFT,
	/* CRU_CLKSEL_CON05 */
	CLK_UART0_FRAC_DIV_SHIFT                 = 0,
	CLK_UART0_FRAC_DIV_MASK                  = 0xFFFFFFFF << CLK_UART0_FRAC_DIV_SHIFT,
	/* CRU_CLKSEL_CON06 */
	SCLK_UART0_SRC_SEL_SHIFT                 = 0,
	SCLK_UART0_SRC_SEL_MASK                  = 0x3 << SCLK_UART0_SRC_SEL_SHIFT,
	CLK_UART1_SRC_DIV_SHIFT                  = 2,
	CLK_UART1_SRC_DIV_MASK                   = 0x1F << CLK_UART1_SRC_DIV_SHIFT,
	/* CRU_CLKSEL_CON07 */
	CLK_UART1_FRAC_DIV_SHIFT                 = 0,
	CLK_UART1_FRAC_DIV_MASK                  = 0xFFFFFFFF << CLK_UART1_FRAC_DIV_SHIFT,
	/* CRU_CLKSEL_CON08 */
	SCLK_UART1_SRC_SEL_SHIFT                 = 0,
	SCLK_UART1_SRC_SEL_MASK                  = 0x3 << SCLK_UART1_SRC_SEL_SHIFT,
	CLK_UART2_SRC_DIV_SHIFT                  = 2,
	CLK_UART2_SRC_DIV_MASK                   = 0x1F << CLK_UART2_SRC_DIV_SHIFT,
	/* CRU_CLKSEL_CON09 */
	CLK_UART2_FRAC_DIV_SHIFT                 = 0,
	CLK_UART2_FRAC_DIV_MASK                  = 0xFFFFFFFF << CLK_UART2_FRAC_DIV_SHIFT,
	/* CRU_CLKSEL_CON10 */
	SCLK_UART2_SRC_SEL_SHIFT                 = 0,
	SCLK_UART2_SRC_SEL_MASK                  = 0x3 << SCLK_UART2_SRC_SEL_SHIFT,
	CLK_UART3_SRC_DIV_SHIFT                  = 2,
	CLK_UART3_SRC_DIV_MASK                   = 0x1F << CLK_UART3_SRC_DIV_SHIFT,
	/* CRU_CLKSEL_CON11 */
	CLK_UART3_FRAC_DIV_SHIFT                 = 0,
	CLK_UART3_FRAC_DIV_MASK                  = 0xFFFFFFFF << CLK_UART3_FRAC_DIV_SHIFT,
	/* CRU_CLKSEL_CON12 */
	SCLK_UART3_SRC_SEL_SHIFT                 = 0,
	SCLK_UART3_SRC_SEL_MASK                  = 0x3 << SCLK_UART3_SRC_SEL_SHIFT,
	CLK_UART4_SRC_DIV_SHIFT                  = 2,
	CLK_UART4_SRC_DIV_MASK                   = 0x1F << CLK_UART4_SRC_DIV_SHIFT,
	/* CRU_CLKSEL_CON13 */
	CLK_UART4_FRAC_DIV_SHIFT                 = 0,
	CLK_UART4_FRAC_DIV_MASK                  = 0xFFFFFFFF << CLK_UART4_FRAC_DIV_SHIFT,
	/* CRU_CLKSEL_CON14 */
	SCLK_UART4_SRC_SEL_SHIFT                 = 0,
	SCLK_UART4_SRC_SEL_MASK                  = 0x3 << SCLK_UART4_SRC_SEL_SHIFT,
	CLK_UART5_SRC_DIV_SHIFT                  = 2,
	CLK_UART5_SRC_DIV_MASK                   = 0x1F << CLK_UART5_SRC_DIV_SHIFT,
	/* CRU_CLKSEL_CON15 */
	CLK_UART5_FRAC_DIV_SHIFT                 = 0,
	CLK_UART5_FRAC_DIV_MASK                  = 0xFFFFFFFF << CLK_UART5_FRAC_DIV_SHIFT,
	/* CRU_CLKSEL_CON16 */
	SCLK_UART5_SRC_SEL_SHIFT                 = 0,
	SCLK_UART5_SRC_SEL_MASK                  = 0x3 << SCLK_UART5_SRC_SEL_SHIFT,
	CLK_UART6_SRC_DIV_SHIFT                  = 2,
	CLK_UART6_SRC_DIV_MASK                   = 0x1F << CLK_UART6_SRC_DIV_SHIFT,
	/* CRU_CLKSEL_CON17 */
	CLK_UART6_FRAC_DIV_SHIFT                 = 0,
	CLK_UART6_FRAC_DIV_MASK                  = 0xFFFFFFFF << CLK_UART6_FRAC_DIV_SHIFT,
	/* CRU_CLKSEL_CON18 */
	SCLK_UART6_SRC_SEL_SHIFT                 = 0,
	SCLK_UART6_SRC_SEL_MASK                  = 0x3 << SCLK_UART6_SRC_SEL_SHIFT,
	CLK_UART7_SRC_DIV_SHIFT                  = 2,
	CLK_UART7_SRC_DIV_MASK                   = 0x1F << CLK_UART7_SRC_DIV_SHIFT,
	/* CRU_CLKSEL_CON19 */
	CLK_UART7_FRAC_DIV_SHIFT                 = 0,
	CLK_UART7_FRAC_DIV_MASK                  = 0xFFFFFFFF << CLK_UART7_FRAC_DIV_SHIFT,
	/* CRU_CLKSEL_CON20 */
	SCLK_UART7_SRC_SEL_SHIFT                 = 0,
	SCLK_UART7_SRC_SEL_MASK                  = 0x3 << SCLK_UART7_SRC_SEL_SHIFT,
	SCLK_UART0_SRC_SEL_CLK_UART0_SRC         = 0U,
	SCLK_UART0_SRC_SEL_CLK_UART0_FRAC        = 1U,
	SCLK_UART0_SRC_SEL_XIN_OSC0_FUNC         = 2U,

	/* CRU_CLKSEL_CON60 */
	CLK_GMAC1_VPU_25M_DIV_SHIFT              = 2,
	CLK_GMAC1_VPU_25M_DIV_MASK               = 0xFF << CLK_GMAC1_VPU_25M_DIV_SHIFT,
	/* CRU_CLKSEL_CON66 */
	CLK_GMAC1_SRC_VPU_DIV_SHIFT              = 0,
	CLK_GMAC1_SRC_VPU_DIV_MASK               = 0x3F << CLK_GMAC1_SRC_VPU_DIV_SHIFT,
	/* CRU_CLKSEL_CON84 */
	CLK_GMAC0_SRC_DIV_SHIFT                  = 3,
	CLK_GMAC0_SRC_DIV_MASK                   = 0x3F << CLK_GMAC0_SRC_DIV_SHIFT,
};

#endif
