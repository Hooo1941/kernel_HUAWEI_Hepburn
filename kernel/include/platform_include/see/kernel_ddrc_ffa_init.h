/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: header file for platform drivers ffa interface
 * Create: 2023/03/24
 */

#ifndef __FFA_DDRC_H__
#define __FFA_DDRC_H__

#include <linux/arm_ffa.h>

#define FFA_LOCAL_BUFFER_MAX 4096

/*
 * send message to ddrc sp
 * success: return 0
 */
int ffa_ddrc_send_msg(struct ffa_send_direct_data *args);

/*
 * send message to ddrc sp with share memory
 * only arg0-arg2 can be used
 * local buffer size: u8 local_buf[FFA_LOCAL_BUFFER_MAX]
 * success: return 0
 */
int ffa_ddrc_send_msg_with_shmem(struct ffa_send_direct_data *args,
				 u8 *local_buf, u64 dir);

#endif