/*
 * word_chn_res.h
 *
 * Manager of chinese word resoure header.
 *
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd.
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
#ifndef WORD_CHN_RES_H
#define WORD_CHN_RES_H

#define WORD_CHN_DATA_SIZE 1044 /* 261 * 4 bytes */
#define WORD_CHN_LOGO_W 261
#define WORD_CHN_LOGO_H 31

void get_word_chn_data(int line_num, unsigned char *data_buf);

#endif // WORD_CHN_RES_H
