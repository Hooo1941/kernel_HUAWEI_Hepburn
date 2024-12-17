/*
 * haptic_misc.c
 *
 * code for vibrator
 *
 * Copyright (c) 2023 Huawei Technologies Co., Ltd.
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

#include <linux/miscdevice.h>
#include <linux/module.h>

#include "haptic_core.h"
#include "haptic_richtap.h"
#include "vib_log.h"

#ifdef AAC_RICHTAP_SUPPORT
#include <linux/mman.h>
#endif

static ssize_t haptic_read(struct file *filp, char *buff, size_t len,
			   loff_t *offset)
{
	struct haptic *haptic = NULL;

	if (!filp || !buff)
		return -EINVAL;

	haptic = (struct haptic *)filp->private_data;
	if (!haptic)
		return -EFAULT;

	vib_warn("reserved for further use");
	return len;
}

static ssize_t haptic_write(struct file *filp, const char *buff, size_t len,
			    loff_t *off)
{
	struct haptic *haptic = NULL;
	char write_buf[MAX_WRITE_BUF_LEN] = { 0 };
	uint64_t type = 0;
	int ret;

	if (!buff || !filp || (len > (MAX_WRITE_BUF_LEN - 1)))
		return -EINVAL;

	haptic = (struct haptic *)filp->private_data;
	if (!haptic)
		return -EFAULT;

	if (copy_from_user(write_buf, buff, len)) {
		vib_err("[haptics_write] copy_from_user failed");
		return -EFAULT;
	}
	if (kstrtoull(write_buf, 10, &type)) {
		vib_err("[haptics_write] read value error");
		return -EINVAL;
	}
	vib_info("get haptic id = %llu", type);

	ret = haptic_play_effect_type(haptic, type);
	if (ret) {
		vib_err("haptic effect:%llu failed", type);
		return ret;
	}

	return len;
}

static struct haptic *g_haptic_misc;
static int haptic_open(struct inode *inode, struct file *file)
{
	if (!file)
		return -EINVAL;

	if (!try_module_get(THIS_MODULE))
		return -ENODEV;

	file->private_data = (void *)g_haptic_misc;

	return 0;
}

static int haptic_release(struct inode *inode, struct file *file)
{
	if (!file)
		return -EINVAL;

	g_haptic_misc = NULL;
	module_put(THIS_MODULE);
	return 0;
}

static const struct file_operations haptic_fops = {
	.owner = THIS_MODULE,
	.read = haptic_read,
	.write = haptic_write,
#ifdef AAC_RICHTAP_SUPPORT
	.unlocked_ioctl = richtap_file_unlocked_ioctl,
	.mmap = richtap_file_mmap,
#endif
	.open = haptic_open,
	.release = haptic_release,
};

static struct miscdevice haptic_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "haptics",
	.fops = &haptic_fops,
};

int haptic_register_misc_dev(struct haptic *haptic)
{
	int ret;

	if (!haptic)
		return -EINVAL;

	ret = misc_register(&haptic_misc);
	if (ret) {
		vib_err("misc fail");
		return ret;
	}

	g_haptic_misc = haptic;
	return 0;
}

void haptic_unregister_misc_dev(void)
{
	misc_deregister(&haptic_misc);
}