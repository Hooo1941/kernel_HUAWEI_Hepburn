/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
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
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/semaphore.h>

#include <teek_client_id.h>
#include <teek_client_api.h>

#include <product_config.h>

#include <bsp_efuse.h>
#include <bsp_mloader.h>
#include "mloader_load_image.h"
#include "mloader_err.h"
#include "mloader_sec_call.h"
#ifdef BSP_CONFIG_PHONE_TYPE
#include <platform_include/see/sec_auth_ca.h> /* phone struct */
#endif

#if defined(BSP_CONFIG_PHONE_TYPE) && !defined(CONFIG_PHONE_LEGACY_SECBOOT)
#define SEC_TA_UUID TEE_SERVICE_SECAUTH
#define SEC_TA_NAME "sec_auth"
#else
#define SEC_TA_UUID TEE_SERVICE_SECBOOT
#define SEC_TA_NAME "sec_boot"
#endif


struct semaphore g_mloader_verifier_sem;
struct semaphore g_mloader_notifier_sem;
struct list_head g_mloader_notifier_list;

int bsp_mloader_notifier_register(mloader_notifier_s *notifier)
{
    if (notifier == NULL || notifier->func == NULL) {
        mloader_print_err("error arg\n");
        return MLOADER_ERR_ARG;
    }

    while (down_interruptible(&g_mloader_notifier_sem)) { }
    list_add(&notifier->list, &g_mloader_notifier_list);
    up(&g_mloader_notifier_sem);
    return 0;
}
EXPORT_SYMBOL_GPL(bsp_mloader_notifier_register);

int mdrv_mloader_notifier_register(mloader_notifier_func notifier_func)
{
    mloader_notifier_s *notifier = (mloader_notifier_s *)kzalloc(sizeof(mloader_notifier_s), GFP_KERNEL);
    if (notifier == NULL) {
        return MLOADER_ERR_NO_MEMORY;
    }

    notifier->func = notifier_func;

    bsp_mloader_notifier_register(notifier);

    return 0;
}
EXPORT_SYMBOL_GPL(mdrv_mloader_notifier_register);

void mloader_load_notifier_process(mloader_stage_e stage, int result, mloader_img_id_e img_id)
{
    mloader_notifier_s *notifier = NULL;

    while (down_interruptible(&g_mloader_notifier_sem)) { }

    list_for_each_entry(notifier, &g_mloader_notifier_list, list) {
        if (notifier->func) {
            (*(notifier->func))(stage, result);
        }
    }

    up(&g_mloader_notifier_sem);
}

#if (defined(CONFIG_LOAD_SEC_IMAGE)) && (defined(CONFIG_TZDRIVER) || defined(CONFIG_TRUSTZONE_HM))

static struct teec_session g_mloader_session;
static struct teec_context g_mloader_context;

int mloader_teek_init(void)
{
    uint32_t result;
    struct teec_uuid svc_uuid = SEC_TA_UUID;
    struct teec_operation operation = { 0 };
    const char *package_name = SEC_TA_NAME;
    u32 root_id = 0;

    result = teek_initialize_context(NULL, &g_mloader_context);
    if (result != TEEC_SUCCESS) {
        /* cov_verified_start */
        mloader_print_err("fail to init teek context, result = 0x%x\n", result);
        goto error;
        /* cov_verified_stop */
    }

    operation.started = 1;
    operation.cancel_flag = 0;
    operation.paramtypes =
        (uint32_t)TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE, TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_INPUT); /*lint !e845*/

    operation.params[2U].tmpref.buffer = (void *)&root_id;
    operation.params[2U].tmpref.size = sizeof(root_id);
    operation.params[3U].tmpref.buffer = (void *)package_name;
    operation.params[3U].tmpref.size = strlen(package_name) + 1;
    result = teek_open_session(&g_mloader_context, &g_mloader_session, &svc_uuid, TEEC_LOGIN_IDENTIFY, NULL, &operation,
        NULL);
    if (result != TEEC_SUCCESS) {
        /* cov_verified_start */
        mloader_print_err("fail to open teek session, result = 0x%x\n", result);
        teek_finalize_context(&g_mloader_context);
        /* cov_verified_stop */
    }

