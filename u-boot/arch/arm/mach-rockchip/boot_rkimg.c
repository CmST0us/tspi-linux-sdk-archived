/*
 * (C) Copyright 2017 Rockchip Electronics Co., Ltd
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <adc.h>
#include <android_bootloader.h>
#include <android_image.h>
#include <bidram.h>
#include <bootm.h>
#include <boot_rkimg.h>
#include <cli.h>
#include <crypto.h>
#include <dm.h>
#include <fs.h>
#include <image.h>
#include <key.h>
#include <mmc.h>
#include <malloc.h>
#include <mp_boot.h>
#include <mtd_blk.h>
#include <nvme.h>
#include <scsi.h>
#include <stdlib.h>
#include <sysmem.h>
#include <asm/io.h>
#include <asm/arch/boot_mode.h>
#include <asm/arch/fit.h>
#include <asm/arch/hotkey.h>
#include <asm/arch/param.h>
#include <asm/arch/resource_img.h>
#include <asm/arch/uimage.h>
#include <dm/ofnode.h>
#include <linux/list.h>
#include <u-boot/sha1.h>
#include <u-boot/sha256.h>
#include <linux/usb/phy-rockchip-usb2.h>

DECLARE_GLOBAL_DATA_PTR;

__weak int rk_board_early_fdt_fixup(void *blob)
{
	return 0;
}

__weak int rk_board_scan_bootdev(void)
{
	const char *devtype_num_set = "run rkimg_bootdev";

	return run_command_list(devtype_num_set, -1, 0);
}

static int bootdev_init(const char *devtype, const char *devnum)
{
#ifdef CONFIG_MMC
	if (!strcmp("mmc", devtype))
		mmc_initialize(gd->bd);
#endif
#ifdef CONFIG_NVME
	if (!strcmp("nvme", devtype)) {
		pci_init();
		if (nvme_scan_namespace())
			return -ENODEV;
	}
#endif
#if defined(CONFIG_SCSI) && defined(CONFIG_CMD_SCSI) && defined(CONFIG_AHCI)
	if (!strcmp("scsi", devtype)) {
		if (scsi_scan(true))
			return -ENODEV;
	}
#endif
	/* Ok, let's test whether we can get the expected boot device or not */
	if (!blk_get_devnum_by_typename(devtype, atoi(devnum)))
		return -ENODEV;

	env_set("devtype", devtype);
	env_set("devnum", devnum);

	return 0;
}

/*
 * Priority: configuration > atags > scan list.
 */
static void boot_devtype_init(void)
{
	char *devtype = NULL, *devnum = NULL;
	char *src = "scan";
	static int done;	/* static */
	int ret;

	if (done)
		return;

#ifdef CONFIG_MP_BOOT
	mpb_post(0);
#endif

	/* configuration */
	if (!param_parse_assign_bootdev(&devtype, &devnum)) {
		if (!bootdev_init(devtype, devnum)) {
			src = "assign";
			goto finish;
		}
	}

	/* atags */
#ifdef CONFIG_ROCKCHIP_PRELOADER_ATAGS
	if (!param_parse_atags_bootdev(&devtype, &devnum)) {
		if (!bootdev_init(devtype, devnum)) {
			src = "atags";
			goto finish;
		}
	}
#endif

	/* scan list */
#ifdef CONFIG_MMC
	mmc_initialize(gd->bd);
#endif
	ret = rk_board_scan_bootdev();
	if (ret) {
		/* Set default if all failed */
		devtype = "mmc";
		devnum = "0";
		env_set("devtype", devtype);
		env_set("devnum", devnum);
	}
finish:
	done = 1;
	printf("Bootdev(%s): %s %s\n", src,
	       env_get("devtype"), env_get("devnum"));
}

