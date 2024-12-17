/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2018. All rights reserved.
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
#include <mdrv_diag.h>
#include <bsp_dump.h>
#include "msg_lite.h"
#include "msg_mntn.h"

#define MSG_MNTN_DIAG_TRANS_REPORT 0x1
#define MSG_MNTN_DIAG_PM_REPORT 0x2
#define MSG_MNTN_DEFAULT (MSG_MNTN_DIAG_PM_REPORT)

static u32 g_msg_mntn_ctrl = 0;
msg_dump_info_s g_msg_dump = { 0 };
msg_dump_spinlock_s g_msg_slock;

void msg_rec_setmntn(u32 val)
{
    g_msg_mntn_ctrl = val;
}

void msg_diag_trans_report(msg_dump_s *info)
{
    mdrv_diag_trans_ind_s trans_msg;
    if ((g_msg_mntn_ctrl & MSG_MNTN_DIAG_TRANS_REPORT) == 0) {
        return;
    }

    trans_msg.ulModule = MDRV_DIAG_GEN_MODULE_EX(DIAG_MODEM_0, DIAG_MODE_COMM, DIAG_MSG_TYPE_BSP);
    trans_msg.ulPid = 0x8003;
    trans_msg.ulReserve = 0;
    trans_msg.ulMsgId = 0x2007;
    trans_msg.ulLength = sizeof(msg_dump_s);
    trans_msg.pData = (void *)info;

    (void)mdrv_diag_trans_report(&trans_msg);
    return;
}

void msg_diag_pm_report(msg_pm_dump_info_s *info)
{
    mdrv_diag_trans_ind_s trans_msg;
    if ((g_msg_mntn_ctrl & MSG_MNTN_DIAG_PM_REPORT) == 0) {
        return;
    }

    trans_msg.ulModule = MDRV_DIAG_GEN_MODULE_EX(DIAG_MODEM_0, DIAG_MODE_COMM, DIAG_MSG_TYPE_BSP);
    trans_msg.ulPid = 0x8003;
    trans_msg.ulReserve = 0;
    trans_msg.ulMsgId = 0x2008;
    trans_msg.ulLength = sizeof(msg_pm_dump_info_s);
    trans_msg.pData = (void *)info;

    (void)mdrv_diag_trans_report(&trans_msg);

    return;
}

void lite_assign_dump_pos(struct msg_chn_hdl *msg_fd_info)
{
    unsigned long lock_flag;
    lite_dump_info_s *plite_dump = &g_msg_dump.lite_dump_info;

    spin_lock_irqsave(&g_msg_slock.lite_lock, lock_flag);
    msg_fd_info->dump_pos = plite_dump->lite_dump_pos++ % plite_dump->lite_info_cnt;
    spin_unlock_irqrestore(&g_msg_slock.lite_lock, lock_flag);
}

void lite_dump_record(const struct msg_addr *src, const struct msg_addr *dst, u32 len, u32 record_pos, s16 trace_code)
{
    u8 direction = src->core == THIS_CORE ? DUMP_SEND_FLAG : DUMP_RECV_FLAG;
    lite_dump_s lite_record = { 0 };
    lite_dump_info_s *plite_dump = &g_msg_dump.lite_dump_info;
    u32 lite_info_cnt = plite_dump->lite_info_cnt;

    if (record_pos >= lite_info_cnt) {
        return;
    }

    if (trace_code == MSG_TRACE_LITE_BEFORE) {
        lite_record.start_time = (u32)bsp_get_slice_value();
        lite_record.src_chnid = src->chnid;
        lite_record.dst_chnid = dst->chnid;
        lite_record.len = len;
        lite_record.trace_code = trace_code;
        lite_record.opposite_coreid = direction == DUMP_SEND_FLAG ? (dst->core) : (src->core);

        if (g_msg_dump.msg_pm_flag && direction) {
            g_msg_dump.msg_pm_dump_info.lite_pm_dump = lite_record;
            g_msg_dump.msg_pm_flag = 0;
            msg_diag_pm_report(&g_msg_dump.msg_pm_dump_info);

            if (g_msg_dump.msg_irq_wakeup) {
                msg_lite_wakeup_callback(src, dst);
                msg_err("wakeup msg src_id 0x%x, dst_id: 0x%x\n", src->chnid, dst->chnid);
                g_msg_dump.msg_irq_wakeup = 0;
            }
        }

        plite_dump->lite_dumps[record_pos + direction * lite_info_cnt] = lite_record;
    } else {
        plite_dump->lite_dumps[record_pos + direction * lite_info_cnt].end_time =
            (u32)bsp_get_slice_value();
        plite_dump->lite_dumps[record_pos + direction * lite_info_cnt].trace_code = trace_code;
    }
}

