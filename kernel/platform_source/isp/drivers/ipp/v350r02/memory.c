/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name  :
 * Description:
 *
 * Date         2020-04-17 16:28:10
 ********************************************************************/
#include <linux/uaccess.h>
#include <linux/err.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include "ipp.h"
#include "memory.h"
#include "cmdlst_common.h"
#include "cfg_table_ipp_path.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"

#define MEM_SIZE_ALIGN_4K_BYTES (4096)

static unsigned int       g_mem_used[WORK_MODULE_MAX][MEM_ID_MAX] = {0};
static unsigned long long g_mem_va[WORK_MODULE_MAX][MEM_ID_MAX] = {0};
static unsigned int       g_mem_da[WORK_MODULE_MAX][MEM_ID_MAX] = {0};

unsigned int g_mem_offset[WORK_MODULE_MAX][MEM_ID_MAX] = {
	[ENH_ONLY] = {
		[MEM_ID_CMDLST_BUF_ENH_ARF  ] = align_up(CMDLST_BUFFER_SIZE * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),

		[MEM_ID_ENH_ARF_CFG_TAB     ] = align_up(sizeof(seg_enh_arf_cfg_t), CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_ENTRY_ENH_ARF] = align_up(sizeof(schedule_cmdlst_link_t) * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_PARA_ENH_ARF ] = align_up(sizeof(cmdlst_para_t), CVDR_ALIGN_BYTES),
	},
	[ARFEATURE_ONLY] = {
		[MEM_ID_CMDLST_BUF_ENH_ARF  ] = align_up(CMDLST_BUFFER_SIZE * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),

		[MEM_ID_ENH_ARF_CFG_TAB     ] = align_up(sizeof(seg_enh_arf_cfg_t), CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_ENTRY_ENH_ARF] = align_up(sizeof(schedule_cmdlst_link_t) * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_PARA_ENH_ARF ] = align_up(sizeof(cmdlst_para_t), CVDR_ALIGN_BYTES),
	},
	[MATCHER_ONLY] = {
		[MEM_ID_CMDLST_BUF_REORDER  ] = align_up(CMDLST_BUFFER_SIZE * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_BUF_COMPARE  ] = align_up(CMDLST_BUFFER_SIZE * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),

		[MEM_ID_REORDER_CFG_TAB     ] = align_up(sizeof(seg_matcher_rdr_cfg_t), CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_ENTRY_REORDER] = align_up(sizeof(schedule_cmdlst_link_t) * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_PARA_REORDER ] = align_up(sizeof(cmdlst_para_t), CVDR_ALIGN_BYTES),

		[MEM_ID_COMPARE_CFG_TAB     ] = align_up(sizeof(seg_matcher_cmp_cfg_t), CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_ENTRY_COMPARE] = align_up(sizeof(schedule_cmdlst_link_t) * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_PARA_COMPARE ] = align_up(sizeof(cmdlst_para_t), CVDR_ALIGN_BYTES),
	},
	[MC_ONLY] = {
		[MEM_ID_CMDLST_BUF_MC       ] = align_up(CMDLST_BUFFER_SIZE * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),

		[MEM_ID_MC_CFG_TAB          ] = align_up(sizeof(seg_mc_cfg_t), CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_ENTRY_MC     ] = align_up(sizeof(schedule_cmdlst_link_t) * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_PARA_MC      ] = align_up(sizeof(cmdlst_para_t), CVDR_ALIGN_BYTES),
	},
	[ENH_ARF] = {
		[MEM_ID_CMDLST_BUF_ENH_ARF  ] = align_up(CMDLST_BUFFER_SIZE * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),

		[MEM_ID_ENH_ARF_CFG_TAB     ] = align_up(sizeof(seg_enh_arf_cfg_t), CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_ENTRY_ENH_ARF] = align_up(sizeof(schedule_cmdlst_link_t) * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_PARA_ENH_ARF ] = align_up(sizeof(cmdlst_para_t), CVDR_ALIGN_BYTES),
	},
	[ARF_MATCHER] = {
		[MEM_ID_CMDLST_BUF_ENH_ARF  ] = align_up(CMDLST_BUFFER_SIZE * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_BUF_COMPARE  ] = align_up(CMDLST_BUFFER_SIZE * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_BUF_REORDER  ] = align_up(CMDLST_BUFFER_SIZE * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),

		[MEM_ID_ENH_ARF_CFG_TAB     ] = align_up(sizeof(seg_enh_arf_cfg_t), CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_ENTRY_ENH_ARF] = align_up(sizeof(schedule_cmdlst_link_t) * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_PARA_ENH_ARF ] = align_up(sizeof(cmdlst_para_t), CVDR_ALIGN_BYTES),

		[MEM_ID_COMPARE_CFG_TAB     ] = align_up(sizeof(seg_matcher_cmp_cfg_t), CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_PARA_COMPARE ] = align_up(sizeof(cmdlst_para_t), CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_ENTRY_COMPARE] = align_up(sizeof(schedule_cmdlst_link_t) * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),

		[MEM_ID_REORDER_CFG_TAB     ] = align_up(sizeof(seg_matcher_rdr_cfg_t), CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_ENTRY_REORDER] = align_up(sizeof(schedule_cmdlst_link_t) * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_PARA_REORDER ] = align_up(sizeof(cmdlst_para_t), CVDR_ALIGN_BYTES),
	},
	[ENH_ARF_MATCHER] = {
		[MEM_ID_CMDLST_BUF_ENH_ARF  ] = align_up(CMDLST_BUFFER_SIZE * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_BUF_COMPARE  ] = align_up(CMDLST_BUFFER_SIZE * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_BUF_REORDER  ] = align_up(CMDLST_BUFFER_SIZE * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),

		[MEM_ID_ENH_ARF_CFG_TAB     ] = align_up(sizeof(seg_enh_arf_cfg_t), CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_ENTRY_ENH_ARF] = align_up(sizeof(schedule_cmdlst_link_t) * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_PARA_ENH_ARF ] = align_up(sizeof(cmdlst_para_t), CVDR_ALIGN_BYTES),

		[MEM_ID_COMPARE_CFG_TAB     ] = align_up(sizeof(seg_matcher_cmp_cfg_t), CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_ENTRY_COMPARE] = align_up(sizeof(schedule_cmdlst_link_t) * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_PARA_COMPARE ] = align_up(sizeof(cmdlst_para_t), CVDR_ALIGN_BYTES),

		[MEM_ID_REORDER_CFG_TAB     ] = align_up(sizeof(seg_matcher_rdr_cfg_t), CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_ENTRY_REORDER] = align_up(sizeof(schedule_cmdlst_link_t) * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_PARA_REORDER ] = align_up(sizeof(cmdlst_para_t), CVDR_ALIGN_BYTES),
	},
	[MATCHER_MC] = {
		[MEM_ID_CMDLST_BUF_REORDER  ] = align_up(CMDLST_BUFFER_SIZE * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_BUF_MC       ] = align_up(CMDLST_BUFFER_SIZE * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_BUF_COMPARE  ] = align_up(CMDLST_BUFFER_SIZE * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),

		[MEM_ID_REORDER_CFG_TAB     ] = align_up(sizeof(seg_matcher_rdr_cfg_t), CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_ENTRY_REORDER] = align_up(sizeof(schedule_cmdlst_link_t) * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_PARA_REORDER ] = align_up(sizeof(cmdlst_para_t), CVDR_ALIGN_BYTES),

		[MEM_ID_MC_CFG_TAB          ] = align_up(sizeof(seg_mc_cfg_t), CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_ENTRY_MC     ] = align_up(sizeof(schedule_cmdlst_link_t) * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_PARA_MC      ] = align_up(sizeof(cmdlst_para_t), CVDR_ALIGN_BYTES),

		[MEM_ID_COMPARE_CFG_TAB     ] = align_up(sizeof(seg_matcher_cmp_cfg_t), CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_ENTRY_COMPARE] = align_up(sizeof(schedule_cmdlst_link_t) * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_PARA_COMPARE ] = align_up(sizeof(cmdlst_para_t), CVDR_ALIGN_BYTES),
	},
	[ARF_MATCHER_MC] = {
		[MEM_ID_CMDLST_BUF_ENH_ARF  ] = align_up(CMDLST_BUFFER_SIZE * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_BUF_MC       ] = align_up(CMDLST_BUFFER_SIZE * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_BUF_REORDER  ] = align_up(CMDLST_BUFFER_SIZE * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_BUF_COMPARE  ] = align_up(CMDLST_BUFFER_SIZE * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),

		[MEM_ID_ENH_ARF_CFG_TAB     ] = align_up(sizeof(seg_enh_arf_cfg_t), CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_ENTRY_ENH_ARF] = align_up(sizeof(schedule_cmdlst_link_t) * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_PARA_ENH_ARF ] = align_up(sizeof(cmdlst_para_t), CVDR_ALIGN_BYTES),

		[MEM_ID_MC_CFG_TAB          ] = align_up(sizeof(seg_mc_cfg_t), CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_ENTRY_MC     ] = align_up(sizeof(schedule_cmdlst_link_t) * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_PARA_MC      ] = align_up(sizeof(cmdlst_para_t), CVDR_ALIGN_BYTES),

		[MEM_ID_REORDER_CFG_TAB     ] = align_up(sizeof(seg_matcher_rdr_cfg_t), CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_ENTRY_REORDER] = align_up(sizeof(schedule_cmdlst_link_t) * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_PARA_REORDER ] = align_up(sizeof(cmdlst_para_t), CVDR_ALIGN_BYTES),

		[MEM_ID_COMPARE_CFG_TAB     ] = align_up(sizeof(seg_matcher_cmp_cfg_t), CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_ENTRY_COMPARE] = align_up(sizeof(schedule_cmdlst_link_t) * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_PARA_COMPARE ] = align_up(sizeof(cmdlst_para_t), CVDR_ALIGN_BYTES),
	},
	[ENH_ARF_MATCHER_MC] = {
		[MEM_ID_CMDLST_BUF_ENH_ARF  ] = align_up(CMDLST_BUFFER_SIZE * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_BUF_REORDER  ] = align_up(CMDLST_BUFFER_SIZE * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_BUF_COMPARE  ] = align_up(CMDLST_BUFFER_SIZE * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_BUF_MC       ] = align_up(CMDLST_BUFFER_SIZE * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),

		[MEM_ID_ENH_ARF_CFG_TAB     ] = align_up(sizeof(seg_enh_arf_cfg_t), CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_ENTRY_ENH_ARF] = align_up(sizeof(schedule_cmdlst_link_t) * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_PARA_ENH_ARF ] = align_up(sizeof(cmdlst_para_t), CVDR_ALIGN_BYTES),

		[MEM_ID_REORDER_CFG_TAB     ] = align_up(sizeof(seg_matcher_rdr_cfg_t), CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_ENTRY_REORDER] = align_up(sizeof(schedule_cmdlst_link_t) * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_PARA_REORDER ] = align_up(sizeof(cmdlst_para_t), CVDR_ALIGN_BYTES),

		[MEM_ID_COMPARE_CFG_TAB     ] = align_up(sizeof(seg_matcher_cmp_cfg_t), CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_ENTRY_COMPARE] = align_up(sizeof(schedule_cmdlst_link_t) * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_PARA_COMPARE ] = align_up(sizeof(cmdlst_para_t), CVDR_ALIGN_BYTES),

		[MEM_ID_MC_CFG_TAB          ] = align_up(sizeof(seg_mc_cfg_t), CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_ENTRY_MC     ] = align_up(sizeof(schedule_cmdlst_link_t) * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_PARA_MC      ] = align_up(sizeof(cmdlst_para_t), CVDR_ALIGN_BYTES),
	},
	[KLT_ONLY] = {
		[MEM_ID_CMDLST_BUF_KLT      ] = align_up(CMDLST_BUFFER_SIZE * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),

		[MEM_ID_KLT_CFG_TAB         ] = align_up(sizeof(seg_klt_cfg_t), CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_ENTRY_KLT    ] = align_up(sizeof(schedule_cmdlst_link_t) * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_PARA_KLT     ] = align_up(sizeof(cmdlst_para_t), CVDR_ALIGN_BYTES),
	},
	[ENH_ARF_KLT] = {
		[MEM_ID_CMDLST_BUF_ENH_ARF  ] = align_up(CMDLST_BUFFER_SIZE * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_BUF_KLT      ] = align_up(CMDLST_BUFFER_SIZE * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),

		[MEM_ID_ENH_ARF_CFG_TAB     ] = align_up(sizeof(seg_enh_arf_cfg_t), CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_ENTRY_ENH_ARF] = align_up(sizeof(schedule_cmdlst_link_t) * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_PARA_ENH_ARF ] = align_up(sizeof(cmdlst_para_t), CVDR_ALIGN_BYTES),

		[MEM_ID_KLT_CFG_TAB         ] = align_up(sizeof(seg_klt_cfg_t), CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_ENTRY_KLT    ] = align_up(sizeof(schedule_cmdlst_link_t) * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES),
		[MEM_ID_CMDLST_PARA_KLT     ] = align_up(sizeof(cmdlst_para_t), CVDR_ALIGN_BYTES),
	},
};

