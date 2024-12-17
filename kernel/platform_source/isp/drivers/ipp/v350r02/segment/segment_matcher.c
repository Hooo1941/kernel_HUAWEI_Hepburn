/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name    segment_matcher.c
 * Description:
 *
 * Date         2020-04-17 16:28:10
 ********************************************************************/

#include <linux/string.h>
#include <linux/printk.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/delay.h>
#include "ipp.h"
#include "memory.h"
#include "segment_matcher.h"
#include "cmdlst_manager.h"
#include "cvdr_drv.h"
#include "cvdr_opt.h"
#include "reorder_drv.h"
#include "compare_drv.h"
#include "compare_reg_offset.h"
#include "ipp_top_reg_offset.h"
#include "cfg_table_ipp_path.h"

#define LOG_TAG LOG_MODULE_MATCHER

// mc update mc matched point + mc index 1600
#define UPDATE_MC_MATCH_POINTS_CFG   (1)
#define UPDATE_MC_INDEX_CFG_CFG      (1)

static int seg_matcher_rdr_set_cvdr_cfg_tbl(
	msg_req_matcher_t *matcher_req,
	cfg_tab_cvdr_t *reorder_cfg_tab, unsigned int layer_num)
{
	cvdr_opt_fmt_t cfmt;
	unsigned int stride = 0;
	pix_format_e format;
	req_rdr_t *req_rdr = &(matcher_req->req_rdr);

	if (req_rdr->streams[layer_num][BI_RDR_FP].buffer) {
		cfmt.id = IPP_VP_RD_RDR;
		cfmt.width = req_rdr->streams[layer_num][BI_RDR_FP].width;
		cfmt.full_width = req_rdr->streams[layer_num][BI_RDR_FP].width;
		cfmt.line_size = req_rdr->streams[layer_num][BI_RDR_FP].width;
		cfmt.height = req_rdr->streams[layer_num][BI_RDR_FP].height;
		cfmt.expand = 0;
		stride = req_rdr->streams[layer_num][BI_RDR_FP].width * 8 / 16; // 8:one line is 64bits; 16:cvdr_align byts
		format = PIXEL_FMT_IPP_D64;
		cfmt.addr = req_rdr->streams[layer_num][BI_RDR_FP].buffer;
		cfg_tbl_cvdr_rd_cfg_d64(reorder_cfg_tab, &cfmt, (align_up(144 * 4096, CVDR_ALIGN_BYTES)), stride);
	}

	if (req_rdr->streams[layer_num][BO_RDR_FP_BLOCK].buffer)
		cfg_tbl_cvdr_nr_wr_cfg(reorder_cfg_tab, IPP_NR_WR_RDR);

	return 0;
}

static int seg_matcher_rdr_set_rdr_cfg_tbl(
	msg_req_matcher_t *matcher_req,
	cfg_tab_reorder_t *reorder_cfg_tab,
	unsigned int layer_num,
	unsigned int rdr_update_flag)
{
	req_rdr_t *req_rdr = &(matcher_req->req_rdr);

	if (memcpy_s(reorder_cfg_tab, sizeof(cfg_tab_compare_t),
				&req_rdr->reg_cfg[layer_num], sizeof(cfg_tab_compare_t))) {
		loge("Failed: memcpy_s");
		return ISP_IPP_ERR;
	}
	reorder_cfg_tab->prefetch_cfg.bits.prefetch_enable = 0;
	reorder_cfg_tab->prefetch_cfg.bits.first_32k_page = req_rdr->streams[layer_num][BO_RDR_FP_BLOCK].buffer >> 15;
	reorder_cfg_tab->rdr_cascade_en = (rdr_update_flag == 0) ? CASCADE_DISABLE : CASCADE_ENABLE;
	return ISP_IPP_OK;
}

static int seg_matcher_rdr_get_cpe_mem(seg_matcher_rdr_cfg_t **seg_cfg_tab,
										cmdlst_para_t **rdr_cmdlst_para, unsigned int work_module)
{
	unsigned long long va = 0;
	unsigned int da = 0;
	int ret = 0;
	// 1.rdr cfg_tbl
	ret = cpe_mem_get(MEM_ID_REORDER_CFG_TAB, work_module, &va, &da);
	if (ret != 0) {
		loge(" Failed : cpe_mem_get %d", MEM_ID_REORDER_CFG_TAB);
		return ISP_IPP_ERR;
	}

	*seg_cfg_tab = (seg_matcher_rdr_cfg_t *)(uintptr_t)va;
// 2. rdr cmdlst_para
	ret = cpe_mem_get(MEM_ID_CMDLST_PARA_REORDER, work_module, &va, &da);
	if (ret != 0) {
		loge("Failed : cpe_mem_get %d", MEM_ID_CMDLST_PARA_REORDER);
		goto ipp_path_rdr_get_cpe_mem_failed;
	}

	*rdr_cmdlst_para = (cmdlst_para_t *)(uintptr_t)va;
	return ISP_IPP_OK;
ipp_path_rdr_get_cpe_mem_failed:
	cpe_mem_free(MEM_ID_REORDER_CFG_TAB, work_module);
	cpe_mem_free(MEM_ID_CMDLST_PARA_REORDER, work_module);
	*seg_cfg_tab = NULL;
	*rdr_cmdlst_para = NULL;

	return ISP_IPP_ERR;
}

