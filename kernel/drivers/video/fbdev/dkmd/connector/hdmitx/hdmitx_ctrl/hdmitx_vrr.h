/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __HDMITX_VRR_H__
#define __HDMITX_VRR_H__

#include <linux/types.h>
#include "hdmitx_ctrl.h"

bool hdmitx_vrr_is_enable(struct hdmitx_ctrl *hdmitx);
void hdmitx_vrr_start(struct hdmitx_ctrl *hdmitx);
void hdmitx_vrr_stop(void);
#endif