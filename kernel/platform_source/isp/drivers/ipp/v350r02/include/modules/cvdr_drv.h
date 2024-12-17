/******************************************************************
* Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
* file name    cvdr_drv.h
* Description:
*
* date         2022-07-13 11:36:22
*******************************************************************/

#ifndef __CVDR_DRV_CS_H__
#define __CVDR_DRV_CS_H__

#include "ipp.h"
#include "cmdlst_common.h"
#include "cfg_table_cvdr.h"

#define IPP_VP_WR_REG_OFFSET    0x20
#define IPP_VP_RD_REG_OFFSET    0x20
#define CVDR_DEV_COUNT          (1)

struct _cvdr_ops_t;
typedef struct _cvdr_dev_t {
	unsigned int base_addr;
	struct _cvdr_ops_t *ops;
	cmd_buf_t *cmd_buf;
} cvdr_dev_t;

typedef struct _cvdr_ops_t {
	int (*set_debug_enable)(cvdr_dev_t *dev, unsigned char wr_peak_en, unsigned char rd_peak_en);
	int (*get_debug_info)(cvdr_dev_t *dev, unsigned char *wr_peak, unsigned char *rd_peak);
	int (*set_vp_wr_ready)(cvdr_dev_t *dev, unsigned int port, cfg_tab_cvdr_t *table);
	int (*set_vp_rd_ready)(cvdr_dev_t *dev, unsigned int port, cfg_tab_cvdr_t *table);

	int (*do_config)(cvdr_dev_t *dev, cfg_tab_cvdr_t *table);
	int (*prepare_cmd)(cvdr_dev_t *dev, cmd_buf_t *cmd_buf, cfg_tab_cvdr_t *table);
	int (*prepare_vprd_cmd)(cvdr_dev_t *dev, cmd_buf_t *cmd_buf, cfg_tab_cvdr_t *table);
	int (*prepare_vpwr_cmd)(cvdr_dev_t *dev, cmd_buf_t *cmd_buf, cfg_tab_cvdr_t *table);
	int (*prepare_nr_cmd)(cvdr_dev_t *dev, cmd_buf_t *cmd_buf, cfg_tab_cvdr_t *table);
} cvdr_ops_t;

int cvdr_prepare_cmd(cvdr_dev_t *dev, cmd_buf_t *cmd_buf, cfg_tab_cvdr_t *table);
int cvdr_prepare_vprd_cmd(cvdr_dev_t *dev, cmd_buf_t *cmd_buf, cfg_tab_cvdr_t *table);
int cvdr_prepare_vpwr_cmd(cvdr_dev_t *dev, cmd_buf_t *cmd_buf, cfg_tab_cvdr_t *table);
int cvdr_prepare_nr_cmd(cvdr_dev_t *dev, cmd_buf_t *cmd_buf, cfg_tab_cvdr_t *table);

extern cvdr_dev_t g_cvdr_devs[CVDR_DEV_COUNT];

#endif /* __CVDR_DRV_CS_H__ */

/* ********************************* END ********************************* */

