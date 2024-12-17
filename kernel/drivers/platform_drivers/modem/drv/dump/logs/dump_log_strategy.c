/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
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
#include <product_config.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <bsp_dt.h>
#include <of.h>
#include <securec.h>
#include <osl_list.h>
#include <osl_malloc.h>
#include <bsp_print.h>
#include <bsp_slice.h>
#include <bsp_file_ops.h>
#include "dump_logs.h"
#include "dump_log_strategy.h"
#include "dump_log_parser.h"
#include "dump_extra_logs.h"
#include <linux/init_syscalls.h>

#undef THIS_MODU
#define THIS_MODU mod_dump

struct dump_log_agent_stru g_strategy_ctrl = { false, 0, NULL };
struct dump_zip_stru g_zip_ctrl;
dump_file_info_s g_dump_file_info;

bool dump_get_link_status(void)
{
    return g_strategy_ctrl.link_enable;
}

s32 dump_match_log_strategy(const char *file_name, struct dump_file_save_strategy *output)
{
    s32 index = 0;
    u32 log_sum = g_strategy_ctrl.log_sum;
    struct dump_file_save_strategy *save_strategy = g_strategy_ctrl.log_strategy;
    if (file_name == NULL || output == NULL || save_strategy == NULL) {
        return BSP_ERROR;
    }
    if (memset_s(output, sizeof(struct dump_file_save_strategy), 0, sizeof(struct dump_file_save_strategy)) != EOK) {
        dump_error("set err\n");
        return BSP_ERROR;
    }
    for (index = 0; index < log_sum; index++) {
        if (strncmp(file_name, save_strategy[index].data_head.filename,
            sizeof(save_strategy[index].data_head.filename)) != 0) {
            continue;
        }
        if (memcpy_s(output, sizeof(struct dump_file_save_strategy), &save_strategy[index],
            sizeof(save_strategy[index])) != EOK) {
            dump_error("cpy err\n");
            return BSP_ERROR;
        }
        /* 拼接非压缩时，直接拼接；拼接且压缩时，最后在压缩应用中压缩再拼接 */
        if (output->is_append == 0 || output->is_compress == 1 || output->data_head.magic != DUMP_FILE_TRANS_MAGIC ||
            output->data_head.links != DUMP_FILE_LINKS_MAGIC) {
            return BSP_ERROR;
        }
        return BSP_OK;
    }
    return BSP_ERROR;
}


void dump_show_strategy(void)
{
    u32 size;
    s32 i = 0;
    struct dump_file_save_strategy *log_strategy = g_strategy_ctrl.log_strategy;
    dump_print("enable:%d\n", g_strategy_ctrl.link_enable);
    if (log_strategy == NULL) {
        dump_error("g_strategy_ctrl.log_strategy is null\n");
        return;
    }
    size = g_strategy_ctrl.log_sum;
    dump_error("log_sum:%u\n", size);
    for (i = 0; i < size; i++) {
        dump_print("filename: %s, dst_file: %s\n", log_strategy[i].data_head.filename,
            log_strategy[i].data_head.dstfilename);
        dump_print("compress: %u, append: %u\n", log_strategy[i].is_compress, log_strategy[i].is_append);
    }
}

int dump_link_probe_src_list(device_node_s *child, struct dump_file_save_strategy *log_strategy, uintptr_t srclist_size,
    struct dump_dstname_list *dstname_list, u32 dstlist_size)
{
    s32 i, j;
    int ret;
    u32 u = 0;
    const char *temp_char = NULL;
    const __be32 *p = NULL;
    struct property *prop = NULL;

    for (i = 0; i < srclist_size; i++) {
        ret = bsp_dt_property_read_string_index(child, "dump_src_name", i, &temp_char);
        if (ret != 0 || temp_char == NULL) {
            dump_error("get src_name err\n");
            return BSP_ERROR;
        }
        if (strncpy_s(log_strategy[i].data_head.filename, sizeof(log_strategy[i].data_head.filename), temp_char,
            strnlen(temp_char, DUMP_FILE_NAME_MAX_SIZE)) < 0) {
            dump_error("strncpy err\n");
            return BSP_ERROR;
        }
        log_strategy[i].data_head.magic = DUMP_FILE_TRANS_MAGIC;
        log_strategy[i].data_head.links = DUMP_FILE_LINKS_MAGIC;
    }

