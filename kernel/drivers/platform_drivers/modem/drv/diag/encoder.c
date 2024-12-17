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
#include <product_config.h>
#include <linux/spinlock.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <osl_types.h>
#include <bsp_version.h>
#include <bsp_dump.h>
#include <bsp_odt.h>
#include "diag_debug.h"
#include "srvchan.h"
#include "transfertask.h"
#include "connectsrv.h"
#include "encoder.h"


#define ENCODE_BASE_MAGIC (0x26765210)
#define ENCODE_HEAD_MAGIC (ENCODE_BASE_MAGIC + 0x21D30139)

struct encode_head {
    u32 magic;
    u32 datalen;
};

struct srch_cfg {
    spinlock_t lock;
    u32 chid;
    u32 dstid;
    u32 type;
    u32 mode;
    u32 prority;
    u32 transen;
    u32 rptren;
    u32 buflen;
    u8 *virtbuf;
    u8 *phybuf;
    u32 rd_buflen;
    u8 *rd_virtbuf;
    u8 *rd_phybuf;
    u8 *rptr_virtaddr;
    u8 *rptr_phyaddr;
};

struct dstch_cfg {
    u32 chid;
    u32 cntworkmode;
    u32 cyclemodeen;
    u32 threshold;
    u32 timeoutmode;
    u32 buflen;
    u8 *virtbuf;
    u8 *phybuf;
    u32 srvpoint;
};

#define ENCODER_CNF_SRCBUF_SIZE CONFIG_DIAG_APCNFCH_SIZE
#define ENCODER_LOG_SRCBUF_SIZE CONFIG_DIAG_APLOGCH_SIZE

struct srch_cfg g_srchcfg[ENCODER_SRCH_BUTT] = {
    {
        .chid = ODT_CODER_SRC_ACPU_CNF,
        .dstid = ODT_CODER_DST_OM_CNF,
        .type = ODT_DATA_TYPE_0,
        .mode = ODT_ENCSRC_CHNMODE_CTSPACKET,
        .prority = ODT_CHAN_PRIORITY_3,
        .transen = ODT_TRANS_ID_EN,
        .rptren = ODT_PTR_IMG_DIS,
        .buflen = ENCODER_CNF_SRCBUF_SIZE,
        .virtbuf = NULL,
        .phybuf = NULL,
        .rd_buflen = 0,
        .rd_virtbuf = NULL,
        .rd_phybuf = NULL,
        .rptr_virtaddr = NULL,
        .rptr_phyaddr = NULL,
    },
    {
        .chid = ODT_CODER_SRC_ACPU_IND,
        .dstid = ODT_CODER_DST_OM_IND,
        .type = ODT_DATA_TYPE_0,
        .mode = ODT_ENCSRC_CHNMODE_CTSPACKET,
        .prority = ODT_CHAN_PRIORITY_1,
        .transen = ODT_TRANS_ID_EN,
        .rptren = ODT_PTR_IMG_DIS,
        .buflen = ENCODER_LOG_SRCBUF_SIZE,
        .virtbuf = NULL,
        .phybuf = NULL,
        .rd_buflen = 0,
        .rd_virtbuf = NULL,
        .rd_phybuf = NULL,
        .rptr_virtaddr = NULL,
        .rptr_phyaddr = NULL,
    }
};

#define ENCODER_CNF_DSTCH_THRESHOLD CONFIG_DIAG_CNFDSTCH_THRESHOLD

#define ENCODER_CNF_DSTBUF_SIZE CONFIG_DIAG_CNFDSTCH_SIZE

#define ENCODER_LOG_DSTCH_THRESHOLD CONFIG_DIAG_LOGDSTCH_THRESHOLD

#define ENCODER_LOG_DSTBUF_SIZE CONFIG_DIAG_LOGDSTCH_SIZE

