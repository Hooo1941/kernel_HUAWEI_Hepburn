/*
 * npu_manager.c
 *
 * about npu manager
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
#include "npu_manager.h"

#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/swap.h>
#include <linux/spinlock.h>
#include <linux/device.h>
#include <linux/pm_wakeup.h>
#include <linux/atomic.h>

#include "npu_proc_ctx.h"
#include "npu_manager_ioctl_services.h"
#include "npu_ioctl_services.h"
#include "npu_calc_channel.h"
#include "npu_calc_cq.h"
#include "npu_stream.h"
#include "npu_shm.h"
#include "npu_common.h"
#include "npu_devinit.h"
#include "npu_log.h"
#include "npu_pm_framework.h"
#include "npu_recycle.h"
#include "bbox/npu_dfx_black_box.h"
#include "npu_mailbox_msg.h"
#include "npu_hwts.h"
#include "npu_svm.h"
#include "npu_reg.h"
#include "npu_ts_report.h"
#include "npd_interface.h"

static dev_t g_npu_manager_major;
static struct class *g_npu_manager_class;
static struct npu_manager_info *g_dev_manager_info;


static int (*npu_feature_call[NPU_MAX_FEATURE_ID])(
	struct npu_proc_ctx *proc_ctx, uint32_t flag) = { NULL };

#ifdef CONFIG_HUAWEI_DSM
static struct dsm_dev dev_npu = {
	.name = "dsm_ai",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = 1024,
};

static struct dsm_client *npu_dsm_client = NULL;
struct dsm_client * npu_get_dsm_client(void)
{
	return npu_dsm_client;
}
#endif

static int npu_manager_open(struct inode *inode, struct file *filep)
{
	int ret;
	uint32_t pid;
	unused(inode);

	npu_drv_debug("start\n");
	ret = npu_insert_item_bypid(0, current->tgid, current->tgid);
	if (ret != 0) {
		npu_drv_err("npu_insert_item_bypid failed\n");
		return ret;
	}
	pid = (uint32_t)current->tgid;
	filep->private_data = (void *)(uintptr_t)pid;
	return 0;
}

static int npu_manager_release(struct inode *inode, struct file *filep)
{
	int ret;
	pid_t pid;
	struct npu_dev_ctx *cur_dev_ctx = NULL;
	struct npu_proc_ctx *proc_ctx = NULL;
	struct npu_proc_ctx *next_proc_ctx = NULL;
	unused(inode);

	pid = (pid_t)(uintptr_t)filep->private_data;
	npu_drv_debug("start\n");

	cur_dev_ctx = get_dev_ctx_by_id(0);
	if (cur_dev_ctx == NULL) {
		npu_drv_err("cur_dev_ctx is null\n");
		return -1;
	}
	mutex_lock(&cur_dev_ctx->pm.npu_power_mutex);
	if (cur_dev_ctx->pm.power_stage == NPU_PM_UP) {
		// below is for multi-process scenario:
		// if one user process exit when still has tasks(resource leak happens),
		// delay svm unbind to powerdown, so that no npu exception is raised
		list_for_each_entry_safe(proc_ctx, next_proc_ctx,
			&cur_dev_ctx->proc_ctx_list, dev_ctx_list) {
			if ((proc_ctx->pid == pid) &&
				(atomic_read(&proc_ctx->wm_set) != 0)) {
				proc_ctx->manager_release = 1;
				mutex_unlock(&cur_dev_ctx->pm.npu_power_mutex);
				npu_drv_warn("the svm item(pid=%d) will be released in npu power down, maybe in npu_release\n", pid);
				return 0;
			}
		}
	}
	mutex_unlock(&cur_dev_ctx->pm.npu_power_mutex);
	ret = npu_release_item_bypid(0, pid, pid);
	if (ret != 0) {
		npu_drv_err("npu_release_item_bypid failed\n");
		return ret;
	}

	return 0;
}

struct npu_manager_info *npu_get_manager_info(void)
{
	return g_dev_manager_info; /* dev_manager_info */
}

const struct file_operations npu_manager_fops = {
	.owner = THIS_MODULE,
	.open = npu_manager_open,
	.release = npu_manager_release,
	.unlocked_ioctl = npu_manager_ioctl,
	.compat_ioctl = npu_manager_ioctl,
};

static char *npu_manager_devnode(struct device *dev, umode_t *mode)
{
	unused(dev);
	if (mode != NULL)
		*mode = 0666;
	return NULL;
}

static int npu_register_manager_chrdev(void)
{
	struct device *i_device = NULL;
	dev_t devno;
	int ret;

	ret = alloc_chrdev_region(&g_npu_manager_major, 0, NPU_MAX_DEVICE_NUM,
		"npu_manager");
	if (ret != 0) {
		npu_drv_err("npu manager alloc_chrdev_region error\n");
		return -1;
	}

	g_dev_manager_info->cdev.owner = THIS_MODULE;
	devno = MKDEV(MAJOR(g_npu_manager_major), 0);

	g_npu_manager_class = class_create(THIS_MODULE, "npu_manager");
	if (IS_ERR(g_npu_manager_class)) {
		npu_drv_err("npu_manager_class create error\n");
		ret = -EINVAL;
		goto class_fail;
	}

	g_npu_manager_class->devnode = npu_manager_devnode;

	cdev_init(&g_dev_manager_info->cdev, &npu_manager_fops);
	ret = cdev_add(&g_dev_manager_info->cdev, devno, NPU_MAX_DEVICE_NUM);
	if (ret != 0) {
		npu_drv_err("npu manager cdev_add error\n");
		goto cdev_setup_fail;
	}

	i_device = device_create(g_npu_manager_class, NULL, devno, NULL, "%s",
		"npu_manager");
	if (i_device == NULL) {
		npu_drv_err("npu manager device_create error\n");
		ret = -ENODEV;
		goto device_create_fail;
	}

	return ret;
device_create_fail:
	cdev_del(&g_dev_manager_info->cdev);
cdev_setup_fail:
	class_destroy(g_npu_manager_class);
class_fail:
	unregister_chrdev_region(g_npu_manager_major, NPU_MAX_DEVICE_NUM);
	return ret;
}

