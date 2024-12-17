/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name    segment_mc.c
 * Description:
 *
 * Date         2020-04-17 16:28:10
 ********************************************************************/

#include <linux/string.h>
#include <linux/printk.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/io.h>
#include "ipp.h"
#include "memory.h"
#include "segment_mc.h"
#include "cmdlst_manager.h"
#include "cvdr_drv.h"
#include "cvdr_opt.h"
#include "mc_drv.h"
#include "mc_reg_offset.h"
#include "ipp_top_reg_offset.h"
#include "cfg_table_ipp_path.h"

#define LOG_TAG LOG_MODULE_MC

static int seg_mc_set_mc_cfg_tbl(msg_req_mc_t *mc_req,
	cfg_tab_mc_t *mc_cfg_tab, unsigned int mc_update_flag)
{
	if (memcpy_s(mc_cfg_tab, sizeof(cfg_tab_mc_t), &mc_req->reg_cfg, sizeof(cfg_tab_mc_t))) {
		loge("Failed: memcpy_s");
		return ISP_IPP_ERR;
	}

	if (mc_cfg_tab->ctrl_cfg.bits.cfg_mode == MC_CFG_MODE_INDEX_PAIRS) {
		mc_cfg_tab->prefech_cfg.cur_prefetch_cfg.bits.cur_prefetch_enable = 0;
		mc_cfg_tab->prefech_cfg.cur_prefetch_cfg.bits.cur_first_32k_page =
			mc_req->streams[MC_CUR_FIRST_PAGE_BUFF].buffer >> 15;
		mc_cfg_tab->prefech_cfg.ref_prefetch_cfg.bits.ref_prefetch_enable = 0;
		mc_cfg_tab->prefech_cfg.ref_prefetch_cfg.bits.ref_first_32k_page =
			mc_req->streams[MC_REF_FIRST_PAGE_BUFF].buffer >> 15;
	}
	mc_cfg_tab->mc_cascade_en = (mc_update_flag == 0) ? CASCADE_DISABLE : CASCADE_ENABLE;

	return ISP_IPP_OK;
}

static int seg_mc_set_cvdr_cfg_tbl(
	msg_req_mc_t *mc_req,
	cfg_tab_cvdr_t *p_cvdr_cfg)
{
	cvdr_opt_fmt_t cfmt;
	pix_format_e format = PIXEL_FMT_IPP_MAX;
	unsigned int line_stride = 0;

	if (memset_s(&cfmt, sizeof(cvdr_opt_fmt_t), 0, sizeof(cvdr_opt_fmt_t))) {
		loge("Failed : memset_s");
		return ISP_IPP_ERR;
	}
	if (mc_req->streams[MC_CUR_FIRST_PAGE_BUFF].buffer != 0 &&
		mc_req->streams[MC_REF_FIRST_PAGE_BUFF].buffer != 0)
		loge_if(cfg_tbl_cvdr_nr_rd_cfg(p_cvdr_cfg, IPP_NR_RD_MC));

	if (mc_req->streams[MC_H_MATRIX_BUFF].buffer != 0) {
		cfmt.id = IPP_VP_WR_MC_RESULT;
		cfmt.width = MC_OUT_RESULT_SIZE;
		cfmt.height = 1;
		cfmt.full_width = cfmt.width;
		cfmt.addr = mc_req->streams[MC_H_MATRIX_BUFF].buffer;
		cfmt.expand = 0;
		format = PIXEL_FMT_IPP_D64;
		line_stride = 0;
		if (dataflow_cvdr_wr_cfg_vp(p_cvdr_cfg, &cfmt, line_stride, ISP_IPP_CLK, format)) {
			loge("Failed: dataflow_cvdr_wr_cfg_vp");
			return ISP_IPP_ERR;
		}
	}

	return ISP_IPP_OK;
}

