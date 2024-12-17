/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name  :
 * Description:
 *
 * Date         2022-06-27 16:28:10
 ********************************************************************/

#ifndef __CFG_TABLE_REORDER_CS_H__
#define __CFG_TABLE_REORDER_CS_H__

#include "reorder_drv_priv.h"

#define RDR_KPT_NUM_RANGE          (256)

typedef union reorder_cfg           rdr_ctrl_cfg_t;
typedef union reorder_block_cfg     rdr_block_cfg_t;
typedef union reorder_total_kpt_num rdr_total_kptnum_cfg_t;
typedef union reorder_prefetch_cfg  rdr_prefetch_cfg_t;

typedef union reorder_kpt_number    rdr_kptnum_cfg_t;
typedef struct _cfg_tab_reorder_t {
	rdr_ctrl_cfg_t          ctrl_cfg;
	rdr_block_cfg_t         block_cfg;
	rdr_total_kptnum_cfg_t  total_kptnum_cfg;
	rdr_prefetch_cfg_t      prefetch_cfg;
	rdr_kptnum_cfg_t        rdr_kptnum_cfg[RDR_KPT_NUM_RANGE];

	unsigned int rdr_cascade_en;
	unsigned int rdr_kpt_num_addr; // address in cmdlst_buffer
	unsigned int rdr_total_num_addr; // address in cmdlst_buffer
} cfg_tab_reorder_t;

#endif /* __CFG_TABLE_REORDER_CS_H__ */
