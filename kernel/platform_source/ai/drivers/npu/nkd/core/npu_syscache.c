/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2021. All rights reserved.
 * Description: syscache api
 * Create: 2021-07-06
 */
#include "npu_syscache.h"
#include <linux/dma-buf.h>
#include <linux/mm_lb/mm_lb.h>
#include <linux/timer.h>
#include <linux/workqueue.h>

#include "npu_common.h"
#include "npu_log.h"
#include "npd_interface.h"

enum npu_syscache_state {
	NPU_SYSCACHE_DISABLE = 0,
	NPU_SYSCACHE_ENABLE,
	NPU_SYSCACHE_ENABLE_AND_HIGH_PRIORITY,
};

static u8 g_syscache_state = NPU_SYSCACHE_DISABLE;

int npu_ioctl_attach_syscache(struct npu_proc_ctx *proc_ctx,
	unsigned long arg)
{
	int ret = 0;
	struct npu_attach_sc msg = {0};
	struct npd_registry *npd = get_npd();
#ifdef CONFIG_MM_LB
	unsigned int pid = PGID_NPU;
#endif
	unused(proc_ctx);

	cond_return_error(npd == NULL, -EINVAL, "npd ptr is NULL\n");
	ret = copy_from_user_safe(&msg, (void __user *)(uintptr_t)arg, sizeof(msg));
	cond_return_error(ret != 0, -EINVAL,
		"fail to copy npu sc attach params, ret = %d\n", ret);

	msg.size = (msg.size < get_sc_size()) ? msg.size : get_sc_size();

	if (!npd->npd_query_sku_value(NPU_SKU_VAR_SUPPORT_SC)) {
		npu_drv_debug("do not support sc\n");
		return 0;
	}

	/* syscahce attach interface with offset */
	npu_drv_info("attach sc fd = 0x%x, offset = 0x%lx, size = 0x%lx\n",
		msg.fd, msg.offset, msg.size);
#ifdef CONFIG_MM_LB
#ifndef CONFIG_MM_LB_V550
	pid = PID_NPU;
#endif
	ret = dma_buf_attach_lb(msg.fd, pid, msg.offset, (size_t)msg.size);
	cond_return_error(ret != 0, -EINVAL,
		"dma buf attach lb fail, ret = %d\n", ret);
#endif

	return 0;
}

int npu_ioctl_detach_syscache(struct npu_proc_ctx *proc_ctx, unsigned long arg)
{
	int ret = 0;
	int fd = 0;
	struct npd_registry *npd = get_npd();
	unused(proc_ctx);

	cond_return_error(npd == NULL, -EINVAL, "npd ptr is NULL\n");
	ret = copy_from_user_safe(&fd, (void __user *)(uintptr_t)arg, sizeof(fd));
	cond_return_error(ret != 0, -EINVAL,
		"fail to copy npu sc detach fd, ret = %d\n", ret);

	if (!npd->npd_query_sku_value(NPU_SKU_VAR_SUPPORT_SC)) {
		npu_drv_debug("do not support sc\n");
		return 0;
	}

	/* syscahce detach interface */
	npu_drv_info("fd = 0x%x", fd);
#ifdef CONFIG_MM_LB
	ret = dma_buf_detach_lb(fd);
	cond_return_error(ret != 0, -EINVAL,
		"dma buf detach lb fail, ret = %d\n", ret);
#endif

	return 0;
}

void npu_sc_timeout(struct timer_list *syscache_timer)
{
#ifdef CONFIG_MM_LB
	unsigned int pid = PID_NPU;
#ifdef CONFIG_MM_LB_V550
	pid = PGID_NPU;
#endif
	int ret = lb_gid_bypass(pid);
	g_syscache_state = NPU_SYSCACHE_DISABLE;
	if (ret != 0)
		npu_drv_warn("npu_sc_timeout %d\n", ret);
#endif
	unused(syscache_timer);
}

