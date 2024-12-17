/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: plat_exception_rst.c header file
 * Author: @CompanyNameTag
 */

#ifndef PLAT_EXCEPTION_RST_H
#define PLAT_EXCEPTION_RST_H

/* 其他头文件包含 */
#include "plat_type.h"
#include "oal_net.h"
#include "oal_workqueue.h"
#include "exception_common.h"
#include "bfgx_exception_rst.h"
#include "wifi_exception_rst.h"

/* 宏定义 */
#define WAIT_BFGX_READ_STACK_TIME 10000 /* 等待bfgx读栈操作完成的时间，5000毫秒 */

#define PLAT_EXCEPTION_RESET_IDLE  0 /* plat 没有在处理异常 */
#define PLAT_EXCEPTION_RESET_BUSY  1 /* plat 正在处理异常 */

#define EXCEPTION_SUCCESS 0
#define EXCEPTION_FAIL    1

#define DFR_TEST_DISABLE 0
#define DFR_TEST_ENABLE  1

#define BFGX_MAX_RESET_CMD_LEN 9
#define BT_RESET_CMD_LEN       4
#define FM_RESET_CMD_LEN       1
#define GNSS_RESET_CMD_LEN     9
#define IR_RESET_CMD_LEN       4
#define NFC_RESET_CMD_LEN      4
#define SLE_RESET_CMD_LEN      6

#define MEMDUMP_ROTATE_QUEUE_MAX_LEN 10

#ifdef HISI_TOP_LOG_DIR
#define BFGX_DUMP_PATH HISI_TOP_LOG_DIR "/hi110x/memdump"
#define WIFI_DUMP_PATH HISI_TOP_LOG_DIR "/wifi/memdump"
#else
#define BFGX_DUMP_PATH "/data/memdump"
#define WIFI_DUMP_PATH "/data/memdump"
#endif

#define DEVICE_MEM_DUMP_MAX_BUFF_SIZE 100

/* STRUCT 定义 */
enum dfr_rst_system_type_e {
    DFR_SINGLE_SYS_RST = 0,
    DFR_ALL_SYS_RST = 1,
    DFR_SYSTEM_RST_TYPE_BOTT
};

enum subsystem_enum {
    SUBSYS_WIFI = 0,
    SUBSYS_BFGX = 1,
    SUBSYS_GT = 2,

    SUBSYS_BOTTOM
};

enum wifi_thread_enum {
    THREAD_WIFI = 0,

    WIFI_THREAD_BOTTOM
};

enum bfgx_thread_enum {
    THREAD_BT = 0,
    THREAD_FM = 1,
    THREAD_GNSS = 2,
    THREAD_IR = 3,
    THREAD_NFC = 4,
    THREAD_IDLE = 5,
    THREAD_SLE = 6,
    BFGX_THREAD_BOTTOM
};

enum exception_type_enum {
    BUART_PANIC = 0, /* 通过uart上传或下发的消息, 异常类型定义以通道名开头 */
    GUART_PANIC = 1,
    HCC_BUS_PANIC = 2,
    HCC_BUS_TRANS_FAIL = 3,
    BUART_WAKEUP_FAIL = 4,
    GUART_WAKEUP_FAIL = 5,
    WIFI_WAKEUP_FAIL = 6,
    GT_WAKEUP_FAIL = 7,

    BT_POWERON_FAIL = 8,
    FM_POWERON_FAIL = 9,
    GNSS_POWERON_FAIL = 10,
    IR_POWERON_FAIL = 11,
    NFC_POWERON_FAIL = 12,
    WIFI_POWERON_FAIL = 13,
    SLE_POWERON_FAIL = 14,

    /* bfgx */
    BUART_BEATHEART_TIMEOUT = 15, /* BUART上报的心跳超时 */
    GUART_BEATHEART_TIMEOUT = 16, /* GUART上报的心跳超时 */
    BFGX_TIMER_TIMEOUT = 17,     /* 定时器超时 */
    BFGX_ARP_TIMEOUT = 18,       /* arp verifier超时，有发无收 */

    /* wifi */
    WIFI_WATCHDOG_TIMEOUT = 19,  /* 看门狗超时 */
    SDIO_DUMPBCPU_FAIL = 20,

    /* gt */
    GT_WATCHDOG_TIMEOUT = 21,    /* 看门狗超时 */
    CHIP_FATAL_ERROR = 22,       /* 芯片致命问题, 单系统dfr不能恢复 */
    GUGONG_POWER_FAIL = 23,     /* 故宫上下电异常 */