int is_exist_mid(msg_dump_s *pinfo, int dump_pos, u8 direction)
{
    int ret;

    if (direction == DUMP_SEND_FLAG) {
        ret = (g_msg_dump.cross_dump_info.cross_dumps[dump_pos].src_mid == pinfo->src_mid);
    } else {
        ret = (g_msg_dump.cross_dump_info.cross_dumps[dump_pos].dst_mid == pinfo->dst_mid);
    }

    return ret;
}

int cross_get_dump_pos(msg_dump_s *pinfo, u8 direction)
{
    u8 dump_pos;
    unsigned long lock_flag;
    cross_dump_info_s *pcross_dump = &g_msg_dump.cross_dump_info;
    u32 cross_info_cnt = pcross_dump->cross_info_cnt;
    u8 cross_dump_base = direction * cross_info_cnt;
    u8 cross_dump_max = cross_info_cnt + cross_dump_base;

    spin_lock_irqsave(&g_msg_slock.cross_lock, lock_flag);
    for (dump_pos = cross_dump_base; dump_pos < cross_dump_max; dump_pos++) {
        if (is_exist_mid(pinfo, dump_pos, direction)) {
            spin_unlock_irqrestore(&g_msg_slock.cross_lock, lock_flag);
            return dump_pos;
        }
    }

    dump_pos = pcross_dump->cross_dump_pos[direction]++ % cross_info_cnt;
    spin_unlock_irqrestore(&g_msg_slock.cross_lock, lock_flag);

    dump_pos = dump_pos + direction * cross_info_cnt;
    return dump_pos;
}

void cross_dump_record(msg_dump_s *pinfo, u8 direction, s16 trace_code)
{
    int record_pos = cross_get_dump_pos(pinfo, direction);
    pinfo->trace_code = trace_code;

    if (g_msg_dump.msg_pm_flag && direction) {
        g_msg_dump.msg_pm_dump_info.cross_pm_dump = *pinfo;
        g_msg_dump.msg_pm_flag = 0;
        msg_diag_pm_report(&g_msg_dump.msg_pm_dump_info);
    }

    g_msg_dump.cross_dump_info.cross_dumps[record_pos] = *pinfo;
}

void inter_dump_record(msg_dump_s *pinfo)
{
    u32 pos;
    inter_dump_info_s *pinter_dump = &g_msg_dump.inter_dump_info;
    pos = pinter_dump->inter_dump_pos & (pinter_dump->inter_info_cnt - 1);
    pinter_dump->inter_dump_pos++;
    pinter_dump->inter_dumps[pos] = *pinfo;
}

void cross_tx_record(msg_dump_s *pinfo, s16 trace_code)
{
    u32 cur_pos;
    unsigned long lock_flag;
    tx_fail_info_s *ptx_fail = &g_msg_dump.tx_fail_info;
    pinfo->trace_code = trace_code;

    spin_lock_irqsave(&g_msg_slock.cross_lock, lock_flag);
    cur_pos = ptx_fail->cross_tx_pos++ & (MSG_DUMP_CROSS_TX_NUM - 1);
    spin_unlock_irqrestore(&g_msg_slock.cross_lock, lock_flag);
    ptx_fail->cross_tx[cur_pos] = *pinfo;
}

