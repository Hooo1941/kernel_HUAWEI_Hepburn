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
#include <stdarg.h>
#include <securec.h>
#include <msg_id.h>
#include <mdrv_msg.h>
#include <osl_types.h>
#include <linux/spinlock.h>
#include <bsp_print.h>
#include <bsp_odt.h>
#include <bsp_diag.h>
#include "diagmsg.h"
#include "diagnve.h"
#include "diag_debug.h"
#include "connectsrv.h"
#include "transfertask.h"
#include "encoder.h"
#include "logreport.h"


enum {
    DIAG_LOG_OFF = 0,
    DIAG_LOG_ON = 1
};

enum {
    LOGTYPE_PRINT = 0,
    LOGTYPE_TRANS = 0x1,
    LOGTYPE_TRACE = 0x4,
    LOGTYPE_BUTT
};

struct snstru {
    u32 sn;   /* serial number */
    spinlock_t lock;
};

struct logsn {
    u32 srcsn;   /* source serial number */
    struct snstru sn_s[LOGTYPE_BUTT];
};

struct logcfg {
    u8 powerlogen;
    u8 powerprintlevel;
    u8 cyclelogen;
    u8 rsv;
};

/*
 * convert tool config value to print level
 * 0x40000000 ----> ERROR
 * 0x20000000 ----> WARNING
 * 0x10000000 ----> NORMAL
 * 0x08000000 ----> INFO
 */
#define CONVERT_PRINTLEVEL(value, level) \
    do { \
        ((value) & 0x40000000) ? ((level) |= (1 << PS_LOG_LEVEL_ERROR)) : (level); \
        ((value) & 0x20000000) ? ((level) |= (1 << PS_LOG_LEVEL_WARNING)) : (level); \
        ((value) & 0x10000000) ? ((level) |= (1 << PS_LOG_LEVEL_NORMAL)) : (level); \
        ((value) & 0x08000000) ? ((level) |= (1 << PS_LOG_LEVEL_INFO)) : (level); \
    } while (0)

struct printlevel {
    u32 boot;
    u32 drv;     /* real log level:info, normal, warning, error */
    u32 pstopon;   /* the top level map, every bit for one level */
    u32 pspids[PS_MID_TOTAL_NUM];   /* pid's level map, every bit for one level */
};

struct pidon {
    u32 src;
    u32 dst;
};

struct traceonoff {
    struct pidon drvpids[DRV_MID_TOTAL_NUM];
    struct pidon pspids[PS_MID_TOTAL_NUM];
};

struct logonoff {
    u32 boot;
    u32 cycle;
    u32 trans;
    struct printlevel print;
    struct traceonoff trace;
};

struct printhead {
    u32 pid;
    u32 level;
    u32 sn;
};

struct transhead {
    u32 pid;
    u32 msgid;
    u32 sn;
};

struct tracehead {
    u32 srcid;
    u32 dstid;
    u32 sn;
    u32 msgid;
};

struct logsn g_logsn = {0};
struct logonoff g_logonoff = {0};

bool is_bootlog_open(void)
{
    return (g_logonoff.boot == DIAG_LOG_ON) ? true : false;
}

void set_bootlog_onoff(u32 bootonoff)
{
    g_logonoff.boot = bootonoff;
}

bool is_cyclelog_open(void)
{
    return (g_logonoff.cycle == DIAG_LOG_ON) ? true : false;
}

void set_cyclelog_onoff(u32 cycleonoff)
{
    g_logonoff.cycle = cycleonoff;
}

static bool is_printlevel_open(enum printmode mode, u32 module, u32 pid)
{
    struct printlevel *printlevel = &g_logonoff.print;
    u32 level = MODULE_PRINTLEVEL(module);
    u32 pspid = pid - MSG_PS_MID_START;

    if (level == PRINTLOG_LEVEL_OFF) {
        return false;
    }

    /* check print level is open or not */
    if (mode == BOOT) {
        if (level <= printlevel->boot) {
            return true;
        }

        return false;
    }

    if (mode == NORMAL) {
        if (pid == DIAG_DRV_HDS_PID) {
            if (level <= printlevel->drv) {
                return true;
            }

            return false;
        }

        if (printlevel->pstopon & (1 << level)) {
            return true;
        }

        if ((pspid < PS_MID_TOTAL_NUM) && \
            (printlevel->pspids[pspid] & (1 << level))) {
            return true;
        }
    }

    return false;
}

static bool is_bootprintlog_open(unsigned int module)
{
    if (!is_bootlog_open()) {
        return false;
    }

    if (is_printlevel_open(BOOT, module, 0)) {
        return true;
    }

    return false;
}