struct dstch_cfg g_dstchcfg[ENCODER_DSTCH_BUTT] = {
    {
        .chid = ODT_CODER_DST_OM_CNF,
        .threshold = ENCODER_CNF_DSTCH_THRESHOLD,
        .timeoutmode = ODT_TIMEOUT_TFR_SHORT,
        .buflen = ENCODER_CNF_DSTBUF_SIZE,
        .virtbuf = NULL,
        .phybuf = NULL,
        .srvpoint = SRVPOINT_CNF
    },
    {
        .chid = ODT_CODER_DST_OM_IND,
        .threshold = ENCODER_LOG_DSTCH_THRESHOLD,
        .timeoutmode = ODT_TIMEOUT_TFR_LONG,
        .buflen = ENCODER_LOG_DSTBUF_SIZE,
        .virtbuf = NULL,
        .phybuf = NULL,
        .srvpoint = SRVPOINT_LOG
    },
};

void encoder_dstch_cfg_query(void)
{
    u32 ret;
    struct ipc_ttmsg tspttmsg = {0};
    odt_encdst_buf_log_cfg_s logcfg = {0};

    bsp_odt_get_log_cfg(&logcfg);

    tspttmsg.msgid = TTMSG_DSTCHCFG_RSP;
    tspttmsg.datasz = sizeof(odt_encdst_buf_log_cfg_s);

    ret = send_ttmsg_to_tsp(&tspttmsg, (u8 *)&logcfg, tspttmsg.datasz);
    if (ret) {
        diag_err("transfer dstch cfg to tsp fail:%d\n", ret);
    }
}

void encoder_set_cyclemodeen(u32 modeen)
{
    struct dstch_cfg *dstchcfg = &g_dstchcfg[ENCODER_DSTCH_LOG];

    dstchcfg->cyclemodeen = modeen;
}

void encoder_switch_cyclemode(void)
{
    int ret;
    struct dstch_cfg *dstchcfg = &g_dstchcfg[ENCODER_DSTCH_LOG];

    if (dstchcfg->cyclemodeen == 0) {
        diag_err("encoder log dst chan cycle mode disabled\n");
        return;
    }

    ret = bsp_odt_set_ind_mode(ODT_IND_MODE_CYCLE);
    if (ret) {
        diag_err("encoder log dst chan cycle mode set fail:0x%x\n", ret);
    }
}

void encoder_switch_cntworkmode(void)
{
    int ret;
    struct dstch_cfg *dstchcfg = &g_dstchcfg[ENCODER_DSTCH_LOG];

    if (!is_tool_connected()) {
        return;
    }

    ret = bsp_odt_set_ind_mode(dstchcfg->cntworkmode);
    if (ret) {
        diag_err("encoder log dst chan cntworkmode:%d set fail:0x%x\n", dstchcfg->cntworkmode, ret);
    }
}

void encoder_set_cntworkmode(u32 cntworkmode)
{
    struct dstch_cfg *dstchcfg = &g_dstchcfg[ENCODER_DSTCH_LOG];

    dstchcfg->cntworkmode = cntworkmode;
}

void encoder_atchange_cntworkmode(u32 cntworkmode)
{
    u32 ret;
    struct ipc_ttmsg tspttmsg = {0};

    encoder_set_cntworkmode(cntworkmode);

    encoder_switch_cntworkmode();

    tspttmsg.msgid = TTMSG_DSTMODE_RSP;

    ret = send_ttmsg_to_tsp(&tspttmsg, NULL, 0);
    if (ret) {
        diag_err("dst chan mode set respone to tsp fail:%d\n", ret);
    }
}

static void write_encdata_tosrcbuf(odt_buffer_rw_s *srcbuf, u8 *data, u32 datalen)
{
    u32 length;

    if (srcbuf->size > datalen) {
        if (memcpy_s(((u8 *)srcbuf->buffer), srcbuf->size, data, datalen)) {
            diag_reportptr_record(DIAG_RPTPTR_MEMCPY_SRCBUF, 1);
        }

        srcbuf->buffer = srcbuf->buffer + datalen;
        srcbuf->size = srcbuf->size - datalen;
        return;
    }

    if (memcpy_s(((u8 *)srcbuf->buffer), srcbuf->size, data, srcbuf->size)) {
        diag_reportptr_record(DIAG_RPTPTR_MEMCPY_SRCBUF, 1);
    }

    length = datalen - srcbuf->size;
    if ((srcbuf->rb_buffer == NULL) || (length > srcbuf->rb_size)) {
        diag_reportptr_record(DIAG_RPTPTR_SRCRBBUF_NULL, 1);
        return;
    }

    if (memcpy_s(srcbuf->rb_buffer, srcbuf->rb_size, (data + srcbuf->size), length)) {
        diag_reportptr_record(DIAG_RPTPTR_MEMCPY_SRCBUF, 1);
    }

    srcbuf->buffer = srcbuf->rb_buffer + length;
    srcbuf->size = srcbuf->rb_size - length;
    srcbuf->rb_buffer = NULL;
    srcbuf->rb_size = 0;
}