void lite_tx_record(u32 record_pos, s16 trace_code)
{
    u32 cur_pos;
    unsigned long lock_flag;
    tx_fail_info_s *ptx_fail = &g_msg_dump.tx_fail_info;
    lite_dump_info_s *plite_dump = &g_msg_dump.lite_dump_info;
    u32 lite_info_cnt = plite_dump->lite_info_cnt;

    if (record_pos >= lite_info_cnt) {
        return;
    }

    spin_lock_irqsave(&g_msg_slock.lite_lock, lock_flag);
    cur_pos = ptx_fail->lite_tx_pos++ & (MSG_DUMP_LITE_TX_NUM - 1);
    spin_unlock_irqrestore(&g_msg_slock.lite_lock, lock_flag);

    ptx_fail->lite_tx[cur_pos] = plite_dump->lite_dumps[record_pos];
    ptx_fail->lite_tx[cur_pos].end_time = (u32)bsp_get_slice_value();
    ptx_fail->lite_tx[cur_pos].trace_code = trace_code;
}

static u32 msg_dump_fill_cxt(u8 *buf, u32 len, msg_dump_head_s *pdump_head, void *cxt, u32 cxt_len)
{
    u32 used_len = 0;

    if (memcpy_s(buf, len, (void *)pdump_head, sizeof(*pdump_head))) {
        return 0;
    }
    used_len += sizeof(*pdump_head);

    if (memcpy_s(buf + used_len, len - used_len, cxt, cxt_len)) {
        return 0;
    }
    used_len += cxt_len;

    return used_len;
}

static inline void msg_dump_fill_tx_content(u8 *dump_cur, u8 *dump_end)
{
    u32 used_len;
    tx_fail_info_s *ptx_dump = &g_msg_dump.tx_fail_info;
    msg_dump_head_s dump_head = { 0 };

    dump_head.dump_magic = MSG_DUMP_TX_FAIL_MAGIC;
    dump_head.dump_size = ptx_dump->lite_tx_size;
    used_len = msg_dump_fill_cxt(dump_cur, dump_end - dump_cur, &dump_head, ptx_dump->lite_tx,
        ptx_dump->lite_tx_size);
    if (used_len == 0) {
        msg_err("fail to copy tx lite dump\n");
        return;
    }
    dump_cur += used_len;

    dump_head.dump_magic = MSG_DUMP_TX_FAIL_MAGIC;
    dump_head.dump_size = ptx_dump->cross_tx_size;
    used_len = msg_dump_fill_cxt(dump_cur, dump_end - dump_cur, &dump_head, ptx_dump->cross_tx,
        ptx_dump->cross_tx_size);
    if (used_len == 0) {
        msg_err("fail to copy tx cross dump\n");
        return;
    }
}

void msg_dump_hook(void)
{
    u32 used_len;
    u8 *dump_cur = g_msg_dump.dump_base;
    u8 *dump_end = g_msg_dump.dump_base + g_msg_dump.dump_size;
    msg_dump_head_s dump_head = { 0 };
    lite_dump_info_s *plite_dump = &g_msg_dump.lite_dump_info;
    cross_dump_info_s *pcross_dump = &g_msg_dump.cross_dump_info;
    inter_dump_info_s *pinter_dump = &g_msg_dump.inter_dump_info;

    dump_head.dump_magic = MSG_DUMP_LITE_MAGIC;
    dump_head.dump_size = plite_dump->lite_dump_size;
    used_len = msg_dump_fill_cxt(dump_cur, dump_end - dump_cur, &dump_head, plite_dump->lite_dumps,
        plite_dump->lite_dump_size);
    if (used_len == 0) {
        msg_err("fail to copy msg lite dump\n");
        return;
    }
    dump_cur += used_len;

    dump_head.dump_magic = MSG_DUMP_CROSS_MAGIC;
    dump_head.dump_size = pcross_dump->cross_dump_size;
    used_len = msg_dump_fill_cxt(dump_cur, dump_end - dump_cur, &dump_head, pcross_dump->cross_dumps,
        pcross_dump->cross_dump_size);
    if (used_len == 0) {
        msg_err("fail to copy msg cross dump\n");
        return;
    }
    dump_cur += used_len;

    dump_head.dump_magic = MSG_DUMP_INTER_MAGIC;
    dump_head.dump_size = pinter_dump->inter_dump_size;
    used_len = msg_dump_fill_cxt(dump_cur, dump_end - dump_cur, &dump_head, pinter_dump->inter_dumps,
        pinter_dump->inter_dump_size);
    if (used_len == 0) {
        msg_err("fail to copy msg inter dump\n");
        return;
    }
    dump_cur += used_len;

    if (memcpy_s(dump_cur, dump_end - dump_cur, &g_msg_dump.msg_pm_dump_info, sizeof(msg_pm_dump_info_s))) {
        msg_err("fail to copy msg dump\n");
        return;
    }
    dump_cur += sizeof(msg_pm_dump_info_s);

    msg_dump_fill_tx_content(dump_cur, dump_end);
    return;
}

