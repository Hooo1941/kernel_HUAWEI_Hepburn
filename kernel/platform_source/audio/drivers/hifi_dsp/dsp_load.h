/*
 * dsp_load.h
 *
 * head of dsp_load.c
 *
 * Copyright (c) 2015-2020 Huawei Technologies Co., Ltd.
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

#ifndef __DSP_LOAD_H__
#define __DSP_LOAD_H__

void dsp_load_init(unsigned char *unsec_virt_addr);
void dsp_load_deinit(void);
#ifdef CONFIG_HIFI_SECOS_LOAD
unsigned long wait_dsp_load_completion_timeout(unsigned long timeout);
#else
static inline unsigned long wait_dsp_load_completion_timeout(unsigned long timeout)
{
	return (timeout != 0) ? timeout : 1;
}
#endif

#endif /* __DSP_LOAD_H__ */