u32 encoder_packed_data(struct encode_data *encdt, u32 srchtype)
{
    u32 alignsize = 0;
    unsigned long flag;
    odt_buffer_rw_s srcbuf = {0};
    struct srch_cfg *srchcfg = NULL;
    struct encode_head enchd = {0};

    if (srchtype >= ENCODER_SRCH_BUTT) {
        return DIAG_ENCODE_INVALID_SRCHTYPE_ERR;
    }

    srchcfg = &g_srchcfg[srchtype];

    alignsize = ALIGN_TO_8BYTE(encdt->headsz + encdt->subheadsz + \
        encdt->datasz + sizeof(struct encode_head));

    spin_lock_irqsave(&srchcfg->lock, flag);
    (void)bsp_odt_get_write_buff(srchcfg->chid, &srcbuf);

    diag_reportsize_record(srchtype, alignsize);

    if ((srcbuf.size + srcbuf.rb_size) < alignsize) {
        spin_unlock_irqrestore(&srchcfg->lock, flag);
        diag_reportptr_record(srchtype, alignsize);
        return DIAG_ENCODE_OVERFLOW_SRCBUF_ERR;
    }

    /* src chan buf need get from odt, not use local to convert(need to optimize) */
    srcbuf.buffer = (char *)(srchcfg->virtbuf + ((u8 *)srcbuf.buffer - srchcfg->phybuf));
    srcbuf.rb_buffer = (char *)(srchcfg->virtbuf + ((u8 *)srcbuf.rb_buffer - srchcfg->phybuf));

    enchd.magic = ENCODE_HEAD_MAGIC;
    enchd.datalen = encdt->headsz + encdt->subheadsz + encdt->datasz;
    write_encdata_tosrcbuf(&srcbuf, (u8 *)&enchd, sizeof(struct encode_head));

    write_encdata_tosrcbuf(&srcbuf, encdt->head, encdt->headsz);

    if (encdt->subhead != NULL) {
        write_encdata_tosrcbuf(&srcbuf, encdt->subhead, encdt->subheadsz);
    }

    write_encdata_tosrcbuf(&srcbuf, encdt->data, encdt->datasz);

    (void)bsp_odt_write_done(srchcfg->chid, alignsize);
    spin_unlock_irqrestore(&srchcfg->lock, flag);

    return 0;
}

static void set_odt_srch_rate(void)
{
    DRV_DIAG_RATE_STRU ratepara = {0};

    if (bsp_odt_set_rate_ctrl(&ratepara)) {
        diag_err("encoder set source chan rate fail\n");
    }
}

#define USB_MAX_DATA_LEN (1024 * 1024)

#define ENCODER_DSTCHID_MASK 0xFFFF

