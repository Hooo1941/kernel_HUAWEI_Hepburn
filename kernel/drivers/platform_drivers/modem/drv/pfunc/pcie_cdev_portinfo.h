/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2018-2020. All rights reserved.
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

#ifndef __PCIE_CDEV_PORTINFO_H__
#define __PCIE_CDEV_PORTINFO_H__
#ifdef __cplusplus
extern "C" {
#endif

#define PCUI_RC_BUFSIZE 8192
#define PCUI_EP_BUFSIZE 1024
#define GPS_RC_BUFSIZE 8192
#define GPS_EP_BUFSIZE 8192
#define G4DIAG_RC_BUFSIZE 2048
#define G4DIAG_EP_BUFSIZE 8192
#define G3DIAG_RC_BUFSIZE 16384
#define G3DIAG_EP_BUFSIZE 16384
#define CTRL_RC_BUFSIZE 8192
#define CTRL_EP_BUFSIZE 1024
#define SKYTONE_RC_BUFSIZE 8192
#define SKYTONE_EP_BUFSIZE 1024
#define AGENTNV_RC_BUFSIZE 0X40000
#define AGENTNV_EP_BUFSIZE 2048
#define AGENTOM_RC_BUFSIZE 2048
#define AGENTOM_EP_BUFSIZE 65536
#define AGENTMSG_RC_BUFSIZE 2048
#define AGENTMSG_EP_BUFSIZE 2048
#define ASHELL_RC_BUFSIZE 8192
#define ASHELL_EP_BUFSIZE 2048
#define MODEM_RC_BUFSIZE 4096
#define MODEM_EP_BUFSIZE 2048
#define PMOM_RC_BUFSIZE 8192
#define PMOM_EP_BUFSIZE 2048
#define RESERVE_BUFSIZE 2048

#ifdef __cplusplus
}
#endif

#endif
