/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name  : cfg_table_ipp_path.h
 * Description: for ipp path
 *
 * Date         2022-06-27 16:28:10
 ********************************************************************/

#ifndef __CFG_TABLE_IPP_PATH_CS_H__
#define __CFG_TABLE_IPP_PATH_CS_H__
#include "ippmessage_common.h"
#include "cfg_table_arfeature.h"
#include "cfg_table_orb_enh.h"
#include "cfg_table_compare.h"
#include "cfg_table_reorder.h"
#include "cfg_table_mc.h"
#include "cfg_table_klt.h"
#include "cfg_table_cvdr.h"

typedef enum {
	LAYER_KLT_DS_FOR_PRE = 0,
	LAYER_KLT_DS_FOR_NXT,
	LAYER_KLT_DO_KLT,
	KLT_MAX_LAYER,
} klt_layers_e;

// for ipp_path
typedef struct _seg_enh_arf_cfg_t {
	cfg_tab_orb_enh_t enh_cur;
	cfg_tab_arf_t  arf_cur[ARFEATURE_MAX_LAYER];
	cfg_tab_cvdr_t arf_cvdr_cur[ARFEATURE_MAX_LAYER]; // arf cvdr include enh cvdr

	cfg_tab_orb_enh_t enh_ref;
	cfg_tab_arf_t  arf_ref[ARFEATURE_MAX_LAYER];
	cfg_tab_cvdr_t arf_cvdr_ref[ARFEATURE_MAX_LAYER];
} seg_enh_arf_cfg_t;

typedef struct _seg_matcher_rdr_cfg_t {
	cfg_tab_reorder_t reorder_cfg_tab[MATCHER_LAYER_MAX];
	cfg_tab_cvdr_t reorder_cvdr_cfg_tab[MATCHER_LAYER_MAX];
} seg_matcher_rdr_cfg_t;

typedef struct _seg_matcher_cmp_cfg_t {
	cfg_tab_compare_t compare_cfg_tab[MATCHER_LAYER_MAX];
	cfg_tab_cvdr_t compare_cvdr_cfg_tab[MATCHER_LAYER_MAX];
} seg_matcher_cmp_cfg_t;

typedef struct _seg_mc_cfg_t {
	cfg_tab_mc_t mc_cfg_tab;
	cfg_tab_cvdr_t mc_cvdr_cfg_tab;
} seg_mc_cfg_t;

typedef struct _seg_klt_cfg_t {
	cfg_tab_klt_t klt_cfg_tab[KLT_MAX_LAYER];
	cfg_tab_cvdr_t klt_cvdr_cfg_tab[KLT_MAX_LAYER];
} seg_klt_cfg_t;

#endif /* __CFG_TABLE_IPP_PATH_CS_H__ */