    i = -1;
    j = -1;
    bsp_dt_property_for_each_u32(child, "dump_src_attr", prop, p, u)
    {
        i++;
        if (i % DUMP_LOG_STRATEGY_ATTR_SIZE == 0) {
            j++;
            log_strategy[j].is_compress = u;
        } else if (i % DUMP_LOG_STRATEGY_ATTR_SIZE == 1) {
            log_strategy[j].is_append = u;
        } else {
            if (log_strategy[j].is_append == 0) {
                continue;
            }
            if (u <= 0 || u > dstlist_size) {
                dump_error("u=%u is err\n", u);
                return BSP_ERROR;
            }
            if (strncpy_s(log_strategy[j].data_head.dstfilename, sizeof(log_strategy[j].data_head.dstfilename),
                dstname_list[u - 1].filename, sizeof(dstname_list[u - 1].filename)) < 0) {
                return BSP_ERROR;
            }
        }
    }

    if ((i + 1) != DUMP_LOG_STRATEGY_ATTR_SIZE * srclist_size || srclist_size != (j + 1)) {
        return BSP_ERROR;
    }
    return BSP_OK;
}

int dump_link_fill_list(device_node_s *child, struct dump_file_save_strategy *log_strategy, uintptr_t srclist_size)
{
    u32 dstlist_size = 0;
    int i;
    int ret = BSP_ERROR;
    struct dump_dstname_list *dstname_list = NULL;
    const char *temp_char = NULL;
    if (bsp_dt_property_read_u32(child, "dstlistsize", &dstlist_size)) {
        dump_error("get size err\n");
        return BSP_ERROR;
    }
    dstname_list = (struct dump_dstname_list *)kmalloc(dstlist_size * (sizeof(struct dump_dstname_list)), GFP_KERNEL);
    if (dstname_list == NULL) {
        dump_error("malloc err\n");
        return BSP_ERROR;
    }
    if (EOK != memset_s(dstname_list, dstlist_size * (sizeof(struct dump_dstname_list)), 0,
        dstlist_size * (sizeof(struct dump_dstname_list)))) {
        dump_error("set err\n");
        goto out;
    }
    for (i = 0; i < dstlist_size; i++) {
        ret = bsp_dt_property_read_string_index(child, "dump_dst_name", i, &temp_char);
        if (ret != 0 || temp_char == NULL) {
            ret = BSP_ERROR;
            dump_error("get dst_name err\n");
            goto out;
        }
        if (strncpy_s(dstname_list[i].filename, sizeof(dstname_list[i].filename), temp_char,
            strnlen(temp_char, DUMP_FILE_NAME_MAX_SIZE)) < 0) {
            dump_error("strncpy err\n");
            goto out;
        }
    }

    if (dump_link_probe_src_list(child, log_strategy, srclist_size, dstname_list, dstlist_size) != BSP_OK) {
        dump_error("get src_attr err!\n");
        goto out;
    }
    ret = BSP_OK;
out:
    if (dstname_list != NULL) {
        kfree(dstname_list);
        dstname_list = NULL;
    }
    return ret;
}