static int cpe_mem_init(unsigned long long sw_va, unsigned long long va, unsigned int da, unsigned long size,
	unsigned int work_module)
{
	unsigned int i = 0;
	int is_first = 1;
	int history_i = 0;
	unsigned long total_used_size = 0;

	for (i = 0; i <= MEM_ID_CMDLST_BUF_KLT; i++) {
		if(g_mem_offset[work_module][i] != 0) {
			total_used_size += g_mem_offset[work_module][i];
			g_mem_used[work_module][i] = 0;

			if (is_first == 1) {
				g_mem_va[work_module][i] = va;
				g_mem_da[work_module][i] = da;
				is_first = 0;
			} else {
				g_mem_va[work_module][i] = g_mem_va[work_module][history_i] + g_mem_offset[work_module][history_i];
				g_mem_da[work_module][i] = g_mem_da[work_module][history_i] + g_mem_offset[work_module][history_i];
			}
			history_i = i;
			logd("g_mem_va[%d][%d]= 0x%llx, g_mem_offset[%d][%d]= 0x%x, (next start addr=0x%llx)",
				work_module, i, g_mem_va[work_module][i],
				work_module, i, g_mem_offset[work_module][i],
				g_mem_va[work_module][i] + g_mem_offset[work_module][i]);
		}
	}

	if (total_used_size > size) {
		loge("Failed : total_used_size.0x%lx > size.0x%lx", total_used_size, size);
		return -ENOMEM;
	}

	is_first = 1;
	total_used_size = 0;
	for (i = MEM_ID_ENH_ARF_CFG_TAB; i < MEM_ID_MAX; i++) {
		if(g_mem_offset[work_module][i] != 0) {
			total_used_size += g_mem_offset[work_module][i];
			g_mem_used[work_module][i] = 0;

			if (is_first == 1) {
				g_mem_va[work_module][i] = sw_va;
				g_mem_da[work_module][i] = 0;
				is_first = 0;
			} else {
				g_mem_va[work_module][i] = g_mem_va[work_module][history_i] + g_mem_offset[work_module][history_i];
				g_mem_da[work_module][i] = 0;
			}
			history_i = i;
			logd("g_mem_va[%d][%d]= 0x%llx, g_mem_offset[%d][%d]= 0x%x, (next start addr=0x%llx)",
				work_module, i, g_mem_va[work_module][i],
				work_module, i, g_mem_offset[work_module][i],
				g_mem_va[work_module][i] + g_mem_offset[work_module][i]);
		}
	}

	if (total_used_size > MEM_HISPCPE_SW_SIZE) {
		loge("Failed : total_used_size.0x%lx > size.0x%x", total_used_size, MEM_HISPCPE_SW_SIZE);
		return -ENOMEM;
	}

	return 0;
}

