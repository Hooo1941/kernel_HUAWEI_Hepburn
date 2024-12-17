/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name    segment_adapter.c
 * Description:
 *
 * Date         2020-04-17 16:28:10
 ********************************************************************/
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/of.h>
#include <linux/regulator/consumer.h>
#include <linux/clk.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/of_irq.h>
#include <linux/iommu.h>
#include <linux/pm_wakeup.h>
#include <linux/ion.h>
#include <linux/ion/mm_ion.h>
#include <linux/genalloc.h>
#include <linux/iommu/mm_iommu.h>
#include <linux/version.h>
#include <platform_include/isp/linux/hipp_common.h>
#include <linux/delay.h>
#include <linux/types.h>
#include "ipp.h"
#include "memory.h"
#include "ipp_core.h"
#include "cmdlst_manager.h"
#include "segment_common.h"
#include "segment_ipp_path.h"
#include "segment_ipp_adapter.h"
#include "segment_ipp_adapter_sub.h"

// debug
#include "ipp_top_drv_priv.h"
#include "ipp_top_reg_offset.h"
#include "cmdlst_reg_offset.h"
#include "cvdr_ipp_reg_offset.h"
#include "orb_enh_reg_offset.h"
#include "arfeature_reg_offset.h"
#include "reorder_reg_offset.h"
#include "compare_reg_offset.h"
#include "mc_reg_offset.h"
#include "klt_reg_offset.h"
#include "cmdlst_manager.h"

// Updated -r02
#define IPPCORE_SWID_INDEX0  0
#define IPPCORE_SWID_LEN0    3
#define IPPCORE_SWID_INDEX1  11
#define IPPCORE_SWID_LEN1    29
#define B_IPP_CVDR__4        (0xFF750000)
#define IB_QOS_OSTD0_CTRL    (0x150)

#ifdef CONFIG_IPP_DEBUG
#define HISP_CPE_TIMEOUT_MS (5000)
#else
#define HISP_CPE_TIMEOUT_MS 200
#endif

typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    rsv_0                  : 8  ; /* [7:0] */
		unsigned int    du_threshold_reached   : 8  ; /* [15:8] */
		unsigned int    max_axiread_id         : 6  ; /* [21:16] */
		unsigned int    axi_wr_buffer_behavior : 2  ; /* [23:22] */
		unsigned int    max_axiwrite_id        : 6  ; /* [29:24] */
		unsigned int    rsv_1                  : 2  ; /* [31:30] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;
} u_axi_jpeg_cvdr_cfg;

struct memory_block_s {
	int shared_fd;
	int size;
	unsigned long prot;
	unsigned int da;
	int usage;
	void *viraddr;
};

struct map_buff_block_s {
	unsigned int work_module; // new add r02
	int32_t     secure;
	int32_t     shared_fd;
	unsigned int     size;
	int   prot;
};

struct map_algo_buff_block_s {
	unsigned int work_module; // new add r02
	unsigned int instance_id;
	int32_t     shared_fd;
	unsigned int     size;
	int   prot;
};

hipp_adapter_s *g_hipp_adapter;

int hispcpe_wait(hipp_adapter_s *dev, cmdlst_channel_id_e wait_channel)
{
	long ret  = 0;
	int times = 0;
	unsigned long timeout;

	if (dev == NULL) {
		loge("Failed: dev is NULL");
		return ISP_IPP_ERR;
	}

	timeout = msecs_to_jiffies(HISP_CPE_TIMEOUT_MS);
	while (1) {
		ret = wait_event_interruptible_timeout(dev->ch_wq_head[wait_channel], dev->ch_ready[wait_channel], (long)timeout);
		if ((ret == -ERESTARTSYS) && (times++ < 200))
			msleep(5);
		else
			break;
	}

	if (ret <= 0) {
		loge("Failed :wait_channel.%d, ret.%ld, times.%d", wait_channel, ret, times);
		return -EINVAL;
	}

	logi("wait_channel.%d, ret.%ld, times.%d", wait_channel, ret, times);
	return ISP_IPP_OK;
}

irqreturn_t hispcpe_irq_handler(int irq, void *devid)
{
	unsigned int i = 0;
	hipp_adapter_s *dev = NULL;
	unsigned int value = 0;
	void __iomem *crg_base;
	dev = (hipp_adapter_s *)devid;

	if (dev != g_hipp_adapter) {
		loge("Failed : dev.%pK", dev);
		return IRQ_NONE;
	}

	crg_base = hipp_get_regaddr(CPE_TOP);

	if (irq == dev->irq) {
		value = readl(crg_base + IPP_TOP_CMDLST_IRQ_STATE_RAW_0_REG);
		value = readl(crg_base + IPP_TOP_CMDLST_IRQ_STATE_RAW_0_REG);
		value &= 0xFFFF;
		logi("IRQ VALUE = 0x%08x", value);

		// Clear irq; We only clear the interrupt signal that we currently have.
		writel(value, crg_base + IPP_TOP_CMDLST_IRQ_CLR_0_REG);
		// Search for all channels because interrupts of more than one channel may be generated at this time.
		for (i = 0; i < CMDLST_CHANNEL_MAX; i++) {
			if (value & (0x1 << i)) {
				dev->ch_ready[i] = 1;
				wake_up_interruptible(&dev->ch_wq_head[i]); // Wakes up all threads in the wait_queue_head.
			}
		}
	}

	return IRQ_HANDLED;
}

int hipp_adapter_register_irq(int irq)
{
	int ret = ISP_IPP_OK;
	hipp_adapter_s *dev = g_hipp_adapter;

	if (dev == NULL) {
		loge("Failed : dev NULL");
		return -EINVAL;
	}

	ret = request_irq(irq, hispcpe_irq_handler, 0, "HIPP_ADAPTER_IRQ", (void *)dev);
	if (ret != 0) {
		loge("Failed: request_irq error, ret.%d", ret);
		return ISP_IPP_ERR;
	}

	dev->irq = irq;
	logi("request irq = %d", irq); // debug

	return ISP_IPP_OK;
}

int hipp_adapter_unregister_irq(int irq)
{
	hipp_adapter_s *dev = g_hipp_adapter;
	if (dev == NULL) {
		loge("Failed : dev NULL");
		return -EINVAL;
	}

	free_irq(irq, (void *)dev);
	dev->irq = 0;

	return ISP_IPP_OK;
}

static int hipp_adapter_init_ipp_top(void)
{
	int i = 0;
	unsigned int irq_clr = 0xFFFF; // 16 channel
	unsigned int ch_cfg_offset = 0x80;
	cmdlst_ch_cfg_t ch_cfg = {0};

	logd("+");
	ch_cfg.u32 = 0;
	if (hispcpe_reg_set(CPE_TOP, IPP_TOP_ENH_VPB_CFG_REG, 0x1)) { // bit1:to arfeature bit0:to CVDR
		loge("Failed: set ENH_VPB_CFG failed");
		return ISP_IPP_ERR;
	}

	if (hispcpe_reg_set(CPE_TOP, IPP_TOP_CMDLST_IRQ_CLR_0_REG, (0xFFFF & irq_clr))) {
		loge("Failed: set CMDLST_IRQ_CLR failed");
		return ISP_IPP_ERR;
	}

	if (hispcpe_reg_set(CPE_TOP, IPP_TOP_CMDLST_IRQ_MSK_0_REG, (0xFFFF & ~irq_clr))) {
		loge("Failed: set CMDLST_IRQ_MSK failed");
		return ISP_IPP_ERR;
	}

	for (i = 0; i < CMDLST_CHANNEL_MAX; i++)
		if (hispcpe_reg_set(CMDLIST_REG, CMDLST_CH_CFG_0_REG + i * ch_cfg_offset, ch_cfg.u32)) {
			loge("Failed: set CMDLST_CH_CFG_0 failed");
			return ISP_IPP_ERR;
		}

	hipp_orcm_clk_enable();
	logd("-");

	return ISP_IPP_OK;
}

static int hipp_adapter_uninit_ipp_top(void)
{
	if (hipp_orcm_clk_disable()) {
		loge("Failed: do hipp_orcm_clk_disable error");
		return ISP_IPP_ERR;
	}

	return ISP_IPP_OK;
}

unsigned int get_cpe_addr_da(unsigned int work_module)
{
	hipp_adapter_s *dev = g_hipp_adapter;

	if (dev == NULL) {
		loge("Failed: NONE cpe_mem_info!");
		return 0;
	}

	return dev->daddr[work_module];
}

void *get_cpe_addr_va(unsigned int work_module)
{
	hipp_adapter_s *dev = g_hipp_adapter;

	if (dev == NULL) {
		loge("Failed: NONE cpe_mem_info!");
		return NULL;
	}

	return dev->virt_addr[work_module];
}

void *get_cpe_addr_sw_va(unsigned int work_module)
{
	hipp_adapter_s *dev = g_hipp_adapter;

	if (dev == NULL) {
		loge("Failed: NONE cpe_mem_info!");
		return NULL;
	}

	return dev->sw_virt_addr[work_module];
}

unsigned long get_cpe_iova_size(unsigned int work_module)
{
	hipp_adapter_s *dev = g_hipp_adapter;

	if (dev == NULL) {
		loge("Failed: NONE cpe_mem_info!");
		return 0;
	}

	return dev->iova_size[work_module];
}

static int dataflow_cvdr_global_config(void)
{
	u_axi_jpeg_cvdr_cfg reg_val;
	void __iomem *crg_base;
	crg_base = hipp_get_regaddr(CVDR_REG);
	if (crg_base == NULL) {
		loge("Failed : hipp_get_regaddr");
		return -EINVAL;
	}

	reg_val.u32 = 0;
	reg_val.bits.du_threshold_reached   = 32;
	reg_val.bits.max_axiread_id         = 63;
	reg_val.bits.axi_wr_buffer_behavior = 0;
	reg_val.bits.max_axiwrite_id        = 31;

	logd("reg_val.u32 = 0x%x", reg_val.u32); // 0x1f3f2000 is ok for EMU test.
	writel(reg_val.u32, crg_base + CVDR_IPP_CVDR_IPP_CVDR_CFG_REG);
	return ISP_IPP_OK;
}

/**
 * @Name: ipp_cfg_qos_reg
 * @Description:init flux ctrl info;
 * Expected configuration.
 * JPG_FLUX_CTRL0_0: When the value of flux_ctrl is 1, the maximum bandwidth of channels 1/16 or 1/32 is limited.
 *                   When the value of flux_ctrl is 0, the maximum bandwidth of channels 1/16 or 32 is not limited.
 * JPG_FLUX_CTRL0_1: The traffic limiting mode is the traffic threshold flow control mode.
 * JPG_FLUX_CTRL1_0: The lower threshold of the current limit is 0, and the upper threshold of the current limit is
 *                   1024 bytes.
 * JPG_FLUX_CTRL1_1: When flux_ctrl is 1, the JPEG read rate limit is set to 1/16 of the maximum channel bandwidth, and
 *                   the write rate limit is set to 1/32 of the maximum channel bandwidth.
 **/
