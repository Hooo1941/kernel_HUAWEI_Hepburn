/*
 * rdr_audio_soc.c
 *
 * audio soc driver
 *
 * Copyright (c) 2015-2020 Huawei Technologies CO., Ltd.
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
#define LOG_TAG "rdr_audio_soc"

#include "rdr_audio_soc.h"

#include <linux/kernel.h>
#include <linux/kthread.h>
#include <platform_include/basicplatform/linux/ipc_rproc.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/of_irq.h>
#include <asm/tlbflush.h>
#include "rdr_print.h"
#include "rdr_inner.h"
#include "rdr_field.h"
#include "rdr_audio_adapter.h"
#include "rdr_audio_dump_socdsp.h"
#ifdef HIFI_RESET_CDEV
#include "rdr_audio_notify_modem.h"
#endif
#include "audio_log.h"
#include "dsp_misc.h"
#include "dsp_om.h"
#include "platform_base_addr_info.h"

#ifdef CONFIG_USB_AUDIO_DSP
#include "usbaudio_ioctl.h"
#endif
#include <linux/platform_drivers/usb/hifi_usb.h>
#ifdef CONFIG_HUAWEI_DSM
#include <dsm_audio/dsm_audio.h>
#endif
#ifdef CONFIG_HIFI_SECOS_LOAD
#include "dsp_load_ca.h"
#else
#include <linux/mm.h>
#include <linux/dma-mapping.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
#include <linux/dma-map-ops.h>
#else
#include <linux/dma-contiguous.h>
#endif
#include <linux/remoteproc.h>
#include <linux/of_reserved_mem.h>
#include <platform_include/see/load_sec_image.h>
#include <platform_include/basicplatform/linux/partition/partition_macro.h>
#include <platform_include/basicplatform/linux/partition/partition_ap_kernel.h>
#include <platform_include/display/linux/dpu_drmdriver.h>
#include <linux/fs.h>
#include <linux/syscalls.h>
#include <asm/cacheflush.h>
#include <asm/tlbflush.h>
#include "teek_client_id.h"
#endif

#define BSP_RESET_OK 0
#define BSP_RESET_ERROR (-1)

#define RDR_COMMENT_LEN 128UL
#define PARTION_INFO_LEN 10
#define DSP_BSS_SEC 2
#define SOC_WDT_TIMEOUT_IRQ_NUM 245U

#define WATCHDOG_DTS_COMP_NAME "hisilicon,sochifi-watchdog"


#define DRV_WATCHDOG_SIZE 0x1000UL

#ifdef CONFIG_ASP_WATCHDOG_V500
#define DRV_WATCHDOG_LOCKEN 0x000
#define DRV_WATCHDOG_LOCKDIS 0x004
#define DRV_WATCHDOG_INTCLR 0x01C
#define DRV_WATCHDOG_CONTRL 0x014
#define DRV_WATCHDOG_STATE 0x02c
#define DRV_WATCHDOG_UNLOCK_NUM 0x1AAEE533
#define DRV_WATCHDOG_LOCK_NUM 0x0
#define DRV_WATCHDOG_CONTROL_DISABLE 0x0
#define DRV_WATCHDOG_INTCLR_NUM 0x1
#define DRV_WATCHDOG_DELAY_32K_CLK_3CYCLE 94
#else
#define DRV_WATCHDOG_CONTRL 0x008
#define DRV_WATCHDOG_INTCLR 0x00C
#define DRV_WATCHDOG_LOCKEN 0xC00
#define DRV_WATCHDOG_LOCKDIS (DRV_WATCHDOG_LOCKEN)
#define DRV_WATCHDOG_UNLOCK_NUM 0x1ACCE551
#define DRV_WATCHDOG_LOCK_NUM 0x0
#define DRV_WATCHDOG_CONTROL_DISABLE 0x0
#define DRV_WATCHDOG_INTCLR_NUM 0x4455 /* Generally speaking, any number is okay */
#endif

#define SOCDSP_DIE_NOTIFY_LPM3 ((0 << 24) | (16 << 16) | (3 << 8) | (1 << 0)) /* bit 24-31 OBJ_AP
                                                                               * bit 16-23 OBJ_PSCI
                                                                               * bit 8-15  CMD_SETTING
                                                                               * bit 0-7   TYPE_POWER
                                                                               */
#define CFG_MMBUF_REMAP_EN 0x130 /* mmbuf remap enable 9bit */
#define CFG_OCRAM_REMAP_EN 0x13C /* ocram remap enable 9bit */
#define ASP_CFG_BASE SOC_ACPU_ASP_CFG_BASE_ADDR
#define FLAG_ROW_LEN 64
#define FLAG_COMMENT_LEN 128
#define PARSE_FLAG_LOG_SIZE (FLAG_ROW_LEN * ARRAY_SIZE(g_socdsp_flag) + FLAG_COMMENT_LEN)

