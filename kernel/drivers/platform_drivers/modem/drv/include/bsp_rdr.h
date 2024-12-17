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


#ifndef __BSP_RDR_H__
#define __BSP_RDR_H__

#include <linux/module.h>
#include <osl_types.h>


#define RDR_OK (0)
#define RDR_ERROR ((u32)(-1))

#if !defined(MODULE_NAME_LEN)
#define MODULE_NAME_LEN (64 - sizeof(unsigned long))
#endif
#define STR_EXCEPTIONDESC_MAXLEN 48
#define STR_TASKNAME_MAXLEN 16
#define STR_USERDATA_MAXLEN 64
#define SUB_NAME_MAXLEN 24

enum EDITION_KIND {
    EDITION_USER = 1,
    EDITION_INTERNAL_BETA = 2,
    EDITION_OVERSEA_BETA = 3,
    EDITION_MAX
};

enum mntn_core {
    MNTN_AP = 0x0,
    MNTN_CP = 0x1,
    MNTN_TEEOS = 0x2,
    MNTN_HIFI = 0x3,
    MNTN_LPM3 = 0x4,
    MNTN_MDMAP_TEE = 0x5,
    MNTN_RESV0 = 0x6,
    MNTN_RESV1 = 0x7,
    MNTN_RESV2 = 0x8,
    MNTN_MDMAP = 0x9,
    MNTN_NR = 0xa,
    MNTN_XLOADER = 0xb,
    MNTN_HIBOOT = 0xc,
    MNTN_NRL2HAC = 0xd,
    MNTN_MTEE = 0xe,
    MNTN_MAX
};

#define RDR_CORE(n) (1 << (n))

enum rdr_core_list {
    RDR_AP = RDR_CORE(MNTN_AP),
    RDR_CP = RDR_CORE(MNTN_CP),
    RDR_TEEOS = RDR_CORE(MNTN_TEEOS),
    RDR_HIFI = RDR_CORE(MNTN_HIFI),
    RDR_LPM3 = RDR_CORE(MNTN_LPM3),
    RDR_MDMAP_TEE = RDR_CORE(MNTN_MDMAP_TEE),
    RDR_RESV0 = RDR_CORE(MNTN_RESV0),
    RDR_RESV1 = RDR_CORE(MNTN_RESV1),
    RDR_RESV2 = RDR_CORE(MNTN_RESV2),
    RDR_MODEMAP = RDR_CORE(MNTN_MDMAP),
    RDR_NR = RDR_CORE(MNTN_NR),
    RDR_XLOADER = RDR_CORE(MNTN_XLOADER),
    RDR_HIBOOT = RDR_CORE(MNTN_HIBOOT),
    RDR_NRL2HAC = RDR_CORE(MNTN_NRL2HAC),
    RDR_MTEE = RDR_CORE(MNTN_MTEE),
    RDR_MAX = RDR_CORE(MNTN_MAX)
};

typedef enum {
    RDR_SAVE_FILE_NORMAL = 0x0,
    RDR_SAVE_FILE_NEED = 0xAABBCCDD,
    RDR_SAVE_FILE_END = 0x5A5A5A5A
} rdr_save_flag_t;

typedef enum {
    RDR_START_POWER_ON = 0,
    RDR_START_REBOOT = 0x5A5A1111,
    RDR_START_EXCH = 0x5A5A2222
} rdr_start_flag_e;

#define RDR_MNTN_EXC_AP_START (0x80000000)
#define RDR_MNTN_EXC_AP_END (0x80FFFFFF)
#define RDR_MNTN_EXC_PHONE_MODEM_START (0x81000000)
#define RDR_MNTN_EXC_PHONE_MODEM_END (0x81FFFFFF)
#define RDR_MNTN_EXC_CP_START (0x82000000)
#define RDR_MNTN_EXC_CP_END (0x82FFFFFF)
#define RDR_MNTN_EXC_LR_START (0)
#define RDR_MNTN_EXC_LR_END (0)
#define RDR_MNTN_EXC_TEEOS_START (0x83000000)
#define RDR_MNTN_EXC_TEEOS_END (0x83FFFFFF)
#define RDR_MNTN_EXC_MDMAP_START (0)
#define RDR_MNTN_EXC_MDMAP_END (0)
#define RDR_MNTN_EXC_LPM3_START (0x85000000)
#define RDR_MNTN_EXC_LPM3_END (0x85FFFFFF)
#define RDR_MNTN_EXC_HIFI_START (0x86000000)
#define RDR_MNTN_EXC_HIFI_END (0x86FFFFFF)
#define RDR_MNTN_EXC_MDMAP_TEE_START (0)
#define RDR_MNTN_EXC_MDMAP_TEE_END (0)
#define RDR_MNTN_EXC_RESV0_START (0)
#define RDR_MNTN_EXC_RESV0_END (0)
#define RDR_MNTN_EXC_RESV1_START (0)
#define RDR_MNTN_EXC_RESV1_END (0)
#define RDR_MNTN_EXC_RESV2_START (0)
#define RDR_MNTN_EXC_RESV2_END (0)
#define RDR_MNTN_EXC_NR_START (0xB0000000)
#define RDR_MNTN_EXC_NR_END (0xBFFFFFFF)
#define RDR_MNTN_INVALID_MODID (0xFFFFFFFF)

