/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2002-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <api.h>
/* TODO: can we just include all these headers whether needed or not? */
#if defined(CONFIG_CMD_BEDBUG)
#include <bedbug/type.h>
#endif
#include <command.h>
#include <console.h>
#include <dm.h>
#include <environment.h>
#include <fdtdec.h>
#include <ide.h>
#include <initcall.h>
#include <init_helpers.h>
#ifdef CONFIG_PS2KBD
#include <keyboard.h>
#endif
#if defined(CONFIG_CMD_KGDB)
#include <kgdb.h>
#endif
#include <malloc.h>
#include <mapmem.h>
#include <memalign.h>
#ifdef CONFIG_BITBANGMII
#include <miiphy.h>
#endif
#include <mmc.h>
#include <nand.h>
#include <of_live.h>
#include <onenand_uboot.h>
#include <scsi.h>
#include <serial.h>
#include <spi.h>
#include <stdio_dev.h>
#include <timer.h>
#include <trace.h>
#include <watchdog.h>
#ifdef CONFIG_ADDR_MAP
#include <asm/mmu.h>
#endif
#include <asm/sections.h>
#include <asm/system.h>
#include <dm/root.h>
#include <linux/compiler.h>
#include <linux/err.h>
#include <efi_loader.h>
#include <sysmem.h>
#include <bidram.h>
#include <boot_rkimg.h>
#include <mtd_blk.h>
#if defined(CONFIG_GPIO_HOG)
#include <asm/gpio.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

ulong monitor_flash_len;

__weak int board_flash_wp_on(void)
{
	/*
	 * Most flashes can't be detected when write protection is enabled,
	 * so provide a way to let U-Boot gracefully ignore write protected
	 * devices.
	 */
	return 0;
}

__weak void cpu_secondary_init_r(void)
{
}

static int initr_secondary_cpu(void)
{
	/*
	 * after non-volatile devices & environment is setup and cpu code have
	 * another round to deal with any initialization that might require
	 * full access to the environment or loading of some image (firmware)
	 * from a non-volatile device
	 */
	/* TODO: maybe define this for all archs? */
	cpu_secondary_init_r();

	return 0;
}

static int initr_trace(void)
{
#ifdef CONFIG_TRACE
	trace_init(gd->trace_buff, CONFIG_TRACE_BUFFER_SIZE);
#endif

	return 0;
}

static int initr_reloc(void)
{
	/* tell others: relocation done */
	gd->flags |= GD_FLG_RELOC | GD_FLG_FULL_MALLOC_INIT;

	return 0;
}

#ifdef CONFIG_ARM
/*
 * Some of these functions are needed purely because the functions they
 * call return void. If we change them to return 0, these stubs can go away.
 */

static void print_cr(void)
{
	u32 reg;

#ifdef CONFIG_ARM64
	reg = get_sctlr();	/* get control reg. */
#else
	reg = get_cr();
#endif
	puts("CR: ");
	if (reg & CR_M)
		puts("M/");
	if (reg & CR_C)
		puts("C/");
	if (reg & CR_I)
		puts("I");
	putc('\n');
}

static int initr_caches(void)
{
	/* Enable caches */
	enable_caches();
	print_cr();

	return 0;
}
#endif

__weak int fixup_cpu(void)
{
	return 0;
}

static int initr_reloc_global_data(void)
{
#ifdef __ARM__
	monitor_flash_len = _end - __image_copy_start;
#elif defined(CONFIG_NDS32) || defined(CONFIG_RISCV)
	monitor_flash_len = (ulong)&_end - (ulong)&_start;
#elif !defined(CONFIG_SANDBOX) && !defined(CONFIG_NIOS2)
	monitor_flash_len = (ulong)&__init_end - gd->relocaddr;
#endif
#if defined(CONFIG_MPC85xx) || defined(CONFIG_MPC86xx)
	/*
	 * The gd->cpu pointer is set to an address in flash before relocation.
	 * We need to update it to point to the same CPU entry in RAM.
	 * TODO: why not just add gd->reloc_ofs?
	 */
	gd->arch.cpu += gd->relocaddr - CONFIG_SYS_MONITOR_BASE;

	/*
	 * If we didn't know the cpu mask & # cores, we can save them of
	 * now rather than 'computing' them constantly
	 */
	fixup_cpu();
#endif
#ifdef CONFIG_SYS_EXTRA_ENV_RELOC
	/*
	 * Some systems need to relocate the env_addr pointer early because the
	 * location it points to will get invalidated before env_relocate is
	 * called.  One example is on systems that might use a L2 or L3 cache
	 * in SRAM mode and initialize that cache from SRAM mode back to being
	 * a cache in cpu_init_r.
	 */
	gd->env_addr += gd->relocaddr - CONFIG_SYS_MONITOR_BASE;
#endif
#ifdef CONFIG_OF_EMBED
	/*
	* The fdt_blob needs to be moved to new relocation address
	* incase of FDT blob is embedded with in image
	*/
	gd->fdt_blob += gd->reloc_off;
#endif
#ifdef CONFIG_EFI_LOADER
	efi_runtime_relocate(gd->relocaddr, NULL);
#endif

	return 0;
}

