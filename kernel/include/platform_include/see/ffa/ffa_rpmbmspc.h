/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: header file for rpmbmspc drivers ffa interface
 * Create: 2023/09/08
 */

#ifndef _FFA_RPMBMSPC_H
#define _FFA_RPMBMSPC_H

#ifdef CONFIG_RPMBMSPC_SUPPORT_FFA

#include <linux/arm_ffa.h>
#define FFA_LOCAL_BUFFER_MAX 4096

/*
 * send message to rpmbmspc sp
 */
int ffa_rpmbmspc_send_msg(u64 arg0, u64 arg1, u64 arg2, u64 arg3);
int ffa_rpmbmspc_send_msg_with_args(struct ffa_send_direct_data *args);

/*
 * send message to rpmbmspc sp with share memory
 * only arg0-arg2 can be used
 * local buffer size: u8 local_buf[FFA_LOCAL_BUFFER_MAX]
 * success: return 0
 */
int ffa_rpmbmspc_send_msg_with_shmem(struct ffa_send_direct_data *args,
				    u8 *local_buf, u64 dir);

#endif   /* CONFIG_RPMBMSPC_SUPPORT_FFA */

#endif  /* _FFA_RPMBMSPC_H */
