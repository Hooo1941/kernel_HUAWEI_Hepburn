/*
 * npu_platform.c
 *
 * about npu platform
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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
#include "npu_platform.h"
#include <linux/irq.h>
#include <linux/irqchip/arm-gic-v3.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/cpu.h>
#include <linux/kthread.h>
#include <linux/dma-mapping.h>
#include <linux/suspend.h>
#include <linux/vmalloc.h>
#include <securec.h>

#include "npu_log.h"
#include "npu_user_common.h"
#include "npu_platform_register.h"
#include "npu_reg.h"
#include "npu_dfx.h"
#include "npu_feature.h"
#include "npu_irq.h"
#ifdef CONFIG_NPU_BBIT_DEBUGFS
#include "npu_bbit_debugfs.h"
#endif
#include "npu_iova_dev.h"
#include "npu_intr_hub_intc.h"
#include "npu_manager.h"
#ifndef CONFIG_NPU_SWTS
#include "npu_dfx_log.h"
#endif
#include "npu_dfx_profiling.h"
#include "npu_dpm.h"
#include "npd_interface.h"
#include "npu_atf_subsys.h"
#include "npu_counter.h"
#ifdef CONFIG_HIBERNATION
#include "npu_load_ts_img.h"
#include "npu_pm_framework.h"
#endif

static struct npu_platform_info *g_platform_info; //  __ro_after_init;
struct npd_registry* g_npd; // NPD driver registry data

struct npd_registry *get_npd(void)
{
	return g_npd;
}
EXPORT_SYMBOL(get_npd);

static int npu_power_up(struct npu_dev_ctx *dev_ctx, u32 work_mode)
{
	if(g_npd != NULL && g_npd->npd_notify)
		return g_npd->npd_notify(dev_ctx, NPD_NOTICE_POWER_UP, work_mode, 0);
	return 0;
}
static int npu_power_down(struct npu_dev_ctx *dev_ctx, u32 work_mode)
{
	if(g_npd != NULL && g_npd->npd_notify)
		return g_npd->npd_notify(dev_ctx, NPD_NOTICE_POWER_DOWN, work_mode, 0);
	return 0;
}

static int nkd_plat_send_ts_ctrl_core(uint32_t core_num)
{
	if(g_npd != NULL && g_npd->npd_notify != NULL)
		return g_npd->npd_notify(NULL, NPD_NOTICE_INFORM_TS_LIMIT_AICORE, (long)core_num, 0);
	return -1;
}

static int npu_plat_init_adapter(struct npu_platform_info *plat_info)
{
	plat_info->adapter.pm_ops.npu_power_up = npu_power_up;
	plat_info->adapter.pm_ops.npu_power_down = npu_power_down;
	plat_info->adapter.res_ops.npu_irq_change_route = NULL;
	plat_info->adapter.res_ops.npu_cqsq_share_alloc = NULL;
	plat_info->adapter.res_ops.npu_trigger_irq = NULL;
#ifndef CONFIG_NPU_SWTS
	plat_info->adapter.res_ops.npu_mailbox_send = g_npd->npd_send_mail;
#endif
	if (plat_info->dts_info.feature_switch[NPU_FEATURE_CTRL_CORE] == 1)
		plat_info->adapter.res_ops.npu_ctrl_core = nkd_plat_send_ts_ctrl_core;
	else
		plat_info->adapter.res_ops.npu_ctrl_core = NULL;
	return 0;
}

static int npu_plat_parse_dtsi(struct platform_device *pdev,
	struct npu_platform_info *plat_info)
{
	struct device_node *root = NULL;
	struct device_node *module_np = NULL;
	int ret;

	root = pdev->dev.of_node;

	module_np = of_get_child_by_name(root, "npu0");
	ret = npu_plat_parse_feature_switch(module_np, plat_info);
	cond_return_error(ret != 0, ret,
		"parse platform feature switch failed\n");

	ret = npu_plat_parse_reg_desc(pdev, plat_info);
	cond_return_error(ret != 0, ret, "parse platform reg desc failed\n");
	ret = npu_plat_map_reg(pdev, plat_info);
	cond_return_error(ret != 0, ret, "map platform reg failed\n");

	ret = npu_plat_parse_irq(pdev, plat_info);
	cond_goto_error(ret != 0, fail_after_map_reg, ret, ret, "parse platform irq failed\n");

	ret = npu_plat_parse_dump_region_desc(root, plat_info);
	if (ret != 0)
		npu_drv_warn("current platform does not support dump regs\n");

	ret = g_npd->npd_set_resmem(plat_info);
	cond_goto_error(ret != 0, fail_after_map_reg, ret, ret, "set platform resmem failed\n");

	module_np = of_get_child_by_name(root, "log");
	ret = npu_plat_parse_dfx_desc(module_np, plat_info,
		&plat_info->resmem_info.dfx_desc[NPU_DFX_DEV_LOG],
		NPU_LOG_MEM_IDX);
	cond_goto_error(ret != 0, fail_after_map_reg, ret, ret,
		"parse platform dfx desc for log failed\n");

	module_np = of_get_child_by_name(root, "profile");
	ret = npu_plat_parse_dfx_desc(module_np, plat_info,
		&plat_info->resmem_info.dfx_desc[NPU_DFX_DEV_PROFILE],
		NPU_PROF_MEM_IDX);
	cond_goto_error(ret != 0, fail_after_map_reg, ret, ret,
		"parse platform dfx desc for profile failed\n");

	module_np = of_get_child_by_name(root, "blackbox");
	ret = npu_plat_parse_dfx_desc(module_np, plat_info,
		&plat_info->resmem_info.dfx_desc[NPU_DFX_DEV_BLACKBOX],
		NPU_BBOX_MEM_IDX);
	cond_goto_error(ret != 0, fail_after_map_reg, ret, ret,
		"parse platform dfx desc for blackbox failed\n");

	return 0;
fail_after_map_reg:
	(void)npu_plat_unmap_reg(pdev, plat_info);
	return ret;
}

static int npu_plat_init_spec(struct npu_platform_info *plat_info)
{
	plat_info->spec.stream_max = NPU_MAX_STREAM_ID;
	plat_info->spec.event_max = NPU_MAX_EVENT_ID + NPU_MAX_HWTS_EVENT_ID;
	plat_info->spec.notify_max = NPU_MAX_NOTIFY_ID;
	plat_info->spec.model_max = NPU_MAX_MODEL_ID;
	plat_info->spec.aicore_max = NPU_PLAT_AICORE_MAX;
	plat_info->spec.calc_sq_max = NPU_MAX_SQ_NUM;
	plat_info->spec.calc_sq_depth = NPU_MAX_SQ_DEPTH;
	plat_info->spec.calc_cq_max = NPU_MAX_CQ_NUM;
	plat_info->spec.calc_cq_depth = NPU_MAX_CQ_DEPTH;
	plat_info->spec.dfx_sq_max = NPU_MAX_DFX_SQ_NUM;
	plat_info->spec.dfx_cq_max = NPU_MAX_DFX_CQ_NUM;
	plat_info->spec.dfx_sq_depth = NPU_MAX_DFX_SQ_DEPTH;
	plat_info->spec.dfx_cq_depth = NPU_MAX_DFX_CQ_DEPTH;
	plat_info->spec.doorbell_stride = NPU_PLAT_DOORBELL_STRIDE;
	return 0;
}

static DEFINE_MUTEX(npu_platform_mutex);

static const struct of_device_id npu_of_match[] = {
	{ .compatible = "hisilicon,npu", },
	{},
};
MODULE_DEVICE_TABLE(of, npu_of_match);

static int npu_platform_suspend(struct platform_device *pdev, pm_message_t state)
{
	unused(pdev);
	unused(state);
	return 0;
}

static int npu_platform_resume(struct platform_device *pdev)
{
	unused(pdev);
	return 0;
}

static void npu_debugfs_init(void)
{
#ifdef CONFIG_NPU_DEBUG
	if (npu_debug_init() != 0)
		npu_drv_err("debug init failed\n");
#endif

#ifdef CONFIG_NPU_BBIT_DEBUGFS
	if (npu_bbit_debugfs_init() != 0)
		npu_drv_err("bbit debugfs init failed\n");
#endif
}

static int npu_platform_probe(struct platform_device *pdev)
{
	int ret;
	struct npu_platform_info *platform_info = NULL;

	npu_drv_warn("enter\n");
	platform_info = kzalloc(sizeof(struct npu_platform_info), GFP_KERNEL);
	cond_return_error(platform_info == NULL, -ENOMEM,
		"kzalloc platform info failed\n");

	platform_info->pdev = &pdev->dev;
	platform_info->plat_type = NPU_PLAT_DEVICE;
	platform_info->env_type = NPU_PLAT_TYPE_ASIC;

	ret = npu_plat_parse_dtsi(pdev, platform_info);
	if (ret != 0) {
		npu_drv_err("prase dtsi failed\n");
		goto PROB_FAIL;
	}

	ret = npu_plat_init_adapter(platform_info);
	if (ret != 0) {
		npu_drv_err("init platform adapter failed\n");
		goto PROB_FAIL;
	}

	ret = npu_plat_init_spec(platform_info);
	if (ret != 0) {
		npu_drv_err("init platform spec failed\n");
		goto PROB_FAIL;
	}

	ret = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64));
	if (ret != 0)
		npu_drv_err("mask coherent failed\n");
#ifdef CONFIG_NPU_INTR_HUB
	ret = npu_intr_hub_irq_init(platform_info);
	if (ret != 0) {
		npu_drv_err("intr_hub_irq init failed\n");
		goto PROB_FAIL;
	}
#endif
	npu_debugfs_init();

	npu_drv_warn("prob success\n");
	g_platform_info = platform_info;
	return 0;

PROB_FAIL:
	kfree(platform_info);
	platform_info = NULL;
	return ret;
}

struct npu_platform_info *npu_plat_get_info(void)
{
	return g_platform_info;
}
EXPORT_SYMBOL(npu_plat_get_info);

static int npu_platform_remove(struct platform_device *pdev)
{
	struct npu_platform_info *plat_info = NULL;

	npu_drv_debug("device remove start\n");

	plat_info = npu_plat_get_info();
	if (plat_info == NULL) {
		npu_drv_info("get platform info failed\n");
		return 0;
	}

	npu_plat_unmap_reg(pdev, plat_info);
	npu_drv_info("device remove succeed\n");

#ifdef CONFIG_NPU_INTR_HUB
	free_irq(plat_info->dts_info.irq_intr_hub, &plat_info->intr_hub);
#endif

	kfree(plat_info);
	g_platform_info = NULL;

	return 0;
}

static int npu_dev_pm_suspend(struct device *dev)
{
	int ret = 0;

	unused(dev);
	if(g_npd != NULL && g_npd->npd_notify) {
		ret = g_npd->npd_notify(get_dev_ctx_by_id(0), NPD_NOTICE_SUSPEND, 0, 0);
		if (ret != 0)
			npu_drv_err("npu_dev_pm_suspend fail");
	}

	return ret;
}

/* Currently there is no resume scene, if there is such a scene in the future, refer to suspend function */
static int npu_dev_pm_resume(struct device *dev)
{
	unused(dev);

	return 0;
}