static int encoder_dstdata_send(u32 dstchid)
{
    u8 *virtbuf = NULL;
    u32 i, ret, curport, leftsize;
    struct dstch_cfg *dstchcfg = NULL;
    odt_buffer_rw_s dstbuf = {0};

    curport = get_srvport_curtype();

    diag_dstsend_record(dstchid, ENCODER_DSTSEND_START_CNT, 1);
    diag_mntn_record_netlink(curport, dstchid, ENCODER_DSTSEND_START_CNT, 1);
    for(i = 0; i < ENCODER_DSTCH_BUTT; i++) {
        if ((g_dstchcfg[i].chid & ENCODER_DSTCHID_MASK) == dstchid) {
            dstchcfg = &g_dstchcfg[i];
            break;
        }
    }

    if (dstchcfg == NULL) {
        diag_dstsend_record(dstchid, ENCODER_DSTSEND_CHNULL_CNT, 1);
        diag_mntn_record_netlink(curport, dstchid, ENCODER_DSTSEND_CHNULL_CNT, 1);
        return DIAG_ENCODE_GET_DSTCH_ERR;
    }

    (void)bsp_odt_get_read_buff(dstchid, &dstbuf);

    leftsize = dstbuf.size + dstbuf.rb_size;
    diag_dstsend_leftsizercd(dstchid, leftsize);

    if ((leftsize == 0) || (dstbuf.buffer == NULL) || (dstbuf.size == 0)) {
        (void)bsp_odt_read_data_done(dstchid, leftsize);    /* clear all */
        diag_dstsend_record(dstchid, ENCODER_DSTSEND_BUFFERR_CNT, 1);
        diag_mntn_record_netlink(curport, dstchid, ENCODER_DSTSEND_BUFFERR_CNT, 1);
        return DIAG_ENCODE_GET_DSTBUF_ERR;
    }

    /* dst chan virt buf need get from odt, not can local convert(need to optimize) */
    virtbuf = (u8 *)dstbuf.buffer - dstchcfg->phybuf + dstchcfg->virtbuf;

    if ((curport == SRVPORT_USB) && (dstbuf.size > USB_MAX_DATA_LEN)) {
        dstbuf.size = USB_MAX_DATA_LEN;
    }

    diag_dstsend_record(dstchid, ENCODER_DSTSEND_TOTAL_LEN, dstbuf.size);

    /* Don't send rb buffer in this time, need to send rb buffer in next time */
    ret = srvport_data_send(dstchcfg->srvpoint, virtbuf, (u8 *)dstbuf.buffer, dstbuf.size);
    if (ret) {
        (void)bsp_odt_read_data_done(dstchid, dstbuf.size);
        diag_dstsend_record(dstchid, ENCODER_DSTSEND_LOST_LEN, dstbuf.size);
        diag_dstsend_record(dstchid, ENCODER_DSTSEND_FAIL_CNT, 1);
        diag_mntn_record_netlink(curport, dstchid, ENCODER_DSTSEND_FAIL_CNT, 1);
        return ret;
    }

    if (curport != SRVPORT_USB) {
        ret = bsp_odt_read_data_done(dstchid, dstbuf.size);
        if (ret) {
            diag_dstsend_record(dstchid, ENCODER_DSTSEND_LOST_LEN, dstbuf.size);
            diag_dstsend_record(dstchid, ENCODER_DSTSEND_FAIL_CNT, 1);
            diag_mntn_record_netlink(curport, dstchid, ENCODER_DSTSEND_FAIL_CNT, 1);
            return ret;
        }
        diag_dstsend_record(dstchid, ENCODER_DSTSEND_SUCC_LEN, dstbuf.size);
        diag_dstsend_record(dstchid, ENCODER_DSTSEND_SUCC_CNT, 1);
        diag_mntn_record_netlink(curport, dstchid, ENCODER_DSTSEND_SUCC_CNT, 1);
    }

    return 0;
}

static void encoder_dstdata_sendcb(u32 srvpoint, u32 length)
{
    u32 dstchid, i, ret;
    struct dstch_cfg *dstchcfg = NULL;

    for(i = 0; i < ENCODER_DSTCH_BUTT; i++) {
        if (g_dstchcfg[i].srvpoint == srvpoint) {
            dstchcfg = &g_dstchcfg[i];
            break;
        }
    }

    if (dstchcfg == NULL) {
        diag_err("get null dst chan cfg, srvpoint:%d\n", srvpoint);
        return;
    }

    dstchid = dstchcfg->chid & ENCODER_DSTCHID_MASK;

    ret = bsp_odt_read_data_done(dstchid, length);
    if (ret) {
        diag_err("encoder dstdata sendcb release dst chan buff fail:%d\n", ret);
        diag_dstsend_record(dstchid, ENCODER_DSTSEND_LOST_LEN, length);
        diag_dstsend_record(dstchid, ENCODER_DSTSEND_FAIL_CNT, 1);
        return;
    }

    diag_dstsend_record(dstchid, ENCODER_DSTSEND_SUCC_LEN, length);
    diag_dstsend_record(dstchid, ENCODER_DSTSEND_SUCC_CNT, 1);
}