void get_msg_dump_info(msg_dump_s *info, void *msg)
{
    info->dst_mid = (u16)MDRV_MSG_HEAD_GET_DSTID(msg);
    info->src_mid = (u16)MDRV_MSG_HEAD_GET_SRCID(msg);
    info->len = MDRV_MSG_HEAD_GET_LEN(msg);
    info->time_stamp = (u32)bsp_get_slice_value();
    info->flag = (u16)(MSG_HEAD_GET_FLAG(msg));
}

static inline u32 msg_dump_lite_init(u32 info_cnt)
{
    u32 dump_size = sizeof(msg_dump_head_s);
    lite_dump_info_s *plite_dump = &g_msg_dump.lite_dump_info;

    plite_dump->lite_info_cnt = info_cnt;
    plite_dump->lite_dump_size = 2 * sizeof(lite_dump_s) * plite_dump->lite_info_cnt;

    plite_dump->lite_dumps = (lite_dump_s *)osl_malloc(plite_dump->lite_dump_size);
    if (plite_dump->lite_dumps == NULL) {
        return 0;
    }
    (void)memset_s(plite_dump->lite_dumps, plite_dump->lite_dump_size, 0, plite_dump->lite_dump_size);
    dump_size += plite_dump->lite_dump_size;

    spin_lock_init(&g_msg_slock.lite_lock);

    return dump_size;
}

static inline u32 msg_dump_cross_init(u32 info_cnt)
{
    u32 dump_size = sizeof(msg_dump_head_s);
    cross_dump_info_s *pcross_dump = &g_msg_dump.cross_dump_info;

    pcross_dump->cross_info_cnt =  info_cnt;
    pcross_dump->cross_dump_size = 2 * sizeof(msg_dump_s) * pcross_dump->cross_info_cnt;

    pcross_dump->cross_dumps = (msg_dump_s *)osl_malloc(pcross_dump->cross_dump_size);
    if (pcross_dump->cross_dumps == NULL) {
        return 0;
    }
    (void)memset_s(pcross_dump->cross_dumps, pcross_dump->cross_dump_size, 0, pcross_dump->cross_dump_size);
    dump_size += pcross_dump->cross_dump_size;

    spin_lock_init(&g_msg_slock.cross_lock);

    return dump_size;
}

static inline u32 msg_dump_inter_init(u32 info_cnt)
{
    u32 dump_size = sizeof(msg_dump_head_s);
    inter_dump_info_s *pinter_dump = &g_msg_dump.inter_dump_info;

    pinter_dump->inter_info_cnt = info_cnt;
    pinter_dump->inter_dump_size = sizeof(msg_dump_s) * pinter_dump->inter_info_cnt;

    pinter_dump->inter_dumps = (msg_dump_s *)osl_malloc(pinter_dump->inter_dump_size);
    if (pinter_dump->inter_dumps == NULL) {
        return 0;
    }
    (void)memset_s(pinter_dump->inter_dumps, pinter_dump->inter_dump_size, 0, pinter_dump->inter_dump_size);
    dump_size += pinter_dump->inter_dump_size;

    return dump_size;
}

