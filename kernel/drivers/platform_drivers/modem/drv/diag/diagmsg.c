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
#include <securec.h>
#include <osl_types.h>
#include <osl_sem.h>
#include <osl_list.h>
#include <osl_malloc.h>
#include <linux/device.h>
#include <linux/pm_wakeup.h>
#include <mdrv_timer.h>
#include <bsp_slice.h>
#include "encoder.h"
#include "decoder.h"
#include "transfertask.h"
#include "diag_debug.h"
#include "diagmsg.h"


struct msgslice {
    u32 segidx;
    u32 msglen;
    u32 offset;
    u32 timestamp;
    struct list_head list;
    struct msghead *msghd;
};

struct msgctrl {
    osl_sem_id slicesem;
    struct list_head slicelist;
    struct wakeup_source *wakelock;
};

static struct msgctrl g_msgctrl;

static u32 get_encode_srch(u32 msgtype)
{
    switch (msgtype) {
        case DIAG_MSG_TYPE_LOG:
            return ENCODER_SRCH_LOG;
        case DIAG_MSG_TYPE_CNF:
            return ENCODER_SRCH_CNF;
        default:
            return ENCODER_SRCH_BUTT;
    }
}

static u32 encode_msg_package(struct packmsg *packmsg, struct msghead *msghd, u32 leftlen, u32 segment)
{
    u32 srchtype;
    u32 dtoffset = 0;
    u32 senddtlen = leftlen;
    u32 msgtype = packmsg->msgtype;
    struct encode_data encdt = {0};

    /* max encode data length is PAYLOAD_MAX_LEN */
    if (senddtlen > PAYLOAD_MAX_LEN) {
        senddtlen = PAYLOAD_MAX_LEN;
    }

    srchtype = get_encode_srch(msgtype);

    encdt.head = (u8 *)msghd;

    if (segment == 0) {
        encdt.headsz = sizeof(struct msghead);
        encdt.subhead = packmsg->datahd;
        encdt.subheadsz = packmsg->datahdsz;
        senddtlen -= packmsg->datahdsz;
    } else {
        /* the second or last package's pyld is all data, not has p_id, p_len */
        encdt.headsz = sizeof(struct msghead) - sizeof(struct payload);
        dtoffset = packmsg->datasz - leftlen;
    }

    encdt.data = packmsg->data + dtoffset;
    encdt.datasz = senddtlen;

    return encoder_packed_data(&encdt, srchtype);
}

static u32 split_msg_package(struct packmsg *packmsg, struct msghead *msghd)
{
    u32 ret = 0;
    int segment = 0;
    int segidx = 0;
    int leftlen = msghd->pyld.p_len;

    if (msghd->pyld.p_len > PAYLOAD_MAX_LEN) {
        msghd->segment = 1;
    }

    while (leftlen > 0) {
        ret = encode_msg_package(packmsg, msghd, leftlen, segment);
        if (ret) {
            diag_reportptr_record(DIAG_RPTPTR_ENCODE_MSG, 1);
            break;
        }

        leftlen -= PAYLOAD_MAX_LEN;

        segment = 1;
        msghd->segidx = ++segidx;
        if (leftlen < PAYLOAD_MAX_LEN) {
            msghd->segend = 1;
        }
    }

    return ret;
}

u32 pack_diagmsg(struct packmsg *packmsg)
{
    struct msghead msghd = {0};

    msghd.sid = DIAG_MSG_SID;
    msghd.ver = DIAG_MSG_VER_5G;
    msghd.mdmid = packmsg->mdmid;
    msghd.ssid = DIAG_MSG_SSID_ACPU;
    msghd.msgtype = packmsg->msgtype;
    msghd.srctransid = packmsg->srcsn;
    msghd.timestamp = mdrv_timer_get_normal_timestamp();

    msghd.pyld.p_id = packmsg->payloadid;
    msghd.pyld.p_len = packmsg->datahdsz + packmsg->datasz;
    msghd.pyld.ver = packmsg->ver;

    if (msghd.pyld.p_len > PACK_MAX_PAYLOAD_LEN) {
        diag_reportptr_record(DIAG_RPTPTR_OVER_MAX_LEN, 1);
        return DIAG_PACKMSG_OVERFLOW_ERR;
    }

    return split_msg_package(packmsg, &msghd);
}

#define MSGSLICE_TIMEOUT_VALUE (3 * 32768) /* 3s */

static void scan_timeout_msgslice(void)
{
    struct msgslice *curslice = NULL;
    struct msgslice *nextslice = NULL;
    struct msgctrl *msgctrl = &g_msgctrl;
    u32 curtime = bsp_get_slice_value();

    osl_sem_down(&msgctrl->slicesem);

    list_for_each_entry_safe(curslice, nextslice, &msgctrl->slicelist, list) {
        if ((curtime > curslice->timestamp) && \
            ((curtime - curslice->timestamp) > MSGSLICE_TIMEOUT_VALUE)) {

            if ((curslice->list.prev != NULL) && (curslice->list.next != NULL)) {
                list_del(&curslice->list);
            }

            if (curslice->msghd != NULL) {
                diag_err("scan one timeout msg slice p_id:0x%x\n", curslice->msghd->pyld.p_id);
                osl_free(curslice->msghd);
            }

            osl_free(curslice);
        }
    }

    osl_sem_up(&msgctrl->slicesem);
}

