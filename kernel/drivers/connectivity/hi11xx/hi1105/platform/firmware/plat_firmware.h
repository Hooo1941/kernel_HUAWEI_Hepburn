/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: plat_firmware.c header file
 * Author: @CompanyNameTag
 */

#ifndef PLAT_FIRMWARE_H
#define PLAT_FIRMWARE_H

/* 头文件包含 */
#include "plat_type.h"
#include "oal_net.h"
#include "oal_hcc_bus.h"
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_exception_rst.h"
#endif
#include "plat_firmware_util.h"

#define READ_MEG_JUMP_TIMEOUT 15000 /* 15s */

#define FILE_CMD_WAIT_TIME_MIN 5000 /* 5000us */
#define FILE_CMD_WAIT_TIME_MAX 5100 /* 5100us */


#define VERSION_LEN  64

#define READ_CFG_BUF_LEN 4096

#define DOWNLOAD_CMD_LEN      32
#define DOWNLOAD_CMD_PARA_LEN 800

#define SHUTDOWN_TX_CMD_LEN 64

#define CMD_JUMP_EXEC_RESULT_SUCC 0
#define CMD_JUMP_EXEC_RESULT_FAIL 1

#define WIFI_MODE_DISABLE 0
#define WIFI_MODE_2G      1
#define WIFI_MODE_5G      2
#define WIFI_MODE_2G_5G   3

/* mp13,38.4M 共时钟请求控制 */
#define STR_REG_PMU_CLK_REQ "0x50000350"

/* 以下是cfg文件配置命令的参数头，一条合法的cfg命令格式为:参数头+命令关键字(QUIT命令除外) */
#define FILE_TYPE_CMD_KEY "ADDR_FILE_"
#define NUM_TYPE_CMD_KEY  "PARA_"

#define CFG_INFO_RESERVE_LEN 8

#ifdef _PRE_WINDOWS_SUPPORT
#define BFGX_AND_WIFI_CFG_MP13_PILOT_PATH       "bfgx_and_wifi_cfg"
#define WIFI_CFG_MP13_PILOT_PATH                "wifi_cfg"
#define BFGX_CFG_MP13_PILOT_PATH                "bfgx_cfg"
#else
#define BFGX_AND_WIFI_CFG_MP13_PILOT_PATH  "/vendor/firmware/hi1103/pilot/bfgx_and_wifi_cfg"
#define WIFI_CFG_MP13_PILOT_PATH           "/vendor/firmware/hi1103/pilot/wifi_cfg"
#define BFGX_CFG_MP13_PILOT_PATH           "/vendor/firmware/hi1103/pilot/bfgx_cfg"
#endif
#define MP13_PILOT_BOOTLOADER_VERSION "Hi1103V100R001C01B083 Dec 16 2017"

#define MP15_PILOT_BOOTLOADER_VERSION "Hi1105V100R001C01B083 Dec 16 2019"

/* mp15 asic cfg file path */
#ifdef _PRE_WINDOWS_SUPPORT
#define BFGX_AND_WIFI_CFG_MP15_ASIC_PATH       "bfgx_and_wifi_cfg"
#define WIFI_CFG_MP15_ASIC_PATH                "wifi_cfg"
#define BFGX_CFG_MP15_ASIC_PATH                "bfgx_cfg"
#define RAM_CHECK_CFG_MP15_ASIC_PATH           "ram_reg_test_cfg"
#define RAM_BCPU_CHECK_CFG_MP15_ASIC_PATH      "reg_bcpu_mem_test_cfg"
#else
#define BFGX_AND_WIFI_CFG_MP15_ASIC_PATH  "/vendor/firmware/hi1105/pilot/bfgx_and_wifi_cfg"
#define WIFI_CFG_MP15_ASIC_PATH           "/vendor/firmware/hi1105/pilot/wifi_cfg"
#define BFGX_CFG_MP15_ASIC_PATH           "/vendor/firmware/hi1105/pilot/bfgx_cfg"
#endif

/* mp16 fpga cfg file path */
#define BFGX_AND_WIFI_CFG_MP16_FPGA_PATH   "/vendor/firmware/shenkuo/fpga/bfgx_and_wifi_cfg"
#define WIFI_CFG_MP16_FPGA_PATH            "/vendor/firmware/shenkuo/fpga/wifi_cfg"
#define BFGX_CFG_MP16_FPGA_PATH            "/vendor/firmware/shenkuo/fpga/bfgx_cfg"

/* mp16 asic cfg file path */
#define BFGX_AND_WIFI_CFG_MP16_ASIC_PATH   "/vendor/firmware/shenkuo/pilot/bfgx_and_wifi_cfg"
#define WIFI_CFG_MP16_ASIC_PATH            "/vendor/firmware/shenkuo/pilot/wifi_cfg"
#define BFGX_CFG_MP16_ASIC_PATH            "/vendor/firmware/shenkuo/pilot/bfgx_cfg"

