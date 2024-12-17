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
#ifndef _BSP_IPC_FUSION_H_
#define _BSP_IPC_FUSION_H_

#include <osl_types.h>
#include <mdrv_sysboot.h>

#ifdef __cplusplus
extern "C" {
#endif

/* *
 * @brief IPC错误句柄
 */
#define IPC_HANDLE_INVAILED (void *)0
/* *
 * @brief IPC处于单独复位阶段
 */
#define IPC_ERR_MODEM_RESETING 1

struct ipc_handle;
/* *
 * @brief ipc句柄定义
 */
typedef struct ipc_handle *ipc_handle_t;
/* *
 * @brief ipc回调函数定义
 */
typedef void (*ipc_cbk)(void *arg);

struct ipc_entry {
    ipc_cbk routine;
    void *arg;
};

typedef enum ipc_res_id {
    IPC_ACPU_INT_SRC_TSP_DUMP = 0x5000,            /* *<底软使用 DUMP发送到ACORE */
    IPC_ACPU_INT_SRC_MCU_DUMP = 0x5002,            /* *<底软使用 dump发送到ACPU */
    IPC_ACPU_INT_SRC_TSP_RESET_DSS0_IDLE = 0x500B, /* *<底软使用 单独复位TSP发送到ACORE */
    IPC_ACPU_INT_SRC_TSP_RESET_DSS1_IDLE = 0x500C, /* *<底软使用 单独复位TSP发送到ACORE */
    IPC_ACPU_INT_SRC_TSP_RESET_DSS2_IDLE = 0x500D, /* *<底软使用 单独复位TSP发送到ACORE */
    IPC_ACPU_INT_SRC_TSP_RESET_DSS3_IDLE = 0x500E, /* *<底软使用 单独复位TSP发送到ACORE */
    IPC_ACPU_INT_SRC_TSP_RESET_SUCC = 0x500F,      /* *<底软使用 单独复位TSP发送到ACORE */
    IPC_ACPU_INT_SRC_CMCPU_WAKEUP = 0x5018,        /* *<底软使用 低功耗唤醒MCU TSP发送到ACORE */
    IPC_ACPU_INT_SRC_CMCPU_LOCKUP = 0x5019,        /* *<底软使用 低功耗唤醒MCU TSP发送到ACORE持锁不睡 */
    IPC_ACPU_INT_SRC_MCPU_VERIFYMDM = 0x501A,      /* *<底软使用 lpm3 ap-cp解耦 */
    IPC_ACPU_INT_SRC_DPA_POWERDOWN = 0x501B,       /* *<底软使用 DPA发送到ACORE DPA下电*/
    IPC_ACPU_INT_SRC_DPA_DUMP = 0x501C,            /* *<底软使用 DPA发送到ACORE DPA DUMP*/
    IPC_ACPU_INT_SRC_TSP_HIFIRESET = 0x5020,       /* *<底软使用 HIFI单独复位 TSP发送到ACORE */

    IPC_MCU_INT_SRC_ACPU_DUMP = 0x5003,       /* *<底软使用 dump发送到MCU */
    IPC_MCU_INT_SRC_ACPU_CCPU_START = 0x5004, /* *<底软使用 ccpu解复位 */
    IPC_MCU_INT_SRC_ACPU_POWER_DOWN = 0x5005, /* *<底软使用 通知m3给acore下电 */

    IPC_TSP_INT_SRC_ACPU_DUMP = 0x5001,          /* *<底软使用 DUMP发送到TSP */
    IPC_TSP_INT_SRC_ACPU_DSS0_STAYUP = 0x5007,   /* *<底软使用 单独复位ACORE发送到TSP持锁不睡 */
    IPC_TSP_INT_SRC_ACPU_DSS1_STAYUP = 0x5008,   /* *<底软使用 单独复位ACORE发送到TSP持锁不睡 */
    IPC_TSP_INT_SRC_ACPU_DSS2_STAYUP = 0x5009,   /* *<底软使用 单独复位ACORE发送到TSP持锁不睡 */
    IPC_TSP_INT_SRC_ACPU_DSS3_STAYUP = 0x500A,   /* *<底软使用 单独复位ACORE发送到TSP持锁不睡 */
    IPC_TSP_INT_SRC_AMCPU_DSS0_WAKAEUP = 0x5010, /* *<底软使用 低功耗唤醒ACORE&MCORE发送到TSP */
    IPC_TSP_INT_SRC_AMCPU_DSS1_WAKAEUP = 0x5011, /* *<底软使用 低功耗唤醒ACORE&MCORE发送到TSP */
    IPC_TSP_INT_SRC_AMCPU_DSS2_WAKAEUP = 0x5012, /* *<底软使用 低功耗唤醒ACORE&MCORE发送到TSP */
    IPC_TSP_INT_SRC_AMCPU_DSS3_WAKAEUP = 0x5013, /* *<底软使用 低功耗唤醒ACORE&MCORE发送到TSP */
    IPC_TSP_INT_SRC_AMCPU_DSS0_LOCKUP = 0x5014,  /* *<底软使用 低功耗唤醒ACORE&MCORE发送到TSP持锁不睡 */
    IPC_TSP_INT_SRC_AMCPU_DSS1_LOCKUP = 0x5015,  /* *<底软使用 低功耗唤醒ACORE&MCORE发送到TSP持锁不睡 */
    IPC_TSP_INT_SRC_AMCPU_DSS2_LOCKUP = 0x5016,  /* *<底软使用 低功耗唤醒ACORE&MCORE发送到TSP持锁不睡 */
    IPC_TSP_INT_SRC_AMCPU_DSS3_LOCKUP = 0x5017,  /* *<底软使用 低功耗唤醒ACORE&MCORE发送到TSP持锁不睡 */
    IPC_TSP_INT_SRC_ACPU_HIFIRESET = 0x5021,     /* *<底软使用 HIFI单独复位 ACORE发送到TSP */
    IPC_TSP_INT_SRC_ACPU_RFILE_NOTIFY = 0x5022,     /* *<底软使用 rfile ACORE发送到TSP */

    /* ipc res for test */
    IPC_TSP_INT_SRC_ANY_IPC1_TEST = 0xFFFF0000,    /* *<底软使用 IPC1测试发送到TSP */
    IPC_TSP_INT_SRC_ANY_IPCS_TEST = 0xFFFF0001,    /* *<底软使用 IPCS测试发送到TSP */
    IPC_TSP_INT_SRC_ANY_IPCNS_TEST = 0xFFFF0002,   /* *<底软使用 IPCNS测试发送到TSP */
    IPC_ACORE_INT_SRC_ANY_IPCNS_TEST = 0xFFFF0003, /* *<底软使用 IPCNS测试发送到ACORE */
    IPC_MCU_INT_SRC_ANY_IPCS_TEST = 0xFFFF0004,    /* *<底软使用 IPCS测试发送到MCU */
    IPC_MCU_INT_SRC_ANY_IPCNS_TEST = 0xFFFF0005,   /* *<底软使用 IPCNS测试发送到MCU */

} ipc_res_id_e;

/* ****************************************************************************
* 函 数 名     : bsp_ipc_init
*
* 功能描述  : IPC模块初始化
*
* 输入参数  : 无
* 输出参数  : 无
*
* 返 回 值     :

**************************************************************************** */
s32 bsp_ipc_init(void);

/* ****************************************************************************
* 函 数 名      : bsp_ipc_open
*
* 功能描述  : ipc逻辑资源号获取句柄函数
*
* 输入参数  : ipc_res_id ipc逻辑资源号
            phandle 句柄
* 输出参数  : 无
*
* 返 回 值      :  OK&ERROR

**************************************************************************** */
s32 bsp_ipc_open(ipc_res_id_e ipc_res_id, ipc_handle_t *phandle);

/* ****************************************************************************
* 函 数 名      : bsp_ipc_bind
*
* 功能描述  : ipc中断绑线程函数
*
* 输入参数  :   phandle 句柄 线程对应mask位
* 输出参数  : 无
*
* 返 回 值      :  OK&ERROR

**************************************************************************** */
s32 bsp_ipc_bind(ipc_handle_t phandle, u32 core_mask);

/* ****************************************************************************
* 函 数 名      : bsp_ipc_send
*
* 功能描述  : 往目标核发送中断
*
* 输入参数  : phandle 句柄

* 输出参数  : 无
*
* 返 回 值      :  OK&ERROR

**************************************************************************** */
s32 bsp_ipc_send(ipc_handle_t phandle);

/* ****************************************************************************
* 函 数 名      : bsp_ipc_connect
*
* 功能描述  : 挂接中断回调函数，此函数默认使能通道所使用中断
*
* 输入参数  : phandle 句柄
            routine 中断服务程序
            void *parameter      中断服务程序参数

* 输出参数  : 无
*
* 返 回 值      :  OK&ERROR

**************************************************************************** */
s32 bsp_ipc_connect(ipc_handle_t phandle, ipc_cbk routine, void *parameter);

/* ****************************************************************************
* 函 数 名      : bsp_ipc_disconnect
*
* 功能描述  : 取消中断回调函数，此函数默认去使能通道所使用中断
*
* 输入参数  : phandle 句柄

* 输出参数  : 无
*
* 返 回 值      :  OK&ERROR

**************************************************************************** */
s32 bsp_ipc_disconnect(ipc_handle_t phandle);

/* ****************************************************************************
* 函 数 名      : bsp_ccore_ipc_enable
*
* 功能描述  : 单独复位ipc功能使能
*
* 返 回 值      :  OK&ERROR
**************************************************************************** */
void bsp_ccore_ipc_enable(void);

/* ****************************************************************************
* 函 数 名      : bsp_ccore_ipc_disable
*
* 功能描述  : 单独复位ipc功能关闭
*
* 返 回 值      :  OK&ERROR
**************************************************************************** */
void bsp_ccore_ipc_disable(void);

/* ****************************************************************************
* 函 数 名      : bsp_ipc_modem_reset
*
* 功能描述  : 单独复位回调接口
*
* 返 回 值      :  OK&ERROR
**************************************************************************** */
void bsp_ipc_modem_reset(drv_reset_cb_moment_e eparam, int usrdata);
#ifdef __cplusplus
}
#endif

#endif /* end #define _BSP_IPC_H_ */