u32 check_drvprintonoff(u32 module, u32 pid)
{
    if (!is_bootprintlog_open(module)) {
        if (!is_tool_connected()) {
            if (is_cyclelog_open()) {
                return 0;
            }

            return DIAG_SRVPORT_NOTCONNECT_ERR;
        }

        if (!is_printlevel_open(NORMAL, module, pid)) {
            return DIAG_PRINTLOG_NOT_OPEN_ERR;
        }
    }

    return 0;
}

u32 check_printonoff(u32 module, u32 pid)
{
    if (!is_bootprintlog_open(module)) {
        if (!is_tool_connected()) {
            return DIAG_SRVPORT_NOTCONNECT_ERR;
        }

        if (!is_printlevel_open(NORMAL, module, pid)) {
            return DIAG_PRINTLOG_NOT_OPEN_ERR;
        }
    }

    return 0;
}

static u32 pack_printlog(u32 module, u32 pid, char *printlog, u32 loglen)
{
    u32 ret;
    unsigned long flag;
    struct printhead printhd = {0};
    struct packmsg packmsg = {0};
    struct snstru *sn_s = &g_logsn.sn_s[LOGTYPE_PRINT];

    spin_lock_irqsave(&sn_s->lock, flag);
    printhd.pid = pid;
    printhd.level = (1 << (31 - MODULE_PRINTLEVEL(module)));
    printhd.sn = sn_s->sn++;

    packmsg.mdmid = MODULE_MODEMID(module);
    packmsg.srcsn = g_logsn.srcsn++;
    packmsg.msgtype = DIAG_MSG_TYPE_LOG;
    packmsg.encsrch = 0xFF;
    packmsg.payloadid = GEN_PAYLOADID(MODULE_GID(module), MODULE_MODE(module), \
        PAYLOAD_TYPE_PRINT, 0);
    packmsg.datahd = (u8 *)&printhd;
    packmsg.datahdsz = sizeof(struct printhead);
    packmsg.data = printlog;
    packmsg.datasz = loglen;

    ret = pack_diagmsg(&packmsg);
    spin_unlock_irqrestore(&sn_s->lock, flag);

    return ret;
}

#define PRINTLOG_MAX_LEN (0x100)
int printlog_report(u32 module, u32 pid, char *file, u32 line, const char *fmt, va_list arg)
{
    int loglen, tmplen;
    char printlog[PRINTLOG_MAX_LEN] = {0};    /* print log max 255 bytes */

    loglen = snprintf_s(printlog, PRINTLOG_MAX_LEN, (PRINTLOG_MAX_LEN - 1), "%s[%d]", file, line);
    if (loglen < 0) {
        diag_err("snprintf file and line fail:%d\n", loglen);
        return loglen;
    }

    tmplen = PRINTLOG_MAX_LEN - loglen;
    tmplen = vsnprintf_s(printlog + strlen(printlog), tmplen, tmplen - 1, fmt, arg);
    if (tmplen < 0) {
        diag_err("printlog report vsnprintf fail:%d\n", tmplen);
        return tmplen;
    }

    loglen += tmplen;

    return pack_printlog(module, pid, printlog, loglen);
}

static int printlog_report_hook(u32 level, const char *fmt, va_list arg)
{
    u32 ret, module;

    /* drv's module is 0xF4900 */
    module = CREATE_MODULE(0, PAYLOAD_MODE_NONE, level, PAYLOAD_GID_BSP);

    ret = check_drvprintonoff(module, DIAG_DRV_HDS_PID);
    if (ret != 0) {
        return ret;
    }

    return printlog_report(module, DIAG_DRV_HDS_PID, "acpu", 0, fmt, arg);
}

static bool is_translog_open(void)
{
    return (g_logonoff.trans == DIAG_LOG_ON) ? true : false;
}

u32 check_transonoff(void)
{
    if (!is_bootlog_open()) {
        if (!is_tool_connected()) {
            return DIAG_SRVPORT_NOTCONNECT_ERR;
        }

        if (!is_translog_open()) {
            return DIAG_TRANSLOG_NOT_OPEN_ERR;
        }
    }

    return 0;
}