int dump_link_list_init(void)
{
    device_node_s *dev = NULL;
    u32 srclist_size = 0;
    s32 ret;
    struct dump_file_save_strategy *log_strategy = NULL;
    dev = bsp_dt_find_compatible_node(NULL, NULL, "driver,dump_linkup_list");
    if (dev == NULL) {
        dump_error("list dts no find\n");
        return BSP_ERROR;
    }
    ret = bsp_dt_property_read_u32(dev, "srclistsize", &srclist_size);
    if (ret) {
        dump_error("get size err\n");
        return BSP_ERROR;
    }
    log_strategy =
        (struct dump_file_save_strategy *)kmalloc(srclist_size * (sizeof(struct dump_file_save_strategy)), GFP_KERNEL);
    if (log_strategy == NULL) {
        dump_error("alloc err\n");
        return BSP_ERROR;
    }
    g_strategy_ctrl.log_sum = (u32)srclist_size;
    if (EOK != memset_s(log_strategy, srclist_size * (sizeof(struct dump_file_save_strategy)), 0,
        srclist_size * (sizeof(struct dump_file_save_strategy)))) {
        dump_error("set err\n");
        kfree(log_strategy);
        return BSP_ERROR;
    }
    ret = dump_link_fill_list(dev, log_strategy, srclist_size);
    if (ret) {
        kfree(log_strategy);
        return BSP_ERROR;
    }

    g_strategy_ctrl.log_strategy = log_strategy;
    return BSP_OK;
}

void dump_link_init(void)
{
    device_node_s *dev = NULL;
    u32 link_enable = 0;
    s32 ret;

    dev = bsp_dt_find_compatible_node(NULL, NULL, "driver,dump_log_strategy");
    if (dev == NULL) {
        dump_error("strategy dts no find\n");
        g_strategy_ctrl.link_enable = false;
        return;
    }

    ret = bsp_dt_property_read_u32(dev, "link_enable", &link_enable);
    if (ret || 0 == link_enable) {
        dump_error("link disable\n");
        g_strategy_ctrl.link_enable = false;
        return;
    }

    ret = dump_link_list_init();
    if (ret == BSP_OK) {
        g_strategy_ctrl.link_enable = true;
    } else {
        g_strategy_ctrl.link_enable = false;
    }
}

static unsigned int compdev_poll(struct file *file, poll_table *wait)
{
    unsigned int ret;
    dump_print("poll waiting.\n");
    poll_wait(file, &g_zip_ctrl.comp_log_ctrl->wq, wait);
    ret = g_zip_ctrl.comp_log_ctrl->trigger_flag ? POLLIN : 0;
    dump_print("poll done once.\n");
    return ret;
}

static ssize_t compdev_read(struct file *file, char __user *buf, size_t count, loff_t *pos)
{
    ssize_t ret = 1;
    dump_print("read zipdev.\n");
    if ((buf == NULL) || (count != sizeof(g_zip_ctrl.zipintf_info))) {
        return BSP_ERROR;
    }
    mutex_lock(&g_zip_ctrl.comp_log_ctrl->mutex);
    ret = copy_to_user(buf, (void *)&g_zip_ctrl.zipintf_info, sizeof(g_zip_ctrl.zipintf_info));
    if (ret) {
        dump_error("copy to user failed.\n");
    }
    g_zip_ctrl.comp_log_ctrl->trigger_flag = 0;

    mutex_unlock(&g_zip_ctrl.comp_log_ctrl->mutex);

    dump_print("read zipdev done.\n");
    return ret;
}

static int compdev_open(struct inode *inode, struct file *file)
{
    mutex_lock(&g_zip_ctrl.comp_log_ctrl->mutex);
    g_zip_ctrl.comp_log_ctrl->fopen_cnt++;
    mutex_unlock(&g_zip_ctrl.comp_log_ctrl->mutex);
    dump_print("open zipdev.\n");
    return 0;
}

static int compdev_release(struct inode *inode, struct file *file)
{
    dump_error("%s entry\n", __func__);
    return 0;
}

static const struct file_operations g_comp_dev_fops = {
    .read = compdev_read,
    .poll = compdev_poll,
    .open = compdev_open,
    .release = compdev_release,
};

