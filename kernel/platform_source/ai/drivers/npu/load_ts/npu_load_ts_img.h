/*
 * npu_load_ts_img.h
 *
 * about load and verify npu tscpu firmware img to reserved addr
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
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
#ifndef _NPU_LOAD_TS_IMG_H_
#define _NPU_LOAD_TS_IMG_H_

void load_npu_reset_status(void);
int load_npu_verify_image(void);

#endif /* _NPU_LOAD_TS_IMG_H_ */

