/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name    cvdr_drv.c
 * Description:
 *
 * Date         2022-07-13
 ******************************************************************/
#include <linux/printk.h>
#include "cvdr_drv.h"
#include "cvdr_ipp_drv_priv.h"
#include "cvdr_ipp_reg_offset.h"
#include "cvdr_ipp_reg_offset_field.h"
#include "cmdlst_manager.h"

#define LOG_TAG LOG_MODULE_CVDR_DRV

#define VP_WR_REG_OFFSET 0x1C
#define VP_RD_REG_OFFSET 0x20
#define NR_RD_REG_OFFSET 0xC
#define NR_WR_REG_OFFSET 0xC
#define ONE_REG_OFFSET   0x4
#define CVDR_SPARE_NUM   4

/********************************************************************************************
 * CVDR Raster Port ID; Please refer to HiStar_v350R02_ipp_subsys_BW.xlsx
 ********************************************************************************************/
unsigned int g_cvdr_vp_wr_id[IPP_VP_WR_MAX] = {
	[IPP_VP_WR_CMDLST]          = 0,
	[IPP_VP_WR_ORB_ENH_Y]       = 3,
	[IPP_VP_WR_ARF_PRY_1]       = 12,
	[IPP_VP_WR_ARF_PRY_2]       = 13,
	[IPP_VP_WR_ARF_DOG_0]       = 14,
	[IPP_VP_WR_ARF_DOG_1]       = 15,
	[IPP_VP_WR_ARF_DOG_2]       = 16,
	[IPP_VP_WR_ARF_DOG_3]       = 17,
	[IPP_VP_WR_ARF_PYR_2_DS]    = 18,
	[IPP_VP_WR_MC_RESULT]       = 19,
	[IPP_VP_WR_KLT_NEXT_LAYER0] = 22,
	[IPP_VP_WR_KLT_NEXT_LAYER1] = 23,
	[IPP_VP_WR_KLT_NEXT_LAYER2] = 24,
	[IPP_VP_WR_KLT_NEXT_LAYER3] = 25,
	[IPP_VP_WR_KLT_NEXT_LAYER4] = 26,
};

unsigned int g_cvdr_vp_rd_id[IPP_VP_RD_MAX] = {
	[IPP_VP_RD_CMDLST]          = 0,
	[IPP_VP_RD_ORB_ENH_Y_HIST]  = 6,
	[IPP_VP_RD_ORB_ENH_Y_IMAGE] = 7,
	[IPP_VP_RD_RDR]             = 9,
	[IPP_VP_RD_CMP]             = 10, // REF
	[IPP_VP_RD_ARF_0]           = 12,
	[IPP_VP_RD_ARF_1]           = 13,
	[IPP_VP_RD_ARF_2]           = 14,
	[IPP_VP_RD_ARF_3]           = 15,
	[IPP_VP_RD_KLT_SRC]         = 22,
};

unsigned int g_cvdr_nr_wr_id[IPP_NR_WR_MAX] = {
	[IPP_NR_WR_RDR]            = 6,
};

unsigned int g_cvdr_nr_rd_id[IPP_NR_RD_MAX] = {
	[IPP_NR_RD_MC]             = 2,
	[IPP_NR_RD_CMP]            = 3, // CUR
	[IPP_NR_RD_KLT_PREV_FRAME] = 5,
	[IPP_NR_RD_KLT_NEXT_FRAME] = 6,
};

/********************************************************************************************
 * function name: cvdr_set_debug_enable
 *
 * description:
 *     set cvdr debug enable
 *
 * input:
 *     dev        : cvdr device
 *     wr_peak_en : enable the FIFO peak functionality over the write port
 *     rd_peak_en : enable the FIFO peak functionality over the read port
 *
 * output:
 *     0  : success
 *     -1 : failed
 ********************************************************************************************/
static int cvdr_set_debug_enable(cvdr_dev_t *dev,
									unsigned char wr_peak_en, unsigned char rd_peak_en)
{
	union u_cvdr_ipp_cvdr_debug_en tmp;
	tmp.u32 = 0;
	tmp.bits.wr_peak_en = wr_peak_en;
	tmp.bits.rd_peak_en = rd_peak_en;
	hispcpe_reg_set(CVDR_REG, CVDR_IPP_CVDR_IPP_CVDR_DEBUG_EN_REG, tmp.u32);
	return ISP_IPP_OK;
}