/* 添加异常类型，请同时修改rdr_exc.c的g_stExcType异常类型 */
typedef enum {
    FAST_POWERUP,            /* 产线块式开机 */
    POWERKEY_POWERUP,        /* 按键500ms开机 */
    VBUS_POWERUP,            /* 插入充电器200ms开机 */
    ALARM_ON_POWERUP,        /* 闹铃唤醒开机 */
    POWERHOLD_POWERUP,       /* power hold触发开机 */
    POWERKEY_SHUTDOWN,       /* 按键10s关机 */
    POWERHOLD_SHUTDOWN,      /* power hold触发关机 */
    FAULT_N_HOLD_SHUTDOWN,
    NORMAL_POWERUP_SHUTDOWN, /* 以上为单板上电启动，只记录启动信息，不保存异常文件 */
    RESTART_REBOOT_START = NORMAL_POWERUP_SHUTDOWN,
    POWERKEY_10S_REBOOT = RESTART_REBOOT_START, /* 按键10s重启 */
    RESETKEY_REBOOT,                            /* 按键重启 */
    SOFT_REBOOT,                                /* 软件重启 */
    RESTART_REBOOT_END,                         /* 单板重启，需要判断软件是否发生异常 */
    PMU_EXCEPTION = RESTART_REBOOT_END,         /* PMU异常，只记录启动信息，不保存异常文件 */
    /* AP的异常类型 AP_EXC_START~AP_EXC_END */
    AP_EXC_START,
    AP_S_WDT_TIMEOUT,
    AP_S_EXC_PANIC,
    AP_S_EXC_SFTRESET,
    AP_S_EXC_PANIC_INT,
    AP_S_DMSS_EXC,
    AP_EXC_END,
    /* LPM3的异常类型 LPM3_EXC_START~LPM3_EXC_END */
    LPM3_EXC_START = AP_EXC_END,
    LPM3_S_EXCEPTION,
    LPM3_S_WDT_TIMEOUT,
    LPM3_EXC_END,
    /* TEEOS的异常类型 TEEOS_EXC_START~TEEOS_EXC_END */
    TEEOS_EXC_START = LPM3_EXC_END,
    TEEOS_S_WDT_TIMEOUT,
    TEE_S_EXCEPTION,
    TEEOS_EXC_END,
    /* CP的异常类型 CP_EXC_START~CP_EXC_END */
    /* CP的异常类型要与adrv_om.h的手机版本保持一致，请谨慎修政 */
    CP_EXC_START = TEEOS_EXC_END,
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
    CP_EXC_END,
    RDR_TEST_WDT_TIMEOUT = CP_EXC_END,
    RDR_TEST_PANIC,
    RDR_TEST_DIE,
    RDR_TEST_STACKOVERFLOW,
    RDR_PHONE_MODEM_EXC,
    RDR_PHONE_MODEM_NORMAL_RESET,
    /* HIFI的异常类型 SOCHIFI_EXC_START ~ SOCHIFI_EXC_END */
    SOCHIFI_EXC_START,
    SOCHIFI_S_EXCEPTION,
    SOCHIFI_EXC_END,
    UNDEFINE,
} MNTN_EXC_TYPE;


enum REBOOT_PRI {
    RDR_REBOOT_NOW = 0x01,
    RDR_REBOOT_WAIT,
    RDR_REBOOT_NO,
    RDR_REBOOT_MAX
};

enum PROCESS_PRI {
    RDR_ERR = 0x01,
    RDR_WARN,
    RDR_OTHER,
    RDR_PPRI_MAX
};

enum REENTRANT {
    RDR_REENTRANT_ALLOW = 0xff00da00,
    RDR_REENTRANT_DISALLOW
};

enum UPLOAD_FLAG {
    RDR_UPLOAD_YES = 0xff00fa00,
    RDR_UPLOAD_NO
};


enum RDR_RETURN {
    RDR_SUCCESSED = 0x9f000000,
    RDR_FAILD = 0x9f000001,
    RDR_NULLPOINTER = 0x9f0000ff
};

