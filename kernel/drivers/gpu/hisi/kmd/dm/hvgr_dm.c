/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/poll.h>
#include <linux/miscdevice.h>
#include <linux/interrupt.h>
#include <linux/kref.h>
#include <linux/mman.h>
#include <linux/io.h>
#include <linux/rcupdate.h>

#include "hvgr_version.h"
#include "hvgr_gpu_id.h"

#include "hvgr_ioctl.h"
#include "apicmd/hvgr_ioctl_dev.h"
#include "external/hvgr_external_base.h"

#include "hvgr_dm_api.h"
#include "hvgr_mem_api.h"
#include "hvgr_sch_api.h"
#include "hvgr_pm_api.h"
#include "hvgr_log_api.h"
#include "hvgr_debugfs_api.h"
#include "hvgr_datan_api.h"
#include "hvgr_mem_api.h"
#include "hvgr_dm_ctx.h"
#include "hvgr_dm_driver_base.h"
#include "hvgr_platform_api.h"

#include "hvgr_fcp.h"
#include "hvgr_assert.h"
#include "hvgr_msync.h"
#if defined(HVGR_SIMULATE) && HVGR_SIMULATE
#include "hvgr_cq_simulate.h"
#endif

static struct hvgr_device *g_gpu_device;
static uint32_t g_gpu_counter;

#define HVGR_PCIE_REG_VIR_MODE      0x300010
#ifndef HVGR_PCIE_REG_JM_TOP
#define HVGR_PCIE_REG_JM_TOP        0x100000
#endif

#define HVGR_DEFAULT_DEV_MODE       0666u

struct hvgr_device *hvgr_get_device(void)
{
	return g_gpu_device;
}

static int hvgr_open(struct inode *knode, struct file *kfile)
{
	struct hvgr_ctx *ctx = NULL;
	struct hvgr_device *gdev = g_gpu_device;

	if (gdev == NULL) {
		pr_err("%s gdev is NULL", __func__);
		return -ENODEV;
	}

	ctx = hvgr_open_context(gdev, kfile);
	if (ctx == NULL) {
		hvgr_err(gdev, HVGR_DM, "%s open ctx failed.", __func__);
		return -EINVAL;
	}

	kfile->private_data = ctx;
	return 0;
}

#define HVGR_DM_WAIT_IOCTL_MAX_TIMES 6000

void hvgr_ctx_kref_release(struct kref *ctx_kref)
{
	struct hvgr_ctx * const ctx = container_of(ctx_kref, struct hvgr_ctx, ctx_kref);

	hvgr_close_context(ctx);
}

static int hvgr_release(struct inode *knode, struct file *kfile)
{
	struct hvgr_ctx * const ctx = (struct hvgr_ctx * const)kfile->private_data;

	if (ctx == NULL)
		return -EINVAL;

	rcu_assign_pointer(kfile->private_data, NULL);
	hvgr_ctx_flag_set(ctx, HVGR_CTX_FLAG_DYING);
	synchronize_rcu();
	(void)kref_put(&ctx->ctx_kref, hvgr_ctx_kref_release);
	return 0;
}

HVGR_STATIC int HVGR_WEAK hvgr_test_ioctl_cfg(struct hvgr_ctx * const ctx)
{
	return 0;
}

static inline bool hvgr_is_illegal_process(struct hvgr_ctx *ctx)
{
	if (likely(ctx->leader == current->group_leader))
		return false;

	hvgr_err(ctx->gdev, HVGR_DM, "ctx_%u in wrong process, ctx->tgid = %u, current->tgid = %u",
		ctx->id, ctx->tgid, current->tgid);
	return true;
}

static struct hvgr_ctx* hvgr_get_ctx(struct file *kfile)
{
	struct hvgr_ctx *ctx = NULL;

	rcu_read_lock();
	ctx = (struct hvgr_ctx *)rcu_dereference(kfile->private_data);
	if (ctx == NULL) {
		pr_err("[hvgr] get ctx is null\n");
		goto rcu_exit;
	}

	if (hvgr_is_illegal_process(ctx))
		goto rcu_exit;

	kref_get(&ctx->ctx_kref);
	rcu_read_unlock();
	return ctx;

rcu_exit:
	rcu_read_unlock();
	return NULL;
}

