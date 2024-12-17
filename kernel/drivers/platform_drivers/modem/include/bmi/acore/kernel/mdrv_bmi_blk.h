/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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

#ifndef __MDRV_BMI_BLK_H__
#define __MDRV_BMI_BLK_H__


/**
 * @brief 读flash接口
 *
 * @param[in]  part_name ,待读分区名
 * @param[in]  part_offset ,待读数据在flash part_name所在分区中的偏移
 * @param[in]  data_buf ,读flash的目的buffer地址
 * @param[in]  data_len ,读取长度
 *
 * @retval 0,表示操作成功。
 * @retval !=0,表示操作失败。
 */
int mdrv_blk_read(const char *part_name, unsigned long long part_offset, void *data_buf, unsigned long long data_len);
/**
 * @brief 写flash接口
 *
 * @param[in]  part_name ,待写分区名
 * @param[in]  part_offset ,待写数据在flash part_name所在分区中的偏移
 * @param[in]  data_buf ,写flash的目的buffer地址
 * @param[in]  data_len ,写取长度
 *
 * @retval 0,表示操作成功。
 * @retval !=0,表示操作失败。
 */
int mdrv_blk_write(const char *part_name, unsigned long long part_offset, void *data_buf, unsigned long long data_len);
/**
 * @brief 根据分区名擦flash接口
 *
 * @attention
 * <ul><li>坏块不擦除;</li></ul>
 *
 * @param[in]  part_name ,待擦分区名
 *
 * @retval 0,表示操作成功。
 * @retval !=0,表示操作失败。
 */
int mdrv_blk_erase(const char *part_name);
/**
 * @brief 查询某个flash块是否为坏块
 *
 * @attention
 * <ul><li>part_offset要求块对齐;</li></ul>
 *
 * @param[in]  part_name ,待擦分区名
 * @param[in]  part_offset ,待查块在分区内flash绝对偏移
 *
 * @retval >0,表示坏块。
 * @retval 0,表示好块。
 * @retval <0,表示操作失败。
 */
int mdrv_blk_isbad(const char *part_name, unsigned long long part_offset);
/**
 * @brief logic block层错误码
 */
#define BLK_LOGIC_NO_VALID_DATA (-10) /**<读操作时，该逻辑块没有对应的有效物理块数据，常见于没有写过有效数据就直接读的场景 */
#define BLK_LOGIC_CHECK_VALID_FAIL (-11) /**<读操作时，此逻辑块对应的物理块为坏块，或者header校验不合法 */
#define BLK_LOGIC_GET_MAP_ERR (-12) /**<读写操作时， 无法创建或获取映射表信息 */
#define BLK_LOGIC_PARAM_ERR (-13) /**<读写接口入参不合法 */
#define BLK_LOGIC_ALLOC_ERR (-14) /**<写操作时，申请临时buffer失败 */
#define BLK_LOGIC_FIND_NEW_PB_ERR (-15) /**<写操作时，无法找到空闲的物理块 */
#define BLK_LOGIC_WRITE_PREPARE_ERR (-16) /**<写操作时，准备待写数据失败 */
#define BLK_LOGIG_OPEN_PART_ERR (-17) /**<读写操作时， 获取底层存储介质信息失败 */
/**
 * @brief 逻辑分区读
 * @attention
 * <ul><li>待读数据长度不能超过目的buffer大小</li></ul>
 *
 * @param[in] part_name, 分区名
 * @param[in] part_offset, 此次读操作在逻辑分区中的起始偏移
 * @param[in] data_buf, 待读数据目的地址
 * @param[in] data_len, 待读数据长度
 *
 * @retval 0，读成功。
 * @retval 非0，读失败，默认返回-1，特殊错误详见logic block层错误码。
 */
int mdrv_blk_logic_read(const char *part_name, unsigned long long part_offset, void *data_buf, unsigned long long data_len);
/**
 * @brief 逻辑分区数据写到flash
 * @attention
 * <ul><li>此接口返回时会将数据写到flash中保存，但在执行此接口过程中异常掉电，此次写入的数据不能保证刷到flash中</li></ul>
 *
 * @param[in] part_name, 分区名
 * @param[in] part_offset, 此次写操作在逻辑分区中的起始偏移
 * @param[in] data_buf, 待写数据源地址
 * @param[in] data_len, 待写数据长度
 *
 * @retval 0，写成功。
 * @retval 非0，写失败，默认返回-1，特殊错误详见logic block层错误码。
 */
int mdrv_blk_logic_write(const char *part_name, unsigned long long part_offset, void *data_buf, unsigned long long data_len);
#endif
