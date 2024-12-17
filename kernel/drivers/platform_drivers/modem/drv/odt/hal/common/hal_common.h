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

#ifndef _ODT_HAL_COMMON_H
#define _ODT_HAL_COMMON_H

#include <osl_types.h>
#include <osl_bio.h>
#include <drv_comm.h>
#include <bsp_dt.h>
#include <bsp_odt.h>
#include "odt_hal.h"


#ifdef __cplusplus
extern "C" {
#endif

#undef THIS_MODU
#define THIS_MODU mod_odt

#define odt_crit(fmt, ...) bsp_err(fmt, ##__VA_ARGS__)
#define odt_error(fmt, ...) bsp_err("<%s %d>" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define odt_info(fmt, ...) bsp_info("<%s %d>" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

extern odt_hal_info_s g_odt_hal_info;
#define ODT_REG_WRITE(reg, data) (writel((data), (g_odt_hal_info.base_addr + (reg))))
#define ODT_REG_READ(reg) (readl(g_odt_hal_info.base_addr + (reg)))
#define ODT_REG_WRITE_RELAXED(reg, data) (writel_relaxed(data, (g_odt_hal_info.base_addr + (reg))))
#define ODT_REG_READ_RELAXED(reg, result) ((result) = readl_relaxed(g_odt_hal_info.base_addr + (reg)))
#define ODT_REG_SETBITS(reg, pos, bits, val) BSP_REG_SETBITS(g_odt_hal_info.base_addr, reg, pos, bits, val)
#define ODT_REG_GETBITS(reg, pos, bits) BSP_REG_GETBITS(g_odt_hal_info.base_addr, reg, pos, bits)

s32 odt_hal_init(void);
device_node_s *odt_hal_get_dev_info(void);
void odt_hal_set_rate_ctrl_threshold(u32 rate_limits);
void odt_hal_set_timeout_threshold(u32 mode, u32 timeout);
void odt_hal_set_clk_ctrl(u32 value);
void odt_hal_global_reset(void);
u32 odt_hal_get_global_reset_state(void);
u32 odt_hal_get_timeout_mode(void);
void odt_hal_set_timeout_mode(u32 value);
void odt_hal_unmask_header_error_int(void);
u32 odt_hal_get_src_debug_cfg(u32 chan_id);
void odt_hal_set_header_error_int_mask(u32 chan_id, u32 value);
void odt_hal_clear_header_error_raw_int(u32 low_chan, u32 high_chan);
void odt_hal_get_header_error_int_state(u32 *low_chan, u32 *high_chan);
u32 odt_hal_get_tfr_modeswt_int_state(void);
u32 odt_hal_get_tfr_modeswt_int_mask(void);
void odt_hal_set_tfr_int_mask(u32 value);
u32 odt_hal_get_overflow_int_mask(void);
void odt_hal_set_overflow_int_mask(u32 value);
void odt_hal_clear_tfr_raw_int(u32 value);
u32 odt_hal_get_overflow_int_state(void);
void odt_hal_clear_overflow_raw_int(u32 value);
u32 odt_hal_get_overflow_raw_int(void);
void odt_hal_set_modeswt_int_mask(u32 value);
void odt_hal_clear_modeswt_raw_int(u32 value);
u32 odt_hal_get_dst_chan_mode(u32 chan_id);
void odt_hal_set_single_chan_tfr_mask(u32 chan_id, u32 value);
void odt_hal_clear_single_chan_ovf_raw_int(u32 chan_id);
void odt_hal_set_single_chan_ovf_mask(u32 chan_id, u32 value);
void odt_hal_clear_single_chan_thr_ovf_raw_int(u32 chan_id);
void odt_hal_set_single_chan_thr_ovf_mask(u32 chan_id, u32 value);
void odt_hal_set_src_buffer_addr(u32 chan_id, u32 addr_low, u32 addr_high);
void odt_hal_set_src_rd_buffer_addr(u32 chan_id, u32 addr_low, u32 addr_high);
void odt_hal_set_src_write_ptr(u32 chan_id, u32 value);
void odt_hal_set_src_read_ptr(u32 chan_id, u32 value);
void odt_hal_set_src_rd_write_ptr(u32 chan_id, u32 value);
void odt_hal_set_src_rd_read_ptr(u32 chan_id, u32 value);
void odt_hal_set_src_buffer_length(u32 chan_id, u32 value);
void odt_hal_set_src_rd_buffer_length(u32 chan_id, u32 value);
void odt_hal_set_rptr_img_addr(u32 chan_id, u32 addr_low, u32 addr_high);
void odt_hal_set_src_rd_timeout(u32 chan_id, u32 value);
void odt_hal_set_src_rd_wptr_addr(u32 chan_id, u32 addr_low, u32 addr_high);
void odt_hal_set_rate_ctrl_group(u32 chan_id, u32 value);
void odt_hal_set_src_chan_start_stop(u32 chan_id, u32 value);
void odt_hal_clear_src_chan_cfg(u32 chan_id);
void odt_hal_clear_rate_ctrl_cnt(u32 chan_id);
void odt_hal_clear_src_rd_raw_int(u32 chan_id);
u32 odt_hal_get_src_write_ptr(u32 chan_id);
u32 odt_hal_get_src_read_ptr(u32 chan_id);
u32 odt_hal_get_src_rd_write_ptr(u32 chan_id);
u32 odt_hal_get_src_start_stop_state(u32 chan_id);
u32 odt_hal_get_src_state(void);
u32 odt_hal_get_single_src_state(u32 chan_id);
s32 odt_hal_get_rate_ctrl_cnt(u32 chan_id);
u32 odt_hal_get_global_int_state(void);
void odt_hal_set_global_halt(u32 value);
void odt_hal_set_single_src_rd_timeout_int_mask(u32 chan_id, u32 value);
void odt_hal_set_single_src_rd_int_mask(u32 chan_id, u32 value);
void odt_hal_src_chan_init(u32 chan_id, odt_src_chan_cfg_s *src_attr);
u32 odt_hal_get_dst_buffer_length(u32 chan_id);
void odt_hal_set_dst_threshold_length(u32 chan_id, u32 value);
void odt_hal_set_dst_arbitrate_length(u32 chan_id, u32 value);
void odt_hal_set_dst_chan_start_stop(u32 chan_id, u32 value);
void odt_hal_set_rate_ctrl_cfg(u32 value1, u32 value2);
void odt_hal_set_rate_ctrl_peroid(u32 value);
void odt_hal_set_dst_write_ptr(u32 chan_id, u32 value);
void odt_hal_set_dst_read_ptr(u32 chan_id, u32 value);
void odt_hal_set_dst_buffer_length(u32 chan_id, u32 value);
void odt_hal_set_single_chan_timeout_mode(u32 chan_id, u32 value);
u32 odt_hal_get_dst_read_ptr(u32 chan_id);
u32 odt_hal_get_dst_write_ptr(u32 chan_id);
void odt_hal_set_dst_chan_trans_id_en(u32 chan_id, u32 value);
void odt_hal_set_dst_chan_mode(u32 chan_id, u32 value);
void odt_hal_set_dst_buffer_addr(u32 chan_id, u32 value1, u32 value2);
void odt_hal_clear_src_rd_raw_int(u32 chan_id);
void odt_hal_clear_src_rd_timout_raw_int(u32 chan_id);


#ifdef __cplusplus
}
#endif

#endif