static inline int msg_dump_tx_fail_init(void)
{
    u32 malloc_size;
    u32 dump_size = sizeof(msg_dump_head_s);
    tx_fail_info_s *ptx_fail_dump = &g_msg_dump.tx_fail_info;

    malloc_size = sizeof(msg_dump_s) * MSG_DUMP_CROSS_TX_NUM;
    ptx_fail_dump->cross_tx = (msg_dump_s *)osl_malloc(malloc_size);
    if (ptx_fail_dump->cross_tx == NULL) {
        return 0;
    }
    ptx_fail_dump->cross_tx_size = malloc_size;
    (void)memset_s(ptx_fail_dump->cross_tx, malloc_size, 0, malloc_size);
    dump_size += malloc_size;

    malloc_size = sizeof(lite_dump_s) * MSG_DUMP_LITE_TX_NUM;
    ptx_fail_dump->lite_tx = (lite_dump_s *)osl_malloc(malloc_size);
    if (ptx_fail_dump->lite_tx == NULL) {
        return 0;
    }
    ptx_fail_dump->lite_tx_size = malloc_size;
    (void)memset_s(ptx_fail_dump->lite_tx, malloc_size, 0, malloc_size);
    dump_size += malloc_size;

    return dump_size;
}

static int msg_dump_register_field(void)
{
    g_msg_dump.dump_base = bsp_dump_register_field(DUMP_MDMAP_MSG, "MSG", g_msg_dump.dump_size, 0);
    if (g_msg_dump.dump_base == NULL) {
        msg_err("get msg field fail!\n");
        return MSG_ERR_ENOMEM;
    }
    (void)memset_s(g_msg_dump.dump_base, g_msg_dump.dump_size, 0, g_msg_dump.dump_size);

    if (bsp_dump_register_hook("MSG", (dump_hook)msg_dump_hook) == -1) {
        msg_err("msg_dump_hook init fail\n");
        return MSG_ERR_EIO;
    }
    return 0;
}

void msg_dump_resume(void)
{
    g_msg_dump.msg_pm_flag = 1;
    return;
}

int msg_irq_wakeup_set(void *cbk_arg)
{
    UNUSED(cbk_arg);
    g_msg_dump.msg_irq_wakeup = 1;
    return 0;
}

int msg_dump_init(void)
{
    s32 ret;
    u32 total_size = 0;
    u32 dump_size, dump_info_cnt[0x3];
    device_node_s *pdev_node = NULL;
    const char *node_path = "/msg_event";

    pdev_node = bsp_dt_find_node_by_path(node_path);
    if (pdev_node == NULL) {
        return MSG_ERR_ENXIO;
    }

    ret = bsp_dt_property_read_u32(pdev_node, "dump_size", &g_msg_dump.dump_size);
    if (ret) {
        return MSG_ERR_ENXIO;
    }
    ret = bsp_dt_property_read_u32_array(pdev_node, "dump_info_cnt", dump_info_cnt, sizeof(dump_info_cnt) / sizeof(dump_info_cnt[0]));
    if (ret) {
        return MSG_ERR_ENXIO;
    }

    dump_size = msg_dump_lite_init(dump_info_cnt[0x0]);
    if(dump_size == 0) {
        msg_err("lite dump init failed!\n");
        return -1;
    }
    total_size += dump_size;

    dump_size = msg_dump_cross_init(dump_info_cnt[0x1]);
    if(dump_size == 0) {
        msg_err("cross dump init failed!\n");
        return -1;
    }
    total_size += dump_size;

    dump_size = msg_dump_inter_init(dump_info_cnt[0x2]);
    if(dump_size == 0) {
        msg_err("inter dump init failed!\n");
        return -1;
    }
    total_size += dump_size;
    total_size += sizeof(msg_pm_dump_info_s);

    dump_size = msg_dump_tx_fail_init();
    if(dump_size == 0) {
        msg_err("tx dump init failed!\n");
        return -1;
    }
    total_size += dump_size;

    if (total_size > g_msg_dump.dump_size) {
        return MSG_ERR_ENOMEM;
    }

    ret = msg_dump_register_field();
    return 0;
}

int msg_mntn_init(void)
{
    int ret;
    /* init hids report default level */
    msg_rec_setmntn(MSG_MNTN_DEFAULT);

    ret = msg_dump_init();
    if (ret) {
        return ret;
    }

    return 0;
}
EXPORT_SYMBOL(msg_rec_setmntn);
EXPORT_SYMBOL(msg_dump_hook);
