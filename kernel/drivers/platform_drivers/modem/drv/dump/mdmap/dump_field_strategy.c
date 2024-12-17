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

#include <securec.h>
#include <osl_types.h>
#include <osl_malloc.h>
#include <bsp_dump_mem.h>
#include <bsp_dt.h>
#include <dump_log_strategy.h>
#include <dump_frame.h>
#include <dump_errno.h>
#include <dump_config.h>
#include "dump_field_strategy.h"


#undef THIS_MODU
#define THIS_MODU mod_dump

#define DUMP_FIELD_STRATEGY_ATTR_BITS 32
#define DUMP_FIELD_STRATEGY_ATTR_ALLOC_POLICY 8
#define DUMP_FIELD_STRATEGY_ATTR_MAX_LEN_BITS (DUMP_FIELD_STRATEGY_ATTR_BITS - DUMP_FIELD_STRATEGY_ATTR_ALLOC_POLICY)

typedef union {
    struct {
        u32 max_len : DUMP_FIELD_STRATEGY_ATTR_MAX_LEN_BITS; /* 24bit maxlen */
        u32 alloc_policy : DUMP_FIELD_STRATEGY_ATTR_ALLOC_POLICY; /* 8bit alloc policy */
    };
    u32 val;
} dump_field_strategy_attr_u;

/* 内部存储用的结构体，用dump_field_strategy_attr_u节约内存 */
typedef struct {
    u32 field_id;
    dump_field_strategy_attr_u attr;
} dump_field_strategy_store_s;

#define DUMP_FIELD_STRATEGY_U32S (sizeof(dump_field_strategy_store_s) / sizeof(u32))

typedef struct {
    u32 mode;
    u32 nums;
    u32 inited;
    dump_field_strategy_store_s *strategys; /* dts中升序排列 */
} dump_field_strategy_ctrl_s;

dump_field_strategy_ctrl_s g_dump_field_strategy_ctrl;

/*
 * 1、策略配置可能会有几十个,二分查找会快一些
 * 2、要求dts中的原始数据已排好序
 * 3、内部接口，不对外暴露
 * 4、这里不判断g_dump_field_strategy_ctrl.strategys是否为空，由本文件中的调用者判断
 */
int dump_field_get_strategy_idx(u32 field_id)
{
    int low = 0;
    int high = (int)g_dump_field_strategy_ctrl.nums - 1;
    int mid;
    while (low <= high) {
        mid = low + (high - low) / 2; //不会溢出
        if (g_dump_field_strategy_ctrl.strategys[mid].field_id > field_id) {
            high = mid - 1;
        } else if (g_dump_field_strategy_ctrl.strategys[mid].field_id < field_id) {
            low = mid + 1;
        } else {
            return mid;
        }
    }
    return DUMP_ERRNO_INVA_PARAM;
}

int dump_field_check_policy_core(int idx, u32 alloc_len)
{
    u32 cur_status = 0;
    u32 cfg_policy, max_len;
    if (dump_cfg_match_cur_dump_level(DUMP_LEVEL_FULLDUMP)) {
        cur_status |= DUMP_MEM_ALLOC_FULLDUMP;
    }
    if (dump_is_in_exception()) {
        cur_status |= DUMP_MEM_ALLOC_EXCEPTION;
    }

    max_len = (u32)g_dump_field_strategy_ctrl.strategys[idx].attr.max_len;
    if (alloc_len > max_len) {
        return DUMP_FIELD_STRATEGY_ALLOC_REJECT;
    }

    /* 检查固定特征是否是否匹配 */
    cfg_policy = DUMP_MEMPOLICY_DECODE_FIX_FEATURE((u32)g_dump_field_strategy_ctrl.strategys[idx].attr.alloc_policy);
    if (!DUMP_MATCH_ATTRIBUTE(cfg_policy, cur_status)) {
        return DUMP_FIELD_STRATEGY_ALLOC_REJECT;
    }

    /* 检查动态特征是否是否匹配 */
    cfg_policy = DUMP_MEMPOLICY_DECODE_DYN_FEATURE((u32)g_dump_field_strategy_ctrl.strategys[idx].attr.alloc_policy);
    if (!DUMP_MATCH_ATTRIBUTE(cfg_policy, cur_status)) {
        return DUMP_FIELD_STRATEGY_ALLOC_LATER;
    }
    return DUMP_FIELD_STRATEGY_ALLOC_ALLOW;
}

