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

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/semaphore.h>
#include <linux/regulator/consumer.h>
#include <bsp_dt.h>
#include <bsp_llt.h>
#include <bsp_sec_call.h>
#include <bsp_efuse.h>
#include <bsp_mci.h>
#include <mdrv_bmi_efuse.h>
#include <bsp_sysctrl.h>
#include <securec.h>
#include "efuse_plat.h"
#include "efuse_driver.h"
#include "efuse_layout.h"
#include "efuse_error.h"

#define THIS_MODU mod_efuse

#define EFUSE_WRITE_PREPARE_DELAY 1000

static struct regulator *g_efuse_regulator = NULL;
static struct efuse_auto_dfs_s g_efuse_auto_dfs;
static struct semaphore g_efuse_semaphore;
static u32 g_max_grp;
static u32 *g_efuse_buffer = NULL;

static inline void efuse_set_value_bit(unsigned int *origin, unsigned int lowbit,unsigned highbit, unsigned int value)
{
    unsigned int mask;

    mask = (((unsigned int)1 << (highbit - lowbit + 1)) - 1) << lowbit;
    *origin = (*origin & (~mask)) | ((value << lowbit) & mask);
}

static void efuse_auto_dfs_en(void)
{
    u32 rdata = 0;

    if (IS_ERR_OR_NULL(g_efuse_auto_dfs.dfs_base_addr)) {
        return;
    }

    rdata = readl((void *)(uintptr_t)(g_efuse_auto_dfs.dfs_base_addr + g_efuse_auto_dfs.dfs_en_offset));

    efuse_set_value_bit(&rdata, g_efuse_auto_dfs.dfs_en_bit_mask, g_efuse_auto_dfs.dfs_en_bit_mask, 1);
    efuse_set_value_bit(&rdata, g_efuse_auto_dfs.dfs_en_bit, g_efuse_auto_dfs.dfs_en_bit, 1);

    writel(rdata, (void *)(uintptr_t)(g_efuse_auto_dfs.dfs_base_addr + g_efuse_auto_dfs.dfs_en_offset));
}

static void efuse_auto_dfs_dis(void)
{
    u32 rdata = 0;

    if (IS_ERR_OR_NULL(g_efuse_auto_dfs.dfs_base_addr)) {
        return;
    }

    rdata = readl((void *)(uintptr_t)(g_efuse_auto_dfs.dfs_base_addr + g_efuse_auto_dfs.dfs_en_offset));

    efuse_set_value_bit(&rdata, g_efuse_auto_dfs.dfs_en_bit_mask, g_efuse_auto_dfs.dfs_en_bit_mask, 1);
    efuse_set_value_bit(&rdata, g_efuse_auto_dfs.dfs_en_bit, g_efuse_auto_dfs.dfs_en_bit, 0);

    writel(rdata, (void *)(uintptr_t)(g_efuse_auto_dfs.dfs_base_addr + g_efuse_auto_dfs.dfs_en_offset));
}

int efuse_auto_dfs_init(void)
{
    int ret = 0;
    device_node_s *node;
    u32 auto_dfs_cfg[3] = {0};

    node = bsp_dt_find_compatible_node(NULL, NULL, "efuse,auto_dfs");
    if (node == NULL) {
        g_efuse_auto_dfs.dfs_base_addr = NULL;
        // 当前平台如果不支持自动降频，正常返回
        efuse_print_info("efuse auto_dfs not support\n");
        return EFUSE_OK;
    }

    ret = bsp_dt_property_read_u32_array(node, "auto_dfs_config", auto_dfs_cfg, sizeof(auto_dfs_cfg) / sizeof(auto_dfs_cfg[0]));
    if (ret) {
        g_efuse_auto_dfs.dfs_base_addr = NULL;
        efuse_print_error("dts auto_dfs_config get failed.");
        return EFUSE_ERROR;
    }

    g_efuse_auto_dfs.dfs_base_addr = bsp_sysctrl_addr_byindex(CRG_AO);
    g_efuse_auto_dfs.dfs_en_offset = auto_dfs_cfg[0] % 0x1000;
    g_efuse_auto_dfs.dfs_en_bit = auto_dfs_cfg[1] % 32;
    g_efuse_auto_dfs.dfs_en_bit_mask = auto_dfs_cfg[2] % 32;

    return EFUSE_OK;
}

