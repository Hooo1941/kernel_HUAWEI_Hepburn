/*
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <platform_include/basicplatform/linux/dfx_fiq.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/smp.h>
#include <linux/kmsg_dump.h>
#include <linux/blkdev.h>
#include <linux/io.h>
#include <linux/version.h>

#ifdef CONFIG_CORESIGHT
#include <linux/coresight.h>
#endif

#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
#include <linux/sched/debug.h>
#endif

#ifdef CONFIG_DDR_DDRC_SEC
#include <platform_include/cee/linux/hisi_ddr.h>
#endif
#include <platform_include/basicplatform/linux/mntn_record_sp.h>
#include <platform_include/basicplatform/linux/eeye_ftrace_pub.h>
#include <platform_include/basicplatform/linux/lpcpu_mntn_l3cache_ecc.h>
#include <platform_include/basicplatform/linux/dfx_universal_wdt.h>
#include <platform_include/basicplatform/linux/rdr_pub.h>
#include <platform_include/basicplatform/linux/pr_log.h>
#include <platform_include/basicplatform/linux/dfx_bbox_diaginfo.h>
#include <platform_include/basicplatform/linux/rdr_platform.h>

#include <asm/stacktrace.h>
#include <asm/exception.h>
#include <asm/system_misc.h>
#include <asm/cacheflush.h>

#include <securec.h>
#include <mntn_subtype_exception.h>

#include "bl31/dfx_bl31_exception.h"
#include "blackbox/rdr_inner.h"
#include "blackbox/rdr_field.h"

#define PR_LOG_TAG FIQ_TAG
static u32 g_fiq_dump_flag;

void dfx_mntn_inform(void)
{
	if (get_bl31_exception_flag() == BL31_PANIC_MAGIC)
		bl31_panic_ipi_handle();
#ifdef CONFIG_DDR_DDRC_SEC
	else
		dmss_ipi_handler();
#endif
}

#define ABNORMAL_RST_FLAG 0xFF

asmlinkage void fiq_dump(struct pt_regs *regs, unsigned int esr)
{
	struct rdr_exception_info_s *p_exce_info = NULL;
	char date[DATATIME_MAXLEN];
	int ret;
	u64 err1_status;
	u64 err1_misc0;
	unsigned int reset_reason;

#if (defined CONFIG_DFX_LOCK_TRACE) && (defined CONFIG_DEBUG_SPINLOCK)
	dump_rq_lock_owner();
#endif

	flush_logbuff_range();
	g_fiq_dump_flag = 0xdeaddead;
#ifdef CONFIG_DFX_MNTN_GT_WATCH_SUPPORT
	if (dfx_pm_gt_watch_type() == GT_WATCH_A)
		dfx_pm_ap_pull_down_state();
#endif
	bust_spinlocks(1); /* bust_spinlocks is open */
	flush_ftrace_buffer_cache();

	/*
	 * If the system is reset abnormally, the reset may fail. In this case, only the watchdog can be reset.
	 * In this case, the previous abnormality records will be overwritten.
	 */
	reset_reason = get_reboot_reason();
	if ((reset_reason == ABNORMAL_RST_FLAG) || (reset_reason == AP_S_AWDT)) {
#ifdef CONFIG_RESET_VM_SOLO
		vm_set_reboot_reason_subtype(reset_reason, HI_APWDT_AP);
#else
		set_subtype_exception(HI_APWDT_AP, true);
#endif
	}

#ifdef CONFIG_CORESIGHT
	coresight_disable_all();
#endif

#ifdef CONFIG_HUAWEI_PRINTK_CTRL
	printk_level_setup(LOGLEVEL_DEBUG);
#endif
	pr_crit("%s begin!\n", __func__);

	pr_emerg("%s", linux_banner);

#ifdef CONFIG_DDR_DDRC_SEC
	dmss_fiq_handler();
#endif

	console_verbose();
	show_regs(regs);

#ifdef CONFIG_MAS_BLK
	mas_blk_panic_flush(); /* Flush the storage device cache */
#endif

	dump_stack();
	smp_send_stop();

#ifdef CONFIG_DFX_BB
	if (!rdr_init_done()) {
		pr_crit("rdr init faild!\n");
		return;
	}

	last_task_stack_dump();
	regs_dump(); /* "sctrl", "pctrl", "peri_crg", "gic" */
#endif

#ifdef CONFIG_DFX_BB
	save_module_dump_mem();
#endif

	dfx_wdt_dump();

	rdr_field_baseinfo_reinit();
	rdr_save_args(MODID_AP_S_WDT, 0, 0);
	p_exce_info = rdr_get_exception_info(MODID_AP_S_WDT);
	if (p_exce_info != NULL) {
		if (memset_s(date, DATATIME_MAXLEN, 0, DATATIME_MAXLEN) != EOK) {
			pr_err("[%s:%d]: memset_s err\n", __func__, __LINE__);
			return;
		}

		ret = snprintf_s(date, DATATIME_MAXLEN, DATATIME_MAXLEN - 1, "%s-%08lld",
			 rdr_get_timestamp(), rdr_get_tick());
		if (unlikely(ret < 0)) {
			pr_crit("snprintf_s ret %d!\n", ret);
			return;
		}

		rdr_fill_edata(p_exce_info, date);

		(void)rdr_exception_trace_record(p_exce_info->e_reset_core_mask,
			p_exce_info->e_from_core, p_exce_info->e_exce_type, p_exce_info->e_exce_subtype);
	}

	rdr_ap_dump_root_head(MODID_AP_S_WDT, AP_S_AWDT, RDR_AP);
	bbox_diaginfo_dump_lastmsg();
	pr_crit("%s end\n", __func__);
	(void)l3cache_ecc_get_status(&err1_status, &err1_misc0);
	mntn_show_stack_cpustall();

	kmsg_dump(KMSG_DUMP_PANIC);
#ifdef CONFIG_GCOV_KERNEL
	gcov_get_gcda();
#endif
	flush_logbuff_range();
	flush_cache_all();

#if defined(CONFIG_DFX_SP805_WATCHDOG) || defined(CONFIG_WATCHDOG_V500)
	wdt_fast_reset();
#endif
	asm("b .");
}