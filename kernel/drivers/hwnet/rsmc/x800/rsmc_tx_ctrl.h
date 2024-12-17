/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * Description: This file declares module type in rsmc.
 * Author: zhongjilei@huawei.com
 * Create: 2020-10-28
 */

#ifndef RSMC_TX_CTRL_H
#define RSMC_TX_CTRL_H

#include "rsmc_msg_loop.h"
#include "rsmc_spi_ctrl.h"
#include "rsmc_x800_device.h"

int send_data(struct tx_data_msg *msg);
void init_tx_data(struct tx_data_msg *msg, bool only_tx);
void init_tx(void);
int start_tx(void);
void tx_sm_set(void);
void tx_update_buf(u32 value);
void save_sign_buff(void);
void tx_rsp_ok(void);
void clear_tx_data(void);
bool tx_complete(void);
void rf_tx_prepare(void);
void set_rf(u32 addr, u32 value);
void set_tx_err(void);

#endif