static int ipp_cfg_qos_reg(void)
{
	unsigned int ipp_qos_value = 0;
	void __iomem *crg_base;
	crg_base = hipp_get_regaddr(CPE_TOP);
	if (crg_base == NULL) {
		loge("Failed : hipp_get_regaddr");
		return -EINVAL;
	}

	writel(0x00000000, crg_base + IPP_TOP_JPG_FLUX_CTRL0_0_REG);
	writel(0x08000105, crg_base + IPP_TOP_JPG_FLUX_CTRL0_1_REG);
	writel(0x00000000, crg_base + IPP_TOP_JPG_FLUX_CTRL1_0_REG);
	writel(0x08000085, crg_base + IPP_TOP_JPG_FLUX_CTRL1_1_REG);
	ipp_qos_value = readl(crg_base + IPP_TOP_JPG_FLUX_CTRL0_0_REG);
	logi("JPG_FLUX_CTRL0_0 = 0x%x (0x0 wanted)", ipp_qos_value);
	ipp_qos_value = readl(crg_base + IPP_TOP_JPG_FLUX_CTRL0_1_REG);
	logi("JPG_FLUX_CTRL0_1 = 0x%x (0x08000105 wanted)", ipp_qos_value);
	ipp_qos_value = readl(crg_base + IPP_TOP_JPG_FLUX_CTRL1_0_REG);
	logi("JPG_FLUX_CTRL1_0 = 0x%x (0x0 wanted)", ipp_qos_value);
	ipp_qos_value = readl(crg_base + IPP_TOP_JPG_FLUX_CTRL1_1_REG);
	logi("JPG_FLUX_CTRL1_1 = 0x%x (0x08000085 wanted)", ipp_qos_value);
	return ISP_IPP_OK;
}

static int hipp_adapter_cfg_qos_cvdr(void)
{
	int ret;
	ret = dataflow_cvdr_global_config();
	if (ret < 0) {
		loge("Failed: dataflow_cvdr_global_config.%d", ret);
		return ret;
	}

	ret = ipp_cfg_qos_reg();
	if (ret < 0) {
		loge("Failed : ipp_cfg_qos_reg");
		return ret;
	}

	return ISP_IPP_OK;
}

int hipp_special_cfg(void)
{
	int ret;

	ret = hipp_adapter_cfg_qos_cvdr();
	if (ret < 0) {
		pr_err("[%s] Failed : hipp_adapter_cfg_qos_cvdr.%d\n", __func__, ret);
		return ret;
	}

	return 0;
}

static int para_check_hipp_powerup(hipp_adapter_s *dev)
{
	if (dev == NULL) {
		loge("Failed : dev NULL");
		return -EINVAL;
	}

	if (dev->ippdrv == NULL) {
		loge("Failed : dev->ippdrv NULL");
		return -EINVAL;
	}

	if (dev->ippdrv->power_up == NULL) {
		loge("Failed : dev->ippdrv->power_up.NULL");
		return -EINVAL;
	}

	if (dev->ippdrv->enable_smmu == NULL) {
		loge("Failed : ippdrv->enable_smmu.NULL");
		return -EINVAL;
	}

	if (dev->ippdrv->disable_smmu == NULL) {
		loge("Failed : ippdrv->disable_smmu.NULL");
		return -EINVAL;
	}

	if (dev->ippdrv->setsid_smmu == NULL) {
		loge("Failed : ippdrv->setsid_smmu.NULL");
		return -EINVAL;
	}

	return ISP_IPP_OK;
}

static int hipp_setsid_smmu(hipp_adapter_s *dev, int sid, int ssid)
{
	int ret = 0;
	ret = dev->ippdrv->setsid_smmu(dev->ippdrv, IPPCORE_SWID_INDEX0, IPPCORE_SWID_LEN0, sid, ssid);
	if (ret != 0) {
		loge("Failed : dev->ippdrv->enable_smmu.%d", ret);
		return ret;
	}

	ret = dev->ippdrv->setsid_smmu(dev->ippdrv, IPPCORE_SWID_INDEX1, IPPCORE_SWID_LEN1, sid, ssid);
	if (ret != 0) {
		loge("Failed : dev->ippdrv->enable_smmu.%d", ret);
		return ret;
	}

	return ISP_IPP_OK;
}

static int hipp_set_pref(hipp_adapter_s *dev)
{
	int ret = 0;
	ret = dev->ippdrv->set_pref(dev->ippdrv, IPPCORE_SWID_INDEX0, IPPCORE_SWID_LEN0);
	if (ret != 0) {
		loge("Failed : dev->ippdrv->enable_smmu.%d", ret);
		return ret;
	}

	ret = dev->ippdrv->set_pref(dev->ippdrv, IPPCORE_SWID_INDEX1, IPPCORE_SWID_LEN1);
	if (ret != 0) {
		loge("Failed : dev->ippdrv->enable_smmu.%d", ret);
		return ret;
	}

	return ISP_IPP_OK;
}

int hipp_powerup(void)
{
	int ret;
	hipp_adapter_s *dev = g_hipp_adapter;
	int sid;
	int ssid;

	logd("+");
	ret = (para_check_hipp_powerup(dev));
	if (ret != 0) {
		loge("Failed : para_check_hipp_powerup");
		return ret;
	}

	ret = dev->ippdrv->power_up(dev->ippdrv);
	if (ret != 0) {
		loge("Failed : dev->ippdrv->power_up.%d", ret);
		return ret;
	}

	if (dev->ippdrv->enable_smmu(dev->ippdrv)) {
		loge("Failed : dev->ippdrv->enable_smmu");
		goto fail_power_down;
	}

	if (get_hipp_smmu_info(&sid, &ssid)) {
		loge("Failed : get_hipp_smmu_info");
		goto fail_disable_smmu;
	}

	if (hipp_setsid_smmu(dev, sid, ssid)) {
		loge("Failed : hipp_setsid_smmu");
		goto fail_disable_smmu;
	}

	if (hipp_set_pref(dev)) {
		loge("Failed : hipp_set_pref");
		goto fail_disable_smmu;
	}

	if (hipp_adapter_init_ipp_top()) {
		loge("Failed : hipp_adapter_init_ipp_top");
		goto fail_disable_smmu;
	}
	logd("-");
	return ISP_IPP_OK;

fail_disable_smmu:
	if (dev->ippdrv->disable_smmu(dev->ippdrv))
		loge("Failed : disable_smmu");
fail_power_down:
	if (dev->ippdrv->power_dn(dev->ippdrv))
		loge("Failed : power_dn");

	logd("-");
	return ISP_IPP_ERR;
}

/**
@name: hispcpe_work_check
@description: Check if the hispcpe device is still working;
**/
int hispcpe_work_check(void)
{
	hipp_adapter_s *dev = g_hipp_adapter;

	if (dev == NULL) {
		loge("Failed : dev.%pK", dev);
		return -EINVAL;
	}

	mutex_lock(&dev->hipp_refs_lock);
	if (dev->hipp_refs[REFS_TYP_EARCMK] > 0) {
		loge("Failed: type.REFS_TYP_EARCMK Opened");
		mutex_unlock(&dev->hipp_refs_lock);
		return -ENOMEM;
	}
	mutex_unlock(&dev->hipp_refs_lock);

	return ISP_IPP_OK;
}

int hipp_powerdn(void)
{
	int ret;
	hipp_adapter_s *dev = g_hipp_adapter;

	logd("+");
	ret = hipp_adapter_uninit_ipp_top();
	if (ret != 0)
		loge("Failed : hipp_adapter_uninit_ipp_top.%d", ret);

	if (dev == NULL) {
		loge("Failed : dev.%pK", dev);
		return -EINVAL;
	}

	if (dev->ippdrv == NULL) {
		loge("Failed : ippdrv.%pK", dev->ippdrv);
		return -ENOMEM;
	}

	if (dev->ippdrv->disable_smmu == NULL) {
		loge("Failed : ippdrv->disable_smmu.%pK", dev->ippdrv->disable_smmu);
		return -ENOMEM;
	}

	ret = dev->ippdrv->disable_smmu(dev->ippdrv);
	if (ret != 0)
		loge("Failed : disable_smmu.%d", ret);

	ret = dev->ippdrv->power_dn(dev->ippdrv);
	if (ret != 0)
		loge("Failed : power_dn.%d", ret);

	logd("-");

	return ISP_IPP_OK;
}

static int hipp_orcm_clean_wait_flag(hipp_adapter_s *dev, work_module_e module)
{
	switch (module) {
	case ENH_ONLY:
	case ARFEATURE_ONLY:
	case ENH_ARF:
		dev->ch_ready[g_channel_map[module].enh_arf_channel] = 0;
		break;
	case ARF_MATCHER:
	case ENH_ARF_MATCHER:
		dev->ch_ready[g_channel_map[module].enh_arf_channel] = 0;
		dev->ch_ready[g_channel_map[module].rdr_channel] = 0;
		dev->ch_ready[g_channel_map[module].cmp_channel] = 0;
		break;
	case MATCHER_MC:
		dev->ch_ready[g_channel_map[module].rdr_channel] = 0;
		dev->ch_ready[g_channel_map[module].cmp_channel] = 0;
		dev->ch_ready[g_channel_map[module].mc_channel]  = 0;
		break;
	case ARF_MATCHER_MC:
	case ENH_ARF_MATCHER_MC:
		dev->ch_ready[g_channel_map[module].enh_arf_channel] = 0;
		dev->ch_ready[g_channel_map[module].rdr_channel] = 0;
		dev->ch_ready[g_channel_map[module].cmp_channel] = 0;
		dev->ch_ready[g_channel_map[module].mc_channel]  = 0;
		break;
	case MATCHER_ONLY:
		dev->ch_ready[g_channel_map[module].rdr_channel] = 0;
		dev->ch_ready[g_channel_map[module].cmp_channel] = 0;
		break;
	case MC_ONLY:
		dev->ch_ready[g_channel_map[module].mc_channel]  = 0;
		break;
	case KLT_ONLY:
		dev->ch_ready[g_channel_map[module].klt_channel] = 0;
		break;
	case ENH_ARF_KLT:
		dev->ch_ready[g_channel_map[module].enh_arf_channel] = 0;
		dev->ch_ready[g_channel_map[module].klt_channel] = 0;
		break;
	default:
		loge("Failed : Invilid work_module = %d", module);
		return ISP_IPP_ERR;
	}

	return ISP_IPP_OK;
}

static int hipp_orcm_enh_arf_klt_wait(hipp_adapter_s *dev, struct msg_req_ipp_cmd_req_t *req)
{
	work_module_e module = req->work_module;
	int ret0 = 0, ret1 = 0;

	unsigned int enh_enable = req->req_enh_arf.req_enh_cur.reg_cfg.enable_cfg.bits.enh_en;
	unsigned int arf_enable = req->req_enh_arf.req_arf_cur.reg_cfg[0].top_cfg.bits.ar_feature_en;
	unsigned int klt_enable = req->req_klt.reg_cfg.klt_klt_cfg.klt_start.bits.start_klt;

	if (enh_enable == 1 || arf_enable == 1)
		ret0 = hispcpe_wait(dev, g_channel_map[module].enh_arf_channel);
	if (req->req_klt.work_mode != KLT_OFF && klt_enable == 1)
		ret1 = hispcpe_wait(dev, g_channel_map[module].klt_channel);

	if (ret0 < 0 || ret1 < 0 ) {
		loge("work_module.%d: ret0.%d, ret1.%d", module, ret0, ret1);
		return ISP_IPP_ERR;
	}

	return ISP_IPP_OK;
}

