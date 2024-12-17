#include "hidprx_ctrl.h"
#include <securec.h>
#include "dkmd_rx.h"
#include "hidprx_sdp.h"
#include "dsc_algorithm.h"
#include "dkmd_rx.h"
#include "hidprx.h"
#include "hidprx_reg.h"

int dprx_ctrl_wait_vactive_start(struct rx_interface *rx, void __user *argp)
{
	int flag = 0;

	if ((rx->wait_vactive_start == NULL) ||
		(rx->wait_vactive_start(rx, &flag))) {
		dprx_pr_err("wait vactive fail\n");
		return -1;
	}

	if (copy_to_user(argp, &flag, sizeof(flag)) != 0) {
		dprx_pr_err("copy flag to user fail, flag=%d", flag);
		return -1;
	}
	return 0;
}

int dprx_ctrl_enable_vactive_start(struct rx_interface *rx, void __user *argp)
{
	int enable = 0;

	if (copy_from_user(&enable, argp, sizeof(enable)) != 0) {
		dprx_pr_err("copy from user fail");
		return -EINVAL;
	}

	if ((rx->enable_vactive_start == NULL) ||
		(rx->enable_vactive_start(rx, enable))) {
		dprx_pr_err("enable vactive fail\n");
		return -EINVAL;
	}

	dprx_pr_info("enable(%d) vactive success\n", enable);

	return 0;
}

int dprx_ctrl_get_timinginfo(struct rx_interface *rx, void __user *argp)
{
	struct disp_input_timing timing = {0};
	dprx_pr_info("get timing start +\n");
	if ((rx->get_timing_info == NULL) ||
		(rx->get_timing_info(rx, &timing))) {
		dprx_pr_err("get timing fail\n");
		return -1;
	}

	if (copy_to_user(argp, &timing, sizeof(timing)) != 0) {
		dprx_pr_err("copy timing to user fail");
		return -1;
	}

	dprx_pr_info("get timing success -\n");

	return 0;
}

int dprx_video_wait_vactive_start(struct rx_interface *rx, int *flag)
{
	int max_cnt = 1;
	struct dpu_dprx_device *dprx_dev = container_of(rx, struct dpu_dprx_device, interface_impl);

	if (dprx_dev == NULL) {
		dprx_pr_err("dprx_dev null\n");
		return -1;
	}

	*flag = 0;
	while (!(dprx_dev->dprx.vactive_flag)) {
		max_cnt--;
		if (max_cnt <= 0)
			return 0;

		msleep(1);
	}

	dprx_dev->dprx.vactive_flag = 0;
	*flag = 1;

	return 0;
}

int dprx_video_enable_vactive_start(struct rx_interface *rx, int enable)
{
	uint32_t val = enable ? 1 : 0;
	struct dpu_dprx_device *dprx_dev = container_of(rx, struct dpu_dprx_device, interface_impl);

	if (dprx_dev == NULL) {
		dprx_pr_err("hdmirx null\n");
		return -1;
	}

	dprx_dev->dprx.vactive_flag = 0;

	return 0;
}

void dprx_video_set_pps_info(struct disp_input_timing *timing)
{
	if (memcpy_s(&timing->dsc.pps, sizeof(struct disp_input_dsc_pps), dsc_cfg, sizeof(struct drm_dsc_config)) != EOK) {
		dprx_pr_err("memcpy_s fail.\n");
		return;
	}
}

static struct dprx_format2dkmd {
	enum pixel_enc_type pixel_encode;
	enum color_depth depth;
	enum disp_input_pixel_encoding input_pixel;
	enum disp_input_color_depth input_depth;
};
static struct dprx_format2dkmd g_dp2dkmd_fmt[] = {
	{RGB,      COLOR_DEPTH_6,  PIXEL_ENCODING_RGB, COLOR_DEPTH_24BIT},
	{RGB,      COLOR_DEPTH_8,  PIXEL_ENCODING_RGB, COLOR_DEPTH_24BIT},
	{RGB,      COLOR_DEPTH_10, PIXEL_ENCODING_RGB, COLOR_DEPTH_30BIT},
	{YCBCR444, COLOR_DEPTH_8,  PIXEL_ENCODING_YUV444, COLOR_DEPTH_24BIT},
	{YCBCR444, COLOR_DEPTH_10, PIXEL_ENCODING_YUV444, COLOR_DEPTH_30BIT},
	{YCBCR422, COLOR_DEPTH_8,  PIXEL_ENCODING_YUV422, COLOR_DEPTH_24BIT},
	{YCBCR422, COLOR_DEPTH_10, PIXEL_ENCODING_YUV422, COLOR_DEPTH_30BIT},
	{YCBCR420, COLOR_DEPTH_8,  PIXEL_ENCODING_YUV420, COLOR_DEPTH_24BIT},
	{YCBCR420, COLOR_DEPTH_10, PIXEL_ENCODING_YUV420, COLOR_DEPTH_30BIT},
};