void npu_sc_enable_and_high(struct npu_dev_ctx *dev_ctx)
{
	int ret = 0;
	struct npd_registry *npd = get_npd();
	cond_return_void(npd == NULL, "npd ptr is NULL\n");
	if (!npd->npd_query_sku_value(NPU_SKU_VAR_SUPPORT_SC)) {
		npu_drv_debug("do not support sc\n");
		return;
	}

#ifdef CONFIG_MM_LB
	mutex_lock(&dev_ctx->syscache_mutex);
	del_timer_sync(&dev_ctx->syscache_timer);
	if (g_syscache_state == NPU_SYSCACHE_ENABLE_AND_HIGH_PRIORITY) {
		goto sc_enable_finish;
	} else if (g_syscache_state == NPU_SYSCACHE_ENABLE) {
		goto sc_priority_up;
	}

	npu_drv_debug("npu_sc_enable_and_high\n");
	ret = lb_gid_enable(PID_NPU);
	cond_goto_error(ret != 0, sc_enable_err, ret,
		ret, "lb gid enable fail, ret = %d\n", ret);
	g_syscache_state = NPU_SYSCACHE_ENABLE;
sc_priority_up:
	ret = lb_up_policy_prio(PID_NPU);
	cond_goto_error(ret != 0, sc_up_policy_err, ret,
		ret, "lb up policy prio fail, ret = %d\n", ret);
	g_syscache_state = NPU_SYSCACHE_ENABLE_AND_HIGH_PRIORITY;
	goto sc_enable_finish;

sc_up_policy_err:
	lb_gid_bypass(PID_NPU);
	g_syscache_state--;
sc_enable_err:
sc_enable_finish:
	mutex_unlock(&dev_ctx->syscache_mutex);
#endif
}

void npu_sc_enable(struct npu_dev_ctx *dev_ctx)
{
	int ret = 0;

	struct npd_registry *npd = get_npd();
	cond_return_void(npd == NULL, "npd ptr is NULL\n");
	if (!npd->npd_query_sku_value(NPU_SKU_VAR_SUPPORT_SC)) {
		npu_drv_debug("do not support sc\n");
		return;
	}

#ifdef CONFIG_MM_LB_V550
	mutex_lock(&dev_ctx->syscache_mutex);
	del_timer_sync(&dev_ctx->syscache_timer);
	if (g_syscache_state == NPU_SYSCACHE_ENABLE) {
		goto sc_enable_finish;
	}

	npu_drv_debug("npu_sc_enable begin\n");
	ret = lb_gid_enable(PGID_NPU);
	cond_goto_error(ret != 0, sc_enable_err, ret,
		ret, "lb gid enable fail, ret = %d\n", ret);
	g_syscache_state = NPU_SYSCACHE_ENABLE;

sc_enable_err:
sc_enable_finish:
	mutex_unlock(&dev_ctx->syscache_mutex);
#else
	unused(ret);
	unused(dev_ctx);
#endif
}

void npu_sc_low(struct npu_dev_ctx *dev_ctx)
{
	int ret = 0;
	struct npd_registry *npd = get_npd();

	cond_return_void(npd == NULL, "npd ptr is NULL\n");
	if (!npd->npd_query_sku_value(NPU_SKU_VAR_SUPPORT_SC)) {
		npu_drv_debug("do not support sc\n");
		return;
	}
#ifdef CONFIG_MM_LB
	mutex_lock(&dev_ctx->syscache_mutex);
#ifndef CONFIG_MM_LB_V550
	if (g_syscache_state != NPU_SYSCACHE_ENABLE_AND_HIGH_PRIORITY)
		goto sc_timer_enable;

	npu_drv_debug("npu_sc_low\n");
	/*  */
	ret = lb_down_policy_prio(PID_NPU);
	g_syscache_state--;
	if (ret != 0)
		npu_drv_warn("lb down policy prio fail, ret = %d\n", ret);
#endif
sc_timer_enable:
	if (g_syscache_state == NPU_SYSCACHE_ENABLE)
		mod_timer(&dev_ctx->syscache_timer, jiffies + 5 * HZ);

	mutex_unlock(&dev_ctx->syscache_mutex);
#endif
}

void npu_sc_disable(struct npu_dev_ctx *dev_ctx)
{
	int ret = 0;
	struct npd_registry *npd = get_npd();
#ifdef CONFIG_MM_LB
	unsigned int pid = PGID_NPU;
#endif

	cond_return_void(npd == NULL, "npd ptr is NULL\n");
	if (!npd->npd_query_sku_value(NPU_SKU_VAR_SUPPORT_SC)) {
		npu_drv_debug("do not support sc\n");
		return;
	}

#ifdef CONFIG_MM_LB
	mutex_lock(&dev_ctx->syscache_mutex);
	npu_drv_debug("npu_sc_low\n");
	del_timer_sync(&dev_ctx->syscache_timer);
#ifndef CONFIG_MM_LB_V550
	pid = PID_NPU;
	ret = lb_down_policy_prio(PID_NPU);
	if (ret != 0)
		npu_drv_warn("lb down policy prio fail, ret = %d\n", ret);
#endif
	ret = lb_gid_bypass(pid);
	g_syscache_state = NPU_SYSCACHE_DISABLE;
	if (ret != 0)
		npu_drv_warn("lb gid bypass fail, ret = %d\n", ret);

	npu_drv_debug("npu_sc_disable\n");
	mutex_unlock(&dev_ctx->syscache_mutex);
#endif
}

