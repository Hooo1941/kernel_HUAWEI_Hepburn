/* *****************************************************************
 *   Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 *   File name    cmdlst_drv.h
 *   Description:
 *
 *   Date         2020-04-17 16:28:35
 *******************************************************************/
#ifndef __CMDLST_DRV_CS_H_INCLUDED__
#define __CMDLST_DRV_CS_H_INCLUDED__

#include "ipp.h"
#include "cmdlst_common.h"
#include "cfg_table_cmdlst.h"


#define CMDLST_OFFSET_ADDR    (0xFFF) // 4k : Data from memory_map.
#define CMD_RD_BURST_LENGTH   (16)
#define CMD_BITS_PER_BYTE         (8)

#define macro_cmd_rd_frame_size_dividor(line_size, bits) \
		((line_size) * (bits) / CMD_BITS_PER_BYTE)
#define macro_cmd_rd_frame_size(data_size, line_size, bits) \
		((data_size) / (macro_cmd_rd_frame_size_dividor(line_size, bits)))
// -------------------------------------------------------------------------------------
#define CMDLST_DEV_COUNT (1)

struct _cmdlst_ops_t;
typedef struct _cmdlst_dev_t {
	unsigned int base_addr;
	struct _cmdlst_ops_t *ops;
	cmd_buf_t *cmd_buf;
} cmdlst_dev_t;

typedef struct _cmdlst_ops_t {
	int (*do_config)(cmdlst_dev_t *dev, cfg_tab_cmdlst_t *table);
	int (*set_sw_ch)(cmdlst_dev_t *dev, cmdlst_ch_cfg_t *cfg, unsigned int channel_id);
	int (*set_sw_resource)(cmdlst_dev_t *dev, cfg_tab_cmdlst_t *cfg);
	int (*set_vp_rd)(cmdlst_dev_t *dev, cfg_tab_cmdlst_t *cfg);
	int (*get_debug_info)(cmdlst_dev_t *dev, cmdlst_debug_info_t *st);
} cmdlst_ops_t;

int cmdlst_do_config(cmdlst_dev_t *dev, cfg_tab_cmdlst_t *config_table);
int cmdlst_set_vp_wr_flush(cmdlst_dev_t *dev, cmd_buf_t *cmd_buf, unsigned int channel_id);

extern  cmdlst_dev_t g_cmdlst_devs[CMDLST_DEV_COUNT];

#endif /* __CMDLST_DRV_CS_H_INCLUDED__ */