static int seg_matcher_rdr_set_cfg_tbl(
	msg_req_matcher_t *matcher_req,
	seg_matcher_rdr_cfg_t *seg_cfg_tab,
	global_info_ipp_t *global_info)
{
	unsigned int i = 0;

	for (i = 0; i < matcher_req->rdr_pyramid_layer; i++) {
		if (seg_matcher_rdr_set_rdr_cfg_tbl(matcher_req, &(seg_cfg_tab->reorder_cfg_tab[i]), i,
			global_info->matcher_update_flag)) {
			loge("Failed: seg_matcher_rdr_set_rdr_cfg_tbl");
			return ISP_IPP_ERR;
		}
		if (seg_matcher_rdr_set_cvdr_cfg_tbl(matcher_req, &(seg_cfg_tab->reorder_cvdr_cfg_tab[i]), i)) {
			loge("Failed: seg_matcher_rdr_set_cvdr_cfg_tbl");
			return ISP_IPP_ERR;
		}
	}

	return ISP_IPP_OK;
}

static int seg_matcher_rdr_set_cmdlst_stripe(
	msg_req_matcher_t *matcher_req,
	cmdlst_para_t *rdr_cmdlst_para,
	global_info_ipp_t *global_info)
{
	unsigned int i = 0;
	unsigned int irq = 0;
	cmdlst_stripe_info_t *cmdlst_stripe = rdr_cmdlst_para->cmd_stripe_info;
	rdr_cmdlst_para->stripe_cnt  = matcher_req->rdr_pyramid_layer * STRIPE_NUM_EACH_RDR;
	rdr_cmdlst_para->channel_id  = global_info->rdr_channel;
	rdr_cmdlst_para->work_module = global_info->work_module;
	rdr_cmdlst_para->cmd_entry_mem_id = MEM_ID_CMDLST_ENTRY_REORDER;
	rdr_cmdlst_para->cmd_buf_mem_id = MEM_ID_CMDLST_BUF_REORDER;

	if (rdr_cmdlst_para->stripe_cnt > CMDLST_STRIPE_MAX_NUM) {
		loge("error : invalid stripe_cnt = %d", rdr_cmdlst_para->stripe_cnt);
		return ISP_IPP_ERR;
	}

	for (i = 0; i < rdr_cmdlst_para->stripe_cnt; i++) {
		irq = 0;
		cmdlst_stripe[i].resource_share  = 1 << IPP_CMD_RES_SHARE_RDR;
		cmdlst_stripe[i].irq_module  = 1 << IPP_CMD_IRQ_MODULE_RDR;
		cmdlst_stripe[i].en_link         = 0;
		cmdlst_stripe[i].ch_link         = 0;
		cmdlst_stripe[i].ch_link_act_nbr = 0;
		cmdlst_stripe[i].is_last_stripe  = 0;
		cmdlst_stripe[i].is_need_set_sop = 0;
		irq = irq | (((unsigned int)(1)) << IPP_RDR_CVDR_VP_RD_EOF_CMDSLT);
		irq = irq | (((unsigned int)(1)) << IPP_RDR_IRQ_DONE);
		irq = irq | (((unsigned int)(1)) << IPP_RDR_CVDR_VP_RD_EOF_FP);
		cmdlst_stripe[i].hw_priority     = CMD_PRIO_HIGH;
		cmdlst_stripe[i].irq_mode        = irq;
		logd("seg_matcher_rdr: stripe=%u, irq=0x%x", i, irq);

		if (matcher_req->matcher_work_mode == NORMAL_MATCHER_MODE ||
			matcher_req->matcher_work_mode == MULTI_RDR_MULTI_CMP) {
			// 2rdr + 1cmp case,first rdr not release token to cmp
			cmdlst_stripe[i].en_link = 1;

			if (i > 0) {
				cmdlst_stripe[i].ch_link = global_info->cmp_channel;
				cmdlst_stripe[i].ch_link_act_nbr = (global_info->mc_en == 0) ?
												   global_info->cmp_layer_stripe_cnt : global_info->cmp_layer_stripe_cnt + 2;
			}
		} else if (matcher_req->matcher_work_mode == SINGLE_RDR_MULTI_CMP) {
			// 1rdr + 7cmp case, release all token to cmp
			cmdlst_stripe[i].en_link = 1;
			cmdlst_stripe[i].ch_link = global_info->cmp_channel;
			cmdlst_stripe[i].ch_link_act_nbr = global_info->cmp_stripe_cnt;
		}
	}

	if (rdr_cmdlst_para->stripe_cnt >= 1)
		cmdlst_stripe[rdr_cmdlst_para->stripe_cnt - 1].is_last_stripe  = 1;
	cmdlst_stripe[0].hw_priority  = CMD_PRIO_LOW;
	return ISP_IPP_OK;
}