void bsp_efuse_ops_prepare(void)
{
    while (down_interruptible(&g_efuse_semaphore)) {
        ;
    }
    efuse_auto_dfs_dis();
}

void bsp_efuse_ops_complete(void)
{
    efuse_auto_dfs_en();
    up(&g_efuse_semaphore);
}

int bsp_efuse_write_prepare(void)
{
    int ret;

    bsp_efuse_ops_prepare();

    if (IS_ERR_OR_NULL(g_efuse_regulator)) {
        efuse_print_error("regulator null, check init status.\n");
        bsp_efuse_ops_complete();
        return EFUSE_ERROR_REGULATOR_GET_FAIL;
    }

    ret = regulator_enable(g_efuse_regulator);
    if (ret) {
        efuse_print_error("regulator_enable error.\n");
        bsp_efuse_ops_complete();
    }

    udelay(EFUSE_WRITE_PREPARE_DELAY); /*lint !e737*/

    return ret;
}

void bsp_efuse_write_complete(void)
{
    if (IS_ERR_OR_NULL(g_efuse_regulator)) {
        efuse_print_error("regulator null, check init status.\n");
    }
    regulator_disable(g_efuse_regulator);
    bsp_efuse_ops_complete();
}

int bsp_efuse_inner_read(u32 *p_buf, u32 group, u32 num, u32 is_sec)
{
    u32 i;
    int ret = 0;
    FUNC_CMD_ID sec_call_cmd = FUNC_EFUSE_READ;

    if ((num == 0) || (group >= g_max_grp) || (num > g_max_grp) || (group + num > g_max_grp) || (p_buf == NULL)) {
        efuse_print_error("error args, group=%d, num=%d.\n", group, num);
        return EFUSE_ERROR;
    }

    bsp_efuse_ops_prepare();

    for (i = 0; i < g_max_grp; i++) {
        g_efuse_buffer[i] = 0;
    }

    if (!is_in_llt()) {
        ret = bsp_sec_call_ext(sec_call_cmd, COMMON_READ, (void *)&g_efuse_buffer[0], sizeof(u32) * g_max_grp);
    }

    for (i = 0; i < num; i++) {
        p_buf[i] = g_efuse_buffer[group + i];
    }

    bsp_efuse_ops_complete();

    return ret;
}

int bsp_efuse_read(u32 *p_buf, u32 group, u32 num)
{
    return bsp_efuse_inner_read(p_buf, group, num, 0);
}

int bsp_efuse_write(u32 *p_buf, u32 group, u32 len)
{
    int ret;

    if ((len == 0) || (group >= g_max_grp) || (len > g_max_grp) || (group + len > g_max_grp) || (p_buf == NULL)) {
        efuse_print_error("error args, group=%d, len=%d.\n", group, len);
        return EFUSE_ERROR;
    }

    ret = bsp_efuse_write_prepare();
    if (ret) {
        return ret;
    }

    ret = memset_s(g_efuse_buffer, sizeof(u32) * g_max_grp, 0, sizeof(u32) * g_max_grp);
    if (ret != EOK) {
        efuse_print_error("<%s> memset_s error, ret = %d\n", __FUNCTION__, ret);
    }

    ret = memcpy_s(&g_efuse_buffer[group], sizeof(u32) * (g_max_grp - group), p_buf, sizeof(u32) * len);
    if (ret != EOK) {
        efuse_print_error("<%s> memcpy_s error, ret = %d\n", __FUNCTION__, ret);
    }

    if (!is_in_llt()) {
        ret = bsp_sec_call_ext(FUNC_EFUSE_WRITE, COMMON_WRITE, (void *)g_efuse_buffer, sizeof(u32) * g_max_grp);
    }

    (void)memset_s(g_efuse_buffer, sizeof(u32) * g_max_grp, 0, sizeof(u32) * g_max_grp);

    bsp_efuse_write_complete();

    return ret;
}

int bsp_efuse_sec_write(unsigned int param)
{
    int ret;

    ret = bsp_efuse_write_prepare();
    if (ret) {
        return ret;
    }

    ret = bsp_sec_call(FUNC_EFUSE_SEC_OPT, param);

    bsp_efuse_write_complete();

    return ret;
}

unsigned int efuse_calc_group_len(u32 bit_start, u32 bits_len)
{
    u32 group_len = 1;
    u32 first_bit = BIT_INDEX_IN_GROUP(bit_start);
    u32 remain_byte = bits_len;

    while ((first_bit + remain_byte) > EFUSE_GROUP_BIT_NUM) {
        group_len++;
        remain_byte -= (EFUSE_GROUP_BIT_NUM - first_bit);
        first_bit = 0;
    }

    return group_len;
}

