/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2023. All rights reserved.
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

#ifndef __MDRV_BMI_OEMINFO_H__
#define __MDRV_BMI_OEMINFO_H__

/*****************************************************************************
     *                           Attention                           *
 * 不会对oeminfo分区保存的数据进行签名、加密，保存的数据不提供安全性保证，
 * 因此oeminfo分区不能存放涉及机密、隐私等需要签名或加密的数据
 ******************************************************************************/

/**
 * @brief oeminfo comm区域id枚举
 *
 */
enum oeminfo_comm_rgn_id {
    UPDATE_COMPLETE_FLAG = 0, /**< 对应结构体struct oeminfo_update_complete_flag， 结构体定义在mdrv_bmi_update.h */
    BOOT_PART_FLAG, /**< 对应结构体struct oeminfo_boot_part_flag， 结构体定义在mdrv_bmi_update.h */
    OEMINFO_COMM_RGN_MAX,
};

/**
 * @brief oeminfo数据单元由data_head+data构成，struct oeminfo_data_head为data_head结构
 */
struct oeminfo_data_head {
    unsigned int total_bytes; /**< oeminfo数据单元中data部分字节数 */
    unsigned int crc; /**< oeminfo数据单元中data部分crc16结果 */
    unsigned int rsv[2];
};

/**
 * @brief 将ddr缓冲中保存的全部数据刷新到flash中
 *
 * @retval 0，刷新成功。
 * @retval 非0，刷新失败。
 */
int mdrv_oeminfo_sync(void);

/**
 * @brief 设置oeminfo cust区域大小
 * @attention
 * <ul><li>此接口需要在oeminfo驱动初始化之前调用，否则cust区域大小使用默认值</li></ul>
 *
 * @param[in] size, 待修改的cust区域大小
 *
 * @retval 0，执行成功。
 * @retval 非0，执行失败。
 */
int mdrv_oeminfo_set_cust_rgn_size(unsigned int size);

/**
 * @brief 读取oeminfo cust区域信息
 *
 * @param[in] offset, 此次读操作在cust区域的偏移，用户计算offset时需要包含struct oeminfo_data_head结构体大小
 * @param[out] buf, 待读数据目的地址
 * @param[in] buf_len, 待读数据长度
 *
 * @retval 0，读成功。
 * @retval 非0，读失败。
 */
int mdrv_oeminfo_cust_rgn_read(unsigned int offset, void *buf, unsigned int buf_len);

/**
 * @brief 写oeminfo cust区域信息到DDR缓存
 * @attention
 * <ul><li>此接口返回时会将数据写到ddr中保存，不会刷新到flash中，需要有用户显示调用mdrv_oeminfo_sync才能将数据刷到flash中，但如果运行中异常掉电，此次写入的数据不能保证刷到ddr、flash中</li></ul>
 *
 * @param[in] offset, 此次写操作在cust区域的偏移，用户计算offset时需要包含struct oeminfo_data_head结构体大小
 * @param[in] buf, 待写数据源地址
 * @param[in] buf_len, 待写数据长度
 *
 * @retval 0，写成功。
 * @retval 非0，写失败。
 */
int mdrv_oeminfo_cust_rgn_write_async(unsigned int offset, const void *buf, unsigned int buf_len);

/**
 * @brief 写oeminfo cust区域信息到DDR缓冲、flash
 * @attention
 * <ul><li>此接口会先将数据刷新到ddr，然后再将数据写到flash中保存，但在执行此接口过程中异常掉电，此次写入的数据不能保证刷到ddr、flash中</li></ul>
 *
 * @param[in] offset, 此次写操作在cust区域的偏移，用户计算offset时需要包含struct oeminfo_data_head结构体大小
 * @param[in] buf, 待写数据源地址
 * @param[in] buf_len, 待写数据长度
 *
 * @retval 0，写成功。
 * @retval 非0，写失败。
 */
int mdrv_oeminfo_cust_rgn_write_sync(unsigned int offset, const void *buf, unsigned int buf_len);

/**
 * @brief 读取oeminfo comm区域信息
 *
 * @param[in] idx, 待读取信息的索引；
 * @param[out] buf, 待读数据目的地址
 * @param[in] buf_len, 待读数据长度
 *
 * @retval 0，读成功。
 * @retval 非0，读失败。
 */
int mdrv_oeminfo_comm_rgn_read(enum oeminfo_comm_rgn_id idx, void *buf, unsigned int buf_len);

/**
 * @brief 写oeminfo comm区域信息到DDR缓存
 * @attention
 * <ul><li>此接口返回时会将数据写到ddr中保存，需要有用户显示调用mdrv_oeminfo_sync才能将数据刷到flash中，但如果运行中异常掉电，此次写入的数据不能保证刷到ddr、flash中</li></ul>
 *
 * @param[in] idx, 待写入信息的索引
 * @param[in] buf, 待写数据源地址
 * @param[in] buf_len, 待写数据长度，不能超过240Bytes
 *
 * @retval 0，写成功。
 * @retval 非0，写失败。
 */
int mdrv_oeminfo_comm_rgn_write_async(enum oeminfo_comm_rgn_id idx, void *buf, unsigned int buf_len);

/**
 * @brief 写oeminfo comm区域信息到flash
 * @attention
 * <ul><li>此接口会先将数据刷新到ddr，然后再将数据写到flash中保存，但在执行此接口过程中异常掉电，此次写入的数据不能保证刷到ddr、flash中</li></ul>
 *
 * @param[in] idx, 待写入信息的索引
 * @param[in] buf, 待写数据源地址
 * @param[in] buf_len, 待写数据长度，不能超过240Bytes
 *
 * @retval 0，写成功。
 * @retval 非0，写失败。
 */
int mdrv_oeminfo_comm_rgn_write_sync(enum oeminfo_comm_rgn_id idx, void *buf, unsigned int buf_len);

#endif