#ifdef CONFIG_AUDIO_SOC_SHARED_IRQ
#define INTR_GIC_FROM_L2_0_AO 0
#define INTR_HUB_AO_BASE_ADDR SOC_ACPU_intr_hub_ao_BASE_ADDR
#define ACTRL_BASE_ADDR SOC_ACPU_ACTRL_BASE_ADDR
#define INTR_HUB_AO_SIZE 0x1000UL
#define INTR_HUB_AO_MASK_SET ((INTR_GIC_FROM_L2_0_AO) * 0x10 + 0x4)
#define INTR_HUB_AO_MASK_CLEAR ( (INTR_GIC_FROM_L2_0_AO) * 0x10 + 0x8)
#define INTR_HUB_AO_STATE ((INTR_GIC_FROM_L2_0_AO) * 0x10 + 0xc)
#define INTR_ASP_WATCHDOG BIT(0)
#define INTR_ASP_DWAXI_DLOCK  BIT(1)
#define INTR_ASP_ADDR_MONITOR BIT(2)

#define ACTRL_SIZE 0x1000UL
#define NONSEC_STATUS4 (0x90)
#define NONSEC_STATUS5 (0x94)
#endif

struct rdr_soc_des_s {
	uint32_t modid;
	uint32_t wdt_irq_num;
	char *pathname;
	void __iomem *wtd_base_addr;
#ifdef CONFIG_AUDIO_SOC_SHARED_IRQ
	void __iomem *intr_hub_ao_addr;
	void __iomem *actrl_base_addr;
#endif
	pfn_cb_dump_done dumpdone_cb;

	struct semaphore dump_sem;
	struct semaphore handler_sem;
	struct wakeup_source *rdr_wl;
	struct task_struct *kdump_task;
	struct task_struct *khandler_task;
};

struct sreset_mgr_lli *g_pmgr_dspreset_data;
static struct rdr_soc_des_s g_soc_des;

#ifndef CONFIG_HIFI_SECOS_LOAD
#define DTS_COMP_HIFICMA_NAME "hisilicon,hifi-cma"
#ifdef CONFIG_HIFI_MEMORY_21M
#define DSP_CMA_IMAGE_SIZE 0xA00000UL
#else
#define DSP_CMA_IMAGE_SIZE 0x600000UL
#endif
struct audio_dsp_cma_struct {
	struct device *device;
};

struct audio_dsp_cma_struct g_dspcma_dev;
#endif
/*lint -e446*/

static void audio_rdr_remap_init(void)
{
	void __iomem *rdr_aspcfg_base = NULL;
	unsigned int read_val;

	rdr_aspcfg_base = ioremap(get_phy_addr(ASP_CFG_BASE, PLT_ASP_CFG_MEM),
		(unsigned long)SZ_4K);
	if (!rdr_aspcfg_base) {
		AUDIO_LOGE("rdr aspcfg base ioremap error");
		return;
	}

	read_val = (unsigned int)readl(rdr_aspcfg_base + CFG_MMBUF_REMAP_EN);
	read_val &= BIT(DRV_MODULE_NAME_LEN);
	if (read_val != 0)
		writel(0x0, (rdr_aspcfg_base + CFG_MMBUF_REMAP_EN));

	read_val = (unsigned int)readl(rdr_aspcfg_base + CFG_OCRAM_REMAP_EN);
	read_val &= BIT(DRV_MODULE_NAME_LEN);
	if (read_val != 0)
		writel(0x0, (rdr_aspcfg_base + CFG_OCRAM_REMAP_EN));

	iounmap(rdr_aspcfg_base);
}

