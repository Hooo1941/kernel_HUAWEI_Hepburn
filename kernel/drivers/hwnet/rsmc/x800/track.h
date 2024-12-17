/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * Description: This file declares module type in rsmc.
 * Author: linlixin2@huawei.com
 * Create: 2020-10-28
 */

#ifndef TRACK_H
#define TRACK_H

#include "rsmc_rx_ctrl.h"
#include "rsmc_spi_ctrl.h"

struct corr_value {
	s16 er;
	s16 ei;
	s16 pr;
	s16 pi;
	s16 lr;
	s16 li;
};

void init_track(struct acq2track_msg *msg);
s32 track_msg_calc(struct track_msg *msg);
void set_magic_code(struct rx_init_msg *msg);
void refresh_code_nco(void);
void refresh_carr_nco(void);
void refresh_nco(void);

#endif