#ifndef FIRMWARE_GF61_CFG_DIR
#define FIRMWARE_GF61_CFG_DIR               "/vendor/firmware/hi1161/pilot"
#endif

/* cfg file name */
#define BFGX_AND_WIFI_CFG_FILE   "bfgx_and_wifi_cfg"
#define GT_CFG_FILE              "gt_asic_cfg"
#define SLE_CFG_FILE             "gle_cfg"
#define BI_CFG_FILE              "bi_cfg"
#define WIFI_CFG_FILE            "wifi_cfg"
#define IR_CFG_FILE              "ir_cfg"
#define BFGX_CFG_FILE            "bfgx_cfg"
#define RAM_WIFI_CHECK_FILE      "reg_wifi_mem_test_cfg"
#define RAM_GT_CHECK_FILE        "reg_gt_mem_test_cfg"
#define RAM_BGCPU_CHECK_FILE     "reg_bgcpu_mem_test_cfg"

#define FILE_COUNT_PER_SEND          1
#define MIN_FIRMWARE_FILE_TX_BUF_LEN 4096
#define MAX_FIRMWARE_FILE_TX_BUF_LEN (512 * 1024)

/* dump the device cpu reg mem when panic,24B mem header + 24*4 reg info */
#define CPU_PANIC_MEMDUMP_SIZE (24 + 24 * 4)

/* 枚举定义 */
enum firmware_cfg_cmd_enum {
    ERROR_TYPE_CMD = 0,     /* 错误的命令 */
    FILE_TYPE_CMD,          /* 下载文件的命令 */
    NUM_TYPE_CMD,           /* 下载配置参数的命令 */
    QUIT_TYPE_CMD           /* 退出命令 */
};

/*
 * 1.首次开机时，使用BFGN_AND_WIFI_CFG，完成首次开机校准，host保存校准数据，完成后device下电
 * 2.deivce下电状态，开wifi，使用BFGN_AND_WIFI_CFG
 * 3.deivce下电状态，开bt，使用BFGX_CFG
 * 4.bt开启，再开wifi，使用WIFI_CFG
 * 5.wifi开启，再开bt，通过sdio解复位BCPU
 */
enum firmware_cfg_file_enum {
    BFGX_AND_WIFI_CFG = 0, /* 加载BFGIN和wifi fimware的命令数组index，执行独立校准 */
    WIFI_CFG,              /* 只加载wifi firmware的命令数组索引，执行独立校准 */
    BFGX_CFG,              /* 只加载bfgx firmware的命令数组索引，不执行独立校准 */
    RAM_REG_TEST_CFG,      /* 产线测试mem reg遍历使用 */
    GT_CFG,
    SLE_CFG,
    BT_CFG,
    IR_CFG,
    CFG_FILE_TOTAL
};

/* STRUCT 定义 */
typedef struct cmd_type_st {
    int32_t cmd_type;
    uint8_t cmd_name[DOWNLOAD_CMD_LEN];
    uint8_t cmd_para[DOWNLOAD_CMD_PARA_LEN];
} cmd_type_info;

typedef struct {
    uint8_t **cfg_path;
    uint8_t *pre_directory;
    uint8_t from_heap_mem[CFG_FILE_TOTAL];
} cfg_file_str;

typedef struct firmware_globals_st {
    int32_t count[CFG_FILE_TOTAL];      /* 存储每个cfg文件解析后有效的命令个数 */
    cmd_type_info *cmd[CFG_FILE_TOTAL]; /* 存储每个cfg文件的有效命令 */
    uint8_t cfg_version[VERSION_LEN];   /* 存储cfg文件中配置的版本号信息 */
    uint8_t dev_version[VERSION_LEN];   /* 存储加载时device侧上报的版本号信息 */
    cfg_file_str file_info;
} firmware_globals;
extern firmware_globals g_cfg_info;

typedef struct firmware_number_type_st {
    uint8_t *key;
    int32_t (*cmd_exec)(uint8_t *key, uint8_t *value);
} firmware_number;

/* 函数声明 */
int32_t number_type_cmd_send(const char *key, const char *value);
int32_t sdio_read_mem(const char *key, const char *value, bool is_wifi);
int32_t firmware_read_cfg(const char *cfg_patch, uint8_t *buf, uint32_t buf_len);
int32_t firmware_parse_cmd(uint8_t *cfg_buffer, uint8_t *cmd_name, uint32_t cmd_name_len,
                           uint8_t *cmd_para, uint32_t cmd_para_len);
int32_t firmware_download_cmd(uint32_t idx, hcc_bus *bus);
int32_t firmware_cfg_init(void);
int32_t firmware_get_cfg(uint8_t *cfg_patch, uint32_t idx);
void firmware_cfg_clear(void);

int32_t parse_file_cmd(uint8_t *string, unsigned long *addr, char **file_path);
#endif /* end of plat_firmware.h */