s32 dump_comp_dev_register(struct zipintf_info_s *zip_info)
{
    s32 ret = 0;

    g_zip_ctrl.comp_log_ctrl = kzalloc(sizeof(*g_zip_ctrl.comp_log_ctrl), GFP_KERNEL);
    if (g_zip_ctrl.comp_log_ctrl == NULL) {
        ret = EINVAL;
        goto out;
    }

    g_zip_ctrl.comp_log_ctrl->zip_info = zip_info;

    g_zip_ctrl.comp_log_ctrl->misc.minor = MISC_DYNAMIC_MINOR;
    g_zip_ctrl.comp_log_ctrl->misc.name = kstrdup("zipdev", GFP_KERNEL);
    if (g_zip_ctrl.comp_log_ctrl->misc.name == NULL) { /*lint !e730: (Info -- Boolean argument to function)*/
        ret = EINVAL;
        goto out_free_log;
    }

    g_zip_ctrl.comp_log_ctrl->misc.fops = &g_comp_dev_fops;
    g_zip_ctrl.comp_log_ctrl->misc.parent = NULL;

    init_waitqueue_head(&g_zip_ctrl.comp_log_ctrl->wq);
    mutex_init(&g_zip_ctrl.comp_log_ctrl->mutex);

    /* finally, initialize the misc device for this log */
    ret = misc_register(&g_zip_ctrl.comp_log_ctrl->misc);
    if (ret) { /*lint !e730: (Info -- Boolean argument to function)*/
        dump_error("failed to register misc device for log '%s'!\n",
                   g_zip_ctrl.comp_log_ctrl->misc.name); /*lint !e429*/
        goto out_free_name;
    }

    dump_print("created zip dev '%s'\n", g_zip_ctrl.comp_log_ctrl->misc.name);
    return 0; /*lint !e429*/

out_free_name:
    kfree(g_zip_ctrl.comp_log_ctrl->misc.name);
    g_zip_ctrl.comp_log_ctrl->misc.name = NULL;
out_free_log:
    kfree(g_zip_ctrl.comp_log_ctrl);
    g_zip_ctrl.comp_log_ctrl = NULL;
out:
    return ret;
}

int dump_get_compress_list(void)
{
    device_node_s *dev = NULL;
    u32 listsize = 0;
    u32 index = 0;
    int ret;
    const char *temp = NULL;

    dev = bsp_dt_find_compatible_node(NULL, NULL, "driver,dump_compress");
    if (dev == NULL) {
        dump_error("dump compress dts node no find!\n");
        return -1;
    }

    if (bsp_dt_property_read_u32(dev, "listsize", &listsize)) {
        listsize = 0;
        dump_error("fail to get listsize!\n");
        return 0;
    }
    g_zip_ctrl.zipintf_info.mfilenum = listsize;

    for (index = 0; index < listsize; index++) {
        ret = bsp_dt_property_read_string_index(dev, "dump_compress_name", index, &temp);
        if (ret != 0 || temp == NULL) {
            dump_error("fail to read dump_compress_name\n");
        } else {
            if (0 > snprintf_s(g_zip_ctrl.zipintf_info.pfile_list[index], MAX_COMPRESS_FILE_NAME,
                MAX_COMPRESS_FILE_NAME - 1, "%s", (char *)temp)) {
                dump_error("snprintf err\n");
            }
        }
    }

    return 0;
}


bool dump_is_minidump(void)
{
    u32 attr = g_dump_file_info.current_attr;
    if ((attr & DUMP_ATTR_SAVE_MINIDUMP) == DUMP_ATTR_SAVE_MINIDUMP) {
        return true;
    }
    return false;
}

bool dump_is_fulldump(void)
{
    u32 attr = g_dump_file_info.current_attr;
    if ((attr & DUMP_ATTR_SAVE_FULLDUMP) == DUMP_ATTR_SAVE_FULLDUMP) {
        return true;
    }
    return false;
}

bool dump_is_secdump(void)
{
    device_node_s *node = NULL;
    node = bsp_dt_find_compatible_node(NULL, NULL, "driver,sec_dump");
    if (node == NULL) {
        return false;
    }
    return true;
}

