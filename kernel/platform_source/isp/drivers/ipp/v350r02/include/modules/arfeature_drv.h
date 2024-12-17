/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name  : arfeature_drv.h
 * Description:
 *
 * Date         2022-07-01 16:28:10
 ********************************************************************/

#ifndef _ARFEATURE_DRV_CS_H_
#define _ARFEATURE_DRV_CS_H_

#include "cmdlst_common.h"
#include "cfg_table_arfeature.h"

#define ARFEATURE_DEV_COUNT (1)

struct _arfeature_ops_t;
typedef struct _arfeature_dev_t {
	unsigned int            base_addr;
	struct _arfeature_ops_t *ops;
	cmd_buf_t               *cmd_buf;
} arfeature_dev_t;

typedef struct _arfeature_ops_t {
	int (*prepare_cmd)(arfeature_dev_t *dev, cmd_buf_t *cmd_buf, cfg_tab_arf_t *table);
} arfeature_ops_t;

int arfeature_prepare_cmd(arfeature_dev_t *dev, cmd_buf_t *cmd_buf, cfg_tab_arf_t *table);
extern arfeature_dev_t g_arfeature_devs[ARFEATURE_DEV_COUNT];

#endif // _ARFEATURE_DRV_CS_H_

