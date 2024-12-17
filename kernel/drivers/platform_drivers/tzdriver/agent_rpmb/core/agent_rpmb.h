/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: function declarment for rpmb
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#ifndef AGENT_RPMB_H
#define AGENT_RPMB_H

#ifdef CONFIG_RPMB_AGENT
int rpmb_agent_register(void);
#else
static inline int rpmb_agent_register(void)
{
	return 0;
}
#endif

#endif