u32 pack_translog(struct translog *translog)
{
    u32 ret = 0;
    unsigned long flag;
    struct transhead transhd = {0};
    struct packmsg packmsg = {0};
    struct snstru *sn_s = &g_logsn.sn_s[LOGTYPE_TRANS];

    spin_lock_irqsave(&sn_s->lock, flag);
    transhd.pid = translog->pid;
    transhd.msgid = translog->msgid;
    transhd.sn = sn_s->sn++;

    packmsg.mdmid = MODULE_MODEMID(translog->module);
    packmsg.srcsn = g_logsn.srcsn++;
    packmsg.msgtype = DIAG_MSG_TYPE_LOG;
    packmsg.encsrch = 0xFF;
    packmsg.payloadid = GEN_PAYLOADID(MODULE_GID(translog->module), MODULE_MODE(translog->module), \
        PAYLOAD_TYPE_TRANS, translog->msgid & 0x7FFFF);
    packmsg.ver = translog->version;
    packmsg.datahd = (u8 *)&transhd;
    packmsg.datahdsz = sizeof(struct transhead);
    packmsg.data = translog->data;
    packmsg.datasz = translog->length;

    ret = pack_diagmsg(&packmsg);
    spin_unlock_irqrestore(&sn_s->lock, flag);

    return ret;
}

#define IS_PS_PID(pid) (((pid) > PS_MID_START) && (pid < PS_MID_END))
#define IS_DRV_PID(pid) (((pid) > DRV_MID_START) && (pid < DRV_MID_END))

bool is_tracelog_open(u32 srcid, u32 dstid)
{
    struct traceonoff *traceonoff = &g_logonoff.trace;
    bool srconoff = false;
    bool dstonoff = false;

    if (IS_DRV_PID(srcid)) {
        srcid -= DRV_MID_START;
        srconoff = (traceonoff->drvpids[srcid].src == DIAG_LOG_ON);
    }

    if (IS_PS_PID(srcid)) {
        srcid -= PS_MID_START;
        srconoff = (traceonoff->pspids[srcid].src == DIAG_LOG_ON);
    }

    if (IS_DRV_PID(dstid)) {
        dstid -= DRV_MID_START;
        dstonoff = (traceonoff->drvpids[dstid].dst == DIAG_LOG_ON);
    }

    if (IS_PS_PID(dstid)) {
        dstid -= PS_MID_START;
        dstonoff = (traceonoff->pspids[dstid].dst == DIAG_LOG_ON);
    }

    if (srconoff || dstonoff) {
        return true;
    }

    return false;
}

u32 pack_tracelog(struct tracelog *tracelog)
{
    u32 ret;
    unsigned long flag;
    struct tracehead tracehd = {0};
    struct packmsg packmsg = {0};
    struct snstru *sn_s = &g_logsn.sn_s[LOGTYPE_TRACE];

    spin_lock_irqsave(&sn_s->lock, flag);
    tracehd.srcid = tracelog->sendpid;
    tracehd.dstid = tracelog->recvpid;
    tracehd.sn = sn_s->sn++;
    tracehd.msgid = tracelog->msgid;

    packmsg.mdmid = 0;
    packmsg.srcsn = g_logsn.srcsn++;
    packmsg.msgtype = DIAG_MSG_TYPE_LOG;
    packmsg.encsrch = 0xFF;
    packmsg.payloadid = GEN_PAYLOADID(MODULE_GID(tracelog->module), MODULE_MODE(tracelog->module), \
        PAYLOAD_TYPE_TRACE, 0);
    packmsg.ver = tracelog->version;
    packmsg.datahd = (u8 *)&tracehd;
    packmsg.datahdsz = sizeof(struct tracehead);
    packmsg.data = (u8 *)tracelog->data;
    packmsg.datasz = tracelog->length;

    ret = pack_diagmsg(&packmsg);
    spin_unlock_irqrestore(&sn_s->lock, flag);

    return ret;
}

void logreport_log_config(u8 *data, u32 datasz)
{
    struct logonoff *logonoff = &g_logonoff;
    struct logcfg *logcfg = (struct logcfg *)data;

    if (datasz != sizeof(struct logcfg)) {
        diag_err("log config recv invalid logcfg data\n");
        return;
    }

    if (!is_tool_connected()) {
        set_bootlog_onoff(logcfg->powerlogen);
    }

    logonoff->print.boot = logcfg->powerprintlevel;

}

void logreport_clear_sn(void)
{
    u32 i;
    unsigned long flag;
    struct snstru *sn_s = NULL;

    for (i = 0; i < LOGTYPE_BUTT; i++) {
        sn_s = &g_logsn.sn_s[i];
        spin_lock_irqsave(&sn_s->lock, flag);
        sn_s->sn = 0;
        spin_unlock_irqrestore(&sn_s->lock, flag);
    }

    g_logsn.srcsn = 0;
}