static long hvgr_ioctl(struct file *kfile, unsigned int cmd, unsigned long arg)
{
	struct hvgr_ctx * const ctx = hvgr_get_ctx(kfile);
	long ret = -EINVAL;

	if (ctx == NULL)
		return -EINVAL;

	if (hvgr_test_ioctl_cfg(ctx) != 0)
		hvgr_err(ctx->gdev, HVGR_DM, "[test]ctx_%u Driver NO configure.", ctx->id);

	switch (cmd) {
	case HVGR_IOCTL_VERSION:
		hvgr_ioctl_handle_r(ctx, HVGR_IOCTL_VERSION,
			hvgr_ioctl_get_driver_info, arg, hvgr_ioctl_para_version);
		break;
	case HVGR_IOCTL_CONFIGURE:
		hvgr_ioctl_handle_w(ctx, HVGR_IOCTL_CONFIGURE,
			hvgr_ioctl_cfg_driver, arg, hvgr_ioctl_para_drv_cfg);
		break;
	case HVGR_IOCTL_DECONFIGURE:
		hvgr_ioctl_handle_w(ctx, HVGR_IOCTL_DECONFIGURE,
			hvgr_ioctl_decfg_driver, arg, hvgr_ioctl_para_drv_cfg);
		break;
	default:
		break;
	}

	if (!hvgr_ctx_flag(ctx, HVGR_CTX_FLAG_CFGED)) {
		hvgr_err(ctx->gdev, HVGR_DM, "ctx_%u Driver NO configure.", ctx->id);
		ret = -EPERM;
		goto exit;
	}

	switch (_IOC_TYPE(cmd)) {
	case HVGR_IOCTL_MODULE_DATAN:
		ret = hvgr_ioctl_datan(ctx, cmd, arg);
		break;
	case HVGR_IOCTL_MODULE_DEV:
		ret = hvgr_ioctl_dm(ctx, cmd, arg);
		break;
	case HVGR_IOCTL_MODULE_PM:
		ret = -EINVAL;
		break;
	case HVGR_IOCTL_MODULE_MEM:
		ret = hvgr_ioctl_mem(ctx, cmd, arg);
		break;
	case HVGR_IOCTL_MODULE_SCH:
		ret = hvgr_sch_cmd_dispatch(ctx, cmd, arg);
		break;
	default:
		hvgr_err(ctx->gdev, HVGR_DM, "Ioctl cmd 0x%x is error.", cmd);
		ret = -EINVAL;
		break;
	}

exit:
	kref_put(&ctx->ctx_kref, hvgr_ctx_kref_release);
	return ret;
}

static ssize_t hvgr_read(struct file *kfile, char __user *buf, size_t count, loff_t *f_pos)
{
	struct hvgr_ctx * const ctx = hvgr_get_ctx(kfile);
	uint32_t nums;
	ssize_t ret = -EINVAL;

	if (ctx == NULL)
		return -EINVAL;

	nums = hvgr_msync_get_event_read_num(ctx, (uint32_t)count);
	if (unlikely(nums == 0))
		goto fail_exit;

	ret = (ssize_t)hvgr_token_dequeue(ctx, buf, nums);
fail_exit:
	(void)kref_put(&ctx->ctx_kref, hvgr_ctx_kref_release);
	return ret;
}

#if defined(CONFIG_LIBLINUX_CDC)
static struct hvgr_ctx* hvgr_get_ctx_no_check_process(struct file *kfile)
{
	struct hvgr_ctx *ctx = NULL;

	rcu_read_lock();
	ctx = (struct hvgr_ctx *)rcu_dereference(kfile->private_data);
	if (ctx == NULL) {
		pr_err("[hvgr] get ctx is null\n");
		goto rcu_exit;
	}

	kref_get(&ctx->ctx_kref);
	rcu_read_unlock();
	return ctx;

rcu_exit:
	rcu_read_unlock();
	return NULL;
}
#endif

static unsigned int hvgr_poll(struct file *kfile, poll_table *wait)
{
#if defined(CONFIG_LIBLINUX_CDC)
	struct hvgr_ctx * const ctx = hvgr_get_ctx_no_check_process(kfile);
#else
	struct hvgr_ctx * const ctx = hvgr_get_ctx(kfile);
#endif
	bool datan_jobtrace_softq = false;
	uint32_t ret = 0;

	if (ctx == NULL)
		return -EINVAL;

	if (hvgr_msync_event_checkdata(ctx) || (atomic_read(&ctx->softq_closed) != 0)) {
		ret = POLLIN | POLLRDNORM;
		goto fail_exit;
	}

	poll_wait(kfile, &ctx->softq_wait, wait);

	if (hvgr_msync_event_checkdata(ctx) || (atomic_read(&ctx->softq_closed) != 0)) {
		ret = POLLIN | POLLRDNORM;
		goto fail_exit;
	}

	/* check if the softq used by Datan JobTrace is not empty */
	hvgr_datan_jobtrace(ctx, JOBTRACE_TYPE_LOCAL, KDATAN_TL_LOCAL_CHK_SOFTQ,
		&datan_jobtrace_softq);
	if (datan_jobtrace_softq)
		ret = POLLIN | POLLRDNORM;

fail_exit:
	(void)kref_put(&ctx->ctx_kref, hvgr_ctx_kref_release);
	return ret;
}