static int irq_handler_thread(void *arg)
{
	AUDIO_LOGI("enter blackbox");

	while (!kthread_should_stop()) {
		if (down_interruptible(&g_soc_des.handler_sem)) {
			AUDIO_LOGE("down sem fail");
			continue;
		}
#ifdef CONFIG_AUDIO_SOC_SHARED_IRQ
		if (g_soc_des.modid == (unsigned int)RDR_AUDIO_ASP_BUS_EXCEPTION_MODID) {
			g_soc_des.modid = 0;
			AUDIO_LOGI("enter rdr process for asp bus error");
			rdr_system_error((unsigned int)RDR_AUDIO_ASP_BUS_EXCEPTION_MODID, 0, 0);
			AUDIO_LOGI("exit rdr process for asp bus error");

			AUDIO_LOGI("exit  blackbox");
			return 0;
		}
#endif
		AUDIO_LOGI("socdsp timestamp: %u socdsp watchdog coming", DSP_STAMP);
#ifdef CONFIG_USB_AUDIO_DSP
		usbaudio_ctrl_hifi_reset_inform();
#endif
		hifi_usb_hifi_reset_inform();
		dsp_reset_release_syscache();
		if (is_dsp_power_on()) {
			audio_rdr_nmi_notify_dsp();
			audio_rdr_remap_init();
		} else {
			AUDIO_LOGE("dsp is power off, do not send nmi & remap");
		}
#ifdef HIFI_RESET_CDEV
		dsp_reset_notify_modem(DSP_RESET_STATE_OFF);
#endif
		hifireset_runcbfun(DRV_RESET_CALLCBFUN_RESET_BEFORE);

		AUDIO_LOGI("enter rdr process for socdsp watchdog");
		rdr_system_error((unsigned int)RDR_AUDIO_SOC_WD_TIMEOUT_MODID, 0, 0);
		AUDIO_LOGI("exit rdr process for socdsp watchdog");
	}

	AUDIO_LOGI("exit  blackbox");

	return 0;
}

static int audio_rdr_ipc_notify_lpm3(unsigned int *msg, int len)
{
	int ret;
	int i;

	for (i = 0; i < len; i++)
		AUDIO_LOGI("rdr: [ap2lpm3 notifiy] msg[%d]: 0x%x", i, msg[i]);

	ret = RPROC_ASYNC_SEND(IPC_ACPU_LPM3_MBX_5, msg, len);
	if (ret != 0)
		AUDIO_LOGE("send mesg to lpm3 fail");

	return ret;
}