/**********************************************************
 * function name: cvdr_get_debug_info
 *
 * description:
 *     get cvdr debug info
 *
 * input:
 *     dev     : cvdr device
 *     wr_peak : peak number of Data Units used for the write functionality
 *     rd_peak : peak number of Data Units used for the read functionality
 *
 * output:
 *     0  : success
 *     -1 : failed
 ***********************************************************/
static int cvdr_get_debug_info(cvdr_dev_t *dev,
									unsigned char *wr_peak, unsigned char *rd_peak)
{
	union u_cvdr_ipp_cvdr_debug tmp;
	tmp.u32 = hispcpe_reg_get(CVDR_REG, CVDR_IPP_CVDR_IPP_CVDR_DEBUG_REG);
	*wr_peak = tmp.bits.wr_peak;
	*rd_peak = tmp.bits.rd_peak;
	return ISP_IPP_OK;
}

/**********************************************************
 * function name: cvdr_set_nr_wr_config
 *
 * description:
 *     config cvdr non-raster write port(nr_wr) configurations
 *
 * input:
 *     dev  : cvdr device
 *     port : nr write port number
 *     en   : enable or bypass nr write port
 *
 * output:
 *     0  : success
 *     -1 : failed
 ***********************************************************/
static int cvdr_set_nr_wr_config(cvdr_dev_t *dev,
									unsigned int port, unsigned char nr_wr_stop,
									unsigned char en)
{
	unsigned int reg_addr = 0;
	union u_cvdr_ipp_nr_wr_cfg_6 tmp;

	switch (port) {
	case IPP_NR_WR_RDR:
		reg_addr = CVDR_IPP_CVDR_IPP_NR_WR_CFG_6_REG;
		break;
	default:
		loge("error, port id = %d", port);
		return ISP_IPP_ERR;
	}

	tmp.u32 = 0;
	tmp.bits.nr_wr_stop_6  = nr_wr_stop;
	tmp.bits.nrwr_enable_6 = en;

	macro_cmdlst_set_reg(dev->base_addr + reg_addr, tmp.u32);
	logd("NR_WR_CFG: port = %d, 0x%x = 0x%x, enable = %d", port, dev->base_addr + reg_addr, tmp.u32, en);

	return ISP_IPP_OK;
}

/**********************************************************
 * function name: cvdr_set_nr_rd_config
 *
 * description:
 *     config ipp cvdr non-raster read port(nr_rd) configurations
 *
 * input:
 *     dev  : cvdr device
 *     port : nr read port number
 *     du   : number of allocated DUs
 *     en   : enable or bypass nr read port
 *
 * output:
 *     0  : success
 *     -1 : failed
 ***********************************************************/
static int cvdr_set_nr_rd_config(cvdr_dev_t *dev,
									unsigned int port, unsigned short du,
									unsigned char nr_rd_stop, unsigned char en)
{
	unsigned int reg_addr = 0;
	union u_cvdr_ipp_nr_rd_cfg_2 tmp;

	switch (port) {
	case IPP_NR_RD_MC:
		reg_addr = CVDR_IPP_CVDR_IPP_NR_RD_CFG_2_REG;
		break;
	case IPP_NR_RD_CMP:
		reg_addr = CVDR_IPP_CVDR_IPP_NR_RD_CFG_3_REG;
		break;
	case IPP_NR_RD_KLT_PREV_FRAME:
		reg_addr = CVDR_IPP_CVDR_IPP_NR_RD_CFG_5_REG;
		break;
	case IPP_NR_RD_KLT_NEXT_FRAME:
		reg_addr = CVDR_IPP_CVDR_IPP_NR_RD_CFG_6_REG;
		break;
	default:
		loge("error, port id = %d", port);
		return ISP_IPP_ERR;
	}

	tmp.u32 = 0;
	tmp.bits.nrrd_allocated_du_2 = du;
	tmp.bits.nr_rd_stop_2        = nr_rd_stop;
	tmp.bits.nrrd_enable_2       = en;

	macro_cmdlst_set_reg(dev->base_addr + reg_addr, tmp.u32);
	logd("NR_RD_CFG: port=%d, 0x%x = 0x%x, allocated_du = %d, enable = %d",
		 port, dev->base_addr + reg_addr, tmp.u32, du, en);

	return ISP_IPP_OK;
}