static int initr_serial(void)
{
	serial_initialize();
	return 0;
}

#if defined(CONFIG_PPC) || defined(CONFIG_M68K) || defined(CONFIG_MIPS)
static int initr_trap(void)
{
	/*
	 * Setup trap handlers
	 */
#if defined(CONFIG_PPC)
	trap_init(gd->relocaddr);
#else
	trap_init(CONFIG_SYS_SDRAM_BASE);
#endif
	return 0;
}
#endif

#ifdef CONFIG_ADDR_MAP
static int initr_addr_map(void)
{
	init_addr_map();

	return 0;
}
#endif

#ifdef CONFIG_POST
static int initr_post_backlog(void)
{
	post_output_backlog();
	return 0;
}
#endif

#if defined(CONFIG_SYS_INIT_RAM_LOCK) && defined(CONFIG_E500)
static int initr_unlock_ram_in_cache(void)
{
	unlock_ram_in_cache();	/* it's time to unlock D-cache in e500 */
	return 0;
}
#endif

#ifdef CONFIG_PCI
static int initr_pci(void)
{
#ifndef CONFIG_DM_PCI
	pci_init();
#endif

	return 0;
}
#endif

static int initr_barrier(void)
{
#ifdef CONFIG_PPC
	/* TODO: Can we not use dmb() macros for this? */
	asm("sync ; isync");
#endif
	return 0;
}

static int initr_malloc(void)
{
	ulong malloc_start;

#if CONFIG_VAL(SYS_MALLOC_F_LEN)
	debug("Pre-reloc malloc() used %#lx bytes (%ld KB)\n", gd->malloc_ptr,
	      gd->malloc_ptr / 1024);
#endif
	/* The malloc area is immediately below the monitor copy in DRAM */
	malloc_start = gd->relocaddr - TOTAL_MALLOC_LEN;
	mem_malloc_init((ulong)map_sysmem(malloc_start, TOTAL_MALLOC_LEN),
			TOTAL_MALLOC_LEN);
	return 0;
}

static int initr_console_record(void)
{
#if defined(CONFIG_CONSOLE_RECORD)
	int ret;

	ret = console_record_init();
	if (!ret)
		console_record_reset_enable();
	return ret;
#else
	return 0;
#endif
}

#ifdef CONFIG_SYS_NONCACHED_MEMORY
static int initr_noncached(void)
{
	noncached_init();
	return 0;
}
#endif

#ifdef CONFIG_OF_LIVE
static int initr_of_live(void)
{
	int ret;

	bootstage_start(BOOTSTAGE_ID_ACCUM_OF_LIVE, "of_live");
	ret = of_live_build(gd->fdt_blob, (struct device_node **)&gd->of_root);
	bootstage_accum(BOOTSTAGE_ID_ACCUM_OF_LIVE);
	if (ret)
		return ret;

	return 0;
}
#endif

#ifdef CONFIG_DM
static int initr_dm(void)
{
	int ret;

	/* Save the pre-reloc driver model and start a new one */
	gd->dm_root_f = gd->dm_root;
	gd->dm_root = NULL;
#ifdef CONFIG_TIMER
	gd->timer = NULL;
#endif
	bootstage_start(BOOTSTATE_ID_ACCUM_DM_R, "dm_r");
	ret = dm_init_and_scan(false);
	bootstage_accum(BOOTSTATE_ID_ACCUM_DM_R);
	if (ret)
		return ret;
#ifdef CONFIG_TIMER_EARLY
	ret = dm_timer_init();
	if (ret)
		return ret;
#endif

	return 0;
}
#endif