error:
    return (int)result;
}

#if !defined(BSP_CONFIG_PHONE_TYPE) || defined(CONFIG_PHONE_LEGACY_SECBOOT)
int mloader_verify_prepare(mloader_img_id_e image)
{
    uint32_t result;
    uint32_t origin = 0;
    struct teec_operation operation;

    operation.started = 1;
    operation.cancel_flag = 0;

    operation.paramtypes = (uint32_t)TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);
    operation.params[0].value.a = image + MODEM_START;

    result = teek_invoke_command(&g_mloader_session, SECBOOT_CMD_ID_RESET_IMAGE, &operation, &origin);
    if (result != TEEC_SUCCESS) {
        /* cov_verified_start */
        mloader_print_err("fail to invoke teek command, result = 0x%x\n", result);
        /* cov_verified_stop */
    }

    return (int)result;
}

int mloader_trans_vrl_to_os(mloader_img_id_e image, void *buf, u32 len)
{
    uint32_t result;
    uint32_t origin = 0;
    struct teec_operation operation;

    operation.started = 1;
    operation.cancel_flag = 0;
    operation.paramtypes =
        (uint32_t)TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_MEMREF_TEMP_INPUT, TEEC_NONE, TEEC_NONE); /*lint !e845*/

    operation.params[0].value.a = image + MODEM_START;

    operation.params[1].tmpref.buffer = buf;
    operation.params[1].tmpref.size = len;

    result = teek_invoke_command(&g_mloader_session, SECBOOT_CMD_ID_COPY_VRL_TYPE, &operation, &origin);
    if (result != TEEC_SUCCESS) {
        /* cov_verified_start */
        mloader_print_err("fail to invoke teek command, result = 0x%x\n", result);
        /* cov_verified_stop */
    }

    return (int)result;
}

int mloader_trans_data_to_os(mloader_img_id_e image, void *buf, const u32 offset, const u32 len)
{
    uint32_t result;
    uint32_t origin = 0;
    struct teec_operation operation;
    u64 img_buf = (u64)virt_to_phys(buf);

    operation.started = 1;
    operation.cancel_flag = 0;
    operation.paramtypes =
        (uint32_t)TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_VALUE_INPUT, TEEC_VALUE_INPUT, TEEC_VALUE_INPUT); /*lint !e845*/

    operation.params[0].value.a = image + MODEM_START;

    /* the follow 64 bits is reserved for dst buffer, which is obsolete */
    operation.params[0].value.b = 0;
    operation.params[1].value.a = 0;

    operation.params[1].value.b = offset;

    /* the follow 64 bits is reserved for src buffer */
    operation.params[2U].value.a = (u32)img_buf;
    operation.params[2U].value.b = (u64)img_buf >> 32U;

    operation.params[3U].value.a = len;
    result = teek_invoke_command(&g_mloader_session, SECBOOT_CMD_ID_COPY_DATA_TYPE, &operation, &origin);
    if (result != TEEC_SUCCESS) {
        /* cov_verified_start */
        mloader_print_err("fail to invoke teek command, result = 0x%x\n", result);
        /* cov_verified_stop */
    }

    return (int)result;
}

int mloader_verify_soc_image(mloader_img_id_e image)
{
    int ret;
    uint32_t result;
    struct teec_operation operation;
    uint32_t origin = 0;

    ret = bsp_efuse_write_prepare();
    if (ret) {
        return ret;
    }
    operation.started = 1;
    operation.cancel_flag = 0;
    operation.paramtypes =
        (uint32_t)TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE); /*lint !e845*/

    operation.params[0].value.a = image + MODEM_START;
    operation.params[0].value.b = 0; /* SECBOOT_LOCKSTATE , not used currently */
    result = teek_invoke_command(&g_mloader_session, SECBOOT_CMD_ID_VERIFY_DATA_TYPE, &operation, &origin);
    if (result != TEEC_SUCCESS) {
        /* cov_verified_start */
        mloader_print_err("fail to verify image %d, result = 0x%x\n", image, result);
        /* cov_verified_stop */
    }
    mloader_print_err("succeed to verify image %d\n", image);
    bsp_efuse_write_complete();
    return (int)result;
}