#ifndef CONFIG_HIFI_SECOS_LOAD
static int rdr_dsp_cma_alloc(phys_addr_t *hifi_addr)
{
	struct audio_dsp_cma_struct *dev = &g_dspcma_dev;
	phys_addr_t phys;
	struct page *page = NULL;
	unsigned int val = 0;
	int ret = 0;

	if (of_property_read_u32(dev->device->of_node, "audio-align", &val)) {
		AUDIO_LOGE("read align val err");
		return -EINVAL;
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
	page = dma_alloc_from_contiguous(dev->device,
		DSP_CMA_IMAGE_SIZE >> PAGE_SHIFT, val, false);
#else
	page = dma_alloc_from_contiguous(dev->device,
		DSP_CMA_IMAGE_SIZE >> PAGE_SHIFT, val, GFP_KERNEL);
#endif
	if (!page) {
		AUDIO_LOGE("dma_alloc_from_contiguous (hifi) alloc err");
		return -ENOMEM;
	}
	phys = page_to_phys(page);
	if (phys & (phys_addr_t)(BIT(val) - 1)) {
		AUDIO_LOGE("align error, align val %u", val);
		return -EINVAL;
	}

	change_secpage_range(phys, (uintptr_t)phys_to_virt(phys),
		DSP_CMA_IMAGE_SIZE, __pgprot(PROT_DEVICE_nGnRE));

	flush_tlb_all();
	__dma_unmap_area(phys_to_virt(phys), DSP_CMA_IMAGE_SIZE, DMA_BIDIRECTIONAL);

	*hifi_addr = phys;
	AUDIO_LOGI("rdr: addr 0x%pK", (void *)(uintptr_t)phys);
	return ret;
}

static int rdr_dsp_cma_free(phys_addr_t addr)
{
	struct audio_dsp_cma_struct *dev = (struct audio_dsp_cma_struct *)&g_dspcma_dev;

	if (addr == 0) {
		AUDIO_LOGE("addr is null");
		return -EINVAL;
	}

	change_secpage_range(addr, (uintptr_t)phys_to_virt(addr),
		DSP_CMA_IMAGE_SIZE, __pgprot(PROT_NORMAL));

	flush_tlb_all();
	(void)dma_release_from_contiguous(dev->device, phys_to_page(addr),
		(int)DSP_CMA_IMAGE_SIZE >> PAGE_SHIFT);

	return 0;
}

static int get_dsp_image_size(unsigned int *size)
{
	ssize_t cnt;
	int ret = 0;
	char path[RDR_FNAME_LEN + 1] = {0};
	struct drv_socdsp_image_head socdsp_head = {{0}};
	mm_segment_t old_fs;
	struct file *file = NULL;

	if (flash_find_ptn_s(PART_FW_HIFI, path, sizeof(path)) < 0) {
		AUDIO_LOGE("partion_name hifi is not in partion table");
		return -EINVAL;
	}

	old_fs = get_fs(); /*lint !e501*/
	set_fs(KERNEL_DS); /*lint !e501*/

	file = filp_open(path, O_RDWR, 0);
	if (IS_ERR_OR_NULL(file)) {
		AUDIO_LOGE("rdr: open %s failed", path);
		set_fs(old_fs);
		return -ENOENT;
	}
	cnt = vfs_read(file, (char *)&socdsp_head, sizeof(socdsp_head), &file->f_pos);
	if (cnt != (int)sizeof(socdsp_head)) {
		AUDIO_LOGE("rdr: read %s failed, return %ld", path, cnt);
		ret = -EINVAL;
	}

	filp_close(file, NULL);
	set_fs(old_fs);
	*size = socdsp_head.image_size;
	AUDIO_LOGI("rdr: %u", socdsp_head.image_size);

	return ret;
}

static bool is_reload_socdsp_by_secos(void)
{
	struct audio_dsp_cma_struct *dev = &g_dspcma_dev;
	const char *enable_status = NULL;

	if (of_property_read_string(dev->device->of_node, "enable-status", &enable_status) == 0) {
		if (strncmp(enable_status, "true", strlen("true")) == 0)
			return true;
	} else {
		AUDIO_LOGE("read enable-status err");
	}

	return false;
}

int load_socdsp_by_secos(void)
{
	struct load_image_info loadinfo = {0};
	phys_addr_t dsp_addr = 0;
	char partion_name[PARTION_INFO_LEN] = PART_FW_HIFI;
	int ret;
	int free_ret;

	ret = rdr_dsp_cma_alloc(&dsp_addr);
	if (ret != 0) {
		AUDIO_LOGE("rdr: alloc cma buf err: %d", ret);
		return ret;
	}

	loadinfo.ecoretype = HIFI;
	loadinfo.image_addr = (unsigned long)dsp_addr;
	loadinfo.partion_name = partion_name;
	ret = get_dsp_image_size(&loadinfo.image_size);
	if (ret != 0) {
		AUDIO_LOGE("rdr: get dsp image size fail: %u, ret: %d",
			loadinfo.image_size, ret);
		goto end;
	}

	if (loadinfo.image_size > DSP_CMA_IMAGE_SIZE || loadinfo.image_size == 0) {
		ret = -EINVAL;
		AUDIO_LOGE("rdr: image size: %u, alloc cma buf size: %lu",
			loadinfo.image_size, DSP_CMA_IMAGE_SIZE);
		goto end;
	}

	ret = bsp_load_and_verify_image(&loadinfo);
	if (ret < 0) {
		AUDIO_LOGE("rdr: bsp load and verify image pre fail: %d", ret);
		goto end;
	}

	AUDIO_LOGI("rdr: bsp load and verify image success");

end:
	free_ret = rdr_dsp_cma_free(dsp_addr);
	if (free_ret != 0) {
		AUDIO_LOGE("rdr: free cma buffer error: %d", free_ret);
		ret = free_ret;
	}

	return ret;
}
#endif

static int reset_socdsp(void)
{
	unsigned int *power_status_addr = NULL;
	unsigned int msg;
	int ret;

	AUDIO_LOGI("enter blackbox");

#ifdef CONFIG_HIFI_SECOS_LOAD
	ret = dsp_load_and_verify_image();
	if (ret) {
		AUDIO_LOGE("load and verify dsp image fail, ret: %d, reboot now", ret);
		BUG_ON(1);
	}
#else
	if (is_reload_socdsp_by_secos()) {
		AUDIO_LOGI("rdr: reload socdsp by secos");
		ret = load_socdsp_by_secos();
		if (ret) {
			AUDIO_LOGE("secos reload socdsp fail, ret: %d, reboot now", ret);
			BUG_ON(1);
		}
	}
#endif

	power_status_addr = ioremap_wc(get_phy_addr(DRV_DSP_POWER_STATUS_ADDR, PLT_HIFI_MEM), 0x4);
	if (!power_status_addr) {
		AUDIO_LOGE("DRV_DSP_POWER_STATUS_ADDR ioremap failed");
	} else {
		writel(DRV_DSP_POWER_OFF, power_status_addr);
		iounmap(power_status_addr);
	}

	msg = SOCDSP_DIE_NOTIFY_LPM3;
	ret = audio_rdr_ipc_notify_lpm3(&msg, 1);
	AUDIO_LOGI("rdr: power off hifi %s", (ret != 0) ? "fail" : "success");

	AUDIO_LOGI("exit  blackbox");

	return ret;
}

static struct sreset_mgr_lli *reset_link_insert(struct sreset_mgr_lli *plink, struct sreset_mgr_lli *punit)
{
	struct sreset_mgr_lli *phead = plink;
	struct sreset_mgr_lli *ppose = plink;
	struct sreset_mgr_lli *ptail = plink;

	if (!plink || !punit) {
		AUDIO_LOGE("input params are not legitimate");
		return NULL;
	}

	while (ppose) {
		/* insert into linked list according to priority */
		if (ppose->cbfuninfo.priolevel > punit->cbfuninfo.priolevel) {
			if (phead == ppose) {
				punit->pnext = ppose;
				phead = punit;
			} else {
				ptail->pnext = punit;
				punit->pnext = ppose;
			}
			break;
		}
		ptail = ppose;
		ppose = ppose->pnext;
	}

	if (!ppose)
		ptail->pnext = punit;

	return phead;
}

static struct sreset_mgr_lli *reset_do_regcbfunc(struct sreset_mgr_lli *plink,
	const char *pname, hifi_reset_cbfunc pcbfun, int userdata, int priolevel)
{
	struct sreset_mgr_lli *phead = plink;
	struct sreset_mgr_lli *pmgr_unit = NULL;

	if ((!pname) || (!pcbfun) || (priolevel < RESET_CBFUNC_PRIO_LEVEL_LOWT
		|| priolevel > RESET_CBFUNC_PRIO_LEVEL_HIGH)) {
		AUDIO_LOGE("fail in ccore reset regcb, fail, name 0x%s, cbfun 0x%pK, prio %d",
			pname, pcbfun, priolevel);
		return NULL;
	}

	pmgr_unit = kmalloc(sizeof(*pmgr_unit), GFP_KERNEL);
	if (pmgr_unit) {
		memset((void *)pmgr_unit, 0, (sizeof(*pmgr_unit)));
		strncpy(pmgr_unit->cbfuninfo.name, pname, (unsigned long)DRV_MODULE_NAME_LEN);
		pmgr_unit->cbfuninfo.priolevel = priolevel;
		pmgr_unit->cbfuninfo.userdata = userdata;
		pmgr_unit->cbfuninfo.cbfun = pcbfun;
	} else {
		AUDIO_LOGE("pmgr unit malloc fail");
		return NULL;
	}

	if (!phead)
		phead = pmgr_unit;
	else
		/* insert linked list according to priority */
		phead = reset_link_insert(phead, pmgr_unit);

	return phead;
}

#ifdef CONFIG_HIFI_BB

int hifireset_regcbfunc(const char *pname, hifi_reset_cbfunc pcbfun, int userdata, int priolevel)
{
	g_pmgr_dspreset_data = reset_do_regcbfunc(g_pmgr_dspreset_data, pname,
		pcbfun, userdata, priolevel);
	AUDIO_LOGI("%s registered a cbfun for dsp reset", pname);

	return 0;
}
#endif /* end of config_audio_dsp_bb */

static void run_all_cbfunction(struct sreset_mgr_lli *phead, enum DRV_RESET_CALLCBFUN_MOMENT eparam)
{
	int ret;

	while (phead) {
		if (phead->cbfuninfo.cbfun) {
			ret = phead->cbfuninfo.cbfun(eparam, phead->cbfuninfo.userdata);
			if (ret)
				AUDIO_LOGE("run %s cb function 0x%pK fail, return %d", phead->cbfuninfo.name, phead->cbfuninfo.cbfun, ret);
			else
				AUDIO_LOGI("run %s cb function 0x%pK succ, return %d", phead->cbfuninfo.name, phead->cbfuninfo.cbfun, ret);
		}
		phead = phead->pnext;
	}
}

static void run_specified_cbfunction(struct sreset_mgr_lli *phead, enum DRV_RESET_CALLCBFUN_MOMENT eparam)
{
	int ret;

	while (phead) {
		/* traverse the linked list of callback functions and call NAS, MDRV callbacks */
		if ((strncmp("NAS_AT", phead->cbfuninfo.name, strlen(phead->cbfuninfo.name)) == 0 ||
			strncmp("MDRV", phead->cbfuninfo.name, strlen(phead->cbfuninfo.name)) == 0) &&
			phead->cbfuninfo.cbfun) {
			AUDIO_LOGI("run %s cb function begin", phead->cbfuninfo.name);
			ret = phead->cbfuninfo.cbfun(eparam, phead->cbfuninfo.userdata);
			if (ret)
				AUDIO_LOGE("run %s cb function 0x%pK fail, return %d", phead->cbfuninfo.name, phead->cbfuninfo.cbfun, ret);
			else
				AUDIO_LOGI("run %s cb function 0x%pK succ, return %d", phead->cbfuninfo.name, phead->cbfuninfo.cbfun, ret);
		}
		phead = phead->pnext;
	}
}

int hifireset_runcbfun(enum DRV_RESET_CALLCBFUN_MOMENT eparam)
{
	struct sreset_mgr_lli *phead = g_pmgr_dspreset_data;

	if (eparam == DRV_RESET_CALLCBFUN_RESET_BEFORE) {
		run_specified_cbfunction(phead, eparam);
	} else {
		run_all_cbfunction(phead, eparam);
	}

	return BSP_RESET_OK;
}

void rdr_audio_soc_reset(unsigned int modid, unsigned int etype, u64 coreid)
{
	int ret;

	AUDIO_LOGI("enter blackbox");

	ret = reset_socdsp();
	if (ret != 0) {
		__pm_relax(g_soc_des.rdr_wl);
		AUDIO_LOGE("rdr: reset socdsp error");
		return;
	}

#ifdef CONFIG_HIFI_DSP_ONE_TRACK
	dsp_watchdog_send_event();
#endif

#ifdef HIFI_RESET_CDEV
	dsp_reset_notify_modem(DSP_RESET_STATE_READY);
#endif
	hifireset_runcbfun(DRV_RESET_CALLCBFUN_RESET_AFTER);

	__pm_relax(g_soc_des.rdr_wl);

	AUDIO_LOGI("exit  blackbox");
}

static irqreturn_t soc_wtd_irq_handler(int irq, void *data)
{
	AUDIO_LOGI("socdsp watchdog irq arrival");

	writel(DRV_WATCHDOG_UNLOCK_NUM, (g_soc_des.wtd_base_addr + DRV_WATCHDOG_LOCKEN));
	writel(DRV_WATCHDOG_INTCLR_NUM, (g_soc_des.wtd_base_addr + DRV_WATCHDOG_INTCLR));
	writel(DRV_WATCHDOG_CONTROL_DISABLE, (g_soc_des.wtd_base_addr + DRV_WATCHDOG_CONTRL));
#ifdef CONFIG_ASP_WATCHDOG_V500
	udelay(DRV_WATCHDOG_DELAY_32K_CLK_3CYCLE);
#endif
	writel(DRV_WATCHDOG_LOCK_NUM, (g_soc_des.wtd_base_addr + DRV_WATCHDOG_LOCKDIS));

	__pm_stay_awake(g_soc_des.rdr_wl);

	up(&g_soc_des.handler_sem);

	return IRQ_HANDLED;
}

#ifdef CONFIG_AUDIO_SOC_SHARED_IRQ
static void mask_asp_addr_monitor_irq(void)
{
	writel(INTR_ASP_ADDR_MONITOR, (g_soc_des.intr_hub_ao_addr + INTR_HUB_AO_MASK_CLEAR));
}

static void mask_asp_bus_exception_irq(void)
{
	writel(INTR_ASP_DWAXI_DLOCK, (g_soc_des.intr_hub_ao_addr + INTR_HUB_AO_MASK_CLEAR));
}

static unsigned int get_soc_irq_state(void)
{
	return readl(g_soc_des.intr_hub_ao_addr + INTR_HUB_AO_STATE);
}

static irqreturn_t asp_addr_monitor_irq_handler(int irq, void *data)
{
	AUDIO_LOGI("asp addr monitor irq arrival");

	mask_asp_addr_monitor_irq();

	return IRQ_HANDLED;
}

static irqreturn_t asp_bus_exception_irq_handler(int irq, void *data)
{
	AUDIO_LOGI("asp bus exception irq arrival");

	mask_asp_bus_exception_irq();

	return IRQ_HANDLED;
}
#endif

static irqreturn_t soc_irq_handler(int irq, void *data)
{
#ifdef CONFIG_AUDIO_SOC_SHARED_IRQ
	unsigned int int_state;

	int_state  = get_soc_irq_state();
	AUDIO_LOGI("rdr: socdsp irq state = 0x%x", int_state);
	AUDIO_LOGI("rdr: actrl state nonsec status4 = 0x%x, status5 = 0x%x",
		readl(g_soc_des.actrl_base_addr + NONSEC_STATUS4), readl(g_soc_des.actrl_base_addr + NONSEC_STATUS5));

	if (int_state & INTR_ASP_DWAXI_DLOCK)
		return asp_bus_exception_irq_handler(irq, data);
	else if (int_state & INTR_ASP_ADDR_MONITOR)
		return asp_addr_monitor_irq_handler(irq, data);
#endif
	return soc_wtd_irq_handler(irq, data);
}

static unsigned int rdr_get_socdsp_watchdog_irq_num(void)
{
	unsigned int irq_num;
	struct device_node *dev_node = NULL;

	dev_node = of_find_compatible_node(NULL, NULL, WATCHDOG_DTS_COMP_NAME);
	if (!dev_node) {
		AUDIO_LOGE("rdr: find device node socdsp watchdog by compatible failed");
		return 0;
	}

	irq_num = irq_of_parse_and_map(dev_node, 0);
	if (irq_num == 0) {
		AUDIO_LOGE("rdr: irq parse and map failed, irq: %u", irq_num);
		return 0;
	}

	return irq_num;
}

static int remap_watchdog_addr(void)
{
	g_soc_des.wtd_base_addr = ioremap(
		get_phy_addr(DRV_WATCHDOG_BASE_ADDR, PLT_WATCHDOG_MEM), DRV_WATCHDOG_SIZE);
	if (!g_soc_des.wtd_base_addr) {
		AUDIO_LOGE("rdr: remap watchdog base addr fail");
		return -ENOMEM;
	}

#ifdef CONFIG_AUDIO_SOC_SHARED_IRQ
	g_soc_des.intr_hub_ao_addr = ioremap(
		get_phy_addr(INTR_HUB_AO_BASE_ADDR, PLT_INT_HUB_AO_MEM), INTR_HUB_AO_SIZE);
	if (!g_soc_des.intr_hub_ao_addr) {
		AUDIO_LOGE("rdr: remap intr hub addr fail");
		iounmap(g_soc_des.wtd_base_addr);
		g_soc_des.wtd_base_addr = NULL;
		return -ENOMEM;
	}

	g_soc_des.actrl_base_addr = ioremap(
		get_phy_addr(ACTRL_BASE_ADDR, PLT_ACTRL_MEM), ACTRL_SIZE);
	if (!g_soc_des.actrl_base_addr) {
		AUDIO_LOGE("rdr: remap actrl base addr fail");
		iounmap(g_soc_des.wtd_base_addr);
		g_soc_des.wtd_base_addr = NULL;
		iounmap(g_soc_des.intr_hub_ao_addr);
		g_soc_des.intr_hub_ao_addr = NULL;
		return -ENOMEM;
	}
#endif
	return 0;
}

static void unremap_watchdog_addr(void)
{
	if (g_soc_des.wtd_base_addr) {
		iounmap(g_soc_des.wtd_base_addr);
		g_soc_des.wtd_base_addr = NULL;
	}

#ifdef CONFIG_AUDIO_SOC_SHARED_IRQ
	if (g_soc_des.intr_hub_ao_addr) {
		iounmap(g_soc_des.intr_hub_ao_addr);
		g_soc_des.intr_hub_ao_addr = NULL;
	}

	if (g_soc_des.actrl_base_addr) {
		iounmap(g_soc_des.actrl_base_addr);
		g_soc_des.actrl_base_addr = NULL;
	}
#endif
}

static int create_rdr_soc_thread(void)
{
	g_soc_des.kdump_task = kthread_run(soc_dump_thread, NULL, "rdr_audio_soc_thread");
	if (!g_soc_des.kdump_task) {
		AUDIO_LOGE("create rdr soc dump thead fail");
		return -EBUSY;
	}

	g_soc_des.khandler_task = kthread_run(irq_handler_thread, NULL, "rdr_audio_soc_wtd_irq_handler_thread");
	if (!g_soc_des.khandler_task) {
		AUDIO_LOGE("create rdr soc wtd irq handler thead fail");
		goto create_khandler_task_fail;
	}

	return 0;

create_khandler_task_fail:
	if (g_soc_des.kdump_task) {
		kthread_stop(g_soc_des.kdump_task);
		up(&g_soc_des.dump_sem);
		g_soc_des.kdump_task = NULL;
	}

	return -EBUSY;
}

static void destroy_rdr_soc_thread(void)
{
	if (g_soc_des.kdump_task) {
		kthread_stop(g_soc_des.kdump_task);
		up(&g_soc_des.dump_sem);
		g_soc_des.kdump_task = NULL;
	}

	if (g_soc_des.khandler_task) {
		kthread_stop(g_soc_des.khandler_task);
		up(&g_soc_des.handler_sem);
		g_soc_des.khandler_task = NULL;
	}
}

static void priv_data_init(void)
{
	g_soc_des.wdt_irq_num = 0;
	g_soc_des.wtd_base_addr = NULL;
#ifdef CONFIG_AUDIO_SOC_SHARED_IRQ
	g_soc_des.intr_hub_ao_addr = NULL;
	g_soc_des.actrl_base_addr = NULL;
#endif

	sema_init(&g_soc_des.dump_sem, 0);
	sema_init(&g_soc_des.handler_sem, 0);
	g_soc_des.kdump_task = NULL;
	g_soc_des.khandler_task = NULL;
}

int rdr_audio_soc_init(void)
{
	int ret;

	AUDIO_LOGI("enter blackbox");

	priv_data_init();

	ret = remap_watchdog_addr();
	if (ret != 0)
		return ret;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,4,0))
	g_soc_des.rdr_wl = wakeup_source_register(NULL, "rdr_sochifi");