static int hvgr_mmap(struct file *kfile, struct vm_area_struct *vma)
{
	struct hvgr_ctx * const ctx = hvgr_get_ctx(kfile);
	int ret;

	if (ctx == NULL)
		return -EINVAL;

	ret = hvgr_mem_mmap(ctx, vma);
	(void)kref_put(&ctx->ctx_kref, hvgr_ctx_kref_release);
	return ret;
}

static int hvgr_check_flags(int flags)
{
	if (((uint64_t)flags & O_CLOEXEC) == 0)
		return -EINVAL;

	return 0;
}

unsigned long hvgr_get_unmapped_area(struct file *filp, unsigned long addr, unsigned long len, unsigned long pgoff, 
	unsigned long flags)
{
	/* do not allow fixed address */
	if (flags & MAP_FIXED)
		return -EINVAL;
	return current->mm->get_unmapped_area(filp, addr, len, pgoff, flags);
}

static const struct file_operations hvgr_kmd_fops = {
	.owner = THIS_MODULE,
	.open = hvgr_open,
	.release = hvgr_release,
	.unlocked_ioctl = hvgr_ioctl,
	.compat_ioctl = hvgr_ioctl,
	.read = hvgr_read,
	.poll = hvgr_poll,
	.mmap = hvgr_mmap,
	.check_flags = hvgr_check_flags,
	.get_unmapped_area = hvgr_get_unmapped_area,
};

void hvgr_free_irqs(struct hvgr_device *gdev)
{
	int i;
	struct hvgr_irq *p = NULL;

	for (i = 0; i < GPU_IRQ_CNT; i++) {
		p = &gdev->dm_dev.irqs[i];
		if (p->reg_done == IRQ_REGISTER_DONE) {
			(void)free_irq(p->irq, gdev);
			p->reg_done = 0;
		}
	}
}

void hvgr_synchronize_irqs(struct hvgr_device *gdev)
{
	int i;

	for (i = 0; i < GPU_IRQ_CNT; i++) {
		if (gdev->dm_dev.irqs[i].irq != 0)
			synchronize_irq(gdev->dm_dev.irqs[i].irq);
	}
}

int hvgr_register_irq(struct hvgr_device * const gdev,
	enum hvgr_irqs_type type, irq_handler_t handler)
{
	int val;
	struct hvgr_irq *p = NULL;

	if (gdev == NULL)
		return -ENODEV;
	if ((type >= GPU_IRQ_CNT) || (handler == NULL)) {
		hvgr_err(gdev, HVGR_DM, "register irq failed. type is %d", type);
		goto irq_failed;
	}
	p = &gdev->dm_dev.irqs[type];

	if (p->reg_done == IRQ_REGISTER_DONE) {
		hvgr_err(gdev, HVGR_DM,
			"register irq failed. %s is registered.", dev_name(gdev->dev));
		goto irq_failed;
	}

	val = request_irq(p->irq, handler, p->flags | IRQF_SHARED, dev_name(gdev->dev), gdev);
	if (val) {
		hvgr_err(gdev, HVGR_DM, "interrupt %d request failed\n", p->irq);
		goto irq_failed;
	}
	p->reg_done = IRQ_REGISTER_DONE;
	return 0;

irq_failed:
	hvgr_free_irqs(gdev);
	return -1;
}