static int cvdr_nr_do_config(cvdr_dev_t *dev, cfg_tab_cvdr_t *table)
{
	unsigned int i = 0;
	int ret = 0;

	if (dev == NULL || table == NULL) {
		loge("input param is invalid!!");
		return ISP_IPP_ERR;
	}

	for (i = 0; i < IPP_NR_WR_MAX; ++i) {
		if (table->nr_wr_cfg[i].to_use == 1) {
			ret = cvdr_set_nr_wr_config(dev, i,
										table->nr_wr_cfg[i].nr_wr_stop,
										table->nr_wr_cfg[i].en);
			if (ret) {
				loge("Failed: cvdr_set_nr_wr_config");
				return ISP_IPP_ERR;
			}
			table->nr_wr_cfg[i].to_use = 0;
		}
	}

	for (i = 0; i < IPP_NR_RD_MAX ; ++i) {
		if (table->nr_rd_cfg[i].to_use == 1) {
			ret = cvdr_set_nr_rd_config(dev, i,
										table->nr_rd_cfg[i].allocated_du,
										table->nr_rd_cfg[i].nr_rd_stop,
										table->nr_rd_cfg[i].en);
			if (ret) {
				loge("Failed: cvdr_set_nr_wr_config");
				return ISP_IPP_ERR;
			}
			table->nr_rd_cfg[i].to_use = 0;
		}
	}

	return ISP_IPP_OK;
}

static void cvdr_vp_wr_config_cfg(cvdr_dev_t *dev,
									unsigned int port, cfg_tab_cvdr_t *table)
{
	union u_cvdr_ipp_vp_wr_sid_cfg_sid0_0 tmp_cfg;
	cvdr_wr_fmt_desc_t *desc = &(table->vp_wr_cfg[port].fmt);

	tmp_cfg.u32  = 0;
	tmp_cfg.bits.vpwr_pixel_format_sid0_0    = desc->pix_fmt;
	tmp_cfg.bits.vpwr_pixel_expansion_sid0_0 = desc->pix_expan;
	tmp_cfg.bits.vpwr_last_page_sid0_0       = desc->last_page;
	macro_cmdlst_set_reg(dev->base_addr +
						 CVDR_IPP_CVDR_IPP_VP_WR_SID_CFG_SID0_0_REG +
						 g_cvdr_vp_wr_id[port] * IPP_VP_WR_REG_OFFSET,
						 tmp_cfg.u32);

	logd("VP_WR_SID_CFG: 0x%x, pix_fmt = %d, pix_expan = %d, last_page = 0x%x, port=%d",
		 dev->base_addr + CVDR_IPP_CVDR_IPP_VP_WR_SID_CFG_SID0_0_REG +
		 g_cvdr_vp_wr_id[port] * IPP_VP_WR_REG_OFFSET,
		 desc->pix_fmt, desc->pix_expan, desc->last_page, port);
}

static void cvdr_vp_wr_config_axi_line(cvdr_dev_t *dev,
									unsigned int port, cfg_tab_cvdr_t *table)
{
	union u_cvdr_ipp_vp_wr_sid_axi_line_sid0_0 tmp_line;
	cvdr_wr_fmt_desc_t *desc = &(table->vp_wr_cfg[port].fmt);

	tmp_line.u32 = 0;
	tmp_line.bits.vpwr_line_stride_sid0_0      = desc->line_stride;
	tmp_line.bits.vpwr_line_wrap_sid0_0        = desc->line_wrap;
	tmp_line.bits.vpwr_line_start_wstrb_sid0_0 = (desc->line_start_wstrb == 0) ?
												0xF : desc->line_start_wstrb;

	macro_cmdlst_set_reg(dev->base_addr +
						 CVDR_IPP_CVDR_IPP_VP_WR_SID_AXI_LINE_SID0_0_REG +
						 g_cvdr_vp_wr_id[port] * IPP_VP_WR_REG_OFFSET,
						 tmp_line.u32);

	logd("VP_WR_SID_AXI_LINE: 0x%x, line_stride = %d, line_wrap = %d, port = %d",
		dev->base_addr + CVDR_IPP_CVDR_IPP_VP_WR_SID_AXI_LINE_SID0_0_REG +
		g_cvdr_vp_wr_id[port] * IPP_VP_WR_REG_OFFSET,
		desc->line_stride, desc->line_wrap, port);
}

