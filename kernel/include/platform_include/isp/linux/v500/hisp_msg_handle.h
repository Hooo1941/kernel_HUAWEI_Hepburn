/*
 * hisp_msg_handle.h
 *
 * Copyright (c) 2023 ISP Technologies CO., Ltd.
 *
 */

#ifndef _HISP_MSG_HANDLE_H
#define _HISP_MSG_HANDLE_H

#include <linux/platform_device.h>
#include <linux/remoteproc.h>
#include <linux/pci.h>
#include <uapi/linux/virtio_types.h>
#include <platform_include/isp/linux/hisp_remoteproc.h>
#include <linux/rpmsg.h>

int hisp_msg_init_memory_pool(unsigned int type, struct hisp_mem_info_s *mem);
int hisp_msg_deinit_memory_pool(unsigned int type, struct hisp_mem_info_s *mem);

#endif /* _HISP_MSG_HANDLE_H */
