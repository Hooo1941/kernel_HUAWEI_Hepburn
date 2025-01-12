/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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

#include <bsp_dt.h>
#include "bsp_print.h"
#include "bsp_odt.h"
#include "hal_common.h"


device_node_s *odt_hal_get_dev_info(void)
{
    return g_odt_hal_info.dev;
}

void odt_hal_set_timeout_threshold(u32 mode, u32 timeout)
{
    u32 temp;

    temp = ODT_REG_READ(ODT_REG_INTTIMEOUT);

    if (mode == ODT_TIMEOUT_TFR_LONG) {
        timeout = (timeout << 0x8) | (temp & 0xff);
    } else {
        timeout = (temp & 0xffffff00) | timeout;
    }

    ODT_REG_WRITE(ODT_REG_INTTIMEOUT, timeout);

    return;
}

void odt_hal_global_reset(void)
{
    ODT_REG_SETBITS(ODT_REG_GBLRST, 0, 1, 0x1);
}

u32 odt_hal_get_global_reset_state(void)
{
    return ODT_REG_GETBITS(ODT_REG_GBLRST, 0, 1);
}

u32 odt_hal_get_timeout_mode(void)
{
    return ODT_REG_GETBITS(ODT_REG_GBLRST, 0x4, 1);
}

void odt_hal_set_single_chan_timeout_mode(u32 chan_id, u32 value)
{
    ODT_REG_SETBITS(ODT_REG_ENCDEST_BUFMODE_CFG(chan_id), 0x3, 1, value);
}

void odt_hal_set_timeout_mode(u32 value)
{
    ODT_REG_SETBITS(ODT_REG_GBLRST, 0x4, 1, value);
}

u32 odt_hal_get_src_debug_cfg(u32 chan_id)
{
    return ODT_REG_GETBITS(ODT_REG_ENCSRC_BUFCFG(chan_id), 0x1F, 1);
}


u32 odt_hal_get_tfr_modeswt_int_state(void)
{
    return ODT_REG_READ(ODT_REG_ENC_CORE0_INT0);
}

u32 odt_hal_get_tfr_modeswt_int_mask(void)
{
    return ODT_REG_READ(ODT_REG_ENC_CORE0_MASK0);
}

void odt_hal_set_tfr_int_mask(u32 value)
{
    ODT_REG_WRITE(ODT_REG_ENC_CORE0_MASK0, value);
}

u32 odt_hal_get_overflow_int_mask(void)
{
    return ODT_REG_READ(ODT_REG_ENC_CORE0_MASK2);
}

void odt_hal_set_overflow_int_mask(u32 value)
{
    ODT_REG_WRITE(ODT_REG_ENC_CORE0_MASK2, value);
}

void odt_hal_clear_tfr_raw_int(u32 value)
{
    ODT_REG_WRITE(ODT_REG_ENC_RAWINT0, value);
}

u32 odt_hal_get_overflow_int_state(void)
{
    return ODT_REG_READ(ODT_REG_ENC_CORE0_INTSTAT2);
}

u32 odt_hal_get_overflow_raw_int(void)
{
    return ODT_REG_READ(ODT_REG_ENC_RAWINT2);
}

void odt_hal_clear_overflow_raw_int(u32 value)
{
    ODT_REG_WRITE(ODT_REG_ENC_RAWINT2, value);
}

void odt_hal_set_modeswt_int_mask(u32 value)
{
    ODT_REG_SETBITS(ODT_REG_ENC_CORE0_MASK0, 0x10, 0x7, value);
}

void odt_hal_clear_modeswt_raw_int(u32 value)
{
    ODT_REG_SETBITS(ODT_REG_ENC_RAWINT0, 0x10, 0x7, value);
}

void odt_hal_set_single_chan_tfr_mask(u32 chan_id, u32 value)
{
    ODT_REG_SETBITS(ODT_REG_ENC_CORE0_MASK0, chan_id, 1, value);
}

void odt_hal_clear_single_chan_ovf_raw_int(u32 chan_id)
{
    ODT_REG_WRITE(ODT_REG_ENC_RAWINT2, 1 << chan_id);
}

void odt_hal_set_single_chan_ovf_mask(u32 chan_id, u32 value)
{
    ODT_REG_SETBITS(ODT_REG_ENC_CORE0_MASK2, chan_id, 1, value);
}

void odt_hal_clear_single_chan_thr_ovf_raw_int(u32 chan_id)
{
    ODT_REG_WRITE(ODT_REG_ENC_RAWINT2, 1 << (chan_id + 16)); /* 偏移在高16位 */
}

void odt_hal_set_single_chan_thr_ovf_mask(u32 chan_id, u32 value)
{
    ODT_REG_SETBITS(ODT_REG_ENC_CORE0_MASK2, chan_id + 0x10, 1, value);
}

u32 odt_hal_get_dst_chan_mode(u32 chan_id)
{
    return ODT_REG_GETBITS(ODT_REG_ENCDEST_BUFMODE_CFG(chan_id), 0x2, 1);
}

void odt_hal_set_dst_chan_mode(u32 chan_id, u32 value)
{
    ODT_REG_SETBITS(ODT_REG_ENCDEST_BUFMODE_CFG(chan_id), 1, 1, value);
}

void odt_hal_set_clk_ctrl(u32 value)
{
    ODT_REG_SETBITS(ODT_REG_CLKCTRL, 0, 1, value);
}


void odt_hal_set_dst_buffer_addr(u32 chan_id, u32 value1, u32 value2)
{
    ODT_REG_WRITE(ODT_REG_ENCDEST_BUFADDR_L(chan_id), value1);
    ODT_REG_WRITE(ODT_REG_ENCDEST_BUFADDR_H(chan_id), value2);
}
void odt_hal_set_dst_write_ptr(u32 chan_id, u32 value)
{
    ODT_REG_WRITE(ODT_REG_ENCDEST_BUFWPTR(chan_id), value);
}

