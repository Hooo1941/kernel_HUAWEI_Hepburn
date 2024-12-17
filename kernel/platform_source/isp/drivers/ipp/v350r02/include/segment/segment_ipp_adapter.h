/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name    segment_ipp_adapter.h
 * Description:
 *
 * Date         2020-04-17 16:28:10
 ********************************************************************/
#ifndef __SEGMENT_IPP_ADAPTER_H__
#define __SEGMENT_IPP_ADAPTER_H__
#include "ippmessage_common.h"
#include "cmdlst_common.h"

typedef struct _hipp_adapter_s {
	wait_queue_head_t ch_wq_head[CMDLST_CHANNEL_MAX]; // ch: cmdlst channel
	int ch_ready[CMDLST_CHANNEL_MAX];
	int irq;

	unsigned int   cpe_rate_value; // Current active rate;
	unsigned int   hipp_refs[REFS_TYP_MAX]; // usage: Check if the IPP_device is still working
	unsigned int   is_cur_mode_working[WORK_MODULE_MAX]; // Multiple req cannot be processed in the same work_module

	unsigned int   mapbuf_ready[WORK_MODULE_MAX];
	int            shared_fd[WORK_MODULE_MAX];
	unsigned long  daddr[WORK_MODULE_MAX];
	unsigned long  iova_size[WORK_MODULE_MAX]; // new add r02
	void           *virt_addr[WORK_MODULE_MAX];
	void           *sw_virt_addr[WORK_MODULE_MAX];
	struct dma_buf *dmabuf[WORK_MODULE_MAX];

	unsigned int   refs_mapbuf_algo[WORK_MODULE_MAX];
	int            algo_fd[WORK_MODULE_MAX][MAX_HIPP_REFS_CNT]; // algo shareEnv map
	unsigned long  algo_daddr[WORK_MODULE_MAX][MAX_HIPP_REFS_CNT]; // algo shareEnv map
	unsigned long  algo_iova_size[WORK_MODULE_MAX][MAX_HIPP_REFS_CNT]; // algo shareEnv map
	struct dma_buf *algo_dmabuf[WORK_MODULE_MAX][MAX_HIPP_REFS_CNT]; // algo shareEnv map

	struct mutex   hipp_refs_lock; // usage: used to lock hipp_refs;
	struct mutex   cpe_rate_lock; // usage: used to lock cpe_rate_value;
	struct mutex   mode_working_lock; // usage: used to lock is_cur_mode_working;
	struct hipp_common_s *ippdrv;
} hipp_adapter_s;

typedef struct _dma_buf_link_t {
	int fd;
	unsigned long start_iova;
	unsigned long iova_size;
	struct dma_buf *dmabuf;

	struct list_head fd_link;
} dma_buf_link_t;

int hipp_adapter_do_request(unsigned long args);
int hispcpe_map_kernel(unsigned long args);
int hispcpe_unmap_kernel(unsigned long args);
int hipp_powerup(void);
int hipp_powerdn(void);
int hispcpe_work_check(void);
void hipp_adapter_exception(void);
int hispcpe_clean_wait_flag(hipp_adapter_s *dev, unsigned int wait_mode);
int hispcpe_wait(hipp_adapter_s *dev, cmdlst_channel_id_e wait_channel);
int hispcpe_map_kernel_algo(unsigned long args);
int hispcpe_unmap_kernel_algo(unsigned long args);
#ifdef CONFIG_IPP_DEBUG
void hipp_resource_init(void);
void hipp_resource_deinit(void);
#endif

#endif // __SEGMENT_IPP_ADAPTER_H__
// end
