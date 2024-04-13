/*
 * (C) Copyright 2012
 * Texas Instruments, <www.ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef	_SPL_H_
#define	_SPL_H_

/* Platform-specific defines */
#include <mmc.h>
#include <linux/compiler.h>
#include <asm/spl.h>

/* Value in r0 indicates we booted from U-Boot */
#define UBOOT_NOT_LOADED_FROM_SPL	0x13578642

/* Boot type */
#define MMCSD_MODE_UNDEFINED	0
#define MMCSD_MODE_RAW		1
#define MMCSD_MODE_FS		2
#define MMCSD_MODE_EMMCBOOT	3

#define SPL_NEXT_STAGE_UNDEFINED	0
#define SPL_NEXT_STAGE_UBOOT		1
#define SPL_NEXT_STAGE_KERNEL		2

struct spl_image_info {
	const char *name;
	u8 os;
	uintptr_t load_addr;
	uintptr_t entry_point;		/* Next stage entry point */
#if CONFIG_IS_ENABLED(ATF)
	uintptr_t entry_point_bl32;
	uintptr_t entry_point_bl33;
#endif
#if CONFIG_IS_ENABLED(OPTEE) || CONFIG_IS_ENABLED(KERNEL_BOOT)
	uintptr_t entry_point_os;	/* point to uboot or kernel */
#endif
	void *fdt_addr;
	u32 boot_device;
	u32 next_stage;
	u32 size;
	u32 flags;
	void *arg;
};

/*
 * Information required to load data from a device
 *
 * @dev: Pointer to the device, e.g. struct mmc *
 * @priv: Private data for the device
 * @bl_len: Block length for reading in bytes
 * @filename: Name of the fit image file.
 * @read: Function to call to read from the device
 */
struct spl_load_info {
	void *dev;
	void *priv;
	int bl_len;
	const char *filename;
	ulong (*read)(struct spl_load_info *load, ulong sector, ulong count,
		      void *buf);
};

/**
 * spl_load_simple_fit() - Loads a fit image from a device.
 * @spl_image:	Image description to set up
 * @info:	Structure containing the information required to load data.
 * @sector:	Sector number where FIT image is located in the device
 * @fdt:	Pointer to the copied FIT header.
 *
 * Reads the FIT image @sector in the device. Loads u-boot image to
 * specified load address and copies the dtb to end of u-boot image.
 * Returns 0 on success.
 */
int spl_load_simple_fit(struct spl_image_info *spl_image,
			struct spl_load_info *info, ulong sector, void *fdt);

#define SPL_COPY_PAYLOAD_ONLY	1

/* SPL common functions */
void preloader_console_init(void);
u32 spl_boot_device(void);
u32 spl_boot_mode(const u32 boot_device);
void spl_next_stage(struct spl_image_info *spl);
void spl_set_bd(void);

/**
 * spl_set_header_raw_uboot() - Set up a standard SPL image structure
 *
 * This sets up the given spl_image which the standard values obtained from
 * config options: CONFIG_SYS_MONITOR_LEN, CONFIG_SYS_UBOOT_START,
 * CONFIG_SYS_TEXT_BASE.
 *
 * @spl_image: Image description to set up
 */
void spl_set_header_raw_uboot(struct spl_image_info *spl_image);

/**
 * spl_parse_image_header() - parse the image header and set up info
 *
 * This parses the legacy image header information at @header and sets up
 * @spl_image according to what is found. If no image header is found, then
 * a raw image or bootz is assumed. If CONFIG_SPL_PANIC_ON_RAW_IMAGE is
 * enabled, then this causes a panic. If CONFIG_SPL_RAW_IMAGE_SUPPORT is not
 * enabled then U-Boot gives up. Otherwise U-Boot sets up the image using
 * spl_set_header_raw_uboot(), or possibly the bootz header.
 *
 * @spl_image: Image description to set up
 * @header image header to parse
 * @return 0 if a header was correctly parsed, -ve on error
 */
int spl_parse_image_header(struct spl_image_info *spl_image,
			   const struct image_header *header);