#ifdef CONFIG_DFX_DEBUG_FS
static ssize_t kmd_show_gpuinfo(struct device *dev, struct device_attribute *attr, char *buf)
{
	static const struct gpu_product_id_name {
		unsigned int id;
		char *name;
	} gpu_product_id_names[] = {
		{ .id = GPU_ID2_PRODUCT_TV200,
		  .name = "Maleoon 910" },
		{ .id = GPU_ID2_PRODUCT_TV210,
		  .name = "hvgr-tv210" },
		{ .id = GPU_ID2_PRODUCT_TV211,
		  .name = "hvgr-tv211" },
		{ .id = GPU_ID2_PRODUCT_TV300,
		  .name = "hvgr-tv300" },
	};
	const char *product_name = "(Unknown GPU)";
	struct hvgr_device *gdev = dev_get_drvdata(dev);
	u32 gpu_id;
	unsigned int product_id;
	unsigned int i;

	gpu_id = gdev->dm_dev.dev_reg.gpu_id;
	product_id = gpu_id >> GPU_ID_VERSION_PRODUCT_ID_SHIFT;

	for (i = 0; i < ARRAY_SIZE(gpu_product_id_names); ++i) {
		const struct gpu_product_id_name *p = &gpu_product_id_names[i];

		if ((p->id & GPU_ID1_PRODUCT_MODEL) ==
			(gpu_id & GPU_ID1_PRODUCT_MODEL)) {
			product_name = p->name;
			break;
		}
	}

	return scnprintf(buf, PAGE_SIZE, "%s %u cores 0x%04X\n",
			product_name, hweight32(gdev->dm_dev.dev_reg.gpc_present),
			product_id);
}
static DEVICE_ATTR(gpuinfo, S_IRUGO, kmd_show_gpuinfo, NULL);
#endif

int hvgr_dm_init(struct hvgr_device * const gdev)
{
	int ret;

	ret = hvgr_fcp_loader_init(gdev);
	if (ret != 0) {
		hvgr_err(gdev, HVGR_DM, "hvgr_fcp_loader_init failed fcp %d", ret);
		return ret;
	}

	hvgr_get_info_from_reg(gdev);

#ifdef CONFIG_DFX_DEBUG_FS
	if (sysfs_create_file(&gdev->dev->kobj, &dev_attr_gpuinfo.attr))
		hvgr_err(gdev, HVGR_DM, "Create gpuinfo failed.");
#endif

	atomic_set(&gdev->ctx_num, 0);
	mutex_init(&gdev->dm_dev.ctx_list_mutex);
	INIT_LIST_HEAD(&gdev->dm_dev.ctx_list);

	(void)hvgr_platform_tbu_init(gdev);

	return hvgr_register_irq(gdev, GPU_IRQ, hvgr_gpu_irq);
}

void hvgr_dm_term(struct hvgr_device * const gdev)
{
#ifdef CONFIG_DFX_DEBUG_FS
	sysfs_remove_file(&gdev->dev->kobj, &dev_attr_gpuinfo.attr);
#endif
	hvgr_free_irqs(gdev);

#ifdef CONFIG_HVGR_VIRTUAL_GUEST
	if (gdev->dm_dev.ecc_wq != NULL)
		destroy_workqueue(gdev->dm_dev.ecc_wq);
#endif
}

/* Register GPU device to kernel. */
int hvgr_misc_register(struct hvgr_device *gdev)
{
	gdev->misc_dev.name = gdev->devname;
	gdev->misc_dev.minor = MISC_DYNAMIC_MINOR;
	gdev->misc_dev.fops = &hvgr_kmd_fops;
	gdev->misc_dev.parent = get_device(gdev->dev);
#ifndef CONFIG_LIBLINUX_CDC
	gdev->misc_dev.mode = HVGR_DEFAULT_DEV_MODE;
#endif
	return misc_register(&gdev->misc_dev);
}
struct hvgr_callback {
	int (*init_func)(struct hvgr_device * const gdev);
	void (*term_func)(struct hvgr_device * const gdev);
};

struct hvgr_callback hvgr_callback_entry[] = {
	{hvgr_dm_init, hvgr_dm_term},
	{hvgr_pm_init, hvgr_pm_term},
	{hvgr_mem_init, hvgr_mem_term},
	{hvgr_sch_init, hvgr_sch_term},
	{hvgr_datan_init, hvgr_datan_term},
	{hvgr_wq_init, NULL},
	{hvgr_dmd_msg_init, hvgr_dmd_msg_term},
#if defined(HVGR_SIMULATE) && HVGR_SIMULATE
	{hvgr_cq_simu_init, hvgr_cq_simu_term},
#endif
#if hvgr_version_ge(HVGR_V350)
	{hvgr_ctx_common_ctx_init, hvgr_ctx_common_ctx_term},
#endif
};

static int hvgr_module_init(struct hvgr_device *gdev, unsigned int *step)
{
	int ret;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(hvgr_callback_entry); i++) {
		if (hvgr_callback_entry[i].init_func == NULL)
			continue;

		ret = hvgr_callback_entry[i].init_func(gdev);
		if (ret != 0) {
			*step = i;
			hvgr_err(gdev, HVGR_DM, "%s module %d in init failed!", __func__, i);
			return ret;
		}
	}

	*step = i;
	return 0;
}