static int seg_matcher_rdr_set_cmdlst_buf(
	msg_req_matcher_t *matcher_req,
	cmdlst_para_t *rdr_cmdlst_para,
	seg_matcher_rdr_cfg_t *seg_cfg_tab,
	global_info_ipp_t *global_info)
{
	unsigned int i = 0;
	schedule_cmdlst_link_t *rdr_cmd_link_entry = NULL;

	rdr_cmd_link_entry = (schedule_cmdlst_link_t *)rdr_cmdlst_para->cmd_entry;

	for (i = 0; i < rdr_cmdlst_para->stripe_cnt; i++) {
		if (cvdr_prepare_nr_cmd(&g_cvdr_devs[0], &(rdr_cmd_link_entry[i].cmd_buf),
										&(seg_cfg_tab->reorder_cvdr_cfg_tab[i]))) {
			loge("Failed: cvdr_prepare_nr_cmd");
			return ISP_IPP_ERR;
		}

		if(reorder_prepare_cmd(&g_reorder_devs[0], &(rdr_cmd_link_entry[i].cmd_buf),
										&(seg_cfg_tab->reorder_cfg_tab[i]))) {
			loge("Failed: reorder_prepare_cmd");
			return ISP_IPP_ERR;
		}

		if(cvdr_prepare_cmd(&g_cvdr_devs[0], &(rdr_cmd_link_entry[i].cmd_buf),
									    &(seg_cfg_tab->reorder_cvdr_cfg_tab[i]))) {
			loge("Failed: cvdr_prepare_cmd");
			return ISP_IPP_ERR;
		}

		if (global_info->matcher_update_flag == 1 && i == 0) {
			global_info->rdr_kpt_num_addr_cur = seg_cfg_tab->reorder_cfg_tab[i].rdr_kpt_num_addr;
			global_info->rdr_total_num_addr_cur = seg_cfg_tab->reorder_cfg_tab[i].rdr_total_num_addr;
			global_info->rdr_frame_height_addr_cur = seg_cfg_tab->reorder_cvdr_cfg_tab[i].rdr_cvdr_frame_size_addr;
		} else if (global_info->matcher_update_flag == 1 && i == 1) {
			global_info->rdr_kpt_num_addr_ref = seg_cfg_tab->reorder_cfg_tab[i].rdr_kpt_num_addr;
			global_info->rdr_total_num_addr_ref = seg_cfg_tab->reorder_cfg_tab[i].rdr_total_num_addr;
			global_info->rdr_frame_height_addr_ref = seg_cfg_tab->reorder_cvdr_cfg_tab[i].rdr_cvdr_frame_size_addr;
		}
	}

	return ISP_IPP_OK;
}


static int seg_matcher_rdr_prepare_cmdlst_para(
	msg_req_matcher_t *matcher_req,
	seg_matcher_rdr_cfg_t *seg_cfg_tab,
	cmdlst_para_t *rdr_cmdlst_para,
	global_info_ipp_t *global_info)
{
	if (seg_matcher_rdr_set_cmdlst_stripe(matcher_req, rdr_cmdlst_para, global_info)) {
		loge("Failed: seg_matcher_rdr_set_cmdlst_stripe");
		return ISP_IPP_ERR;
	}

	if (df_sched_prepare(rdr_cmdlst_para)) {
		loge("Failed: df_sched_prepare");
		return ISP_IPP_ERR;
	}

	if (seg_matcher_rdr_set_cmdlst_buf(matcher_req, rdr_cmdlst_para, seg_cfg_tab, global_info)) {
		loge("Failed: seg_matcher_rdr_set_cmdlst_buf");
		return ISP_IPP_ERR;
	}

	return ISP_IPP_OK;
}

#ifdef IPP_UT_DEBUG
static void seg_matcher_dump_rdr_req(req_rdr_t *req_rdr, unsigned int rdr_pyramid_layer)
{
	unsigned int i = 0, j= 0;
	cfg_tab_reorder_t *reg_cfg = NULL;
	unsigned int tmp_arr[RDR_KPT_NUM_RANGE] = {0};

	if (req_rdr == NULL) return;
	logi("---------dump rdr start---------");
	logi("---------rdr stream's info:");

	for (i = 0; i < rdr_pyramid_layer; i++) {
		for (j = 0; j < RDR_STREAM_MAX; j++) {
			logi("streams[%d][%d].width  = %d", i, j, req_rdr->streams[i][j].width);
			logi("streams[%d][%d].height = %d", i, j, req_rdr->streams[i][j].height);
			logi("streams[%d][%d].stride = %d", i, j, req_rdr->streams[i][j].stride);
			logi("streams[%d][%d].fd     = %d", i, j, req_rdr->streams[i][j].fd);
			logi("streams[%d][%d].format = %d", i, j, req_rdr->streams[i][j].format);
			logi("streams[%d][%d].offset = 0x%08x", i, j, req_rdr->streams[i][j].offset);
			logi("streams[%d][%d].total_size = 0x%x", i, j, req_rdr->streams[i][j].total_size);
		}

		logi("---------rdr reg_cfg, layer.%d:", i);
		reg_cfg = &req_rdr->reg_cfg[i];
		logi("reorder_en = %d", reg_cfg->ctrl_cfg.bits.reorder_en);
		logi("descriptor_type = %d", reg_cfg->ctrl_cfg.bits.descriptor_type);
		logi("total_kpt = %d", reg_cfg->total_kptnum_cfg.bits.total_kpt_num);
		logi("blk_v_num = %d", reg_cfg->block_cfg.bits.blk_v_num);
		logi("blk_h_num = %d", reg_cfg->block_cfg.bits.blk_h_num);
		logi("blk_num   = %d", reg_cfg->block_cfg.bits.blk_num);
		logi("reorder_first_32k_page = 0x%x", reg_cfg->prefetch_cfg.bits.first_32k_page);

		memcpy_s(tmp_arr, RDR_KPT_NUM_RANGE * sizeof(unsigned int),
			reg_cfg->rdr_kptnum_cfg, RDR_KPT_NUM_RANGE * sizeof(rdr_kptnum_cfg_t));
		log_dump_array("reorder_kpt_number = ", "%5d, ", tmp_arr, RDR_KPT_NUM_RANGE, 32);
	}
	logi("---------dump rdr end---------");

	return;
}
#endif

