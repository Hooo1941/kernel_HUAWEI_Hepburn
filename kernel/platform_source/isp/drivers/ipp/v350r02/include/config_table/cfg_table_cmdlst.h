/* *****************************************************************
 *   Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 *   File name    cfg_table_cmdlst.h
 *   Description:
 *
 *   Date         2022-07-06 16:28:35
 *******************************************************************/

#ifndef __CONFIG_TABLE_CMDLST_H_INCLUDED__
#define __CONFIG_TABLE_CMDLST_H_INCLUDED__

#include "cmdlst_drv_priv.h"

typedef union cmdlst_sw_resources        cmdlst_sw_resource_cfg_t;
typedef union cmdlst_sw_task             cmdlst_sw_task_cfg_t;
typedef union cmdlst_token_cfg           cmdlst_token_cfg_t;
typedef union cmdlst_cmd_cfg             cmdlst_cmd_cfg_t;
typedef union cmdlst_sw_cvdr_rd_addr     cmdlst_sw_cvdr_rd_addr_t;
typedef union cmdlst_sw_cvdr_rd_data_0   cmdlst_sw_cvdr_rd_data0_t;
typedef union cmdlst_sw_cvdr_rd_data_1   cmdlst_sw_cvdr_rd_data1_t;
typedef union cmdlst_sw_cvdr_rd_data_2   cmdlst_sw_cvdr_rd_data2_t;
typedef union cmdlst_sw_cvdr_rd_data_3   cmdlst_sw_cvdr_rd_data3_t;
typedef union cmdlst_ch_cfg              cmdlst_ch_cfg_t;
typedef union cmdlst_start_addr          cmdlst_start_addr_cfg_t;
typedef union cmdlst_end_addr            cmdlst_end_addr_cfg_t;
typedef union cmdlst_cfg                 cmdlst_burst_cfg_t;


typedef struct _cmdlst_debug_info_t { // for debug
	unsigned int channel_id;

	unsigned int fsm_info;
	unsigned int last_exec;
} cmdlst_debug_info_t;


typedef struct _cfg_tab_cmdlst_t {
	unsigned int                 channel_id;
	cmdlst_sw_resource_cfg_t     sw_resource_cfg;
	cmdlst_sw_task_cfg_t         sw_task_cfg;
	cmdlst_sw_cvdr_rd_addr_t     sw_cvdr_rd_addr;
	cmdlst_sw_cvdr_rd_data0_t    sw_cvdr_rd_data0;
	cmdlst_sw_cvdr_rd_data1_t    sw_cvdr_rd_data1;
	cmdlst_sw_cvdr_rd_data2_t    sw_cvdr_rd_data2;
	cmdlst_sw_cvdr_rd_data3_t    sw_cvdr_rd_data3;
} cfg_tab_cmdlst_t;

#endif /* __CONFIG_TABLE_CMDLST_H_INCLUDED__ */
/********************************** END **********************************/

