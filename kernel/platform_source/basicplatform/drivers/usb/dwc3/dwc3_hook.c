/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: hooks for DesignWare USB3 DRD Controller
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2  of
 * the License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/version.h>
#include <linux/of.h>
#include <linux/device.h>
#undef CREATE_TRACE_POINTS
#include <trace/hooks/hck_hook/hck_usb.h>
#include "io.h"
#include "dwc3_hook.h"

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
static void dwc3_get_hisi_quirks(void *data, struct dwc3 *dwc)
{
	struct device *dev = dwc->dev;

	dwc->dis_u1u2_quirk = device_property_read_bool(dev,
				"snps,dis_u1u2_quirk");
	dwc->gctl_reset_quirk = device_property_read_bool(dev,
				"snps,gctl-reset-quirk");
	dwc->dis_pipe_clk_quirk = device_property_read_bool(dev,
				"snps,dis-pipe-clk-quirk");
	dwc->en_host_u2_susphy_quirk = device_property_read_bool(dev,
				"snps,en-host-u2-susphy-quirk");
	dwc->dis_dcfg_lpm = device_property_read_bool(dev,
				"snps,dis-dcfg-lpm");
	dwc->pullup_ulpi_reset_phy_quirk = device_property_read_bool(dev,
				"snps,pullup-ulpi-reset-phy-quirk");
	dwc->filter_se0_fsls_quirk = device_property_read_bool(dev,
				"snps,filter-se0-fsls-quirk");
	dwc->loa_filter_en_quirk = device_property_read_bool(dev,
				"snps,loa-filter-en-quirk");
	dwc->support_s3_wakeup = device_property_read_bool(dev,
				"snps,xhci-support-s3-wakeup");
	dwc->enable_p4_gate = device_property_read_bool(dev,
			      "snps,enable-p4-gate");
	dwc->clear_svc_opp_per_hs = device_property_read_bool(dev,
				    "snps,clear-svc-opp-per-hs");
	dwc->disable_rx_thres = device_property_read_bool(dev,
				"snps,disable-rx-thres");
	dwc->set_svc_opp_per_hs_sep = device_property_read_bool(dev,
				"snps,set-svc-opp-per-hs-sep");
	dwc->adjust_dtout = device_property_read_bool(dev,
				"snps,adjust-dtout");
	dwc->force_disable_host_lpm = device_property_read_bool(dev,
				"snps,force-disable-host-lpm");
	dwc->enable_hst_imm_retry = device_property_read_bool(dev,
				"snps,enable-hst-imm-retry");

	dev_info(dev, "%s: %d%d%d%d-%d%d%d%d-%d%d%d%d-%d%d%d%d\n", __func__,
			dwc->dis_u1u2_quirk, dwc->gctl_reset_quirk,
			dwc->dis_pipe_clk_quirk, dwc->en_host_u2_susphy_quirk,
			dwc->dis_dcfg_lpm, dwc->pullup_ulpi_reset_phy_quirk,
			dwc->filter_se0_fsls_quirk, dwc->loa_filter_en_quirk,
			dwc->support_s3_wakeup, dwc->enable_p4_gate,
			dwc->clear_svc_opp_per_hs, dwc->disable_rx_thres,
			dwc->set_svc_opp_per_hs_sep, dwc->adjust_dtout,
			dwc->force_disable_host_lpm, dwc->enable_hst_imm_retry);
}

static void dwc3_global_control_quirks(void *data, struct dwc3 *dwc)
{
	u32 reg;

	dev_info(dwc->dev, "%s\n", __func__);

	if (dwc->dis_pipe_clk_quirk) {
		dev_info(dwc->dev, "Disable Pipe Clk\n");
		reg = dwc3_readl(dwc->regs, DWC3_GUCTL1);
		reg |= DWC3_GUCTL1_DISREFCLKGTNG_BIT | DWC3_GUCTL1_RESERVED26;
		dwc3_writel(dwc->regs, DWC3_GUCTL1, reg);
	}

	if (dwc->filter_se0_fsls_quirk) {
		dev_info(dwc->dev, "Filter SE0 FSLS quirk\n");
		reg = dwc3_readl(dwc->regs, DWC3_GUCTL1);
		reg |= DWC3_GUCTL1_FILTER_SE0_FSLS_EOP;
		dwc3_writel(dwc->regs, DWC3_GUCTL1, reg);
	}

	if (dwc->loa_filter_en_quirk) {
		dev_info(dwc->dev, "LOA filter en quirk\n");
		reg = dwc3_readl(dwc->regs, DWC3_GUCTL1);
		reg |= DWC3_GUCTL1_LOA_FILTER_EN;
		dwc3_writel(dwc->regs, DWC3_GUCTL1, reg);
	}
}

static void dwc3_enable_p4_gate(struct dwc3 *dwc)
{
	u32 reg;

	/* BugNo: 9001169999 */
	reg = dwc3_readl(dwc->regs, DWC3_LLUCTL(0));
	reg |= DWC3_LLUCTL_SUPPORT_P4_PG;
	dwc3_writel(dwc->regs, DWC3_LLUCTL(0), reg);
}

static void dwc3_device_quirks(void *data, struct dwc3 *dwc)
{
	dev_info(dwc->dev, "%s\n", __func__);

	if (dwc->enable_p4_gate)
		dwc3_enable_p4_gate(dwc);
}

