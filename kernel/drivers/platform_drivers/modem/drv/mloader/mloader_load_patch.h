/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
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
#ifndef MLOADER_LOAD_PATCH_H
#define MLOADER_LOAD_PATCH_H

#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <bsp_mloader.h>
#include "mloader_load_image.h"
#include <bsp_modem_patch.h>
#include "osl_sem.h"

#define NVME_COLD_PATCH_ID 260
#define MODEM_UPDATE_FAIL_COUNT_MAX 3
#define MODEM_PATCH_LIST_NOTE_MAX 32
#define MLOADER_COLD_PATCH_PATH "/patch_hw/hot_modem_fw"
#define MODEM_PATCH_SEM_TIMEOUT 500 // 毫秒

#define MLOADER_MSG_GET_PATCH_STATUS (1)
#define MLOADER_MSG_SET_PATCH_STATUS (2)
#define MLOADER_MSG_SET_PATCH_TYPE (3)

#define MODEM_PATCH_DUMP_SIZE (1280)
#define MODEM_PATCH_DUMP_START 0xABABABAB
#define MODEM_PATCH_DUMP_END 0xBABABABA


#pragma pack(push, 1)
struct modem_patch_cfg {
    u8 status; // 补丁是否已经打上，只要任意一个补丁镜像加载、拼接成功，该成员就会被置1;
               // modem在异常复位时，如果该成员被置为1，更新成员fail_count
    u8 update_count; // 冷补丁更新次数
    u8 fail_count;   // modem升级失败次数，当该成员超过3次后，再次启动时就不会加载补丁镜像
    u8 nv_rollback; // 如果冷补丁存在，modem复位达到3次后，需要更新此标记，进行nv冷补丁回退
};
#pragma pack(pop)

struct modem_patch {
    struct list_head boot_info_list;
    struct modem_patch_cfg cfg;
    char *dump_addr;
    int dump_offset;
    int is_inited;
    osl_sem_id boot_info_sem;
};

struct modem_patch_boot_info {
    struct list_head list;
    char file_name[MLOADER_FILE_NAME_LEN];
    struct modem_patch_info patch_info;
};

int mloader_modem_patch_init(struct platform_device *pdev);
int mloader_patch_msg_process(const char *img_name, u32 type_cmd, u32 patch_data);
int mloader_save_patch_config_info(void);
int mloader_load_patch(const char *file_name, u32 ddr_offset, u32 *patch_size);

#endif
