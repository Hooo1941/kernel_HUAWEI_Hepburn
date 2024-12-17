#include <linux/irq.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/cpu.h>
#include <linux/kthread.h>
#include <linux/dma-mapping.h>

#include "npd_interface.h"
#include "npu_platform.h"
#include "npu_adapter.h"
#include "npu_pm_internal.h"
#include "npu_platform_exception.h"
#include "npu_arch_exception.h"
#include "npu_platform_pm.h"
#include "npu_autofs_plat.h"
#include "npu_resmem.h"
#include "npu_mntn.h"
#include "npu_sink_task_verify.h"
#include "npu_sink_sqe_fmt.h"
#include "npu_except_mgr.h"
#include "npu_log.h"

// declare static, only for NKD, NPD do not call
static int npd_dump_exception(struct npu_dev_ctx *dev_ctx, struct npu_exception_record* er)
{
	struct hwts_exception_report_info *report = er->u.ex_hwts_report;
	if (er->ex_source == NPU_EXCEPTION_SRC_TASK_TIMEOUT) {
		npu_timeout_exception_dump(dev_ctx);
	} else if (er->ex_source == NPU_EXCEPTION_SRC_TS) {
		if (report != NULL) {
			if (report->service_type == NPU_SERVICE_TYPE_ISPNN) {
				npu_ispnn_exception_dump(dev_ctx, report->exception_type, report->channel_id);
				return NPU_EXCEPTION_HANDLED;
			}
			if (report->exception_type >= NPU_EXCEPTION_TYPE_HWTS_TASK_EXCEPTION
				&& report->exception_type < NPU_EXCEPTION_TYPE_MAX){
				npu_hwts_exception_dump(dev_ctx, report->exception_type, report->hwts_sq_id, report->channel_id);
			} else {
				npu_drv_warn("invalid exception_type %d", report->exception_type);
			}
		} else {
			npu_drv_warn("exception form ts has no report");
		}
	} else {
		npu_drv_warn("bad exception record with source %d\n", er->ex_source);
	}

	return NPU_EXCEPTION_CONTINUE;
}

int __attribute__((weak)) npd_handle_exception(struct npu_dev_ctx *dev_ctx, struct npu_exception_record* er)
{
	unused(dev_ctx);
	unused(er);
	return NPU_EXCEPTION_CONTINUE;
}

static int npd_switch_unit(struct npu_dev_ctx *dev_ctx, enum npu_sfu_e sfu, enum npu_workmode_flag mode, void *para)
{
	int ret = 0;
	switch(sfu)
	{
	case NPU_SFU_AUTOFS:
		if(mode == POWER_ON)
			npu_autofs_on(para);
		else
			npu_autofs_off(para);
		break;
	case NPU_SFU_HWTS_AICORE_POOL:
		ret = npu_plat_switch_hwts_aicore_pool(dev_ctx, (npu_work_mode_info_t *)para, mode);
		break;
	default:
		ret = -1;
		break;
	}

	return ret;
}

static bool npd_query_sku_value(enum npu_sku_var_e sv)
{
	if(sv == NPU_SKU_VAR_SUPPORT_SC)
		return npu_plat_is_support_sc();
	return false;
}

static int npd_notify(struct npu_dev_ctx *dev_ctx, npd_notice_t pn, long para1, long para2)
{
	int ret = 0;
	unused(para2);

	switch(pn)
	{
	case NPD_NOTICE_PM_INIT:
		ret = npu_pm_init(dev_ctx);
		break;
	case NPD_NOTICE_POWER_UP:
		ret = npu_plat_pm_powerup(dev_ctx, (u32)para1);
		break;
	case NPD_NOTICE_POWER_DOWN:
		ret = npu_plat_pm_powerdown(dev_ctx, (u32)para1);
		break;
	case NPD_NOTICE_SUSPEND:
		ret = npu_plat_dev_pm_suspend();
		break;
	case NPD_NOTICE_MNTN_RESET:
		npu_plat_mntn_reset();
		break;
	case NPD_NOTICE_INFORM_TS_LIMIT_AICORE:
		ret = npu_plat_send_ts_ctrl_core((u32)para1);
		break;
	default:
		ret = -1;
		break;
	}

	return ret;
}

int __attribute__((weak)) npu_format_sw_sink_sqe(npu_model_desc_t *model_desc, void *stream_buf_addr,
	u16 *first_task_id, u8 devid, int stream_idx)
{
	unused(model_desc);
	unused(stream_buf_addr);
	unused(first_task_id);
	unused(devid);
	unused(stream_idx);
	return 0;
}

int __attribute__((weak)) npu_format_hw_sink_sqe(npu_model_desc_info_t *model_desc_info,
	void *stream_buf_addr, void *hwts_sq_addr, u64 ts_sq_len)
{
	unused(model_desc_info);
	unused(stream_buf_addr);
	unused(hwts_sq_addr);
	unused(ts_sq_len);
	return 0;
}

int __attribute__((weak)) npu_nosec_get_workmode(struct npu_proc_ctx *proc_ctx,
	npu_rt_task_t *comm_task)
{
	unused(proc_ctx);
	unused(comm_task);
	return NPU_NONSEC;
}

struct npd_registry g_npd_registry = {
	.version = NPD_REGISTRY_V1,
	.npd_notify = npd_notify,
	.npd_dump_exception = npd_dump_exception,
	.npd_handle_exception = npd_handle_exception,
	.npd_switch_unit = npd_switch_unit,
	.npd_query_sku_value = npd_query_sku_value,
	.npd_send_mail = npu_plat_res_mailbox_send,
	.npd_set_resmem = npu_plat_set_resmem,
	.npd_verify_sink_tasks = npu_verify_sink_tasks,
	.npd_format_sw_sqe = npu_format_sw_sink_sqe,
	.npd_format_hw_sqe = npu_format_hw_sink_sqe,
	.npd_nosec_get_workmode = npu_nosec_get_workmode,
};

static int __init npd_init(void)
{
	int ret;
	struct npu_dev_ctx *dev_ctx = NULL;

	if (npu_plat_bypass_status() == NPU_BYPASS)
		return 0;

	g_npd_registry.feature_mask = npu_get_feature_bits();

	nkd_register_plat_driver(&g_npd_registry);
	dev_ctx = get_dev_ctx_by_id(0);
	if (dev_ctx == NULL)
		return -1;
	ret = npu_exception_svc_init(dev_ctx);
	cond_return_error(ret != 0, ret, "npu_exception_svc_init_fail, ret = %d\n", ret);
	return 0;
}
module_init(npd_init);

static void __exit npd_exit(void)
{
	struct npu_dev_ctx *dev_ctx = NULL;

	if (npu_plat_bypass_status() == NPU_BYPASS)
		return;

	nkd_unregister_plat_driver();
	dev_ctx = get_dev_ctx_by_id(0);
	if (dev_ctx == NULL)
		return;
	npu_exception_svc_deinit(dev_ctx);
}
module_exit(npd_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("NPU Platform Driver(NPD)");
MODULE_VERSION("V1.0");
