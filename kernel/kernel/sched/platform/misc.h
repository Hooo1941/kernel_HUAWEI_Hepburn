/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 * Description:
 * Miscellaneous little independent features.
 */
#ifndef __SCHED_MISC_H
#define __SCHED_MISC_H

#ifdef CONFIG_GICD_MISS_PPI_WAKE_REQ_WORKAROUND
void check_gic_missed_wakeup(void);
#endif

#endif