void spl_board_prepare_for_linux(void);
void spl_board_prepare_for_boot(void);
int spl_board_ubi_load_image(u32 boot_device);

/**
 * jump_to_image_linux() - Jump to a Linux kernel from SPL
 *
 * This jumps into a Linux kernel using the information in @spl_image.
 *
 * @spl_image: Image description to set up
 */
void __noreturn jump_to_image_linux(struct spl_image_info *spl_image);

/**
 * spl_start_uboot() - Check if SPL should start the kernel or U-Boot
 *
 * This is called by the various SPL loaders to determine whether the board
 * wants to load the kernel or U-Boot. This function should be provided by
 * the board.
 *
 * @return 0 if SPL should start the kernel, 1 if U-Boot must be started
 */
int spl_start_uboot(void);

/**
 * spl_display_print() - Display a board-specific message in SPL
 *
 * If CONFIG_SPL_DISPLAY_PRINT is enabled, U-Boot will call this function
 * immediately after displaying the SPL console banner ("U-Boot SPL ...").
 * This function should be provided by the board.
 */
void spl_display_print(void);

/**
 * struct spl_boot_device - Describes a boot device used by SPL
 *
 * @boot_device: A number indicating the BOOT_DEVICE type. There are various
 * BOOT_DEVICE... #defines and enums in U-Boot and they are not consistently
 * numbered.
 * @boot_device_name: Named boot device, or NULL if none.
 *
 * Note: Additional fields can be added here, bearing in mind that SPL is
 * size-sensitive and common fields will be present on all boards. This
 * struct can also be used to return additional information about the load
 * process if that becomes useful.
 */
struct spl_boot_device {
	uint boot_device;
	const char *boot_device_name;
};

/**
 * Holds information about a way of loading an SPL image
 *
 * @name: User-friendly name for this method (e.g. "MMC")
 * @boot_device: Boot device that this loader supports
 * @load_image: Function to call to load image
 */
struct spl_image_loader {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
	const char *name;
#endif
	uint boot_device;
	/**
	 * load_image() - Load an SPL image
	 *
	 * @spl_image: place to put image information
	 * @bootdev: describes the boot device to load from
	 */
	int (*load_image)(struct spl_image_info *spl_image,
			  struct spl_boot_device *bootdev);
};

/* Declare an SPL image loader */
#define SPL_LOAD_IMAGE(__name)					\
	ll_entry_declare(struct spl_image_loader, __name, spl_image_loader)

/*
 * _priority is the priority of this method, 0 meaning it will be the top
 * choice for this device, 9 meaning it is the bottom choice.
 * _boot_device is the BOOT_DEVICE_... value
 * _method is the load_image function to call
 */
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
#define SPL_LOAD_IMAGE_METHOD(_name, _priority, _boot_device, _method) \
	SPL_LOAD_IMAGE(_method ## _priority ## _boot_device) = { \
		.name = _name, \
		.boot_device = _boot_device, \
		.load_image = _method, \
	}
#else
#define SPL_LOAD_IMAGE_METHOD(_name, _priority, _boot_device, _method) \
	SPL_LOAD_IMAGE(_method ## _priority ## _boot_device) = { \
		.boot_device = _boot_device, \
		.load_image = _method, \
	}
#endif

/* SPL FAT image functions */
int spl_load_image_fat(struct spl_image_info *spl_image,
		       struct blk_desc *block_dev, int partition,
		       const char *filename);
int spl_load_image_fat_os(struct spl_image_info *spl_image,
			  struct blk_desc *block_dev, int partition);

void __noreturn jump_to_image_no_args(struct spl_image_info *spl_image);

/* SPL EXT image functions */
int spl_load_image_ext(struct spl_image_info *spl_image,
		       struct blk_desc *block_dev, int partition,
		       const char *filename);
int spl_load_image_ext_os(struct spl_image_info *spl_image,
			  struct blk_desc *block_dev, int partition);

/**
 * spl_early_init() - Set up device tree and driver model in SPL if enabled
 *
 * Call this function in board_init_f() if you want to use device tree and
 * driver model early, before board_init_r() is called.
 *
 * If this is not called, then driver model will be inactive in SPL's
 * board_init_f(), and no device tree will be available.
 */