int cpe_init_memory(unsigned int work_module)
{
	unsigned int da;
	unsigned long long va;
	unsigned long long sw_va;
	unsigned long iova_size; // new add r02
	int ret;

	da = get_cpe_addr_da(work_module);
	if (da == 0) {
		loge(" Failed : CPE Device da false");
		return -ENOMEM;
	}

	va = (unsigned long long)(uintptr_t)get_cpe_addr_va(work_module);
	if (va == 0) {
		loge(" Failed : CPE Device va false");
		return -ENOMEM;
	}

	sw_va = (unsigned long long)(uintptr_t)get_cpe_addr_sw_va(work_module);
	if (sw_va == 0) {
		loge(" Failed : CPE Device sw_va false");
		return -ENOMEM;
	}

	iova_size = get_cpe_iova_size(work_module);
	if (iova_size == 0) {
		loge(" Failed : CPE Device iova_size false");
		return -ENOMEM;
	}

	ret = cpe_mem_init(sw_va, va, da, iova_size, work_module);
	if (ret != 0) {
		loge(" Failed : cpe_mem_init .%d", ret);
		return -ENOMEM;
	}

	return 0;
}

int cpe_mem_get(cpe_mem_id_e mem_id, unsigned int work_module, unsigned long long *va, unsigned int *da)
{
	if (va == NULL || da == NULL) {
		loge(" Failed : va.%pK, da.%pK", va, da);
		return -ENOMEM;
	}

	if (mem_id >= MEM_ID_MAX) {
		loge(" Failed : mem_id.(%u >= %u)", mem_id, MEM_ID_MAX);
		return -ENOMEM;
	}

	if (g_mem_used[work_module][mem_id] == 1) {
		loge(" Failed : Unable to alloc, g_mem_used[%u][%u] = 1", work_module, mem_id);
		return -ENOMEM;
	}

	*da = (unsigned int)g_mem_da[work_module][mem_id];
	*va = (unsigned long long)g_mem_va[work_module][mem_id];
	if (*va == 0) {
		loge(" Failed get va mem: work_module = %u, mem_id = %u", work_module, mem_id);
		return -ENOMEM;
	}

	if (mem_id <= MEM_ID_CMDLST_BUF_KLT) {
		if (*da == 0) {
			loge(" Failed get da mem: work_module = %u, mem_id = %u", work_module, mem_id);
			return -ENOMEM;
		}
	}

	g_mem_used[work_module][mem_id] = 1;
	return 0;
}

