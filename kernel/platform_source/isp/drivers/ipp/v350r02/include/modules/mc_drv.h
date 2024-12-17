/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name  : mc_drv.h
 * Description:
 *
 * Date         2022-07-04 16:28:10
 ********************************************************************/

#ifndef _MC_DRV_CS_H_
#define _MC_DRV_CS_H_

#include "cmdlst_common.h"
#include "cfg_table_mc.h"

#define MC_DEV_COUNT (1)

struct _mc_ops_t;
typedef struct _mc_dev_t {
	unsigned int       base_addr;
	struct  _mc_ops_t *ops;
	cmd_buf_t         *cmd_buf;
} mc_dev_t;

typedef struct _mc_ops_t {
	int (*prepare_cmd)(mc_dev_t *dev, cmd_buf_t *cmd_buf, cfg_tab_mc_t *table);
} mc_ops_t;

int mc_prepare_cmd(mc_dev_t *dev, cmd_buf_t *cmd_buf, cfg_tab_mc_t *table);
int mc_enable_cmd(mc_dev_t *dev, cmd_buf_t *cmd_buf, cfg_tab_mc_t *table);
extern mc_dev_t g_mc_devs[MC_DEV_COUNT];

#endif /* _MC_DRV_CS_H_ */
/* ************************************** END ************** */