s32 dump_trigger_compress(const char *logpath, int pathlen, struct dump_file_save_info_s *datainfo)
{
    if (!dump_is_fulldump()) {
        return BSP_OK;
    }
    if (pathlen >= COMPRESS_FILE_PATH_LEN) {
        dump_error("pathlen %d too big.\n", pathlen);
        return BSP_ERROR;
    }
    dump_print("dump_trigger_compress here for path %s.\n", logpath);

    if (g_zip_ctrl.comp_log_ctrl == NULL || logpath == NULL || datainfo == NULL) {
        dump_error("param is NULL.\n");
        return BSP_ERROR;
    }

    if (0 > snprintf_s(g_zip_ctrl.zipintf_info.mfilepath, COMPRESS_FILE_PATH_LEN, COMPRESS_FILE_PATH_LEN - 1, "%s",
        (char *)logpath)) {
        dump_error("snprintf err\n");
        return BSP_ERROR;
    }
    if (EOK != memcpy_s(&g_zip_ctrl.zipintf_info.saveinfo, sizeof(struct dump_file_save_info_s), datainfo,
        sizeof(struct dump_file_save_info_s))) {
        dump_error("memcpy err\n");
        return BSP_ERROR;
    }
    dump_print("dump_trigger_compress wakeup for path %s, file %s.\n", g_zip_ctrl.zipintf_info.mfilepath,
        g_zip_ctrl.zipintf_info.pfile_list[0]);
    mutex_lock(&g_zip_ctrl.comp_log_ctrl->mutex);
    g_zip_ctrl.comp_log_ctrl->trigger_flag = 1;
    mutex_unlock(&g_zip_ctrl.comp_log_ctrl->mutex);
    wake_up_interruptible(&g_zip_ctrl.comp_log_ctrl->wq);

    return BSP_OK;
}

void dump_wait_compress_done(const char *log_path)
{
    char compfilename[COMPRESS_FILE_PATH_LEN] = {0};
    int count = 0;

    if (!dump_is_fulldump()) {
        return;
    }
    if (log_path == NULL) {
        dump_error("log_path err\n");
        return;
    }

    // wait compress done
    dump_print("[0x%x] wait compress done\n", bsp_get_slice_value());

    if (g_zip_ctrl.zipintf_info.mfilenum > 0) {
        count = 0;
        if (0 > snprintf_s(compfilename, COMPRESS_FILE_PATH_LEN, COMPRESS_FILE_PATH_LEN - 1, "%sCOMPDONE", log_path)) {
            dump_error("snprintf err.\n");
        }
        while (bsp_eaccess(compfilename)) {
            if (count++ < DUMP_COMPRESS_TIMEOUT_CNTS) {
                msleep(DUMP_COMPRESS_WAIT_PER_CNT);
            } else {
                break;
            }
        }
    }
    dump_print("[0x%x] compress done with count %d.\n", bsp_get_slice_value(), count);
}

void dump_logzip_init(void)
{
    if (!dump_is_fulldump()) {
        return;
    }

    if (dump_get_compress_list()) {
        dump_error("get compress list failed !\n");
        return;
    }

    if (dump_comp_dev_register(&g_zip_ctrl.zipintf_info)) {
        dump_error("comp_dev_register fail\n");
        return;
    }
}

dump_product_type_e bsp_dump_get_product_type(void)
{
    u32 attr = g_dump_file_info.current_attr;
    if ((attr & DUMP_ATTR_SAVE_MBB) == DUMP_ATTR_SAVE_MBB) {
        return DUMP_MBB;
    } else if ((attr & DUMP_ATTR_SAVE_PHONE) == DUMP_ATTR_SAVE_PHONE) {
        return DUMP_PHONE;
    }
    return DUMP_PRODUCT_BUTT;
}

dump_file_attr_s *dump_get_file_attr(dump_save_file_id_e id)
{
    u32 i;
    for (i = 0; i < g_dump_file_info.file_num; i++) {
        if (id == g_dump_file_info.file_list[i].id) {
            return &g_dump_file_info.file_list[i];
        }
    }
    return NULL;
}

const dump_file_attr_s *dump_get_file_attr_by_name(const char *file_name)
{
    u32 i;
    for (i = 0; i < g_dump_file_info.file_num; i++) {
        if (strncmp(g_dump_file_info.file_list[i].name, file_name, DUMP_FILE_NAME_LEN) == 0) {
            return &g_dump_file_info.file_list[i];
        }
    }
    return NULL;
}