static int get_bootdev_type(void)
{
	char *boot_media = NULL, *devtype = NULL;
	char boot_options[128] = {0};
	static int appended;
	ulong devnum = 0;
	int type = 0;

	devtype = env_get("devtype");
	devnum = env_get_ulong("devnum", 10, 0);

	/* For current use(Only EMMC support!) */
	if (!devtype) {
		devtype = "mmc";
		printf("Use emmc as default boot media\n");
	}

	if (!strcmp(devtype, "mmc")) {
		type = IF_TYPE_MMC;
		if (devnum == 1)
			boot_media = "sd";
		else
			boot_media = "emmc";
	} else if (!strcmp(devtype, "rknand")) {
		type = IF_TYPE_RKNAND;
		boot_media = "nand";
	} else if (!strcmp(devtype, "spinand")) {
		type = IF_TYPE_SPINAND;
		boot_media = "nand"; /* kernel treat sfc nand as nand device */
	} else if (!strcmp(devtype, "spinor")) {
		type = IF_TYPE_SPINOR;
		boot_media = "nor";
	} else if (!strcmp(devtype, "ramdisk")) {
		type = IF_TYPE_RAMDISK;
		boot_media = "ramdisk";
	} else if (!strcmp(devtype, "mtd")) {
		type = IF_TYPE_MTD;
		boot_media = "mtd";
	} else if (!strcmp(devtype, "scsi")) {
		type = IF_TYPE_SCSI;
		boot_media = "scsi";
	} else if (!strcmp(devtype, "nvme")) {
		type = IF_TYPE_NVME;
		boot_media = "nvme";
	} else {
		/* Add new to support */
	}

	if (!appended && boot_media) {
		appended = 1;

	/*
	 * The legacy rockchip Android (SDK < 8.1) requires "androidboot.mode="
	 * to be "charger" or boot media which is a rockchip private solution.
	 *
	 * The official Android rule (SDK >= 8.1) is:
	 * "androidboot.mode=normal" or "androidboot.mode=charger".
	 *
	 * Now that this U-Boot is usually working with higher version
	 * Android (SDK >= 8.1), we follow the official rules.
	 *
	 * Common: androidboot.mode=charger has higher priority, don't override;
	 */
#ifdef CONFIG_RKIMG_ANDROID_BOOTMODE_LEGACY
		/* rknand doesn't need "androidboot.mode="; */
		if (env_exist("bootargs", "androidboot.mode=charger") ||
		    (type == IF_TYPE_RKNAND) ||
		    (type == IF_TYPE_SPINAND) ||
		    (type == IF_TYPE_SPINOR))
			snprintf(boot_options, sizeof(boot_options),
				 "storagemedia=%s", boot_media);
		else
			snprintf(boot_options, sizeof(boot_options),
				 "storagemedia=%s androidboot.mode=%s",
				 boot_media, boot_media);
#else
		/*
		 * 1. "storagemedia": This is a legacy variable to indicate board
		 *    storage media for kernel and android.
		 *
		 * 2. "androidboot.storagemedia": The same purpose as "storagemedia",
		 *    but the android framework will auto create property by
		 *    variable with format "androidboot.xxx", eg:
		 *
		 *    "androidboot.storagemedia" => "ro.boot.storagemedia".
		 *
		 *    So, U-Boot pass this new variable is only for the convenience
		 *    to Android.
		 */
		if (env_exist("bootargs", "androidboot.mode=charger"))
			snprintf(boot_options, sizeof(boot_options),
				 "storagemedia=%s androidboot.storagemedia=%s",
				 boot_media, boot_media);
		else
			snprintf(boot_options, sizeof(boot_options),
				 "storagemedia=%s androidboot.storagemedia=%s "
				 "androidboot.mode=normal ",
				 boot_media, boot_media);
#endif
		env_update("bootargs", boot_options);
	}

	return type;
}

static struct blk_desc *dev_desc;

struct blk_desc *rockchip_get_bootdev(void)
{
	int dev_type;
	int devnum;

	if (dev_desc)
		return dev_desc;

	boot_devtype_init();
	dev_type = get_bootdev_type();
	devnum = env_get_ulong("devnum", 10, 0);

	dev_desc = blk_get_devnum_by_type(dev_type, devnum);
	if (!dev_desc) {
		printf("%s: Can't find dev_desc!\n", __func__);
		return NULL;
	}

#ifdef CONFIG_MMC
	if (dev_type == IF_TYPE_MMC) {
		struct mmc *mmc;
		const char *timing[] = {
			"Legacy", "High Speed", "High Speed", "SDR12",
			"SDR25", "SDR50", "SDR104", "DDR50",
			"DDR52", "HS200", "HS400", "HS400 Enhanced Strobe"};

		mmc = find_mmc_device(devnum);
		printf("MMC%d: %s, %dMhz\n", devnum,
		       timing[mmc->timing], mmc->clock / 1000000);
	}
#endif

