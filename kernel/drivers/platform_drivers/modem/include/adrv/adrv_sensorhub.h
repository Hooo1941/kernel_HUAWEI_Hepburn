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

#ifndef ADRV_SENSORHUB_H
#define ADRV_SENSORHUB_H

#define SUBCMD_LEN 4

enum sensorhub_bastet_tag {
    TAG_BASTET = 0xA0,
};

/* tag bastet for heartbeat proxy */
enum sensorhub_bastet_cmd {
    CMD_BST_AP_TO_SENSOR = 0x65,
    CMD_BST_SENSOR_TO_AP = 0x66,
    CMD_BST_CP_TO_SENSOR = 0x67,
    CMD_BST_SENSOR_TO_CP = 0x68,
    CMD_BST_WF_TO_SENSOR = 0x69,
    CMD_BST_SENSOR_TO_WF = 0x6a,
};

enum sensorhub_bastet_sub_cmd {
    SUB_CMD_BST_DSPP_MSG = 0x20, /* dspp protocol config msg for AP-Sensor */
	SUB_CMD_BST_DATA_PKT,        /* heartbeat socket IP packets */
	SUB_CMD_BST_CTRL_MSG,        /* config misg for CP-Sensor and Wifi-Sensor */
};

struct pkt_header {
    unsigned char tag;
    unsigned char cmd;
    unsigned char resp:1;
    unsigned char hw_trans_mode:2; /* 0:IPC 1:SHMEM 2:64bitsIPC */
    unsigned char rsv:5; /* 5 bits */
    unsigned char partial_order;
    unsigned char tranid;
    unsigned char app_tag;
    unsigned short length;
};

struct pkt_subcmd_req_t {
	struct pkt_header hd;
	uint32_t subcmd;
};

struct write_info {
    int tag;
    int cmd;
    const void *wr_buf; /* maybe NULL */
    int wr_len; /* maybe zero */
};

/**
 * 函数名：write_customize_cmd
 *
 * 功能描述：A核发送数据到Sensorhub
 *
 */
int write_customize_cmd(const struct write_info *wr, struct read_info *rd, bool is_lock);

/**
 * 函数名：register_mcu_send_cmd_notifier
 *
 * 功能描述：注册接收Sensorhub发到A核数据的回调函数
 *
 */
int register_mcu_event_notifier(int tag, int cmd, int (*notify)(const struct pkt_header *head));

/**
 * 函数名：unregister_mcu_event_notifier
 *
 * 功能描述：解注册接收Sensorhub发到A核数据的回调函数
 *
 */
int unregister_mcu_event_notifier(int tag, int cmd, int (*notify)(const struct pkt_header *head));

/**
 * 函数名：is_sensorhub_disabled
 *
 * 功能描述：判断当前sensorhub是否可用
 *
 */
 int is_sensorhub_disabled(void);

#endif
