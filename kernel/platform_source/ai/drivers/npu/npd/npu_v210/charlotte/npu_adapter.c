/*
 * npu_adapter.c
 *
 * about npu adapter
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#include "npu_adapter.h"

#include <asm/barrier.h>
#include <securec.h>

#include "npd_interface.h"
#include "npu_log.h"

u32 npu_get_feature_bits(void)
{
	return NPU_FEATUE_GLOBAL_EXCEPTION_REG;
}

int npu_plat_res_mailbox_send(void *mailbox, unsigned long mailbox_len,
	const void *message, unsigned long message_len)
{
	int ret;

	cond_return_error(message_len > mailbox_len, -1,
		"message len =%d, too long", message_len);

	ret = memcpy_s(mailbox, mailbox_len, message, message_len);
	cond_return_error(ret != 0, ret, "memcpy failed. ret=%d\n", ret);
	if (message_len < mailbox_len) {
		ret = memset_s(mailbox + message_len, mailbox_len - message_len, 0, mailbox_len - message_len);
		cond_return_error(ret != 0, ret, "memset failed. ret=%d\n", ret);
	}
	mb();
	return ret;
}