bool dump_get_file_save_flag(dump_save_file_id_e id)
{
    dump_file_attr_s *fileinfo = dump_get_file_attr(id);
    if (fileinfo != NULL) {
        return fileinfo->save_flag;
    }
    return false;
}

dump_subsys_e dump_get_file_subsys(dump_save_file_id_e id)
{
    u32 attr;
    dump_file_attr_s *fileinfo = dump_get_file_attr(id);
    if (fileinfo != NULL) {
        attr = fileinfo->attr >> DUMP_FILE_SYS_OFFSET;
        switch (attr) {
            case BIT(SUBSYS_AP):
                return SUBSYS_AP;
            case BIT(SUBSYS_CP):
                return SUBSYS_CP;
            case BIT(SUBSYS_M3):
                return SUBSYS_M3;
            case SUBSYS_COMM_ATTR:
                return SUBSYS_COMM;
            default:
                return SUBSYS_BUTT;
        }
    }
    return SUBSYS_BUTT;
}

bool dump_file_is_secdump_attr(dump_save_file_id_e id)
{
    u32 attr;
    dump_file_attr_s *fileinfo = dump_get_file_attr(id);
    if (fileinfo != NULL) {
        attr = fileinfo->attr;
        if ((attr & DUMP_ATTR_SAVE_SEC_DUMP) == DUMP_ATTR_SAVE_SEC_DUMP) {
            return true;
        } else {
            return false;
        }
    }
    return false;
}

static inline bool is_current_plat_match(u32 except_attr)
{
    u32 current_plat = DUMP_ATTR_DECODE_PLAT(g_dump_file_info.current_attr);
    u32 except_plat = DUMP_ATTR_DECODE_PLAT(except_attr);
    return DUMP_EXISTS_ATTRIBUTE(except_plat, current_plat);
}

static inline bool is_current_dump_level_match(u32 except_attr)
{
    u32 current_level = DUMP_ATTR_DECODE_DUMP_LEVEL(g_dump_file_info.current_attr);
    u32 except_level = DUMP_ATTR_DECODE_DUMP_LEVEL(except_attr);
    return DUMP_EXISTS_ATTRIBUTE(except_level, current_level);
}

static inline bool is_current_ddr_access_match(u32 except_attr)
{
    u32 current_feature = DUMP_ATTR_DECODE_DDR_ACCESS(g_dump_file_info.current_attr);
    u32 except_feature = DUMP_ATTR_DECODE_DDR_ACCESS(except_attr);
    return DUMP_MATCH_ATTRIBUTE(except_feature, current_feature);
}

static inline bool is_current_feature_match(u32 except_attr)
{
    bool flag;
    /* 查看unsec_access和sec dump是否同时开启 */
    /* 如果同时开启，对应的unsec_access的log就不用存,如果不是同时开启，是否保存取决于unsec_access属性 */
    u32 current_feature = DUMP_ATTR_DECODE_FEATURE(g_dump_file_info.current_attr);
    u32 except_feature = DUMP_ATTR_DECODE_FEATURE(except_attr);
    flag = DUMP_SEC_ATTR_MATCH(except_feature, current_feature);
    if (flag == true) {
        return false;
    }
    return is_current_ddr_access_match(except_attr);
}


bool dump_is_file_need_save(dump_save_file_id_e id)
{
    dump_file_attr_s *file_info = dump_get_file_attr(id);
    if (file_info == NULL) {
        return false;
    }
    if (!is_current_plat_match(file_info->attr)) {
        return false;
    }
    if (!is_current_dump_level_match(file_info->attr)) {
        return false;
    }
    if (!is_current_feature_match(file_info->attr)) {
        return false;
    }
    return true;
}

bool dump_can_unsec_access_ddr(void)
{
    //主线默认安全访问modem_ddr，不再支持配置为非安全
    return false;
}

s32 dump_get_cur_attr_flag(void)
{
    return g_dump_file_info.attr_flag;
}