static int initr_bootstage(void)
{
	bootstage_mark_name(BOOTSTAGE_ID_START_UBOOT_R, "board_init_r");

	return 0;
}

__weak int power_init_board(void)
{
	return 0;
}

static int initr_announce(void)
{
	ulong addr;

#ifndef CONFIG_SKIP_RELOCATE_UBOOT
	addr = gd->relocaddr;
#else
	addr = CONFIG_SYS_TEXT_BASE;
#endif
	debug("Now running in RAM - U-Boot at: %08lx\n", addr);

	return 0;
}

#ifdef CONFIG_NEEDS_MANUAL_RELOC
static int initr_manual_reloc_cmdtable(void)
{
	fixup_cmdtable(ll_entry_start(cmd_tbl_t, cmd),
		       ll_entry_count(cmd_tbl_t, cmd));
	return 0;
}
#endif

#if defined(CONFIG_MTD_NOR_FLASH)
static int initr_flash(void)
{
	ulong flash_size = 0;
	bd_t *bd = gd->bd;

	puts("Flash: ");

	if (board_flash_wp_on())
		printf("Uninitialized - Write Protect On\n");
	else
		flash_size = flash_init();

	print_size(flash_size, "");
#ifdef CONFIG_SYS_FLASH_CHECKSUM
	/*
	* Compute and print flash CRC if flashchecksum is set to 'y'
	*
	* NOTE: Maybe we should add some WATCHDOG_RESET()? XXX
	*/
	if (env_get_yesno("flashchecksum") == 1) {
		printf("  CRC: %08X", crc32(0,
			(const unsigned char *) CONFIG_SYS_FLASH_BASE,
			flash_size));
	}
#endif /* CONFIG_SYS_FLASH_CHECKSUM */
	putc('\n');

	/* update start of FLASH memory    */
#ifdef CONFIG_SYS_FLASH_BASE
	bd->bi_flashstart = CONFIG_SYS_FLASH_BASE;
#endif
	/* size of FLASH memory (final value) */
	bd->bi_flashsize = flash_size;

#if defined(CONFIG_SYS_UPDATE_FLASH_SIZE)
	/* Make a update of the Memctrl. */
	update_flash_size(flash_size);
#endif


#if defined(CONFIG_OXC) || defined(CONFIG_RMU)
	/* flash mapped at end of memory map */
	bd->bi_flashoffset = CONFIG_SYS_TEXT_BASE + flash_size;
#elif CONFIG_SYS_MONITOR_BASE == CONFIG_SYS_FLASH_BASE
	bd->bi_flashoffset = monitor_flash_len;	/* reserved area for monitor */
#endif
	return 0;
}
#endif

#if defined(CONFIG_PPC) && !defined(CONFIG_DM_SPI)
static int initr_spi(void)
{
	/* MPC8xx does this here */
#ifdef CONFIG_MPC8XX_SPI
#if !defined(CONFIG_ENV_IS_IN_EEPROM)
	spi_init_f();
#endif
	spi_init_r();
#endif
	return 0;
}
#endif

#ifdef CONFIG_CMD_NAND
/* go init the NAND */
static int initr_nand(void)
{
#ifndef CONFIG_USING_KERNEL_DTB
	puts("NAND:  ");
	nand_init();
	printf("%lu MiB\n", nand_size() / 1024);
#endif
	return 0;
}
#endif

#if defined(CONFIG_CMD_ONENAND)
/* go init the NAND */
static int initr_onenand(void)
{
	puts("NAND:  ");
	onenand_init();
	return 0;
}
#endif

#ifdef CONFIG_MMC
static int initr_mmc(void)
{
/*
 * When CONFIG_USING_KERNEL_DTB is enabled, mmc has been initialized earlier.
 */
#ifndef CONFIG_USING_KERNEL_DTB
	puts("MMC:   ");
	mmc_initialize(gd->bd);
#endif
	return 0;
}
#endif

#if !defined(CONFIG_USING_KERNEL_DTB) || !defined(CONFIG_ENV_IS_NOWHERE)
/*
 * Tell if it's OK to load the environment early in boot.
 *
 * If CONFIG_OF_CONTROL is defined, we'll check with the FDT to see
 * if this is OK (defaulting to saying it's OK).
 *
 * NOTE: Loading the environment early can be a bad idea if security is
 *       important, since no verification is done on the environment.
 *
 * @return 0 if environment should not be loaded, !=0 if it is ok to load
 */
