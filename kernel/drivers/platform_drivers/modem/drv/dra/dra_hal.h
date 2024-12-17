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

#ifndef __DRA_HAL_H__
#define __DRA_HAL_H__
#include "dra.h"
#include "bsp_dra.h"

void __dra_release_pool_init(struct dra *dra);
void dra_fifo_init(struct dra *dra, struct dra_fifo *fifo, int level);
void dra_force_fifo_full(struct dra *dra, struct dra_fifo *fifo, int level);
int dra_ipipe_init(struct dra *dra, struct dra_ipipe *ipipe, unsigned int idx);
void dra_interupt_init(struct dra *dra);
void dra_ipipe_read_update(struct dra *dra, unsigned int rptr, unsigned int idx);
void dra_input(struct dra *dra, unsigned long long buf, struct dra_opipe *opipe);
void dra_bigpool_input(struct dra *dra, unsigned long long buf, struct dra_opipe *opipe);
unsigned int dra_get_space(struct dra *dra, struct dra_opipe *opipe);
unsigned int dra_get_intr_status(struct dra *dra);
unsigned int dra_get_busy_cnt(struct dra *dra, struct dra_opipe *opipe);
unsigned int dra_get_rls(struct dra *dra);
void dra_set_rls(struct dra *dra, unsigned int rptr);
void dra_set_intr(struct dra *dra);
void dra_top_intr(struct dra *dra);
void dra_basic_cfg(struct dra *dra);
int dra_hal_suspend(struct dra *dra);
int dra_hal_resume(struct dra *dra);
void dra_detach(struct dra *dra);
void dra_fifo_reinit(struct dra *dra, struct dra_fifo *fifo, int level);
void dra_hds_transreport_hal(struct dra *dra, struct dra_debug_info *dbg_info);
void dra_ddr_buf_fifo_init(struct dra *dra, struct dra_fifo *fifo, unsigned int level);
unsigned int dra_hal_set_adq(struct dra *dra, unsigned int level, unsigned long long adqbase, unsigned int write,
    unsigned int depth, unsigned int ipipe);
unsigned int dra_hal_set_rls(struct dra *dra, unsigned long long rlsbase, unsigned int update_ptr,
    unsigned int depth, struct dra_rls_para *rls_para);
void dra_set_backup_regs(struct dra *dra);
void dra_rls_fifo_init(struct dra *dra, struct dra_rls_ch *rls, unsigned int level);
void dra_rls_input(struct dra *dra, unsigned long long buf, struct dra_rls_ch *rls, unsigned int level);
void dra_get_big_pool_ptr(struct dra *dra, struct dra_opipe *opipe, unsigned int *wptr, unsigned int *rptr);
int dra_enable_alloc_ch(struct dra *dra, unsigned int idx, int enable);
int dra_enable_rls_ch(struct dra *dra, unsigned int idx, int enable);

#endif