int mloader_common_verify(void)
{
    int ret;

    mloader_load_notifier_process(MLOADER_STAGE_IMG_VERIFY_IN, 0, MODEM_IMG_INNER);
    ret = mloader_verify_soc_image(MODEM_IMG_INNER);
    mloader_load_notifier_process(MLOADER_STAGE_IMG_VERIFY_OUT, ret, MODEM_IMG_INNER);
    return ret;
}
#else
static const struct img_id_name_tbl g_match_tbl[] = {
    { MODEM_IMG_CCPU_REE, MODEM_CCPU_IMG_NM},
    { MODEM_IMG_CCPU_REE_PATCH, MODEM_COLD_PATCH_IMG_NM},
    { MODEM_IMG_LPMCU, MODEM_LPMCU_IMG_NM},
    { MODEM_IMG_CCPU_TEE, MODEM_CCPU_TEE_IMG_NM},
    { MODEM_IMG_CCPU_TEE_PATCH, MODEM_CCPU_TEE_PATCH_IMG_NM},
    { MODEM_IMG_FW_DTB, MODEM_FW_DTB_IMG_NM},
    { MODEM_IMG_CCPU_DTB, MODEM_CCPU_DTB_IMG_NM},
    { MODEM_IMG_CCPU_DTO, MODEM_CCPU_DTO_IMG_NM},
};

static int image_id_info_init(mloader_img_id_e image, struct image_id_t *id)
{
    u32 i;
    for (i = 0; i < sizeof(g_match_tbl) / sizeof(g_match_tbl[0]); i++) {
        if (g_match_tbl[i].img_type == image) {
            return strcpy_s(id->image_name, sizeof(id->image_name), g_match_tbl[i].img_name);
        }
    }

    return -1;
}

int mloader_verify_prepare(mloader_img_id_e image)
{
    uint32_t result;
    uint32_t origin = 0;
    struct teec_operation operation = { 0 };
    struct image_id_t id = {
        .partition_name = MODEM_PARTITION_NM,
    };

    if (image_id_info_init(image, &id)) {
        return -1;
    }

    operation.started = 1;
    operation.cancel_flag = 0;
    operation.paramtypes = (uint32_t)TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);
    operation.params[0].tmpref.buffer = (void *)&id;
    operation.params[0].tmpref.size = sizeof(id);

    result = teek_invoke_command(&g_mloader_session, SEC_AUTH_CMD_ID_INIT, &operation, &origin);
    if (result != TEEC_SUCCESS) {
        mloader_print_err("fail to invoke teek command, result = 0x%x\n", result);
    }

    return (int)result;
}

int mloader_trans_vrl_to_os(mloader_img_id_e image, void *buf, const u32 len)
{
    uint32_t result;
    uint32_t origin = 0;
    struct teec_operation operation = { 0 };
    struct image_id_t id = {
        .partition_name = MODEM_PARTITION_NM,
    };

    if (image_id_info_init(image, &id)) {
        return -1;
    }

    operation.started = 1;
    operation.cancel_flag = 0;
    operation.paramtypes =
        (uint32_t)TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_INPUT, TEEC_NONE, TEEC_NONE);
    operation.params[0].tmpref.buffer = (void *)&id;
    operation.params[0].tmpref.size = sizeof(id);
    operation.params[1].tmpref.buffer = buf;
    operation.params[1].tmpref.size = len;

    result = teek_invoke_command(&g_mloader_session, SEC_AUTH_CMD_ID_READ_CERT, &operation, &origin);
    if (result != TEEC_SUCCESS) {
        mloader_print_err("fail to invoke teek command, result = 0x%x\n", result);
    }

    return (int)result;
}