static void cvdr_vp_wr_config_if(cvdr_dev_t *dev,
									unsigned int port, cfg_tab_cvdr_t *table)
{
	union u_cvdr_ipp_vp_wr_if_cfg_0 tmp_if_cfg;
	tmp_if_cfg.u32 = 0;
	tmp_if_cfg.bits.vpwr_prefetch_bypass_0 = 1;
	macro_cmdlst_set_reg(dev->base_addr +
						 CVDR_IPP_CVDR_IPP_VP_WR_IF_CFG_0_REG +
						 g_cvdr_vp_wr_id[port] * IPP_VP_WR_REG_OFFSET,
						 tmp_if_cfg.u32);
}

static void cvdr_vp_wr_config_limiter(cvdr_dev_t *dev,
									unsigned int port, cfg_tab_cvdr_t *table)
{
	union u_cvdr_ipp_vp_wr_limiter_0 tmp_limiter;
	tmp_limiter.u32 = 0;
	tmp_limiter.bits.vpwr_access_limiter_0_0 = 0xF;
	tmp_limiter.bits.vpwr_access_limiter_1_0 = 0xF;
	tmp_limiter.bits.vpwr_access_limiter_2_0 = 0xF;
	tmp_limiter.bits.vpwr_access_limiter_3_0 = 0xF;
	tmp_limiter.bits.vpwr_access_limiter_reload_0 = 0xF;
	macro_cmdlst_set_reg(dev->base_addr +
						 CVDR_IPP_CVDR_IPP_VP_WR_LIMITER_0_REG +
						 g_cvdr_vp_wr_id[port] * IPP_VP_WR_REG_OFFSET,
						 tmp_limiter.u32);
}

static void cvdr_vp_wr_config_axi_fs(cvdr_dev_t *dev,
									unsigned int port, cfg_tab_cvdr_t *table)
{
	union u_cvdr_ipp_vp_wr_sid_axi_fs_sid0_0 tmp_fs;
	cvdr_wr_fmt_desc_t *desc = &(table->vp_wr_cfg[port].fmt);
	tmp_fs.u32 = 0;
	tmp_fs.bits.vpwr_address_frame_start_sid0_0 = (desc->fs_addr >> 2) >> 2;

	if (port == IPP_VP_WR_ARF_PRY_1 && table->arf_updata_wr_addr_flag == 1) {
		macro_cmdlst_set_addr_align(CVDR_ALIGN_BYTES);
		table->arf_cvdr_wr_addr = dev->cmd_buf->start_addr_isp_map +
								  dev->cmd_buf->data_size;
		logd("cmd_buf->data_size = 0x%x, table->arf_cvdr_wr_addr = 0x%x",
			dev->cmd_buf->data_size, table->arf_cvdr_wr_addr);

		macro_cmdlst_set_reg_data(CMDLST_PADDING_DATA);
		macro_cmdlst_set_reg_data(CMDLST_PADDING_DATA);
		macro_cmdlst_set_reg(dev->base_addr +
							 CVDR_IPP_CVDR_IPP_VP_WR_SID_AXI_FS_SID0_0_REG +
							 g_cvdr_vp_wr_id[port] * IPP_VP_WR_REG_OFFSET,
							 tmp_fs.u32);
	} else {
		macro_cmdlst_set_reg(dev->base_addr +
							 CVDR_IPP_CVDR_IPP_VP_WR_SID_AXI_FS_SID0_0_REG +
							 g_cvdr_vp_wr_id[port] * IPP_VP_WR_REG_OFFSET,
							 tmp_fs.u32);
	}

	logd("VP_WR_AXI_FS: 0x%x, fs_addr = 0x%x, vpwr_address_frame_start_sid0_0 = 0x%x",
		 dev->base_addr + CVDR_IPP_CVDR_IPP_VP_WR_SID_AXI_FS_SID0_0_REG +
		 g_cvdr_vp_wr_id[port] * IPP_VP_WR_REG_OFFSET,
		 desc->fs_addr, tmp_fs.bits.vpwr_address_frame_start_sid0_0);
}