static void dwc3_set_svc_opp_per_hs_sep(struct dwc3 *dwc)
{
	u32 reg;

	/* BugNo: 9001212079 */
	reg = dwc3_readl(dwc->regs, DWC3_GUCTL3);
	reg &= ~DWC3_GUCTL3_SVC_OPP_PER_HS_SEP_MASK;
	reg |= DWC3_GUCTL3_SVC_OPP_PER_HS_SEP(3);
	dwc3_writel(dwc->regs, DWC3_GUCTL3, reg);
}

static void dwc3_enable_hst_imm_retry(struct dwc3 *dwc)
{
	u32 reg;

	/* BugNo: 9001162113 */
	reg = dwc3_readl(dwc->regs, DWC3_GUCTL);
	reg |= DWC3_GUCTL_USBHSTINIMMRETRYEN;
	dwc3_writel(dwc->regs, DWC3_GUCTL, reg);
}

static void dwc3_force_disable_host_lpm(struct dwc3 *dwc)
{
	u32 reg;

	/* BugNo: 9001238552 */
	reg = dwc3_readl(dwc->regs, DWC3_GUCTL1);
	reg |= (DWC3_GUCTL1_HW_LPM_CAP_DISABLE | DWC3_GUCTL1_HW_LPM_HLE_DISABLE);
	dwc3_writel(dwc->regs, DWC3_GUCTL1, reg);
}

static void dwc3_adjust_dtout(struct dwc3 *dwc)
{
	u32 reg;

	/* BugNo: 9001227814 */
	reg = dwc3_readl(dwc->regs, DWC3_GUCTL);
	reg &= ~DWC3_GUCTL_DTOUT_MASK;
	reg |= DWC3_GUCTL_DTOUT(8);
	dwc3_writel(dwc->regs, DWC3_GUCTL, reg);
}

static void dwc3_disable_rx_thres_cfg(struct dwc3 *dwc)
{
	u32 reg;

	/* BugNo: 9001208988 */
	reg = dwc3_readl(dwc->regs, DWC3_GRXTHRCFG);
	reg &= ~DWC3_GRXTHRCFG_USBRXPKTCNTSEL;
	dwc3_writel(dwc->regs, DWC3_GRXTHRCFG, reg);
}

static void dwc3_clear_svc_opp_per_hs(struct dwc3 *dwc)
{
	u32 reg;

	/* BugNo: 9001205968 */
	reg = dwc3_readl(dwc->regs, DWC31_GUCTL2);
	reg &= ~DWC31_GUCTL2_SVC_OPP_PER_HS_MASK;
	dwc3_writel(dwc->regs, DWC31_GUCTL2, reg);
}

static void dwc3_enable_usb2phy_suspend(struct dwc3 *dwc)
{
	u32 reg;

	dev_info(dwc->dev, "%s: enable u2 phy suspend in host\n", __func__);
	reg = dwc3_readl(dwc->regs, DWC3_GUSB2PHYCFG(0));
	reg |= DWC3_GUSB2PHYCFG_SUSPHY;
	dwc3_writel(dwc->regs, DWC3_GUSB2PHYCFG(0), reg);
}

static void dwc3_otg_split_disable(struct dwc3 *dwc)
{
	u32 reg;

	/* step 2: set SPLIT boundary disable */
	reg = dwc3_readl(dwc->regs, DWC3_GUCTL3);
	reg |= (DWC3_GUCTL3_SPLITDISABLE);
	dwc3_writel(dwc->regs, DWC3_GUCTL3, reg);

	dev_info(dwc->dev, "DWC3_GUCTL3 [%x = 0x%x]\n", DWC3_GUCTL3, reg);
}

static void dwc3_host_quirks(void *data, struct dwc3 *dwc)
{
	dev_info(dwc->dev, "%s\n", __func__);

	if (dwc->dis_split_quirk)
		dwc3_otg_split_disable(dwc);

	if (dwc->en_host_u2_susphy_quirk)
		dwc3_enable_usb2phy_suspend(dwc);

	if (dwc->clear_svc_opp_per_hs)
		dwc3_clear_svc_opp_per_hs(dwc);

	if (dwc->disable_rx_thres)
		dwc3_disable_rx_thres_cfg(dwc);

	if (dwc->set_svc_opp_per_hs_sep)
		dwc3_set_svc_opp_per_hs_sep(dwc);

	if (dwc->adjust_dtout)
		dwc3_adjust_dtout(dwc);

	if (dwc->force_disable_host_lpm)
		dwc3_force_disable_host_lpm(dwc);

	if (dwc->enable_hst_imm_retry)
		dwc3_enable_hst_imm_retry(dwc);
}

void dwc3_register_hooks(void)
{
	register_trace_hck_vh_dwc3_get_quirks(dwc3_get_hisi_quirks, NULL);
	register_trace_hck_vh_dwc3_global_quirks(dwc3_global_control_quirks, NULL);
	register_trace_hck_vh_dwc3_device_quirks(dwc3_device_quirks, NULL);
	register_trace_hck_vh_dwc3_host_quirks(dwc3_host_quirks, NULL);
}
#else
void dwc3_register_hooks(void) {}
#endif

