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

/**
 *  @brief   飞龙架构2.0 nva模块对外头文件
 *  @file    mdrv_bmi_nva.h
 *  @version v1.0
 *  @date    2023.07.25
 *  @note    该文件增删改需通过接口变更评审
 *  <ul><li>v1.0|2023.07.25|创建文件</li></ul>
 *  @since
 */

#ifndef _MDRV_BMI_NVA_H_
#define _MDRV_BMI_NVA_H_

#ifdef __cplusplus
extern "C" {
#endif
/**
 *  @brief   nva 模块在acore上的对外头文件
 *  @file    mdrv_nva.h
 *  @version v1.0
 *  @date    2023.06.12
 *  @note    修改记录(版本号|修订日期|说明)
 * <ul><li>v1.0|2023.06.12|创建文件</li></ul>
 *  @since
 */

/**
 * @brief 初始化状态.
 */
enum {
    NVA_INIT_FAIL = 0x0,
    NVA_INIT_OK = 0x1,
};

/**
 * @brief 接口返回错误码.
 */
enum {
    NVA_OK,
    ERROR_FAIL,         /* NVA模块错误 */
    ERROR_PARAM,        /* 入参错误 */
    ERROR_NOT_INIT,     /* 模块未初始化 */
    ERROR_RESOURCE,     /* 申请内存失败 */
    ERROR_NOT_EXIST,    /* 对应ID不存在 */
    ERROR_FILE_WRITE,   /* 写分区失败 */
    ERROR_HASH,         /* 计算hash失败 */
    ERROR_HASH_FINISH,  /* 计算hmac失败 */
    ERROR_HASH_CHECK,   /* hmac不匹配 */
    ERROR_NO_WRITE,     /* 对应ID没有写属性 */
    ERROR_DATA_LEN,     /* 输入len error */
    ERROR_NO_PARTITION, /* 分区不存在 */
    ERROR_NO_BLK_WR,    /* 不支持block写 */
};

struct nva_flushbk_cb {
    int (*flushbk)(const char *nva_bin, unsigned int size);
};

/**
 * @brief nva 读接口
 * @par 描述:
 * nva 读接口
 * @param[in] item_id: 要读取的nva id
 * @param[in] offset: 对应nva项的偏移，单位字节
 * @param[in] data: 读取的nva项内存
 * @param[in] len: 要读取的长度，offset + len不能超过nva项的整体长度
 * @retval 参见错误码
 */
int mdrv_nva_read(unsigned int item_id, int offset, void *data, unsigned int len);

/**
 * @brief nva 写接口
 * @par 描述:
 * nva 写接口
 * @param[in] item_id: 要读取的nva id
 * @param[in] offset: 对应nva项的偏移，单位字节
 * @param[in] data: 要写的nva项内存
 * @param[in] len: 要写的长度，offset + len不能超过nva项的整体长度
 * @attention 只能针对XML中RW属性配置为1的NVA项进行写
 * @retval 参见错误码
 */
int mdrv_nva_write(unsigned int item_id, int offset, void *data, unsigned int len);

/**
 * @brief nva 调试写接口
 * @par 描述:
 * nva 调试写接口
 * @param[in] item_id: 要读取的nva id
 * @param[in] offset: 对应nva项的偏移，单位字节
 * @param[in] data: 要写的nva项内存
 * @param[in] len: 要写的长度，offset + len不能超过nva项的整体长度
 * @attention 可以忽略NVA的RW属性进行写，只能给受控的调试接口调用
 * @retval 参见错误码
 */
int mdrv_nva_write_om(unsigned int id, int offset, void *data, unsigned int len);

/**
 * @brief nva 更新备份分区
 * @par 描述:
 * nva 更新备份分区，用户根据需求将工作区数据备份到备份分区
 * @retval 参见错误码:
 *                    ERROR_FILE_WRITE-6：写分区失败
 *                    ERROR_HASH_CHECK-9：hmac不匹配
 *                    ERROR_NO_PARTITION-12:当前分区不存在
 *                    ERROR_NO_BLK_WR-13：不支持block写
 */
int mdrv_nva_update_backup(void);

/**
 * @brief nva 更新产品线备份分区
 * @par 描述:
 * nva 更新产品线备份分区，用户根据需求将工作区数据备份到产品线备份分区
 * @retval 参见错误码:
 *                    ERROR_FILE_WRITE-6：写分区失败
 *                    ERROR_HASH_CHECK-9：hmac不匹配
 *                    ERROR_NO_PARTITION-12:当前分区不存在
 *                    ERROR_NO_BLK_WR-13：不支持block写
 */
int mdrv_nva_update_backupbk(void);

/**
 * @brief nva 更新出厂分区
 * @par 描述:
 * nva 更新出厂分区，正式使用在产线工位进行出厂分区写
 * @retval 参见错误码
 */
int mdrv_nva_update_default(void);

/**
 * @brief nva 恢复出厂
 * @par 描述:
 * nva 恢复出厂，调用后nva针对FACTORY属性为1的NVA项从出厂区恢复，并写入工作区
 * @retval 参见错误码
 */
int mdrv_nva_revert_factory(void);

/**
 * @brief nva 获取初始化状态
 * @par 描述:
 * nva 获取初始化状态
 * @retval 参见错误码
 */
int mdrv_nva_init_status(void);

/**
 * @brief nva获取对应nva项的长度
 * @param[in] item_id: 要读取的nva id
 * @param[out] len: 输出查询的长度
 * @retval 参见错误码
 */
int mdrv_nva_get_len(unsigned int item_id, unsigned int *len);

/*
 * @brief 向产品线提供的回调结构体
 *
 * @par 描述:
 * 向产品线提供的回调结构体
 *
 * flush_bk：
 * @param[in]  nva_bin：待写入的数据地址
 *             size：待写入的数据大小
 * attention:flush_bk回调需使用bsp_blk_logic_write写镜像数据
 * get_nva_whitelist
 * @param[out]  size：白名单的个数
 * 返回值为白名单存放的内存地址
 */
typedef struct {
    int (*flush_bk)(const char *nva_bin, unsigned int size);
    unsigned int *(*get_nva_whitelist)(unsigned int *size);
} nva_cb_s;

/*
 * @brief 向产品线提供的回调注册接口
 *
 * @par 描述:
 * 向产品线提供的回调注册接口
 *
 * @param[in]  cb：产品线的回调结构体指针
 * @retval  执行结果，0为成功，其他为失败
 *
 */
int mdrv_nva_register_cb(nva_cb_s *cb);

#ifdef __cplusplus
}
#endif

#endif