int spl_early_init(void);

/**
 * spl_init() - Set up device tree and driver model in SPL if enabled
 *
 * You can optionally call spl_early_init(), then optionally call spl_init().
 * This function will be called from board_init_r() if not called earlier.
 *
 * Both spl_early_init() and spl_init() perform a similar function except that
 * the latter will not set up the malloc() area if
 * CONFIG_SPL_STACK_R_MALLOC_SIMPLE_LEN is enabled, since it is assumed to
 * already be done by a calll to spl_relocate_stack_gd() before board_init_r()
 * is reached.
 *
 * This function will be called from board_init_r() if not called earlier.
 *
 * If this is not called, then driver model will be inactive in SPL's
 * board_init_f(), and no device tree will be available.
 */
int spl_init(void);

#ifdef CONFIG_SPL_BOARD_INIT
void spl_board_init(void);
#endif

/**
 * spl_was_boot_source() - check if U-Boot booted from SPL
 *
 * This will normally be true, but if U-Boot jumps to second U-Boot, it will
 * be false. This should be implemented by board-specific code.
 *
 * @return true if U-Boot booted from SPL, else false
 */
bool spl_was_boot_source(void);

/**
 * spl_dfu_cmd- run dfu command with chosen mmc device interface
 * @param usb_index - usb controller number
 * @param mmc_dev -  mmc device nubmer
 *
 * @return 0 on success, otherwise error code
 */
int spl_dfu_cmd(int usbctrl, char *dfu_alt_info, char *interface, char *devstr);

int spl_mmc_find_device(struct mmc **mmcp, u32 boot_device);
int spl_mmc_load_image(struct spl_image_info *spl_image,
		       struct spl_boot_device *bootdev);

/**
 * spl_invoke_atf - boot using an ARM trusted firmware image
 */
void spl_invoke_atf(struct spl_image_info *spl_image);

/**
 * bl31_entry - Fill bl31_params structure, and jump to bl31
 */
void bl31_entry(struct spl_image_info *spl_image,
		uintptr_t bl31_entry, uintptr_t bl32_entry,
		uintptr_t bl33_entry, uintptr_t fdt_addr);

/**
 * spl_optee_entry - entry function for optee
 *
 * args defind in op-tee project
 * https://github.com/OP-TEE/optee_os/
 * core/arch/arm/kernel/generic_entry_a32.S
 * @arg0: pagestore
 * @arg1: (ARMv7 standard bootarg #1)
 * @arg2: device tree address, (ARMv7 standard bootarg #2)
 * @arg3: non-secure entry address (ARMv7 bootarg #0)
 */
void spl_optee_entry(void *arg0, void *arg1, void *arg2, void *arg3);

/**
 * board_return_to_bootrom - allow for boards to continue with the boot ROM
 *
 * If a board (e.g. the Rockchip RK3368 boards) provide some
 * supporting functionality for SPL in their boot ROM and the SPL
 * stage wants to return to the ROM code to continue booting, boards
 * can implement 'board_return_to_bootrom'.
 */
void board_return_to_bootrom(void);

/**
 * spl_cleanup_before_jump() - cleanup cache/mmu/interrupt, etc before jump
 *			       to next stage.
 */
void spl_cleanup_before_jump(struct spl_image_info *spl_image);

/**
 * spl_perform_fixups() - arch/board-specific callback before processing
 *                        the boot-payload
 */
void spl_perform_fixups(struct spl_image_info *spl_image);

/**
 * spl_board_prepare_for_jump() - arch/board-specific callback exactly before
 *				  jumping to next stage
 */
int spl_board_prepare_for_jump(struct spl_image_info *spl_image);

/**
 * spl_kernel_partition() - arch/board-specific callback to get kernel partition
 */
#ifdef CONFIG_SPL_KERNEL_BOOT
const char *spl_kernel_partition(struct spl_image_info *spl,
				 struct spl_load_info *info);
#endif

#endif