static int hipp_orcm_wait(hipp_adapter_s *dev, struct msg_req_ipp_cmd_req_t *req)
{
	int ret0 = 0, ret1 = 0, ret2 = 0, ret3 = 0;
	work_module_e module = req->work_module;
	logi("work_module = %d", module);

	switch (module) {
	case ENH_ONLY:
	case ARFEATURE_ONLY:
	case ENH_ARF:
		ret0 = hispcpe_wait(dev, g_channel_map[module].enh_arf_channel);
		break;
	case MATCHER_ONLY:
		if (req->req_matcher.rdr_pyramid_layer == 0 && req->req_matcher.cmp_pyramid_layer == 0) {
			loge("Failed: rdr and cmp's pyramid_layer are 0");
			return ISP_IPP_ERR;
		}
		if (req->req_matcher.rdr_pyramid_layer != 0)
			ret0 = hispcpe_wait(dev, g_channel_map[module].rdr_channel);
		if (req->req_matcher.cmp_pyramid_layer != 0)
			ret1 = hispcpe_wait(dev, g_channel_map[module].cmp_channel);
		break;
	case MC_ONLY:
		ret0 = hispcpe_wait(dev, g_channel_map[module].mc_channel);
		break;
	case ARF_MATCHER:
	case ENH_ARF_MATCHER:
		ret0 = hispcpe_wait(dev, g_channel_map[module].enh_arf_channel);
		ret1 = hispcpe_wait(dev, g_channel_map[module].rdr_channel);
		ret2 = hispcpe_wait(dev, g_channel_map[module].cmp_channel);
		break;
	case MATCHER_MC:
		ret0 = hispcpe_wait(dev, g_channel_map[module].rdr_channel);
		ret1 = hispcpe_wait(dev, g_channel_map[module].cmp_channel);
		ret2 = hispcpe_wait(dev, g_channel_map[module].mc_channel);
		break;
	case ARF_MATCHER_MC:
	case ENH_ARF_MATCHER_MC:
		ret0 = hispcpe_wait(dev, g_channel_map[module].enh_arf_channel);
		ret1 = hispcpe_wait(dev, g_channel_map[module].rdr_channel);
		ret2 = hispcpe_wait(dev, g_channel_map[module].cmp_channel);
		ret3 = hispcpe_wait(dev, g_channel_map[module].mc_channel);
		break;
	case KLT_ONLY:
		ret0 = hispcpe_wait(dev, g_channel_map[module].klt_channel);
		break;
	case ENH_ARF_KLT:
		ret0 = hipp_orcm_enh_arf_klt_wait(dev, req);
		break;
	default:
		loge("Failed : Invilid work_module = %d", module);
		return ISP_IPP_ERR;
	}

	if (ret0 < 0 || ret1 < 0 || ret2 < 0 || ret3 < 0) {
		loge("work_module.%d: ret0.%d, ret1.%d, ret2.%d, ret3.%d", module, ret0, ret1, ret2, ret3);
		return ISP_IPP_ERR;
	}

	return ISP_IPP_OK;
}

static int hipp_orcm_eop_handle(hipp_adapter_s *dev, struct msg_req_ipp_cmd_req_t *req)
{
	work_module_e work_module = req->work_module;

	if (ipp_eop_handler(work_module)) {
		loge("do ipp_eop_handler failed, work_module = %d", work_module);
		return ISP_IPP_ERR;
	}

	switch (work_module) {
	case ENH_ONLY:
		dequeue_channel_ready_list(g_channel_map[ENH_ONLY].enh_arf_channel);
		break;
	case ARFEATURE_ONLY:
		dequeue_channel_ready_list(g_channel_map[ARFEATURE_ONLY].enh_arf_channel);
		break;
	case MATCHER_ONLY:
		dequeue_channel_ready_list(g_channel_map[MATCHER_ONLY].rdr_channel);
		dequeue_channel_ready_list(g_channel_map[MATCHER_ONLY].cmp_channel);
		break;
	case MC_ONLY:
		dequeue_channel_ready_list(g_channel_map[MC_ONLY].mc_channel);
		break;
	case ENH_ARF:
		dequeue_channel_ready_list(g_channel_map[ENH_ARF].enh_arf_channel);
		break;
	case ARF_MATCHER:
		dequeue_channel_ready_list(g_channel_map[ARF_MATCHER].enh_arf_channel);
		dequeue_channel_ready_list(g_channel_map[ARF_MATCHER].rdr_channel);
		dequeue_channel_ready_list(g_channel_map[ARF_MATCHER].cmp_channel);

		break;
	case ENH_ARF_MATCHER:
		dequeue_channel_ready_list(g_channel_map[ENH_ARF_MATCHER].enh_arf_channel);
		dequeue_channel_ready_list(g_channel_map[ENH_ARF_MATCHER].rdr_channel);
		dequeue_channel_ready_list(g_channel_map[ENH_ARF_MATCHER].cmp_channel);
		break;
	case MATCHER_MC:
		dequeue_channel_ready_list(g_channel_map[MATCHER_MC].rdr_channel);
		dequeue_channel_ready_list(g_channel_map[MATCHER_MC].cmp_channel);
		dequeue_channel_ready_list(g_channel_map[MATCHER_MC].mc_channel);
		break;
	case ARF_MATCHER_MC:
		dequeue_channel_ready_list(g_channel_map[ARF_MATCHER_MC].enh_arf_channel);
		dequeue_channel_ready_list(g_channel_map[ARF_MATCHER_MC].rdr_channel);
		dequeue_channel_ready_list(g_channel_map[ARF_MATCHER_MC].cmp_channel);
		dequeue_channel_ready_list(g_channel_map[ARF_MATCHER_MC].mc_channel);
		break;
	case ENH_ARF_MATCHER_MC:
		dequeue_channel_ready_list(g_channel_map[ENH_ARF_MATCHER_MC].enh_arf_channel);
		dequeue_channel_ready_list(g_channel_map[ENH_ARF_MATCHER_MC].rdr_channel);
		dequeue_channel_ready_list(g_channel_map[ENH_ARF_MATCHER_MC].cmp_channel);
		dequeue_channel_ready_list(g_channel_map[ENH_ARF_MATCHER_MC].mc_channel);
	case KLT_ONLY:
		dequeue_channel_ready_list(g_channel_map[KLT_ONLY].klt_channel);
		break;
	case ENH_ARF_KLT:
		dequeue_channel_ready_list(g_channel_map[ENH_ARF_KLT].enh_arf_channel);
		dequeue_channel_ready_list(g_channel_map[ENH_ARF_KLT].klt_channel);
		break;
	default:
		loge("Failed : Invilid work_module = %d", work_module);
		return ISP_IPP_ERR;
	}

	return ISP_IPP_OK;
}

// Add the function interface for releasing dma_buf. before respond req.
static int fd_to_iova(struct msg_req_ipp_cmd_req_t *req, ipp_stream_t *stream_info, struct list_head *fd_link_head)
{
	unsigned long iova, iova_size = 0;
	hipp_adapter_s *dev = g_hipp_adapter;
	struct dma_buf *modulebuf = NULL;
	dma_buf_link_t *pos = NULL;
	dma_buf_link_t *n = NULL;
	dma_buf_link_t *new_node = NULL;

	if (dev->algo_fd[req->work_module][req->instance_id] == stream_info->fd) {
		// Algo fd has been iommu_map, return 'iova+offset' directly.
		stream_info->buffer = (unsigned int)(dev->algo_daddr[req->work_module][req->instance_id]
			+ stream_info->offset);
		return ISP_IPP_OK;
	}

	list_for_each_entry_safe(pos, n, fd_link_head, fd_link) {
		if (pos->fd == stream_info->fd) {
			// If the current fd has been iommu_map, return 'iova+offset' directly.
			stream_info->buffer = (unsigned int)(pos->start_iova + stream_info->offset);
			logd("fd.%d has been iommu_map; buffer.0x%x, offset.0x%08x",
					pos->fd, stream_info->buffer, stream_info->offset);
			logd("-");
			return ISP_IPP_OK;
		}
	}

	if (dev->ippdrv->iommu_map == NULL) {
		loge("dev->ippdrv->iommu_map is NULL");
		return -EINVAL;
	}

	modulebuf = dma_buf_get(stream_info->fd);
	if (IS_ERR_OR_NULL(modulebuf)) {
		loge("Failed : dma_buf_get");
		return -EINVAL;
	}

	if (modulebuf->size < stream_info->total_size) {
		loge("Failed : size not equal, size.%zu, stream_info->total_size.%d", modulebuf->size, stream_info->total_size);
		dma_buf_put(modulebuf);
		return -EINVAL;
	}

	iova = dev->ippdrv->iommu_map(dev->ippdrv, modulebuf, IOMMU_READ | IOMMU_WRITE, &iova_size);
	if ((iova == 0) || (iova_size < stream_info->total_size)) {
		loge("Failed : prot.%d, size.%ld, stream_info->total_size.%d",
			stream_info->prot, iova_size, stream_info->total_size);
		dma_buf_put(modulebuf);
		return -EINVAL;
	}

	stream_info->buffer = (unsigned int)(iova + stream_info->offset);
	logd("fd.%d first do iommu_map; start_iova.0x%lx; buffer.0x%x, offset.0x%x",
		stream_info->fd, iova, stream_info->buffer, stream_info->offset);
	new_node = (dma_buf_link_t *)vmalloc(sizeof(dma_buf_link_t));
	if (new_node == NULL) {
		loge("Failed : alloc new_node failed");
		goto free_iommu_map;
	}

	new_node->fd = stream_info->fd;
	new_node->start_iova = iova;
	new_node->iova_size = iova_size;
	new_node->dmabuf = (void *)modulebuf;
	list_add(&new_node->fd_link, fd_link_head);
	logd("-");

	return ISP_IPP_OK;
free_iommu_map:
	if (dev->ippdrv->iommu_unmap != NULL) {
		if (dev->ippdrv->iommu_unmap(dev->ippdrv, modulebuf, iova))
			loge("Failed : iommu_unmap, sharedfd.%d, iova.0x%lx", stream_info->fd, iova);
	}
	dma_buf_put(modulebuf);
	logd("-");

	return -EINVAL;
}

static void get_enh_kernel_buff_addr(struct msg_req_ipp_cmd_req_t *req, msg_req_enh_t *msg_enh,
	struct list_head *fd_link_head)
{
	unsigned int i = 0;
	for (i = 0; i < ORB_ENH_STREAM_MAX; i++) {
		msg_enh->streams[i].buffer = 0; // must set inital val 0
		if (msg_enh->streams[i].fd >= 0
			&& msg_enh->streams[i].total_size <= MAP_ALGO_BUF_MAX_SIZE
			&& msg_enh->streams[i].offset < msg_enh->streams[i].total_size) {
			logd("ENH: start do streams[%d]:", i);
			fd_to_iova(req, &msg_enh->streams[i], fd_link_head);
		}
	}
	return;
}