void encoder_reset_cnf_dstch(void)
{
    struct dstch_cfg *dstchcfg = &g_dstchcfg[ENCODER_DSTCH_CNF];
    odt_buffer_rw_s dstbuf = {0};

    (void)bsp_odt_get_read_buff(dstchcfg->chid, &dstbuf);

    (void)bsp_odt_read_data_done(dstchcfg->chid, dstbuf.size + dstbuf.rb_size);
}

/* source chan threshold + min dma byte:(4 * 1024) + 8 + 128 */
#define ENCODER_DSTCH_FREEBUF_THRESHOLD CONFIG_DIAG_DSTCH_FREEBUF_THRD

static u32 cnf_dstch_init(void)
{
    u32 ret;
    odt_dst_chan_cfg_s dstch = {0};
    struct platform_device *pdev = NULL;
    struct dstch_cfg *dstchcfg = &g_dstchcfg[ENCODER_DSTCH_CNF];
    const bsp_version_info_s *ver = bsp_get_version_info();

    pdev = bsp_get_odt_pdev();
    if (pdev == NULL) {
        diag_err("encoder cnf dst chan get odt pdev fail\n");
        return DIAG_ENCODE_GET_ODTPDEV_ERR;
    }

    dstchcfg->virtbuf = (u8 *)dma_alloc_coherent(&pdev->dev, dstchcfg->buflen, (dma_addr_t *)&dstchcfg->phybuf, GFP_KERNEL);
    if (dstchcfg->virtbuf == NULL) {
        diag_err("encoder cnf dst chan malloc dma buffer fail\n");
        return DIAG_ENCODE_DMA_MALLOC_ERR;
    }

    if ((ver != NULL) && ((ver->plat_type == PLAT_ESL) || (ver->plat_type == PLAT_HYBRID))) {
        dstchcfg->threshold = 0x20;
    }

    dstch.encdst_thrh = 0x2 * ENCODER_DSTCH_FREEBUF_THRESHOLD;
    dstch.coder_dst_setbuf.output_start = dstchcfg->phybuf;
    dstch.coder_dst_setbuf.output_end = (dstchcfg->phybuf + dstchcfg->buflen) - 1;
    dstch.coder_dst_setbuf.threshold = dstchcfg->threshold;
    dstch.encdst_timeout_mode = dstchcfg->timeoutmode;

    ret = bsp_odt_coder_set_dst_chan(dstchcfg->chid, &dstch);
    if (ret) {
        diag_err("encoder cnf dst chan set cfg fail:%d\n", ret);
        dma_free_coherent(&pdev->dev, dstchcfg->buflen, (void *)dstchcfg->virtbuf, (dma_addr_t)(uintptr_t)dstchcfg->phybuf);
        return ret;
    }

    bsp_odt_encdst_dsm_init(dstchcfg->chid, ODT_DEST_DSM_ENABLE);

    (void)bsp_odt_register_read_cb(dstchcfg->chid, encoder_dstdata_send);

    (void)bsp_odt_dst_channel_enable(dstchcfg->chid);

    return 0;
}

static u32 log_dstch_init(void)
{
    u32 ret;
    odt_dst_chan_cfg_s dstch = {0};
    odt_encdst_buf_log_cfg_s logcfg = {0};
    struct dstch_cfg *dstchcfg = &g_dstchcfg[ENCODER_DSTCH_LOG];
    const bsp_version_info_s *ver = bsp_get_version_info();

    encoder_set_cntworkmode(ODT_IND_MODE_DIRECT);

    encoder_set_cyclemodeen(0);

    bsp_odt_get_log_cfg(&logcfg);

    dstchcfg->virtbuf = (u8 *)(uintptr_t)logcfg.vir_buffer;
    dstchcfg->phybuf = (u8 *)(uintptr_t)logcfg.phy_buffer_addr;
    dstchcfg->buflen = logcfg.buffer_size;

    if ((ver != NULL) && ((ver->plat_type == PLAT_ESL) || (ver->plat_type == PLAT_HYBRID))) {
        dstchcfg->threshold = 0x200;
    }

    dstch.encdst_thrh = 0x2 * ENCODER_DSTCH_FREEBUF_THRESHOLD;
    dstch.coder_dst_setbuf.output_start = dstchcfg->phybuf;
    dstch.coder_dst_setbuf.output_end = (dstchcfg->phybuf + dstchcfg->buflen) - 1;
    dstch.coder_dst_setbuf.threshold = dstchcfg->threshold;
    dstch.encdst_timeout_mode = dstchcfg->timeoutmode;

    ret = bsp_odt_coder_set_dst_chan(dstchcfg->chid, &dstch);
    if (ret) {
        diag_err("encodet log dst chan set cfg fail:%d\n", ret);
        return ret;
    }

    (void)bsp_odt_register_read_cb(dstchcfg->chid, encoder_dstdata_send);

    (void)bsp_odt_dst_channel_enable(dstchcfg->chid);

    return 0;
}


