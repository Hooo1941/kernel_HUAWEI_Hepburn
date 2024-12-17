/*
 * dsp_load_ca.h
 *
 * head of dsp_load_ca.c
 *
 * Copyright (c) 2022-2023 Huawei Technologies Co., Ltd.
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

#ifndef __DSP_LOAD_CA_H__
#define __DSP_LOAD_CA_H__

#define LOAD_DSP_OK                              0
#define LOAD_DSP_ERROR                           1
#define DSP_IMG_MAX_SIZE                         0xA00000 /* 10M */
#define DSP_CA_SEND_MAX_SIZE                     0x100000 /* 1M */

int dsp_read_image(void);
int dsp_load_and_verify_image(void);

#endif /* __DSP_LOAD_CA_H__ */

