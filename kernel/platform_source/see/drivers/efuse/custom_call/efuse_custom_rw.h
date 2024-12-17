/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 * Description: efuse inner header to provide custom read / write service
 * Create: 2023/01/30
 */
#ifndef __EFUSE_CUSTOM_RW_H__
#define __EFUSE_CUSTOM_RW_H__

#include <linux/types.h>

/*
 * Read efuse item in customized way.
 *
 * @param  buf                   efuse item data will be written to buf
 * @param  size                  capacity of buffer in u8 counts / efuse item size in u8 counts
 * @param  fid                   for sub-layer call
 *
 * @return
 * EFUSE_OK                      operation succeed
 * other                         error code
 */
u32 efuse_custom_read(u8 *buf, u32 *size, u32 fid);

/*
 * Write efuse item in customized way.
 *
 * @param  buf                   data in buf will be written to efuse
 * @param  size                  capacity of buffer in u8 counts ([size] bytes will be written)
 * @param  fid                   for sub-layer call
 *
 * @return
 * EFUSE_OK                      operation succeed
 * other                         error code
 */
u32 efuse_custom_write(u8 *buf, u32 size, u32 fid);

#endif /* __EFUSE_CUSTOM_RW_H__ */