#ifdef CONFIG_HIBERNATION

static vir_addr_t g_rsd_mem_tmp = 0;
static vir_addr_t g_rsd_mem_virt = 0;
static int npu_s4_rsd_ddr_backup(void)
{
	int ret = 0;
	g_rsd_mem_virt = (vir_addr_t)(uintptr_t)ioremap_wc(NPU_NONSEC_RESERVED_DDR_BASE_ADDR + NPU_NS_TSCPU_FW_SIZE,
		RESERVED_NPU_AI_SERVER_PHYMEM_SIZE);
	if (g_rsd_mem_virt == 0) {
		npu_drv_err("ioremap_wc failed!");
		return -1;
	}
	g_rsd_mem_tmp = (vir_addr_t)(uintptr_t)vmalloc(RESERVED_NPU_AI_SERVER_PHYMEM_SIZE);
	if (g_rsd_mem_tmp == 0) {
		iounmap((void *)(uintptr_t)g_rsd_mem_virt);
		g_rsd_mem_virt = 0;
		npu_drv_err("vmalloc failed!");
		return -1;
	}
	ret = memcpy_s((void *)(uintptr_t)g_rsd_mem_tmp, RESERVED_NPU_AI_SERVER_PHYMEM_SIZE,
		(void *)(uintptr_t)g_rsd_mem_virt, RESERVED_NPU_AI_SERVER_PHYMEM_SIZE);
	if (ret != 0) {
		npu_drv_err("memset error %d\n", ret);
		iounmap((void *)(uintptr_t)g_rsd_mem_virt);
		g_rsd_mem_virt = 0;
		vfree((void *)(uintptr_t)g_rsd_mem_tmp);
		g_rsd_mem_tmp = 0;
		return ret;
	}
	return 0;
}