	printf("PartType: %s\n", part_get_type(dev_desc));

#ifdef CONFIG_MTD_BLK
	mtd_blk_map_partitions(dev_desc);
#endif
	return dev_desc;
}

void rockchip_set_bootdev(struct blk_desc *desc)
{
	dev_desc = desc;
}

/*
 * detect download key status by adc, most rockchip
 * based boards use adc sample the download key status,
 * but there are also some use gpio. So it's better to
 * make this a weak function that can be override by
 * some special boards.
 */
#define KEY_DOWN_MIN_VAL	0
#define KEY_DOWN_MAX_VAL	30

__weak int rockchip_dnl_key_pressed(void)
{
#if defined(CONFIG_DM_KEY)
	return key_is_pressed(key_read(KEY_VOLUMEUP));

#elif defined(CONFIG_ADC)
	const void *blob = gd->fdt_blob;
	int node, ret, channel = 1;
	u32 val, chns[2];

	node = fdt_node_offset_by_compatible(blob, 0, "adc-keys");
	if (node >= 0) {
		if (!fdtdec_get_int_array(blob, node, "io-channels", chns, 2))
			channel = chns[1];
	}

	ret = adc_channel_single_shot("saradc", channel, &val);
	if (ret) {
		printf("%s: Failed to read saradc, ret=%d\n", __func__, ret);
		return 0;
	}

	return ((val >= KEY_DOWN_MIN_VAL) && (val <= KEY_DOWN_MAX_VAL));
#endif

	return 0;
}

void setup_download_mode(void)
{
	int vbus = 1; /* Assumed 1 in case of no rockusb */

	boot_devtype_init();

	if (rockchip_dnl_key_pressed() || is_hotkey(HK_ROCKUSB_DNL)) {
		printf("download %skey pressed... ",
		       is_hotkey(HK_ROCKUSB_DNL) ? "hot" : "");
#ifdef CONFIG_CMD_ROCKUSB
		vbus = rockchip_u2phy_vbus_detect();
#endif
		if (vbus > 0) {
			printf("%sentering download mode...\n",
			       IS_ENABLED(CONFIG_CMD_ROCKUSB) ?
			       "" : "no rockusb, ");

			/* try rockusb download and brom download */
			run_command("download", 0);
		} else {
			printf("entering recovery mode!\n");
			env_set("reboot_mode", "recovery-key");
		}
	} else if (is_hotkey(HK_FASTBOOT)) {
		env_set("reboot_mode", "fastboot");
	}
}

void board_run_recovery_wipe_data(void)
{
	struct bootloader_message bmsg;
	struct blk_desc *dev_desc;
	disk_partition_t part_info;
#ifdef CONFIG_ANDROID_BOOT_IMAGE
	u32 bcb_offset = android_bcb_msg_sector_offset();
#else
	u32 bcb_offset = BCB_MESSAGE_BLK_OFFSET;
#endif
	int cnt, ret;

	printf("Rebooting into recovery to do wipe_data\n");
	dev_desc = rockchip_get_bootdev();
	if (!dev_desc) {
		printf("%s: dev_desc is NULL!\n", __func__);
		return;
	}

	ret = part_get_info_by_name(dev_desc, PART_MISC, &part_info);
	if (ret < 0) {
		printf("%s: Could not found misc partition, just run recovery\n",
		       __func__);
		goto out;
	}

	memset((char *)&bmsg, 0, sizeof(struct bootloader_message));
	strcpy(bmsg.command, "boot-recovery");
	strcpy(bmsg.recovery, "recovery\n--wipe_data");
	bmsg.status[0] = 0;
	cnt = DIV_ROUND_UP(sizeof(struct bootloader_message), dev_desc->blksz);
	ret = blk_dwrite(dev_desc, part_info.start + bcb_offset, cnt, &bmsg);
	if (ret != cnt)
		printf("Wipe data failed, ret=%d\n", ret);
out:
	/* now reboot to recovery */
	env_set("reboot_mode", "recovery");
	run_command("run bootcmd", 0);
}

#if defined(CONFIG_USING_KERNEL_DTB) || defined(CONFIG_CMD_BOOTM) || \
    defined(CONFIG_CMD_BOOTZ) || defined(CONFIG_CMD_BOOTI)