static int seg_matcher_rdr_check_para(msg_req_matcher_t *matcher_req)
{
	unsigned int i = 0;

	if (matcher_req->rdr_pyramid_layer < 1 || matcher_req->rdr_pyramid_layer > MATCHER_LAYER_MAX) {
		loge("Failed: invalid rdr_pyramid_layer.%d", matcher_req->rdr_pyramid_layer);
		return ISP_IPP_ERR;
	}
	if (matcher_req->matcher_work_mode < NORMAL_MATCHER_MODE || matcher_req->matcher_work_mode >= MATCHER_MODE_MAX) {
		loge("Failed: invalid matcher_work_mode.%d", matcher_req->matcher_work_mode);
		return ISP_IPP_ERR;
	}

	for (i = 0; i < matcher_req->rdr_pyramid_layer; i++) {
		if (matcher_req->req_rdr.streams[i][BI_RDR_FP].buffer == 0 ||
			(matcher_req->req_rdr.streams[i][BO_RDR_FP_BLOCK].buffer  == 0) ||
			(matcher_req->req_rdr.streams[i][BO_RDR_FP_BLOCK].buffer & 0x7FFF) != 0) {
			loge("Failed: req_rdr.streams[%d][BO_RDR_FP_BLOCK].buffer.0x%x is not 32k page align", i,
				matcher_req->req_rdr.streams[i][BO_RDR_FP_BLOCK].buffer);
			return ISP_IPP_ERR;
		}
	}
	return ISP_IPP_OK;
}

int seg_matcher_rdr_request_handler(
	msg_req_matcher_t *matcher_req,
	global_info_ipp_t *global_info)
{
	int ret = 0;
	seg_matcher_rdr_cfg_t *seg_cfg_tab = NULL;
	cmdlst_para_t *rdr_cmdlst_para = NULL;

	if (matcher_req->rdr_pyramid_layer == 0) {
		logi("matcher req: rdr_pyramid_layer is 0, do nothing");
		return ISP_IPP_OK;
	}

	ret = seg_matcher_rdr_check_para(matcher_req);
	if (ret != 0) {
		loge("Failed: seg_matcher_rdr_check_para");
		return ISP_IPP_ERR;
	}

#ifdef IPP_UT_DEBUG
	seg_matcher_dump_rdr_req(&matcher_req->req_rdr, matcher_req->rdr_pyramid_layer);
#endif
	ret = seg_matcher_rdr_get_cpe_mem(&seg_cfg_tab, &rdr_cmdlst_para, global_info->work_module);
	if (ret != 0) {
		loge("Failed: seg_matcher_rdr_get_cpe_mem");
		return ISP_IPP_ERR;
	}

	ret = seg_matcher_rdr_set_cfg_tbl(matcher_req, seg_cfg_tab, global_info);
	if (ret != 0) {
		loge("Failed: seg_matcher_rdr_set_cfg_tbl");
		goto out;
	}

	ret = seg_matcher_rdr_prepare_cmdlst_para(matcher_req, seg_cfg_tab, rdr_cmdlst_para, global_info);
	if (ret != 0) {
		loge("Failed: seg_matcher_rdr_prepare_cmdlst_para");
		goto out;
	}

	ret = df_sched_start(rdr_cmdlst_para);
	if (ret != 0)
		loge("Failed: df_sched_start");

out:
	cpe_mem_free(MEM_ID_REORDER_CFG_TAB, global_info->work_module);
	return ret;
}

static int seg_matcher_cmp_set_cvdr_cfg_tbl(msg_req_matcher_t *matcher_req,
	cfg_tab_cvdr_t *compare_cvdr_cfg_tab, unsigned int layer_num)
{
	cvdr_opt_fmt_t cfmt;
	pix_format_e format;
	req_cmp_t *req_cmp = &(matcher_req->req_cmp);
	unsigned int stride = 0;

	if (req_cmp->streams[layer_num][BI_CMP_REF_FP].buffer) {
		cfmt.id = IPP_VP_RD_CMP;
		cfmt.width = req_cmp->streams[layer_num][BI_CMP_REF_FP].width;
		cfmt.full_width = req_cmp->streams[layer_num][BI_CMP_REF_FP].width;
		cfmt.line_size = req_cmp->streams[layer_num][BI_CMP_REF_FP].width;
		cfmt.height = req_cmp->streams[layer_num][BI_CMP_REF_FP].height;
		cfmt.expand = 1;
		format = PIXEL_FMT_IPP_D64;
		stride = cfmt.width * 8 / 16; // 8:one line is 64bits; 16:cvdr_align byts
		cfmt.addr = req_cmp->streams[layer_num][BI_CMP_REF_FP].buffer;
		cfg_tbl_cvdr_rd_cfg_d64(compare_cvdr_cfg_tab, &cfmt, (align_up(CMP_IN_INDEX_NUM, CVDR_ALIGN_BYTES)), stride);
	}

	if (req_cmp->streams[layer_num][BI_CMP_CUR_FP].buffer)
		cfg_tbl_cvdr_nr_rd_cfg(compare_cvdr_cfg_tab, IPP_NR_RD_CMP);

	return 0;
}

static int seg_matcher_cmp_set_cmp_cfg_tbl(
	msg_req_matcher_t *matcher_req,
	cfg_tab_compare_t *compare_cfg_tab,
	unsigned int layer_num,
	unsigned int cmp_update_flag)
{
	unsigned int i = 0;
	req_cmp_t *req_cmp = &(matcher_req->req_cmp);
	if (memcpy_s(compare_cfg_tab, sizeof(cfg_tab_compare_t), &req_cmp->reg_cfg[i], sizeof(cfg_tab_compare_t))) {
		loge("Failed: memcpy_s");
		return ISP_IPP_ERR;
	}

	compare_cfg_tab->prefetch_cfg.bits.prefetch_enable = 0;
	compare_cfg_tab->prefetch_cfg.bits.first_32k_page = req_cmp->streams[layer_num][BI_CMP_CUR_FP].buffer >> 15;
	compare_cfg_tab->cmp_cascade_en = (cmp_update_flag == 0) ? CASCADE_DISABLE : CASCADE_ENABLE;
	return ISP_IPP_OK;
}

