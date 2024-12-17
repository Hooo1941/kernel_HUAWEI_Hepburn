/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>

#include "dkmd_log.h"
#include "dkmd_object.h"
#include "dkmd_opr_id.h"
#include "dpu_opr_config.h"
#include "dpu_config_utils.h"

static uint32_t s_scf_lut_addr_tlb_v110[] = {
	DPU_WCH1_OFFSET,  /* only wch1 support scf */
	DPU_ARSR0_OFFSET,
	DPU_ARSR1_OFFSET,
	DPU_VSCF0_OFFSET,
	DPU_VSCF1_OFFSET,
};

struct dbuf_config_info g_dbuf_config_info;

// RQOS_IN, RQOS_OUT, FLUX_REQ_BEF_IN, FLUX_REQ_BEF_OUT
static uint32_t g_dbuf_thd_req[DBUF_THD_TYPE_MAX] = {40, 45, 35, 40};

// Pool A
static struct lbuf_node_config_info g_lb_pool_a[] = {
	{0, 44}, // SDMA0~2->OV0~2
	{1, 4}, // SDMA0~2->OV0~2/CLD0
	{2, 4}, // SDMA0~2->OV0~2/CLD0/SROT0
};

// Pool B
static struct lbuf_node_config_info g_lb_pool_b[] = {
	{0, 8}, // OV0~2/SDMA0~2->ARSR0~1/VSCF0~1/HDR/UVUP
	{1, 24}, // SDMA0~2->ARSR0~1/VSCF0~1/HDR/UVUP | SROT0->ARSR1/VSCF1
	{2, 12}, // OV0~2/SDMA0~2->ARSR0~1/VSCF0~1/HDR/UVUP | SROT0->ARSR1/VSCF1
};

// Pool C
static struct lbuf_node_config_info g_lb_pool_c[] = {
	{0, 12}, // HDR/VSCF0~1/ARSR0~1/UVUP->OV0~2/HDR/ARSR0~1
	{1, 12}, // HDR/VSCF0~1/ARSR0~1/UVUP->OV0~2/HDR/ARSR0~1 | SROT0->ARSR1/VSCF1
};

// Pool D: CLD
static struct lbuf_node_config_info g_lb_pool_d[] = {
	{0, 4}, // CLD0->OV0~2
};

// Pool E: WCH1
static struct lbuf_node_config_info g_lb_pool_e[] = {
	{0, 6}, // UVUP/HDR/OV1~2->WCH1
};

// Pool F: WCH2
static struct lbuf_node_config_info g_lb_pool_f[] = {
	{0, 6}, // VSCF0~1/OV1~2->WCH2 | SROT0->ARSR1/VSCF1
};

static struct lbuf_pool_config_info g_lb_pool[] = {
	{array_len(g_lb_pool_a), (struct lbuf_node_config_info*)g_lb_pool_a},
	{array_len(g_lb_pool_b), (struct lbuf_node_config_info*)g_lb_pool_b},
	{array_len(g_lb_pool_c), (struct lbuf_node_config_info*)g_lb_pool_c},
	{array_len(g_lb_pool_d), (struct lbuf_node_config_info*)g_lb_pool_d},
	{array_len(g_lb_pool_e), (struct lbuf_node_config_info*)g_lb_pool_e},
	{array_len(g_lb_pool_f), (struct lbuf_node_config_info*)g_lb_pool_f},
};

static struct lbuf_pool_node_config_info g_lb_pool_node_config_info = {
	array_len(g_lb_pool), (struct lbuf_pool_config_info*)g_lb_pool
};

static int32_t g_node_0[] = {
	HW_NODE_ID_0,
};

static int32_t g_node_1[] = {
	HW_NODE_ID_1,
};

static int32_t g_node_2[] = {
	HW_NODE_ID_2,
};

static int32_t g_node_0_1[] = {
	HW_NODE_ID_0, HW_NODE_ID_1,
};

static int32_t g_node_0_2[] = {
	HW_NODE_ID_0, HW_NODE_ID_2,
};

static int32_t g_node_1_2[] = {
	HW_NODE_ID_1, HW_NODE_ID_2,
};

static int32_t g_node_0_1_2[] = {
	HW_NODE_ID_0, HW_NODE_ID_1, HW_NODE_ID_2,
};

