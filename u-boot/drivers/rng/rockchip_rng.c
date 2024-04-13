// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Fuzhou Rockchip Electronics Co., Ltd
 */
#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <rng.h>
#include <asm/arch-rockchip/hardware.h>
#include <asm/io.h>
#include <linux/iopoll.h>
#include <linux/string.h>

#define RK_HW_RNG_MAX 32

#define _SBF(s, v)	((v) << (s))

/* start of CRYPTO V1 register define */
#define CRYPTO_V1_CTRL				0x0008
#define CRYPTO_V1_RNG_START			BIT(8)
#define CRYPTO_V1_RNG_FLUSH			BIT(9)

#define CRYPTO_V1_TRNG_CTRL			0x0200
#define CRYPTO_V1_OSC_ENABLE			BIT(16)
#define CRYPTO_V1_TRNG_SAMPLE_PERIOD(x)		(x)

#define CRYPTO_V1_TRNG_DOUT_0			0x0204
/* end of CRYPTO V1 register define */

/* start of CRYPTO V2 register define */
#define CRYPTO_V2_RNG_CTL			0x0400
#define CRYPTO_V2_RNG_64_BIT_LEN		_SBF(4, 0x00)
#define CRYPTO_V2_RNG_128_BIT_LEN		_SBF(4, 0x01)
#define CRYPTO_V2_RNG_192_BIT_LEN		_SBF(4, 0x02)
#define CRYPTO_V2_RNG_256_BIT_LEN		_SBF(4, 0x03)
#define CRYPTO_V2_RNG_FATESY_SOC_RING		_SBF(2, 0x00)
#define CRYPTO_V2_RNG_SLOWER_SOC_RING_0		_SBF(2, 0x01)
#define CRYPTO_V2_RNG_SLOWER_SOC_RING_1		_SBF(2, 0x02)
#define CRYPTO_V2_RNG_SLOWEST_SOC_RING		_SBF(2, 0x03)
#define CRYPTO_V2_RNG_ENABLE			BIT(1)
#define CRYPTO_V2_RNG_START			BIT(0)
#define CRYPTO_V2_RNG_SAMPLE_CNT		0x0404
#define CRYPTO_V2_RNG_DOUT_0			0x0410
/* end of CRYPTO V2 register define */

/* start of TRNG V1 register define */
#define TRNG_V1_CTRL				0x0000
#define TRNG_V1_CTRL_NOP			_SBF(0, 0x00)
#define TRNG_V1_CTRL_RAND			_SBF(0, 0x01)
#define TRNG_V1_CTRL_SEED			_SBF(0, 0x02)

#define TRNG_V1_MODE				0x0008
#define TRNG_V1_MODE_128_BIT			_SBF(3, 0x00)
#define TRNG_V1_MODE_256_BIT			_SBF(3, 0x01)

#define TRNG_V1_IE				0x0010
#define TRNG_V1_IE_GLBL_EN			BIT(31)
#define TRNG_V1_IE_SEED_DONE_EN			BIT(1)
#define TRNG_V1_IE_RAND_RDY_EN			BIT(0)

#define TRNG_V1_ISTAT				0x0014
#define TRNG_V1_ISTAT_RAND_RDY			BIT(0)

/* RAND0 ~ RAND7 */
#define TRNG_V1_RAND0				0x0020
#define TRNG_V1_RAND7				0x003C

#define TRNG_V1_AUTO_RQSTS			0x0060

#define TRNG_V1_VERSION				0x00F0
#define TRNG_v1_VERSION_CODE			0x46BC
/* end of TRNG V1 register define */

/* start of RKRNG register define */
#define RKRNG_CTRL				0x0010
#define RKRNG_CTRL_INST_REQ			BIT(0)
#define RKRNG_CTRL_RESEED_REQ			BIT(1)
#define RKRNG_CTRL_TEST_REQ			BIT(2)
#define RKRNG_CTRL_SW_DRNG_REQ			BIT(3)
#define RKRNG_CTRL_SW_TRNG_REQ			BIT(4)