static void cvdr_vp_rd_config_cfg(cvdr_dev_t *dev,
									unsigned int port, cfg_tab_cvdr_t *table)
{
	union u_cvdr_ipp_vp_rd_cfg_0 tmp_cfg;
	cvdr_rd_fmt_desc_t *desc = &(table->vp_rd_cfg[port].fmt);
	tmp_cfg.u32  = 0;
	tmp_cfg.bits.vprd_pixel_format_0    = desc->pix_fmt;
	tmp_cfg.bits.vprd_pixel_expansion_0 = desc->pix_expan;
	tmp_cfg.bits.vprd_allocated_du_0    = desc->allocated_du;
	tmp_cfg.bits.vprd_last_page_0       = desc->last_page;
	macro_cmdlst_set_reg(dev->base_addr + CVDR_IPP_CVDR_IPP_VP_RD_CFG_0_REG +
						 g_cvdr_vp_rd_id[port] * IPP_VP_RD_REG_OFFSET,
						 tmp_cfg.u32);

	logd("VP_RD_CFG: 0x%x, pix_fmt = %d, pix_expan = %d, allocated_du = %d, last_page = 0x%x",
		 dev->base_addr + CVDR_IPP_CVDR_IPP_VP_RD_CFG_0_REG +
		 g_cvdr_vp_rd_id[port] * IPP_VP_RD_REG_OFFSET,
		 desc->pix_fmt, desc->pix_expan,
		 desc->allocated_du, desc->last_page);
}

static void cvdr_vp_rd_config_lwg(cvdr_dev_t *dev,
									unsigned int port, cfg_tab_cvdr_t *table)
{
	union u_cvdr_ipp_vp_rd_lwg_0 tmp_lwg;
	cvdr_rd_fmt_desc_t *desc = &(table->vp_rd_cfg[port].fmt);
	tmp_lwg.u32  = 0;
	tmp_lwg.bits.vprd_line_size_0 = desc->line_size;
	tmp_lwg.bits.vprd_ihleft_0    = desc->vprd_ihleft; // Should set 0 in IPP; We don't use the cropping function;

	macro_cmdlst_set_reg(dev->base_addr + CVDR_IPP_CVDR_IPP_VP_RD_LWG_0_REG +
						 g_cvdr_vp_rd_id[port] * IPP_VP_RD_REG_OFFSET,
						 tmp_lwg.u32);

	logd("VP_RD_LWG: 0x%x = 0x%x, line_size = %d, vprd_ihleft = %d",
		 dev->base_addr + CVDR_IPP_CVDR_IPP_VP_RD_LWG_0_REG +
		 g_cvdr_vp_rd_id[port] * IPP_VP_RD_REG_OFFSET,
		 tmp_lwg.u32, desc->line_size, desc->vprd_ihleft);
}

static void cvdr_vp_rd_config_fhg(cvdr_dev_t *dev,
									unsigned int port, cfg_tab_cvdr_t *table)
{
	union u_cvdr_ipp_vp_rd_fhg_0 tmp_fhg;
	cvdr_rd_fmt_desc_t *desc = &(table->vp_rd_cfg[port].fmt);
	tmp_fhg.u32  = 0;
	tmp_fhg.bits.vprd_frame_size_0 = desc->frame_size;

	if (port == IPP_VP_RD_RDR || port == IPP_VP_RD_CMP) {
		macro_cmdlst_set_addr_align(CVDR_ALIGN_BYTES);

		if (port == IPP_VP_RD_RDR)
			table->rdr_cvdr_frame_size_addr = dev->cmd_buf->start_addr_isp_map +
											  dev->cmd_buf->data_size;

		if (port == IPP_VP_RD_CMP)
			table->cmp_cvdr_frame_size_addr = dev->cmd_buf->start_addr_isp_map +
											  dev->cmd_buf->data_size;

		macro_cmdlst_set_reg_data(CMDLST_PADDING_DATA);
		macro_cmdlst_set_reg_data(CMDLST_PADDING_DATA);
		macro_cmdlst_set_reg(dev->base_addr +
							 CVDR_IPP_CVDR_IPP_VP_RD_FHG_0_REG +
							 g_cvdr_vp_rd_id[port] * IPP_VP_RD_REG_OFFSET,
							 tmp_fhg.u32);
	} else {
		macro_cmdlst_set_reg(dev->base_addr +
							 CVDR_IPP_CVDR_IPP_VP_RD_FHG_0_REG +
							 g_cvdr_vp_rd_id[port] * IPP_VP_RD_REG_OFFSET,
							 tmp_fhg.u32);
	}

	logd("VP_RD_FHG: 0x%x = 0x%x, frame_size = %d",
		 dev->base_addr + CVDR_IPP_CVDR_IPP_VP_RD_FHG_0_REG +
		 g_cvdr_vp_rd_id[port] * IPP_VP_RD_REG_OFFSET,
		 tmp_fhg.u32, desc->frame_size);
}