    BT_POWER_OFF_FAIL = 24,
    FM_POWER_OFF_FAIL = 25,
    GNSS_POWER_OFF_FAIL = 26,
    IR_POWER_OFF_FAIL = 27,
    NFC_POWER_OFF_FAIL = 28,
    SLE_POWER_OFF_FAIL = 29,

    EXCEPTION_TYPE_BOTTOM = 30
};

enum dump_cmd_type {
    CMD_READM_WIFI_SDIO = 0,
    CMD_READM_WIFI_UART = 1,
    CMD_READM_BFGX_UART = 2,
    CMD_READM_BFGX_SDIO = 3,
    CMD_READM_ME_UART = 4,
    CMD_READM_ME_SDIO = 5,
    CMD_READM_GT_PCIE = 6,

    CMD_DUMP_BUFF
};

enum excp_test_cfg_em {
    BFGX_POWEON_FAULT = 0,
    BFGX_POWEOFF_FAULT = 1,
    BFGX_WKUP_FAULT = 2,
    WIFI_WKUP_FAULT = 3,
    WIFI_POWER_ON_FAULT = 4,
    HEARTBEATER_TIMEOUT_FAULT = 5,
    EXCP_TEST_CFG_BUTT
};

struct st_wifi_dump_mem_info {
    unsigned long mem_addr;
    uint32_t size;
    uint8_t *file_name;
};

struct st_wifi_dfr_callback {
    void (*wifi_recovery_complete)(uint32_t proc_result);
    void (*notify_wifi_to_recovery)(void);
};

struct st_bfgx_reset_cmd {
    uint32_t len;
    uint8_t cmd[BFGX_MAX_RESET_CMD_LEN];
};

struct excp_type_info_s {
    int32_t excp_cnt;
    uint32_t fail_cnt;
    uint32_t rst_type_cnt[DFR_SYSTEM_RST_TYPE_BOTT];
    ktime_t stime;
    unsigned long long maxtime;
};

typedef struct {
    uint32_t (*is_ir_mode_need_dfr)(uint32_t subsys_type);
    uint32_t (*is_chip_force_reset)(void);
    uint32_t (*is_single_wifi)(void);
    int32_t (*get_power_state)(excp_comm_t *node_ptr);
    int32_t (*pre_do)(excp_comm_t *node_ptr, excp_record_t *excp_record);
    int32_t (*power_reset)(excp_comm_t *node_ptr);
    void (*power_off)(excp_comm_t *node_ptr, uint8_t flag);
    int32_t (*firmware_download_pre)(excp_comm_t *node_ptr);
    int32_t (*firmware_download_do)(excp_comm_t *node_ptr);
    int32_t (*firmware_download_mode_combo)(excp_comm_t *node_ptr);
    int32_t (*firmware_download_fail_do)(excp_comm_t *node_ptr);
    int32_t (*firmware_download_post)(excp_comm_t *node_ptr);
    int32_t (*post_do)(excp_comm_t *node_ptr, excp_record_t *excp_record);
} reset_handler_t;

typedef struct {
    uint32_t exception_type;
    uint32_t subsys_type;
    uint32_t core_type;
    void (*get_subsys_core_func)(excp_record_t*);
} excp_search_table_t;

typedef struct {
    uint32_t exception_type;
    uint32_t bus_id;
} excp_panic_table_t;

typedef struct {
    uint32_t sla_core_idx;
    uint32_t sla_core_type;
    uint32_t bus_id;  /* 通过bus_id获取pm_data */
    const uint8_t *sla_core_name;
    reset_handler_t *sla_reset_func;
} excp_sla_core_cfg_t;

/* 主核不对自己处理，若主核异常则上升到子系统进行恢复 */
typedef struct {
    uint32_t master_core_type;
    const uint8_t *master_core_name;
    reset_handler_t *master_reset_func;
    uint32_t sla_core_cnt;
    excp_sla_core_cfg_t *sla_core_cfg;
} excp_core_cfg_t;

typedef struct {
    uint32_t subsys_idx;
    const uint8_t *subsys_name;
    uint32_t subsys_type;
    uint32_t which_cfg;
    uint32_t chr_id;  /* chr event id */
    uint32_t bus_id;  /* 通过bus_id获取pm_data */
    reset_handler_t *subsys_reset_func;
    uint32_t slave_pm_flag; /* 是否需要注册从核pm, true:需要, false:不需要 */
    excp_core_cfg_t *core_cfg;
} excp_subsys_cfg_t;