static int seg_matcher_cmp_set_cmdlst_stripe(
	msg_req_matcher_t *matcher_req,
	cmdlst_para_t *cmp_cmdlst_para,
	global_info_ipp_t *global_info)
{
	unsigned int i = 0;
	unsigned int irq = 0;
	unsigned int stripe_cnt = 0;
	cmdlst_stripe_info_t *cmdlst_stripe = NULL;

	global_info->cmp_layer_stripe_cnt = 1;
	if (matcher_req->req_cmp.streams[0][BO_CMP_MATCHED_INDEX_OUT].buffer) // Include matched_kpt
		global_info->cmp_layer_stripe_cnt += 1;

	if (matcher_req->req_cmp.streams[0][BO_CMP_MATCHED_DIST_OUT].buffer)
		global_info->cmp_layer_stripe_cnt += 1;

	stripe_cnt = matcher_req->cmp_pyramid_layer * global_info->cmp_layer_stripe_cnt;
	stripe_cnt = (global_info->mc_update_flag == 0) ?
					stripe_cnt : (stripe_cnt + UPDATE_MC_MATCH_POINTS_CFG + UPDATE_MC_INDEX_CFG_CFG);
	cmp_cmdlst_para->stripe_cnt  = stripe_cnt;
	cmp_cmdlst_para->channel_id  = global_info->cmp_channel;
	cmp_cmdlst_para->work_module = global_info->work_module;
	cmp_cmdlst_para->cmd_entry_mem_id = MEM_ID_CMDLST_ENTRY_COMPARE;
	cmp_cmdlst_para->cmd_buf_mem_id = MEM_ID_CMDLST_BUF_COMPARE;
	global_info->cmp_stripe_cnt = stripe_cnt;
	cmdlst_stripe = cmp_cmdlst_para->cmd_stripe_info;

	logi("cmp: total_stripe_cnt = %d, cmp_layer_stripe_cnt = %d",
		cmp_cmdlst_para->stripe_cnt, global_info->cmp_layer_stripe_cnt);

	if (cmp_cmdlst_para->stripe_cnt > CMDLST_STRIPE_MAX_NUM) {
		loge("error : invalid stripe_cnt = %d", cmp_cmdlst_para->stripe_cnt);
		return ISP_IPP_ERR;
	}

	for (i = 0; i < stripe_cnt; i++) {
		irq = 0;

		if (i % global_info->cmp_layer_stripe_cnt != 0 || (i != 0 && global_info->mc_update_flag == 1)) {
			irq = irq | (((unsigned int)(1)) << IPP_CMP_CVDR_VP_RD_EOF_CMDLST);
			irq = irq | (((unsigned int)(1)) << IPP_CMP_CVDR_VP_WR_EOF_CMDLST);
		} else {
			irq = irq | (((unsigned int)(1)) << IPP_CMP_CVDR_VP_RD_EOF_CMDLST);
			irq = irq | (((unsigned int)(1)) << IPP_CMP_IRQ_DONE);
			irq = irq | (((unsigned int)(1)) << IPP_CMP_CVDR_VP_RD_EOF_FP);
		}

		logd("seg_matcher_cmp: stripe=%u, irq=0x%x", i, irq);
		cmdlst_stripe[i].hw_priority = CMD_PRIO_HIGH;
		cmdlst_stripe[i].resource_share  = 1 << IPP_CMD_RES_SHARE_CMP;
		cmdlst_stripe[i].irq_module  = 1 << IPP_CMD_IRQ_MODULE_CMP;
		cmdlst_stripe[i].irq_mode        = irq;
		cmdlst_stripe[i].en_link         = 0;
		cmdlst_stripe[i].ch_link         = 0;
		cmdlst_stripe[i].ch_link_act_nbr = 0;
		cmdlst_stripe[i].is_last_stripe  = 0;
		cmdlst_stripe[i].is_need_set_sop = 0;
	}

	if (stripe_cnt >= 1) {
		cmdlst_stripe[stripe_cnt - 1].ch_link = (global_info->mc_en == 0) ? 0 : global_info->mc_channel;
		cmdlst_stripe[stripe_cnt - 1].ch_link_act_nbr = (global_info->mc_en == 0) ? 0 : global_info->mc_stripe_cnt;
		cmdlst_stripe[stripe_cnt - 1].is_last_stripe  = 1;
	}
	cmdlst_stripe[0].hw_priority = CMD_PRIO_LOW;
	return ISP_IPP_OK;
}

static unsigned int seg_matcher_cmp_update_mc_cmdlst_buf(schedule_cmdlst_link_t *cmd_link_entry,
		global_info_ipp_t *global_info, unsigned int i)
{
	cmdlst_wr_cfg_t cmdlst_wr_cfg;
	if (memset_s(&cmdlst_wr_cfg, sizeof(cmdlst_wr_cfg_t), 0, sizeof(cmdlst_wr_cfg_t))) {
		loge("Failed : memset_s");
		return ISP_IPP_ERR;
	}
	cmdlst_wr_cfg.is_incr = 1;
	cmdlst_wr_cfg.read_mode = 0;

	// updata
	cmdlst_wr_cfg.data_size = 1;
	cmdlst_wr_cfg.reg_rd_addr = IPP_BASE_ADDR_COMPARE + COMPARE_MATCH_POINTS_REG;
	cmdlst_wr_cfg.is_wstrb = 1;
	cmdlst_wr_cfg.read_mode = 0;
	cmdlst_wr_cfg.buff_wr_addr = global_info->mc_matched_num_addr;
	loge_if(seg_ipp_set_cmdlst_wr_buf(&cmd_link_entry[i++].cmd_buf, &cmdlst_wr_cfg));

	cmdlst_wr_cfg.data_size = MC_KPT_NUM_MAX;
	cmdlst_wr_cfg.is_wstrb = 0;
	cmdlst_wr_cfg.reg_rd_addr = IPP_BASE_ADDR_COMPARE + COMPARE_INDEX_0_REG;
	cmdlst_wr_cfg.read_mode = 1;
	cmdlst_wr_cfg.buff_wr_addr = global_info->mc_index_addr;
	loge_if(seg_ipp_set_cmdlst_wr_buf(&cmd_link_entry[i++].cmd_buf, &cmdlst_wr_cfg));
	return i;
}

