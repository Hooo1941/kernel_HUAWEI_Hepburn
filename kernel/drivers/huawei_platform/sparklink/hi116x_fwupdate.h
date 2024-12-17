/*
 * hi116x_fwupdate.h
 *
 * interface for huawei hi116x_fwupdate driver
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#ifndef _HI116X_FWUPDATE_H_
#define _HI116X_FWUPDATE_H_
#include "hi116x_driver.h"

/* the caller must be ensure that the ops and di is not null */
int hi1162_fwupdate(struct sparklink_uart_ops *ops, struct hi1162_device_info *di);

#endif /* _HI116X_FWUPDATE_H_ */
