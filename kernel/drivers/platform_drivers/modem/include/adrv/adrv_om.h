/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
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


#ifndef ADRV_OM_H
#define ADRV_OM_H

#include <linux/module.h>

typedef struct {
    unsigned long base_addr;
    unsigned int  buffer_size;
    unsigned int  rsv_mem_usable;
    unsigned int  bbp_mem_flag;
    unsigned int  threshold_timeout;
    unsigned int  reserved[4]; /* 4:reserved for future use */
}mdmbuf_info_s;

typedef struct {
    unsigned int version;
    mdmbuf_info_s meminfo;
}membuf_attr_s;

membuf_attr_s *get_mdm_dfx_mem_info(void);
unsigned int get_mdm_bbpds_mem_flag(void);

#define STR_EXCEPTIONDESC_MAXLEN 48

/* 与AP 约定定义成如下格式，如果变更，有AP变更 */
typedef enum {
    CP_S_MODEMDMSS = 0x70,
    CP_S_MODEMNOC = 0x71,
    CP_S_MODEMAP = 0x72,
    CP_S_EXCEPTION = 0x73,
    CP_S_RESETFAIL = 0x74,
    CP_S_NORMALRESET = 0x75,
    CP_S_RILD_EXCEPTION = 0x76,
    CP_S_3RD_EXCEPTION = 0x77,
    CP_S_DRV_EXC = 0x78,
    CP_S_PAM_EXC = 0x79,
    CP_S_GUAS_EXC = 0x7a,
    CP_S_CTTF_EXC = 0x7b,
    CP_S_CAS_CPROC_EXC = 0x7c,
    CP_S_GUDSP_EXC = 0x7d,
    CP_S_TLPS_EXC = 0x7e,
    CP_S_TLDSP_EXC = 0x7f,
    CP_S_CPHY_EXC = 0x80,
    CP_S_GUCNAS_EXC = 0x81,
} EXCH_SOURCE;

enum RDR_SAVE_LOG_FLAG {
    RDR_SAVE_DMESG = (0x1 << 0),       /**< 0:indicates log from dmesg */
    RDR_SAVE_CONSOLE_MSG = (0x1 << 1), /**< 1:indicates log from console msg */
    RDR_SAVE_BL31_LOG = (0x1 << 2),    /**< 2:indicates log from BL31 */
    RDR_SAVE_LOGBUF = (0x1 << 3),      /**< 3:indicates log form logbuf */
};

struct list_head_rdr {
    struct list_head_rdr *next, *prev;
};
typedef void (*rdr_e_callback)(unsigned int, void *);

struct rdr_exception_info_s {
    struct list_head_rdr e_list; /**< list_head_rdr instead of list_head  to solve the confliction between
                                    <linux/types.h> */
    unsigned int e_modid;        /**< exception id,if modid equal 0, will auto generation modid, and return it. */
    unsigned int e_modid_end;   /**< can register modid region. [modid~modid_end];need modid_end >= modid. 
                                   if modid_end equal 0, will be register modid only, but modid & modid_end cant equal 0 at the same time. */
    unsigned int e_process_priority;  /**< exception process priority. */
    unsigned int e_reboot_priority;   /**< exception reboot priority. */
    unsigned long long e_notify_core_mask;  /**< eed notify other core mask. */
    unsigned long long e_reset_core_mask;   /**< need reset other core mask. */
    unsigned long long e_from_core; /**< the core of happen exception. */
    unsigned int e_reentrant;       /**< whether to allow exception reentrant. */
    unsigned int e_exce_type;       /**< the type of exception. */
    unsigned int e_exce_subtype;
    unsigned int e_upload_flag;
    unsigned char e_from_module[MODULE_NAME_LEN];   /**< the module of happen excption */
    unsigned char e_desc[STR_EXCEPTIONDESC_MAXLEN]; /**< the desc of happen excption */
    unsigned int e_save_log_flags; /**< set bit 1 to save the log(dmsg, console, bl31log) */
    unsigned int e_reserve_u32;    /**< reserve u32 */
    void *e_reserve_p;             /**< reserve void *. */
    rdr_e_callback e_callback;
};

enum UPLOAD_FLAG {
    RDR_UPLOAD_YES = 0xff00fa00,
    RDR_UPLOAD_NO
};
enum REENTRANT {
    RDR_REENTRANT_ALLOW = 0xff00da00,
    RDR_REENTRANT_DISALLOW
};