void odt_hal_set_dst_read_ptr(u32 chan_id, u32 value)
{
    ODT_REG_WRITE(ODT_REG_ENCDEST_BUFRPTR(chan_id), value);
}

void odt_hal_set_dst_buffer_length(u32 chan_id, u32 value)
{
    ODT_REG_WRITE(ODT_REG_ENCDEST_BUFDEPTH(chan_id), value);
}

u32 odt_hal_get_dst_buffer_length(u32 chan_id)
{
    return ODT_REG_READ(ODT_REG_ENCDEST_BUFDEPTH(chan_id));
}

void odt_hal_set_dst_threshold_length(u32 chan_id, u32 value)
{
    ODT_REG_WRITE(ODT_REG_ENCDEST_BUFTHRESHOLD(chan_id), value);
}
void odt_hal_set_dst_arbitrate_length(u32 chan_id, u32 value)
{
    ODT_REG_WRITE(ODT_REG_ENCDEST_BUFTHRH(chan_id), value);
}

void odt_hal_set_dst_chan_start_stop(u32 chan_id, u32 value)
{
    ODT_REG_SETBITS(ODT_REG_ENCDEST_BUFMODE_CFG(chan_id), 0, 1, value);
}

void odt_hal_set_src_buffer_addr(u32 chan_id, u32 addr_low, u32 addr_high)
{
    ODT_REG_WRITE(ODT_REG_ENCSRC_BUFADDR_L(chan_id), addr_low);
    ODT_REG_WRITE(ODT_REG_ENCSRC_BUFADDR_H(chan_id), addr_high);
}

u32 odt_hal_get_dst_read_ptr(u32 chan_id)
{
    return ODT_REG_READ(ODT_REG_ENCDEST_BUFRPTR(chan_id));
}

u32 odt_hal_get_dst_write_ptr(u32 chan_id)
{
    return ODT_REG_READ(ODT_REG_ENCDEST_BUFWPTR(chan_id));
}

void odt_hal_set_src_rd_buffer_addr(u32 chan_id, u32 addr_low, u32 addr_high)
{
    ODT_REG_WRITE(ODT_REG_ENCSRC_RDQADDR_L(chan_id), addr_low);
    ODT_REG_WRITE(ODT_REG_ENCSRC_RDQADDR_H(chan_id), addr_high);
}

void odt_hal_set_src_write_ptr(u32 chan_id, u32 value)
{
    ODT_REG_WRITE(ODT_REG_ENCSRC_BUFWPTR(chan_id), value);
}

void odt_hal_set_src_read_ptr(u32 chan_id, u32 value)
{
    ODT_REG_WRITE(ODT_REG_ENCSRC_BUFRPTR(chan_id), value);
}

void odt_hal_set_src_rd_write_ptr(u32 chan_id, u32 value)
{
    ODT_REG_WRITE(ODT_REG_ENCSRC_RDQWPTR(chan_id), value);
}

void odt_hal_set_src_rd_read_ptr(u32 chan_id, u32 value)
{
    ODT_REG_WRITE(ODT_REG_ENCSRC_RDQRPTR(chan_id), value);
}


void odt_hal_set_dst_chan_id(u32 chan_id, u32 value)
{
    ODT_REG_SETBITS(ODT_REG_ENCSRC_BUFCFG(chan_id), 0x4, 0x2, value);
}

void odt_hal_set_src_buffer_length(u32 chan_id, u32 value)
{
    ODT_REG_WRITE(ODT_REG_ENCSRC_BUFDEPTH(chan_id), value);
}

void odt_hal_set_src_rd_buffer_length(u32 chan_id, u32 value)
{
    ODT_REG_SETBITS(ODT_REG_ENCSRC_RDQCFG(chan_id), 0, 0x10, value);
    ODT_REG_SETBITS(ODT_REG_ENCSRC_RDQCFG(chan_id), 0x10, 0x10, 0);
}


void odt_hal_set_src_rd_wptr_addr(u32 chan_id, u32 addr_low, u32 addr_high)
{
    ODT_REG_WRITE(ODT_REG_ENCSRC_RDQ_WPTRIMG_L(chan_id), addr_low);
    ODT_REG_WRITE(ODT_REG_ENCSRC_RDQ_WPTRIMG_H(chan_id), addr_high);
}

void odt_hal_set_src_chan_start_stop(u32 chan_id, u32 value)
{
    ODT_REG_SETBITS(ODT_REG_ENCSRC_BUFCFG(chan_id), 0, 1, value);
}


u32 odt_hal_get_src_write_ptr(u32 chan_id)
{
    return ODT_REG_READ(ODT_REG_ENCSRC_BUFWPTR(chan_id));
}

u32 odt_hal_get_src_read_ptr(u32 chan_id)
{
    return ODT_REG_READ(ODT_REG_ENCSRC_BUFRPTR(chan_id));
}

u32 odt_hal_get_src_rd_write_ptr(u32 chan_id)
{
    return ODT_REG_READ(ODT_REG_ENCSRC_RDQWPTR(chan_id));
}

u32 odt_hal_get_src_start_stop_state(u32 chan_id)
{
    return ODT_REG_GETBITS(ODT_REG_ENCSRC_BUFCFG(chan_id), 0, 1);
}

u32 odt_hal_get_global_int_state(void)
{
    return ODT_REG_READ(ODT_REG_GBL_INTSTAT);
}

void odt_hal_set_global_halt(u32 value)
{
    ODT_REG_SETBITS(ODT_REG_GBLRST, 0x10, 1, value);
}


