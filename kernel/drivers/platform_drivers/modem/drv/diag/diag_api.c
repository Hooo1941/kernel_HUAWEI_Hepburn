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
#include <linux/string.h>
#include <linux/compiler.h>
#include <osl_types.h>
#include <mdrv_diag.h>
#include <bsp_diag.h>
#include "diagmsg.h"
#include "logreport.h"
#include "connectsrv.h"
#include "diag_debug.h"


static char *get_last_filename(char *filename)
{
    char *lastname = NULL;

    if (filename == NULL) {
        return " ";
    }

    lastname = (char *)strrchr(filename, '/');
    if (lastname == NULL) {
        return filename;
    }

    return (lastname++);
}

u32 bsp_diag_drv_log_report(u32 level, char *fmt, ...)
{
    va_list arg;
    u32 module, ret;

    module = CREATE_MODULE(0, PAYLOAD_MODE_NONE, level, PAYLOAD_GID_BSP);

    ret = check_drvprintonoff(module, DIAG_DRV_HDS_PID);
    if (ret != 0) {
        return ret;
    }

    va_start(arg, fmt);
    ret = printlog_report(module, DIAG_DRV_HDS_PID, "acpu", 0, fmt, arg);
    va_end(arg);

    return ret;
}

u32 mdrv_diag_log_report(u32 module, u32 pid, char *filename, u32 line, char *fmt, ...)
{
    u32 ret;
    va_list arg;

    ret = check_printonoff(module, pid);
    if (ret != 0) {
        return ret;
    }

    va_start(arg, fmt);
    ret = printlog_report(module, pid, get_last_filename(filename), line, fmt, arg);
    va_end(arg);

    return ret;
}

/* pid used for component's flag, can config to tool's pidtable.xml */
unsigned int bsp_diag_trans_report(u32 msgid, u32 pid, u8 *data, u16 length)
{
    u32 ret;
    struct translog translog = {0};

    if (unlikely((data == NULL) || (length == 0))) {
        return DIAG_INVALID_LOGPARAM_ERR;
    }

    ret = check_transonoff();
    if (unlikely(ret != 0)) {
        return ret;
    }

    translog.module = CREATE_MODULE(0, PAYLOAD_MODE_NONE, 0, PAYLOAD_GID_BSP);
    translog.pid = pid;
    translog.msgid = msgid;
    translog.length = length;
    translog.data = data;

    return pack_translog(&translog);
}

unsigned int mdrv_diag_trans_report(mdrv_diag_trans_ind_s *translog)
{
    u32 ret;

    if (unlikely(translog == NULL)) {
        return DIAG_INVALID_LOGPARAM_ERR;
    }

    ret = check_transonoff();
    if (unlikely(ret != 0)) {
        return ret;
    }

    return pack_translog((struct translog *)translog);
}

unsigned int mdrv_diag_trace_report(mdrv_diag_trace_s *logdata)
{
    struct tracelog *tracelog = (struct tracelog *)logdata;

    if (unlikely(tracelog == NULL)) {
        return DIAG_INVALID_LOGPARAM_ERR;
    }

    if (!is_bootlog_open()) {
        if (!is_tool_connected()) {
            return DIAG_SRVPORT_NOTCONNECT_ERR;
        }

        if (!is_tracelog_open(tracelog->sendpid, tracelog->recvpid)) {
            return DIAG_TRACELOG_NOT_OPEN_ERR;
        }
    }

    return pack_tracelog(tracelog);
}

unsigned int mdrv_diag_get_conn_state(void)
{
    return get_toolcnt_state();
}

unsigned int mdrv_diag_conn_state_notify_fun_reg(mdrv_diag_conn_state_notify_fun notifier)
{
    return connectstate_notifer_register(notifier);
}

