/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name    segment_ipp_path.c
 * Description:
 *
 * Date         2020-04-17 16:28:10
 ********************************************************************/
#include <linux/string.h>
#include <linux/printk.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/io.h>
#include "segment_ipp_path.h"
#include "cfg_table_ipp_path.h"
#include "cmdlst_reg_offset.h"

#define LOG_TAG LOG_MODULE_IPP_PATH
#define ACTIVE_TOKEN_NBR_EN_SHIFT 8

char *work_mod[] = {
	"KLT_ONLY",
	"ENH_ARF",
	"ENH_ARF_KLT",
	"ENH_ARF_MATCHER",
	"ENH_ARF_MATCHER_MC",

	// UT test mode
	"ENH_ONLY",
	"ARFEATURE_ONLY",
	"MATCHER_ONLY",
	"MC_ONLY",
	"ARF_MATCHER",
	"MATCHER_MC",
	"ARF_MATCHER_MC",
	"WORK_MODULE_MAX",
};

char *work_fra[] = {
	"CUR_ONLY",
	"CUR_REF",
};

static int get_feature_flag(global_info_ipp_t *p_global_info, work_module_e work_module,
	struct msg_req_ipp_cmd_req_t *req)
{
	unsigned int enh_enable;
	unsigned int arf_enable;
	unsigned int klt_enable;

	logd("+");
	enh_enable = req->req_enh_arf.req_enh_cur.reg_cfg.enable_cfg.bits.enh_en;
	arf_enable = req->req_enh_arf.req_arf_cur.reg_cfg[0].top_cfg.bits.ar_feature_en;
	klt_enable = req->req_klt.work_mode;
	p_global_info->work_module = work_module;
	logi("p_global_info->work_module=%u, name.%s", p_global_info->work_module, work_mod[p_global_info->work_module]);
	switch (work_module) {
	case KLT_ONLY:
		p_global_info->klt_en = 1;
		p_global_info->klt_update_flag = 0;
		break;
	case ENH_ARF:
		p_global_info->enh_en = enh_enable;
		p_global_info->arf_en = 1;
		break;
	case ENH_ARF_KLT:
		p_global_info->enh_en = enh_enable;
		p_global_info->arf_en = arf_enable; // second frame arf is off
		if (klt_enable == KLT_OFF)
			p_global_info->klt_en = 0;
		else
		    p_global_info->klt_en = 1;
		break;
	case ENH_ARF_MATCHER:
		p_global_info->enh_en = enh_enable;
		p_global_info->arf_en = 1;
		p_global_info->matcher_en = 1;
		p_global_info->matcher_update_flag = 1;
		break;
	case ENH_ARF_MATCHER_MC:
		p_global_info->enh_en = enh_enable;
		p_global_info->arf_en = 1;
		p_global_info->matcher_en = 1;
		p_global_info->matcher_update_flag = 1;
		p_global_info->mc_en = 1;
		p_global_info->mc_update_flag = 1;
		break;

	case ENH_ONLY:
		p_global_info->enh_en = 1;
		break;
	case ARFEATURE_ONLY:
		p_global_info->arf_en = 1;
		break;
	case MATCHER_ONLY:
		p_global_info->matcher_en = 1;
		p_global_info->matcher_update_flag = 0;
		break;
	case MC_ONLY:
		p_global_info->mc_en = 1;
		p_global_info->mc_update_flag = 0;
		break;
	case ARF_MATCHER:
		p_global_info->arf_en = 1;
		p_global_info->matcher_en = 1;
		p_global_info->matcher_update_flag = 1;
		break;
	case MATCHER_MC:
		p_global_info->matcher_en = 1;
		p_global_info->matcher_update_flag = 0;
		p_global_info->mc_en = 1;
		p_global_info->mc_update_flag = 1;
		break;
	case ARF_MATCHER_MC:
		p_global_info->arf_en = 1;
		p_global_info->matcher_en = 1;
		p_global_info->matcher_update_flag = 1;
		p_global_info->mc_en = 1;
		p_global_info->mc_update_flag = 1;
		break;

	default:
		loge("error:Invalid work_module.%d", work_module);
		return ISP_IPP_ERR;
	}

	logi("enh_en.%d, arf_en.%d, matcher_en.%d, mc_en.%d, klt_en.%d",
		p_global_info->enh_en, p_global_info->arf_en,
		p_global_info->matcher_en, p_global_info->mc_en, p_global_info->klt_en);
	logi("matcher_update_flag.%d, mc_update_flag.%d, klt_update_flag.%d",
		p_global_info->matcher_update_flag, p_global_info->mc_update_flag, p_global_info->klt_update_flag);
	logd("-");
	return ISP_IPP_OK;
}