static int seg_matcher_cmp_set_cmdlst_buf(
	msg_req_matcher_t *matcher_req,
	cmdlst_para_t *cmdlst_para,
	seg_matcher_cmp_cfg_t *seg_cfg_tab,
	global_info_ipp_t *global_info)
{
	unsigned int i = 0;
	unsigned int cur_layer = 0;
	cmdlst_wr_cfg_t cmdlst_wr_cfg;
	schedule_cmdlst_link_t *cmd_link_entry = NULL;
	if (memset_s(&cmdlst_wr_cfg, sizeof(cmdlst_wr_cfg_t), 0, sizeof(cmdlst_wr_cfg_t))) {
		loge("Failed : memset_s");
		return ISP_IPP_ERR;
	}
	cmdlst_wr_cfg.is_incr = 1;
	cmdlst_wr_cfg.read_mode = 0;
	cmd_link_entry = (schedule_cmdlst_link_t *)cmdlst_para->cmd_entry;

	for (i = 0; i < cmdlst_para->stripe_cnt;) {
		cur_layer = i / global_info->cmp_layer_stripe_cnt;
		logi("i=%d, cur_layer = %d", i, cur_layer);
		cvdr_prepare_nr_cmd(&g_cvdr_devs[0], &cmd_link_entry[i].cmd_buf, &(seg_cfg_tab->compare_cvdr_cfg_tab[cur_layer]));
		compare_prepare_cmd(&g_compare_devs[0], &cmd_link_entry[i].cmd_buf, &(seg_cfg_tab->compare_cfg_tab[cur_layer]));
		cvdr_prepare_cmd(&g_cvdr_devs[0], &cmd_link_entry[i++].cmd_buf, &(seg_cfg_tab->compare_cvdr_cfg_tab[cur_layer]));

		if (global_info->matcher_update_flag == 1) { // arf_en
			global_info->cmp_kpt_num_addr_cur = seg_cfg_tab->compare_cfg_tab[cur_layer].cmp_cur_kpt_addr;
			global_info->cmp_kpt_num_addr_ref = seg_cfg_tab->compare_cfg_tab[cur_layer].cmp_ref_kpt_addr;
			global_info->cmp_total_num_addr = seg_cfg_tab->compare_cfg_tab[cur_layer].cmp_total_kpt_addr;
			global_info->cmp_frame_height_addr = seg_cfg_tab->compare_cvdr_cfg_tab[cur_layer].cmp_cvdr_frame_size_addr;
		}

		if (matcher_req->req_cmp.streams[cur_layer][BO_CMP_MATCHED_INDEX_OUT].buffer) {
			cmdlst_wr_cfg.data_size = CMP_INDEX_RANGE_MAX;
			cmdlst_wr_cfg.is_wstrb = 0;
			cmdlst_wr_cfg.reg_rd_addr = IPP_BASE_ADDR_COMPARE + COMPARE_INDEX_0_REG;
			cmdlst_wr_cfg.buff_wr_addr = matcher_req->req_cmp.streams[cur_layer][BO_CMP_MATCHED_INDEX_OUT].buffer;
			loge_if(seg_ipp_set_cmdlst_wr_buf_cmp(&cmd_link_entry[i++].cmd_buf, &cmdlst_wr_cfg,
													COMPARE_INDEX_0_REG - COMPARE_MATCH_POINTS_REG));
		}

		if (matcher_req->req_cmp.streams[cur_layer][BO_CMP_MATCHED_DIST_OUT].buffer) {
			cmdlst_wr_cfg.data_size = CMP_INDEX_RANGE_MAX;
			cmdlst_wr_cfg.is_wstrb = 0;
			cmdlst_wr_cfg.reg_rd_addr = IPP_BASE_ADDR_COMPARE + COMPARE_DISTANCE_0_REG;
			cmdlst_wr_cfg.buff_wr_addr = matcher_req->req_cmp.streams[cur_layer][BO_CMP_MATCHED_DIST_OUT].buffer;
			loge_if(seg_ipp_set_cmdlst_wr_buf(&cmd_link_entry[i++].cmd_buf, &cmdlst_wr_cfg));
		}

		if (global_info->mc_update_flag == 1) {
			logd("cmp update mc_matched_num_addr and mc_index_addr");
			i = seg_matcher_cmp_update_mc_cmdlst_buf(cmd_link_entry, global_info, i);
		}
	}

	return ISP_IPP_OK;
}

