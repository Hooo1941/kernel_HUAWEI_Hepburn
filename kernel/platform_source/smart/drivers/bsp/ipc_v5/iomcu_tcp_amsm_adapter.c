/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 * Description: Contexthub ipc tcp amsm stub.
 * Create: 2021-10-14
 */
#include <linux/completion.h>
#include <platform_include/smart/linux/iomcu_status.h>
#include <platform_include/smart/linux/iomcu_ipc.h>
#include <platform_include/basicplatform/linux/ipc_rproc.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <securec.h>
#include <securectype.h>
#include "iomcu_link_ipc.h"
#include "iomcu_link_ipc_shm.h"
#ifndef CONFIG_SECURITY_HEADER_FILE_REPLACE
#include <iomcu_ddr_map.h>
#else
#include <internal_security_interface.h>
#endif

int shmem_send(enum obj_tag module_id, const void *usr_buf, unsigned int usr_buf_size)
{
	struct write_info wr;
	uint32_t ddr_shmem_send_size;
	if (usr_buf == NULL) {
		pr_err("[%s] usr_buf null err\n", __func__);
		return -1;
	}
#ifdef CONFIG_SECURITY_HEADER_FILE_REPLACE
	if(!get_ddr_dts_status()) {
		pr_err("[%s]: failed to get ddr dts!\n", __func__);
		return -1;
	}
#endif

#ifdef CONFIG_SECURITY_HEADER_FILE_REPLACE
	ddr_shmem_send_size = dts_ddr_size_get(DTSI_DDR_SHMEM_AP_SEND_ADDR_AP);
#else
	ddr_shmem_send_size = DDR_SHMEM_AP_SEND_SIZE;
#endif
	if (usr_buf_size > ddr_shmem_send_size) {
		pr_err("[%s] invalid usr_buf size %u\n", __func__, usr_buf_size);
		return -1;
	}

	// alloc shmem data, to add sub-cmd before user data.
	unsigned int shmem_len = usr_buf_size + sizeof(unsigned int);
	unsigned int *shmem_data = (unsigned int *)kzalloc((size_t)shmem_len, GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(shmem_data)) {
		pr_err("%s kmalloc fail\n", __func__);
		return -ENOMEM;
	}

	// set sub cmd.
	shmem_data[0] = SUB_CMD_SHMEM_REQ;

	// copy user data to shmem data.
	int result = memcpy_s((void *)(&(shmem_data[1])), (size_t)usr_buf_size, usr_buf, (size_t)usr_buf_size);
	if (result != EOK) {
		kfree((void *)shmem_data);
		shmem_data = NULL;
		pr_err("[%s] memcpy_s fail[%d]\n", __func__, result);
		return result;
	}

	(void)memset_s(&wr, sizeof(wr), 0, sizeof(wr));
	wr.tag = module_id;
	wr.cmd = CMD_CMN_CONFIG_REQ;
	wr.wr_buf = (void *)shmem_data;
	wr.wr_len = shmem_len;

	result =  write_customize_cmd(&wr, NULL, false);

	// free shmem data.
	kfree((void *)shmem_data);
	shmem_data = NULL;

	return result;
}
