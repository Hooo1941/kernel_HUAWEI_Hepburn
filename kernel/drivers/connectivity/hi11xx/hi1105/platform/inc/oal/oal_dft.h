/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: oal_dft.c header file
 * Author: @CompanyNameTag
 */

#ifndef OAL_DFT_H
#define OAL_DFT_H
#include "oal_types.h"

#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT
void oal_dft_print_error_key_info(void);
void oal_dft_print_all_key_info(void);
#endif

int32_t oal_dft_init(void);
void oal_dft_exit(void);

#endif /* end of oal_dtf.h */
