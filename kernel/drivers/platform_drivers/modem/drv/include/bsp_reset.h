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

#ifndef BSP_RESET_H
#define BSP_RESET_H

#include <osl_common.h>
#include <mdrv_sysboot.h>
#include <bsp_slice.h>
#include <bsp_sysctrl.h>

/* NOTE： 本文件为对其它模块暴露的接口定义 */

#define RESET_OK (0)
#define RESET_ERROR (-1)

#define RESET_STAGE_GET(reset_info) ((reset_info)&0xff)
#define RESET_ACTION_GET(reset_info) (((reset_info) >> 16) & 0xff)
#define RESET_INFO_MAKEUP(action, stage) ((action << 16) | (stage))

enum { MDM_RESET_UNSUPPORT = 0, MDM_RESET_SUPPORT = 1 };

enum RESET_SYSFS_VALUE {
    DRV_MODEM_OFF = 2,
    DRV_MODEM_RESET = 10,
    DRV_MODEM_RILD_SYS_ERR = 11,
};

enum MODEM_RESET_BOOT_MODE { CCORE_BOOT_NORMAL = 0, CCORE_IS_REBOOT = 0x87654321UL };

enum DRV_RESET_CB_PIOR {
    DRV_RESET_CB_PIOR_MIN = 0,
    DRV_RESET_CB_PIOR_ICC = 0,
    DRV_RESET_CB_PIOR_PFA = 3,
    DRV_RESET_CB_PIOR_USB = 4,
    DRV_RESET_CB_PIOR_DRA = 5,
    DRV_RESET_CB_PIOR_PFA_TFT = 6,
    DRV_RESET_CB_PIOR_GMAC = 7,
    DRV_RESET_CB_PIOR_IQI = 8,
    DRV_RESET_CB_PIOR_SOUND = 9,
    DRV_RESET_CB_PIOR_SCI = 10,
    // RESET_CALLCBFUN_PIOR_<组件名>
    // ...
    DRV_RESET_CB_PIOR_CSHELL = 16,
    DRV_RESET_CB_PIOR_RFILE,
    DRV_RESET_CB_PIOR_PCIE_PM,
    DRV_RESET_CB_PIOR_DUMPAP = 45,
    DRV_RESET_CB_PIOR_DUMPCP = 46,
    DRV_RESET_CB_PIOR_DUMPNR = 47,
    DRV_RESET_CB_PIOR_DUMPM3 = 48,
    DRV_RESET_CB_PIOR_DIAG = 49,
    DRV_RESET_CB_PIOR_ALL = 51,
    DRV_RESET_CB_PIOR_MAX
};

enum MODEM_ACTION { MODEM_POWER_OFF, MODEM_POWER_ON, MODEM_RESET, MODEM_NORMAL, INVALID_MODEM_ACTION };

/* M核和C核之间核间消息约定: 复位阶段信息复用DRV_RESET_CALLCBFUN_MOMENT枚举定义 */
enum RESET_ICC_MSG {
    RESET_MCORE_BEFORE_RESET_OK,
    RESET_MCORE_RESETING_OK,
    RESET_MCORE_AFTER_RESET_OK,
    RESET_MCORE_BEFORE_AP_TO_MDM_BUS_ERR,
    RESET_MCORE_BEFORE_MDM_TO_AP_BUS_ERR,
    RESET_MCORE_BEFORE_MODEM_ACCESS_DDRC_ERR,
    RESET_MCORE_BEFORE_NOC_POWER_IDLEIN_ERR,
    RESET_MCORE_BEFORE_NOC_POWER_IDLEINACK_ERR,
    RESET_MCORE_BEFORE_NOC_POWER_IDLEOUT_ERR,
    RESET_MCORE_BEFORE_NOC_POWER_IDLEOUTACK_ERR,
    RESET_CCORE_REBOOT_OK,
    RESET_ACORE_HOST_IDLE_REQ,
    RESET_CCORE_HOST_IN_IDLE,
    RESET_INNER_CORE_INVALID_MSG,
};

/* 标识可否向对方核发送核间消息 */
enum RESET_MULTICORE_CHANNEL_STATUS {
    CCORE_STATUS = 0x00000001,
    MCORE_STATUS = 0x00000002,
    HIFI_STATUS = 0x00000004,
    INVALID_STATUS = 0x10000000
};

enum RESET_CBFUNC_PRIO { RESET_CBFUNC_PRIO_LEVEL_LOWT = 0, RESET_CBFUNC_PRIO_LEVEL_HIGH = 51 };

/* mdrv 封装接口 */
int bsp_reset_cb_func_register(const char *name, pdrv_reset_cbfun func, drv_reset_cb_moment_e user_data, int prior);

/* 底软内部调用接口 */
void bsp_modem_power_off(void);
/*
 * @attention
 * <ul><li>该接口不支持重入</li></ul>
 * <ul><li>该接口只能由dump模块调用，其它模块禁止调用</li></ul>
 */
int bsp_cp_reset(void);
// can only be called after nva init ok 
u32 bsp_reset_is_feature_on(void);
u32 bsp_reset_is_successful(u32 timeout_ms);
u32 bsp_reset_ccore_is_reboot(void);
void bsp_reset_ccore_init_ok(void);
s32 bsp_rfile_reset_cb(drv_reset_cb_moment_e eparam, s32 usrdata);
int bsp_dump_sec_channel_free(drv_reset_cb_moment_e eparam, s32 usrdata);
ssize_t bsp_reset_set_dev(const char *buf, size_t count);

#endif /*  __BSP_RESET_H__ */
