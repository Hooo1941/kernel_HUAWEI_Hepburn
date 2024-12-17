/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
 * foss@huawei.com
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
 *
 */
#include <teek_client_id.h>
#include <teek_client_api.h>
#include <bsp_print.h>
#include "sysboot_sec_call.h"

#define THIS_MODU mod_sysboot

static struct teec_session g_sysboot_load_session;
static struct teec_context g_sysboot_load_context;

int sysboot_teek_init(void)
{
    uint32_t result;
    struct teec_uuid svc_uuid = TEE_SERVICE_SECBOOT;
    struct teec_operation operation = { 0 };
    const char *package_name = "sec_boot";
    u32 root_id = 0;

    result = teek_initialize_context(NULL, &g_sysboot_load_context);
    if (result != TEEC_SUCCESS) {
        /* cov_verified_start */
        bsp_err("teek_initialize_context failed, result %#x\n", result);
        goto error;
        /* cov_verified_stop */
    }

    operation.started = 1;
    operation.cancel_flag = 0;
    operation.paramtypes =
        (uint32_t)TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE, TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_INPUT); /*lint !e845*/

    operation.params[0x2].tmpref.buffer = (void *)(&root_id);
    operation.params[0x2].tmpref.size = sizeof(root_id);
    operation.params[0x3].tmpref.buffer = (void *)(package_name);
    operation.params[0x3].tmpref.size = strlen(package_name) + 1;
    result = teek_open_session(&g_sysboot_load_context, &g_sysboot_load_session, &svc_uuid, TEEC_LOGIN_IDENTIFY, NULL,
        &operation, NULL);
    if (result != TEEC_SUCCESS) {
        /* cov_verified_start */
        bsp_err("teek_open_session failed, result %#x\n", result);
        teek_finalize_context(&g_sysboot_load_context);
        /* cov_verified_stop */
    }

error:
    return (int)result;
}

int sysboot_teek_trans_data_to_os(const char *file_buffer, unsigned int file_size)
{
    if (file_buffer == NULL) {
        bsp_err("file buffer is NULL,please check\n");
        return -1;
    }

    return (int)teek_send_secfile(&g_sysboot_load_session, file_buffer, file_size);
}

int sysboot_teek_load_modem_teeos(void)
{
    struct teec_session *session = &g_sysboot_load_session;
    uint32_t result;
    struct teec_operation operation = { 0 };
    uint32_t origin = 0;

    operation.started = 1;
    operation.cancel_flag = 0;
    operation.paramtypes = (uint32_t)TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE, TEEC_NONE, TEEC_NONE);
    result = teek_invoke_command(session, SECBOOT_CMD_ID_BSP_LOAD_MODEM_TEEOS, &operation, &origin);
    if (result != TEEC_SUCCESS) {
        bsp_err("load modem teeos failed, ret = 0x%x\n", result);
        return (int)result;
    }
    return (int)result;
}

void sysboot_teek_uninit(void)
{
    teek_close_session(&g_sysboot_load_session);
    teek_finalize_context(&g_sysboot_load_context);
}