static void cvdr_vp_rd_config_axi_line(cvdr_dev_t *dev,
									unsigned int port, cfg_tab_cvdr_t *table)
{
	union u_cvdr_ipp_vp_rd_axi_line_0 tmp_line;
	cvdr_rd_fmt_desc_t *desc  = &(table->vp_rd_cfg[port].fmt);

	tmp_line.u32 = 0;
	tmp_line.bits.vprd_line_stride_0 = desc->line_stride;
	tmp_line.bits.vprd_line_wrap_0   = desc->line_wrap;
	macro_cmdlst_set_reg(dev->base_addr +
						 CVDR_IPP_CVDR_IPP_VP_RD_AXI_LINE_0_REG +
						 g_cvdr_vp_rd_id[port] * IPP_VP_RD_REG_OFFSET,
						 tmp_line.u32);

	logd("VP_RD_AXI_LINE: 0x%x = 0x%x, line_stride = %d, line_wrap = %d",
		 dev->base_addr + CVDR_IPP_CVDR_IPP_VP_RD_AXI_LINE_0_REG +
		 g_cvdr_vp_rd_id[port] * IPP_VP_RD_REG_OFFSET,
		 tmp_line.u32, desc->line_stride, desc->line_wrap);
}

static void cvdr_vp_rd_config_if(cvdr_dev_t *dev,
									unsigned int port, cfg_tab_cvdr_t *table)
{
	union u_cvdr_ipp_vp_rd_if_cfg_0 tmp_if_cfg;
	tmp_if_cfg.u32 = 0;
	tmp_if_cfg.bits.vprd_prefetch_bypass_0 = 1; // 1: disabled pre-fecth;
	macro_cmdlst_set_reg(dev->base_addr +
						 CVDR_IPP_CVDR_IPP_VP_RD_IF_CFG_0_REG +
						 g_cvdr_vp_rd_id[port] * IPP_VP_RD_REG_OFFSET,
						 tmp_if_cfg.u32);
}

static void cvdr_vp_rd_config_limiter(cvdr_dev_t *dev,
									unsigned int port, cfg_tab_cvdr_t *table)
{
	union u_cvdr_ipp_vp_rd_limiter_0 tmp_limiter;
	tmp_limiter.u32 = 0;
	tmp_limiter.bits.vprd_access_limiter_0_0 = 0xF;
	tmp_limiter.bits.vprd_access_limiter_1_0 = 0xF;
	tmp_limiter.bits.vprd_access_limiter_2_0 = 0xF;
	tmp_limiter.bits.vprd_access_limiter_3_0 = 0xF;
	tmp_limiter.bits.vprd_access_limiter_reload_0 = 0xF;
	macro_cmdlst_set_reg(dev->base_addr +
						 CVDR_IPP_CVDR_IPP_VP_RD_LIMITER_0_REG +
						 g_cvdr_vp_rd_id[port] * IPP_VP_RD_REG_OFFSET,
						 tmp_limiter.u32);
}

static void cvdr_vp_rd_config_axi_fs(cvdr_dev_t *dev,
									unsigned int port, cfg_tab_cvdr_t *table)
{
	union u_cvdr_ipp_vp_rd_axi_fs_0 tmp_fs;
	cvdr_rd_fmt_desc_t *desc = &(table->vp_rd_cfg[port].fmt);

	if (desc->fs_addr & 0xF) {
		loge("error: invalid cvdr axi fs para");
		return;
	}

	tmp_fs.u32 = 0;
	tmp_fs.bits.vprd_axi_frame_start_0  = (desc->fs_addr >> 2) >> 2;
	macro_cmdlst_set_reg(dev->base_addr +
						 CVDR_IPP_CVDR_IPP_VP_RD_AXI_FS_0_REG +
						 g_cvdr_vp_rd_id[port] * IPP_VP_RD_REG_OFFSET,
						 tmp_fs.u32);

	logd("VP_RD_AXI_FS: 0x%x = 0x%x, port = %d, fs_addr = 0x%x, vprd_axi_frame_start_0 = 0x%x",
		 dev->base_addr + CVDR_IPP_CVDR_IPP_VP_RD_AXI_FS_0_REG +
		 g_cvdr_vp_rd_id[port] * IPP_VP_RD_REG_OFFSET,
		 tmp_fs.u32, port, desc->fs_addr, tmp_fs.bits.vprd_axi_frame_start_0);
}

