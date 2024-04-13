/*
 * Copyright (c) 2015 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <mapmem.h>
#include <dm/root.h>
#include <dm/util.h>

static void show_devices(struct udevice *dev, int depth, int last_flag)
{
	int i, is_last;
	struct udevice *child;

	/* print the first 11 characters to not break the tree-format. */
	printf(" %08lx    %-10.10s [ %c ]   %-25.25s  ",
	       (ulong)dev, dev->uclass->uc_drv->name,
	       dev->flags & DM_FLAG_ACTIVATED ? '+' : ' ', dev->driver->name);

	for (i = depth; i >= 0; i--) {
		is_last = (last_flag >> i) & 1;
		if (i) {
			if (is_last)
				printf("    ");
			else
				printf("|   ");
		} else {
			if (is_last)
				printf("`-- ");
			else
				printf("|-- ");
		}
	}

#ifdef CONFIG_USING_KERNEL_DTB_V2
	printf("%s %s\n", dev->name, dev->flags & DM_FLAG_KNRL_DTB ? "" : "*");
#else
	int pre_reloc, remained;

	pre_reloc = dev->flags & DM_FLAG_KNRL_DTB ? 0 : 1;
	remained = pre_reloc ? !list_empty(&dev->uclass_node) : 0;
	printf("%s %s%s\n", dev->name, pre_reloc ? "*" : "", remained ? "*" : "");
#endif
	list_for_each_entry(child, &dev->child_head, sibling_node) {
		is_last = list_is_last(&child->sibling_node, &dev->child_head);
		show_devices(child, depth + 1, (last_flag << 1) | is_last);
	}
}

void dm_dump_all(void)
{
	struct udevice *root;

	root = dm_root();
	if (root) {
		printf(" Addr        Class      Probed    Driver                   Name\n");
		printf("-------------------------------------------------------------------------\n");
		show_devices(root, -1, 0);
	}
}

/**
 * dm_display_line() - Display information about a single device
 *
 * Displays a single line of information with an option prefix
 *
 * @dev:	Device to display
 */
static void dm_display_line(struct udevice *dev)
{
	printf("  [ %c ] %s @ %08lx",
	       dev->flags & DM_FLAG_ACTIVATED ? '+' : ' ',
	       dev->name, (ulong)map_to_sysmem(dev));
	if (dev->seq != -1 || dev->req_seq != -1)
		printf(", seq %d, (req %d)", dev->seq, dev->req_seq);
	if (dev->driver->id == UCLASS_BLK) {
		struct blk_desc *desc = dev_get_uclass_platdata(dev);
		printf(" | %s%d", blk_get_if_type_name(desc->if_type), desc->devnum);
	}
	printf(" %c", dev->flags & DM_FLAG_KNRL_DTB ? ' ' : '*');
	puts("\n");
}

void dm_dump_uclass(void)
{
	struct uclass *uc;
	int ret;
	int id;

	for (id = 0; id < UCLASS_COUNT; id++) {
		struct udevice *dev;

		ret = uclass_get(id, &uc);
		if (ret)
			continue;

		printf("uclass %d: %s\n", id, uc->uc_drv->name);
		if (list_empty(&uc->dev_head))
			continue;
		list_for_each_entry(dev, &uc->dev_head, uclass_node) {
			dm_display_line(dev);
		}
		puts("\n");
	}
}