typedef struct {
    uint32_t chip_type;
    const uint8_t *chip_name;
    uint32_t subsys_cnt;
    excp_subsys_cfg_t *subsys_cfg;
    reset_handler_t *chip_reset_func;
    excp_search_table_t *excp_search; /* 数组：异常类型查询表 */
    uint32_t excp_search_size; /* 数组excp_search大小 */
    uint32_t *bfgx_poweron_fail; /* 数组：bfgx上电失败对应异常类型 */
    uint32_t poweron_fail_size;  /* bfgx_poweron_fail数组大小 */
    uint32_t *bfgx_poweroff_fail; /* 数组：bfgx下电失败对应异常类型 */
    uint32_t poweroff_fail_size;  /* bfgx_poweroff_fail数组大小 */
    uint32_t *excp_sys_map; /* 数组：系统映射表 */
    uint32_t sys_map_size;  /* 数组excp_sys_map大小 */
    excp_panic_table_t *panic_table; /* 数组：panic查询表 */
    uint32_t panic_table_size; /* 数组panic_table大小  */
} excp_chip_cfg_t;

typedef enum {
    EXCP_SYS0 = 0x0,
    EXCP_SYS1 = 0x1,
    EXCP_SYS2,
    EXCP_SYS_IDX_BUTT
} excp_subsys_enum;

typedef enum {
    EXCP_CORE0 = 0x0,
    EXCP_CORE_IDX_BUTT
} excp_core_idx_enum;

typedef enum {
    EXCP_W_SYS = 0x0, /* w_cpu */
    EXCP_B_SYS = 0x1, /* b_cpu, MP16: b_cpu & g_cpu */
    EXCP_GLF_SYS,     /* MP16C: sle & gf */
    EXCP_UNKOWN_SYS,  /* 未知子系统, 待查询 */
    EXCP_ALL_SYS,     /* 全系统 */
    EXCP_SYS_BUTT,
} excp_sys_enum;

typedef enum {
    W_CPU = 0x0,
    B_CPU = 0x1,
    SLE_CPU,  /* MP16C */
    GF_CPU,   /* MP16C */
    G_CPU,    /* MP16 */
    UNKOWN_CORE,
    ALL_CORE,
    CORE_BUTT
} excp_core_enum;

typedef struct {
    excp_sla_core_cfg_t *sla_cfg;
    excp_comm_t node;
    void *sla_core_pm; /* 从核pm资源: 低功耗timer, 上电ack, 睡眠唤醒work */
} excp_sla_res_t;

/* 主核不对自己处理，若主核异常则上升到子系统进行恢复 */
typedef struct {
    excp_core_cfg_t *master_cfg;
    void *master_pm;   /* 主核pm资源: 低功耗timer, 上电ack, 睡眠唤醒work */
    uint32_t sla_core_cnt;
    excp_sla_res_t *sla_cores;
    excp_comm_t node;
    /* 维测 */
    uint32_t sla_excp_total_cnt; /* 所有从core异常次数 */
    uint32_t *sla_excp_arr; /* 各从core级异常次数 */
    uint32_t sla_excp_size; /* sla_excp_arr size */
    uint32_t sla_excp_stats[EXCEPTION_TYPE_BOTTOM]; /* 造成从core异常的异常类型统计 */
} excp_core_res_t;

typedef struct {
    excp_subsys_cfg_t *subsys_cfg;
    excp_core_res_t *core_attr;
    void *subsys_pm;   /* 主核pm资源: 低功耗timer, 上电ack, 睡眠唤醒work */
    excp_comm_t node;

    /* 维测 */
    uint32_t subsys_excp_cnt; /* 子系统级异常次数 */
    uint32_t subsys_excp_stats[EXCEPTION_TYPE_BOTTOM]; /* 造成子系统异常的异常类型统计 */
} excp_subsys_res_t;

typedef struct {
    excp_chip_cfg_t *chip_cfg;
    uint32_t subsys_cnt;
    excp_subsys_res_t *subsys_attr;
    excp_comm_t node;
    struct blocking_notifier_head nh;

    /* 维测 */
    uint32_t chip_excp_cnt; /* 芯片级异常次数 */
    uint32_t chip_excp_stats[EXCEPTION_TYPE_BOTTOM]; /* 造成芯片级异常的异常类型统计 */
} excp_chip_res_t;

typedef excp_chip_cfg_t* (*CHIP_CFG_FUNC)(void);

struct st_exception_info {
    excp_chip_res_t *excp_chip; /* 暂时将chip级异常放在st_exception_info内, 后面考虑二者合二为一 */
    excp_record_t record;
    uint32_t exception_reset_enable;
    uint32_t subsys_type;
    uint32_t thread_type;
    uint32_t excetion_type;