// lbuf doc HW_POOL_ID_A has below HW_NODE_ID
static struct lbuf_conn_pool_config_info g_lb_conn_pool_a_cfg_info[CONN_POOL_INDEX_MAX] = {
	{HW_POOL_ID_A, array_len(g_node_0_1_2), (int32_t *)g_node_0_1_2},
	{HW_POOL_ID_A, array_len(g_node_1_2), (int32_t *)g_node_1_2},
	{HW_POOL_ID_A, array_len(g_node_2), (int32_t *)g_node_2},
};

// lbuf doc HW_POOL_ID_B has below HW_NODE_ID
static struct lbuf_conn_pool_config_info g_lb_conn_pool_b_cfg_info[CONN_POOL_INDEX_MAX] = {
	{HW_POOL_ID_B, array_len(g_node_0_1_2), (int32_t *)g_node_0_1_2},
	{HW_POOL_ID_B, array_len(g_node_0_2), (int32_t *)g_node_0_2},
	{HW_POOL_ID_B, array_len(g_node_1_2), (int32_t *)g_node_1_2},
};

// lbuf doc HW_POOL_ID_C has below HW_NODE_ID
static struct lbuf_conn_pool_config_info g_lb_conn_pool_c_cfg_info[CONN_POOL_INDEX_2] = {
	{HW_POOL_ID_C, array_len(g_node_1), (int32_t *)g_node_1},
	{HW_POOL_ID_C, array_len(g_node_0_1), (int32_t *)g_node_0_1},
};

// lbuf doc HW_POOL_ID_D has below HW_NODE_ID
static struct lbuf_conn_pool_config_info g_lb_conn_pool_d_cfg_info[CONN_POOL_INDEX_1] = {
	{HW_POOL_ID_D, array_len(g_node_0), (int32_t *)g_node_0},
};

// lbuf doc HW_POOL_ID_E has below HW_NODE_ID
static struct lbuf_conn_pool_config_info g_lb_conn_pool_e_cfg_info[CONN_POOL_INDEX_1] = {
	{HW_POOL_ID_E, array_len(g_node_0), (int32_t *)g_node_0},
};

// lbuf doc HW_POOL_ID_F has below HW_NODE_ID
static struct lbuf_conn_pool_config_info g_lb_conn_pool_f_cfg_info[CONN_POOL_INDEX_1] = {
	{HW_POOL_ID_F, array_len(g_node_0), (int32_t *)g_node_0},
};

static struct lbuf_conn_pool_node_config_info g_sdma0_ov0_pool_node_config_info[] = {
	// HW_NODE_ID_0, HW_NODE_ID_1, HW_NODE_ID_2,
	{CONN_POOL_INDEX_0, (struct lbuf_conn_pool_config_info *)g_lb_conn_pool_a_cfg_info},
};

static struct lbuf_conn_pool_node_config_info g_sdma0_cld0_pool_node_config_info[] = {
	// HW_NODE_ID_1, HW_NODE_ID_2,
	{CONN_POOL_INDEX_1, (struct lbuf_conn_pool_config_info *)g_lb_conn_pool_a_cfg_info},
};

static struct lbuf_conn_pool_node_config_info g_sdma0_srot0_pool_node_config_info[] = {
	// HW_NODE_ID_2,
	{CONN_POOL_INDEX_2, (struct lbuf_conn_pool_config_info *)g_lb_conn_pool_a_cfg_info},
};

static struct lbuf_conn_pool_node_config_info g_sdma0_arsr0_pool_node_config_info[] = {
	// HW_NODE_ID_0, HW_NODE_ID_1, HW_NODE_ID_2,
	{CONN_POOL_INDEX_0, (struct lbuf_conn_pool_config_info *)g_lb_conn_pool_b_cfg_info},
};

static struct lbuf_conn_pool_node_config_info g_ov0_arsr0_pool_node_config_info[] = {
	// HW_NODE_ID_0, HW_NODE_ID_2,
	{CONN_POOL_INDEX_1, (struct lbuf_conn_pool_config_info *)g_lb_conn_pool_b_cfg_info},
};

static struct lbuf_conn_pool_node_config_info g_ov1_wch1_pool_node_config_info[] = {
	// HW_NODE_ID_0,
	{CONN_POOL_INDEX_0, (struct lbuf_conn_pool_config_info *)g_lb_conn_pool_e_cfg_info},
};