/**********************************************************
 * function name: cvdr_set_vp_wr_ready
 *
 * description:
 *     config cvdr video port write(vp_wr) configurations
 *
 * input:
 *     dev  : cvdr device
 *     port : write port number
 *     desc : vp wr configurations info
 *     addr : start address
 *
 * output:
 *     0  : success
 *     -1 : failed
 ***********************************************************/
static int cvdr_set_vp_wr_ready(cvdr_dev_t *dev,
									unsigned int port, cfg_tab_cvdr_t *table)
{
	cvdr_wr_fmt_desc_t *desc = NULL;
	if (port >= IPP_VP_WR_MAX) {
		loge("error vp_wr port id.%d", port);
		return ISP_IPP_ERR;
	}

	desc = &(table->vp_wr_cfg[port].fmt);
	logd("set wr port %d addr = 0x%x, dev->base_addr = 0x%x", port, desc->fs_addr, dev->base_addr);
	if (desc->fs_addr & 0xF) {
		loge("Failed: fs_addr.0x%x is not 16bytes align", desc->fs_addr);
		return ISP_IPP_ERR;
	}

	cvdr_vp_wr_config_cfg(dev, port, table);
	cvdr_vp_wr_config_axi_line(dev, port, table);
	cvdr_vp_wr_config_if(dev, port, table);
	cvdr_vp_wr_config_limiter(dev, port, table);
	cvdr_vp_wr_config_axi_fs(dev, port, table);
	return ISP_IPP_OK;
}

/**********************************************************
 * function name: cvdr_set_vp_rd_ready
 *
 * description:
 *     config cvdr video port read(vp_rd) configurations
 *
 * input:
 *     dev  : cvdr device
 *     port : read port number
 *     desc : vp rd configurations info
 *     addr : start address
 *
 * output:
 *     0  : success
 *     -1 : failed
 ***********************************************************/
static int cvdr_set_vp_rd_ready(cvdr_dev_t *dev,
									unsigned int port, cfg_tab_cvdr_t *table)
{
	if (port >= IPP_VP_RD_MAX) {
		loge("error vp_rd port id.%d", port);
		return ISP_IPP_ERR;
	}

	cvdr_vp_rd_config_cfg(dev, port, table);
	cvdr_vp_rd_config_lwg(dev, port, table);
	cvdr_vp_rd_config_fhg(dev, port, table);
	cvdr_vp_rd_config_axi_line(dev, port, table);
	cvdr_vp_rd_config_if(dev, port, table);
	cvdr_vp_rd_config_limiter(dev, port, table);
	cvdr_vp_rd_config_axi_fs(dev, port, table);
	return ISP_IPP_OK;
}

/**********************************************************
 * function name: cvdr_do_config
 *
 * description:
 *     according to cvdr_config_table config cvdr
 *
 * input:
 *     dev    : cvdr device
 *     table  : cvdr configurations info table
 *
 * output:
 *     0  : success
 *     -1 : failed
 ***********************************************************/
static int cvdr_do_config(cvdr_dev_t *dev, cfg_tab_cvdr_t *table)
{
	int i = 0, ret = 0;

	if (dev == NULL || table == NULL) {
		loge("input param is invalid!!");
		return ISP_IPP_ERR;
	}

	for (i = 0; i < IPP_VP_WR_MAX; ++i) {
		if (table->vp_wr_cfg[i].to_use == 1) {
			if (cvdr_set_vp_wr_ready(dev, i, table)) {
				loge("Failed: cvdr_set_vp_wr_ready");
				return ISP_IPP_ERR;
			}
			table->vp_wr_cfg[i].to_use = 0;
		}
	}

	for (i = IPP_VP_RD_MAX - 1; i >= 0; --i) {
		if (table->vp_rd_cfg[i].to_use == 1) {
			if (cvdr_set_vp_rd_ready(dev, i, table)) {
				loge("Failed: cvdr_set_vp_rd_ready");
				return ISP_IPP_ERR;
			}
			table->vp_rd_cfg[i].to_use = 0;
		}
	}

	for (i = 0; i < IPP_NR_WR_MAX; ++i) {
		if (table->nr_wr_cfg[i].to_use == 1) {
			ret = cvdr_set_nr_wr_config(dev, i,
										table->nr_wr_cfg[i].nr_wr_stop,
										table->nr_wr_cfg[i].en);
			if (ret) {
				loge("Failed: cvdr_set_nr_wr_config");
				return ISP_IPP_ERR;
			}
			table->nr_wr_cfg[i].to_use = 0;
		}
	}

	for (i = 0; i < IPP_NR_RD_MAX; ++i) {
		if (table->nr_rd_cfg[i].to_use == 1) {
			ret = cvdr_set_nr_rd_config(dev, i,
										table->nr_rd_cfg[i].allocated_du,
										table->nr_rd_cfg[i].nr_rd_stop,
										table->nr_rd_cfg[i].en);
			if (ret) {
				loge("Failed: cvdr_set_nr_rd_config");
				return ISP_IPP_ERR;
			}
			table->nr_rd_cfg[i].to_use = 0;
		}
	}

	return ISP_IPP_OK;
}

