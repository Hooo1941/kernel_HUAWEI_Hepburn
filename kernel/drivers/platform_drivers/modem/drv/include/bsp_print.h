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

#ifndef __BSP_PRINT_H__
#define __BSP_PRINT_H__

#include <product_config.h>
#include <stdarg.h>
#include <drv_comm.h>
#include <mdrv_print.h>
#include <bsp_ring_buffer.h>

#ifdef __cplusplus
extern "C" {
#endif


/* ****************************
 * 宏定义
 * *************************** */
#undef MOD
#define MOD(x) #x

#undef BSP_MOD
#if defined(BSP_CONFIG_PHONE_TYPE)
#define BSP_MOD(x) (MOD(x))
#else
#define BSP_MOD(x) ((MOD(x)) + 4)
#endif
/* ****************************
 * 枚举定义
 * BSP_PRINT_OFF: 表示打印关闭
 * BSP_P_FATAL的打印级别最高
 * *************************** */
enum bsp_log_level {
    BSP_PRINT_OFF = 0,
    BSP_P_FATAL = 1,
    BSP_P_ERR,
    BSP_P_WRN,
    BSP_P_INFO,
    BSP_P_DEBUG,
    BSP_LEVEL_BUTT
};

/* module_tag:模块名，格式必须为 mod_xxx */
typedef enum _module_tag {
    /* mdrv_1 start */
    mod_amon,
    mod_mid,
    mod_noc,
    mod_qic,
    mod_dmss,
    mod_rdr,
    mod_adump,
    mod_mdump,
    mod_watchpoint,
    mod_coresight,
    mod_print,
    mod_userlog,
    mod_pdlock,
    mod_dlock,
    mod_sci,
    mod_dump,
    mod_nrrdr,
    mod_homi,
    mod_omp,
    mod_diag,
    mod_diag_sys,
    mod_diag_dbg,
    mod_diag_port,
    mod_scm,
    mod_ppm,
    mod_soft_dec,
    mod_iqi,
    mod_odt,
    mod_bbpds,
    mod_diag_vcom,
    mod_version,
    mod_nv,
    mod_nva,
    mod_chr,
    mod_deflate,
    mod_dsp,
    mod_dspdfs,
    mod_dspdvs,
    mod_nrdsp,
    mod_mailbox,
    mod_mailbox_nr,
    mod_xmailbox,
    mod_dsp_dvfs,
    mod_cpm,
    mod_nrcpm,
    mod_simhotplug,
    mod_busprobe,
    mod_ccpudebug,
    /* mdrv_1 end */
    /* mdrv_2 start */
    mod_emi,
    mod_pwm,
    mod_security,
    mod_coul,
    mod_blk,
    mod_mtd,
    mod_fmc,
    mod_ptable,
    mod_anten,
    mod_rffe,
    mod_abb,
    mod_memrepair,
    mod_mipi,
    mod_power,
    mod_bbp,
    mod_gpio,
    mod_pintrl,
    mod_mmc,
    mod_mloader,
    mod_pinctrl,
    mod_adc,
    mod_yaffs,
    mod_ubi,
    mod_hkadc,
    mod_temperature,
    mod_easyrf,
    mod_efuse,
    mod_vsim,
    mod_lcd,
    mod_leds,
    mod_rtc,
    mod_tsensor,
    mod_rfile,
    mod_hifi,
    mod_onoff,
    mod_sec,
    mod_ios,
    mod_key,
    mod_i2c,
    mod_spi,
    mod_spi_secondary,
    mod_load_fpga,
    mod_audio,
    mod_charger,
    mod_aslr,
    mod_gnss,
    mod_modem_boot,
    mod_modem_load,
    mod_sysboot,
    mod_sound,
    mod_crypto_engine,
    mod_crypto_gm,
    mod_dload,
    mod_dfxcert,
    /* mdrv_2 end */
    /* mdrv_3 start */
    mod_nmi,
    mod_thermal,
    mod_init,
    mod_l2cache,
    mod_osal,
    mod_osl,
    mod_cpufreq,
    mod_busfreq,
    mod_loadmon,
    mod_aptimer,
    mod_slice,
    mod_cpuidle,
    mod_archpm,
    mod_pmom,
    mod_avs,
    mod_sys_bus,
    mod_sysctrl,
    mod_wakelock,
    mod_wdt,
    mod_clk,
    mod_dpm,
    mod_of,
    mod_edma,
    mod_perf_stat,
    mod_mperf,
    mod_pm,
    mod_nr_pm,
    mod_nr_dpm,
    mod_nr_wakesource,
    mod_hotplug,
    mod_hwadp,
    mod_malloc_m,
    mod_mem,
    mod_pmu,
    mod_pmctrl,
    mod_regulator_m,
    mod_reset_m,
    mod_rsracc,
    mod_softtimer,
    mod_sync,
    mod_s_mem,
    mod_vic,
    mod_aximem,
    mod_memory,
    mod_ocp,
    /* mdrv_3 end */
    /* mdrv_4 start */
    mod_pfa_tft,
    mod_wan,
    mod_pss,
    mod_dra,
    mod_syscache,
    mod_ipsec,
    mod_pcdev,
    mod_pmdm,
    mod_ptty,
    mod_pcie,
    mod_pcie_nic,
    mod_pfa,
    mod_xlat,
    mod_pfa_v7,
    mod_socket,
    mod_ecipher,
    mod_hcode,
    mod_cipher,
    mod_udi,
    mod_ppp_ndis,
    mod_rt1711,
    mod_gmac,
    mod_vcom,
    mod_relay,
    mod_hmi,
    mod_btest,
    mod_dpa,
    /* mdrv_4 end */

    /* mdrv_5 start */
    mod_tzpc,
    mod_dialup_uart,
    mod_ipc,
    mod_ipc_msg,
    mod_ipcmsg,
    mod_icc,
    mod_fipc,
    mod_uart,
    mod_msg,
    mod_dfc,
    mod_at_uart,
    mod_cshell,
    mod_dynmem_rpt,
    mod_llt_tool,
    mod_llt_load,
    mod_ecdc,
    mod_sec_call,
    mod_reset,
    mod_mcore_vfs,
    mod_test,
    mod_mprobe,
    mod_vbat,
    mod_bkip,
    /* mdrv_5 end */
    mod_vlan,
    mod_rpmb,
    mod_all,        /* 代表所有的模块 */
    MODU_MAX = 256, /* 边界值 */
} module_tag_e;

typedef int (*bsp_log_string_pull_func)(unsigned int level, const char *fmt, va_list arglist);

typedef enum {
    LOGMEM_CCPUBOOT = 0,
    LOGMEM_CCPU,
    LOGMEM_ONCHIP,
    LOGMEM_XLOAER,
    LOGMEM_HIBOOT,
    LOGMEM_MCUBOOT,
    LOGMEM_MCU,
    LOGMEM_ACPUBOOT,
    LOGMEM_ACPU,
    LOGMEM_CCPU_TEE,
    LOGMEM_BUTT
} syslog_logmem_module_e;

typedef struct {
    char* virt_addr;
    unsigned long phy_addr;
    unsigned int length;
} syslog_logmem_area_info_s;

/* ****************************
 * 结构体定义
 * *************************** */
struct bsp_syslevel_ctrl { // 各模块设置默认打印级别
    unsigned int con_level;
    unsigned int logbuf_level;
};

struct bsp_print_tag {
    unsigned int modlevel;
};

struct bsp_earlylog_info {
    unsigned char *data_addr;
    unsigned int data_len;
};
struct log_usr_info;

/* if return val == 0 success, else fail */
typedef int (*USR_READ_FUNC)(struct log_usr_info *usr_info, char *buf, u32 count);

/* if return val == 0 success, else fail */
typedef int (*USR_OPEN_FUNC)(struct log_usr_info *usr_info);

/* if return val == 0 success, else fail */
typedef int (*USR_CLOSE_FUNC)(struct log_usr_info *usr_info);

/**
 * struct log_mem - represents shared information between reader & writer
 * This structure is allocated by writer
 */
struct log_mem {
    u32 magic;         /* magic number: protect memory  */
    u32 write;         /* write offset of ring buffer */
    u32 read;          /* read offset of ring buffer */
    u32 size;          /* ring buffer size */
    u32 app_is_active; /* reader's ready flag */
};

/**
 * struct log_usr_info - represents user information which register to modem log
 * This structure is allocated by writer
 */
struct log_usr_info {
    char               *dev_name;     /* misc device name */
    u32                mem_is_ok;     /* whether are two momory block(ring buffer & ring buffer info) initilized  */
    char               *ring_buf;     /* The data area(ring buffer) */
    struct log_mem     *mem;          /* The management area(ring buffer info) */
    u32                wake_interval; /* The interval of wakeup acore to trigger log file write, just used in ccore */
    USR_READ_FUNC      read_func;     /* user function in log read method */
    USR_OPEN_FUNC      open_func;     /* user function in log open method */
    USR_CLOSE_FUNC     close_func;     /* user function in log release method */
};
/**
 * bsp_modem_log_init - for ccore init decleration
 */
int bsp_modem_log_init(void);
/**
 * bsp_modem_log_register - tell modem log which user information is
 * @usr_info: information which modem log need to know
 *
 * Returns 0 if success
 */
s32 bsp_modem_log_register(struct log_usr_info *usr_info);

/**
 * modem_log_ring_buffer_get - get generic ring buffer(struct ring buffer) from struct log_usr_info
 * @usr_info: information regitered to modem log
 * @rb: generic ring buffer info
 */
void modem_log_ring_buffer_get(struct log_usr_info *usr_info, struct ring_buffer *rb);

/**
 * bsp_modem_log_wakeup_one - for modem save
 * @usr_info: information regitered to modem log
 */
void bsp_modem_log_wakeup_one(struct log_usr_info *usr_info);

/* ****************************
 * 函数声明
 * *************************** */
#if defined(ENABLE_BUILD_PRINT)
void bsp_print(module_tag_e modid, enum bsp_log_level level, char *fmt, ...) __attribute__((format(printf, 3, 4)));
void bsp_print_send_hids(unsigned int level, const char *print_buf, va_list arglist);
#else
__attribute__((format(printf, 3, 4))) static inline void bsp_print(module_tag_e modid, enum bsp_log_level level,
    char *fmt, ...)
{
    return;
}
static inline void bsp_print_send_hids(unsigned int level, const char *print_buf, va_list arglist)
{
    return;
}
#endif
/*
logl设置HIDS和控制台的系统默认打印级别
logm设置模块打印级别
logc查询打印级别
*/
int logs(unsigned int console, unsigned int logbuf);
int logm(unsigned int modid, unsigned int level);
void logc(unsigned int modid);

void __bsp_print(unsigned int modid, unsigned int level, const char *fmt, va_list arglist);
#define bsp_fatal(fmt, ...) (bsp_print(THIS_MODU, BSP_P_FATAL, "[%s]:" fmt, BSP_MOD(THIS_MODU), ##__VA_ARGS__))
#define bsp_err(fmt, ...) (bsp_print(THIS_MODU, BSP_P_ERR, "[%s]:" fmt, BSP_MOD(THIS_MODU), ##__VA_ARGS__))
#define bsp_wrn(fmt, ...) (bsp_print(THIS_MODU, BSP_P_WRN, "[%s]:" fmt, BSP_MOD(THIS_MODU), ##__VA_ARGS__))
#define bsp_info(fmt, ...) (bsp_print(THIS_MODU, BSP_P_INFO, "[%s]:" fmt, BSP_MOD(THIS_MODU), ##__VA_ARGS__))
#define bsp_debug(fmt, ...) (bsp_print(THIS_MODU, BSP_P_DEBUG, "[%s]:" fmt, BSP_MOD(THIS_MODU), ##__VA_ARGS__))

void bsp_store_log_buf(char *buf, unsigned int len);
void bsp_log_string_pull_hook(bsp_log_string_pull_func hook);
int bsp_print_hook_register(mdrv_print_hook hook);
int bsp_print_hook_unregister(int hook_id);

unsigned int bsp_print_get_len_limit(void);
struct bsp_syslevel_ctrl bsp_print_get_level(void);
void dump_save_last_kmsg(void);
void print_register_last_kmsg(void);

int bsp_syslog_get_logmem_info(syslog_logmem_module_e type, syslog_logmem_area_info_s* info);
int bsp_syslog_save_stage_log(syslog_logmem_module_e stage);

#ifdef __cplusplus
}
#endif

#endif
