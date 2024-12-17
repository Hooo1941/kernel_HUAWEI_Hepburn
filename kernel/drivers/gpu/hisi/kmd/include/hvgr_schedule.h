/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */
#ifndef HVGR_SCHEDULE_H
#define HVGR_SCHEDULE_H
#include <linux/types.h>

struct hvgr_ctx;
struct hvgr_device;

#define HVGR_SCH_MODE_JOBSLOT    0
#define HVGR_SCH_MODE_CQ         1

#define HVGR_SCH_SUBMIT_IGNORE   0
#define HVGR_SCH_SUBMIT_ALLOW    1

#define HVGR_SCH_INTRDISABLE     0
#define HVGR_SCH_INTRENABLE      1

enum sch_rst_status {
	HVGR_SCH_SOFT_CLEAR = 0,
	HVGR_SCH_SOFT_STOP,
	HVGR_SCH_SOFT_CLEAR_GPU_TASK,
	HVGR_SCH_DUMP_REGISTER,
};

struct hvgr_sch_backend_ops {
	int (*init)(struct hvgr_device * const gdev);
	void (*term)(struct hvgr_device * const gdev);
	long (*ctx_create)(struct hvgr_ctx * const ctx);
	void (*ctx_destroy)(struct hvgr_ctx * const ctx);
	long (*cmd_dispatch)(struct hvgr_ctx *const ctx, unsigned int cmd, unsigned long arg);
	int (*suspend)(struct hvgr_device * const gdev);
	void (*resume)(struct hvgr_device * const gdev);
	void (*start)(struct hvgr_device * const gdev);
	void (*stop)(struct hvgr_device * const gdev);
	void (*ctx_stop)(struct hvgr_ctx * const ctx);
	void (*reset)(struct hvgr_device * const gdev, enum sch_rst_status reset_stage);
	void (*set_mode)(struct hvgr_device * const gdev, uint32_t mode, uint32_t sch_flag);
	void (*intr_switch)(struct hvgr_device * const gdev, uint32_t flag);
};

union hvgr_sch_sub_dev {
	struct hvgr_cq_dev *cq_dev;
	/* add other struct in here. */
};

struct hvgr_sch_dev {
	union hvgr_sch_sub_dev sch_sub_dev;
	struct hvgr_sch_backend_ops *ops;
};

struct hvgr_sch_backend_ops *hvgr_sched_get_ops(void);

#endif