int mloader_trans_data_to_os(mloader_img_id_e image, void *buf, const u32 offset, const u32 len)
{
    uint32_t result;
    uint32_t origin = 0;
    struct teec_operation operation = { 0 };
    struct image_id_t id = {
        .partition_name = MODEM_PARTITION_NM,
    };

    if (image_id_info_init(image, &id)) {
        return -1;
    }

    operation.started = 1;
    operation.cancel_flag = 0;
    operation.paramtypes =
        (uint32_t)TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_INPUT, TEEC_VALUE_INPUT, TEEC_NONE);
    operation.params[0].tmpref.buffer = (void *)&id;
    operation.params[0].tmpref.size = sizeof(id);
    operation.params[1].tmpref.buffer = buf;
    operation.params[1].tmpref.size = len;
    operation.params[2U].value.a = offset;
    operation.params[2U].value.b = 0;

    result = teek_invoke_command(&g_mloader_session, SEC_AUTH_CMD_ID_READ_DATA, &operation, &origin);
    if (result != TEEC_SUCCESS) {
        mloader_print_err("fail to invoke teek command, result = 0x%x\n", result);
    }

    return (int)result;
}

int mloader_verify_soc_image(mloader_img_id_e image)
{
    int ret;
    uint32_t result;
    struct teec_operation operation = { 0 };
    uint32_t origin = 0;
    struct authed_data out = { 0 };
    struct image_id_t id = {
        .partition_name = MODEM_PARTITION_NM,
    };

    if (image_id_info_init(image, &id)) {
        return -1;
    }

    ret = bsp_efuse_write_prepare();
    if (ret) {
        return ret;
    }
    operation.started = 1;
    operation.cancel_flag = 0;
    operation.paramtypes = (uint32_t)TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEE_PARAM_TYPE_MEMREF_INOUT, TEEC_NONE,
        TEEC_NONE);
    operation.params[0].tmpref.buffer = (void *)&id;
    operation.params[0].tmpref.size = sizeof(id);
    operation.params[1].tmpref.buffer = (void *)&out;
    operation.params[1].tmpref.size = sizeof(out);
    result = teek_invoke_command(&g_mloader_session, SEC_AUTH_CMD_ID_VERIFY, &operation, &origin);
    bsp_efuse_write_complete();
    if (result != TEEC_SUCCESS) {
        mloader_print_err("fail to verify image %d, result = 0x%x\n", image, result);
        return (int)result;
    }
    mloader_print_err("succeed to verify image %d\n", image);

    operation.paramtypes =
        (uint32_t)TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);
    result = teek_invoke_command(&g_mloader_session, SEC_AUTH_CMD_ID_FINAL, &operation, &origin);
    if (result != TEEC_SUCCESS) {
        mloader_print_err("auth final result = 0x%x\n", result);
    }
    return (int)result;
}

