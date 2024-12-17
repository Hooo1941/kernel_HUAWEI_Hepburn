/*
 * word_eng_res.h
 *
 * Manager of english word resoure header.
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
#ifndef WORD_ENG_RES_H
#define WORD_ENG_RES_H

#define WORD_ENG_DATA_SIZE 1624 /* 406 * 4 bytes */
#define WORD_ENG_LOGO_W 406
#define WORD_ENG_LOGO_H 33

void get_word_eng_data(int line_num, unsigned char *data_buf);

#endif // WORD_ENG_RES_H