int dump_field_check_pass_strategy(int idx, u32 alloc_len)
{
    /* 不在白名单里，拒绝申请 */
    if (idx == DUMP_ERRNO_INVA_PARAM) {
        return DUMP_FIELD_STRATEGY_ALLOC_REJECT;
    }
    return dump_field_check_policy_core(idx, alloc_len);
}

int dump_field_check_block_strategy(int idx, u32 alloc_len)
{
    /* 不在受限名单里，允许申请 */
    if (idx == DUMP_ERRNO_INVA_PARAM) {
        return DUMP_FIELD_STRATEGY_ALLOC_ALLOW;
    }
    return dump_field_check_policy_core(idx, alloc_len);
}

int dump_field_check_strategy(u32 field_id, u32 alloc_len)
{
    int idx;
    int ret = DUMP_OK;
    if ((g_dump_field_strategy_ctrl.mode == DUMP_FIELD_STRATEGY_DISABLE) ||
        (g_dump_field_strategy_ctrl.strategys == NULL)) {
        return DUMP_FIELD_STRATEGY_ALLOC_ALLOW;
    }

    idx = dump_field_get_strategy_idx(field_id);
    switch  (g_dump_field_strategy_ctrl.mode) {
        case DUMP_FIELD_STRATEGY_PASS:
            ret = dump_field_check_pass_strategy(idx, alloc_len);
            break;
        case DUMP_FIELD_STRATEGY_BLOCK:
            ret = dump_field_check_block_strategy(idx, alloc_len);
            break;
        /* 不应该走到这，到这里说明g_dump_field_strategy_ctrl被踩/跳变/篡改了 */
        default:
            ret = DUMP_FIELD_STRATEGY_ALLOC_REJECT;
            break;
    }
    return ret;
}

int dump_field_strategy_init(void)
{
    u32 max_num = dump_get_u32_cfg(DUMP_CFG_FIELD_NUM_MAX);
    device_node_s *node = NULL;

    if (g_dump_field_strategy_ctrl.inited) {
        return DUMP_OK;
    }

    node = bsp_dt_find_node_by_path("/dump_field_strategy");
    if (node == NULL) {
        dump_error("no dts node\n");
        return DUMP_ERRNO_DTS_NODE;
    }

    if (bsp_dt_property_read_u32(node, "mode", &g_dump_field_strategy_ctrl.mode)) {
        dump_error("err dts prop\n");
        return DUMP_ERRNO_DTS_PROP;
    }

    if (g_dump_field_strategy_ctrl.mode >= (u32)DUMP_FIELD_STRATEGY_BUTT) {
        dump_error("err param\n");
        return DUMP_ERRNO_INVA_PARAM;
    }

    if (g_dump_field_strategy_ctrl.mode == (u32)DUMP_FIELD_STRATEGY_DISABLE) {
        return DUMP_OK;
    }

    if (bsp_dt_property_read_u32(node, "nums", &g_dump_field_strategy_ctrl.nums)) {
        dump_error("err dts prop\n");
        return DUMP_ERRNO_DTS_PROP;
    }

    /* 不论是哪种模式，strategy数目超过field数目只是理论上可能，但实际是不应该出现的 */
    if (g_dump_field_strategy_ctrl.nums > max_num) {
        dump_error("err param\n");
        g_dump_field_strategy_ctrl.mode = (u32)DUMP_FIELD_STRATEGY_DISABLE;
        return DUMP_ERRNO_INVA_PARAM;
    }

    g_dump_field_strategy_ctrl.strategys = osl_malloc(sizeof(dump_field_strategy_store_s) * g_dump_field_strategy_ctrl.nums);
    if (g_dump_field_strategy_ctrl.strategys == NULL) {
        dump_error("err malloc\n");
        g_dump_field_strategy_ctrl.mode = (u32)DUMP_FIELD_STRATEGY_DISABLE;
        return DUMP_ERRNO_MALLOC;
    }

    if (bsp_dt_property_read_u32_array(node, "strategys", (u32 *)g_dump_field_strategy_ctrl.strategys,
                                   g_dump_field_strategy_ctrl.nums * DUMP_FIELD_STRATEGY_U32S) != 0) {
        dump_error("err dts prop\n");
        g_dump_field_strategy_ctrl.mode = (u32)DUMP_FIELD_STRATEGY_DISABLE;
        return DUMP_ERRNO_MALLOC;
    }

    g_dump_field_strategy_ctrl.inited = 1;
    return DUMP_OK;
}

