/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name  : reorder_drv.h
 * Description:
 *
 * Date         2022-07-04 16:28:10
 ********************************************************************/

#ifndef __REORDER_DRV_CS_H_
#define __REORDER_DRV_CS_H_

#include "cmdlst_common.h"
#include "cfg_table_reorder.h"

#define REORDER_DEV_COUNT (1)

struct _reorder_ops_t;
typedef struct _reorder_dev_t {
	unsigned int           base_addr;
	struct _reorder_ops_t  *ops;
	cmd_buf_t              *cmd_buf;
} reorder_dev_t;

typedef struct _reorder_ops_t {
	int (*prepare_cmd)(reorder_dev_t *dev, cmd_buf_t *cmd_buf, cfg_tab_reorder_t *table);
} reorder_ops_t;

int reorder_prepare_cmd(reorder_dev_t *dev, cmd_buf_t *cmd_buf, cfg_tab_reorder_t *table);
extern reorder_dev_t g_reorder_devs[REORDER_DEV_COUNT];

#endif /* __REORDER_DRV_CS_H_ */

