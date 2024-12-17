/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
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

#ifndef __MDRV_MEMORY_LAYOUT_H__
#define __MDRV_MEMORY_LAYOUT_H__

/**
 * @brief 获取DDR内存布局
 * @par 描述:
 * 获取DDR内存布局
 * @param[in] name: 要获取的DDR内存段名称（名称在DTS中注册）name长度不超过16字节
 * ------名称-------------说明--------------
 * "mntn_ddr"         维测内存空间
 * "mdm_dts_ddr"      modem DTB空间
 * "mdm_ddr"          modem运行空间
 * "odt_bbpds_ddr"    odt数采空间
 * "rf_sub6g_ddr"     RFIC sub6g镜像
 * "rf_hf_ddr"        RFIC高频镜像
 * "pde_ddr"          PDE镜像空间
 * "share_nsro_ddr"   非安全只读不可写共享内存
 * "share_unsec_ddr"  非安全共享内存
 * "share_sec_ddr"    安全共享内存
 * "tvp_ddr"          TVP运行空间
 * -----------------------------------------
 * @param[in] size: ddr内存段的大小
 * @retval 0 申请失败
 * @retval 其它 返回DDR内存段的物理地址
 */
unsigned long mdrv_mem_region_get(const char *name, unsigned int *size);

#endif /* __MDRV_MEMORY_LAYOUT_H__ */ 