static int seg_matcher_cmp_get_cpe_mem(seg_matcher_cmp_cfg_t **seg_cfg_tab,
										cmdlst_para_t **cmp_cmdlst_para, unsigned int work_module)
{
	unsigned long long va = 0;
	unsigned int da = 0;
	int ret = 0;

	// 1. cmp cfg_tbl
	ret = cpe_mem_get(MEM_ID_COMPARE_CFG_TAB, work_module, &va, &da);
	if (ret != 0) {
		loge(" Failed : cpe_mem_get %d", MEM_ID_COMPARE_CFG_TAB);
		return ISP_IPP_ERR;
	}

	*seg_cfg_tab = (seg_matcher_cmp_cfg_t *)(uintptr_t)va;

	// 2. cmp cmdlst_para
	ret = cpe_mem_get(MEM_ID_CMDLST_PARA_COMPARE, work_module, &va, &da);
	if (ret != 0) {
		loge("Failed : cpe_mem_get %d", MEM_ID_CMDLST_PARA_COMPARE);
		goto out;
	}

	*cmp_cmdlst_para = (cmdlst_para_t *)(uintptr_t)va;

	return ISP_IPP_OK;
out:
	cpe_mem_free(MEM_ID_COMPARE_CFG_TAB, work_module);
	cpe_mem_free(MEM_ID_CMDLST_PARA_COMPARE, work_module);
	*seg_cfg_tab = NULL;
	*cmp_cmdlst_para = NULL;

	return ISP_IPP_ERR;
}

static int seg_matcher_cmp_prepare_cmdlst_para(
	msg_req_matcher_t *matcher_req,
	seg_matcher_cmp_cfg_t *seg_cfg_tab,
	cmdlst_para_t *cmp_cmdlst_para,
	global_info_ipp_t *global_info)
{
	int ret = 0;

	ret = seg_matcher_cmp_set_cmdlst_stripe(matcher_req, cmp_cmdlst_para, global_info);
	if (ret != 0) {
		loge("Failed : seg_matcher_cmp_set_cmdlst_stripe");
		return ISP_IPP_ERR;
	}

	ret = df_sched_prepare(cmp_cmdlst_para);
	if (ret != 0) {
		loge("Failed : df_sched_prepare");
		return ISP_IPP_ERR;
	}

	ret = seg_matcher_cmp_set_cmdlst_buf(matcher_req, cmp_cmdlst_para, seg_cfg_tab, global_info);
	if (ret != 0) {
		loge("Failed : seg_matcher_cmp_set_cmdlst_buf");
		return ISP_IPP_ERR;
	}

	return ISP_IPP_OK;
}

static int seg_matcher_cmp_set_cfg_tbl(
	msg_req_matcher_t *matcher_req,
	seg_matcher_cmp_cfg_t *seg_cfg_tab,
	global_info_ipp_t *global_info)
{
	unsigned int i = 0;

	for (i = 0; i < matcher_req->cmp_pyramid_layer; i++) {
		if (seg_matcher_cmp_set_cmp_cfg_tbl(matcher_req, &(seg_cfg_tab->compare_cfg_tab[i]), i,
			global_info->matcher_update_flag)) {
			loge("Failed: seg_matcher_cmp_set_cmp_cfg_tbl");
			return ISP_IPP_ERR;
		}
		if (seg_matcher_cmp_set_cvdr_cfg_tbl(matcher_req, &(seg_cfg_tab->compare_cvdr_cfg_tab[i]), i)) {
			loge("Failed: seg_matcher_cmp_set_cvdr_cfg_tbl");
			return ISP_IPP_ERR;
		}
	}
	return ISP_IPP_OK;
}

#ifdef IPP_UT_DEBUG
static void seg_matcher_dump_cmp_req(req_cmp_t *req_cmp, unsigned int cmp_pyramid_layer)
{
	unsigned int i = 0, j= 0;
	cfg_tab_compare_t *reg_cfg = NULL;
	unsigned int tmp_arr[CMP_KPT_NUM_RANGE] = {0};

	if (req_cmp == NULL) return;
	logi("---------dump cmp start---------");
	logi("---------cmp stream's info:");

	for (i = 0; i < cmp_pyramid_layer; i++) {
		for (j = 0; j < CMP_STREAM_MAX; j++) {
			logi("streams[%d][%d].width  = %d", i, j, req_cmp->streams[i][j].width);
			logi("streams[%d][%d].height = %d", i, j, req_cmp->streams[i][j].height);
			logi("streams[%d][%d].stride = %d", i, j, req_cmp->streams[i][j].stride);
			logi("streams[%d][%d].fd     = %d", i, j, req_cmp->streams[i][j].fd);
			logi("streams[%d][%d].format = %d", i, j, req_cmp->streams[i][j].format);
			logi("streams[%d][%d].offset = 0x%08x", i, j, req_cmp->streams[i][j].offset);
			logi("streams[%d][%d].total_size = 0x%x", i, j, req_cmp->streams[i][j].total_size);
		}

		logi("---------cmp reg_cfg, layer.%d:", i);
		reg_cfg = &req_cmp->reg_cfg[i];
		logi("ctrl_cfg: compare_en      = %d", reg_cfg->ctrl_cfg.bits.compare_en);
		logi("block_cfg: blk_v_num      = %d", reg_cfg->blk_cfg.bits.blk_v_num);
		logi("block_cfg: blk_h_num      = %d", reg_cfg->blk_cfg.bits.blk_h_num);
		logi("block_cfg: blk_num        = %d", reg_cfg->blk_cfg.bits.blk_num);
		logi("search_cfg: v_radius      = %d", reg_cfg->search_cfg.bits.v_radius);
		logi("search_cfg: h_radius      = %d", reg_cfg->search_cfg.bits.h_radius);
		logi("search_cfg: dis_ratio     = %d", reg_cfg->search_cfg.bits.dis_ratio);
		logi("search_cfg: dis_threshold = %d", reg_cfg->search_cfg.bits.dis_threshold);
		logi("stat_cfg: stat_en         = %d", reg_cfg->stat_cfg.bits.stat_en);
		logi("stat_cfg: max3_ratio      = %d", reg_cfg->stat_cfg.bits.max3_ratio);
		logi("stat_cfg: bin_factor      = %d", reg_cfg->stat_cfg.bits.bin_factor);
		logi("stat_cfg: bin_num         = %d", reg_cfg->stat_cfg.bits.bin_num);
		logi("offset_cfg: cenh_offset   = %d", reg_cfg->offset_cfg.bits.cenh_offset);
		logi("offset_cfg: cenv_offset   = %d", reg_cfg->offset_cfg.bits.cenv_offset);
		logi("total_kptnum_cfg.total_kptnum = %d", reg_cfg->total_kptnum_cfg.bits.total_kpt_num);

		memcpy_s(tmp_arr, CMP_KPT_NUM_RANGE * sizeof(unsigned int),
				reg_cfg->kptnum_cfg.ref_kpt_num, CMP_KPT_NUM_RANGE * sizeof(cmp_ref_kpt_number_t));
		log_dump_array("ref_kpt_number = ", "%5d, ", tmp_arr, CMP_KPT_NUM_RANGE, 8);

		memcpy_s(tmp_arr, CMP_KPT_NUM_RANGE * sizeof(unsigned int),
				reg_cfg->kptnum_cfg.cur_kpt_num, CMP_KPT_NUM_RANGE * sizeof(cmp_cur_kpt_number_t));
		log_dump_array("cur_kpt_number = ", "%5d, ", tmp_arr, CMP_KPT_NUM_RANGE, 8);
	}

	logi("---------dump cmp end---------");
	return;
}
#endif

