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
#ifndef __LOGREPORT_H__
#define __LOGREPORT_H__

#include <stdarg.h>
#include <osl_types.h>
#include <mdrv_msg.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

enum {
    PRINTLOG_LEVEL_OFF = 0,
    PRINTLOG_LEVEL_ERROR,
    PRINTLOG_LEVEL_WARNING,
    PRINTLOG_LEVEL_NORMAL,
    PRINTLOG_LEVEL_INFO,
    PRINTLOG_LEVEL_BUTT,
};

/*
 * module:
 * 31~24 bit: modem id
 * 23~20 bit: reserve
 * 19~16 bit: mode
 * 15~12 bit: print level
 * 11~ 8 bit: group id
 * 7 ~ 0 bit: reserve
 * gid is payload group id, level is 0 or print level
 */
#define CREATE_MODULE(mdmid, mode, level, gid) \
    ((((mdmid) & 0xFF) << 24) | (((mode) & 0xF) << 16) | (((level) & 0xF) << 12) | (((gid) & 0xF) << 8))

#define MODULE_MODEMID(module) ((module) >> 24)
#define MODULE_MODE(module) (((module) & 0x000F0000) >> 16)   /* mode is one of LTE/TDS/GSM//NR etc */
#define MODULE_PRINTLEVEL(module) (((module) & 0xF000) >> 12)
#define MODULE_GID(module) (((module) & 0xF00) >> 8)   /* group id */

#define ADD_MODULE_GID(module, gid) (((module) &(~(0xF00))) | (((gid) << 8) & (0xF00)))

/*
 * convert tool config value to print level
 * 0x40000000 ----> ERROR
 * 0x20000000 ----> WARNING
 * 0x10000000 ----> NORMAL
 * 0x08000000 ----> INFO
 */
#define CONVERT_TO_PRINTLEVELMASK(value, level) do { \
        ((value) & 0x40000000) ? ((level) |= (1 << PS_LOG_LEVEL_ERROR)) : (level); \
        ((value) & 0x20000000) ? ((level) |= (1 << PS_LOG_LEVEL_WARNING)) : (level); \
        ((value) & 0x10000000) ? ((level) |= (1 << PS_LOG_LEVEL_NORMAL)) : (level); \
        ((value) & 0x08000000) ? ((level) |= (1 << PS_LOG_LEVEL_INFO)) : (level); \
} while (0)


/* print level config */
struct ptlvl_cfg {
    u32 pid;
    u32 level;
};

enum printmode {
    BOOT = 1,
    NORMAL = 2,
};

struct trace_cfg {
    u32 pid;
    u32 type;
    u32 onoff;
};

struct translog {
    u32 module;
    u32 pid;
    u32 msgid;
    u32 rsv;
    u32 length : 16;
    u32 rsv1 : 8;
    u32 version : 8;
    u8 *data;
};

struct tracelog {
    u32 module;
    u32 sendpid;
    u32 recvpid;
    u32 msgid;
    u32 length: 16;
    u32 rsv: 8;
    u32 version: 8;
    void *data;
};

bool is_bootlog_open(void);

bool is_cyclelog_open(void);

u32 check_drvprintonoff(u32 module, u32 pid);

u32 check_printonoff(u32 module, u32 pid);

int printlog_report(u32 module, u32 pid, char *file, u32 line, const char *fmt, va_list arg);

u32 check_transonoff(void);

u32 pack_translog(struct translog *translog);

bool is_tracelog_open(u32 srcid, u32 dstid);

u32 pack_tracelog(struct tracelog *tracelog);

void logreport_log_config(u8 *data, u32 datasz);

void logreport_onoff_reset(void);

void logreport_clear_sn(void);

void logreport_onoff_allopen(void);

void set_printlog_onoff(struct ptlvl_cfg *ptlvl, u32 len);

void set_drvprint_onoff(u32 drvlevel);

void set_tracelog_onoff(struct trace_cfg *tcfg, u32 len);

void set_translog_onoff(u32 transonoff);

u32 logreport_init(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif   /* __LOGREPORT_H__ */