#define RKRNG_STATE				0x0014
#define RKRNG_STATE_INST_ACK			BIT(0)
#define RKRNG_STATE_RESEED_ACK			BIT(1)
#define RKRNG_STATE_TEST_ACK			BIT(2)
#define RKRNG_STATE_SW_DRNG_ACK			BIT(3)
#define RKRNG_STATE_SW_TRNG_ACK			BIT(4)

/* DRNG_DATA_0 ~ DNG_DATA_7 */
#define RKRNG_DRNG_DATA_0			0x0070
#define RKRNG_DRNG_DATA_7			0x008C

/* end of RKRNG register define */

#define RK_RNG_TIME_OUT	50000  /* max 50ms */

#define trng_write(pdata, pos, val)	writel(val, (pdata)->base + (pos))
#define trng_read(pdata, pos)		readl((pdata)->base + (pos))

struct rk_rng_soc_data {
	int (*rk_rng_init)(struct udevice *dev);
	int (*rk_rng_read)(struct udevice *dev, void *data, size_t len);
};

struct rk_rng_platdata {
	fdt_addr_t base;
	struct rk_rng_soc_data *soc_data;
	struct clk hclk;
};

static int rk_rng_do_enable_clk(struct udevice *dev, int enable)
{
	struct rk_rng_platdata *pdata = dev_get_priv(dev);
	int ret;

	if (!pdata->hclk.dev)
		return 0;

	ret = enable ? clk_enable(&pdata->hclk) : clk_disable(&pdata->hclk);
	if (ret == -ENOSYS || !ret)
		return 0;

	printf("rk rng: failed to %s clk, ret=%d\n",
	       enable ? "enable" : "disable", ret);

	return ret;
}

static int rk_rng_enable_clk(struct udevice *dev)
{
	return rk_rng_do_enable_clk(dev, 1);
}

static int rk_rng_disable_clk(struct udevice *dev)
{
	return rk_rng_do_enable_clk(dev, 0);
}

static int rk_rng_read_regs(fdt_addr_t addr, void *buf, size_t size)
{
	u32 count = RK_HW_RNG_MAX / sizeof(u32);
	u32 reg, tmp_len;

	if (size > RK_HW_RNG_MAX)
		return -EINVAL;

	while (size && count) {
		reg = readl(addr);
		tmp_len = min(size, sizeof(u32));
		memcpy(buf, &reg, tmp_len);
		addr += sizeof(u32);
		buf += tmp_len;
		size -= tmp_len;
		count--;
	}

	return 0;
}

static int cryptov1_rng_read(struct udevice *dev, void *data, size_t len)
{
	struct rk_rng_platdata *pdata = dev_get_priv(dev);
	u32 reg = 0;
	int retval;

	if (len > RK_HW_RNG_MAX)
		return -EINVAL;

	/* enable osc_ring to get entropy, sample period is set as 100 */
	writel(CRYPTO_V1_OSC_ENABLE | CRYPTO_V1_TRNG_SAMPLE_PERIOD(100),
	       pdata->base + CRYPTO_V1_TRNG_CTRL);

	rk_clrsetreg(pdata->base + CRYPTO_V1_CTRL, CRYPTO_V1_RNG_START,
		     CRYPTO_V1_RNG_START);

	retval = readl_poll_timeout(pdata->base + CRYPTO_V1_CTRL, reg,
				    !(reg & CRYPTO_V1_RNG_START),
				    RK_RNG_TIME_OUT);
	if (retval)
		goto exit;

	rk_rng_read_regs(pdata->base + CRYPTO_V1_TRNG_DOUT_0, data, len);

exit:
	/* close TRNG */
	rk_clrreg(pdata->base + CRYPTO_V1_CTRL, CRYPTO_V1_RNG_START);

	return 0;
}

