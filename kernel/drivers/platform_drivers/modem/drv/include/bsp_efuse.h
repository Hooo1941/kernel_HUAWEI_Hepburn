/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2020. All rights reserved.
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

#ifndef __BSP_EFUSE_H__
#define __BSP_EFUSE_H__

#include <product_config.h>
#include <osl_types.h>
#include <bsp_print.h>
#include <mdrv_bmi_efuse.h>
#include <securec.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ****************************************************************************
 * Attention                           *
 * *****************************************************************************
 * Description : Driver for efuse
 * Core        : Acore、Ccore、Fastboot
 * Header File : the following head files need to be modified at the same time
 * : /acore/kernel/platform_drivers/include/bsp_efuse.h
 * : /ccore/include/ccpu/bsp_efuse.h
 * : /ccore/include/fusion/bsp_efuse.h
 * : /fastboot/include/bsp_efuse.h
 * **************************************************************************** */
#define EFUSE_GROUP_BIT_NUM (32)
#define BIT_INDEX_IN_GROUP(bit) ((bit) & 0x1F)
#define GROUP_INDEX(bit) ((bit) >> 0x5)
#define GROUP_LEN(bit_len) (((bit_len) + EFUSE_GROUP_BIT_NUM - 1) / EFUSE_GROUP_BIT_NUM)

/* *
 * efuse信息对应的ID
 */
typedef enum {
    /* ------芯片硬连线------- */
    /* ------0x100->0x1FF------- */
    EFUSE_CHIP_INFO_MIN = 0x100,
    EFUSE_CHIP_ID,
    EFUSE_ARM_DBG_CTRL,
    EFUSE_MP_FLAG,
    EFUSE_KCE,
    EFUSE_DFT_AUTH_KEY,
    EFUSE_USB_BOOT_DIS_CTRL,
    EFUSE_CORESIGHT_RST_CTRL,
    EFUSE_HIFI_DBG_CTRL,
    EFUSE_CS_DEVICE_CTRL,
    EFUSE_UART_DBG_CTRL,
    EFUSE_PDE_DBG_CTRL,
    EFUSE_TXP_NIDEN_CTRL,
    EFUSE_ACPU_NIDEN_CTRL,
    EFUSE_SEC_DBG_RST_CTRL,
    EFUSE_DFT_DISABLE_SEL,
    EFUSE_BOOT_SEL,
    EFUSE_APB_RD_HUK_DIS,
    EFUSE_APB_RD_SCP_DIS,
    EFUSE_DFT_AUTH_KEY_RD_CTRL,
    EFUSE_ARM_DBG_CTRL_DBGEN,
    EFUSE_ARM_DBG_CTRL_NIDEN,
    EFUSE_ARM_DBG_CTRL_SPIDEN,
    EFUSE_ARM_DBG_CTRL_SPNIDEN,
    EFUSE_JTAGEN_CTRL,
    EFUSE_GMECC_KEY_RD_CTRL,

    EFUSE_CHIP_INFO_MAX,
    /* ------解决方案使用------- */
    /* ------0x200->0x5FF------- */
    EFUSE_DRV_INFO_MIN = 0x200,
    EFUSE_DRV_DIE_ID,                     /* dieid信息字段 */
    EFUSE_DRV_HUK,                        /* huk信息字段 */
    EFUSE_DRV_5G_LAN_MAC_ADDR,            /* 5g lan mac字段 */
    EFUSE_DRV_ACPU_OP85_FREQ,             /* ACPU 0.85V电压档降频控制字段 */
    EFUSE_DRV_TSP_OP85_FREQ,              /* TSP 0.85V电压档降频控制字段 */
    EFUSE_DRV_ACPU_ONLY,                  /* acpu only控制字段 */
    EFUSE_DRV_TSENSOR_REF,                /* tsensor参考温度字段 */
    EFUSE_DRV_TSENSOR_000,                /* tsensor 000字段 */
    EFUSE_DRV_TSENSOR_001,                /* tsensor 001字段 */
    EFUSE_DRV_DMC_AVS_TEST_PASS,          /* DMC电源域AVS电压字段 */
    EFUSE_DRV_PERI_MDM_AVS_TEST_PASS,     /* PERI_MDM电源域AVS电压字段 */
    EFUSE_DRV_PASENSOR_DMC,               /* DMC PA sensor字段 */
    EFUSE_DRV_PASENSOR_MDM,               /* PERI_MDM sensor字段 */
    EFUSE_DRV_AVS_DISABLE,                /* AVS禁用字段 */
    EFUSE_DRV_VEHICLE_FLAGS,              /* 车载芯片标记字段 */
    EFUSE_DRV_PERI_MDM_AVS_VOLT_PASS,     /* PERI_MDM电源域大招抬压字段 */
    EFUSE_DRV_DMC_AVS_VOLT_PASS,          /* DMC电源域大招抬压字段 */
    EFUSE_MDRV_BBP_IDDQ,                   /* bbp iddq信息 */
    EFUSE_MDRV_BBP_PDT_OP85_FREQ,          /* BBP PDT 0.85V电压档降频控制字段 */
    EFUSE_MDRV_BBP_DEBUFMEM,               /* BBP DEBUF_MEM裁剪控制字段 */
    EFUSE_MDRV_BBP_23GMEM,                 /* BBP 23G_MEM裁剪控制字段 */
    EFUSE_MDRV_BBP_PDF0BYPASS,             /* BBP PDF0_bypass控制字段 */
    EFUSE_MDRV_BBP_PDF1BYPASS,             /* BBP PDF1_bypass控制字段 */
    EFUSE_DRV_GJ_HBK,                     /* 国际启动 根公钥hash值 */
    EFUSE_DRV_USB_VENDOR_DEVICE_ID,       /* USB VENDOR & DEVICE id */

    EFUSE_DRV_INFO_MAX,

    /* ------用户使用------- */
    /* ------0x600->0x6FF------- */
    EFUSE_USER_INFO_MIN = 0x600,
    EFUSE_MDRV_PRODUCT_REGIONS,           /* 产线使用的efuse字段 */
    EFUSE_USER_INFO_MAX,

    EFUSE_LAYOUT_ID_MAX
} efuse_layout_id_e;

