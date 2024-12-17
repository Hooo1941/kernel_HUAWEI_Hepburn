/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name  : klt_drv.h
 * Description:
 *
 * Date         2022-07-04 16:28:10
 ********************************************************************/

#ifndef __KLT_DRV_CS_H_
#define __KLT_DRV_CS_H_

#include "cmdlst_common.h"
#include "cfg_table_klt.h"

#define KLT_DEV_COUNT (1)

struct _klt_ops_t;
typedef struct _klt_dev_t {
	unsigned int      base_addr;
	struct _klt_ops_t *ops;
	cmd_buf_t         *cmd_buf;
} klt_dev_t;

typedef struct _klt_ops_t {
	int (*prepare_klt_ds_cmd)(klt_dev_t *dev, cmd_buf_t *cmd_buf, klt_ds_cfg_t *table);
	int (*prepare_klt_klt_cmd)(klt_dev_t *dev, cmd_buf_t *cmd_buf, cfg_tab_klt_t *table);
	int (*klt_start)(klt_dev_t *dev, cmd_buf_t *cmd_buf, klt_start_cfg_t *start_cfg);
} klt_ops_t;

int klt_prepare_ds_cmd(klt_dev_t *dev, cmd_buf_t *cmd_buf, klt_ds_cfg_t *table);
int klt_prepare_klt_cmd(klt_dev_t *dev, cmd_buf_t *cmd_buf, cfg_tab_klt_t *table);
int klt_klt_start_cmd(klt_dev_t *dev, cmd_buf_t *cmd_buf, klt_start_cfg_t *start_cfg);

extern klt_dev_t g_klt_devs[KLT_DEV_COUNT];

#endif /* __KLT_DRV_CS_H_ */

