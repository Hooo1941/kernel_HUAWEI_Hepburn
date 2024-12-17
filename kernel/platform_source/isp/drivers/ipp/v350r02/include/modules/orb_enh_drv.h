/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name    orb_enh_drv.h
 * Description:
 *
 * Date         2022-07-01 17:36:21
 ******************************************************************/

#ifndef _ORB_ENH_DRV_CS_H_
#define _ORB_ENH_DRV_CS_H_

#include "cmdlst_common.h"
#include "cfg_table_orb_enh.h"

#define ORB_ENH_DEV_COUNT (1)

struct _orb_enh_ops_t;
typedef struct _orb_enh_dev_t {
	unsigned int           base_addr;
	struct _orb_enh_ops_t  *ops;
	cmd_buf_t              *cmd_buf;
} orb_enh_dev_t;

typedef struct _orb_enh_ops_t {
	int (*prepare_cmd)(orb_enh_dev_t *dev, cmd_buf_t *cmd_buf, cfg_tab_orb_enh_t *table);
} orb_enh_ops_t;

int orb_enh_prepare_cmd(orb_enh_dev_t *dev, cmd_buf_t *cmd_buf, cfg_tab_orb_enh_t *table);
extern orb_enh_dev_t g_orb_enh_devs[ORB_ENH_DEV_COUNT];

#endif /* _ORB_ENH_DRV_CS_H_ */
/* **************************************end*********************************** */