static int seg_mc_set_cfg_tbl(
	msg_req_mc_t *mc_req,
	seg_mc_cfg_t *seg_cfg_tab,
	global_info_ipp_t *global_info)
{
	int ret = 0;
	ret = seg_mc_set_mc_cfg_tbl(mc_req, &seg_cfg_tab->mc_cfg_tab, global_info->mc_update_flag);
	if (ret != 0) {
		loge("Failed : seg_mc_set_mc_cfg_tbl");
		return ISP_IPP_ERR;
	}

	ret = seg_mc_set_cvdr_cfg_tbl(mc_req, &seg_cfg_tab->mc_cvdr_cfg_tab);
	if (ret != 0) {
		loge("Failed : seg_mc_set_cvdr_cfg_tbl");
		return ISP_IPP_ERR;
	}

	return ISP_IPP_OK;
}

static int seg_mc_set_cmdlst_stripe(
	msg_req_mc_t *mc_req,
	cmdlst_para_t *mc_cmdlst_para,
	global_info_ipp_t *global_info)
{
	cmdlst_stripe_info_t *cmdlst_stripe = mc_cmdlst_para->cmd_stripe_info;
	unsigned int i = 0;
	unsigned int stripe_cnt = 1;
	unsigned int irq = 0;
	stripe_cnt = (mc_req->mc_stat_buff[MC_COORDINATE_PAIRS_BUFF].buffer == 0) ?
				 (stripe_cnt) : (stripe_cnt + 1);
	stripe_cnt = (mc_req->mc_stat_buff[MC_LOW_KPT_BUFF].buffer == 0) ?
				 (stripe_cnt) : (stripe_cnt + 1);

	mc_cmdlst_para->stripe_cnt  = stripe_cnt;
	mc_cmdlst_para->channel_id  = global_info->mc_channel;
	mc_cmdlst_para->work_module = global_info->work_module;
	mc_cmdlst_para->cmd_entry_mem_id = MEM_ID_CMDLST_ENTRY_MC;
	mc_cmdlst_para->cmd_buf_mem_id = MEM_ID_CMDLST_BUF_MC;
	global_info->mc_stripe_cnt = stripe_cnt;

	logi("mc stripe_cnt = %u", stripe_cnt);
	for (i = 0; i < stripe_cnt; i++) {
		irq = 0;

		if (i != 0)
			irq = irq | (((unsigned int)(1)) << IPP_MC_CVDR_VP_WR_EOF_CMDLST);
		else
			irq = irq | (((unsigned int)(1)) << IPP_MC_IRQ_DONE);

		cmdlst_stripe[i].hw_priority	 = CMD_PRIO_HIGH;
		cmdlst_stripe[i].resource_share  = 1 << IPP_CMD_RES_SHARE_MC;
		cmdlst_stripe[i].irq_module  = 1 << IPP_CMD_IRQ_MODULE_MC;
		cmdlst_stripe[i].irq_mode		 = irq;
		cmdlst_stripe[i].en_link		 = 0; // mc is the last one.
		cmdlst_stripe[i].ch_link		 = 0;
		cmdlst_stripe[i].ch_link_act_nbr = 0;
		cmdlst_stripe[i].is_last_stripe  = 0;
		cmdlst_stripe[i].is_need_set_sop = 0;
	}

	cmdlst_stripe[stripe_cnt - 1].is_last_stripe  = 1;
	cmdlst_stripe[0].hw_priority = CMD_PRIO_LOW;

	return ISP_IPP_OK;
}