static u32 efuse_calc_bit_mask(u32 bit)
{
    if (bit >= EFUSE_GROUP_BIT_NUM) {
        return 0xFFFFFFFF;
    }
    return ((1u << bit) - 1);
}

static void efuse_move_align(efuse_trans_s *trans)
{
    u32 i;
    u32 bits_len = trans->bits_len;
    u32 *src = trans->src;
    u32 *dst = trans->dst;
    u32 src_group_len = trans->src_group_len;

    for (i = 0; i < src_group_len; i++) {
        dst[i] = src[i];
        if (bits_len < EFUSE_GROUP_BIT_NUM) {
            dst[i] &= efuse_calc_bit_mask(bits_len);
        } else {
            bits_len -= EFUSE_GROUP_BIT_NUM;
        }
    }
}

static void efuse_move_read(efuse_trans_s *trans)
{
    u32 i;
    u32 first_bit;
    u32 front_len;
    u32 behind_len;
    u32 front_val;
    u32 behind_val;
    u32 bits_len = trans->bits_len;
    u32 *src = trans->src;
    u32 *dst = trans->dst;
    u32 src_group_len = trans->src_group_len;
    u32 dst_group_len = trans->dst_group_len;

    // 如果起始bit是0，说明不需要拼接
    first_bit = BIT_INDEX_IN_GROUP(trans->bit_start);
    if (first_bit == 0) {
        efuse_move_align(trans);
        return;
    }

    // 如果需要读的是一个group的部分bit，不涉及跨group，移位即可
    if (first_bit + bits_len <= EFUSE_GROUP_BIT_NUM) {
        *dst = (*src >> first_bit) & efuse_calc_bit_mask(bits_len);
        return;
    }

    for (i = 0; i < dst_group_len; i++) {
        front_len = first_bit;
        behind_len = EFUSE_GROUP_BIT_NUM - first_bit;

        if (bits_len < behind_len) {
            behind_len = bits_len;
        }

        // 先将前一个组的first_bit到32bit的数据移位拷贝到front_val中
        front_val = src[i] >> front_len;
        front_val &= efuse_calc_bit_mask(behind_len);
        dst[i] = front_val;
        bits_len -= behind_len;

        // 将后一组的0-firts_bit数据拷贝到behind_val中再与front_val移位拼接
        if ((i + 1) < src_group_len) {
            if (bits_len < front_len) {
                front_len = bits_len;
            }
            behind_val = src[i + 1];
            behind_val &= efuse_calc_bit_mask(front_len);
            behind_val <<= behind_len;
            dst[i] |= behind_val;
            bits_len -= front_len;
        }
    }
}

int bsp_efuse_read_by_bit(unsigned int *pbuffer, unsigned int bit_start, unsigned int bits_len)
{
    int ret;
    u32 i;
    u32 group_len;
    u32 pack_num;
    u32 read_group_len;
    u32 read_bit_len;
    efuse_trans_s trans;
    u32 src_val[EFUSE_MAX_PACK_SIZE] = {0};
    u32 max_bit_num = bsp_efuse_get_max_grp_num() * EFUSE_GROUP_BIT_NUM;

    if (pbuffer == NULL || bits_len == 0 || bit_start >= max_bit_num || bits_len > max_bit_num ||
        (bit_start + bits_len) > max_bit_num) {
        efuse_print_error("error args, bit_start=0x%x, bits_len=0x%x\n", bit_start, bits_len);
        return EFUSE_ERROR;
    }

    group_len = efuse_calc_group_len(bit_start, bits_len);
    pack_num = ((group_len + EFUSE_MAX_PACK_SIZE - 1) / EFUSE_MAX_PACK_SIZE);
    read_bit_len = bits_len;
    trans.bit_start = bit_start;
    trans.src = &src_val[0];
    trans.dst = pbuffer;

    for (i = 0; i < pack_num; i++) {
        if (group_len < EFUSE_MAX_PACK_SIZE) {
            read_group_len = group_len;
            trans.bits_len = read_bit_len;
        } else {
            read_group_len = EFUSE_MAX_PACK_SIZE;
            trans.bits_len = (EFUSE_MAX_PACK_SIZE * EFUSE_GROUP_BIT_NUM);
        }
        ret = bsp_efuse_read(&src_val[0], GROUP_INDEX(trans.bit_start), read_group_len);
        if (ret) {
            return ret;
        }

        trans.src_group_len = read_group_len;
        trans.dst_group_len = ((trans.bits_len + EFUSE_GROUP_BIT_NUM - 1) / EFUSE_GROUP_BIT_NUM);
        efuse_move_read(&trans);
        if (group_len <= EFUSE_MAX_PACK_SIZE) {
            break;
        }
        group_len -= EFUSE_MAX_PACK_SIZE;
        read_bit_len -= (EFUSE_MAX_PACK_SIZE * EFUSE_GROUP_BIT_NUM);
        trans.bit_start += (EFUSE_MAX_PACK_SIZE * EFUSE_GROUP_BIT_NUM);
        trans.dst += EFUSE_MAX_PACK_SIZE;
    }

    return EFUSE_OK;
}
EXPORT_SYMBOL_GPL(bsp_efuse_read_by_bit);