static void get_channel_id(global_info_ipp_t  *p_global_info, work_module_e work_module)
{
	if (g_channel_map[work_module].enh_arf_channel != CMDLST_CHANNEL_MAX) // g_channel_map is global
		p_global_info->enh_arf_channel = g_channel_map[work_module].enh_arf_channel; // get channel id

	if (g_channel_map[work_module].enh_arf_channel != CMDLST_CHANNEL_MAX)
		p_global_info->enh_arf_channel = g_channel_map[work_module].enh_arf_channel;

	if (g_channel_map[work_module].rdr_channel != CMDLST_CHANNEL_MAX)
		p_global_info->rdr_channel = g_channel_map[work_module].rdr_channel;

	if (g_channel_map[work_module].cmp_channel != CMDLST_CHANNEL_MAX)
		p_global_info->cmp_channel = g_channel_map[work_module].cmp_channel;

	if (g_channel_map[work_module].mc_channel != CMDLST_CHANNEL_MAX)
		p_global_info->mc_channel = g_channel_map[work_module].mc_channel;

	if (g_channel_map[work_module].klt_channel != CMDLST_CHANNEL_MAX)
		p_global_info->klt_channel = g_channel_map[work_module].klt_channel;

	return;
}

int ipp_path_request_handler(struct msg_req_ipp_cmd_req_t *req)
{
	int ret = 0;
	unsigned int active_token_nbr_en = 0;
	unsigned int cmdlst_channel_value = 0;
	// 1. Creating a Private Resource: global_info;
	global_info_ipp_t global_info = {0};

	logi("strat process: frame_num.%d, instance_id.%d, work_module.%d",
		req->frame_num, req->instance_id, req->work_module);
	// 2.Obtain module resources and channel resources.
	get_channel_id(&global_info, req->work_module);
	get_feature_flag(&global_info, req->work_module, req);

	// 3.Call seg handler according to the flag of each module.
	if (global_info.klt_en == 1) {
		active_token_nbr_en = (global_info.arf_en == 1 || global_info.enh_en == 1) ? 1 : 0;
		cmdlst_channel_value = (active_token_nbr_en << ACTIVE_TOKEN_NBR_EN_SHIFT);
		hispcpe_reg_set(CMDLIST_REG, CMDLST_CH_CFG_0_REG + global_info.klt_channel * 0x80, cmdlst_channel_value);

		ret = seg_klt_request_handler(&req->req_klt, &global_info);
		if (ret != 0) {
			loge("Failed : klt_request_handler");
			return ISP_IPP_ERR;
		}
	}

	if (global_info.mc_en == 1) {
		active_token_nbr_en = (global_info.matcher_en == 1) ? 1 : 0;
		cmdlst_channel_value = (active_token_nbr_en << ACTIVE_TOKEN_NBR_EN_SHIFT);
		hispcpe_reg_set(CMDLIST_REG, CMDLST_CH_CFG_0_REG + global_info.mc_channel * 0x80, cmdlst_channel_value);

		ret = seg_mc_request_handler(&req->req_mc, &global_info);
		if (ret != 0) {
			loge("Failed : seg_mc_request_handler");
			return ISP_IPP_ERR;
		}
	}

	if (global_info.matcher_en == 1) {
		logi("rdr_pyramid_layer.%d, cmp_pyramid_layer.%d, matcher_work_mode.%d",
			req->req_matcher.rdr_pyramid_layer, req->req_matcher.cmp_pyramid_layer, req->req_matcher.matcher_work_mode);
		active_token_nbr_en = 1;
		if (global_info.arf_en == 0) // for matcher only, rdr_only, matcher_mc work module
			active_token_nbr_en = 0;

		cmdlst_channel_value = (active_token_nbr_en << ACTIVE_TOKEN_NBR_EN_SHIFT);
		hispcpe_reg_set(CMDLIST_REG, CMDLST_CH_CFG_0_REG + global_info.rdr_channel * 0x80, cmdlst_channel_value);

		active_token_nbr_en = (req->req_matcher.rdr_pyramid_layer == 0) ? (0) : (1);
		cmdlst_channel_value = (active_token_nbr_en << ACTIVE_TOKEN_NBR_EN_SHIFT);
		hispcpe_reg_set(CMDLIST_REG, CMDLST_CH_CFG_0_REG + global_info.cmp_channel * 0x80, cmdlst_channel_value);

		ret = seg_matcher_cmp_request_handler(&req->req_matcher, &global_info);
		if (ret != 0) {
			loge("Failed : seg_matcher_cmp_request_handler");
			return ISP_IPP_ERR;
		}

		ret = seg_matcher_rdr_request_handler(&req->req_matcher, &global_info);
		if (ret != 0) {
			loge("Failed : seg_matcher_rdr_request_handler");
			return ISP_IPP_ERR;
		}
	}

	if (global_info.enh_en == 1 || global_info.arf_en == 1) { // The ENH and ARF share the same channel.
		ret = seg_enh_arf_request_handler(&req->req_enh_arf, &global_info);
		if (ret != 0) {
			loge("Failed : seg_enh_arf_request_handler");
			return ISP_IPP_ERR;
		}
	}
	logi("end process: frame_num.%d, instance_id.%d, work_module.%d",
		req->frame_num, req->instance_id, req->work_module);
	// 4.ret status
	return ISP_IPP_OK;
}