static void hvgr_module_term(struct hvgr_device *gdev, unsigned int step)
{
	while (step != 0) {
		step--;
		if (hvgr_callback_entry[step].term_func == NULL)
			continue;

		hvgr_callback_entry[step].term_func(gdev);
	}
}

static void hvgr_power_on_gpu(struct hvgr_device *gdev)
{
	if (!hvgr_is_always_on(gdev))
		return;

	/*
	 * When the FPGA/ASIC device is powered on, it is unstable.
	 * If the job is submitted immediately, There will be GPU hang.
	 * Therefore, we need to power on the GPU in the device probe
	 * phase.
	 */
	(void)hvgr_pm_request_gpu(gdev, false);
	/*
	 * After hvgr_pm_request_gpu is executed, user_ref is incremented
	 * by one. We need to call hvgr_pm_release_gpu to reduce
	 * user_ref by one.
	 * The FPGA/ASIC is in always_on mode. Therefore, when
	 * hvgr_pm_release_gpu is executed, only user_ref is
	 * decreased by one and the GPU is not powered off.
	 */
	hvgr_pm_release_gpu(gdev);
}

static void hvgr_change_mode(struct hvgr_device * const gdev)
{
	u32 val = 0;
	void __iomem *pcie_addr = gdev->dm_dev.dev_reg.reg_base - HVGR_PCIE_REG_JM_TOP;

	if (!hvgr_hw_has_issue(gdev, HVGR_HW_ISSUE_50210) ||
		gdev->device_type == HVGR_DEVICE_TYPE_SOC)
		return;

	writel(val, pcie_addr + HVGR_PCIE_REG_VIR_MODE);

	dev_info(gdev->dev, "send change vir mode command, val = 0x%x\n", val);
}

int hvgr_device_probe(struct hvgr_device *gdev)
{
	int ret = 0;
	unsigned int step = 0;

	g_gpu_device = gdev;

	(void)scnprintf(gdev->devname, HVGR_DEV_NAME_NUM, "%s%d", HVGR_DIRVER_NAME, g_gpu_counter);
	/* debugfs & log module init frist */
	(void)hvgr_debugfs_init(gdev);
	hvgr_log_init(gdev);

	/* get external operation object */
	gdev->subsys_ops = hvgr_subsys_ops_get();
	if (unlikely(gdev->subsys_ops == NULL)) {
		hvgr_err(gdev, HVGR_DM, "%s get external operation object failed.", __func__);
		goto probe_fail;
	}

	if (unlikely(hvgr_platform_smmu_set_sid(gdev) != 0)) {
		hvgr_err(gdev, HVGR_DM, "%s hvgr platform smmu set sid failed", __func__);
		goto probe_fail;
	}

	/* GPU Buck on */
	ret = hvgr_pm_gpu_access_enable(gdev);
	if (ret != 0) {
		hvgr_err(gdev, HVGR_DM, "%s hvgr_pm_gpu_access_enable failed %d", __func__, ret);
		goto buck_on_fail;
	}

	hvgr_change_mode(gdev);

	/* All module init. */
	ret = hvgr_module_init(gdev, &step);
	if (ret != 0)
		goto probe_fail;

	/* Register GPU device to kernel. */
	ret = hvgr_misc_register(gdev);
	if (ret != 0) {
		hvgr_err(gdev, HVGR_DM, "%s hvgr_misc_register failed %d", __func__, ret);
		goto probe_fail;
	}

	/* GPU Buck off */
	ret = hvgr_pm_gpu_access_disable(gdev);
	if (ret != 0) {
		hvgr_err(gdev, HVGR_DM, "%s hvgr_pm_gpu_access_disable failed %d", __func__, ret);
		goto buck_off_fail;
	}

	g_gpu_counter++;
	hvgr_power_on_gpu(gdev);
	return 0;

buck_off_fail:
	misc_deregister(&gdev->misc_dev);
probe_fail:
	hvgr_fcp_loader_term(gdev);
	hvgr_module_term(gdev, step);
	hvgr_pm_gpu_access_disable(gdev);
	hvgr_pm_gpu_access_term(gdev);
buck_on_fail:
	hvgr_debugfs_term(gdev);
	g_gpu_device = NULL;
	return -1;
}

int hvgr_device_remove(struct hvgr_device *gdev)
{
	hvgr_fcp_loader_term(gdev);
	misc_deregister(&gdev->misc_dev);
	hvgr_module_term(gdev, ARRAY_SIZE(hvgr_callback_entry));
	hvgr_pm_gpu_access_term(gdev);
	hvgr_debugfs_term(gdev);

	g_gpu_device = NULL;
	return 0;
}