static int seg_mc_set_cmdlst_buf(
	msg_req_mc_t *mc_req,
	cmdlst_para_t *cmdlst_para,
	seg_mc_cfg_t *seg_cfg_tab,
	global_info_ipp_t *global_info)
{
	unsigned int i = 0;
	cmdlst_wr_cfg_t cmdlst_wr_cfg;
	schedule_cmdlst_link_t *cmd_link_entry = cmdlst_para->cmd_entry;
	if (memset_s(&cmdlst_wr_cfg, sizeof(cmdlst_wr_cfg_t), 0, sizeof(cmdlst_wr_cfg_t))) {
		loge("Failed : memset_s");
		return ISP_IPP_ERR;
	}
	cmdlst_wr_cfg.read_mode = 0;

	for (i = 0; i < cmdlst_para->stripe_cnt;) {
		if (cvdr_prepare_cmd(&g_cvdr_devs[0], &cmd_link_entry[i].cmd_buf, &seg_cfg_tab->mc_cvdr_cfg_tab)) {
			loge("Failed: cvdr_prepare_cmd");
			return ISP_IPP_ERR;
		}

		if (mc_prepare_cmd(&g_mc_devs[0], &cmd_link_entry[i].cmd_buf, &seg_cfg_tab->mc_cfg_tab)) {
			loge("Failed: mc_prepare_cmd");
			return ISP_IPP_ERR;
		}

		if (global_info->mc_en == 1) {
			global_info->mc_matched_num_addr = seg_cfg_tab->mc_cfg_tab.mc_match_points_addr;
			global_info->mc_index_addr = seg_cfg_tab->mc_cfg_tab.mc_index_addr;
		}

		if (mc_enable_cmd(&g_mc_devs[0], &cmd_link_entry[i++].cmd_buf, &seg_cfg_tab->mc_cfg_tab)) {
			loge("Failed: mc_enable_cmd");
			return ISP_IPP_ERR;
		}

		if (mc_req->mc_stat_buff[MC_COORDINATE_PAIRS_BUFF].buffer) {
			cmdlst_wr_cfg.is_incr = 0;
			cmdlst_wr_cfg.data_size = MC_COORDINATE_PAIRS;
			cmdlst_wr_cfg.is_wstrb = 0;
			cmdlst_wr_cfg.buff_wr_addr = mc_req->mc_stat_buff[MC_COORDINATE_PAIRS_BUFF].buffer;
			cmdlst_wr_cfg.reg_rd_addr = IPP_BASE_ADDR_MC + MC_COORDINATE_PAIRS_0_REG;
			loge_if(seg_ipp_set_cmdlst_wr_buf(&cmd_link_entry[i++].cmd_buf, &cmdlst_wr_cfg));
		}

		if (mc_req->mc_stat_buff[MC_LOW_KPT_BUFF].buffer) {
			cmdlst_wr_cfg.is_incr = 0;
			cmdlst_wr_cfg.data_size = 1;
			cmdlst_wr_cfg.is_wstrb = 0;
			cmdlst_wr_cfg.buff_wr_addr = mc_req->mc_stat_buff[MC_LOW_KPT_BUFF].buffer;
			cmdlst_wr_cfg.reg_rd_addr = IPP_BASE_ADDR_MC + MC_INLIER_NUMBER_REG;
			loge_if(seg_ipp_set_cmdlst_wr_buf(&cmd_link_entry[i++].cmd_buf, &cmdlst_wr_cfg));
		}
	}

	return ISP_IPP_OK;
}

static int seg_mc_get_cpe_mem(seg_mc_cfg_t **seg_mc_cfg, cmdlst_para_t **cmdlst_para, unsigned int work_module)
{
	unsigned long long va = 0;
	unsigned int da = 0;
	int ret = 0;
	// 1. mc cfg_tbl
	ret = cpe_mem_get(MEM_ID_MC_CFG_TAB, work_module, &va, &da);
	if (ret != 0) {
		loge("Failed : cpe_mem_get %d", MEM_ID_MC_CFG_TAB);
		return ISP_IPP_ERR;
	}

	*seg_mc_cfg = (seg_mc_cfg_t *)(uintptr_t)va;

	// 2. mc cmdlst_para
	ret = cpe_mem_get(MEM_ID_CMDLST_PARA_MC, work_module, &va, &da);
	if (ret != 0) {
		loge("Failed : cpe_mem_get %d", MEM_ID_CMDLST_PARA_MC);
		goto out;
	}

	*cmdlst_para = (cmdlst_para_t *)(uintptr_t)va;

	return ISP_IPP_OK;
out:
	cpe_mem_free(MEM_ID_MC_CFG_TAB, work_module);
	cpe_mem_free(MEM_ID_CMDLST_PARA_MC, work_module);
	*seg_mc_cfg = NULL;
	*cmdlst_para = NULL;
	return ISP_IPP_ERR;
}