static int seg_matcher_cmp_check_para(msg_req_matcher_t *matcher_req)
{
	unsigned int i = 0;
	unsigned int index_cnt = 0;
	unsigned int dist_cnt = 0;

	if (matcher_req->cmp_pyramid_layer < 1 || matcher_req->cmp_pyramid_layer > MATCHER_LAYER_MAX) {
		loge("invalid cmp req cmp_pyramid_layer.%d", matcher_req->cmp_pyramid_layer);
		return ISP_IPP_ERR;
	}
	if (matcher_req->matcher_work_mode < NORMAL_MATCHER_MODE || matcher_req->matcher_work_mode >= MATCHER_MODE_MAX) {
		loge("invalid matcher_work_mode.%d", matcher_req->matcher_work_mode);
		return ISP_IPP_ERR;
	}

#ifdef IPP_UT_DEBUG
	seg_matcher_dump_cmp_req(&matcher_req->req_cmp, matcher_req->cmp_pyramid_layer);
#endif
	for (i = 0; i < matcher_req->cmp_pyramid_layer; i++) {
		if (matcher_req->req_cmp.streams[i][BI_CMP_CUR_FP].buffer == 0 ||
			matcher_req->req_cmp.streams[i][BI_CMP_REF_FP].buffer == 0 ||
			(matcher_req->req_cmp.streams[i][BI_CMP_CUR_FP].buffer & 0x7FFF) != 0) {
			loge("Failed: req_cmp streams input buffer invalid");
			return ISP_IPP_ERR;
		}
		if (matcher_req->req_cmp.streams[i][BO_CMP_MATCHED_INDEX_OUT].buffer != 0)
			index_cnt += 1;

		if (matcher_req->req_cmp.streams[i][BO_CMP_MATCHED_DIST_OUT].buffer != 0)
			dist_cnt += 1;
	}

	if ((index_cnt != 0) && (index_cnt != matcher_req->cmp_pyramid_layer)) {
		loge("Failed: index_cnt.%d, cmp_pyramid_layer.%d", index_cnt, matcher_req->cmp_pyramid_layer);
		return ISP_IPP_ERR;
	}
	if ((dist_cnt != 0) && (dist_cnt != matcher_req->cmp_pyramid_layer)) {
		loge("Failed: dist_cnt.%d, cmp_pyramid_layer.%d", index_cnt, matcher_req->cmp_pyramid_layer);
		return ISP_IPP_ERR;
	}
	return ISP_IPP_OK;
}

int seg_matcher_cmp_request_handler(
	msg_req_matcher_t *matcher_req,
	global_info_ipp_t *global_info)
{
	int ret = 0;
	seg_matcher_cmp_cfg_t *seg_cfg_tab = NULL;
	cmdlst_para_t *cmp_cmdlst_para = NULL;
	logd("+");

	if (matcher_req->cmp_pyramid_layer == 0) {
		logi("matcher req: cmp_pyramid_layer is 0, do nothing");
		return ISP_IPP_OK;
	}

	ret = seg_matcher_cmp_check_para(matcher_req);
	if (ret != 0) {
		loge("Failed : seg_matcher_cmp_check_para");
		return ISP_IPP_ERR;
	}

	ret = seg_matcher_cmp_get_cpe_mem(&seg_cfg_tab, &cmp_cmdlst_para, global_info->work_module);
	if (ret != 0) {
		loge("Failed : seg_matcher_cmp_get_cpe_mem");
		return ISP_IPP_ERR;
	}

	ret = seg_matcher_cmp_set_cfg_tbl(matcher_req, seg_cfg_tab, global_info);
	if (ret != 0) {
		loge("Failed : seg_ipp_path_cmp_prepare_cmdlst_para");
		goto out;
	}

	ret = seg_matcher_cmp_prepare_cmdlst_para(matcher_req, seg_cfg_tab, cmp_cmdlst_para, global_info);
	if (ret != 0) {
		loge("Failed : seg_ipp_path_cmp_prepare_cmdlst_para");
		goto out;
	}

	ret = df_sched_start(cmp_cmdlst_para);
	if (ret != 0)
		loge("Failed : df_sched_start");

out:
	cpe_mem_free(MEM_ID_COMPARE_CFG_TAB, global_info->work_module);
	logd("-");
	return ret;
}

// end