int bsp_efuse_get_value(efuse_layout_id_e id)
{
    efuse_layout_s layout_info = {0};
    u32 value = 0;

    if (bsp_efuse_get_layout_info(id, &layout_info)) {
        return EFUSE_ERROR_GET_LAYOUT_INFO;
    }

    if (bsp_efuse_read_by_bit(&value, layout_info.bit, layout_info.bit_len)) {
        return EFUSE_ERROR;
    }

    return (int)value;
}

static int atp_mod_efuse_func(unsigned char *buffer, unsigned int len)
{
    struct mci_atp_mod_efuse *pam = NULL;

    if (buffer == NULL) {
        efuse_print_error("error buffer args\n");
        return EFUSE_ERROR;
    }

    if (len < sizeof(struct mci_atp_mod_efuse)) {
        efuse_print_error("error len args\n");
        return EFUSE_ERROR;
    }

    pam = (struct mci_atp_mod_efuse *)buffer;

    if (pam->word_cnt > EFUSE_MAX_PACK_SIZE) {
        efuse_print_error("error word_cnt args\n");
        return EFUSE_ERROR;
    }

    if (len < sizeof(struct mci_atp_mod_efuse) + pam->word_cnt * sizeof(unsigned int)) {
        efuse_print_error("error len args\n");
        return EFUSE_ERROR;
    }

    return mdrv_efuse_ioctl(pam->cmd, pam->arg, pam->buffer, pam->word_cnt);
}

static int __init bsp_efuse_init(void)
{
    int ret;
    unsigned int max_len = 0;
    bsp_err("bsp_efuse_init start.\n");
    ret = efuse_layout_init();
    if (ret) {
        efuse_print_error("efuse layout init fail\n");
        return EFUSE_ERROR;
    }

    g_max_grp = bsp_efuse_get_max_grp_num();

    g_efuse_buffer = kzalloc(g_max_grp * sizeof(u32), GFP_KERNEL);
    if (IS_ERR_OR_NULL(g_efuse_buffer)) {
        efuse_print_error("malloc buffer fail\n");
        return EFUSE_ERROR;
    }

    sema_init(&g_efuse_semaphore, 1);

    g_efuse_regulator = regulator_get(NULL, "EFUSE-vcc");
    if (g_efuse_regulator == NULL) {
        efuse_print_error("regulator_get error.\n");
        return EFUSE_ERROR_REGULATOR_GET_FAIL;
    }

    ret = efuse_auto_dfs_init();
    if (ret) {
        efuse_print_error("efuse auto dfs init fail\n");
        return EFUSE_ERROR;
    }

    max_len = sizeof(struct mci_atp_mod_efuse) + MCI_ATP_MOD_EFUSE_MAX_LEN;
    ret = bsp_mci_atp_mod_register(ATP_MOD_ID_EFUSE, atp_mod_efuse_func, MCI_ATP_MOD_EFUSE_TIMEOUT_MS, max_len);
    if (ret) {
        efuse_print_error("atp register error, err = %d\n", ret);
        return EFUSE_ERROR;
    }

    bsp_err("bsp_efuse_init end.\n");
    return EFUSE_OK;
}

fs_initcall_sync(bsp_efuse_init);

EXPORT_SYMBOL(bsp_efuse_read);
EXPORT_SYMBOL(bsp_efuse_write);
EXPORT_SYMBOL(bsp_efuse_get_value);
