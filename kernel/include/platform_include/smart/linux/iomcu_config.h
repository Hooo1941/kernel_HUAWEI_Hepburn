/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
 * Description: Contexthub share memory driver
 * Create: 2019-10-01
 */
#ifndef __IOMCU_CONFIG_H__
#define __IOMCU_CONFIG_H__

#include <platform_include/smart/linux/base/ap/ddr_config/ddr_config.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief get iomcu_config by id.
 *
 * @return NA
 */
void *iomcu_config_get(int id, int len);

/**
 * @brief write timestamp to iomcu_config.
 *
 * @return NA
 */
void write_timestamp_base_to_sharemem(void);

#ifdef __cplusplus
}
#endif
#endif