static struct msgslice *get_matched_msgslice(struct msghead *msghd, u32 slicelen)
{
    u32 msglen;
    struct msgslice *curslice = NULL;
    struct msgslice *nextslice = NULL;
    struct msgctrl *msgctrl = &g_msgctrl;

    osl_sem_down(&msgctrl->slicesem);
    list_for_each_entry_safe(curslice, nextslice, &msgctrl->slicelist, list) {
        curslice->msghd->segidx = msghd->segidx;
        curslice->msghd->segend = msghd->segend;

        /* the pyld of second package does not contain p_id and p_len, it's all data */
        if (memcmp(curslice->msghd, msghd, (sizeof(struct msghead) - sizeof(struct payload))) == 0) {
            osl_sem_up(&msgctrl->slicesem);
            return curslice;
        }
    }
    osl_sem_up(&msgctrl->slicesem);

    if (msghd->segidx != 0) {
        diag_err("get matched can't find this segment slice p_id:0x%x\n", msghd->pyld.p_id);
        return NULL;
    }

    curslice = (struct msgslice *)osl_malloc(sizeof(struct msgslice));
    if (curslice == NULL) {
        diag_err("get matched msgslice malloc fail\n");
        return NULL;
    }

    INIT_LIST_HEAD(&curslice->list);

    msglen = msghd->pyld.p_len + sizeof(struct msghead);
    curslice->msglen = msglen;

    curslice->msghd = (struct msghead *)osl_malloc(msglen);
    if (curslice->msghd == NULL) {
        diag_err("get matched malloc msg head fail\n");
        osl_free(curslice);
        return NULL;
    }

    if (memcpy_s(curslice->msghd, msglen, msghd, slicelen)) {
        diag_err("get matched memcpy slice msg fail\n");
    }

    curslice->segidx = 0;
    curslice->offset = slicelen;
    curslice->timestamp = mdrv_timer_get_normal_timestamp();

    osl_sem_down(&msgctrl->slicesem);
    list_add_tail(&curslice->list, &msgctrl->slicelist);
    osl_sem_up(&msgctrl->slicesem);

    return curslice;
}

static u32 merge_matched_msgslice(struct msgslice *msgslice, struct msghead *msghd, u32 slicelen)
{
    u32 datalen = slicelen - sizeof(struct msghead) + sizeof(struct payload);
    u8 *dataptr = (u8 *)msgslice->msghd + msgslice->offset;
    u32 leftlen = msgslice->msglen - msgslice->offset;

    if ((datalen > leftlen) && (msghd->segend == 0)) {
        diag_err("merge matched msg data length err\n");
        return DIAG_PARSEMSG_MERGE_MSGLEN_ERR;
    }

    datalen = (datalen > leftlen) ? leftlen : datalen;

    if (memcpy_s(dataptr, leftlen, (u8 *)&msghd->pyld, datalen)) {
        diag_err("merge matched msgslice fail\n");
        return DIAG_PARSEMSG_MERGE_MSGLEN_ERR;
    }

    msgslice->segidx = msghd->segidx;
    msgslice->offset += datalen;
    msgslice->timestamp = mdrv_timer_get_normal_timestamp();

    return 0;
}

#define DIAG_PACK_MSG 0x6961

static u32 combine_msg_package(struct msghead *msghd, u32 msglen, struct msghead **packmsg)
{
    u32 ret;
    struct msgslice *msgslice = NULL;
    u32 msghdlen = (msghd->ver == DIAGMSG_NEW_VER) ? \
        sizeof(struct msghead) : sizeof(struct oldmsghead);
    u32 pyldlen = (msghd->ver == DIAGMSG_NEW_VER) ? \
        msghd->pyld.p_len : ((struct oldmsghead *)msghd)->pyld.p_len;

    /* only msg slice of segidx=0 has p_id and p_len,
     * the following msg slice's payload is all data, when splitting msg
     */
    if (((msghd->segidx == 0) && (pyldlen > PACK_MAX_PAYLOAD_LEN)) \
        || (((msghd->segment == 0) || ((msghd->ver != DIAGMSG_NEW_VER))) && \
        (msglen != (msghdlen + pyldlen)))) {
        diag_err("get matched msgslice length too long(msglen:0x%x, p_len:0x%x)\n", msglen, pyldlen);
        return DIAG_PARSEMSG_GET_INVALID_MSG_ERR;
    }

    /* old version msg can't be splitted to segment */
    if ((msghd->ver != DIAGMSG_NEW_VER) || (msghd->segment == 0)) {
        *packmsg = msghd;
        return DIAG_PACK_MSG;
    }

    scan_timeout_msgslice();

    msgslice = get_matched_msgslice(msghd, msglen);
    if (msgslice == NULL) {
        diag_err("combine msg package get matched slice fail\n");
        return DIAG_PARSEMSG_GET_MATCHED_MSG_ERR;
    }

    if ((msghd->segidx == 0) || (msghd->segidx != (msgslice->segidx + 1))) {
        diag_dbg("msg package no need to combine\n");
        return 0;
    }

    ret = merge_matched_msgslice(msgslice, msghd, msglen);
    if (ret) {
        diag_err("msg package merge fail:%d\n", ret);
        return DIAG_PARSEMSG_MERGE_MSGSLICE_ERR;
    }

    if (msghd->segend) {
        *packmsg = msgslice->msghd;
        return DIAG_PACK_MSG;
    }

    return 0;
}

