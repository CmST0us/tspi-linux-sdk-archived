/*
 * (C) Copyright 2019 Rockchip Electronics Co., Ltd
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <boot_rkimg.h>
#include <image.h>
#include <malloc.h>
#include <sysmem.h>
#include <asm/arch/resource_img.h>
#include <asm/arch/uimage.h>

int uimage_load_one(struct blk_desc *dev_desc, disk_partition_t *part,
		    int pos_off, int size, void *dst)
{
	u32 blknum, blkoff;
	u32 unused;
	ulong blksz;
	void *buf;

	blksz  = dev_desc->blksz;
	blkoff = pos_off / blksz;
	unused = pos_off - blkoff * blksz;
	blknum = DIV_ROUND_UP(size, blksz) + !!unused;

	if (!size)
		return -EINVAL;

	if (!IS_ALIGNED(unused, ARCH_DMA_MINALIGN)) {
		buf = memalign(ARCH_DMA_MINALIGN, blknum * blksz);
		if (!buf)
			return -ENOMEM;

		if (blk_dread(dev_desc, part->start + blkoff,
			      blknum, buf) != blknum) {
			free(buf);
			return -EIO;
		}

		memcpy(dst, buf + unused, size);
		free(buf);
	} else {
		if (blk_dread(dev_desc, part->start + blkoff,
		      blknum, (void *)((ulong)dst - unused)) != blknum)
			return -EIO;
	}

	return 0;
}

static image_header_t *uimage_get_hdr(struct blk_desc *dev_desc,
				      disk_partition_t *part)
{
	image_header_t *hdr;

	hdr = memalign(ARCH_DMA_MINALIGN, RK_BLK_SIZE);
	if (!hdr)
		return NULL;

	if (blk_dread(dev_desc, part->start, 1, hdr) != 1)
		goto err;

	if (!image_check_magic(hdr) || (image_get_type(hdr) != IH_TYPE_MULTI))
		goto err;

	return hdr;
err:
	free(hdr);
	return NULL;
}

void *uimage_load_bootables(void)
{
	struct blk_desc *dev_desc;
	disk_partition_t part;
	image_header_t *hdr;
	char *part_name;
	ulong raddr;
	ulong kaddr;
	ulong faddr;
	int blknum;

	raddr = env_get_ulong("ramdisk_addr_r", 16, 0);
	kaddr = env_get_ulong("kernel_addr_r", 16, 0);
	faddr = env_get_ulong("fdt_addr_r", 16, 0);

	if (!faddr || !kaddr || !raddr)
		return NULL;

	dev_desc = rockchip_get_bootdev();
	if (!dev_desc) {
		UIMG_I("No dev_desc\n");
		return NULL;
	}

	if (rockchip_get_boot_mode() == BOOT_MODE_RECOVERY)
		part_name = PART_RECOVERY;
	else
		part_name = PART_BOOT;

	if (part_get_info_by_name(dev_desc, part_name, &part) < 0) {
		UIMG_I("No %s partition\n", part_name);
		return NULL;
	}

	hdr = uimage_get_hdr(dev_desc, &part);
	if (!hdr)
		return NULL;

	/* load */
	blknum = DIV_ROUND_UP(image_get_image_size(hdr), dev_desc->blksz);
	hdr = sysmem_alloc(MEM_UIMAGE, blknum * dev_desc->blksz);
	if (!hdr)
		return NULL;

	if (blk_dread(dev_desc, part.start, blknum, (void *)hdr) != blknum) {
		UIMG_I("Failed to read %s data\n", part.name);
		return NULL;
	}

	return hdr;
}

int uimage_sysmem_reserve_each(image_header_t *hdr, u32 *ramdisk_sz)
{
	ulong raddr, kaddr, faddr;
	ulong data, size;
	int blknum;
	int blksz = RK_BLK_SIZE;

	raddr = env_get_ulong("ramdisk_addr_r", 16, 0);
	kaddr = env_get_ulong("kernel_addr_r", 16, 0);
	faddr = env_get_ulong("fdt_addr_r", 16, 0);

	if (!faddr || !kaddr || !raddr)
		return -EINVAL;

	/* kernel */
	image_multi_getimg(hdr, 0, &data, &size);
	blknum = DIV_ROUND_UP(size, blksz);
	if (!sysmem_alloc_base(MEM_KERNEL, (phys_addr_t)kaddr,
			       blknum * blksz))
		return -ENOMEM;

	/* ramdisk */
	image_multi_getimg(hdr, 1, &data, &size);
	*ramdisk_sz = size;
	blknum = DIV_ROUND_UP(size, blksz);
	if (size && !sysmem_alloc_base(MEM_RAMDISK, (phys_addr_t)raddr,
			       blknum * blksz))
		return -ENOMEM;

	/* fdt */
	image_multi_getimg(hdr, 2, &data, &size);
	blknum = DIV_ROUND_UP(size, blksz);
	if (!sysmem_alloc_base(MEM_FDT, (phys_addr_t)faddr,
			       blknum * blksz))
		return -ENOMEM;

	env_set_hex("fdt_high", faddr);
	env_set_hex("initrd_high", raddr);
	env_set("bootm-reloc-at", "y");

	return 0;
}

int uimage_sysmem_free_each(image_header_t *img, u32 ramdisk_sz)
{
	ulong raddr, kaddr, faddr;

	raddr = env_get_ulong("ramdisk_addr_r", 16, 0);
	kaddr = env_get_ulong("kernel_addr_r", 16, 0);
	faddr = env_get_ulong("fdt_addr_r", 16, 0);

	sysmem_free((phys_addr_t)img);
	sysmem_free((phys_addr_t)kaddr);
	sysmem_free((phys_addr_t)faddr);
	if (ramdisk_sz)
		sysmem_free((phys_addr_t)raddr);

	return 0;
}

#ifdef CONFIG_ROCKCHIP_RESOURCE_IMAGE
int uimage_init_resource(struct blk_desc *dev_desc,
			 disk_partition_t *out_part,
			 ulong *out_blk_offset)
{
	disk_partition_t part;
	image_header_t *hdr;
	char *part_name = PART_BOOT;
	ulong data, size;

	if (!dev_desc)
		return ENODEV;

#ifndef CONFIG_ANDROID_AB
	if (rockchip_get_boot_mode() == BOOT_MODE_RECOVERY)
		part_name = PART_RECOVERY;
#endif
	if (part_get_info_by_name(dev_desc, part_name, &part) < 0) {
		UIMG_I("No %s partition\n", part_name);
		return -ENODEV;
	}

	hdr = uimage_get_hdr(dev_desc, &part);
	if (!hdr)
		return -ENODEV;

	image_multi_getimg(hdr, 3, &data, &size);
	free(hdr);

	*out_part = part;
	*out_blk_offset = DIV_ROUND_UP(data, dev_desc->blksz);

	return 0;
}
#endif

