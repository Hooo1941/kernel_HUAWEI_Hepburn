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

#ifndef __MDRV_PARTITION_H__
#define __MDRV_PARTITION_H__

/**
 * @brief 分区名长度最大值
 */
#define PART_NAMELEN                    16

/**
 * @brief 一级分区表列表元素
 */
struct nand_part {
    char name[PART_NAMELEN];
    unsigned int start;
    unsigned int length;
    unsigned int has_second; /* *<此一级分区是否包含二级分区标记 */
};
/**
 * @brief 二级分区表列表元素
 */
struct nand_second_part {
    char name[PART_NAMELEN];
    unsigned int length;
};
/**
 * @brief ubi设备列表元素
 */
struct ubi_dev_part {
    char name[PART_NAMELEN];
    short dev_id;
    short badblock_per1024; /* *<每1024个物理块预期最大坏块个数，与内核原生attach流程参数含义一致 */
};
/**
 * @brief ubi卷列表元素
 */
struct ubi_vol_part {
    char name[PART_NAMELEN];
    short vol_id;
    unsigned short is_block_flag; /* *<是否需要创建ubi块设备标记 */
};

/**
 * @brief 获取一级分区个数
 *
 * @attention
 * <ul><li>仅用于nand启动场景，emmc启动场景不支持</li></ul>
 * <ul><li>如果ptable_buf为空，则基于当前系统中分区表返回所需信息</li></ul>
 *
 * @param[in]  ptable_buf, 分区表数据buffer，不包含安全校验头，buffer属于内核态
 * @param[in]  ptable_buf_size, 分区表数据buffer大小
 * @param[out]  part_num, 一级分区个数
 *
 * @retval 0,表示操作成功。
 * @retval !=0,表示操作失败。
 */
int mdrv_partition_get_part_num(unsigned int *part_num, const char *ptable_buf, unsigned int ptable_buf_size);

/**
 * @brief 获取一级分区列表
 *
 * @attention
 * <ul><li>仅用于nand启动场景，emmc启动场景不支持</li></ul>
 * <ul><li>如果ptable_buf为空，则基于当前系统中分区表返回所需信息</li></ul>
 *
 * @param[in]  ptable_buf, 分区表数据buffer，不包含安全校验头，buffer属于内核态
 * @param[in]  ptable_buf_size, 分区表数据buffer大小
 * @param[in]  part, 一级分区列表地址，由用户申请空间，本接口将信息填充
 * @param[in]  part_num, 一级分区个数
 *
 * @retval 0,表示操作成功。
 * @retval !=0,表示操作失败。
 */
int mdrv_partition_get_part_list(struct nand_part *part, unsigned int part_num, const char *ptable_buf, unsigned int ptable_buf_size);

/**
 * @brief 获取所属一级分区的二级分区个数
 *
 * @attention
 * <ul><li>仅用于nand启动场景，emmc启动场景不支持</li></ul>
 * <ul><li>如果ptable_buf为空，则基于当前系统中分区表返回所需信息</li></ul>
 *
 * @param[in]  first_part_name, 一级分区名
 * @param[in]  ptable_buf, 分区表数据buffer，不包含安全校验头，buffer属于内核态
 * @param[in]  ptable_buf_size, 分区表数据buffer大小
 * @param[out]  second_part_num, 二级分区个数
 *
 * @retval 0,表示操作成功。
 * @retval !=0,表示操作失败。
 */
int mdrv_partition_get_second_part_num(const char* first_part_name, unsigned int *second_part_num, const char *ptable_buf, unsigned int ptable_buf_size);
/**
 * @brief 获取所属一级分区的二级分区列表
 *
 * @attention
 * <ul><li>仅用于nand启动场景，emmc启动场景不支持</li></ul>
 * <ul><li>如果ptable_buf为空，则基于当前系统中分区表返回所需信息</li></ul>
 *
 * @param[in]  first_part_name, 一级分区名
 * @param[in]  second_part, 二级分区列表地址，由用户申请空间，本接口将信息填充
 * @param[in]  second_part_num, 二级分区个数
 * @param[in]  ptable_buf, 分区表数据buffer，不包含安全校验头，buffer属于内核态
 * @param[in]  ptable_buf_size, 分区表数据buffer大小
 *
 * @retval 0,表示操作成功。
 * @retval !=0,表示操作失败。
 */