static void destroy_msg_package(struct msghead *msghd)
{
    struct msgslice *curslice = NULL;
    struct msgslice *nextslice = NULL;
    struct msgctrl *msgctrl = &g_msgctrl;

    if (msghd->segment == 0) {
        return;
    }

    osl_sem_down(&msgctrl->slicesem);
    list_for_each_entry_safe(curslice, nextslice, &msgctrl->slicelist, list) {
        curslice->msghd->segidx = msghd->segidx;
        curslice->msghd->segend = msghd->segend;

        if (memcmp(curslice->msghd, msghd, sizeof(struct msghead)) != 0) {
            continue;
        }

        if ((curslice->list.prev != NULL) && (curslice->list.next != NULL)) {
            list_del(&curslice->list);
        }

        if (curslice->msghd != NULL) {
            osl_free(curslice->msghd);
        }

        osl_free(curslice);
    }
    osl_sem_up(&msgctrl->slicesem);
}

u32 parse_diagmsg(u8 *data, u32 len)
{
    u32 ret;
    static int parsecnt = 0;
    struct msghead *msghd = NULL;
    struct msghead *packmsg = NULL;
    struct msgctrl *msgctrl = &g_msgctrl;
    struct ipc_ttmsg tspttmsg = {0};

    diag_srvptr_record(DIAG_SRVPTR_PARSEMSG_IN, 0, ++parsecnt);
    if ((data == NULL) || (len < sizeof(struct msghead)) || (len > DIAGMSG_SLICE_MAX_LEN)) {
        diag_err("parse diag msg recv invalid data\n");
        diag_srvptr_record(DIAG_SRVPTR_PARSEMSG_END, DIAG_PARSEMSG_DATA_ERR, parsecnt);
        return DIAG_PARSEMSG_DATA_ERR;
    }

    msghd = (struct msghead*)data;

    if ((msghd->ver != DIAGMSG_NEW_VER) && (len < sizeof(struct oldmsghead))) {
        diag_err("parse diag msg recv invalid old msg data\n");
        diag_srvptr_record(DIAG_SRVPTR_PARSEMSG_END, DIAG_PARSEMSG_DATA_ERR, parsecnt);
        return DIAG_PARSEMSG_DATA_ERR;
    }

    __pm_stay_awake(msgctrl->wakelock);

    ret = combine_msg_package(msghd, len, &packmsg);
    if (ret != DIAG_PACK_MSG) {
        __pm_relax(msgctrl->wakelock);
        diag_srvptr_record(DIAG_SRVPTR_PARSEMSG_END, ret, parsecnt);
        return ret;
    }

    tspttmsg.msgid = TTMSG_SERVER_REQUEST;

    if (packmsg->ver == DIAGMSG_NEW_VER) {
        tspttmsg.datasz = sizeof(struct msghead) + packmsg->pyld.p_len;
    } else {
        tspttmsg.datasz = sizeof(struct oldmsghead) + ((struct oldmsghead *)packmsg)->pyld.p_len;
    }

    diag_srvptr_record(DIAG_SRVPTR_SEND_DIAGMSG, packmsg->ver, tspttmsg.datasz);
    ret = send_ttmsg_to_tsp(&tspttmsg, (u8 *)packmsg, tspttmsg.datasz);
    if (ret) {
        __pm_relax(msgctrl->wakelock);
        diag_srvptr_record(DIAG_SRVPTR_PARSEMSG_END, ret, parsecnt);
        diag_err("transfer msg to tsp fail:%d\n", ret);
        return ret;
    }

    destroy_msg_package(packmsg);

    __pm_relax(msgctrl->wakelock);

    diag_srvptr_record(DIAG_SRVPTR_PARSEMSG_END, 0, parsecnt);
    return 0;
}

u32 diagmsg_init(void)
{
    u32 ret;
    struct msgctrl *msgctrl = &g_msgctrl;

    msgctrl->wakelock = wakeup_source_register(NULL, "diagmsg");
    if (msgctrl->wakelock == NULL) {
        diag_err("diag msg ctrl wakelock register fail\n");
        return DIAG_MSG_REGISTER_WAKELOCK_ERR;
    }

    osl_sem_init(1, &msgctrl->slicesem);

    INIT_LIST_HEAD(&msgctrl->slicelist);

    ret = init_encoder();
    if (ret) {
        diag_err("diag encoder init fail\n");
        return ret;
    }

    ret = init_decoder();
    if (ret) {
        diag_err("diag decoder init fail\n");
        return ret;
    }

    return ret;
}

