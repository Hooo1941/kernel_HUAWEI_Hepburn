/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: ufs virtualization
 * Create: 2023-02-01
 */

#ifndef __UFS_VIRTUALIZ_AGENT_H__
#define __UFS_VIRTUALIZ_AGENT_H__

#include <linux/types.h>
#include "ufs_virtualiz.h"

struct ufs_agent_ops {
	char name[MAX_SHMEC_NAME_SIZE];
	int irq;
	int (*recv_event_process)(char *buf, unsigned int buf_size);
	int (*suspend)(void);
	int (*resume)(void);
};

#ifdef CONFIG_SCSI_UFS_VIRTUALIZ_AGENT
int ufs_virtualiz_agent_register(struct ufs_agent_ops *ops, shmec_module type);
int ufs_virtualiz_agent_unregister(shmec_module type);
int ufs_virtualiz_agent_send_buf(char *buf, unsigned int buf_size, shmec_module type);
#else
static inline int ufs_virtualiz_agent_register(struct ufs_agent_ops *ops, shmec_module type)
{ return 0; }
static inline int ufs_virtualiz_agent_unregister(shmec_module type)
{ return 0; }
static inline int ufs_virtualiz_agent_send_buf(char *buf, unsigned int buf_size, shmec_module type)
{ return 0; }
#endif

#endif