#else
	g_soc_des.rdr_wl = wakeup_source_register("rdr_sochifi");
#endif
	if (g_soc_des.rdr_wl == NULL) {
		AUDIO_LOGE("request wakeup source failed");
		ret = -ENOMEM;
		goto wakeup_source_register_err;
	}

	ret = create_rdr_soc_thread();
	if (ret != 0)
		goto create_soc_thread_fail;

	g_soc_des.wdt_irq_num = rdr_get_socdsp_watchdog_irq_num();
	if (g_soc_des.wdt_irq_num == 0) {
		AUDIO_LOGE("rdr: get socdsp watchdog irq num fail");
		goto get_irq_num_fail;
	}

#ifdef CONFIG_AUDIO_SOC_SHARED_IRQ
	mask_asp_addr_monitor_irq();
#endif

	ret = request_irq(g_soc_des.wdt_irq_num, soc_irq_handler, 0, "soc wdt handler", NULL);
	if (ret != 0) {
		AUDIO_LOGE("request irq soc wdt irq handler failed return 0x%x", ret);
		goto request_irp_fail;
	}

#ifdef HIFI_RESET_CDEV
	ret = dsp_reset_dev_init();
	if (ret != 0) {
		AUDIO_LOGE("dsp reset dev init failed return 0x%x", ret);
		goto dev_init_error;
	}
