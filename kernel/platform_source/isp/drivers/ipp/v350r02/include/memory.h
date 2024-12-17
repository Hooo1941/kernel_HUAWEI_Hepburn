/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name  :
 * Description:
 *
 * Date         2020-04-17 16:28:10
 ********************************************************************/

#ifndef _MEMORY_CS_H_INCLUDED
#define _MEMORY_CS_H_INCLUDED

#define MEM_HISPCPE_SW_SIZE     0x32000    /* 200K FOR SW */

typedef enum { // The enh and arf share a set of cmdlst buff/cfg_tbl.
	MEM_ID_CMDLST_BUF_ENH_ARF = 0,
	MEM_ID_CMDLST_BUF_REORDER,
	MEM_ID_CMDLST_BUF_COMPARE,
	MEM_ID_CMDLST_BUF_MC,
	MEM_ID_CMDLST_BUF_KLT,

	MEM_ID_ENH_ARF_CFG_TAB, // 5
	MEM_ID_CMDLST_ENTRY_ENH_ARF,
	MEM_ID_CMDLST_PARA_ENH_ARF,

	MEM_ID_REORDER_CFG_TAB, // 8
	MEM_ID_CMDLST_ENTRY_REORDER,
	MEM_ID_CMDLST_PARA_REORDER,

	MEM_ID_COMPARE_CFG_TAB, // 11
	MEM_ID_CMDLST_ENTRY_COMPARE,
	MEM_ID_CMDLST_PARA_COMPARE,

	MEM_ID_MC_CFG_TAB, // 14
	MEM_ID_CMDLST_ENTRY_MC,
	MEM_ID_CMDLST_PARA_MC,

	MEM_ID_KLT_CFG_TAB, // 17
	MEM_ID_CMDLST_ENTRY_KLT,
	MEM_ID_CMDLST_PARA_KLT,

	MEM_ID_MAX
} cpe_mem_id_e;

struct cpe_va_da {
	unsigned long long va;
	unsigned int       da;
};

extern void *get_cpe_addr_va(unsigned int work_module);
extern void *get_cpe_addr_sw_va(unsigned int work_module);
extern unsigned int get_cpe_addr_da(unsigned int work_module);
extern unsigned long get_cpe_iova_size(unsigned int work_module);
int cpe_mem_get(cpe_mem_id_e mem_id, unsigned int work_module, unsigned long long *va, unsigned int *da);
void cpe_mem_free(cpe_mem_id_e mem_id, unsigned int work_module);
int cpe_init_memory(unsigned int work_module);
unsigned int calculate_mem_size_info(unsigned int work_module);
int hispipp_cfg_check(unsigned long args);
#ifdef CONFIG_IPP_DEBUG
void cpe_set_memory(unsigned int work_module);
#endif

#endif /* _MEMORY_CS_H_INCLUDED */
