/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: wifi_exception_res.c header file
 * Author: @CompanyNameTag
 */

#ifndef WIFI_EXCEPTION_RST_H
#define WIFI_EXCEPTION_RST_H

#include "oal_hcc_bus.h"

void wifi_memdump_finish(void);
int32_t wifi_notice_hal_memdump(void);
int32_t wifi_exception_mem_dump(hcc_bus *hi_bus);
int32_t wifi_memdump_enquenue(uint8_t *buf_ptr, uint16_t count);
int32_t get_wifi_subsys_state(excp_comm_t *node_ptr);
int32_t wifi_reset_pre_do(excp_comm_t *node_ptr, excp_record_t *excp_record);
int32_t wifi_power_reset(excp_comm_t *node_ptr);
void wifi_excp_power_off(excp_comm_t *node_ptr, uint8_t flag);
int32_t wifi_firmware_download_pre(excp_comm_t *node_ptr);
int32_t wifi_firmware_download_do(excp_comm_t *node_ptr);
int32_t wifi_firmware_download_post(excp_comm_t *node_ptr);
int32_t wifi_reset_post_do(excp_comm_t *node_ptr, excp_record_t *excp_record);
#endif