enum RDR_SAVE_LOG_FLAG {
    RDR_SAVE_DMESG = (0x1 << 0),
    RDR_SAVE_CONSOLE_MSG = (0x1 << 1),
    RDR_SAVE_BL31_LOG = (0x1 << 2), /* 1左移2，第2比特为1 */
    RDR_SAVE_LOGBUF = (0x1 << 3),   /* 1左移3，第3比特为1 */
};

#if !defined(__HIBOOT__)
typedef void (*rdr_e_callback)(u32, void *);
typedef void (*rdr_exc_info_callback)(u32, void *, void *);

/*
 *   struct list_head   e_list;
 *   u32 modid,             exception id;
 *      if modid equal 0, will auto generation modid, and return it.
 *   u32 modid_end,         can register modid region. [modid~modid_end];
        need modid_end >= modid,
 *      if modid_end equal 0, will be register modid only,
        but modid & modid_end cant equal 0 at the same time.
 *   u32 process_priority,  exception process priority
 *   u32 reboot_priority,   exception reboot priority
 *   u64 save_log_mask,     need save log mask
 *   u64 notify_core_mask,  need notify other core mask
 *   u64 reset_core_mask,   need reset other core mask
 *   u64 from_core,         the core of happen exception
 *   u32 reentrant,         whether to allow exception reentrant
 *   u32 exce_type,         the type of exception
 *   char* from_module,         the module of happen excption
 *   char* desc,                the desc of happen excption
 *   u32 save_log_flags,		set bit 1 to save the log(dmsg, console, bl31log)
 *   rdr_e_callback callback,   will be called when excption has processed.
 *   u32 reserve_u32;       reserve u32
 *   void* reserve_p            reserve void *
 */
struct rdr_exception_info_s {
    struct list_head e_list;
    u32 e_modid;            /* 必填 */
    u32 e_modid_end;        /* 必填 e_modid_end>= e_modid */
    u32 e_process_priority; /* 必填  < RDR_PPRI_MAX */
    u32 e_reboot_priority;  /* 必填 < RDR_REBOOT_MAX */
    u64 e_notify_core_mask; /* 必填 < RDR_MAX */
    u64 e_reset_core_mask;  /* 必填 < RDR_MAX */
    u64 e_from_core;        /* 必填 < RDR_MAX,枚举值使用bsp_rdr.h */
    u32 e_reentrant;        /* 选填，默认不支持重入 */
    u32 e_exce_type;        /* 必填 与rdr_exc.h中对庿 */
    u32 e_sub_exc_type;
    u32 e_upload_flag;                   /* 选填，默认不支持upload */
    u8 e_from_module[MODULE_NAME_LEN];   /* 必填 */
    u8 e_desc[STR_EXCEPTIONDESC_MAXLEN]; /* 必填 */
    u32 e_save_log_flags;
    u32 e_reserve_u32;         /* 保留 */
    void *e_reserve_p;         /* 保留 */
    rdr_e_callback e_callback; /* 选填,不填时必须为穿 */
    rdr_exc_info_callback excinfo_callbak;
};
struct exp_subtype {
    unsigned int exception;
    unsigned char category_name[SUB_NAME_MAXLEN];
    unsigned char subtype_name[SUB_NAME_MAXLEN];
    unsigned int subtype_num;
};
#define RDR_EXC_SUBTYPE_MAP(x, y, z) { x, #y, #z, z },

/*
 * func name: pfn_cb_dump_done
 * func args:
 * u32   modid
 * exception id
 * u64   coreid
 * which core done
 * return value     null
 */
typedef void (*pfn_cb_dump_done)(u32 modid, u64 coreid);

/*
 * func name: pfn_dump
 * func args:
 * u32   modid
 * exception id
 * u64   coreid
 * exception core
 * u32   etype
 * exception type
 * char*     logpath
 * exception log path
 * pfn_cb_dump_done fndone
 * return       mask bitmap.
 * 注意:回调过程中不允许有阻塞操使
 */
typedef void (*pfn_dump)(u32 modid, u32 etype, u64 coreid, char *logpath, pfn_cb_dump_done fndone);
/*
 * func name: pfn_reset
 * func args:
 * u32   modid
 * exception id
 * u32   coreid
 * exception core
 * u32   e_type
 * exception type
 * return value     null
 * 注意:回调过程中不允许有阻塞操使
 */
typedef void (*pfn_reset)(u32 modid, u32 etype, u64 coreid);

struct rdr_module_ops_pub {
    pfn_dump ops_dump;
    pfn_reset ops_reset;
};

struct rdr_register_module_result {
    u64 log_addr;
    u32 log_len;
    u64 nve;
};

#ifdef CONFIG_MODEM_RDR
/*
 * func name: rdr_register_exception_type
 * func args:
 * struct rdr_exception_info_pub* s_e_type
 * return value     e_modid
 * < 0 error
 * >=0 success
 */
