/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name  :
 * Description:
 *
 * Date         2022-06-27 16:28:10
 ********************************************************************/

#ifndef __CFG_TABLE_COMPARE_CS_H__
#define __CFG_TABLE_COMPARE_CS_H__

#include "compare_drv_priv.h"

#define CMP_KPT_NUM_RANGE		(256)


typedef union compare_cfg            cmp_ctrl_cfg_t;
typedef union compare_block_cfg      cmp_block_cfg_t;
typedef union compare_search_cfg     cmp_search_cfg_t;
typedef union compare_stat_cfg       cmp_stat_cfg_t;
typedef union compare_prefetch_cfg   cmp_prefetch_cfg_t;
typedef union compare_offset_cfg     cmp_offset_cfg_t;
typedef union compare_total_kpt_num  cmp_total_kptnum_cfg_t;
typedef union compare_ref_kpt_number cmp_ref_kpt_number_t;
typedef union compare_cur_kpt_number cmp_cur_kpt_number_t;

typedef struct _cmp_kptnum_cfg_t {
	cmp_ref_kpt_number_t ref_kpt_num[CMP_KPT_NUM_RANGE];
	cmp_cur_kpt_number_t cur_kpt_num[CMP_KPT_NUM_RANGE];
} cmp_kptnum_cfg_t;

typedef struct _cfg_tab_compare_t {
	cmp_ctrl_cfg_t     ctrl_cfg;
	cmp_block_cfg_t    blk_cfg;
	cmp_search_cfg_t   search_cfg;
	cmp_offset_cfg_t   offset_cfg;
	cmp_stat_cfg_t     stat_cfg;
	cmp_total_kptnum_cfg_t  total_kptnum_cfg;
	cmp_prefetch_cfg_t prefetch_cfg;
	cmp_kptnum_cfg_t   kptnum_cfg;

	unsigned int    cmp_cascade_en; // used for ipp_path
	unsigned int    cmp_total_kpt_addr; // address in cmdlst_buffer
	unsigned int    cmp_ref_kpt_addr; // address in cmdlst_buffer
	unsigned int    cmp_cur_kpt_addr; // address in cmdlst_buffer
} cfg_tab_compare_t;

#endif /* __CFG_TABLE_COMPARE_CS_H__ */
