/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: mp13_exception_rst.c header file
 * Author: @CompanyNameTag
 */

#ifndef MP13_EXCEPTION_RST_H
#define MP13_EXCEPTION_RST_H

excp_chip_cfg_t *get_mp13_chip_cfg(void);
int32_t firmware_download_mode_combo_mp13(excp_comm_t *node_ptr);

int32_t mp13_wifi_subsys_reset(excp_comm_t *node_ptr);
int32_t mp13_bfgx_subsys_reset(excp_comm_t *node_ptr);
#endif