static int cryptov2_rng_read(struct udevice *dev, void *data, size_t len)
{
	struct rk_rng_platdata *pdata = dev_get_priv(dev);
	u32 reg = 0;
	int retval;

	if (len > RK_HW_RNG_MAX)
		return -EINVAL;

	/* enable osc_ring to get entropy, sample period is set as 100 */
	writel(100, pdata->base + CRYPTO_V2_RNG_SAMPLE_CNT);

	reg |= CRYPTO_V2_RNG_256_BIT_LEN;
	reg |= CRYPTO_V2_RNG_SLOWER_SOC_RING_0;
	reg |= CRYPTO_V2_RNG_ENABLE;
	reg |= CRYPTO_V2_RNG_START;

	rk_clrsetreg(pdata->base + CRYPTO_V2_RNG_CTL, 0xffff, reg);

	retval = readl_poll_timeout(pdata->base + CRYPTO_V2_RNG_CTL, reg,
				    !(reg & CRYPTO_V2_RNG_START),
				    RK_RNG_TIME_OUT);
	if (retval)
		goto exit;

	rk_rng_read_regs(pdata->base + CRYPTO_V2_RNG_DOUT_0, data, len);

exit:
	/* close TRNG */
	rk_clrreg(pdata->base + CRYPTO_V2_RNG_CTL, 0xffff);

	return retval;
}

static int trngv1_init(struct udevice *dev)
{
	u32 status, version;
	u32 auto_reseed_cnt = 1000;
	struct rk_rng_platdata *pdata = dev_get_priv(dev);

	version = trng_read(pdata, TRNG_V1_VERSION);
	if (version != TRNG_v1_VERSION_CODE) {
		printf("wrong trng version, expected = %08x, actual = %08x",
		       TRNG_V1_VERSION, version);
		return -EFAULT;
	}

	/* wait in case of RND_RDY triggered at firs power on */
	readl_poll_timeout(pdata->base + TRNG_V1_ISTAT, status,
			   (status & TRNG_V1_ISTAT_RAND_RDY),
			   RK_RNG_TIME_OUT);

	/* clear RAND_RDY flag for first power on */
	trng_write(pdata, TRNG_V1_ISTAT, status);

	/* auto reseed after (auto_reseed_cnt * 16) byte rand generate */
	trng_write(pdata, TRNG_V1_AUTO_RQSTS, auto_reseed_cnt);

	return 0;
}

static int trngv1_rng_read(struct udevice *dev, void *data, size_t len)
{
	struct rk_rng_platdata *pdata = dev_get_priv(dev);
	u32 reg = 0;
	int retval;

	if (len > RK_HW_RNG_MAX)
		return -EINVAL;

	trng_write(pdata, TRNG_V1_MODE, TRNG_V1_MODE_256_BIT);
	trng_write(pdata, TRNG_V1_CTRL, TRNG_V1_CTRL_RAND);

	retval = readl_poll_timeout(pdata->base + TRNG_V1_ISTAT, reg,
				    (reg & TRNG_V1_ISTAT_RAND_RDY),
				    RK_RNG_TIME_OUT);
	/* clear ISTAT */
	trng_write(pdata, TRNG_V1_ISTAT, reg);

	if (retval)
		goto exit;

	rk_rng_read_regs(pdata->base + TRNG_V1_RAND0, data, len);

exit:
	/* close TRNG */
	trng_write(pdata, TRNG_V1_CTRL, TRNG_V1_CTRL_NOP);

	return retval;
}

static int rkrng_init(struct udevice *dev)
{
	struct rk_rng_platdata *pdata = dev_get_priv(dev);
	u32 reg = 0;

	rk_clrreg(pdata->base + RKRNG_CTRL, 0xffff);

	reg = trng_read(pdata, RKRNG_STATE);
	trng_write(pdata, RKRNG_STATE, reg);

	return 0;
}

