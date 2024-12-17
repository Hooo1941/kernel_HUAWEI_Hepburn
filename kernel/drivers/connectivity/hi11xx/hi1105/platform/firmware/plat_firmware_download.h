/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: plat_firmware_download.c header file
 * Author: @CompanyNameTag
 */

#ifndef PLAT_FIRMWARE_DOWNLOAD_H
#define PLAT_FIRMWARE_DOWNLOAD_H

typedef int32_t (*firmware_downlaod_privfunc)(void);

int firmware_download_function(uint32_t which_cfg, hcc_bus *bus);
int32_t get_firmware_download_flag(void);
int firmware_download_function_priv(uint32_t which_cfg, firmware_downlaod_privfunc, hcc_bus *bus);

#endif /* PLAT_FIRMWARE_DOWNLOAD_H */