static void get_arf_kernel_buff_addr(struct msg_req_ipp_cmd_req_t *req, msg_req_arf_t *msg_arf,
	struct list_head *fd_link_head)
{
	unsigned int i = 0, j = 0;

	for (i = 0; i < ARFEATURE_MAX_LAYER; i++) {
		for (j = 0; j < ARFEATURE_STREAM_MAX; j++) {
			msg_arf->streams[i][j].buffer = 0;
			if (msg_arf->streams[i][j].fd >= 0
				&& msg_arf->streams[i][j].total_size <= MAP_ALGO_BUF_MAX_SIZE
				&& msg_arf->streams[i][j].offset < msg_arf->streams[i][j].total_size) {
				logd("ARF: start do streams[%d][%d]:", i, j);
				fd_to_iova(req, &msg_arf->streams[i][j], fd_link_head);
			}
		}
	}

	for (i = 0; i < ARF_STAT_BUFF_MAX; i++) {
		msg_arf->arf_stat_buff[i].buffer = 0;
		if (msg_arf->arf_stat_buff[i].fd >= 0
			&& msg_arf->arf_stat_buff[i].total_size <= MAP_ALGO_BUF_MAX_SIZE
			&& msg_arf->arf_stat_buff[i].offset < msg_arf->arf_stat_buff[i].total_size) {
			logd("ARF: start do arf_stat_buff[%d]:", i);
			fd_to_iova(req, &msg_arf->arf_stat_buff[i], fd_link_head);
		}
	}

	return;
}

static void get_matcher_kernel_buff_addr(struct msg_req_ipp_cmd_req_t *req, msg_req_matcher_t *msg_matcher,
		struct list_head *fd_link_head)
{
	unsigned int i = 0, j = 0;

	for (i = 0; i < msg_matcher->rdr_pyramid_layer; i++) {
		for (j = 0; j < RDR_STREAM_MAX; j++) {
			msg_matcher->req_rdr.streams[i][j].buffer = 0;
			if (msg_matcher->req_rdr.streams[i][j].fd >= 0
				&& msg_matcher->req_rdr.streams[i][j].total_size <= MAP_ALGO_BUF_MAX_SIZE
				&& msg_matcher->req_rdr.streams[i][j].offset < msg_matcher->req_rdr.streams[i][j].total_size) {
				logd("RDR: start do streams[%d][%d]:", i, j);
				fd_to_iova(req, &msg_matcher->req_rdr.streams[i][j], fd_link_head);
			}
		}
	}

	for (i = 0; i < msg_matcher->cmp_pyramid_layer; i++) {
		for (j = 0; j < CMP_STREAM_MAX; j++) {
			msg_matcher->req_cmp.streams[i][j].buffer = 0;
			if (msg_matcher->req_cmp.streams[i][j].fd >= 0
				&& msg_matcher->req_cmp.streams[i][j].total_size <= MAP_ALGO_BUF_MAX_SIZE
				&& msg_matcher->req_cmp.streams[i][j].offset < msg_matcher->req_cmp.streams[i][j].total_size) {
				logd("CMP: start do streams[%d][%d]:", i, j);
				fd_to_iova(req, &msg_matcher->req_cmp.streams[i][j], fd_link_head);
			}
		}
	}
	return;
}

static void get_mc_kernel_buff_addr(struct msg_req_ipp_cmd_req_t *req, msg_req_mc_t *msg_mc,
	struct list_head *fd_link_head)
{
	unsigned int i = 0;

	for (i = 0; i < MC_STREAM_MAX; i++) {
		msg_mc->streams[i].buffer = 0;
		if (msg_mc->streams[i].fd >= 0
			&& msg_mc->streams[i].total_size <= MAP_ALGO_BUF_MAX_SIZE
			&& msg_mc->streams[i].offset < msg_mc->streams[i].total_size) {
			logd("MC: start do streams[%d]:", i);
			fd_to_iova(req, &msg_mc->streams[i], fd_link_head);
		}
	}

	for (i = 0; i < MC_STAT_BUFF_MAX; i++) {
		msg_mc->mc_stat_buff[i].buffer = 0;
		if (msg_mc->mc_stat_buff[i].fd >= 0
			&& msg_mc->mc_stat_buff[i].total_size <= MAP_ALGO_BUF_MAX_SIZE
			&& msg_mc->mc_stat_buff[i].offset < msg_mc->mc_stat_buff[i].total_size) {
			logd("MC: start do mc_stat_buff[%d]:", i);
			fd_to_iova(req, &msg_mc->mc_stat_buff[i], fd_link_head);
		}
	}
	return;
}

static void get_klt_kernel_buff_addr(struct msg_req_ipp_cmd_req_t *req, msg_req_klt_t *msg_klt,
	struct list_head *fd_link_head)
{
	unsigned int i = 0;

	for (i = 0; i < KLT_STREAM_MAX; i++) {
		msg_klt->streams[i].buffer = 0;
		if (msg_klt->streams[i].fd >= 0
			&& msg_klt->streams[i].total_size <= MAP_ALGO_BUF_MAX_SIZE
			&& msg_klt->streams[i].offset < msg_klt->streams[i].total_size) {
			logd("KLT: start do streams[%d]:", i);
			fd_to_iova(req, &msg_klt->streams[i], fd_link_head);
		}
	}

	for (i = 0; i < KLT_STAT_BUFF_MAX; i++) {
		msg_klt->klt_stat_buff[i].buffer = 0;
		if (msg_klt->klt_stat_buff[i].fd >= 0
			&& msg_klt->klt_stat_buff[i].total_size <= MAP_ALGO_BUF_MAX_SIZE
			&& msg_klt->klt_stat_buff[i].offset < msg_klt->klt_stat_buff[i].total_size) {
			logd("KLT: start do klt_stat_buff[%d]:", i);
			fd_to_iova(req, &msg_klt->klt_stat_buff[i], fd_link_head);
		}
	}

	return;
}

static int get_kernel_buff_check_req_para(struct msg_req_ipp_cmd_req_t *req) {
	msg_req_matcher_t *req_matcher = &req->req_matcher;

	if (req->work_module == MATCHER_ONLY ||
		req->work_module == ARF_MATCHER ||
		req->work_module == ENH_ARF_MATCHER ||
		req->work_module == MATCHER_MC ||
		req->work_module == ARF_MATCHER_MC ||
		req->work_module == ENH_ARF_MATCHER_MC) {
		if (req_matcher->rdr_pyramid_layer > MATCHER_LAYER_MAX ||
			req_matcher->cmp_pyramid_layer > MATCHER_LAYER_MAX) {
			loge("Failed: invalid para, rdr_pyramid_layer = %d, cmp_pyramid_layer = %d",
				req_matcher->rdr_pyramid_layer, req_matcher->cmp_pyramid_layer);
			return ISP_IPP_ERR;
		}
	}

	return ISP_IPP_OK;
}

/**
* Name: hispcpe_get_kernel_buff_addr
* Desc: Convert streams.fd to an address that can be accessed by the kernel side.
**/
static int hispcpe_get_kernel_buff_addr(struct msg_req_ipp_cmd_req_t *req, struct list_head *fd_link_head)
{
	if (get_kernel_buff_check_req_para(req))
		return ISP_IPP_ERR;

	switch (req->work_module) {
	case ENH_ONLY:
		get_enh_kernel_buff_addr(req, &req->req_enh_arf.req_enh_cur, fd_link_head);
		break;
	case ARFEATURE_ONLY:
		logd("ARFEATURE_ONLY: req_arf_cur");
		get_arf_kernel_buff_addr(req, &req->req_enh_arf.req_arf_cur, fd_link_head);
		if (req->req_enh_arf.work_frame == CUR_REF) {
			logd("ARFEATURE_ONLY: req_arf_ref");
			get_arf_kernel_buff_addr(req, &req->req_enh_arf.req_arf_ref, fd_link_head);
		}
		break;
	case MATCHER_ONLY:
		get_matcher_kernel_buff_addr(req, &req->req_matcher, fd_link_head);
		break;
	case MC_ONLY:
		get_mc_kernel_buff_addr(req, &req->req_mc, fd_link_head);
		break;
	case ENH_ARF:
		get_enh_kernel_buff_addr(req, &req->req_enh_arf.req_enh_cur, fd_link_head);
		get_arf_kernel_buff_addr(req, &req->req_enh_arf.req_arf_cur, fd_link_head);
		get_enh_kernel_buff_addr(req, &req->req_enh_arf.req_enh_ref, fd_link_head);
		get_arf_kernel_buff_addr(req, &req->req_enh_arf.req_arf_ref, fd_link_head);
		break;
	case ARF_MATCHER:
		get_arf_kernel_buff_addr(req, &req->req_enh_arf.req_arf_cur, fd_link_head);
		get_arf_kernel_buff_addr(req, &req->req_enh_arf.req_arf_ref, fd_link_head);
		get_matcher_kernel_buff_addr(req, &req->req_matcher, fd_link_head);
		break;
	case ENH_ARF_MATCHER:
		get_enh_kernel_buff_addr(req, &req->req_enh_arf.req_enh_cur, fd_link_head);
		get_arf_kernel_buff_addr(req, &req->req_enh_arf.req_arf_cur, fd_link_head);
		get_enh_kernel_buff_addr(req, &req->req_enh_arf.req_enh_ref, fd_link_head);
		get_arf_kernel_buff_addr(req, &req->req_enh_arf.req_arf_ref, fd_link_head);
		get_matcher_kernel_buff_addr(req, &req->req_matcher, fd_link_head);
		break;
	case MATCHER_MC:
		get_matcher_kernel_buff_addr(req, &req->req_matcher, fd_link_head);
		get_mc_kernel_buff_addr(req, &req->req_mc, fd_link_head);
		break;
	case ARF_MATCHER_MC:
		get_arf_kernel_buff_addr(req, &req->req_enh_arf.req_arf_cur, fd_link_head);
		get_arf_kernel_buff_addr(req, &req->req_enh_arf.req_arf_ref, fd_link_head);
		get_matcher_kernel_buff_addr(req, &req->req_matcher, fd_link_head);
		get_mc_kernel_buff_addr(req, &req->req_mc, fd_link_head);
		break;
	case ENH_ARF_MATCHER_MC:
		get_enh_kernel_buff_addr(req, &req->req_enh_arf.req_enh_cur, fd_link_head);
		get_enh_kernel_buff_addr(req, &req->req_enh_arf.req_enh_ref, fd_link_head);
		get_arf_kernel_buff_addr(req, &req->req_enh_arf.req_arf_cur, fd_link_head);
		get_arf_kernel_buff_addr(req, &req->req_enh_arf.req_arf_ref, fd_link_head);
		get_matcher_kernel_buff_addr(req, &req->req_matcher, fd_link_head);
		get_mc_kernel_buff_addr(req, &req->req_mc, fd_link_head);
		break;
	case KLT_ONLY:
		get_klt_kernel_buff_addr(req, &req->req_klt, fd_link_head);
		break;
	case ENH_ARF_KLT:
		get_enh_kernel_buff_addr(req, &req->req_enh_arf.req_enh_cur, fd_link_head);
		get_enh_kernel_buff_addr(req, &req->req_enh_arf.req_enh_ref, fd_link_head);
		get_arf_kernel_buff_addr(req, &req->req_enh_arf.req_arf_cur, fd_link_head);
		get_arf_kernel_buff_addr(req, &req->req_enh_arf.req_arf_ref, fd_link_head);
		get_klt_kernel_buff_addr(req, &req->req_klt, fd_link_head);
		break;
	default:
		loge("Failed : Invilid work_module = %d", req->work_module);
		return ISP_IPP_ERR;
	}
	return ISP_IPP_OK;
}