int cvdr_prepare_vprd_cmd(cvdr_dev_t *dev, cmd_buf_t *cmd_buf, cfg_tab_cvdr_t *table)
{
	int i = 0, ret = 0;

	loge_if_null_set_ret(dev, ret);
	if (ret) return ret;
	loge_if_null_set_ret(cmd_buf, ret);
	if (ret) return ret;
	loge_if_null_set_ret(table, ret);
	if (ret) return ret;

	cvdr_dev_t cvdr_dev = *dev;
	cvdr_dev.cmd_buf = cmd_buf;

	for (i = IPP_VP_RD_MAX - 1; i >= 0; --i) {
		if (table->vp_rd_cfg[i].to_use == 1) {
			ret = cvdr_set_vp_rd_ready(&cvdr_dev, i, table);
			table->vp_rd_cfg[i].to_use = 0;
		}
	}
	return ret;
}

int cvdr_prepare_vpwr_cmd(cvdr_dev_t *dev, cmd_buf_t *cmd_buf, cfg_tab_cvdr_t *table)
{
	unsigned int i = 0;
	int ret = 0;

	loge_if_null_set_ret(dev, ret);
	if (ret) return ret;
	loge_if_null_set_ret(cmd_buf, ret);
	if (ret) return ret;
	loge_if_null_set_ret(table, ret);
	if (ret) return ret;

	cvdr_dev_t cvdr_dev = *dev;
	cvdr_dev.cmd_buf = cmd_buf;

	for (i = 0; i < IPP_VP_WR_MAX; ++i) {
		if (table->vp_wr_cfg[i].to_use == 1) {
			ret = cvdr_set_vp_wr_ready(&cvdr_dev, i, table);
			table->vp_wr_cfg[i].to_use = 0;
		}
	}

	return ret;
}

int cvdr_prepare_cmd(cvdr_dev_t *dev, cmd_buf_t *cmd_buf, cfg_tab_cvdr_t *table)
{
	int ret = 0;
	loge_if_null_set_ret(dev, ret);
	if (ret) return ret;
	loge_if_null_set_ret(cmd_buf, ret);
	if (ret) return ret;
	loge_if_null_set_ret(table, ret);
	if (ret) return ret;

	cvdr_dev_t cvdr_dev = *dev;
	cvdr_dev.cmd_buf = cmd_buf;
	ret = cvdr_do_config(&cvdr_dev, table);
	return ret;
}

int cvdr_prepare_nr_cmd(cvdr_dev_t *dev, cmd_buf_t *cmd_buf, cfg_tab_cvdr_t *table)
{
	int ret = 0;
	loge_if_null_set_ret(dev, ret);
	if (ret) return ret;
	loge_if_null_set_ret(cmd_buf, ret);
	if (ret) return ret;
	loge_if_null_set_ret(table, ret);
	if (ret) return ret;

	cvdr_dev_t cvdr_dev = *dev;
	cvdr_dev.cmd_buf = cmd_buf;
	ret = cvdr_nr_do_config(&cvdr_dev, table);
	return ret;
}

static cvdr_ops_t cvdr_ops = {
	.set_debug_enable = cvdr_set_debug_enable,
	.get_debug_info   = cvdr_get_debug_info,
	.set_vp_wr_ready  = cvdr_set_vp_wr_ready,
	.set_vp_rd_ready  = cvdr_set_vp_rd_ready,

	.do_config        = cvdr_do_config,
	.prepare_cmd      = cvdr_prepare_cmd,
	.prepare_vprd_cmd = cvdr_prepare_vprd_cmd,
	.prepare_vpwr_cmd = cvdr_prepare_vpwr_cmd,
};

cvdr_dev_t g_cvdr_devs[CVDR_DEV_COUNT] = {
	[0] = {
		.base_addr = IPP_BASE_ADDR_CVDR,
		.ops = &cvdr_ops,
	},
};