static int seg_mc_check_req_para(msg_req_mc_t *mc_req) {
	unsigned int mc_mode = 0;
	mc_mode = mc_req->reg_cfg.ctrl_cfg.bits.cfg_mode;

	if ((mc_mode == MC_CFG_MODE_INDEX_PAIRS) &&
		(mc_req->streams[MC_CUR_FIRST_PAGE_BUFF].buffer == 0
		 || mc_req->streams[MC_REF_FIRST_PAGE_BUFF].buffer == 0)) {
		loge(" Failed : mc_addr is incorrect");
		return ISP_IPP_ERR;
	}
	return ISP_IPP_OK;
}

static int seg_mc_prepare_cmdlst_para(
	msg_req_mc_t *mc_req,
	cmdlst_para_t *mc_cmdlst_para,
	seg_mc_cfg_t *seg_cfg_tab,
	global_info_ipp_t *global_info)
{
	int ret = 0;

	ret = seg_mc_set_cmdlst_stripe(mc_req, mc_cmdlst_para, global_info);
	if (ret != 0) {
		loge("Failed : seg_mc_set_cmdlst_stripe");
		return ISP_IPP_ERR;
	}

	ret = df_sched_prepare(mc_cmdlst_para);
	if (ret != 0) {
		loge("Failed : df_sched_prepare");
		return ISP_IPP_ERR;
	}

	ret = seg_mc_set_cmdlst_buf(mc_req, mc_cmdlst_para, seg_cfg_tab, global_info);
	if (ret != 0) {
		loge("Failed : seg_mc_set_cmdlst_buf");
		return ISP_IPP_ERR;
	}

	return ISP_IPP_OK;
}