static int hispcpe_release_kernel_buff_addr(struct list_head *fd_link_head, unsigned int work_module)
{
	dma_buf_link_t *pos = NULL;
	dma_buf_link_t *n = NULL;
	hipp_adapter_s *dev = g_hipp_adapter;

	if (dev->ippdrv->iommu_unmap == NULL) {
		loge("iommu_unmap is NULL");
		return ISP_IPP_ERR;
	}

	list_for_each_entry_safe(pos, n, fd_link_head, fd_link) {
		if (pos != NULL && pos->dmabuf != NULL && pos->fd != 0) {
			logd("Cur note info: fd.%d, start_iova.0x%lx, iova_size.0x%lx", pos->fd, pos->start_iova, pos->iova_size);
			if (dev->ippdrv->iommu_unmap(dev->ippdrv, pos->dmabuf, pos->start_iova))
				loge("Failed: fd.%d, start_iova.0x%lx, iova_size.0x%lx", pos->fd, pos->start_iova, pos->iova_size);

			dma_buf_put(pos->dmabuf);
			list_del(&pos->fd_link);
			vfree(pos);
		} else {
			loge("Failed: The wrong list_node is added to the fd_link_head: pos->dmabuf.0x%pK, pos->fd.%d ",
				pos->dmabuf, pos->fd);
		}
	}

	return ISP_IPP_OK;
}

static void dump_cmdlst_buff(struct msg_req_ipp_cmd_req_t *req)
{
	unsigned long long va = 0;
	unsigned int da = 0;
	unsigned int i;
	int ret = 0;
	cpe_mem_free(MEM_ID_CMDLST_PARA_REORDER, req->work_module);
	cpe_mem_free(MEM_ID_CMDLST_PARA_COMPARE, req->work_module);
	ret = cpe_mem_get(MEM_ID_CMDLST_PARA_REORDER, req->work_module, &va, &da);
	if (ret != 0) {
		loge("Failed : cpe_mem_get %d", MEM_ID_CMDLST_PARA_REORDER);
		return;
	}
	cmdlst_para_t *cmp_cmdlst_para = (cmdlst_para_t *)(uintptr_t)va;
	schedule_cmdlst_link_t *new_entry = (schedule_cmdlst_link_t *)cmp_cmdlst_para->cmd_entry;
	loge("RDR CMDLST BUFF:");
	for (i = 0; i < cmp_cmdlst_para->stripe_cnt; ++i)
		cmdlst_buff_dump(&(new_entry[i].cmd_buf));

	ret = cpe_mem_get(MEM_ID_CMDLST_PARA_COMPARE, req->work_module, &va, &da);
	if (ret != 0) {
		loge("Failed : cpe_mem_get %d", MEM_ID_CMDLST_PARA_COMPARE);
		return;
	}
	cmp_cmdlst_para = (cmdlst_para_t *)(uintptr_t)va;
	new_entry = (schedule_cmdlst_link_t *)cmp_cmdlst_para->cmd_entry;
	loge("CMP CMDLST BUFF:");
	for (i = 0; i < cmp_cmdlst_para->stripe_cnt; ++i)
		cmdlst_buff_dump(&(new_entry[i].cmd_buf));
}

static void dump_reg(void)
{
	void __iomem *crg_base;
	int i;
	unsigned int value, value2, value3, value4;
	crg_base = hipp_get_regaddr(CPE_TOP);
	value = readl(crg_base + IPP_TOP_ENH_IRQ_REG3_REG);
	loge("enh_irq_state_raw = 0x%08x", value);
	value = readl(crg_base + IPP_TOP_ARF_IRQ_REG3_REG);
	loge("arf_irq_state_raw = 0x%08x", value);
	value = readl(crg_base + IPP_TOP_RDR_IRQ_REG3_REG);
	loge("rdr_irq_state_raw = 0x%08x", value);
	value = readl(crg_base + IPP_TOP_CMP_IRQ_REG3_REG);
	loge("cmp_irq_state_raw = 0x%08x", value);
	value = readl(crg_base + IPP_TOP_MC_IRQ_REG3_REG);
	loge("mc_irq_state_raw = 0x%08x", value);
	value = readl(crg_base + IPP_TOP_KLT_IRQ_REG3_REG);
	loge("klt_irq_state_raw = 0x%08x", value);
	crg_base = hipp_get_regaddr(ARF_REG);
	value = readl(crg_base + AR_FEATURE_TOTAL_KPT_NUM_REG);
	loge("arf reg: offset = 0x%x, value = 0x%x", AR_FEATURE_TOTAL_KPT_NUM_REG, value);

	crg_base = hipp_get_regaddr(ORB_ENH);
	loge("enh reg:");
	for (i = ORB_ENH_ENABLE_CFG_REG; i <= ORB_ENH_LUT_SCALE_REG; i += 16) {
		value = readl(crg_base + i);
		value2 = readl(crg_base + i + 0x4);
		value3 = readl(crg_base + i + 0x8);
		value4 = readl(crg_base + i + 0xC);
		loge("offset(0x%x - 0x%x): value(0x%08x  0x%08x  0x%08x  0x%08x)", i, i + 0xC, value, value2, value3, value4);
	}
	crg_base = hipp_get_regaddr(ARF_REG);
	loge("arf reg:");
	for (i = AR_FEATURE_TOP_CFG_REG; i <= AR_FEATURE_ROI_WINDOW_MODE_CFG_REG; i += 4) {
		value = readl(crg_base + i);
		loge("offset = 0x%x, value = 0x%08x", i, value);
	}
	loge("arf kpt num:");
	for (i = AR_FEATURE_KPT_NUMBER_0_REG; i <= AR_FEATURE_KPT_NUMBER_255_REG; i += 16) {
		value = readl(crg_base + i);
		value2 = readl(crg_base + i + 0x4);
		value3 = readl(crg_base + i + 0x8);
		value4 = readl(crg_base + i + 0xC);
		loge("offset(0x%x - 0x%x): value(0x%08x  0x%08x  0x%08x  0x%08x)", i, i + 0xC, value, value2, value3, value4);
	}
	loge("arf total kpt num and debug:");
	for (i = AR_FEATURE_TOTAL_KPT_NUM_REG; i <= AR_FEATURE_DEBUG_3_REG; i += 4) {
		value = readl(crg_base + i);
		loge("offset = 0x%x, value = 0x%08x", i, value);
	}
	crg_base = hipp_get_regaddr(REORDER_REG);
	loge("rdr reg:");
	for (i = REORDER_REORDER_CFG_REG; i <= REORDER_KPT_NUMBER_255_REG; i += 16) {
		value = readl(crg_base + i);
		value2 = readl(crg_base + i + 0x4);
		value3 = readl(crg_base + i + 0x8);
		value4 = readl(crg_base + i + 0xC);
		loge("offset(0x%x - 0x%x): value(0x%08x  0x%08x  0x%08x  0x%08x)", i, i + 0xC, value, value2, value3, value4);
	}
	crg_base = hipp_get_regaddr(COMPARE_REG);
	loge("cmp reg:");
	for (i = COMPARE_COMPARE_CFG_REG; i <= COMPARE_TOTAL_KPT_NUM_REG; i += 4) {
		value = readl(crg_base + i);
		loge("offset = 0x%x, value = 0x%08x", i, value);
	}
	value = readl(crg_base + COMPARE_MATCH_POINTS_REG);
	loge("offset = 0x%x, value = 0x%08x", COMPARE_MATCH_POINTS_REG, value);
	value = readl(crg_base + COMPARE_DEBUG_0_REG);
	loge("offset = 0x%x, value = 0x%08x", COMPARE_DEBUG_0_REG, value);
	value = readl(crg_base + COMPARE_DEBUG_1_REG);
	loge("offset = 0x%x, value = 0x%08x", COMPARE_DEBUG_1_REG, value);

	crg_base = hipp_get_regaddr(MC_REG);
	loge("mc reg:");
	for (i = MC_EN_CFG_REG; i <= MC_INDEX_CFG_0_REG; i += 16) {
		value = readl(crg_base + i);
		value2 = readl(crg_base + i + 0x4);
		value3 = readl(crg_base + i + 0x8);
		value4 = readl(crg_base + i + 0xC);
		loge("offset(0x%x - 0x%x): value(0x%08x  0x%08x  0x%08x  0x%08x)", i, i + 0xC, value, value2, value3, value4);
	}
	loge("mc debug:");
	value = readl(crg_base + MC_DEBUG_0_REG);
	value2 = readl(crg_base + MC_DEBUG_0_REG + 0x4);
	value3 = readl(crg_base + MC_DEBUG_0_REG + 0x8);
	value4 = readl(crg_base + MC_DEBUG_0_REG + 0xC);
	loge("offset(0x%x - 0x%x): value(0x%08x  0x%08x  0x%08x  0x%08x)", i, i + 0xC, value, value2, value3, value4);

	crg_base = hipp_get_regaddr(KLT_REG);
	loge("klt reg:");
	for (i = KLT_DS_CONFIG_REG; i <= KLT_KP_NEXT_BUFF_Y_499_REG; i += 16) {
		value = readl(crg_base + i);
		value2 = readl(crg_base + i + 0x4);
		value3 = readl(crg_base + i + 0x8);
		value4 = readl(crg_base + i + 0xC);
		loge("offset(0x%x - 0x%x): value(0x%08x  0x%08x  0x%08x  0x%08x)", i, i + 0xC, value, value2, value3, value4);
	}
}