int mloader_common_verify(void)
{
    int ret;
    uint32_t result;
    uint32_t origin = 0, size = 0;
    struct teec_operation operation = { 0 };
    struct authed_data out = { 0 };
    struct image_id_t id = {
        .partition_name = MODEM_PARTITION_NM,
        .image_name = MODEM_COMM_IMG_NM,
    }; // images loaded by ccore use fixed para

    operation.started = 1;
    operation.cancel_flag = 0;
    operation.params[0].tmpref.buffer = (void *)&id;
    operation.params[0].tmpref.size = sizeof(id);
    operation.params[1].tmpref.buffer = (void *)bsp_mloader_get_local_buffer(&size); // stub for para check
    operation.params[1].tmpref.size = CONFIG_VRL_SIZE; // stub for para check

    mloader_load_notifier_process(MLOADER_STAGE_IMG_VERIFY_IN, 0, MODEM_IMG_INNER);

    operation.paramtypes = (uint32_t)TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);
    result = teek_invoke_command(&g_mloader_session, SEC_AUTH_CMD_ID_INIT, &operation, &origin);
    if (result != TEEC_SUCCESS) {
        goto err;
    }

    operation.paramtypes = (uint32_t)TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_INPUT, TEEC_NONE, TEEC_NONE);
    result = teek_invoke_command(&g_mloader_session, SEC_AUTH_CMD_ID_READ_CERT, &operation, &origin);
    if (result != TEEC_SUCCESS) {
        goto err;
    }

    operation.paramtypes =
        (uint32_t)TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_INPUT, TEEC_VALUE_INPUT, TEEC_NONE);
    result = teek_invoke_command(&g_mloader_session, SEC_AUTH_CMD_ID_READ_DATA, &operation, &origin);
    if (result != TEEC_SUCCESS) {
        goto err;
    }

    operation.params[1].tmpref.buffer = (void *)&out;
    operation.params[1].tmpref.size = sizeof(out);
    operation.paramtypes = (uint32_t)TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEE_PARAM_TYPE_MEMREF_INOUT, TEEC_NONE, TEEC_NONE);
    result = teek_invoke_command(&g_mloader_session, SEC_AUTH_CMD_ID_VERIFY, &operation, &origin);
    if (result != TEEC_SUCCESS) {
        goto err;
    }

    operation.paramtypes = (uint32_t)TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);
    result = teek_invoke_command(&g_mloader_session, SEC_AUTH_CMD_ID_FINAL, &operation, &origin);
    if (result != TEEC_SUCCESS) {
        goto err;
    }

err:
    mloader_load_notifier_process(MLOADER_STAGE_IMG_VERIFY_OUT, (int)result, MODEM_IMG_INNER);
    mloader_print_err("comm img verify result 0x%x\n", result);
    return (int)result;
}
#endif

int bsp_mloader_sec_prepare(mloader_img_id_e image)
{
    return mloader_verify_prepare(image);
}

int bsp_mloader_sec_copy_vrl(mloader_img_id_e image, void *buf, const u32 len)
{
    return mloader_trans_vrl_to_os(image, buf, len);
}

int bsp_mloader_sec_copy_data(mloader_img_id_e image, void *buf, const u32 ddr_offset, const u32 len)
{
    return mloader_trans_data_to_os(image, buf, ddr_offset, len);
}
int bsp_mloader_sec_verify(mloader_img_id_e image)
{
    int ret;
    mloader_load_notifier_process(MLOADER_STAGE_IMG_VERIFY_IN, 0, image);
    ret = mloader_verify_soc_image(image);
    mloader_load_notifier_process(MLOADER_STAGE_IMG_VERIFY_OUT, 0, image);
    return ret;
}
#else
int mloader_teek_init(void)
{
    return 0;
}
int mloader_verify_prepare(mloader_img_id_e image)
{
    return 0;
}
int mloader_common_verify(void)
{
    return 0;
}
int bsp_mloader_sec_copy_vrl(mloader_img_id_e image, void *buf, const u32 len)
{
    return 0;
}
int bsp_mloader_sec_copy_data(mloader_img_id_e image, void *buf, const u32 ddr_offset, const u32 len)
{
    return 0;
}
int bsp_mloader_sec_verify(mloader_img_id_e image)
{
    return 0;
}
#endif

int mloader_verifier_init(void)
{
    int ret;

    sema_init(&g_mloader_verifier_sem, 1);
    sema_init(&g_mloader_notifier_sem, 1);

    INIT_LIST_HEAD(&g_mloader_notifier_list);

    ret = mloader_teek_init();
    if (ret) {
        mloader_print_err("fail to init mloader CA, ret = %d\n", ret);
        return ret;
    }

    mloader_print_err("mloader verifier init ok\n");

    return ret;
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
module_init(mloader_verifier_init);
#endif