static u32 encoder_dstch_init(void)
{
    u32 ret;

    ret = cnf_dstch_init();
    if (ret) {
        diag_err("encoder cnf dst chan init fail:%d\n", ret);
        return ret;
    }

    ret = log_dstch_init();
    if (ret) {
        diag_err("encoder log dst chan init fail:%d\n", ret);
        return ret;
    }

    register_srvpoint_sendcb(encoder_dstdata_sendcb);


    return 0;
}

static u32 encoder_srch_init(void)
{
    u32 i, ret;
    struct srch_cfg *srchcfg = NULL;
    struct platform_device *pdev = bsp_get_odt_pdev();

    if (pdev == NULL) {
        diag_err("encoder source chan init get odt pdev fail\n");
        return DIAG_ENCODE_GET_ODTPDEV_ERR;
    }

    for (i = 0; i < ENCODER_SRCH_BUTT; i++) {
        odt_src_chan_cfg_s srch = {0};

        srchcfg = &g_srchcfg[i];

        srchcfg->virtbuf = (u8 *)dma_alloc_coherent(&pdev->dev, srchcfg->buflen, (dma_addr_t *)&srchcfg->phybuf, GFP_KERNEL);
        if (srchcfg->virtbuf == NULL) {
            diag_err("encoder source chan init malloc dma buf fail\n");
            return DIAG_ENCODE_DMA_MALLOC_ERR;
        }

        spin_lock_init(&srchcfg->lock);

        srch.dest_chan_id = srchcfg->dstid;
        srch.data_type = srchcfg->type;
        srch.mode = srchcfg->mode;
        srch.priority = srchcfg->prority;
        srch.trans_id_en = srchcfg->transen;
        srch.rptr_img_en = srchcfg->rptren;
        srch.bypass_en = ODT_HDLC_ENABLE;
        srch.data_type_en = ODT_DATA_TYPE_EN;
        srch.debug_en = ODT_ENC_DEBUG_DIS;
        srch.coder_src_setbuf.input_start = srchcfg->phybuf;
        srch.coder_src_setbuf.input_end = (srchcfg->phybuf + srchcfg->buflen) - 1;

        ret = bsp_odt_coder_set_src_chan(srchcfg->chid, &srch);
        if (ret) {
            dma_free_coherent(&pdev->dev, srchcfg->buflen, (void *)srchcfg->virtbuf, (dma_addr_t)(uintptr_t)srchcfg->phybuf);
            diag_err("encoder source chan init set cfg fail:%d\n", ret);
            return ret;
        }

        ret = bsp_odt_start(srchcfg->chid);
        if (ret) {
            dma_free_coherent(&pdev->dev, srchcfg->buflen, (void *)srchcfg->virtbuf, (dma_addr_t)(uintptr_t)srchcfg->phybuf);
            diag_err("encoder source chan init start fail:%d\n", ret);
            return ret;
        }
    }

    return 0;
}

u32 init_encoder(void)
{
    u32 ret;

    /* set odt source chan rate */
    set_odt_srch_rate();

    ret = encoder_dstch_init();
    if (ret) {
        diag_err("encoder dst chan init fail:%d\n", ret);
        return ret;
    }

    ret = encoder_srch_init();
    if (ret) {
        diag_err("encoder src chan init fail:%d\n", ret);
        return ret;
    }

    diag_err("encoder dst and src chan init success\n");
    return 0;
}

