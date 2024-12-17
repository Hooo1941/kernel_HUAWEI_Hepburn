/*******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name    segment_arfeature.h
 * Description:
 *
 * Date        2020-04-23
 ******************************************************************/

#ifndef __SEG_ARFEATURE_CS_H__
#define __SEG_ARFEATURE_CS_H__

#include "ippmessage.h"
#include "segment_common.h"

typedef struct _seg_enh_arf_tmp_info {
	unsigned int i;
	unsigned int pre_stripe_num;
} seg_enh_arf_tmp_info;

#define SEG_ENH_ARF_MAX_BLK_H_NUM       (16)
#define SEG_ENH_ARF_MAX_BLK_V_NUM       (16)
#define SEG_ENH_ARF_GRID_NUM_RANG       (SEG_ENH_ARF_MAX_BLK_H_NUM * SEG_ENH_ARF_MAX_BLK_V_NUM)
#define SEG_ENH_ARF_KPT_LIMIT_RANG      (256)
#define SEG_ENH_ARF_MINSCR_KPTCNT_SIZE  (align_up((SEG_ENH_ARF_GRID_NUM_RANG*4), CVDR_ALIGN_BYTES))
#define SEG_ENH_ARF_SUM_SCORE_SIZE      (align_up((SEG_ENH_ARF_GRID_NUM_RANG*4), CVDR_ALIGN_BYTES))

int seg_enh_arf_request_handler(msg_req_enh_arf_t *req, global_info_ipp_t *global_info);

#endif // __SEG_ARFEATURE_CS_H__ 