static int npu_s4_rsd_ddr_recover(void)
{
	int ret = 0;
	if (g_rsd_mem_tmp != 0 && g_rsd_mem_virt != 0) {
		ret = memcpy_s((void *)(uintptr_t)g_rsd_mem_virt, RESERVED_NPU_AI_SERVER_PHYMEM_SIZE,
			(void *)(uintptr_t)g_rsd_mem_tmp, RESERVED_NPU_AI_SERVER_PHYMEM_SIZE);
		if (ret != 0)
			npu_drv_err("memset error %d\n", ret);

		iounmap((void *)(uintptr_t)g_rsd_mem_virt);
		g_rsd_mem_virt = 0;
		vfree((void *)(uintptr_t)g_rsd_mem_tmp);
		g_rsd_mem_tmp = 0;
	} else {
		npu_drv_err("invalid rsd mem addr, can not cp rsd mem\n");
	}
	return ret;
}

static int npu_s4_power_down(void)
{
	int ret = 0;
	u32 curr_work_mode;
	struct npu_dev_ctx *cur_dev_ctx = NULL;

	cur_dev_ctx = get_dev_ctx_by_id(0);
	if (cur_dev_ctx == NULL || cur_dev_ctx->pm.wakeup == NULL) {
		npu_drv_err("get current device failed");
		return -EINVAL;
	}

	mutex_lock(&cur_dev_ctx->pm.npu_power_mutex);
	if (cur_dev_ctx->pm.wm_cnt[NPU_NONSEC] != 0 || cur_dev_ctx->pm.wm_cnt[NPU_SEC] != 0) {
		npu_drv_err("current device is busy");
		mutex_unlock(&cur_dev_ctx->pm.npu_power_mutex);
		return -EAGAIN;
	}

	if (list_empty_careful(&cur_dev_ctx->proc_ctx_list)) {
		mutex_unlock(&cur_dev_ctx->pm.npu_power_mutex);
		return 0;
	}

	curr_work_mode = cur_dev_ctx->pm.work_mode;
	npu_drv_warn("curr_work_mode = 0x%x", curr_work_mode);

	if (npu_bitmap_clear(curr_work_mode, NPU_NONSEC) != 0) {
		npu_drv_err("npu is not idle in other workmode\n");
		mutex_unlock(&cur_dev_ctx->pm.npu_power_mutex);
		return -EAGAIN;
	}

	if (npu_bitmap_get(curr_work_mode, NPU_NONSEC) != 0) {
		npu_drv_warn("start npu suspend powerdown\n");
		ret = npu_suspend_powerdown(cur_dev_ctx);
		if (ret != 0) {
			npu_drv_err("npu powerdown failed\n");
			mutex_unlock(&cur_dev_ctx->pm.npu_power_mutex);
			return -EAGAIN;
		}
	}
	mutex_unlock(&cur_dev_ctx->pm.npu_power_mutex);
	return 0;
}