static int hispcpe_ipp_path_request(hipp_adapter_s *dev, struct msg_req_ipp_cmd_req_t *req)
{
	int ret = 0, ret_1 = 0, ret_2 = 0;
	// 1.Define the dma_buf linked list header and init it. Each request applies for its own list header.
	struct list_head fd_link_head;

	ret = hipp_orcm_clean_wait_flag(dev, req->work_module);
	if (ret != 0) {
		loge("Failed: hipp_orcm_clean_wait_flag.%d", ret);
		return ret;
	}

	INIT_LIST_HEAD(&fd_link_head);
	ret = hispcpe_get_kernel_buff_addr(req, &fd_link_head);
	if (ret != 0) {
		loge("Failed: hispcpe_get_kernel_buff_addr.%d", ret);
		return ret;
	}

	mutex_lock(&dev->hipp_refs_lock);
	if (dev->hipp_refs[REFS_TYP_EARCMK] > MAX_HIPP_REFS_CNT) {
		loge("max hipp refs.%d", dev->hipp_refs[REFS_TYP_EARCMK]);
		mutex_unlock(&dev->hipp_refs_lock);
		return ISP_IPP_ERR;
	}
	dev->hipp_refs[REFS_TYP_EARCMK]++;
	mutex_unlock(&dev->hipp_refs_lock);

	ret = ipp_path_request_handler(req);
	if (ret != 0) {
		loge("Failed: ipp_path_request_handler.ret = %d, frame_num = %d, work_module = %d", ret,
			req->frame_num, req->work_module);
		goto ipp_path_req_done;
	}

	ret = hipp_orcm_wait(dev, req);
	if (ret != 0) {
		loge("Failed: hispcpe_orcm_wait.%d", ret);
		dump_reg();
		dump_cmdlst_buff(req);
		goto ipp_path_req_done;
	}

ipp_path_req_done:
	ret_1 = hipp_orcm_eop_handle(dev, req);
	if (ret_1 != 0)
		loge("Failed : hipp_orcm_eop_handle.%d", ret_1);

	ret_2 = hispcpe_release_kernel_buff_addr(&fd_link_head, req->work_module);
	if (ret_2 != 0)
		loge("Failed : hispcpe_release_kernel_buff_addr.%d", ret_2);

	mutex_lock(&dev->hipp_refs_lock);
	dev->hipp_refs[REFS_TYP_EARCMK]--;
	mutex_unlock(&dev->hipp_refs_lock);

	if (ret_2 != 0 || ret_1 != 0 || ret != 0)
		return ISP_IPP_ERR;
	else
		return ISP_IPP_OK;
}

static int hipp_is_level2_soc_spec(void)
{
	const char *soc_spec = NULL;
	int ret;

	struct device_node *np = of_find_compatible_node(NULL, NULL, "hisilicon, soc_spec");
	if (np == NULL) {
		pr_err("[%s] of_find_compatible_node fail\n", __func__);
		return 0;
	}

	ret = of_property_read_string(np, "soc_spec_set", &soc_spec);
	if (ret < 0) {
		pr_err("[%s] of_property_read_string fail\n", __func__);
		return 0;
	}

	ret = strncmp(soc_spec, "level2", strlen("level2"));
	if (ret == 0) {
		pr_err("[%s] soc spec is level2, reduce ipp clk rate\n", __func__);
		return 1;
	}

	pr_info("[%s] soc spec is not level2\n", __func__);
	return 0;
}

/**
 * Name: config_cpe_clk_rate
 * Description: if a request wishes to pull down that current frequency and other requests are using the high frequency;
 *     the operation of pulling down the frequency will not be performed;
 * Key Val:
 *     CLK_RATE_TURBO  = 0, // 640
 *     CLK_RATE_NORMAL = 1, // 480
 *     CLK_RATE_HSVS   = 2, // 400
 *     CLK_RATE_SVS    = 3, // 277
 *     CLK_RATE_OFF    = 4, // 104
 *     MAX_CLK_RATE;
 **/
static int config_cpe_clk_rate(unsigned int req_rate)
{
	int ret =ISP_IPP_OK;
	hipp_adapter_s *dev = g_hipp_adapter;
	char *clk_name[MAX_CLK_RATE] = {"TURBO", "NORMAL", "HSVS", "SVS", "OFF"};
	int is_level2_chip = 0;

	loge_if_null_set_ret(dev, ret);
	if (ret) return ret;

	mutex_lock(&dev->hipp_refs_lock);
	mutex_lock(&dev->cpe_rate_lock);
	if (dev->hipp_refs[REFS_TYP_EARCMK] > 0) {
		if (req_rate < dev->cpe_rate_value) // Note: CLK_RATE_TURBO < CLK_RATE_SVS
			dev->cpe_rate_value = req_rate;
	} else { // Means that no request is being processed by IPP-hardware.
		dev->cpe_rate_value = req_rate;
	}
	mutex_unlock(&dev->cpe_rate_lock);
	mutex_unlock(&dev->hipp_refs_lock);

	is_level2_chip = hipp_is_level2_soc_spec();
	mutex_lock(&dev->hipp_refs_lock);
	mutex_lock(&dev->cpe_rate_lock);
	if (is_level2_chip == 1 && dev->cpe_rate_value >= CLK_RATE_TURBO && dev->cpe_rate_value < CLK_RATE_SVS) {
		loge("Level2 chip, Reduce clk rate from %s to %s",
			clk_name[dev->cpe_rate_value], clk_name[dev->cpe_rate_value + 1]);
		dev->cpe_rate_value += 1;
	}
	mutex_unlock(&dev->cpe_rate_lock);
	mutex_unlock(&dev->hipp_refs_lock);

	ret = dev->ippdrv->set_jpgclk_rate(dev->ippdrv, dev->cpe_rate_value);
	if (ret) {
		loge("Failed: failed to set rate_value.%s", clk_name[dev->cpe_rate_value]);
		ret = dev->ippdrv->set_jpgclk_rate(dev->ippdrv, CLK_RATE_SVS);
		if (ret) loge("Failed: failed to set rate_value.%s", clk_name[CLK_RATE_SVS]);
	}
	return ret;
}

static int reset_cpe_clk_rate(void)
{
	int ret =ISP_IPP_OK;
	hipp_adapter_s *dev = g_hipp_adapter;
	char *clk_name[MAX_CLK_RATE] = {"TURBO", "NORMAL", "HSVS", "SVS", "OFF"};

	loge_if_null_set_ret(dev, ret);
	if (ret) return ret;

	mutex_lock(&dev->hipp_refs_lock);
	mutex_lock(&dev->cpe_rate_lock);
	if (dev->hipp_refs[REFS_TYP_EARCMK] == 0) {
		dev->cpe_rate_value = CLK_RATE_SVS; // reset the cpe_rate_value
		ret = dev->ippdrv->set_jpgclk_rate(dev->ippdrv, dev->cpe_rate_value);
		if (ret) loge("Failed: failed to set rate_value.%s", clk_name[dev->cpe_rate_value]);
	}
	mutex_unlock(&dev->cpe_rate_lock);
	mutex_unlock(&dev->hipp_refs_lock);
	return ret;
}

int hipp_adapter_do_request(unsigned long args)
{
	int ret = 0;
	unsigned long remaining_bytes = 0;
	hipp_adapter_s *dev = g_hipp_adapter;
	struct msg_req_ipp_cmd_req_t *req = NULL;
	void __user *p_user_args = (void __user *)(uintptr_t)args;

	loge_if_null_set_ret(p_user_args, ret);
	if (ret) return ret;

	req = (struct msg_req_ipp_cmd_req_t *)vmalloc(sizeof(struct msg_req_ipp_cmd_req_t));
	if (req == NULL) {
		loge("Failed to vmalloc memory for req");
		return -EINVAL;
	}

	if (memset_s(req, sizeof(struct msg_req_ipp_cmd_req_t), 0, sizeof(struct msg_req_ipp_cmd_req_t))) {
		loge("Failed : fail to memset req to zero");
		ret = ISP_IPP_ERR;
		goto free_req_vmalloc_memory;
	}

	if ((sizeof(struct msg_req_ipp_cmd_req_t) + (uintptr_t)p_user_args) < (uintptr_t)p_user_args) {
		pr_err("[%s] Failed arg size is too large\n", __func__);
		ret = ISP_IPP_ERR;
		goto free_req_vmalloc_memory;
	}

	remaining_bytes = copy_from_user(req, p_user_args, sizeof(struct msg_req_ipp_cmd_req_t));
	if (remaining_bytes != 0) {
		loge("Failed : copy_from_user failed, remaining_bytes = %ld", remaining_bytes);
		ret = ISP_IPP_ERR;
		goto free_req_vmalloc_memory;
	}

	if (req->work_module < KLT_ONLY || req->work_module >= WORK_MODULE_MAX
		|| req->instance_id >= MAX_HIPP_REFS_CNT
		||req->rate_value > CLK_RATE_SVS) {
		loge("Failed : invalid req, work_module.%d, instance_id.%d, rate_value.%d", req->work_module,
			req->instance_id, req->rate_value);
		ret = ISP_IPP_ERR;
		goto free_req_vmalloc_memory;
	}

	mutex_lock(&dev->mode_working_lock);
	if (dev->is_cur_mode_working[req->work_module] == 1) {
		mutex_unlock(&dev->mode_working_lock);
		loge("Failed : Multiple req cannot be processed in the same work_module.%u", req->work_module);
		ret = ISP_IPP_ERR;
		goto free_req_vmalloc_memory;
	}
	dev->is_cur_mode_working[req->work_module] = 1;
	mutex_unlock(&dev->mode_working_lock);

	if (dev->mapbuf_ready[req->work_module] == 0) {
		loge("Failed : no map_kernel ops before, work_module.%u", req->work_module);
		ret = ISP_IPP_ERR;
		goto clean_cur_mode_working_flag;
	}

	ret = config_cpe_clk_rate(req->rate_value);
	if (ret != 0) {
		loge("Failed : set_cpe_clk_rate.%d", ret);
		goto clean_cur_mode_working_flag;
	}

	ret = hispcpe_ipp_path_request(dev, req);
	if (ret != 0) {
		loge("Failed: hispcpe_ipp_path_request.ret = %d, frame_num = %d, work_module = %d", ret,
			req->frame_num, req->work_module);
		goto clean_cur_mode_working_flag;
	}

	ret = reset_cpe_clk_rate();
	if (ret != 0) loge("Failed : reset_cpe_clk_rate.%d", ret);

clean_cur_mode_working_flag:
	mutex_lock(&dev->mode_working_lock);
	dev->is_cur_mode_working[req->work_module] = 0;
	mutex_unlock(&dev->mode_working_lock);

free_req_vmalloc_memory:
	vfree(req);
	req = NULL;
	return ret;
}

static int check_map_buf_para(hipp_adapter_s *dev, struct map_buff_block_s *frame_buf)
{
	if (frame_buf->shared_fd < 0) {
		loge("Failed : share_fd.%d", frame_buf->shared_fd);
		return -EINVAL;
	}

	if(frame_buf->work_module >= WORK_MODULE_MAX) {
		pr_err("[%s] work_module = %d invalid\n", __func__, frame_buf->work_module);
		return -EINVAL;
	}

	if (frame_buf->size > MAP_ALGO_BUF_MAX_SIZE) {
		loge("Failed : size.%d exceed max size %d", frame_buf->size, MAP_ALGO_BUF_MAX_SIZE);
		return -EINVAL;
	}

	if (dev->ippdrv == NULL) {
		loge("Failed : ippdrv NULL");
		return -EINVAL;
	}

	if (dev->ippdrv->iommu_map == NULL) {
		loge("Failed : ippdrv->iommu_map NULL");
		return -EINVAL;
	}

	if (dev->ippdrv->kmap == NULL) {
		loge("Failed : ippdrv->kmap NULL");
		return -EINVAL;
	}

	return ISP_IPP_OK;
}

/**
 * name:hispcpe_map_kernel
 * description:This interface is called once for each work_mode. The same work_mode cannot call the
 *             interface multiple times.
 **/