int mdrv_partition_get_second_part_list(const char* first_part_name, struct nand_second_part *second_part, unsigned int second_part_num, const char *ptable_buf, unsigned int ptable_buf_size);
/**
 * @brief 获取ubi附加信息中的ubi设备个数
 *
 * @attention
 * <ul><li>仅用于nand启动场景，emmc启动场景不支持</li></ul>
 * <ul><li>如果ptable_buf为空，则基于当前系统中分区表返回所需信息</li></ul>
 *
 * @param[in]  ptable_buf, 分区表数据buffer，不包含安全校验头，buffer属于内核态
 * @param[in]  ptable_buf_size, 分区表数据buffer大小
 * @param[out]  ubi_dev_num, ubi设备个数
 *
 * @retval 0,表示操作成功。
 * @retval !=0,表示操作失败。
 */
int mdrv_partition_get_ubi_dev_num(unsigned int *ubi_dev_num, const char *ptable_buf, unsigned int ptable_buf_size);
/**
 * @brief 获取ubi设备列表
 *
 * @attention
 * <ul><li>仅用于nand启动场景，emmc启动场景不支持</li></ul>
 * <ul><li>如果ptable_buf为空，则基于当前系统中分区表返回所需信息</li></ul>
 *
 * @param[in]  ubi_dev_part, ubi设备列表地址，由用户申请空间，本接口将信息填充
 * @param[in]  ubi_dev_num, ubi设个数
 * @param[in]  ptable_buf, 分区表数据buffer，不包含安全校验头，buffer属于内核态
 * @param[in]  ptable_buf_size, 分区表数据buffer大小
 *
 * @retval 0,表示操作成功。
 * @retval !=0,表示操作失败。
 */
int mdrv_partition_get_ubi_dev_part_list(struct ubi_dev_part *ubi_dev_part, unsigned int ubi_dev_num, const char *ptable_buf, unsigned int ptable_buf_size);
/**
 * @brief 获取ubi附加信息中所属ubi一级分区的ubi卷个数
 *
 * @attention
 * <ul><li>仅用于nand启动场景，emmc启动场景不支持</li></ul>
 * <ul><li>如果ptable_buf为空，则基于当前系统中分区表返回所需信息</li></ul>
 *
 * @param[in]  first_part_name, 一级分区名
 * @param[in]  ptable_buf, 分区表数据buffer，不包含安全校验头，buffer属于内核态
 * @param[in]  ptable_buf_size, 分区表数据buffer大小
 * @param[out]  ubi_vol_num, 卷个数
 *
 * @retval 0,表示操作成功。
 * @retval !=0,表示操作失败。
 */
int mdrv_partition_get_ubi_vol_num(const char* first_part_name, unsigned int *ubi_vol_num, const char *ptable_buf, unsigned int ptable_buf_size);

/**
 * @brief 获取所属ubi一级分区的ubi卷列表
 *
 * @attention
 * <ul><li>仅用于nand启动场景，emmc启动场景不支持</li></ul>
 * <ul><li>如果ptable_buf为空，则基于当前系统中分区表返回所需信息</li></ul>
 *
 * @param[in]  first_part_name, 一级分区名
 * @param[in]  ubi_vol_part, ubi卷列表地址，由用户申请空间，本接口将信息填充
 * @param[in]  ubi_vol_num, ubi卷个数
 * @param[in]  ptable_buf, 分区表数据buffer，不包含安全校验头，buffer属于内核态
 * @param[in]  ptable_buf_size, 分区表数据buffer大小
 *
 * @retval 0,表示操作成功。
 * @retval !=0,表示操作失败。
 */
int mdrv_partition_get_ubi_vol_part_list(char* first_part_name, struct ubi_vol_part *ubi_vol_part, unsigned int ubi_vol_num, const char *ptable_buf, unsigned int ptable_buf_size);


/* 升级时更新分区表，不包含重新创建mtd设备 */
/**
 * @brief 更新海思内存中分区表
 *
 * @attention
 * <ul><li>仅用于nand启动场景，emmc启动场景不支持</li></ul>
 * <ul><li>仅更新内存分区表，不包含重新创建mtd设备</li></ul>
 * <ul><li>不可重入</li></ul>
 *
 * @param[in]  ptable_buf, 升级后ptable镜像buffer，不包含安全校验头，buffer属于内核态
 * @param[in]  ptable_buf_size, 升级后ptable镜像buffer大小
 *
 * @retval 0,表示操作成功。
 * @retval !0,表示操作失败。
 */
int mdrv_partition_flush_mem(const char *ptable_buf, unsigned int ptable_buf_size);

#endif
