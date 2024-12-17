/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef PCS_LOG_H
#define PCS_LOG_H

#include <linux/kernel.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PCS_LOG_LVL_HIGH (1U << 0)
#define PCS_LOG_LVL_ERR (1U << 1)
#define PCS_LOG_LVL_INFO (1U << 2)
#define PCS_LOG_LVL_DBG (1U << 3)

#define pcs_log_h(fmt, ...) do { \
	if (pcs_log_level & PCS_LOG_LVL_HIGH) \
		pr_err("[PCS_H]: <%s> "fmt"\n", __func__, ##__VA_ARGS__); \
} while (0)

#define pcs_log_e(fmt, ...) do { \
	if (pcs_log_level & PCS_LOG_LVL_ERR) \
		pr_err("[PCS_E]: <%s> "fmt"\n", __func__, ##__VA_ARGS__); \
} while (0)

#define pcs_log_i(fmt, ...) do { \
	if (pcs_log_level & PCS_LOG_LVL_INFO) \
		pr_warn("[PCS_I]: <%s> "fmt"\n", __func__, ##__VA_ARGS__); \
} while (0)

#define pcs_log_d(fmt, ...) do { \
	if (pcs_log_level & PCS_LOG_LVL_DBG) \
		pr_notice("[PCS_D]: <%s> "fmt"\n", __func__, ##__VA_ARGS__); \
} while (0)

extern u32 pcs_log_level;

enum pcs_diag_cause {
	PCS_DIAG_CAUSE_RCV_ENTRY = 0,
	PCS_DIAG_CAUSE_SEND_ENTRY,
	PCS_DIAG_CAUSE_RCV_LEN_INVALID,

	/* req */
	PCS_DIAG_CAUSE_RCV_DIR_LEN_INVALID,
	PCS_DIAG_CAUSE_TOO_MANY_NODE,
	PCS_DIAG_CAUSE_ALLOC_NODE_FAILED,
	PCS_DIAG_CAUSE_READ_FILE_FAILED,
	PCS_DIAG_CAUSE_ALLOC_BUFF_FAILED,

	/* ack */
	PCS_DIAG_CAUSE_NODE_INVALID,
	PCS_DIAG_CAUSE_ACK_PARAM_ABNORMAL,
	PCS_DIAG_CAUSE_ACK_FINISH,

	/* dir abnormal */
	PCS_DIAG_CAUSE_AP_DIR_ABNORMAL,
	PCS_DIAG_CAUSE_AP_DIR_INVALID,
	PCS_DIAG_CAUSE_CP_DIR_ABNORMAL,
	PCS_DIAG_CAUSE_CP_SET_DIR_ABNORMAL,
	PCS_DIAG_CAUSE_PATH_INVALID,
	PCS_DIAG_CAUSE_OPEN_FILE_FAILED,
};

/* RINC_FUSION[0xC950,0xC95F] */
enum {
	ID_DIAG_PCS_BC_CAP_REQ_INFO = 0xC950,
	ID_DIAG_PCS_BC_CAP_ACK_INFO = 0xC951,
	ID_DIAG_PCS_RRC_READ_INFO = 0xC952,
	ID_DIAG_PCS_DIR_INFO = 0xC953,
};

void pcs_trans_report(u32 msgid, void *data, u32 len);

#ifdef __cplusplus
}
#endif

#endif