int hispcpe_map_kernel(unsigned long args)
{
	void *virt_addr = NULL;
	void *sw_virt_addr = NULL;
	struct map_buff_block_s frame_buf = { 0 };
	unsigned long iova, iova_size = 0;
	void __user *args_mapkernel = (void __user *)(uintptr_t) args;
	hipp_adapter_s *dev = g_hipp_adapter;
	struct dma_buf *modulebuf = NULL;

	if ((args_mapkernel == NULL) || (dev == NULL)) {
		loge("Failed : args_mapkernel.%pK", args_mapkernel);
		return -EINVAL;
	}

	if ((sizeof(frame_buf) + (uintptr_t)args_mapkernel) < (uintptr_t)args_mapkernel)
		return -EINVAL;

	if (copy_from_user(&frame_buf, args_mapkernel, sizeof(frame_buf))) {
		loge("Failed : copy_from_user");
		return -EINVAL;
	}

	if (check_map_buf_para(dev, &frame_buf)) {
		loge("Failed : check_map_buf_para");
		return -EINVAL;
	}

	if (dev->mapbuf_ready[frame_buf.work_module] == 1) {
		loge("Failed : map_kernel already done, work_module.%d", frame_buf.work_module);
		return -EINVAL;
	}

	modulebuf = dma_buf_get(frame_buf.shared_fd);
	if (IS_ERR_OR_NULL(modulebuf)) {
		loge("Failed : dma_buf_get, work_module.%d", frame_buf.work_module);
		return -EINVAL;
	}

	if (modulebuf->size != frame_buf.size) {
		loge("Failed : size not equal, work_module.%d, modulebuf->size = %lu, frame_buf.size = 0x%x",
			frame_buf.work_module, modulebuf->size, frame_buf.size);
		dma_buf_put(modulebuf);
		return -EINVAL;
	}

	iova = dev->ippdrv->iommu_map(dev->ippdrv, modulebuf, IOMMU_READ | IOMMU_WRITE, &iova_size);
	if ((iova == 0) || (iova_size != frame_buf.size)) {
		loge("Failed : iommu_map, prot.%d, size.%d, work_module.%d",
			frame_buf.prot, frame_buf.size, frame_buf.work_module);
		dma_buf_put(modulebuf);
		return -EINVAL;
	}

	virt_addr = dev->ippdrv->kmap(dev->ippdrv, modulebuf);
	if (virt_addr == NULL) {
		loge("Failed : kmap, sharedfd.%d, work_module.%d", frame_buf.shared_fd, frame_buf.work_module);
		goto free_kmap;
	}

	sw_virt_addr = vmalloc(MEM_HISPCPE_SW_SIZE); // for cfg table
	if (sw_virt_addr == NULL) {
		loge("Failed : vmalloc sw_virt_addr");
		goto free_kmap;
	}

	if (memset_s(sw_virt_addr, MEM_HISPCPE_SW_SIZE, 0, MEM_HISPCPE_SW_SIZE)) {
		loge(" Failed : memset_s sw_virt_addr");
		goto free_kmap;
	}

	dev->shared_fd[frame_buf.work_module] = frame_buf.shared_fd;
	dev->daddr[frame_buf.work_module]     = iova;
	dev->iova_size[frame_buf.work_module] = iova_size;
	dev->virt_addr[frame_buf.work_module] = virt_addr;
	dev->dmabuf[frame_buf.work_module]    = modulebuf;
	dev->sw_virt_addr[frame_buf.work_module] = sw_virt_addr;

	logd("map kernel ok: shared_fd.%d, work_module.%d", frame_buf.shared_fd, frame_buf.work_module);
	if (cpe_init_memory(frame_buf.work_module)) {
		loge("Failed : cpe_init_memory error, work_module.%d", frame_buf.work_module);
		goto free_kmap;
	}
	dev->mapbuf_ready[frame_buf.work_module] = 1;
	return ISP_IPP_OK;
free_kmap:
	if (sw_virt_addr != NULL)
		vfree(sw_virt_addr);

	if (virt_addr != NULL) {
		if (dev->ippdrv->kunmap(dev->ippdrv, modulebuf, virt_addr))
			loge("Failed : kunmap, sharedfd.%d, virt_addr.%pK, work_module.%d",
				frame_buf.shared_fd, virt_addr, frame_buf.work_module);
	}

	if (dev->ippdrv->iommu_unmap(dev->ippdrv, modulebuf, iova))
		loge("Failed : iommu_unmap, sharedfd.%d, iova.0x%lx, work_module.%d",
			frame_buf.shared_fd, iova, frame_buf.work_module);

	dma_buf_put(modulebuf);
	dev->mapbuf_ready[frame_buf.work_module] = 0;
	dev->shared_fd[frame_buf.work_module] = 0;
	dev->daddr[frame_buf.work_module]     = 0;
	dev->iova_size[frame_buf.work_module] = 0;
	dev->virt_addr[frame_buf.work_module] = NULL;
	dev->dmabuf[frame_buf.work_module]    = NULL;
	dev->sw_virt_addr[frame_buf.work_module] = NULL;

	return -EINVAL;
}

int hispcpe_unmap_kernel(unsigned long args)
{
	int ret_1 = 0, ret_2 = 0;
	unsigned int work_module;
	void __user *args_mapkernel = (void __user *)(uintptr_t) args;
	hipp_adapter_s *dev = g_hipp_adapter;

	if ((args_mapkernel == NULL) || (dev == NULL)) {
		loge("Failed : args_mapkernel.%pK", args_mapkernel);
		return -EINVAL;
	}

	if ((sizeof(unsigned int) + (uintptr_t)args_mapkernel) < (uintptr_t)args_mapkernel) {
		pr_err("[%s] Failed arg size is too large\n", __func__);
		return -EINVAL;
	}

	if (copy_from_user(&work_module, args_mapkernel, sizeof(unsigned int))) {
		loge("Failed : copy_from_user");
		return -EINVAL;
	}

	if (work_module >= WORK_MODULE_MAX) {
		loge("Failed : work_module.%d is invalid", work_module);
		return -EINVAL;
	}

	if (dev->mapbuf_ready[work_module] == 0) {
		loge("Failed : mapbuf_ready[%d] is 0, no memory needs to be released", work_module);
		return -EINVAL;
	}

	if (dev->ippdrv == NULL) {
		loge("Failed : ippdrv NULL");
		return -EINVAL;
	}

	if (dev->ippdrv->iommu_unmap == NULL || dev->ippdrv->kunmap == NULL) {
		loge("Failed : ptr is NULL, ippdrv->iommu_unmap.%pK, kunmap.%pK",
			dev->ippdrv->iommu_unmap, dev->ippdrv->kunmap);
		return -EINVAL;
	}

	logd("do unmap: work_module.%u, mapbuf_ready.%u, shared_fd.%d, "
		"iova_size.0x%lx, dmabuf.0x%pK, virt_addr.0x%pK, daddr.0x%lx",
		work_module, dev->mapbuf_ready[work_module], dev->shared_fd[work_module],
		dev->iova_size[work_module], dev->dmabuf[work_module], dev->virt_addr[work_module], dev->daddr[work_module]);

	if (dev->daddr[work_module] != 0) {
		if (dev->ippdrv->iommu_unmap(dev->ippdrv, dev->dmabuf[work_module], dev->daddr[work_module])) {
			loge("Failed: iommu_unmap, work_module.%u", work_module);
			ret_1 = -EINVAL;
		}
		dev->daddr[work_module] = 0; // clean daddr
	}

	if (dev->virt_addr[work_module] != NULL && dev->dmabuf[work_module] != NULL) {
		if (dev->ippdrv->kunmap(dev->ippdrv, dev->dmabuf[work_module], dev->virt_addr[work_module])) {
			loge("Failed : kunmap, work_module.%u", work_module);
			ret_2 = -EINVAL;
		}
		dma_buf_put(dev->dmabuf[work_module]);
		dev->virt_addr[work_module] = NULL;
		dev->dmabuf[work_module]    = NULL;
	}

	if (dev->sw_virt_addr[work_module] != NULL) {
		vfree(dev->sw_virt_addr[work_module]);
		dev->sw_virt_addr[work_module] = NULL;
	}

	dev->mapbuf_ready[work_module] = 0;
	dev->shared_fd[work_module] = 0;
	dev->iova_size[work_module] = 0;

	return (ret_1 || ret_2);
}

static int check_map_algo_para(hipp_adapter_s *dev, struct map_algo_buff_block_s *frame_buf)
{
	if (frame_buf->shared_fd < 0) {
		loge("Failed : share_fd.%d", frame_buf->shared_fd);
		return -EINVAL;
	}

	if (frame_buf->work_module >= WORK_MODULE_MAX || frame_buf->instance_id >= MAX_HIPP_REFS_CNT) {
		loge("Failed : work_module.%d, instance_id.%d is invalid", frame_buf->work_module, frame_buf->instance_id);
		return -EINVAL;
	}

	if (frame_buf->size > MAP_ALGO_BUF_MAX_SIZE) {
		loge("Failed : size.%d exceed max size %d", frame_buf->size, MAP_ALGO_BUF_MAX_SIZE);
		return -EINVAL;
	}

	if (dev->ippdrv == NULL) {
		loge("Failed : ippdrv NULL");
		return -EINVAL;
	}

	if (dev->ippdrv->iommu_map == NULL) {
		loge("Failed : ippdrv->iommu_map NULL");
		return -EINVAL;
	}

	return ISP_IPP_OK;
}

int hispcpe_map_kernel_algo(unsigned long args)
{
	struct map_algo_buff_block_s frame_buf = { 0 };
	unsigned long iova, iova_size = 0;
	void __user *args_mapkernel = (void __user *)(uintptr_t) args;
	hipp_adapter_s *dev = g_hipp_adapter;
	struct dma_buf *modulebuf = NULL;

	if ((args_mapkernel == NULL) || (dev == NULL)) {
		loge("Failed : algo args_mapkernel.%pK", args_mapkernel);
		return -EINVAL;
	}

	if ((sizeof(frame_buf) + (uintptr_t)args_mapkernel) < (uintptr_t)args_mapkernel) {
		pr_err("[%s] Failed arg size is too large\n", __func__);
		return -EINVAL;
	}

	if (copy_from_user(&frame_buf, args_mapkernel, sizeof(frame_buf))) {
		loge("Failed : algo copy_from_user");
		return -EINVAL;
	}

	if (check_map_algo_para(dev, &frame_buf)) {
		loge("Failed : algo check_map_algo_para");
		return -EINVAL;
	}

	if (dev->refs_mapbuf_algo[frame_buf.work_module] >= MAX_HIPP_REFS_CNT) {
		loge("Failed : refs_mapbuf_algo[%d] already exceed max user %d", frame_buf.work_module, MAX_HIPP_REFS_CNT);
		return -EINVAL;
	}

	modulebuf = dma_buf_get(frame_buf.shared_fd);
	if (IS_ERR_OR_NULL(modulebuf)) {
		loge("Failed : algo dma_buf_get, work_module.%d", frame_buf.work_module);
		return -EINVAL;
	}

	if (modulebuf->size != frame_buf.size) {
		loge("Failed : algo size not equal, work_module.%d, modulebuf->size = %lu, frame_buf.size = 0x%x",
			frame_buf.work_module, modulebuf->size, frame_buf.size);
		dma_buf_put(modulebuf);
		return -EINVAL;
	}

	iova = dev->ippdrv->iommu_map(dev->ippdrv, modulebuf, IOMMU_READ | IOMMU_WRITE, &iova_size);
	if ((iova == 0) || (iova_size != (unsigned long)frame_buf.size)) {
		loge("Failed : algo iommu_map, prot.%d, size.%d, work_module.%d",
			frame_buf.prot, frame_buf.size, frame_buf.work_module);
		dma_buf_put(modulebuf);
		return -EINVAL;
	}

	if ((dev->algo_fd[frame_buf.work_module][frame_buf.instance_id] != 0) ||
		(dev->algo_daddr[frame_buf.work_module][frame_buf.instance_id] !=0) ||
		(dev->algo_iova_size[frame_buf.work_module][frame_buf.instance_id] != 0) ||
		(dev->algo_dmabuf[frame_buf.work_module][frame_buf.instance_id] != NULL)) {
		loge("Failed : work_module.%d, instance_id.%d, already maped", frame_buf.work_module, frame_buf.instance_id);
		dma_buf_put(modulebuf);
		return -EINVAL;
	}
	dev->algo_fd[frame_buf.work_module][frame_buf.instance_id] = frame_buf.shared_fd;
	dev->algo_daddr[frame_buf.work_module][frame_buf.instance_id]     = iova;
	dev->algo_iova_size[frame_buf.work_module][frame_buf.instance_id] = iova_size;
	dev->algo_dmabuf[frame_buf.work_module][frame_buf.instance_id]    = modulebuf;
	dev->refs_mapbuf_algo[frame_buf.work_module]++;

	logi("algo map_kernel ok. work_module.%d, instance_id.%d, refs_mapbuf_algo.%u",
		frame_buf.work_module, frame_buf.instance_id, dev->refs_mapbuf_algo[frame_buf.work_module]);
	return ISP_IPP_OK;
}