#endif

	AUDIO_LOGI("exit  blackbox");

	return ret;

#ifdef HIFI_RESET_CDEV
dev_init_error:
	if (g_soc_des.wdt_irq_num > 0)
		free_irq(g_soc_des.wdt_irq_num, NULL);
#endif

request_irp_fail:
get_irq_num_fail:
	destroy_rdr_soc_thread();
create_soc_thread_fail:
	wakeup_source_unregister(g_soc_des.rdr_wl);
	g_soc_des.rdr_wl = NULL;
wakeup_source_register_err:
	unremap_watchdog_addr();

	AUDIO_LOGI("exit  blackbox");

	return ret;
}

void rdr_audio_soc_exit(void)
{
	AUDIO_LOGI("enter blackbox");


	if (g_soc_des.wdt_irq_num > 0)
		free_irq(g_soc_des.wdt_irq_num, NULL);

	destroy_rdr_soc_thread();

	wakeup_source_unregister(g_soc_des.rdr_wl);
	g_soc_des.rdr_wl = NULL;

	unremap_watchdog_addr();

	AUDIO_LOGI("exit  blackbox");
}
/*lint +e446*/

#ifndef CONFIG_HIFI_SECOS_LOAD
static int audio_socdsp_cma_probe(struct platform_device *pdev)
{
	struct audio_dsp_cma_struct *dev = (struct audio_dsp_cma_struct *)&g_dspcma_dev;
	int ret;

	memset(dev, 0, sizeof(*dev));
	dev->device = &(pdev->dev);

	ret = of_reserved_mem_device_init(dev->device);
	if (ret != 0) {
		dev->device = NULL;
		AUDIO_LOGE("socdsp cma device init failed return 0x%x", ret);
	}

	return ret;
}

static int audio_socdsp_cma_remove(struct platform_device *pdev)
{
	struct audio_dsp_cma_struct *dev = (struct audio_dsp_cma_struct *)&g_dspcma_dev;

	of_reserved_mem_device_release(dev->device);
	memset(dev, 0, sizeof(*dev));

	return 0;
}

static const struct of_device_id audio_socdsp_cma_of_match[] = {
	{
		.compatible = DTS_COMP_HIFICMA_NAME,
		.data = NULL,
	},
	{},
};
MODULE_DEVICE_TABLE(of, audio_socdsp_cma_of_match);

static struct platform_driver audio_socdsp_cma_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "hifi-cma",
		.of_match_table = of_match_ptr(audio_socdsp_cma_of_match),
	},
	.probe = audio_socdsp_cma_probe,
	.remove = audio_socdsp_cma_remove,
};

static int __init audio_socdsp_cma_init(void)
{
	return platform_driver_register(&audio_socdsp_cma_driver);
}
subsys_initcall(audio_socdsp_cma_init);

static void __exit audio_socdsp_cma_exit(void)
{
	platform_driver_unregister(&audio_socdsp_cma_driver);
}
module_exit(audio_socdsp_cma_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("hifi reset cma module");
#endif

