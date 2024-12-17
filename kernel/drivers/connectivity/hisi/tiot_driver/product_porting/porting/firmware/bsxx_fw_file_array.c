/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 *
 * Description:  bsxx firmware dummy file arrays.
 * Author: @CompanyNameTag
 * History:
 * 2023-11-01, Create file.
 */
#include "bsxx_fw_file_array.h"
#include "tiot_types.h"

#define BSXX_FW_ATTR

BSXX_FW_ATTR const char g_bsxx_cfg_file[] = {
    0x00, 0x00, 0x00, 0x00
};

const tiot_file g_bsxx_fw_files[] = {
    { "cfg", sizeof(g_bsxx_cfg_file), g_bsxx_cfg_file },
};

const tiot_file_path g_bsxx_fw_file_path = {
    g_bsxx_fw_files, sizeof(g_bsxx_fw_files) / sizeof(tiot_file)
};

const tiot_file_path *bsxx_fw_file_path_get(void)
{
    return &g_bsxx_fw_file_path;
}
