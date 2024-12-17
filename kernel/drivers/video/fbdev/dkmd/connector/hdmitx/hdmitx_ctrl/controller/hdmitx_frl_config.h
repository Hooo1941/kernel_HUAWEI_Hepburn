/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
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

#ifndef __HDMITX_FRL_CONFIG_H__
#define __HDMITX_FRL_CONFIG_H__

#include <linux/types.h>

enum frl_data_source {
	FRL_GAP_PACKET,
	FRL_VIDEO,
	FRL_TRAINING_PATTERN
};

struct frl_reg {
	u32 id;
	void *frl_base;
	void *frl_training_base;
};

void hal_frl_init(const struct frl_reg *frl);
void hal_frl_config_16to18_table(const struct frl_reg *frl);
void hal_frl_set_training_pattern(const struct frl_reg *frl, u8 lnx, u8 pattern);
void hal_frl_set_training_ffe(const struct frl_reg *frl, u8 lnx, u8 ffe);
u8 hal_frl_get_training_ffe(const struct frl_reg *frl, u8 lnx);
void hal_frl_set_training_rate(const struct frl_reg *frl, u8 rate);
u8 hal_frl_get_training_rate(const struct frl_reg *frl);
void hal_frl_set_source_data(const struct frl_reg *frl, enum frl_data_source source);
enum frl_data_source hal_frl_get_source_data(const struct frl_reg *frl);
void hal_frl_set_pfifo_threshold(const struct frl_reg *frl, u32 up, u32 down);

#endif /* __HAL_HDMITX_FRL_H__ */