static int should_load_env(void)
{
#ifdef CONFIG_OF_CONTROL
	return fdtdec_get_config_int(gd->fdt_blob, "load-environment", 1);
#elif defined CONFIG_DELAY_ENVIRONMENT
	return 0;
#else
	return 1;
#endif
}

static int initr_env(void)
{
	/* initialize environment */
	if (should_load_env())
		env_relocate();
	else
		set_default_env(NULL);
#ifdef CONFIG_OF_CONTROL
	env_set_addr("fdtcontroladdr", gd->fdt_blob);
#endif

	/* Initialize from environment */
	load_addr = env_get_ulong("loadaddr", 16, load_addr);

	return 0;
}
#endif

#ifdef CONFIG_USING_KERNEL_DTB
/*
 * If !defined(CONFIG_ENV_IS_NOWHERE):
 *
 * Storage env or kernel dtb load depends on bootdev, while bootdev
 * depends on env varname: devtype, devnum and rkimg_bootdev, etc.
 * So we have to use nowhere env firstly and cover the storage env
 * after it is loaded.
 *
 * Providing a minimum and necessary nowhere env for board init to
 * avoid covering the other varnames in storage env.
 */
static int initr_env_nowhere(void)
{
#ifdef CONFIG_ENV_IS_NOWHERE
	set_default_env(NULL);
#if defined(CONFIG_ENVF)
	/* init envf and partitiont from envf before any partition query action */
	env_load();
#endif
	return 0;
#else
	const char env_minimum[] = {
		ENV_MEM_LAYOUT_SETTINGS
#ifdef ENV_MEM_LAYOUT_SETTINGS1
		ENV_MEM_LAYOUT_SETTINGS1
#endif
#ifdef RKIMG_DET_BOOTDEV
		RKIMG_DET_BOOTDEV
#endif
	};

	return set_board_env((char *)env_minimum, ENV_SIZE, 0, true);
#endif
}

#if !defined(CONFIG_ENV_IS_NOWHERE)
/*
 * storage has been initialized in board_init(), we could switch env
 * from nowhere to storage, i.e. CONFIG_ENV_IS_IN_xxx=y.
 */
static int initr_env_switch(void)
{
	ALLOC_CACHE_ALIGN_BUFFER(env_t, env_nowhere, 1);
	char *data;
	int ret;

	data = env_get("bootargs");
	if (data) {
		env_set("bootargs_tmp", data);
		env_set("bootargs", NULL);
	}

	/* Export nowhere env for late use */
	ret = env_export(env_nowhere);
	if (ret) {
		printf("%s: export nowhere env fail, ret=%d\n", __func__, ret);
		return -EINVAL;
	}

	/* Destroy nowhere env and import storage env */
	initr_env();

	/* Append/override nowhere env to storage env */
	set_board_env((char *)env_nowhere->data, ENV_SIZE, H_NOCLEAR, false);

	/*
	 * Restore nowhere bootargs to append/override the one in env storage.
	 *
	 * Without this, the entire "bootargs" in storage env is replaces by
	 * the one in env_nowhere->data.
	 */
	data = env_get("bootargs_tmp");
	if (data) {
		env_update("bootargs", data);
		env_set("bootargs_tmp", NULL);
	}

	return 0;
}
#endif	/* CONFIG_ENV_IS_NOWHERE */
#endif	/* CONFIG_USING_KERNEL_DTB */

#ifdef CONFIG_SYS_BOOTPARAMS_LEN
static int initr_malloc_bootparams(void)
{
	gd->bd->bi_boot_params = (ulong)malloc(CONFIG_SYS_BOOTPARAMS_LEN);
	if (!gd->bd->bi_boot_params) {
		puts("WARNING: Cannot allocate space for boot parameters\n");
		return -ENOMEM;
	}
	return 0;
}
#endif

static int initr_jumptable(void)
{
	jumptable_init();
	return 0;
}

#if defined(CONFIG_API)
static int initr_api(void)
{
	/* Initialize API */
	api_init();
	return 0;
}
#endif

/* enable exceptions */
#ifdef CONFIG_ARM
static int initr_enable_interrupts(void)
{
	enable_interrupts();
	return 0;
}
#endif