void logreport_onoff_reset(void)
{
    struct logonoff *logonoff = &g_logonoff;

    logonoff->boot = DIAG_LOG_OFF;

    logonoff->trans = DIAG_LOG_OFF;

    (void)memset_s((void *)&logonoff->print, sizeof(struct printlevel), 0, sizeof(struct printlevel));
    (void)memset_s((void *)&logonoff->trace, sizeof(struct traceonoff), 0, sizeof(struct traceonoff));
}

void logreport_onoff_clear(void)
{
    /* close all log onoff to off */
    (void)memset_s((u8 *)&g_logonoff, sizeof(struct logonoff), 0, sizeof(struct logonoff));
}

void logreport_onoff_allopen(void)
{
    u32 i;
    struct logonoff *logonoff = &g_logonoff;

    logonoff->trans = DIAG_LOG_ON;
    logonoff->print.drv = PRINTLOG_LEVEL_INFO;
    logonoff->print.pstopon = 0xFF;

    for (i = 0; i < PS_MID_TOTAL_NUM; i++) {
        logonoff->print.pspids[i] = 0xFF;
    }

    for (i = 0; i < DRV_MID_TOTAL_NUM; i++) {
        logonoff->trace.drvpids[i].src = DIAG_LOG_ON;
        logonoff->trace.drvpids[i].dst = DIAG_LOG_ON;
    }

    for (i = 0; i < PS_MID_TOTAL_NUM; i++) {
        logonoff->trace.pspids[i].src = DIAG_LOG_ON;
        logonoff->trace.pspids[i].dst = DIAG_LOG_ON;
    }
}

void set_printlog_onoff(struct ptlvl_cfg *ptlvl, u32 len)
{
    u32 i, pspid;
    struct ptlvl_cfg *tmpptlvl = NULL;
    struct printlevel *printlevel = &g_logonoff.print;

    if (ptlvl->pid == 0xFFFFFFFF) {
        (void)memset_s((u8 *)printlevel->pspids, sizeof(u32) * PS_MID_TOTAL_NUM, 0, sizeof(u32) * PS_MID_TOTAL_NUM);
        CONVERT_TO_PRINTLEVELMASK(ptlvl->level, printlevel->pstopon);
        return;
    }

    printlevel->pstopon = 0;

    for (i = 0; i < (len / sizeof(struct ptlvl_cfg)); i++) {
        tmpptlvl = ptlvl + i;
        pspid = tmpptlvl->pid - MSG_PS_MID_START;
        if (pspid < PS_MID_TOTAL_NUM) {
            CONVERT_TO_PRINTLEVELMASK(tmpptlvl->level, printlevel->pspids[pspid]);
        }
    }
}

void set_drvprint_onoff(u32 drvlevel)
{
    struct printlevel *printlevel = &g_logonoff.print;

    printlevel->drv = drvlevel;
}

void set_tracelog_onoff(struct trace_cfg *tcfg, u32 len)
{
    u32 i, tmppid;
    struct trace_cfg *tmptcfg = NULL;
    struct traceonoff *tonoff = &g_logonoff.trace;

    for (i = 0; i < (len / sizeof(struct trace_cfg)); i++) {
        tmptcfg = tcfg + i;

        if (IS_DRV_PID(tmptcfg->pid)) {
            tmppid = tmptcfg->pid - DRV_MID_START;
            if (tmptcfg->type == 0) {
                tonoff->drvpids[tmppid].src = (tmptcfg->onoff >> 31) & 0x1;
            } else {
                tonoff->drvpids[tmppid].dst = (tmptcfg->onoff >> 31) & 0x1;
            }
        }

        if (IS_PS_PID(tmptcfg->pid)) {
            tmppid = tmptcfg->pid - PS_MID_START;
            if (tmptcfg->type == 0) {
                tonoff->pspids[tmppid].src = (tmptcfg->onoff >> 31) & 0x1;
            } else {
                tonoff->pspids[tmppid].dst = (tmptcfg->onoff >> 31) & 0x1;
            }
        }
    }
}

void set_translog_onoff(u32 transonoff)
{
    g_logonoff.trans = transonoff;
}

u32 logreport_init(void)
{
    int i = 0;
    struct snstru *sn_s = g_logsn.sn_s;

    logreport_onoff_clear();

    for (i = 0; i < LOGTYPE_BUTT; i++) {
        spin_lock_init(&sn_s[i].lock);
    }

    bsp_log_string_pull_hook(printlog_report_hook);

    mdmlog_nve_init();

    return 0;
}