#ifdef IPP_UT_DEBUG
static void seg_mc_dump_req(msg_req_mc_t *mc_req)
{
	unsigned int i = 0;
	cfg_tab_mc_t *reg_cfg = NULL;
	unsigned int tmp_arr[128] = {0};

	if (mc_req == NULL) return;
	logi("---------dump mc start---------");
	logi("---------mc stream's info:");
	for (i = 0; i < MC_STREAM_MAX; i++) {
		logi("streams[%d].width  = %d", i, mc_req->streams[i].width);
		logi("streams[%d].height = %d", i, mc_req->streams[i].height);
		logi("streams[%d].stride = %d", i, mc_req->streams[i].stride);
		logi("streams[%d].fd     = %d", i, mc_req->streams[i].fd);
		logi("streams[%d].format = %d", i, mc_req->streams[i].format);
		logi("streams[%d].offset = 0x%x", i, mc_req->streams[i].offset);
		logi("streams[%d].total_size = 0x%x", i, mc_req->streams[i].total_size);
	}

	for (i = 0; i < MC_STAT_BUFF_MAX; i++)
		logi("mc_stat_buff: i=%d, width.%d, height.%d, stride.%d, fd.%d, prot.%d, offset.0x%08x, format.%d",
			i, mc_req->mc_stat_buff[i].width, mc_req->mc_stat_buff[i].height,
			mc_req->mc_stat_buff[i].stride, mc_req->mc_stat_buff[i].fd,
			mc_req->mc_stat_buff[i].prot, mc_req->mc_stat_buff[i].offset,
			mc_req->mc_stat_buff[i].format);

	logi("---------mc reg_cfg:");
	reg_cfg = &mc_req->reg_cfg;
	logi("mc_en=%d", reg_cfg->mc_en_cfg.bits.mc_en);
	logi("imu_en=%d, imu_static=%d", reg_cfg->imu_cfg.bits.imu_en, reg_cfg->imu_cfg.bits.imu_static);
	logi("cfg_mode=%d, push_inliers_en=%d, stat_en=%d",
		reg_cfg->ctrl_cfg.bits.cfg_mode, reg_cfg->ctrl_cfg.bits.push_inliers_en, reg_cfg->ctrl_cfg.bits.stat_en);
	logi("max_iterations=%d, svd_iterations=%d",
		reg_cfg->ctrl_cfg.bits.max_iterations, reg_cfg->ctrl_cfg.bits.svd_iterations);
	logi("inlier_th=0x%x", reg_cfg->inlier_thld_cfg.bits.inlier_th);
	logi("inlier_num_th=%d", reg_cfg->inlier_number_cfg.bits.inlier_num_th);
	logi("kpt_size=%d", reg_cfg->kpt_size_cfg.bits.kpt_size);
	logi("mc_static_cfg:static_ratio=0x%x, inv_ds_ratio=0x%x, matched_num_th=%d",
		reg_cfg->static_cfg.static_cfg1.bits.static_ratio,
		reg_cfg->static_cfg.static_cfg1.bits.inv_ds_ratio,
		reg_cfg->static_cfg.static_cfg2.bits.matched_num_th);
	logi("matched_kpts=%d", reg_cfg->matched_kpts_cfg.bits.matched_kpts);

	memcpy_s(tmp_arr, sizeof(tmp_arr), reg_cfg->index_pairs_cfg.index_pairs, sizeof(tmp_arr));
	log_dump_array("index_pairs = ", "0x%08x, ", tmp_arr, sizeof(tmp_arr)/sizeof(tmp_arr[0]), 8);
	memcpy_s(tmp_arr, sizeof(tmp_arr), reg_cfg->coord_pairs_cfg.cur_coordinate, sizeof(tmp_arr));
	log_dump_array("cur_coordinate = ", "0x%08x, ", tmp_arr, sizeof(tmp_arr)/sizeof(tmp_arr[0]), 8);
	memcpy_s(tmp_arr, sizeof(tmp_arr), reg_cfg->coord_pairs_cfg.ref_coordinate, sizeof(tmp_arr));
	log_dump_array("ref_coordinate = ", "0x%08x, ", tmp_arr, sizeof(tmp_arr)/sizeof(tmp_arr[0]), 8);

	logi("---------dump mc end---------");
	return;
}
#endif

int seg_mc_request_handler(
	msg_req_mc_t *mc_req,
	global_info_ipp_t *global_info)
{
	int ret = 0;
	seg_mc_cfg_t *seg_mc_cfg = NULL;
	cmdlst_para_t *cmdlst_para = NULL;

	ret = seg_mc_check_req_para(mc_req);
	if (ret != 0) {
		loge("Failed : seg_mc_check_req_para");
		return ISP_IPP_ERR;
	}
#ifdef IPP_UT_DEBUG
	seg_mc_dump_req(mc_req);
#endif

	ret = seg_mc_get_cpe_mem(&seg_mc_cfg, &cmdlst_para, global_info->work_module);
	if (ret != 0) {
		loge("Failed : seg_mc_get_cpe_mem");
		goto out;
	}

	ret = seg_mc_set_cfg_tbl(mc_req, seg_mc_cfg, global_info);
	if (ret != 0) {
		loge("Failed : seg_mc_set_cfg_tbl");
		goto out;
	}

	ret = seg_mc_prepare_cmdlst_para(mc_req, cmdlst_para, seg_mc_cfg, global_info);
	if (ret != 0) {
		loge("Failed : seg_mc_prepare_cmdlst_para");
		goto out;
	}

	ret = df_sched_start(cmdlst_para);
	if (ret != 0)
		loge("Failed : df_sched_start");

out:
	cpe_mem_free(MEM_ID_MC_CFG_TAB, global_info->work_module);
	return ret;
}

// end file

