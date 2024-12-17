/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name  : compare_drv.h
 * Description:
 *
 * Date         2022-07-04 16:28:10
 ********************************************************************/

#ifndef __COMPARE_DRV_CS_H_
#define __COMPARE_DRV_CS_H_

#include "cmdlst_common.h"
#include "cfg_table_compare.h"

#define COMPARE_DEV_COUNT (1)

struct _compare_ops_t;

typedef struct _compare_dev_t {
	unsigned int           base_addr;
	struct _compare_ops_t  *ops;
	cmd_buf_t              *cmd_buf;
} compare_dev_t;

typedef struct _compare_ops_t {
	int (*prepare_cmd)(compare_dev_t *dev, cmd_buf_t *cmd_buf, cfg_tab_compare_t *table);
} compare_ops_t;

int compare_prepare_cmd(compare_dev_t *dev, cmd_buf_t *cmd_buf, cfg_tab_compare_t *table);
extern compare_dev_t g_compare_devs[COMPARE_DEV_COUNT];

#endif /* __COMPARE_DRV_CS_H_ */
/* ******************** END ********** */