#ifdef CONFIG_ROCKCHIP_DTB_VERIFY
#ifdef CONFIG_DM_CRYPTO
static int crypto_csum(u32 cap, char *input, u32 input_len, u8 *output)
{
	sha_context csha_ctx;
	struct udevice *dev;

	dev = crypto_get_device(cap);
	if (!dev) {
		printf("Can't find expected crypto device\n");
		return -ENODEV;
	}

	csha_ctx.algo = cap;
	csha_ctx.length = input_len;
	crypto_sha_csum(dev, &csha_ctx, (char *)input,
			input_len, output);

	return 0;
}

static int fdt_check_hash(void *fdt_addr, u32 fdt_size,
			  char *hash_cmp, u32 hash_size)
{
	uchar hash[32];

	if (!hash_size)
		return 0;

	if (hash_size == 20)
		crypto_csum(CRYPTO_SHA1, fdt_addr, fdt_size, hash);
	else if (hash_size == 32)
		crypto_csum(CRYPTO_SHA256, fdt_addr, fdt_size, hash);
	else
		return -EINVAL;

	printf("HASH(c): ");
	if (memcmp(hash, hash_cmp, hash_size)) {
		printf("error\n");
		return -EBADF;
	}

	printf("OK\n");

	return 0;
}

#else
static int fdt_check_hash(void *fdt_addr, u32 fdt_size,
			  char *hash_cmp, u32 hash_size)
{
	uchar hash[32];

	if (!hash_size)
		return 0;

	if (hash_size == 20)
		sha1_csum((const uchar *)fdt_addr, fdt_size, hash);
	else if (hash_size == 32)
		sha256_csum((const uchar *)fdt_addr, fdt_size, hash);
	else
		return -EINVAL;

	printf("HASH(s): ");
	if (memcmp(hash, hash_cmp, hash_size)) {
		printf("error\n");
		return -EBADF;
	}

	printf("OK\n");

	return 0;
}
#endif
#endif	/* CONFIG_ROCKCHIP_DTB_VERIFY */

#if defined(CONFIG_ROCKCHIP_EARLY_DISTRO_DTB)
static int rockchip_read_distro_dtb(void *fdt_addr)
{
	const char *cmd = "part list ${devtype} ${devnum} -bootable devplist";
	char *devnum, *devtype, *devplist;
	char devnum_part[12];
	char fdt_hex_str[19];
	char *fs_argv[5];

	if (!rockchip_get_bootdev() || !fdt_addr)
		return -ENODEV;

	if (run_command_list(cmd, -1, 0)) {
		printf("Failed to find -bootable\n");
		return -EINVAL;
	}

	devplist = env_get("devplist");
	if (!devplist)
		devplist = "1";

	devtype = env_get("devtype");
	devnum = env_get("devnum");
	sprintf(devnum_part, "%s:%s", devnum, devplist);
	sprintf(fdt_hex_str, "0x%lx", (ulong)fdt_addr);

	fs_argv[0] = "load";
	fs_argv[1] = devtype,
	fs_argv[2] = devnum_part;
	fs_argv[3] = fdt_hex_str;
	fs_argv[4] = CONFIG_ROCKCHIP_EARLY_DISTRO_DTB_PATH;

	if (do_load(NULL, 0, 5, fs_argv, FS_TYPE_ANY))
		return -EIO;

	if (fdt_check_header(fdt_addr))
		return -EBADF;

	printf("DTB(Distro): %s\n", CONFIG_ROCKCHIP_EARLY_DISTRO_DTB_PATH);

	return 0;
}
#endif

enum {
	LOCATE_DISTRO,
	LOCATE_RESOURCE,
	LOCATE_FIT,
	LOCATE_END,
};

static int dtb_scan(void *fdt, int where)
{
	if (where == LOCATE_DISTRO) {
#ifdef CONFIG_ROCKCHIP_EARLY_DISTRO_DTB
		return rockchip_read_distro_dtb(fdt);
#endif
	} else if (where == LOCATE_RESOURCE) {
#ifdef CONFIG_ROCKCHIP_RESOURCE_IMAGE
		int hash_size = 0;
		char *hash;
		u32 ret;

		ret = rockchip_read_resource_dtb(fdt, &hash, &hash_size);
		if (ret) {
			printf("Failed to load DTB, ret=%d\n", ret);
			return ret;
		}

		if (fdt_check_header(fdt)) {
			printf("Invalid DTB magic !\n");
			return -EBADF;
		}
#ifdef CONFIG_ROCKCHIP_DTB_VERIFY
		if (hash_size && fdt_check_hash(fdt,
			fdt_totalsize(fdt), hash, hash_size)) {
			printf("Invalid DTB hash !\n");
			return -EBADF;
		}
#endif
		return 0;
#endif
	} else if (where == LOCATE_FIT) {
#if defined(CONFIG_ROCKCHIP_FIT_IMAGE) && !defined(CONFIG_ROCKCHIP_RESOURCE_IMAGE)
		return fit_image_read_dtb(fdt);
#endif
	}

	return -EINVAL;
}