int hispcpe_unmap_kernel_algo(unsigned long args)
{
	struct map_algo_buff_block_s frame_buf = { 0 };
	unsigned int work_module;
	unsigned int instance_id;
	void __user *args_mapkernel = (void __user *)(uintptr_t) args;
	hipp_adapter_s *dev = g_hipp_adapter;

	if ((args_mapkernel == NULL) || (dev == NULL)) {
		loge("Failed : args_mapkernel.%pK", args_mapkernel);
		return -EINVAL;
	}

	if ((sizeof(frame_buf) + (uintptr_t)args_mapkernel) < (uintptr_t)args_mapkernel) {
		pr_err("[%s] Failed arg size is too large\n", __func__);
		return -EINVAL;
	}

	if (copy_from_user(&frame_buf, args_mapkernel, sizeof(frame_buf))) {
		loge("Failed : copy_from_user");
		return -EINVAL;
	}

	work_module = frame_buf.work_module;
	instance_id = frame_buf.instance_id;
	if (work_module >= WORK_MODULE_MAX || instance_id >= MAX_HIPP_REFS_CNT) {
		loge("Failed : work_module.%d, instance_id.%d is invalid", work_module, instance_id);
		return -EINVAL;
	}

	if (dev->refs_mapbuf_algo[work_module] <= 0) {
		loge("Failed : refs_mapbuf_algo[%d] is 0, no algo memory needs to be released", work_module);
		return -EINVAL;
	}

	if (dev->ippdrv == NULL) {
		loge("Failed : ippdrv NULL");
		return -EINVAL;
	}

	if (dev->ippdrv->iommu_unmap == NULL) {
		loge("Failed : ptr is NULL, ippdrv->iommu_unmap.%pK", dev->ippdrv->iommu_unmap);
		return -EINVAL;
	}

	if (dev->algo_daddr[work_module][instance_id] != 0) {
		if (dev->ippdrv->iommu_unmap(dev->ippdrv, dev->algo_dmabuf[work_module][instance_id],
				dev->algo_daddr[work_module][instance_id])) {
			loge("Failed: algo iommu_unmap, work_module.%u", work_module);
			return -EINVAL;
		}
		dev->algo_daddr[work_module][instance_id] = 0;
	}

	if (dev->algo_dmabuf[work_module][instance_id] != NULL)
		dma_buf_put(dev->algo_dmabuf[work_module][instance_id]);
	dev->algo_dmabuf[work_module][instance_id] = NULL;
	dev->algo_fd[work_module][instance_id] = 0;
	dev->algo_iova_size[work_module][instance_id] = 0;

	if (dev->refs_mapbuf_algo[work_module] > 0)
		dev->refs_mapbuf_algo[work_module]--;

	logi("algo unmap_kernel ok: work_module.%u,instance_id.%d, refs_mapbuf_algo.%u",
		work_module, instance_id, dev->refs_mapbuf_algo[work_module]);

	return ISP_IPP_OK;
}

#ifdef CONFIG_IPP_DEBUG
void hipp_resource_deinit(void)
{
	hipp_adapter_s *dev = g_hipp_adapter;
	int i;
	logi("+");

	if (dev == NULL) {
		loge("Failed : dev.%pK", dev);
		return;
	}

	for (i = 0; i < WORK_MODULE_MAX;i++) {
		dev->mapbuf_ready[i] = 0;
		dev->iova_size[i] = 0;
		dev->virt_addr[i] = NULL;
		dev->daddr[i] = 0;
		if (dev->sw_virt_addr[i] != NULL) {
			vfree(dev->sw_virt_addr[i]);
			dev->sw_virt_addr[i] = NULL;
		}
	}

	logi("-");
}

void hipp_resource_init(void)
{
	hipp_adapter_s *dev = g_hipp_adapter;
	void *sw_virt_addr = NULL;
	unsigned int i;
	int ret;

	if (dev == NULL) {
		loge("Failed : dev.%pK", dev);
		return;
	}

	for (i = 0; i < WORK_MODULE_MAX;i++) {
		dev->mapbuf_ready[i] = 1;
		dev->virt_addr[i] = NULL;
		dev->sw_virt_addr[i] = NULL;
		dev->daddr[i] = 0;
		dev->iova_size[i] = MAP_ALGO_BUF_MAX_SIZE;
		sw_virt_addr = vmalloc(MEM_HISPCPE_SW_SIZE);
		if (sw_virt_addr == NULL) {
			loge("Failed : vmalloc sw_virt_addr");
			goto resource_deinit;
		}

		if (memset_s(sw_virt_addr, MEM_HISPCPE_SW_SIZE, 0, MEM_HISPCPE_SW_SIZE)) {
		    loge(" Failed : memset_s sw_virt_addr");
		    goto resource_deinit;
		}

		dev->sw_virt_addr[i] = sw_virt_addr;
		dev->virt_addr[i] = sw_virt_addr;
		dev->daddr[i] = 1;
		ret = cpe_init_memory(i);
		if (ret < 0)
			loge("Failed : cpe_init_memory:%u", i);
		cpe_set_memory(i);
	}

	logi("hipp_resource_init succ");
	return;
resource_deinit:
	hipp_resource_deinit();
}
#endif

void hipp_adapter_exception(void)
{
	int i;
	int j;
	hipp_adapter_s *dev = g_hipp_adapter;
	logi("+");

	if (dev == NULL) {
		loge("Failed : dev.%pK", dev);
		return;
	}
	if (dev->ippdrv == NULL) {
		loge("Failed : ippdrv NULL");
		return;
	}

	if (dev->ippdrv->iommu_unmap == NULL) {
		loge("Failed : ippdrv->iommu_unmap NULL");
		return;
	}

	if (dev->ippdrv->kunmap == NULL) {
		loge("Failed : ippdrv->kunmap NULL");
		return;
	}

	for (i = 0; i < WORK_MODULE_MAX; i++) {
		if (dev->daddr[i] != 0) {
			if (dev->ippdrv->iommu_unmap(dev->ippdrv, dev->dmabuf[i], dev->daddr[i]))
				loge("Failed: iommu_unmap.%d", i);

			dev->daddr[i] = 0;
		}

		if (dev->virt_addr[i] != NULL && dev->dmabuf[i] != NULL) {
			if (dev->ippdrv->kunmap(dev->ippdrv, dev->dmabuf[i], dev->virt_addr[i]))
				loge("Failed : kunmap.%d", i);

			dma_buf_put(dev->dmabuf[i]);
			dev->virt_addr[i] = NULL;
			dev->dmabuf[i]    = NULL;
		}

		dev->mapbuf_ready[i] = 0;
		dev->shared_fd[i] = 0;
		dev->iova_size[i] = 0;

		for (j = 0; j < MAX_HIPP_REFS_CNT; j++) {
			if (dev->algo_daddr[i][j] != 0) {
				if (dev->ippdrv->iommu_unmap(dev->ippdrv, dev->algo_dmabuf[i][j], dev->algo_daddr[i][j]))
					loge("Failed: algo iommu_unmap, work_module.%d, instanceId.%d", i, j);
				dev->algo_daddr[i][j] = 0;
			}

			if (dev->algo_dmabuf[i][j] != NULL)
				dma_buf_put(dev->algo_dmabuf[i][j]);
			dev->algo_dmabuf[i][j] = NULL;
			dev->algo_fd[i][j] = 0;
			dev->algo_iova_size[i][j] = 0;
			if (dev->refs_mapbuf_algo[i] > 0)
				dev->refs_mapbuf_algo[i]--;
		}
	}

	logi("-");
}

int hipp_adapter_probe(struct platform_device *pdev)
{
	hipp_adapter_s *dev = NULL;
	unsigned int i = 0;
	logi("+");
	g_hipp_adapter = NULL;
	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (dev == NULL)
		return -ENOMEM;

	dev->ippdrv = hipp_common_register(IPP_UNIT, &pdev->dev);
	if (dev->ippdrv == NULL) {
		loge("Failed : IPP_UNIT register");
		goto free_dev;
	}

	for (i = 0; i < CMDLST_CHANNEL_MAX; i++)
		init_waitqueue_head(&dev->ch_wq_head[i]); // new r02

	for (i = 0; i < REFS_TYP_MAX; i++)
		dev->hipp_refs[i] = 0;

	mutex_init(&dev->hipp_refs_lock);
	mutex_init(&dev->cpe_rate_lock);
	mutex_init(&dev->mode_working_lock);

	g_hipp_adapter = dev;
	cmdlst_priv_init();
	logi("-");
	return 0;
free_dev:
	kfree(dev);
	return -ENOMEM;
}

void hipp_adapter_remove(void)
{
	hipp_adapter_s *dev = g_hipp_adapter;
	int ret;
	unsigned int i = 0;
	logi("+");

	if (dev == NULL) {
		loge("Failed : drvdata NULL");
		return;
	}

	ret = hipp_common_unregister(IPP_UNIT);
	if (ret != 0)
		loge("Failed: IPP unregister.%d", ret);

	if (dev->ippdrv != NULL)
		dev->ippdrv = NULL;

	for (i = 0; i < REFS_TYP_MAX; i++)
		dev->hipp_refs[i] = 0;

	mutex_destroy(&dev->hipp_refs_lock);
	mutex_destroy(&dev->cpe_rate_lock);
	mutex_destroy(&dev->mode_working_lock);
	cmdlst_priv_uninit();
	kfree(dev);
	logi("-");
}
// ---- END FILE ----