static struct lbuf_conn_pool_node_config_info g_ov1_wch2_pool_node_config_info[] = {
	// HW_NODE_ID_0,
	{CONN_POOL_INDEX_0, (struct lbuf_conn_pool_config_info *)g_lb_conn_pool_f_cfg_info},
};

static struct lbuf_conn_pool_node_config_info g_srot0_arsr1_pool_node_config_info[] = {
	// HW_NODE_ID_1, HW_NODE_ID_2,
	{CONN_POOL_INDEX_2, (struct lbuf_conn_pool_config_info *)g_lb_conn_pool_b_cfg_info},
	// HW_NODE_ID_1,
	{CONN_POOL_INDEX_0, (struct lbuf_conn_pool_config_info *)g_lb_conn_pool_c_cfg_info},
	// HW_NODE_ID_0,
	{CONN_POOL_INDEX_0, (struct lbuf_conn_pool_config_info *)g_lb_conn_pool_f_cfg_info},
};

static struct lbuf_conn_pool_node_config_info g_hdr_ov0_pool_node_config_info[] = {
	// HW_NODE_ID_0, HW_NODE_ID_1,
	{CONN_POOL_INDEX_1, (struct lbuf_conn_pool_config_info *)g_lb_conn_pool_c_cfg_info},
};

static struct lbuf_conn_pool_node_config_info g_cld0_ov0_pool_node_config_info[] = {
	// HW_NODE_ID_0,
	{CONN_POOL_INDEX_0, (struct lbuf_conn_pool_config_info *)g_lb_conn_pool_d_cfg_info},
};