static int rkrng_rng_read(struct udevice *dev, void *data, size_t len)
{
	struct rk_rng_platdata *pdata = dev_get_priv(dev);
	u32 reg = 0;
	int retval;

	if (len > RK_HW_RNG_MAX)
		return -EINVAL;

	rk_rng_enable_clk(dev);

	reg = RKRNG_CTRL_SW_DRNG_REQ;

	rk_clrsetreg(pdata->base + RKRNG_CTRL, 0xffff, reg);

	retval = readl_poll_timeout(pdata->base + RKRNG_STATE, reg,
				    (reg & RKRNG_STATE_SW_DRNG_ACK),
				    RK_RNG_TIME_OUT);
	if (retval)
		goto exit;

	trng_write(pdata, RKRNG_STATE, reg);

	rk_rng_read_regs(pdata->base + RKRNG_DRNG_DATA_0, data, len);

exit:
	/* close TRNG */
	rk_clrreg(pdata->base + RKRNG_CTRL, 0xffff);

	rk_rng_disable_clk(dev);

	return retval;
}

static int rockchip_rng_read(struct udevice *dev, void *data, size_t len)
{
	unsigned char *buf = data;
	unsigned int i;
	int ret = -EIO;

	struct rk_rng_platdata *pdata = dev_get_priv(dev);

	if (!len)
		return 0;

	if (!pdata->soc_data || !pdata->soc_data->rk_rng_read)
		return -EINVAL;

	for (i = 0; i < len / RK_HW_RNG_MAX; i++, buf += RK_HW_RNG_MAX) {
		ret = pdata->soc_data->rk_rng_read(dev, buf, RK_HW_RNG_MAX);
		if (ret)
			goto exit;
	}

	if (len % RK_HW_RNG_MAX)
		ret = pdata->soc_data->rk_rng_read(dev, buf,
						   len % RK_HW_RNG_MAX);

exit:
	return ret;
}

static int rockchip_rng_ofdata_to_platdata(struct udevice *dev)
{
	struct rk_rng_platdata *pdata = dev_get_priv(dev);

	memset(pdata, 0x00, sizeof(*pdata));

	pdata->base = (fdt_addr_t)dev_read_addr_ptr(dev);
	if (!pdata->base)
		return -ENOMEM;

	clk_get_by_index(dev, 0, &pdata->hclk);

	return 0;
}

static int rockchip_rng_probe(struct udevice *dev)
{
	struct rk_rng_platdata *pdata = dev_get_priv(dev);
	int ret = 0;

	pdata->soc_data = (struct rk_rng_soc_data *)dev_get_driver_data(dev);

	if (pdata->soc_data->rk_rng_init)
		ret = pdata->soc_data->rk_rng_init(dev);

	return ret;
}

static const struct rk_rng_soc_data cryptov1_soc_data = {
	.rk_rng_read = cryptov1_rng_read,
};

static const struct rk_rng_soc_data cryptov2_soc_data = {
	.rk_rng_read = cryptov2_rng_read,
};

static const struct rk_rng_soc_data trngv1_soc_data = {
	.rk_rng_init = trngv1_init,
	.rk_rng_read = trngv1_rng_read,
};

static const struct rk_rng_soc_data rkrng_soc_data = {
	.rk_rng_init = rkrng_init,
	.rk_rng_read = rkrng_rng_read,
};

static const struct dm_rng_ops rockchip_rng_ops = {
	.read = rockchip_rng_read,
};

static const struct udevice_id rockchip_rng_match[] = {
	{
		.compatible = "rockchip,cryptov1-rng",
		.data = (ulong)&cryptov1_soc_data,
	},
	{
		.compatible = "rockchip,cryptov2-rng",
		.data = (ulong)&cryptov2_soc_data,
	},
	{
		.compatible = "rockchip,trngv1",
		.data = (ulong)&trngv1_soc_data,
	},
	{
		.compatible = "rockchip,rkrng",
		.data = (ulong)&rkrng_soc_data,
	},
	{},
};

U_BOOT_DRIVER(rockchip_rng) = {
	.name = "rockchip-rng",
	.id = UCLASS_RNG,
	.of_match = rockchip_rng_match,
	.ops = &rockchip_rng_ops,
	.probe = rockchip_rng_probe,
	.ofdata_to_platdata = rockchip_rng_ofdata_to_platdata,
	.priv_auto_alloc_size = sizeof(struct rk_rng_platdata),
};