void dprx_video_set_pixel_info(struct dprx_ctrl *dprx_ctrl, struct disp_input_timing *timing)
{
	int i;
	int size;
	timing->width = dprx_ctrl->msa.h_width;
	timing->height = dprx_ctrl->msa.v_height;
	timing->dsc.dsc_enable = dprx_ctrl->dsc_en;
	size = sizeof(g_dp2dkmd_fmt) / sizeof(g_dp2dkmd_fmt[0]);
	for (i = 0; i < size; i++) {
		if ((g_dp2dkmd_fmt[i].pixel_encode == dprx_ctrl->v_params.pix_enc) &&
			(g_dp2dkmd_fmt[i].depth == dprx_ctrl->v_params.bpc)) {
			timing->pixel_encoding = g_dp2dkmd_fmt[i].input_pixel;
			timing->depth = g_dp2dkmd_fmt[i].input_depth;
			dprx_pr_info("pixel : %u, %u \n", timing->pixel_encoding, timing->depth);
		}
	}
}

int dprx_video_get_timinginfo(struct rx_interface *rx, struct disp_input_timing *timing)
{
	dprx_pr_info("dprx_video_get_timinginfo +\n");
	struct dpu_dprx_device *dprx_dev = container_of(rx, struct dpu_dprx_device, interface_impl);

	dprx_video_set_pixel_info(&dprx_dev->dprx, timing);
	dprx_pr_info("dsc_en : %u \n", dprx_dev->dprx.dsc_en);
	if (dprx_dev->dprx.dsc_en)
		dprx_video_set_pps_info(timing);

	dprx_pr_info("dprx_video_get_timinginfo -\n");
	return 0;
}

void dprx_ctrl_interface_init(struct rx_interface *impl)
{
	impl->wait_vactive_start = dprx_video_wait_vactive_start;
	impl->enable_vactive_start = dprx_video_enable_vactive_start;
	impl->get_timing_info = dprx_video_get_timinginfo;

	return;
}

void dprx_phy_ctrl_reg_resistance_high(struct dprx_ctrl *dprx)
{
	dprx_pr_info(" +\n");
	void __iomem *phy_base;
	phy_base = dprx->base + DPRX_PHY_REG_OFFSET;
	outp32(phy_base + REG_T_DA_14, 0xffffffff);
	outp32(phy_base + REG_T_DA_15, 0x0);
	outp32(phy_base + DA_TEST_EN, 0x2000);
	outp32(phy_base + DA_TEST_EN_2, 0x20000);
	dprx_pr_info(" -\n");
}

void dprx_phy_set_resistance_high_state(struct dprx_ctrl *dprx)
{
	dprx_pr_info(" +\n");
	uint32_t reg = 0;
	if (dprx->id == 0) {
		reg = inp32(dprx->hsdt1_crg_base + DPRX0_DIS_RESET_OFFSET); /* 0xEB045000 + 0x64 */
		reg |= DPRX0_RST_HIDPRX0PHY_POR_BIT;
		reg |= DPRX0_IP_PRST_HIDPRX0PHY_BIT;
		reg |= DPRX0_APB_DIS_RESET_BIT;
		reg |= DPRX0_APB_BRIDGE_DIS_RESET_BIT;
		outp32(dprx->hsdt1_crg_base + DPRX0_DIS_RESET_OFFSET, reg);
		dprx_pr_info(" rx0\n");
		reg = inp32(dprx->hsdt1_crg_base + DPRX0_CLK_GATE_OFFSET); /* 0xEB045000 + 0x10 */
		reg |= DPRX0_GT_CLK_HIDPRX0PHY_REF;
		reg |= DPRX0_APB_CLK_GATE_BIT;
		reg |= DPRX0_GT_PCLK_HIDPRX0PHY;
		reg |= DPRX0_APB_BRIDGE_CLK_GATE_BIT;
		outp32(dprx->hsdt1_crg_base + DPRX0_CLK_GATE_OFFSET, reg);
	} else {
		reg = inp32(dprx->hsdt1_crg_base + DPRX1_DIS_RESET_OFFSET); /* 0xEB045000 + 0x64 */
		reg |= DPRX0_RST_HIDPRX1PHY_POR_BIT;
		reg |= DPRX0_IP_PRST_HIDPRX1PHY_BIT;
		reg |= DPRX1_APB_DIS_RESET_BIT;
		reg |= DPRX0_APB_BRIDGE_DIS_RESET_BIT;
		outp32(dprx->hsdt1_crg_base + DPRX1_DIS_RESET_OFFSET, reg);
		dprx_pr_info(" rx1\n");
		reg = inp32(dprx->hsdt1_crg_base + DPRX1_CLK_GATE_OFFSET); /* 0xEB045000 + 0x10 */
		reg |= DPRX1_GT_CLK_HIDPRX1PHY_REF;
		reg |= DPRX1_APB_CLK_GATE_BIT;
		reg |= DPRX1_GT_PCLK_HIDPRX1PHY;
		reg |= DPRX1_APB_BRIDGE_CLK_GATE_BIT;
		outp32(dprx->hsdt1_crg_base + DPRX1_CLK_GATE_OFFSET, reg);
	}
	dprx_phy_ctrl_reg_resistance_high(dprx);
	dprx_pr_info(" -\n");
}