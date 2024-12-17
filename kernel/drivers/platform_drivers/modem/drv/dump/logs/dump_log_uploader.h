/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef __DUMP_LOG_UPLOADER_H__
#define __DUMP_LOG_UPLOADER_H__

#include <product_config.h>
#include <osl_sem.h>
#include <osl_types.h>
#include <bsp_dump.h>
#include <dump_frame.h>
#include "dump_log_strategy.h"

#define DUMP_NOT_UPLOAD (-1)
#define DUMP_UPLOAD_NOCRYPTO 0
#define DUMP_UPLOAD_CRYPTO 1

#define DUMP_DATA_PER_BLOCK (64 * 1024)

#define DUMP_UPLOAD_INIT_TIMEOUT_VAL_MS 2000 /* 默认提供2s的超时，为前几包数据提高稳定性 */
#define DUMP_UPLOAD_DEFAULT_DATARATE_PER_MS 1800 /* moses 25s传完 */

#define DUMP_UPLOAD_WAIT_LAST_PKT_DONE_DELTA_MS_TINY 3 /* 一次等待3ms */
#define DUMP_UPLOAD_WAIT_LAST_PKT_DONE_DELTA_MS 50 /* 一次等待50ms */
#define DUMP_UPLOAD_WAIT_LAST_PKT_DONE_TIMEOUT_MS 500 /* 与APP处理上报速率、APP 接收buffer大小有关 */
#define DUMP_UPLOAD_SERVER_CONNECT_TIMEOUT_MS 1500 /* 与网络环境有关 */
struct dump_upload_file_head {
    char filename[RDR_DUMP_FILE_PATH_LEN];
};

typedef struct {
    bool feature_on;
    bool genl_connected;
    bool server_connected;
    u32 server_connect_trigger_time;
    u32 total_data_len;
    u32 send_len;
    u32 processed_len;
    u32 last_send_time_ms;
    u32 last_send_timeout_ms;
    osl_sem_id upload_sem;
    struct dump_upload_file_head pkt_head;
} dump_upload_ctrl_s;

struct dump_kernel_cmd_pkt {
    u32 msg_type;
    u32 data;
};

enum dump_kernel_msg_type {
    DUMP_KERNEL_MSG_TYPE_CMD,
    DUMP_KERNEL_MSG_TYPE_UPLOAD,
    DUMP_KERNEL_MSG_TYPE_BUTT
};

enum dump_kernel_msg_cmd_op {
    DUMP_KERNEL_CMD_OP_CONNECT,
    DUMP_KERNEL_CMD_OP_DISCONNECT,
    DUMP_KERNEL_CMD_OP_REPORT_SEND_DATA,
    DUMP_KERNEL_CMD_OP_BUTT,
};

enum dump_kernel_msg_uplaod_op {
    DUMP_KERNEL_UPLOAD_OP_CRYPTO_BEGIN = 0x0,
    DUMP_KERNEL_UPLOAD_OP_CRYPTO_FILE_NAME = DUMP_KERNEL_UPLOAD_OP_CRYPTO_BEGIN,
    DUMP_KERNEL_UPLOAD_OP_CRYPTO_FILE_DATA,
    DUMP_KERNEL_UPLOAD_OP_CRYPTO_BUTT,

    DUMP_KERNEL_UPLOAD_OP_NOCRYPTO_BEGIN = 0x1000,
    DUMP_KERNEL_UPLOAD_OP_NOCRYPTO_FILE_NAME = DUMP_KERNEL_UPLOAD_OP_NOCRYPTO_BEGIN,
    DUMP_KERNEL_UPLOAD_OP_NOCRYPTO_FILE_DATA,
    DUMP_KERNEL_UPLOAD_OP_NOCRYPTO_BUTT
};

enum dump_app_cmd {
    DUMP_APP_CMD_SET_CONN_STATE,
    DUMP_APP_CMD_SET_SERVER_STATE,
    DUMP_APP_CMD_SET_UPLOAD_LEN,
    DUMP_APP_CMD_BUTT,
};

struct dump_app_cmd_pkt {
    unsigned int cmd;
    unsigned int data;
};

int dump_uploader_init(void);
int dump_uploader_wait_done(void);
int dump_uploader_prepare(void);
void dump_upload_file(dump_save_file_id_e file_id, struct file *file, const char *data, u32 data_len, u32 offsset);
int dump_uploader_reset_proc(void);
#endif