int rockchip_read_dtb_file(void *fdt)
{
	int locate, ret;
	int size;

	for (locate = 0; locate < LOCATE_END; locate++) {
		ret = dtb_scan(fdt, locate);
		if (!ret)
			break;
	}
	if (ret) {
		printf("No valid DTB, ret=%d\n", ret);
		return ret;
	}

	/* reserved memory */
	size = fdt_totalsize(fdt);
	if (!sysmem_alloc_base(MEM_FDT, (phys_addr_t)fdt,
		ALIGN(size, RK_BLK_SIZE) + CONFIG_SYS_FDT_PAD))
		return -ENOMEM;

	/* fixup/overlay */
	rk_board_early_fdt_fixup(fdt);
#if defined(CONFIG_ANDROID_BOOT_IMAGE) && defined(CONFIG_OF_LIBFDT_OVERLAY)
	android_fdt_overlay_apply((void *)fdt);
#endif

	return 0;
}
#endif

int rockchip_ram_read_dtb_file(void *img, void *fdt)
{
	int format;
	int ret;

	format = (genimg_get_format(img));
#ifdef CONFIG_ANDROID_BOOT_IMAGE
	if (format == IMAGE_FORMAT_ANDROID) {
		struct andr_img_hdr *hdr = img;
		struct blk_desc *dev_desc;
		ulong offset;

		dev_desc = rockchip_get_bootdev();
		if (!dev_desc)
			return -ENODEV;

		offset = hdr->page_size + ALIGN(hdr->kernel_size, hdr->page_size) +
			ALIGN(hdr->ramdisk_size, hdr->page_size);
#ifdef CONFIG_ROCKCHIP_RESOURCE_IMAGE
		ret = resource_setup_ram_list(dev_desc, (void *)hdr + offset);
		if (ret)
			return ret;

		return rockchip_read_dtb_file((void *)fdt);
#else
		if (fdt_check_header((void *)offset))
			return -EINVAL;

		memcpy(fdt, (char *)offset, fdt_totalsize(offset));
		if (!sysmem_alloc_base(MEM_FDT, (phys_addr_t)fdt,
			ALIGN(fdt_totalsize(fdt), RK_BLK_SIZE) + CONFIG_SYS_FDT_PAD))
			return -ENOMEM;

		return 0;
#endif
	}
#endif
#if IMAGE_ENABLE_FIT
	if (format == IMAGE_FORMAT_FIT) {
		const void *data;
		size_t size;
		int noffset;
#ifdef CONFIG_ROCKCHIP_RESOURCE_IMAGE
		const char *path = "/images/resource";
#else
		const char *path = "/images/fdt";
#endif

		noffset = fdt_path_offset(img, path);
		if (noffset < 0)
			return noffset;

#ifdef CONFIG_ROCKCHIP_RESOURCE_IMAGE
		ret = fit_image_get_data(img, noffset, &data, &size);
		if (ret < 0)
			return ret;

		dev_desc = rockchip_get_bootdev();
		if (!dev_desc)
			return -ENODEV;

		ret = resource_setup_ram_list(dev_desc, (void *)data);
		if (ret) {
			printf("resource_setup_ram_list fail, ret=%d\n", ret);
			return ret;
		}

		return rockchip_read_dtb_file((void *)fdt);
#else

		ret = fit_image_get_data(img, noffset, &data, &size);
		if (ret)
			return ret;

		if (fdt_check_header(data))
			return -EINVAL;

		memcpy(fdt, data, size);
		if (!sysmem_alloc_base(MEM_FDT, (phys_addr_t)fdt,
			ALIGN(fdt_totalsize(fdt), RK_BLK_SIZE) + CONFIG_SYS_FDT_PAD))
			return -ENOMEM;

		printf("Load DTB from 'images/fdt'\n");

		return 0;
#endif
	}
#endif

	return -EINVAL;
}