static int npu_dev_pm_s4_suspend(struct device *dev)
{
	int ret = 0;
	npu_drv_warn("npu_dev_pm_s4_suspend");

	// 1. reset load npu status
	load_npu_reset_status();

	// 2. power down npu
	ret = npu_s4_power_down();
	if (ret != 0) {
		npu_drv_err("powerdown fail, ret = %d\n", ret);
		return ret;
	}

	// 3. rsd mem cp to tmp
	ret = npu_s4_rsd_ddr_backup();
	return ret;
}

static int npu_hibernate_notify(struct notifier_block *nb, unsigned long mode, void *_unused)
{
	int ret = 0;
	npu_drv_warn("npu_hibernate_notify, mode = %u", mode);
	switch (mode) {
	case PM_POST_HIBERNATION:
		ret = load_npu_verify_image();
		npu_drv_warn("load_npu_verify_image, ret = %d", ret);

		(void)npu_s4_rsd_ddr_recover();
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

static struct notifier_block npu_hibernate_nb = {
	.notifier_call = npu_hibernate_notify,
	.priority = -1
};

static int __init npu_hibernate_init(void)
{
	int ret;

	ret = register_pm_notifier(&npu_hibernate_nb);
	if (ret != 0)
		pr_err("Can't register npu_hibernate_nb notifier ret %d\n", ret);

	return 0;
}
late_initcall(npu_hibernate_init);
#endif

static const struct dev_pm_ops npu_dev_pm_ops = {
	.suspend = npu_dev_pm_suspend,
	.resume  = npu_dev_pm_resume,
#ifdef CONFIG_HIBERNATION
	.freeze_noirq = npu_dev_pm_s4_suspend,
#endif
};

static struct platform_driver npu_platform_driver = {
	.probe = npu_platform_probe,
	.remove = npu_platform_remove,
	.suspend = npu_platform_suspend,
	.resume = npu_platform_resume,
	.driver = {
		.name = "npu_platform",
		.of_match_table = npu_of_match,
		.pm = &npu_dev_pm_ops,
	},
};

static int __init npu_platform_init(void)
{
	npu_drv_warn("nkd started\n"); // waiting for npd register

	return 0;
}
module_init(npu_platform_init);

static void __exit npu_platform_exit(void)
{
	npu_drv_warn("nkd cleanup\n");
}
module_exit(npu_platform_exit);

int nkd_register_plat_driver(struct npd_registry *npd)
{
	int ret;
	g_npd = npd;

	npu_drv_warn("npd is registering\n");

	ret = npu_intr_hub_init();
	if (ret) {
		npu_drv_err("npu intr_hub init fail\n");
		return ret;
	}
	/* init sequence is:
	 * (1) platform -> iova (used by hwts sq/cq + profiling) -> manager | prof
	 * (2) platform -> log
	 */
	ret = platform_driver_register(&npu_platform_driver);
	if (ret) {
		npu_drv_err("insmod devdrv platform driver fail\n");
		npu_intr_hub_exit();
		return ret;
	}
	npu_drv_warn("succeed\n");

	ret = npu_iova_init();
	if (ret) {
		npu_drv_err("iova init fail\n");
		platform_driver_unregister(&npu_platform_driver);
		npu_intr_hub_exit();
		return ret;
	}
	ret = npu_manager_init();
#ifndef CONFIG_NPU_SWTS
	log_drv_module_init();
#endif
	npu_prof_module_init();
#if defined CONFIG_DPM_HWMON && defined CONFIG_NPU_DPM_ENABLED
	dpm_npu_init();
#endif

	return ret;
}
EXPORT_SYMBOL(nkd_register_plat_driver);

int nkd_unregister_plat_driver(void)
{
	npu_drv_warn("npd is unregistering\n");
	g_npd = NULL;
#if defined CONFIG_DPM_HWMON && defined CONFIG_NPU_DPM_ENABLED
	dpm_npu_exit();
#endif
	npu_prof_module_exit();
#ifndef CONFIG_NPU_SWTS
	log_drv_module_exit();
#endif
	npu_manager_exit();
	npu_iova_exit();
	platform_driver_unregister(&npu_platform_driver);
	npu_intr_hub_exit();

	return 0;
}
EXPORT_SYMBOL(nkd_unregister_plat_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("NPU driver");
MODULE_VERSION("V1.0");
