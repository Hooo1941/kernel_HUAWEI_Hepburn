/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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

#ifndef RESET_DEBUG_H
#define RESET_DEBUG_H

#ifdef __cplusplus /* __cplusplus */
extern "C" {
#endif /* __cplusplus */

// 0 for old version
#define DUMP_PARSE_SCRIPT_VERSION   1

#define RESET_ACORE_DUMP_SIZE       1024        // 单独复位acore dump空间总大小申请，2KB
#define RESET_CCORE_DUMP_SIZE       6144        // 单独复位ccore dump空间总大小申请，6KB
#define RESET_MCORE_DUMP_SIZE       256        // 单独复位mcore dump空间总大小申请，1KB

#define MAGIC_ACORE_DUMP_HEAD       0xaaaaaaaa
#define MAGIC_CCORE_DUMP_HEAD       0xcccccccc
#define MAGIC_MCORE_DUMP_HEAD       0xdddddddd

#define MAGIC_ACORE_DUMP_RECORD     0x55

// C核dump 数据格式魔术字定义
#define MAGIC_32_FRAME_DUMP_TYPE        0xabababab
#define MAGIC_16_FRAME_GENERAL_THREAD   0xabab
#define MAGIC_16_FRAME_PRIMARY_THREAD   0xabab
#define MAGIC_32_HOST_DUMP_TYPE         0xcdcdcdcd
#define MAGIC_16_HOST_RECORD_START      0xcdcd
#define MAGIC_16_HOST_RECORD            0xcccc
#define MAGIC_16_HOST_RECORD_END        0xdcdc

#define CP_HOST_IS_REGISTERED  1

typedef enum {
    RECORD_NORMAL = 1,
    RECORD_ABNORMAL
} record_type_e;

/* cpreset 主流程关键步骤打点 */
typedef enum  {
    RESET_DUMP_PREPARE,
    RESET_DUMP_POWER_OFF,
    RESET_DUMP_CB_BEFORE,
    RESET_DUMP_MSG_TO_HIFI,
    RESET_DUMP_CCOER_IN_IDLE,
    RESET_DUMP_FIPC_BEFORE,

    RESET_DUMP_MLOADER_LOAD_RESET,
    RESET_DUMP_MDMPOWER_DOWN,
    RESET_DUMP_POWER_ON_START,
    RESET_DUMP_FIPC_RESETTING,
    RESET_DUMP_RFILE_RESETTING,

    RESET_DUMP_SEND_MSG2_M3_FAIL_RESTING,
    RESET_DUMP_RECV_WRONG_MSG_FROM_M3_RESTING,
    RESET_DUMP_LOAD_MODEM_IMG,
    RESET_DUMP_CB_INVOKE_RESTING,
    RESET_DUMP_WAIT_CCORE_RELAY_TIMEOUT,
    RESET_DUMP_POWER_ON_RESETTING,

    RESET_DUMP_CB_INVOKE_AFTER,
    RESET_DUMP_SEND_MSG2_M3_FAIL_AFTER,
    RESET_DUMP_RECV_WRONG_MSG_FROM_M3_AFTER,
    RESET_DUMP_MAX
} reset_stage_e;

typedef struct {
    u32 magic;
    u32 dump_size;
    u32 version;
    u32 record_num;
}acore_file_head_s;

typedef struct {
    u32 magic;
    u32 dump_size;
    u32 host_num;
    u32 version;
}ccore_file_head_s;

typedef struct {
    u32 magic;
    u32 dump_size;
    u32 version;
    u32 record_num;
}mcore_file_head_s;

// 每类数据类型头, data_len 包含头大小
typedef struct {
    u32 data_type;
    u32 data_len;
}reset_data_head_s;

// C核框架打点内容结构体
#define MAX_FRAME_RECORD_NAME_LEN   12      // 框架打点明文最大长度，包含结束符
typedef struct {
    union {
        u32 value;
        struct {
            u32 flag : 8;       // fail or succ branch
            u32 thread_id : 8;
            u32 magic_num : 16;
        }bit_data;
    } record_info;
    char name[MAX_FRAME_RECORD_NAME_LEN];
    u32 timestamp;
}reset_frame_record_s;

#define MAX_HOST_NAME_LEN   10      // c核组件注册名最大长度, 包含结束符
typedef struct {
    char name[MAX_HOST_NAME_LEN];
    u32 prior;
    u32 record_data_len;
    u32 is_registered;
}reset_host_data_head_s;

// 组件回调打点内容结构体
typedef struct {
    u32 record_info;
    u32 timestamp;
    int val;
}reset_host_record_s;

#define GET_ACORE_RECORD(type, stage)   (((int)(MAGIC_ACORE_DUMP_RECORD & 0xff) << 24) \
    | (((int)(type) & 0xff) << 16) | ((int)(stage) & 0xffff))

typedef struct {
    union {
        int val;
        struct {
            int stage : 16;      // 绑定打点名字的枚举 reset_stage_e, 用于打点解析
            int type : 8;       // 确定打点类型枚举 record_type_e
            int magic : 8;      // 用于判断打点是否成功的magic
        } bit_data;
    } record_info;

    int time_stamp;
}acore_dump_record_s;

typedef struct {
    uintptr_t base_addr;
    acore_file_head_s *file_head;
    acore_dump_record_s *dump_addr;
    u32 record_num_supported;
    u32 main_stage;
    u32 dump_status;
}acore_dump_cfg_s;

typedef struct {
    uintptr_t base_addr;
    ccore_file_head_s *file_head;
}ccore_dump_cfg_s;

typedef struct {
    uintptr_t base_addr;
    mcore_file_head_s *file_head;
}mcore_dump_cfg_s;

typedef struct {
    acore_dump_cfg_s acore_dump_info;
    ccore_dump_cfg_s ccore_dump_info;
    mcore_dump_cfg_s mcore_dump_info;
}reset_dump_ctrl_s;

void reset_cp_dump_parse(void);
void reset_dump_clear_ccore_mem(void);
u32 get_updated_main_stage(void);
void reinit_main_stage(void);
int reset_dump_ctrl_init(void);
void reset_timestamp_dump(reset_stage_e stage, record_type_e type);
void reset_reinit_acore_dump(void);

#ifdef __cplusplus /* __cplusplus */
}
#endif /* __cplusplus */

#endif /* RESET_DEBUG_H */
