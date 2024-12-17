/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name    segment_ipp_adapter_sub.c
 * Description:
 *
 * Date         2020-04-17 16:28:10
 ********************************************************************/
#include <linux/module.h>
#include <linux/platform_device.h>
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
#include "ipp_top_reg_offset.h"

enum HIPP_CLK_STATUS {
	IPP_CLK_DISABLE = 0,
	IPP_CLK_EN = 1,
	IPP_CLK_STATUS_MAX
};

int hipp_orcm_clk_enable(void)
{
	void __iomem *crg_base;
	logd("+");
	crg_base = hipp_get_regaddr(CPE_TOP);
	if (crg_base == NULL) {
		loge("Failed : hipp_get_regaddr");
		return -EINVAL;
	}

	writel(IPP_CLK_EN, crg_base + IPP_TOP_ENH_CRG_CFG0_REG);
	writel(IPP_CLK_EN, crg_base + IPP_TOP_ARF_CRG_CFG0_REG);
	writel(IPP_CLK_EN, crg_base + IPP_TOP_RDR_CRG_CFG0_REG);
	writel(IPP_CLK_EN, crg_base + IPP_TOP_CMP_CRG_CFG0_REG);
	writel(IPP_CLK_EN, crg_base + IPP_TOP_MC_CRG_CFG0_REG);
	writel(IPP_CLK_EN, crg_base + IPP_TOP_KLT_CRG_CFG0_REG);
	logd("-");
	return ISP_IPP_OK;
}

int hipp_orcm_clk_disable(void)
{
	void __iomem *crg_base;
	logd("+");
	crg_base = hipp_get_regaddr(CPE_TOP);
	if (crg_base == NULL) {
		loge("Failed : hipp_get_regaddr");
		return -EINVAL;
	}

	writel(IPP_CLK_DISABLE, crg_base + IPP_TOP_ENH_CRG_CFG0_REG);
	writel(IPP_CLK_DISABLE, crg_base + IPP_TOP_ARF_CRG_CFG0_REG);
	writel(IPP_CLK_DISABLE, crg_base + IPP_TOP_RDR_CRG_CFG0_REG);
	writel(IPP_CLK_DISABLE, crg_base + IPP_TOP_CMP_CRG_CFG0_REG);
	writel(IPP_CLK_DISABLE, crg_base + IPP_TOP_MC_CRG_CFG0_REG);
	writel(IPP_CLK_DISABLE, crg_base + IPP_TOP_KLT_CRG_CFG0_REG);
	logd("-");
	return ISP_IPP_OK;
}

// -------------End File-------------