void dump_cur_attr_init(void)
{
    if (g_dump_file_info.attr_flag == BSP_TRUE) {
        return;
    }


#ifdef CONFIG_MODEM_MINI_DUMP
    g_dump_file_info.current_attr |= DUMP_ATTR_SAVE_MINIDUMP;
#endif


#ifdef BSP_CONFIG_PHONE_TYPE
    g_dump_file_info.current_attr |= DUMP_ATTR_SAVE_PHONE;
#else
    g_dump_file_info.current_attr |= DUMP_ATTR_SAVE_MBB;
#endif
    if (dump_can_unsec_access_ddr() == true) {
        g_dump_file_info.current_attr |= DUMP_ATTR_SAVE_DDR_UNSEC_ACCESS;
    }
    if (dump_is_secdump() == true) {
        g_dump_file_info.current_attr |= DUMP_ATTR_SAVE_SEC_DUMP;
    }

    g_dump_file_info.attr_flag = BSP_TRUE;
    dump_print("current_plat_attr=0x%x, num=%d!\n", g_dump_file_info.current_attr, g_dump_file_info.file_num);
}

dump_file_attr_s *dump_get_file_info_storage(dump_save_file_id_e id)
{
    u32 i;
    for (i = 0; i < g_dump_file_info.file_num; i++) {
        if (id == g_dump_file_info.file_list[i].id) {
            dump_error("file[%d] attr overlay\n", id);
            return &g_dump_file_info.file_list[i];
        }
    }
    if (i < g_dump_file_info.list_len) {
        return &g_dump_file_info.file_list[i];
    }
    return NULL;
}

s32 dump_file_attr_init(device_node_s *dev)
{
    const char *file_name = NULL;
    dump_file_attr_s *file_info = NULL;
    u32 ret, file_id;

    if (dev == NULL) {
        dump_error("para err!\n");
        return BSP_ERROR;
    }
    file_id = 0xFFFFFFFF;
    ret = (u32)bsp_dt_property_read_u32(dev, "file_id", &file_id);
    if (ret != 0) {
        dump_error("get file_attr fail!\n");
        return (int)ret;
    }
    file_info = dump_get_file_info_storage(file_id);
    if (file_info == NULL) {
        dump_error("file list ovf\n");
    }
    file_info->id = file_id;
    ret = (u32)bsp_dt_property_read_u32(dev, "save_flag", (u32 *)(uintptr_t)(&file_info->save_flag));
    ret |= (u32)bsp_dt_property_read_string(dev, "file_name", &file_name);
    if ((ret != 0) || (file_name == NULL)) {
        dump_error("get file_attr fail!\n");
        return (int)ret;
    }
    ret = (u32)strncpy_s(file_info->name, sizeof(file_info->name), file_name, strlen(file_name));
    if (ret != EOK) {
        dump_error("file_name strncpy filed!\n");
        return (int)ret;
    }

    if (bsp_dt_property_read_u32(dev, "attribute", &file_info->attr)) {
        dump_error("attribute not found!\n");
        return BSP_ERROR;
    }
    g_dump_file_info.file_num++;
    return BSP_OK;
}

s32 dump_get_file_cfg_flag(void)
{
    return g_dump_file_info.file_flag;
}

void dump_file_cfg_init(void)
{
    device_node_s *dev = NULL;
    device_node_s *child = NULL;
    u32 num = 0;

    if (g_dump_file_info.file_flag == BSP_TRUE) {
        return;
    }

    dev = bsp_dt_find_node_by_path("/dump_file_cfg");
    if (dev == NULL) {
        dump_error("dump_file_cfg not found");
    }
    num = (u32)bsp_dt_get_child_count(dev);
    num += dump_get_extra_logs_file_num();
    g_dump_file_info.file_list = osl_malloc(num * sizeof(dump_file_attr_s));
    if (g_dump_file_info.file_list == NULL) {
        dump_error("file_list is null!");
        return;
    }
    g_dump_file_info.list_len = num;
    bsp_dt_for_each_child_of_node(dev, child)
    {
        (void)dump_file_attr_init(child);
    }
    /* iot定制的extra fulldump日志属性初始化，需要在file_cfg节点后初始化，用于覆盖原始配置文件 */
    dump_extra_logs_init();

    g_dump_file_info.file_flag = BSP_TRUE;
}