static struct lbuf_conn_config_info g_lb_conn_config_info[] = {
	// SDMA0->
	{(0 << 8) | OPERATOR_SDMA, (0 << 8) | OPERATOR_OV, array_len(g_sdma0_ov0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_sdma0_ov0_pool_node_config_info}, // OV0
	{(0 << 8) | OPERATOR_SDMA, (1 << 8) | OPERATOR_OV, array_len(g_sdma0_ov0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_sdma0_ov0_pool_node_config_info}, // OV1
	{(0 << 8) | OPERATOR_SDMA, (2 << 8) | OPERATOR_OV, array_len(g_sdma0_ov0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_sdma0_ov0_pool_node_config_info}, // OV2
	{(0 << 8) | OPERATOR_SDMA, (0 << 8) | OPERATOR_CLD, array_len(g_sdma0_cld0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_sdma0_cld0_pool_node_config_info}, // CLD0
	{(0 << 8) | OPERATOR_SDMA, (1 << 8) | OPERATOR_SROT, array_len(g_sdma0_srot0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_sdma0_srot0_pool_node_config_info}, // SROT0
	{(0 << 8) | OPERATOR_SDMA, (0 << 8) | OPERATOR_ARSR, array_len(g_sdma0_arsr0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_sdma0_arsr0_pool_node_config_info}, // ARSR0
	{(0 << 8) | OPERATOR_SDMA, (1 << 8) | OPERATOR_ARSR, array_len(g_sdma0_arsr0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_sdma0_arsr0_pool_node_config_info}, // ARSR1
	{(0 << 8) | OPERATOR_SDMA, (0 << 8) | OPERATOR_VSCF, array_len(g_sdma0_arsr0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_sdma0_arsr0_pool_node_config_info}, // VSCF0
	{(0 << 8) | OPERATOR_SDMA, (1 << 8) | OPERATOR_VSCF, array_len(g_sdma0_arsr0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_sdma0_arsr0_pool_node_config_info}, // VSCF1
	{(0 << 8) | OPERATOR_SDMA, (0 << 8) | OPERATOR_HDR, array_len(g_sdma0_arsr0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_sdma0_arsr0_pool_node_config_info}, // HDR
	{(0 << 8) | OPERATOR_SDMA, (0 << 8) | OPERATOR_UVUP, array_len(g_sdma0_arsr0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_sdma0_arsr0_pool_node_config_info}, // UVUP 10

	// SDMA1->
	{(1 << 8) | OPERATOR_SDMA, (0 << 8) | OPERATOR_OV, array_len(g_sdma0_ov0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_sdma0_ov0_pool_node_config_info}, // OV0
	{(1 << 8) | OPERATOR_SDMA, (1 << 8) | OPERATOR_OV, array_len(g_sdma0_ov0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_sdma0_ov0_pool_node_config_info}, // OV1
	{(1 << 8) | OPERATOR_SDMA, (2 << 8) | OPERATOR_OV, array_len(g_sdma0_ov0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_sdma0_ov0_pool_node_config_info}, // OV2
	{(1 << 8) | OPERATOR_SDMA, (0 << 8) | OPERATOR_CLD, array_len(g_sdma0_cld0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_sdma0_cld0_pool_node_config_info}, // CLD0
	{(1 << 8) | OPERATOR_SDMA, (1 << 8) | OPERATOR_SROT, array_len(g_sdma0_srot0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_sdma0_srot0_pool_node_config_info}, // SROT0
	{(1 << 8) | OPERATOR_SDMA, (0 << 8) | OPERATOR_ARSR, array_len(g_sdma0_arsr0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_sdma0_arsr0_pool_node_config_info}, // ARSR0
	{(1 << 8) | OPERATOR_SDMA, (1 << 8) | OPERATOR_ARSR, array_len(g_sdma0_arsr0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_sdma0_arsr0_pool_node_config_info}, // ARSR1
	{(1 << 8) | OPERATOR_SDMA, (0 << 8) | OPERATOR_VSCF, array_len(g_sdma0_arsr0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_sdma0_arsr0_pool_node_config_info}, // VSCF0
	{(1 << 8) | OPERATOR_SDMA, (1 << 8) | OPERATOR_VSCF, array_len(g_sdma0_arsr0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_sdma0_arsr0_pool_node_config_info}, // VSCF1
	{(1 << 8) | OPERATOR_SDMA, (0 << 8) | OPERATOR_HDR, array_len(g_sdma0_arsr0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_sdma0_arsr0_pool_node_config_info}, // HDR
	{(1 << 8) | OPERATOR_SDMA, (0 << 8) | OPERATOR_UVUP, array_len(g_sdma0_arsr0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_sdma0_arsr0_pool_node_config_info}, // UVUP 21

	// SDMA2->
	{(2 << 8) | OPERATOR_SDMA, (0 << 8) | OPERATOR_OV, array_len(g_sdma0_ov0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_sdma0_ov0_pool_node_config_info}, // OV0
	{(2 << 8) | OPERATOR_SDMA, (1 << 8) | OPERATOR_OV, array_len(g_sdma0_ov0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_sdma0_ov0_pool_node_config_info}, // OV1
	{(2 << 8) | OPERATOR_SDMA, (2 << 8) | OPERATOR_OV, array_len(g_sdma0_ov0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_sdma0_ov0_pool_node_config_info}, // OV2
	{(2 << 8) | OPERATOR_SDMA, (0 << 8) | OPERATOR_CLD, array_len(g_sdma0_cld0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_sdma0_cld0_pool_node_config_info}, // CLD0
	{(2 << 8) | OPERATOR_SDMA, (1 << 8) | OPERATOR_SROT, array_len(g_sdma0_srot0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_sdma0_srot0_pool_node_config_info}, // SROT0
	{(2 << 8) | OPERATOR_SDMA, (0 << 8) | OPERATOR_ARSR, array_len(g_sdma0_arsr0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_sdma0_arsr0_pool_node_config_info}, // ARSR0
	{(2 << 8) | OPERATOR_SDMA, (1 << 8) | OPERATOR_ARSR, array_len(g_sdma0_arsr0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_sdma0_arsr0_pool_node_config_info}, // ARSR1
	{(2 << 8) | OPERATOR_SDMA, (0 << 8) | OPERATOR_VSCF, array_len(g_sdma0_arsr0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_sdma0_arsr0_pool_node_config_info}, // VSCF0
	{(2 << 8) | OPERATOR_SDMA, (1 << 8) | OPERATOR_VSCF, array_len(g_sdma0_arsr0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_sdma0_arsr0_pool_node_config_info}, // VSCF1
	{(2 << 8) | OPERATOR_SDMA, (0 << 8) | OPERATOR_HDR, array_len(g_sdma0_arsr0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_sdma0_arsr0_pool_node_config_info}, // HDR
	{(2 << 8) | OPERATOR_SDMA, (0 << 8) | OPERATOR_UVUP, array_len(g_sdma0_arsr0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_sdma0_arsr0_pool_node_config_info}, // UVUP 32

	// OV0->
	{(0 << 8) | OPERATOR_OV, (0 << 8) | OPERATOR_ARSR, array_len(g_ov0_arsr0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_ov0_arsr0_pool_node_config_info}, // ARSR0
	{(0 << 8) | OPERATOR_OV, (1 << 8) | OPERATOR_ARSR, array_len(g_ov0_arsr0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_ov0_arsr0_pool_node_config_info}, // ARSR1
	{(0 << 8) | OPERATOR_OV, (0 << 8) | OPERATOR_UVUP, array_len(g_ov0_arsr0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_ov0_arsr0_pool_node_config_info}, // UVUP 35

	// OV1->
	{(1 << 8) | OPERATOR_OV, (0 << 8) | OPERATOR_ARSR, array_len(g_ov0_arsr0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_ov0_arsr0_pool_node_config_info}, // ARSR0
	{(1 << 8) | OPERATOR_OV, (1 << 8) | OPERATOR_ARSR, array_len(g_ov0_arsr0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_ov0_arsr0_pool_node_config_info}, // ARSR1
	{(1 << 8) | OPERATOR_OV, (0 << 8) | OPERATOR_UVUP, array_len(g_ov0_arsr0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_ov0_arsr0_pool_node_config_info}, // UVUP
	{(1 << 8) | OPERATOR_OV, (1 << 8) | OPERATOR_WCH, array_len(g_ov1_wch1_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_ov1_wch1_pool_node_config_info}, // WCH1
	{(1 << 8) | OPERATOR_OV, (2 << 8) | OPERATOR_WCH, array_len(g_ov1_wch2_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_ov1_wch2_pool_node_config_info}, // WCH2 40

	// OV2->
	{(2 << 8) | OPERATOR_OV, (0 << 8) | OPERATOR_ARSR, array_len(g_ov0_arsr0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_ov0_arsr0_pool_node_config_info}, // ARSR0
	{(2 << 8) | OPERATOR_OV, (1 << 8) | OPERATOR_ARSR, array_len(g_ov0_arsr0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_ov0_arsr0_pool_node_config_info}, // ARSR1
	{(2 << 8) | OPERATOR_OV, (0 << 8) | OPERATOR_UVUP, array_len(g_ov0_arsr0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_ov0_arsr0_pool_node_config_info}, // UVUP
	{(2 << 8) | OPERATOR_OV, (1 << 8) | OPERATOR_WCH, array_len(g_ov1_wch1_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_ov1_wch1_pool_node_config_info}, // WCH1
	{(2 << 8) | OPERATOR_OV, (2 << 8) | OPERATOR_WCH, array_len(g_ov1_wch2_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_ov1_wch2_pool_node_config_info}, // WCH2 45

	// SROT1->
	{(1 << 8) | OPERATOR_SROT, (1 << 8) | OPERATOR_ARSR, array_len(g_srot0_arsr1_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_srot0_arsr1_pool_node_config_info}, // ARSR1
	{(1 << 8) | OPERATOR_SROT, (1 << 8) | OPERATOR_VSCF, array_len(g_srot0_arsr1_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_srot0_arsr1_pool_node_config_info}, // VSCF1 47

	// HDR0->
	{(0 << 8) | OPERATOR_HDR, (0 << 8) | OPERATOR_OV, array_len(g_hdr_ov0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_hdr_ov0_pool_node_config_info}, // OV0
	{(0 << 8) | OPERATOR_HDR, (1 << 8) | OPERATOR_OV, array_len(g_hdr_ov0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_hdr_ov0_pool_node_config_info}, // OV1
	{(0 << 8) | OPERATOR_HDR, (2 << 8) | OPERATOR_OV, array_len(g_hdr_ov0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_hdr_ov0_pool_node_config_info}, // OV2
	{(0 << 8) | OPERATOR_HDR, (1 << 8) | OPERATOR_WCH, array_len(g_ov1_wch1_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_ov1_wch1_pool_node_config_info}, // WCH1 51

	// UVUP->
	{(0 << 8) | OPERATOR_UVUP, (0 << 8) | OPERATOR_OV, array_len(g_hdr_ov0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_hdr_ov0_pool_node_config_info}, // OV0
	{(0 << 8) | OPERATOR_UVUP, (1 << 8) | OPERATOR_OV, array_len(g_hdr_ov0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_hdr_ov0_pool_node_config_info}, // OV1
	{(0 << 8) | OPERATOR_UVUP, (2 << 8) | OPERATOR_OV, array_len(g_hdr_ov0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_hdr_ov0_pool_node_config_info}, // OV2
	{(0 << 8) | OPERATOR_UVUP, (1 << 8) | OPERATOR_WCH, array_len(g_ov1_wch1_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_ov1_wch1_pool_node_config_info}, // WCH1
	{(0 << 8) | OPERATOR_UVUP, (0 << 8) | OPERATOR_ARSR, array_len(g_hdr_ov0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_hdr_ov0_pool_node_config_info}, // ARSR0
	{(0 << 8) | OPERATOR_UVUP, (1 << 8) | OPERATOR_ARSR, array_len(g_hdr_ov0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_hdr_ov0_pool_node_config_info}, // ARSR1 57

	// ARSR0->
	{(0 << 8) | OPERATOR_ARSR, (0 << 8) | OPERATOR_OV, array_len(g_hdr_ov0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_hdr_ov0_pool_node_config_info}, // OV0
	{(0 << 8) | OPERATOR_ARSR, (1 << 8) | OPERATOR_OV, array_len(g_hdr_ov0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_hdr_ov0_pool_node_config_info}, // OV1
	{(0 << 8) | OPERATOR_ARSR, (2 << 8) | OPERATOR_OV, array_len(g_hdr_ov0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_hdr_ov0_pool_node_config_info}, // OV2
	{(0 << 8) | OPERATOR_ARSR, (0 << 8) | OPERATOR_HDR, array_len(g_hdr_ov0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_hdr_ov0_pool_node_config_info}, // HDR 61

	// ARSR1->
	{(1 << 8) | OPERATOR_ARSR, (0 << 8) | OPERATOR_OV, array_len(g_hdr_ov0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_hdr_ov0_pool_node_config_info}, // OV0
	{(1 << 8) | OPERATOR_ARSR, (1 << 8) | OPERATOR_OV, array_len(g_hdr_ov0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_hdr_ov0_pool_node_config_info}, // OV1
	{(1 << 8) | OPERATOR_ARSR, (2 << 8) | OPERATOR_OV, array_len(g_hdr_ov0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_hdr_ov0_pool_node_config_info}, // OV2
	{(1 << 8) | OPERATOR_ARSR, (0 << 8) | OPERATOR_HDR, array_len(g_hdr_ov0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_hdr_ov0_pool_node_config_info}, // HDR 65

	// VSCF0->
	{(0 << 8) | OPERATOR_VSCF, (0 << 8) | OPERATOR_OV, array_len(g_hdr_ov0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_hdr_ov0_pool_node_config_info}, // OV0
	{(0 << 8) | OPERATOR_VSCF, (1 << 8) | OPERATOR_OV, array_len(g_hdr_ov0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_hdr_ov0_pool_node_config_info}, // OV1
	{(0 << 8) | OPERATOR_VSCF, (2 << 8) | OPERATOR_OV, array_len(g_hdr_ov0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_hdr_ov0_pool_node_config_info}, // OV2
	{(0 << 8) | OPERATOR_VSCF, (0 << 8) | OPERATOR_HDR, array_len(g_hdr_ov0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_hdr_ov0_pool_node_config_info}, // HDR
	{(0 << 8) | OPERATOR_VSCF, (2 << 8) | OPERATOR_WCH, array_len(g_ov1_wch2_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_ov1_wch2_pool_node_config_info}, // WCH2 70

	// VSCF1->
	{(1 << 8) | OPERATOR_VSCF, (0 << 8) | OPERATOR_OV, array_len(g_hdr_ov0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_hdr_ov0_pool_node_config_info}, // OV0
	{(1 << 8) | OPERATOR_VSCF, (1 << 8) | OPERATOR_OV, array_len(g_hdr_ov0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_hdr_ov0_pool_node_config_info}, // OV1
	{(1 << 8) | OPERATOR_VSCF, (2 << 8) | OPERATOR_OV, array_len(g_hdr_ov0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_hdr_ov0_pool_node_config_info}, // OV2
	{(1 << 8) | OPERATOR_VSCF, (0 << 8) | OPERATOR_HDR, array_len(g_hdr_ov0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_hdr_ov0_pool_node_config_info}, // HDR
	{(1 << 8) | OPERATOR_VSCF, (2 << 8) | OPERATOR_WCH, array_len(g_ov1_wch2_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_ov1_wch2_pool_node_config_info}, // WCH2 75

	// CLD0->
	{(0 << 8) | OPERATOR_CLD, (0 << 8) | OPERATOR_OV, array_len(g_cld0_ov0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_cld0_ov0_pool_node_config_info}, // OV0
	{(0 << 8) | OPERATOR_CLD, (1 << 8) | OPERATOR_OV, array_len(g_cld0_ov0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_cld0_ov0_pool_node_config_info}, // OV1
	{(0 << 8) | OPERATOR_CLD, (2 << 8) | OPERATOR_OV, array_len(g_cld0_ov0_pool_node_config_info),
		(struct lbuf_conn_pool_node_config_info *)g_cld0_ov0_pool_node_config_info}, // OV2 78
};

uint32_t *dpu_config_get_scf_lut_addr_tlb(uint32_t *length)
{
	// TODO: According to the different DTS to parse platform

	*length = ARRAY_SIZE(s_scf_lut_addr_tlb_v110);
	return s_scf_lut_addr_tlb_v110;
}

uint32_t *dpu_config_get_arsr_lut_addr_tlb(uint32_t *length)
{
	// TODO: According to the different DTS to parse platform

	*length = (uint32_t)ARRAY_SIZE(g_arsr_offset);
	return g_arsr_offset;
}

uint32_t dpu_config_get_version(void)
{
	return DPU_ACCEL_DPUV700;
}

struct lbuf_pool_node_config_info *dpu_get_lb_pool_node_config_info(void)
{
	return &g_lb_pool_node_config_info;
}

struct lbuf_conn_pool_node_config_info *dpu_get_lb_conn_pool_node_config_info(int32_t cur_opr,
	int32_t nxt_opr, uint32_t *pool_size)
{
	uint32_t index;

	dpu_assert(!pool_size);

	for (index = 0; index < array_len(g_lb_conn_config_info); ++index) {
		if (cur_opr == g_lb_conn_config_info[index].cur_opr && nxt_opr == g_lb_conn_config_info[index].nxt_opr) {
			*pool_size = g_lb_conn_config_info[index].conn_pool_size;
			dpu_pr_debug("cur_opr=%#x nxt_opr=%#x found config: index=%u pool_size=%u",
				cur_opr, nxt_opr, index, *pool_size);
			return g_lb_conn_config_info[index].lb_conn_pool_node_cfg_info;
		}
	}

	dpu_pr_err("cur_opr=%#x nxt_opr=%#x cannot found lbuf_conn_pool_node_config_info", cur_opr, nxt_opr);
	return NULL;
}

uint32_t dpu_get_pool_a_lb_max_size(void)
{
	return 52; // SDMA0~2->OV0~2/CLD0/SROT0
}

uint32_t dpu_get_pool_b_srot_lb_num(void)
{
	return 36; // SROT0->ARSR1/VSCF1 24 + 12
}

uint32_t dpu_get_pool_b_lb_constraint(uint32_t lb_num)
{
	return lb_num <= 12 ? lb_num : 12;
}

uint32_t dpu_is_support_ebit(void)
{
	return 0;
}

static int32_t dbuf_calc_thd(uint32_t last_width, uint32_t yres, uint32_t fps, uint32_t *dbuf_thd)
{
	uint32_t last_unit_per_line = last_width / DPU_DBUF_UNIT;
	uint32_t time_per_line;
	uint32_t req_line;
	uint32_t i;

	if (unlikely(last_unit_per_line == 0 || yres == 0 || fps == 0)) {
		dpu_pr_err("[dbuf]error!, last_unit_per_line=%u, yres=%u, fps=%u", last_unit_per_line, yres, fps);
		return -1;
	}

	/* use panel yres as vtotal to calculate time_per_line */
	time_per_line = PERCENT * HZ_TO_US / fps / yres;
	if (unlikely(time_per_line == 0)) {
		dpu_pr_err("[dbuf]time_per_line is 0");
		return -1;
	}

	/* last opr afford all request_line */
	for (i = 0; i < (uint32_t)DBUF_THD_TYPE_MAX; ++i) {
		req_line = g_dbuf_thd_req[i] * PERCENT / time_per_line;
		dbuf_thd[i] = req_line * last_unit_per_line;
		dpu_pr_debug("[dbuf]dbuf_thd[%d]=%d", i, dbuf_thd[i]);
	}
	return 0;
}

static void dbuf_set_thd_params(uint32_t *dbuf_thd, struct dpu_lbuf_dbuf0_reg *dbuf0_reg)
{
	// scene0
	dbuf0_reg->thd_rqos.reg.dbuf0_thd_rqos_in = dbuf_thd[DBUF_THD_RQOS_IN];
	dbuf0_reg->thd_rqos.reg.dbuf0_thd_rqos_out = dbuf_thd[DBUF_THD_RQOS_OUT];
	dbuf0_reg->thd_flux_req_bef.reg.dbuf0_thd_flux_req_bef_in = dbuf_thd[DBUF_THD_FLUX_REQ_BEF_IN];
	dbuf0_reg->thd_flux_req_bef.reg.dbuf0_thd_flux_req_bef_out = dbuf_thd[DBUF_THD_FLUX_REQ_BEF_OUT];
	dbuf0_reg->thd_flux_req_aft.reg.dbuf0_thd_flux_req_aft_in = dbuf_thd[DBUF_THD_FLUX_REQ_BEF_IN];
	dbuf0_reg->thd_flux_req_aft.reg.dbuf0_thd_flux_req_aft_out = dbuf_thd[DBUF_THD_FLUX_REQ_BEF_OUT];
	dbuf0_reg->thd_dfs_ok.reg.dbuf0_thd_dfs_ok = dbuf_thd[DBUF_THD_FLUX_REQ_BEF_IN];
	dbuf0_reg->ctrl.reg.dbuf0_rqos_dat = 3; /* has confirm with chip */
}

static struct dbuf_config_info *dbuf_set_config_info(struct dpu_lbuf_dbuf0_reg *dbuf0_reg)
{
	// scene 0
	g_dbuf_config_info.addr_offset[0] = DPU_LBUF_DBUF0_CTRL_ADDR(DPU_LBUF_OFFSET);
	g_dbuf_config_info.value[0] = dbuf0_reg->ctrl.value;
	g_dbuf_config_info.addr_offset[1] = DPU_LBUF_DBUF0_THD_RQOS_ADDR(DPU_LBUF_OFFSET);
	g_dbuf_config_info.value[1] = dbuf0_reg->thd_rqos.value;
	g_dbuf_config_info.addr_offset[2] = DPU_LBUF_DBUF0_THD_DFS_OK_ADDR(DPU_LBUF_OFFSET);
	g_dbuf_config_info.value[2] = dbuf0_reg->thd_dfs_ok.value;
	g_dbuf_config_info.addr_offset[3] = DPU_LBUF_DBUF0_THD_FLUX_REQ_BEF_ADDR(DPU_LBUF_OFFSET);
	g_dbuf_config_info.value[3] = dbuf0_reg->thd_flux_req_bef.value;
	g_dbuf_config_info.addr_offset[4] = DPU_LBUF_DBUF0_THD_FLUX_REQ_AFT_ADDR(DPU_LBUF_OFFSET);
	g_dbuf_config_info.value[4] = dbuf0_reg->thd_flux_req_aft.value;
	return &g_dbuf_config_info;
}

/* scene0: OV->DPP0->DSC0->ITF0, OV->DPP0->ITF0 */
struct dbuf_config_info *dbuf_get_config_info(uint32_t xres, uint32_t yres,
	uint32_t dsc_en, uint32_t dsc_out_width, uint32_t fps)
{
	struct dpu_lbuf_dbuf0_reg dbuf0_reg = {0};
	uint32_t dbuf_thd[DBUF_THD_TYPE_MAX] = {0};
	uint32_t last_width = (dsc_en == 0) ? xres : dsc_out_width;

	if (unlikely(dbuf_calc_thd(last_width, yres, fps, dbuf_thd) != 0))
		return NULL;

	dbuf_set_thd_params(dbuf_thd, &dbuf0_reg);
	return dbuf_set_config_info(&dbuf0_reg);
}