#ifdef CONFIG_CMD_NET
static int initr_ethaddr(void)
{
	bd_t *bd = gd->bd;

	/* kept around for legacy kernels only ... ignore the next section */
	eth_env_get_enetaddr("ethaddr", bd->bi_enetaddr);
#ifdef CONFIG_HAS_ETH1
	eth_env_get_enetaddr("eth1addr", bd->bi_enet1addr);
#endif
#ifdef CONFIG_HAS_ETH2
	eth_env_get_enetaddr("eth2addr", bd->bi_enet2addr);
#endif
#ifdef CONFIG_HAS_ETH3
	eth_env_get_enetaddr("eth3addr", bd->bi_enet3addr);
#endif
#ifdef CONFIG_HAS_ETH4
	eth_env_get_enetaddr("eth4addr", bd->bi_enet4addr);
#endif
#ifdef CONFIG_HAS_ETH5
	eth_env_get_enetaddr("eth5addr", bd->bi_enet5addr);
#endif
	return 0;
}
#endif /* CONFIG_CMD_NET */

#ifdef CONFIG_CMD_KGDB
static int initr_kgdb(void)
{
	puts("KGDB:  ");
	kgdb_init();
	return 0;
}
#endif

#if defined(CONFIG_LED_STATUS)
static int initr_status_led(void)
{
#if defined(CONFIG_LED_STATUS_BOOT)
	status_led_set(CONFIG_LED_STATUS_BOOT, CONFIG_LED_STATUS_BLINKING);
#else
	status_led_init();
#endif
	return 0;
}
#endif

#if defined(CONFIG_SCSI) && !defined(CONFIG_DM_SCSI)
static int initr_scsi(void)
{
	puts("SCSI:  ");
	scsi_init();

	return 0;
}
#endif

#ifdef CONFIG_BITBANGMII
static int initr_bbmii(void)
{
	bb_miiphy_init();
	return 0;
}
#endif

#ifdef CONFIG_CMD_NET
static int initr_net(void)
{
	puts("Net:   ");
	eth_initialize();
#if defined(CONFIG_RESET_PHY_R)
	debug("Reset Ethernet PHY\n");
	reset_phy();
#endif
	return 0;
}
#endif

#ifdef CONFIG_POST
static int initr_post(void)
{
	post_run(NULL, POST_RAM | post_bootmode_get(0));
	return 0;
}
#endif

#if defined(CONFIG_CMD_PCMCIA) && !defined(CONFIG_IDE)
static int initr_pcmcia(void)
{
	puts("PCMCIA:");
	pcmcia_init();
	return 0;
}
#endif

#if defined(CONFIG_IDE)
static int initr_ide(void)
{
	puts("IDE:   ");
#if defined(CONFIG_START_IDE)
	if (board_start_ide())
		ide_init();
#else
	ide_init();
#endif
	return 0;
}
#endif

#if defined(CONFIG_PRAM)
/*
 * Export available size of memory for Linux, taking into account the
 * protected RAM at top of memory
 */
int initr_mem(void)
{
	ulong pram = 0;
	char memsz[32];

# ifdef CONFIG_PRAM
	pram = env_get_ulong("pram", 10, CONFIG_PRAM);
# endif
	sprintf(memsz, "%ldk", (long int) ((gd->ram_size / 1024) - pram));
	env_set("mem", memsz);

	return 0;
}
#endif

#ifdef CONFIG_CMD_BEDBUG
static int initr_bedbug(void)
{
	bedbug_init();

	return 0;
}
#endif

#ifdef CONFIG_PS2KBD
static int initr_kbd(void)
{
	puts("PS/2:  ");
	kbd_init();
	return 0;
}
#endif

__weak int interrupt_debugger_init(void)
{
	return 0;
}

__weak int board_initr_caches_fixup(void)
{
	return 0;
}

static int run_main_loop(void)
{
#ifdef CONFIG_SANDBOX
	sandbox_main_loop_init();
#endif
	/* main_loop() can return to retry autoboot, if so just run it again */
	for (;;)
		main_loop();
	return 0;
}

/*
 * Over time we hope to remove these functions with code fragments and
 * stub funtcions, and instead call the relevant function directly.
 *
 * We also hope to remove most of the driver-related init and do it if/when
 * the driver is later used.
 *
 * TODO: perhaps reset the watchdog in the initcall function after each call?
 */