static void npu_unregister_manager_chrdev(void)
{
	dev_t devno = MKDEV(MAJOR(g_npu_manager_major), 0);
	device_destroy(g_npu_manager_class, devno);
	cdev_del(&g_dev_manager_info->cdev);
	class_destroy(g_npu_manager_class);
	g_npu_manager_class = NULL;
	unregister_chrdev_region(g_npu_manager_major, NPU_MAX_DEVICE_NUM);
}

int npu_feature_enable(struct npu_proc_ctx *proc_ctx, uint32_t feature_id,
	uint32_t enable)
{
	int ret = 0;

	if (feature_id >= NPU_MAX_FEATURE_ID) {
		npu_drv_err("feature id %u is invalid\n", feature_id);
		return -EINVAL;
	}
	if (npu_feature_call[feature_id] == NULL) {
		npu_drv_err("npu feature call invalid feature id = %d\n", feature_id);
		return -EINVAL;
	}
	ret = npu_feature_call[feature_id](proc_ctx, enable);
	if (ret != 0) {
		npu_drv_err("feature id = %u, enable = %u failed\n", feature_id,
			enable);
		return -EINVAL;
	}

	return ret;
}

static void npu_feature_call_init(void)
{
	int i;

	for (i = 0; i < NPU_MAX_FEATURE_ID; i++)
		npu_feature_call[i] = NULL;

	npu_feature_call[NPU_INTERFRAME_FEATURE_ID] = npu_enable_interframe_ilde;
}

int npu_manager_init(void)
{
	int ret;
	const u8 dev_id = 0;
	struct npu_platform_info *plat_info = NULL;

	npu_drv_warn("npu dev %d drv manager init start\n", dev_id);
	plat_info = npu_plat_get_info();
	cond_return_error(plat_info == NULL, -ENODEV,
		"get plat info failed\n");

#if (!defined CONFIG_NPU_SWTS) && (!defined COMPILE_ST)
	/* bbox black box init */
	ret = npu_black_box_init();
	cond_return_error(ret != 0, ret,
		"Failed npu black box init! ret = %d\n", ret);
#endif

	ret = npu_bus_errprobe_register();
	if (ret != 0)
		npu_drv_err("Failed npu bus errprobe register ! ret = %d\n", ret);

	g_dev_manager_info = kzalloc(sizeof(struct npu_manager_info),
		GFP_KERNEL);
	cond_goto_error(g_dev_manager_info == NULL, alloc_manager_info_failed, ret, -ENOMEM,
		"kzalloc npu dev manager info failed\n");

	g_dev_manager_info->plat_info = NPU_MANAGER_DEVICE_ENV;
	ret = npu_register_manager_chrdev();
	cond_goto_error(ret != 0, register_manager_chrdev_failed, ret, -ENODEV,
		"register npu manager chrdev failed ret = %d\n", ret);

	npu_feature_call_init();

#ifdef CONFIG_HUAWEI_DSM
	if (npu_dsm_client == NULL) {
		npu_dsm_client = dsm_register_client(&dev_npu);
		if (npu_dsm_client == NULL)
			npu_drv_err("dsm register client register fail\n");
	}
#endif

	// npu device resoure init
	ret = npu_devinit(THIS_MODULE, dev_id);
	cond_goto_error(ret != 0, npu_devinit_failed, ret, -ENODEV,
		"npu dev %d npu devinit failed\n", dev_id);

#ifndef CONFIG_NPU_SWTS
	ret = npu_request_cq_report_irq_bh();
	cond_goto_error(ret != 0, npu_devinit_failed, ret, ret,
		"npu request cq report irq bh failed\n");
#endif

	/* init AO power flag register which used for sec power sync */
	npu_plat_set_npu_power_status(0);
	npu_drv_warn("success\n");

	return ret;
npu_devinit_failed:
	npu_unregister_manager_chrdev();
register_manager_chrdev_failed:
	kfree(g_dev_manager_info);
	g_dev_manager_info = NULL;
alloc_manager_info_failed:
#if (!defined CONFIG_NPU_SWTS) && (!defined COMPILE_ST)
	(void)npu_black_box_exit();
#endif
	npu_drv_err("failed\n");
	return ret;
}

void npu_manager_exit(void)
{
	const u8 dev_id = 0; // default npu 0 on lite plat

#ifdef CONFIG_HUAWEI_DSM
	if (npu_dsm_client != NULL) {
		dsm_unregister_client(npu_dsm_client, &dev_npu);
		npu_dsm_client = NULL;
	}
#endif
#ifndef CONFIG_NPU_SWTS
	// free irq and kill tasklet
	(void)npu_free_cq_report_irq_bh();
#endif
	// dev resource unregister
	(void)npu_devexit(dev_id);
	// unregister npu manager char dev
	npu_unregister_manager_chrdev();
	// free mem
	if (g_dev_manager_info != NULL) {
		kfree(g_dev_manager_info);
		g_dev_manager_info = NULL;
	}

#if (!defined CONFIG_NPU_SWTS) && (!defined COMPILE_ST)
	/* bbox black box uninit */
	(void)npu_black_box_exit();
#endif
}
