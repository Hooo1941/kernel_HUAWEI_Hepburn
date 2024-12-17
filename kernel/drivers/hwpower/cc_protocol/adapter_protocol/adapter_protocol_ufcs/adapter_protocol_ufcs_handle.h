/* SPDX-License-Identifier: GPL-2.0 */
/*
 * adapter_protocol_ufcs_handle.h
 *
 * ufcs protocol irq event handle
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#ifndef _ADAPTER_PROTOCOL_UFCS_HANDLE_H_
#define _ADAPTER_PROTOCOL_UFCS_HANDLE_H_

#include <chipset_common/hwpower/common_module/power_event_ne.h>

void hwufcs_handle_set_test_mode(bool flag);
void hwufcs_handle_msg(void);
#endif /* _ADAPTER_PROTOCOL_UFCS_HANDLE_H_ */
