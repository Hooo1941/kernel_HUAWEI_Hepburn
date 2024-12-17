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
 */
#ifndef __CORESIGHT_ERRNO_H__
#define __CORESIGHT_ERRNO_H__
enum coresight_trace_error_s {
    CORESIGHT_TRACE_OK = 0,
    CORESIGHT_INIT_OK = 0,
    CORESIGHT_ETF_OK = 0,
    CORESIGHT_NUMS_ERR,
    CORESIGHT_ETF_INIT_ERR,
    CORESIGHT_FUNNEL_INIT_ERR,
    CORESIGHT_TSPETF_MAP_ERR,
    CORESIGHT_NO_DDR,
    CORESIGHT_DDR_MAP_ERR,
    CORESIGHT_TRACE_NODONE,
    CORESIGHT_TYPE_ERR,
    CORESIGHT_PROGRAM_TRACE_SET_ERR,
    CORESIGHT_DATA_TRACE_SET_ERR,

    CORESIGHT_ETF_NOFOUND,
    CORESIGHT_NO_ETF_NUM,
    CORESIGHT_NO_ETF_MODE,
    CORESIGHT_ETF_MALLOC_ERR,
    CORESIGHT_ETF_CHILD_ERR,
    CORESIGHT_ETF_CHILD_NOFOUND,
    CORESIGHT_TIME_OUT,
    CORESIGHT_ETF_ENABLE_FAIL,

    CORESIGHT_MDM_FUNNEL_NOFOUND,
    CORESIGHT_NO_NUM,
    CORESIGHT_MALLOC_ERR,
    CORESIGHT_FUNNEL_CHILD_NOFOUND,
    CORESIGHT_NO_CHNNL,
    CORESIGHT_CHILD_ERR,
    CORESIGHT_FUNNEL_ENABLE_FAIL,

    CORESIGHT_CS_NOFOUND,
    CORESIGHT_CS_MAP_ERR,

    CORESIGHT_REP_BASE_ERR,
    CORESIGHT_REP_STATE_ERR,
    CORESIGHT_REP_SEL_ERR,
    CORESIGHT_NO_REP,
    CORESIGHT_REP_NO_NUM,
    CORESIGHT_REP_MALLOC_ERR,
    CORESIGHT_REP_OVERFLOW,
    CORESIGHT_REP_MAP_ERR,
    CORESIGHT_REP_CHNNL0_ERR,
    CORESIGHT_REP_CHNNL1_ERR,

    CORESIGHT_DEV_NOFOUND,
    CORESIGHT_MAP_ERR,
    CORESIGHT_CHILD_NOFOUND,
    CORESIGHT_GET_MEM_ERR,
    CORESIGHT_DEV_ERR,
    CORESIGHT_VIRT_ERR,
};
#endif