static init_fnc_t init_sequence_r[] = {
	initr_trace,
	initr_reloc,
	/* TODO: could x86/PPC have this also perhaps? */
#ifdef CONFIG_ARM
	initr_caches,
	/* Note: For Freescale LS2 SoCs, new MMU table is created in DDR.
	 *	 A temporary mapping of IFC high region is since removed,
	 *	 so environmental variables in NOR flash is not availble
	 *	 until board_init() is called below to remap IFC to high
	 *	 region.
	 */
#endif
	initr_reloc_global_data,

	/*
	 * Some platform requires to reserve memory regions for some firmware
	 * to avoid kernel touches it, but U-Boot may have communication with
	 * firmware by share memory. So that we had better reserve firmware
	 * region after the initr_caches() which enables MMU and init
	 * translation table, we need firmware region to be mapped as cacheable
	 * like other regions, otherwise there would be dcache coherence issue
	 * between firmware and U-Boot.
	 */
	board_initr_caches_fixup,

#if defined(CONFIG_SYS_INIT_RAM_LOCK) && defined(CONFIG_E500)
	initr_unlock_ram_in_cache,
#endif
	initr_barrier,
	initr_malloc,
#ifdef CONFIG_BIDRAM
	bidram_initr,
#endif
#ifdef CONFIG_SYSMEM
	sysmem_initr,
#endif
	log_init,
	initr_bootstage,	/* Needs malloc() but has its own timer */
	initr_console_record,
#ifdef CONFIG_SYS_NONCACHED_MEMORY
	initr_noncached,
#endif
	bootstage_relocate,

	interrupt_init,
#ifdef CONFIG_ARM
	initr_enable_interrupts,
#endif
	interrupt_debugger_init,

#ifdef CONFIG_OF_LIVE
	initr_of_live,
#endif
#ifdef CONFIG_DM
	initr_dm,
#endif

#ifdef CONFIG_USING_KERNEL_DTB
	initr_env_nowhere,
#endif
#if defined(CONFIG_BOARD_EARLY_INIT_R)
	board_early_init_r,
#endif

#if defined(CONFIG_ARM) || defined(CONFIG_NDS32) || defined(CONFIG_RISCV)
	board_init,	/* Setup chipselects */
#endif
#if defined(CONFIG_USING_KERNEL_DTB) && !defined(CONFIG_ENV_IS_NOWHERE)
	initr_env_switch,
#endif
	/*
	 * TODO: printing of the clock inforamtion of the board is now
	 * implemented as part of bdinfo command. Currently only support for
	 * davinci SOC's is added. Remove this check once all the board
	 * implement this.
	 */
#ifdef CONFIG_CLOCKS
	set_cpu_clk_info, /* Setup clock information */
#endif
#ifdef CONFIG_EFI_LOADER
	efi_memory_init,
#endif
	stdio_init_tables,
	initr_serial,
	initr_announce,
	INIT_FUNC_WATCHDOG_RESET
#ifdef CONFIG_NEEDS_MANUAL_RELOC
	initr_manual_reloc_cmdtable,
#endif
#if defined(CONFIG_PPC) || defined(CONFIG_M68K) || defined(CONFIG_MIPS)
	initr_trap,
#endif
#ifdef CONFIG_ADDR_MAP
	initr_addr_map,
#endif
	INIT_FUNC_WATCHDOG_RESET
#ifdef CONFIG_POST
	initr_post_backlog,
#endif
	INIT_FUNC_WATCHDOG_RESET

#ifndef CONFIG_USING_KERNEL_DTB
	/* init before storage(for: devtype, devnum, ...) */
	initr_env,
#endif
#if defined(CONFIG_PCI) && defined(CONFIG_SYS_EARLY_PCI_INIT)
	/*
	 * Do early PCI configuration _before_ the flash gets initialised,
	 * because PCU ressources are crucial for flash access on some boards.
	 */
	initr_pci,
#endif
#ifdef CONFIG_ARCH_EARLY_INIT_R
	arch_early_init_r,
#endif
	power_init_board,
#ifdef CONFIG_MTD_NOR_FLASH
	initr_flash,
#endif
	INIT_FUNC_WATCHDOG_RESET
#if defined(CONFIG_PPC) || defined(CONFIG_M68K) || defined(CONFIG_X86)
	/* initialize higher level parts of CPU like time base and timers */
	cpu_init_r,
#endif
#ifdef CONFIG_PPC
	initr_spi,
#endif
#ifdef CONFIG_CMD_NAND
	initr_nand,
#endif
#ifdef CONFIG_CMD_ONENAND
	initr_onenand,
#endif
#ifdef CONFIG_MMC
	initr_mmc,
#endif

#ifdef CONFIG_SYS_BOOTPARAMS_LEN
	initr_malloc_bootparams,
#endif
	INIT_FUNC_WATCHDOG_RESET
	initr_secondary_cpu,
#if defined(CONFIG_ID_EEPROM) || defined(CONFIG_SYS_I2C_MAC_OFFSET)
	mac_read_from_eeprom,
#endif
	INIT_FUNC_WATCHDOG_RESET
#if defined(CONFIG_PCI) && !defined(CONFIG_SYS_EARLY_PCI_INIT)
	/*
	 * Do pci configuration
	 */
	initr_pci,
#endif
	stdio_add_devices,
	initr_jumptable,
#ifdef CONFIG_API
	initr_api,
#endif
	console_init_r,		/* fully init console as a device */
#ifdef CONFIG_DISPLAY_BOARDINFO_LATE
	console_announce_r,
	show_board_info,
#endif
#ifdef CONFIG_ARCH_MISC_INIT
	arch_misc_init,		/* miscellaneous arch-dependent init */
#endif
#ifdef CONFIG_MISC_INIT_R
	misc_init_r,		/* miscellaneous platform-dependent init */
#endif
	INIT_FUNC_WATCHDOG_RESET
#ifdef CONFIG_CMD_KGDB
	initr_kgdb,
#endif

#if defined(CONFIG_MICROBLAZE) || defined(CONFIG_M68K)
	timer_init,		/* initialize timer */
#endif
#if defined(CONFIG_LED_STATUS)
	initr_status_led,
#endif
	/* PPC has a udelay(20) here dating from 2002. Why? */
#ifdef CONFIG_CMD_NET
	initr_ethaddr,
#endif
#if defined(CONFIG_GPIO_HOG)
	gpio_hog_probe_all,
#endif
#ifdef CONFIG_BOARD_LATE_INIT
	board_late_init,
#endif
#if defined(CONFIG_SCSI) && !defined(CONFIG_DM_SCSI)
	INIT_FUNC_WATCHDOG_RESET
	initr_scsi,
#endif
#ifdef CONFIG_BITBANGMII
	initr_bbmii,
#endif
#ifdef CONFIG_CMD_NET
	INIT_FUNC_WATCHDOG_RESET
	initr_net,
#endif
#ifdef CONFIG_POST
	initr_post,
#endif
#if defined(CONFIG_CMD_PCMCIA) && !defined(CONFIG_IDE)
	initr_pcmcia,
#endif
#if defined(CONFIG_IDE)
	initr_ide,
#endif
#ifdef CONFIG_LAST_STAGE_INIT
	INIT_FUNC_WATCHDOG_RESET
	/*
	 * Some parts can be only initialized if all others (like
	 * Interrupts) are up and running (i.e. the PC-style ISA
	 * keyboard).
	 */
	last_stage_init,
#endif
#ifdef CONFIG_CMD_BEDBUG
	INIT_FUNC_WATCHDOG_RESET
	initr_bedbug,
#endif
#if defined(CONFIG_PRAM)
	initr_mem,
#endif
#ifdef CONFIG_PS2KBD
	initr_kbd,
#endif
	run_main_loop,
};

void board_init_r(gd_t *new_gd, ulong dest_addr)
{
	/*
	 * Set up the new global data pointer. So far only x86 does this
	 * here.
	 * TODO(sjg@chromium.org): Consider doing this for all archs, or
	 * dropping the new_gd parameter.
	 */
#if CONFIG_IS_ENABLED(X86_64)
	arch_setup_gd(new_gd);
#endif

#ifdef CONFIG_NEEDS_MANUAL_RELOC
	int i;
#endif

#if !defined(CONFIG_X86) && !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
	gd = new_gd;
#endif
	gd->flags &= ~GD_FLG_LOG_READY;

#ifdef CONFIG_NEEDS_MANUAL_RELOC
	for (i = 0; i < ARRAY_SIZE(init_sequence_r); i++)
		init_sequence_r[i] += gd->reloc_off;
#endif

	if (initcall_run_list(init_sequence_r))
		hang();

	/* NOTREACHED - run_main_loop() does not return */
	hang();
}