enum REBOOT_PRI {
    RDR_REBOOT_NOW = 0x01,
    RDR_REBOOT_WAIT,
    RDR_REBOOT_NO,
    RDR_REBOOT_MAX
};
enum CORE_LIST {
    RDR_AP = 0x1,
    RDR_CP = 0x2,
    RDR_HIFI = 0x8,
    RDR_LPM3 = 0x10,
    RDR_MODEMAP = 0x200,
};
enum PROCESS_PRI {
    RDR_ERR = 0x01,
    RDR_WARN,
    RDR_OTHER,
    RDR_PPRI_MAX
};


typedef void (*pfn_cb_dump_done)(unsigned int modid, unsigned long long coreid);
typedef void (*pfn_dump)(unsigned int modid, unsigned int etype, unsigned long long coreid, char *logpath,
    pfn_cb_dump_done fndone);
typedef void (*pfn_reset)(unsigned int modid, unsigned int etype, unsigned long long coreid);

struct rdr_module_ops_pub {
    pfn_dump ops_dump;
    pfn_reset ops_reset;
};

struct rdr_register_module_result {
    unsigned long long log_addr;
    unsigned int log_len;
    unsigned long long nve;
};

#ifdef CONFIG_DFX_BB
/**
 * @brief 异常处理注册
 *
 * @par 描述:
 * 注册异常处理
 *
 * @param[in]  e, 异常信息
 *
 * @retval >0, e_modid_end
 * @retval 0, 处理失败
 */
unsigned int rdr_register_exception(struct rdr_exception_info_s *e);

/**
 * @brief 异常处理注册
 *
 * @par 描述:
 * 注册异常处理
 *
 * @param[in]  coreid, 异常核ID
 * @param[in]  ops, 异常处理函数与reset函数
 * @param[in]  retinfo, 注册返回信息
 *
 * @retval 非NULL, 执行成功
 * @retval NULL, 执行失败
 */
int rdr_register_module_ops(unsigned long long coreid, struct rdr_module_ops_pub *ops,
    struct rdr_register_module_result *retinfo);


/**
 * @brief 软件异常记录
 *
 * @par 描述:
 * 软件异常记录接口，完成异常文件记录之后主动复位单板
 *
 * @retval 无
 */
void rdr_system_error(unsigned int modid, unsigned int arg1, unsigned int arg2);

#else
static inline unsigned int rdr_register_exception(struct rdr_exception_info_s *e)
{
    return 0;
}
static inline int rdr_register_module_ops(unsigned long long coreid, struct rdr_module_ops_pub *ops,
    struct rdr_register_module_result *retinfo)
{
    return -1;
}
static inline void rdr_system_error(unsigned int modid, unsigned int arg1, unsigned int arg2) {}
#endif

/**
 * @brief 当已注册到rdr上的异常发生时，需要调用此接口触发保存log和reset流程。此接口供AP使用。（待删除）
 *
 * @par 描述:
 * 当已注册到rdr上的异常发生时，需要调用此接口触发保存log和reset流程。此接口供AP使用。
 *
 * @retval 无
 */
void rdr_syserr_process_for_ap(unsigned int modid, unsigned long long arg1, unsigned long long arg2);

/**
 * @brief 上次整机重启是否是由Modem引起
 *
 * @par 描述:
 * 判断上次整机重启是否是由Modem引起
 *
 * @retval 非0, modem引起的重启
 * @retval 0, 不是modem引起的重启
 */
unsigned int is_reboot_reason_from_modem(void);

/**
 * @brief 获取保存log的路径
 *
 * @par 描述:
 * 整机重启后如果modem需要保存log，获取保存log的路径
 *
 * @retval 非NULL, log的路径
 * @retval NULL, 执行失败
 */
char *rdr_get_reboot_logpath(void);

enum EDITION_KIND {
    EDITION_USER = 1,
    EDITION_INTERNAL_BETA = 2,
    EDITION_OVERSEA_BETA = 3,
    EDITION_MAX
};

#ifdef CONFIG_DFX_BB
 /**
 * @brief 获取保存log的路径
 *
 * @par 描述:
 * 整机重启后如果modem需要保存log，获取保存log的路径
 *
 * @retval 0x01, User
 * @retval 0x02, Internal BETA
 * @retval Oversea BETA
 */
unsigned int bbox_check_edition(void);
#else
static inline unsigned int bbox_check_edition(void)
{
    return EDITION_USER;
}
#endif

#endif
