/*
 * dynbrd.c
 *
 * Dynamic brd creation interface
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/errno.h>
#include <linux/uaccess.h>
#ifdef CONFIG_BOOT_DETECTOR_HISI
#include<linux/brd.h>
#endif
#include <hwbootfail/chipsets/common/bootfail_common.h>

#include "brd_import.h"

#ifdef CONFIG_BOOT_DETECTOR_HISI
static struct list_head *brd_devices_ptr = NULL;
static struct mutex *brd_devices_mutex_ptr = NULL;

static int brd_get_avail_id(void)
{
	struct brd_device *brd = NULL;
	int minor_limit = 1UL << MINORBITS;
	unsigned int occupied;
	int i;

	for (i = 0; i < minor_limit; ++i) {
		occupied = 0;

		list_for_each_entry(brd, brd_devices_ptr, brd_list) {
			if (i == brd->brd_number) {
				occupied = 1;
				break;
			}
		}

		if (occupied == 0)
			return i;
	}

	return -1;
}

int brd_create(unsigned int size)
{
	struct brd_device *brd = NULL;
	int id;
	int brd_number = 0;

	if (!brd_devices_ptr)
		brd_devices_ptr = get_brd_devices();

	if (!brd_devices_mutex_ptr)
		brd_devices_mutex_ptr = get_brd_devices_mutex();

	mutex_lock(brd_devices_mutex_ptr);

	id = brd_get_avail_id();
	if (id == (1UL << MINORBITS)) {
		mutex_unlock(brd_devices_mutex_ptr);
		return -1;
	}

	brd = brd_alloc(id);
	if (!brd) {
		mutex_unlock(brd_devices_mutex_ptr);
		return -1;
	}

	set_capacity(brd->brd_disk, size * 2);

	list_add_tail(&brd->brd_list, brd_devices_ptr);
	brd->brd_disk->queue = brd->brd_queue;
	add_disk(brd->brd_disk);
	brd_number = brd->brd_number;
	mutex_unlock(brd_devices_mutex_ptr);

	return brd_number;
}

int brd_delete(int nr)
{
	struct brd_device *brd = NULL;
	struct brd_device *next = NULL;
	int rd_nr = get_rd_nr();
	if (nr < rd_nr)
		return -1;

	if (!brd_devices_ptr)
		brd_devices_ptr = get_brd_devices();

	if (!brd_devices_mutex_ptr)
		brd_devices_mutex_ptr = get_brd_devices_mutex();

	mutex_lock(brd_devices_mutex_ptr);

	list_for_each_entry_safe(brd, next, brd_devices_ptr, brd_list) {
		if (nr == brd->brd_number) {
			dynamic_brd_del_one(brd);
			mutex_unlock(brd_devices_mutex_ptr);
			return 0;
		}
	}

	mutex_unlock(brd_devices_mutex_ptr);

	return -EINVAL;
}
#endif

int create_dynamic_ramdisk(struct dbrd_ioctl_block *arg_u)
{
	struct dbrd_ioctl_block ioc;
	int rc = -EFAULT;

	if (copy_from_user(&ioc, arg_u, sizeof(ioc)))
		return rc;

	ioc.number = brd_create(ioc.kbytes);
	if (ioc.number == -1)
		return -EINVAL;

	if (copy_to_user(arg_u, &ioc, sizeof(ioc)))
		return rc;

	return 0;
}

int delete_dynamic_ramdisk(struct dbrd_ioctl_block *arg_u)
{
	struct dbrd_ioctl_block ioc;
	int rc = -EFAULT;

	if (copy_from_user(&ioc, arg_u, sizeof(struct dbrd_ioctl_block)))
		return rc;

	return brd_delete(ioc.number);
}