typedef struct {
    efuse_layout_id_e name_id;
    u32 bit;
    u32 bit_len;
} efuse_layout_s;

#if (defined(CONFIG_EFUSE) && defined(CONFIG_EFUSE_IOT))
/* ****************************************************************************
 * 函 数 名  : bsp_efuse_read
 *
 * 功能描述  : 按组读取EFUSE中的数据
 *
 * 输入参数  : group  起始group
 * count  数组长度
 * 输出参数  : pBuf ：EFUSE中的数据
 * *************************************************************************** */
int bsp_efuse_read(u32 *pbuffer, u32 group, u32 count);
/* ****************************************************************************
 * 函 数 名  : bsp_efuse_write
 *
 * 功能描述  : 烧写Efsue
 *
 * 输入参数  : pBuf:待烧写的EFUSE值
 * group,Efuse地址偏移
 * count,烧写长度
 * *************************************************************************** */
int bsp_efuse_write(u32 *pbuffer, u32 group, u32 count);
/* 功能：获取efuse排布信息
 * id: efuse信息字符串的枚举ID，具体可参考efuse_layout_id_e
 * layout_info：info输出参数
 */
int bsp_efuse_get_layout_info(efuse_layout_id_e name_id, efuse_layout_s *layout_info);
/* if bsp_efuse_write_prepare() return val == 0 success
 * bsp_efuse_write_complete() must be used in pairs with bsp_efuse_write_prepare()
 */
int bsp_efuse_read_by_bit(unsigned int *pbuffer, unsigned int bit_start, unsigned int bits_len);
int bsp_efuse_write_prepare(void); /* supply for burn nvcounter */
void bsp_efuse_write_complete(void);
void bsp_efuse_ops_prepare(void);
void bsp_efuse_ops_complete(void);
/**
 * @brief 用于Efuse中相关域的读取
 *
 * @par 描述:
 * 用于Efuse中相关域的读取
 *
 * @param[in] id 对应字段的宏,具体可参考efuse_layout_id_e
 *
 * @retval >=0，表示传入的枚举对应字段的数值。
 * @retval <0，表示读取失败。
 *
 * @attention
 * <ul><li>该接口只可以用于读取efuse域不超过31bit的字段。</li></ul>
 * <ul><li>当调用该函数失败时请打印该函数返回值，便于问题定位。</li></ul>
 *
 * @par 依赖:
 * <ul><li>bsp_efuse.h：该接口声明所在的头文件。</li></ul>
 */
int bsp_efuse_get_value(efuse_layout_id_e id);
#else
static inline __attribute__((unused)) int bsp_efuse_read(u32 *pbuffer __attribute__((unused)),
    u32 group __attribute__((unused)), u32 count __attribute__((unused)))
{
    (void)memset_s(pbuffer, sizeof(u32) * EFUSE_MAX_PACK_SIZE, 0, sizeof(u32) * count);
    return 0;
}
static inline __attribute__((unused)) int bsp_efuse_write(u32 *pbuffer __attribute__((unused)),
    u32 group __attribute__((unused)), u32 count __attribute__((unused)))
{
    return 0;
}
static inline __attribute__((unused)) int bsp_efuse_get_layout_info(efuse_layout_id_e name_id __attribute__((unused)),
    efuse_layout_s *layout_info __attribute__((unused))) {
    return 0;
}
static inline __attribute__((unused)) int bsp_efuse_write_prepare(void)
{
    return 0;
}

static inline __attribute__((unused)) int bsp_efuse_get_value(efuse_layout_id_e id)
{
    return 0;
}
static inline __attribute__((unused)) void bsp_efuse_write_complete(void) {}
static inline __attribute__((unused)) void bsp_efuse_ops_prepare(void) {}
static inline __attribute__((unused)) void bsp_efuse_ops_complete(void) {}
#endif

#ifdef __cplusplus
}
#endif
#endif