    atomic_t is_reseting_device;

    oal_workqueue_stru *plat_exception_rst_workqueue;
    oal_work_stru plat_exception_rst_work;
    oal_wakelock_stru plat_exception_rst_wlock;

    struct st_wifi_dfr_callback *wifi_callback;

    oal_spin_lock_stru exception_spin_lock;

    struct excp_type_info_s etype_info[EXCEPTION_TYPE_BOTTOM];

    /* 下边的变量调试使用 */
    uint32_t subsystem_rst_en;
    uint32_t sys_rst_download_dbg;  /* 用于构造下载失败场景, dfr下电 */
    uint32_t sys_rst_uart_suspend;  /* 用于构造uart suspend场景, dfr下电 */

    /* wifi异常触发 */
    oal_work_stru wifi_excp_worker;
    oal_work_stru wifi_excp_recovery_worker;
    oal_workqueue_stru *wifi_exception_workqueue;
    uint32_t wifi_excp_type;
};

typedef struct excp_info_str_s {
    uint32_t id;
    char *name;
} excp_info_str_t;

typedef struct {
    uint32_t chip_cnt;
    uint32_t subsys_cnt;
    uint32_t core_cnt;
} excp_cnt_t;

typedef struct {
    int32_t enable;
    uintptr_t data;
} excp_test_cfg_t;

/* EXTERN FUNCTION */
int32_t mod_beat_timer(uint8_t on);
int32_t is_bfgx_exception(void);
struct st_exception_info *get_exception_info_reference(void);
int32_t wait_bfgx_memdump_complete(struct ps_core_s *ps_core_d);
int32_t plat_exception_handler(uint32_t subsys_type, uint32_t thread_type, uint32_t exception_type);
int32_t plat_wifi_exception_rst_register(void *data);
int32_t plat_exception_reset_init(void);
int32_t plat_exception_reset_exit(void);
int32_t wifi_exception_work_submit(hcc_bus *hi_bus, uint32_t wifi_excp_type);
int32_t plat_power_fail_info_set(uint32_t subsys_type, uint32_t thread_type, uint32_t exception_type);
void plat_power_fail_process_done(uint32_t excp_type);
int32_t plat_get_excp_total_cnt(excp_cnt_t *excp_cnt);
int32_t in_plat_exception_reset(void);
int32_t plat_get_dfr_sinfo(char *buf, int32_t index);
int32_t is_dfr_test_en(enum excp_test_cfg_em excp_cfg, uintptr_t data);
void set_excp_test_en(enum excp_test_cfg_em excp_cfg, uintptr_t data);
void cancel_excp_test_en(enum excp_test_cfg_em excp_cfg, uintptr_t data);
oal_workqueue_stru *wifi_get_exception_workqueue(void);
int32_t gt_device_mem_dump(hcc_bus *hi_bus, int32_t excep_type);
int32_t wifi_device_mem_dump(hcc_bus *hi_bus, int32_t excep_type);
uint32_t get_panic_excp_by_uart_idx(uint32_t uart_idx);
uint32_t get_wakeup_excp_by_uart_idx(uint32_t uart_idx);
uint32_t get_poweron_fail_excp(uint32_t sys);
uint32_t get_poweroff_fail_excp(uint32_t sys);

int32_t subsys_all_core_pre_proc(excp_comm_t *node_ptr, excp_record_t *excp_record);
int32_t subsys_all_core_download_fail_do(excp_comm_t *node_ptr);
int32_t subsys_all_core_firmware_download_post(excp_comm_t *node_ptr);
int32_t subsys_all_core_reset_post_do(excp_comm_t *node_ptr, excp_record_t *excp_record);
int32_t chip_mpxx_pre_do(excp_comm_t *node_ptr, excp_record_t *excp_record);
int32_t chip_mpxx_power_reset(excp_comm_t *node_ptr);
int32_t chip_mpxx_firmware_download_pre(excp_comm_t *node_ptr);
int32_t chip_mpxx_firmware_download_do(excp_comm_t *node_ptr);
int32_t chip_mpxx_firmware_download_post(excp_comm_t *node_ptr);
int32_t chip_mpxx_post_do(excp_comm_t *node_ptr, excp_record_t *excp_record);
int32_t chip_reset_main(excp_record_t *excp_record, excp_chip_res_t *chip_res);
void excp_pm_data_register(void);
int dfr_notifier_chain_register(struct notifier_block *nb);
int dfr_notifier_chain_unregister(struct notifier_block *nb);
int dfr_notifier_chain_call(unsigned long excp_type);
#endif
