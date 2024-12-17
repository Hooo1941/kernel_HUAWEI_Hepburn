/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: bfgx_exception_rst.c header file
 * Author: @CompanyNameTag
 */

#ifndef BFGX_EXCEPTION_RST_H
#define BFGX_EXCEPTION_RST_H

/* 其他头文件包含 */
#include "oal_schedule.h"
#include "oal_workqueue.h"
#include "hw_bfg_ps.h"
#include "exception_common.h"

/* plat cfg cmd */
#define PLAT_EXCEPTION_ENABLE  1
#define PLAT_EXCEPTION_DISABLE 0

#define MPXX_DFR_WAIT_SSI_IDLE_MS 5000

typedef enum {
    DFR_HAL_GNSS = 0,
    DFR_HAL_BT = 1,
    DFR_HAL_FM = 2,
    DFR_HAL_IR = 3,
    PLAT_BFGX_DUMP_FILE_READ = 4,
    PLAT_ME_DUMP_FILE_READ = 5,

    PLAT_BFGX_EXCP_CMD_BOTT
} plat_bfgx_excp_cmd;

typedef enum {
    DFR_HAL_WIFI = 0,
    PLAT_WIFI_DUMP_FILE_READ = 1,
    PLAT_GT_DUMP_FILE_READ = 2,
} plat_wifi_excp_cmd;

enum plat_cfg {
    PLAT_DFR_CFG = 0,
    PLAT_BEATTIMER_TIMEOUT_RESET_CFG = 1,
    PLAT_CFG_BUFF,

    PLAT_DUMP_FILE_READ = 100,
    PLAT_DUMP_ROTATE_FINISH = 101
};

void plat_beat_timeout_reset_set(unsigned long arg);
void bfgx_memdump_finish(struct ps_core_s *ps_core_d);
int32_t bfgx_notice_hal_memdump(struct ps_core_s *ps_core_d);
int32_t bfgx_memdump_enquenue(struct ps_core_s *ps_core_d, uint8_t *buf_ptr, uint16_t count);
void bfgx_beat_timer_expire(oal_timeout_func_para_t data);
void bfgx_dump_stack(struct ps_core_s *ps_core_d);
int32_t bfgx_master_core_reset_pre_do(excp_comm_t *node_ptr, excp_record_t *excp_record);
int32_t bfgx_slave_core_reset_pre_do(excp_comm_t *node_ptr, excp_record_t *excp_record);
int32_t bfgx_master_core_download_fail_do(excp_comm_t *node_ptr);
int32_t bfgx_slave_core_download_fail_do(excp_comm_t *node_ptr);
int32_t bfgx_master_core_firmware_download_post(excp_comm_t *node_ptr);
int32_t bfgx_master_core_reset_post_do(excp_comm_t *node_ptr, excp_record_t *excp_record);
int32_t bfgx_slave_core_reset_post_do(excp_comm_t *node_ptr, excp_record_t *excp_record);
int32_t get_bfgx_subsys_state(excp_comm_t *node_ptr);
int32_t bfgx_reset_pre_do(excp_comm_t *node_ptr, excp_record_t *excp_record);
int32_t bfgx_subsys_power_reset(excp_comm_t *node_ptr);
void bfgx_subsys_power_off_proc(excp_comm_t *node_ptr, uint8_t flag, uint32_t sys);
int32_t bfgx_firmware_download_do(excp_comm_t *node_ptr);
int32_t bfgx_download_fail_do(excp_comm_t *node_ptr);
int32_t bfgx_firmware_download_post(excp_comm_t *node_ptr);
int32_t bfgx_reset_post_do(excp_comm_t *node_ptr, excp_record_t *excp_record);
#endif