u32 rdr_register_exception(struct rdr_exception_info_s *e);

/*
 * func name: rdr_register_module_ops
 * func args:
 * u32 coreid,       core id;
 * .
 * struct rdr_module_ops_pub* ops;
 * struct rdr_register_module_result* retinfo
 * return value     e_modid
 * < 0 error
 * >=0 success
 */
int rdr_register_module_ops(u64 coreid, struct rdr_module_ops_pub *ops, struct rdr_register_module_result *retinfo);


/*
 * func name: rdr_system_error
 * func args:
 * u32 modid,         modid( must be registered);
 * u32 arg1,          arg1;
 * u32 arg2,          arg2;
 * char *  data,          short message.
 * u32 length,        len(IMPORTANT: <=4k)
 * return void
 */
void rdr_system_error(u32 modid, u32 arg1, u32 arg2);
void rdr_fatal_system_error(u32 modid, u32 arg1, u32 arg2);
/*
 * 函数吿: bbox_check_edition
 * 函数参数:
 * void
 * 返回倿:
 * unsigned int:    返回版本信息
 * 0x01        USER
 * 0x02        INTERNAL BETA
 * 0x03        OVERSEA BETA
 *
 * 该函数会访问用户的data分区，因此依赖于文件系统的正确挂载?
 * 由于没有超时机制，等待文件系统挂载的过程会导致进程进兿
 * 不确定时长的睡眠。综上在不能睡眠的场景不能调用该接口?
 */
unsigned int bbox_check_edition(void);
/*
 * func name: rdr_save_file
 * .
 * func args:
 * char*  filename,            path of watit file.
 * void * data,                data
 * void * phy_addr,            phy_addr,,拼片场景霿要传物理地址
 * u32 length,                 data length
 * u32 max_size,               file max length
 * bool append                 is append file
 *
 * return
 * 0  success
 */
s32 rdr_save_file(const char *filename, const void *data, u32 length, u32 max_size, bool append);
/*
 * 通过pcie分段传输日志接口
 * char*  filename,            path of watit file.
 * void * data,                data
 * u32 part_length,            data part length
 * u32 file_length,            file  length
 */
int rdr_save_part_file(char *filename, const void *data, void *phy_addr, u32 part_length, u32 file_length);
int rdr_init_trans_part_file_info(char *filename, u32 file_length);
/* ****************************************************************************
 * 凿 敿 吿  : is_reboot_reason_from_modem
 *
 * 功能描述  : 判断上次整机重启是否是由Modem引起
 *
 * 输入参数  :
 *
 * 输出参数  : 旿
 *
 * 迿 囿 倿  : 旿
 *
 * 其它说明  : kernel
 *
 * *************************************************************************** */
unsigned int is_reboot_reason_from_modem(void);
/* ****************************************************************************
 * 凿 敿 吿  : rdr_get_reboot_logpath
 *
 * 功能描述  : 整机重启后如果modem霿要保存log，获取保存log的路徿
 * 输入参数  :
 *
 * 输出参数  : 旿
 *
 * 迿 囿 倿  : 旿
 *
 * 其它说明  : kernel
 *
 * *************************************************************************** */
char *rdr_get_reboot_logpath(void);
s32 rdr_create_exc_dir(const char *dir_name, char *dstpath, u32 dstsize, const char *date);
#else
static inline u32 rdr_register_exception(struct rdr_exception_info_s *e)
{
    return 0;
}
static inline int rdr_register_module_ops(u64 coreid, struct rdr_module_ops_pub *ops,
    struct rdr_register_module_result *retinfo)
{
    return -1;
}
static inline void rdr_system_error(u32 modid, u32 arg1, u32 arg2) {}
static inline void rdr_fatal_system_error(u32 modid, u32 arg1, u32 arg2) {}
static inline unsigned int bbox_check_edition(void)
{
    return EDITION_INTERNAL_BETA;
}
static inline s32 rdr_save_file(const char *filename, const void *data, void *phy_addr, u32 length, u32 max_size,
    bool append)
{
    return -1;
}
static inline int rdr_save_part_file(char *filename, const void *data, void *phy_addr, u32 part_length, u32 file_length)
{
    return -1;
}
static inline int rdr_init_trans_part_file_info(char *filename, u32 file_length)
{
    return 0;
}
static inline unsigned int is_reboot_reason_from_modem(void)
{
    return 0;
}
static inline char *rdr_get_reboot_logpath(void)
{
    return NULL;
}

static inline s32 rdr_create_exc_dir(const char *dir_name, char *dstpath, u32 dstsize, const char *date)
{
    return -1;
}
#endif


#endif /* __HIBOOT__ */

#endif /* __RDR_PUB_H__ */