void cpe_mem_free(cpe_mem_id_e mem_id, unsigned int work_module)
{
	if (mem_id >= MEM_ID_MAX) {
		loge(" Failed : mem_id.(%u >= %u)", mem_id, MEM_ID_MAX);
		return;
	}

	if (g_mem_used[work_module][mem_id] == 0) {
		logi("no need to do mem_free, g_mem_used[%u][%u] = 0", work_module, mem_id);
		return;
	}

	g_mem_used[work_module][mem_id] = 0;
}

/**
@name: calculate_mem_size_info
@description: Calculate the amount of memory that current work_mode need;
**/
unsigned int calculate_mem_size_info(unsigned int work_module)
{
	unsigned int i = 0;
	unsigned int mem_size = 0;

	for (i = 0; i < MEM_ID_MAX; i++)
		if(g_mem_offset[work_module][i] != 0) {
			mem_size += g_mem_offset[work_module][i];
			logd("g_mem_offset[%d][%d]=0x%x", work_module, i, g_mem_offset[work_module][i]);
		}
	mem_size = align_up(mem_size, MEM_SIZE_ALIGN_4K_BYTES);
	return mem_size;
}

#ifdef CONFIG_IPP_DEBUG
void cpe_set_memory(unsigned int work_module)
{
	int i = 0;
	for (i = 0; i <= MEM_ID_CMDLST_BUF_KLT; i++) {
		g_mem_va[work_module][i] = 0;
		g_mem_da[work_module][i] = 0;
	}
}
#endif

#pragma GCC diagnostic pop
// end file

