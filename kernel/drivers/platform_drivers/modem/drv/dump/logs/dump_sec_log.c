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

#include <linux/kernel.h>
#include <bsp_dt.h>
#include <linux/of_device.h> /* for of_dma_configure */
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/mm.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/version.h>
#include <asm/cacheflush.h>
#include <securec.h>
#include <osl_types.h>
#include <osl_bio.h>
#include <osl_io.h>
#include <osl_malloc.h>
#include <bsp_slice.h>
#include <bsp_nvim.h>
#include <bsp_rfile.h>
#include <bsp_odt.h>
#include <bsp_dump.h>
#include <bsp_fiq.h>
#include <bsp_reset.h>
#include <dump_frame.h>
#include <dump_area.h>
#include "dump_logs.h"
#include "dump_sec_log.h"
#include "dump_log_strategy.h"
#include "dump_extra_logs.h"
#include "dump_teeos_service.h"

#undef THIS_MODU
#define THIS_MODU mod_dump

int dump_sec_channel_init(void);

#define DUMP_SECMEM_WAIT_DATARATE_MS (60 * 1024) /* odt每秒120M/s, 以50M/s来计算超时 */

struct platform_device *g_dump_sec_pdev = NULL;
u8 *g_sec_dump_debug = NULL;
dump_sec_secos_packet_s *g_sec_dump_secos_packet = NULL;

dump_sec_drvdata_s g_sec_dump_drvdata = {
    .odt_dst_info = {
        1,
        ODT_CODER_DST_DUMP,
        DUMP_SEC_CODER_DST_IND_SIZE,
        DUMP_SEC_CODER_DST_THRESHOLD,
        ODT_TIMEOUT_TFR_LONG,
        NULL,
        NULL,
        NULL,
    }
};

static inline void dump_sec_invalid_cache(void *ptr, u32 size)
{
    (void)dma_map_single(&g_dump_sec_pdev->dev, ptr, size, DMA_FROM_DEVICE);
}

dump_ddr_trans_head_info_s *dump_sec_get_modem_trance_node(dump_ddr_trans_head_info_s *trancs_list,
                                                           dump_ddr_trans_head_info_s *dump_head, u32 head_size)
{
    u32 i = 0;
    dump_ddr_trans_head_info_s *trans_head = NULL;
    if (trancs_list == NULL || dump_head == NULL) {
        dump_error("get sec dump field fail\n");
        return NULL;
    }

    for (i = 0; i < head_size / sizeof(dump_ddr_trans_head_info_s); i++) {
        trans_head = &trancs_list[i];
        if (trans_head->magic == DUMP_SEC_TRANS_FILE_MAGIC &&
            0 == strncmp(trans_head->file_name, dump_head->file_name, strlen(dump_head->file_name))) {
            return trans_head;
        }
    }
    return NULL;
}

dump_ddr_trans_head_info_s *dump_sec_get_trance_info(dump_ddr_trans_head_info_s *dump_head)
{
    u8 *addr = NULL;
    u32 field_len = SEC_DUMP_CP_FILE_HEADER_SIZE_MAX;
    dump_field_map_s *field_desc = NULL;
    dump_ddr_trans_head_info_s *node = NULL;
    addr = bsp_dump_get_field_addr(DUMP_LRCCPU_SEC_DUMP_TARNS);
    field_desc = (dump_field_map_s *)bsp_dump_get_field_map(DUMP_LRCCPU_SEC_DUMP_TARNS);
    if (field_desc != NULL) {
        if (field_desc->length < SEC_DUMP_CP_FILE_HEADER_SIZE_MAX) {
            field_len = field_desc->length;
        }
        node = dump_sec_get_modem_trance_node((dump_ddr_trans_head_info_s *)addr, dump_head, field_len);
    }
    if (node == NULL) {
        node = dump_sec_get_modem_trance_node(g_sec_dump_drvdata.odt_src_info.dump_sec_header_vaddr, dump_head,
                                              SEC_DUMP_FILE_HEADER_SIZE);
    }
    return node;
}

bool dump_check_default_sec_dump(void)
{
    u32 i;
    dump_sec_file_info_s *file_info = g_sec_dump_secos_packet->file_list;
    for (i = 0; i < SEC_DUMP_RETRANS_FILE_LIST_NUM; i++) {
        if (file_info[i].attribute & DUMP_ATTR_SAVE_DEFAULT) {
            return false;
        }
    }
    return true;
}

void dump_sec_debug(dump_ddr_trans_head_info_s *head_info)
{
    dump_ddr_trans_head_info_s *dump_head = NULL;
    u32 i = 0;
    if (g_sec_dump_debug == NULL) {
        return;
    }

    if (head_info == NULL) {
        dump_error("head_info is null\n");
        return;
    }
    dump_head = (dump_ddr_trans_head_info_s *)(uintptr_t)g_sec_dump_debug;
    for (i = 0; i < DUMP_SEC_DEBUG_SIZE / (sizeof(dump_ddr_trans_head_info_s)); i++) {
        if (dump_head[i].magic == DUMP_SEC_TRANS_FILE_MAGIC) {
            continue;
        }
        if (memcpy_s(&dump_head[i], sizeof(dump_ddr_trans_head_info_s), head_info,
                    sizeof(dump_ddr_trans_head_info_s))) {
            dump_debug("error");
        }
        break;
    }
}

void dump_sec_debug_clear_all(void)
{
    (void)memset_s((void *)g_sec_dump_debug, DUMP_SEC_DEBUG_SIZE, 0, DUMP_SEC_DEBUG_SIZE);
}

s32 dump_sec_check_head_info(dump_ddr_trans_head_info_s *head_info)
{
    dump_ddr_trans_head_info_s *node = NULL;
    node = dump_sec_get_trance_info(head_info);
    if (node != NULL) {
        if (node->total_length == head_info->total_length) {
            dump_print("%s total_len(0x%x) match\n", node->file_name, node->total_length);
            return BSP_OK;
        }
        return BSP_ERROR;
    }
    dump_print("can not get node \n");
    return BSP_ERROR;
}

void dump_sec_mask_file_saved(dump_ddr_trans_head_info_s *head_info)
{
    u32 i;
    for (i = 0; (i < SEC_DUMP_RETRANS_FILE_LIST_NUM) && g_sec_dump_secos_packet->file_list[i].name[0]; i++) {
        if (strncmp(head_info->file_name, g_sec_dump_secos_packet->file_list[i].name, DUMP_SECDUMP_NAME_LEN) == 0) {
            g_sec_dump_secos_packet->file_list[i].attribute &= ~DUMP_ATTR_SAVE_DEFAULT;
            break;
        }
    }
}

bool dump_mask_default_sec_dump_unsaved(void)
{
    u32 i;
    dump_sec_file_info_s *file_info = g_sec_dump_secos_packet->file_list;
    for (i = 0; i < SEC_DUMP_RETRANS_FILE_LIST_NUM; i++) {
        if (file_info[i].name[0] != '\0') {
            file_info[i].attribute |= DUMP_ATTR_SAVE_DEFAULT;
        } else {
            break;
        }
    }
    return true;
}
struct file *dump_sec_get_file(const char *path, dump_ddr_trans_head_info_s *head_info)
{
    int ret, total_length;
    struct file *fd = NULL;
    struct dump_file_save_strategy log_strategy;
    char file_name[MODEM_DUMP_FILE_NAME_LENGTH] = {0};

    total_length = (int)(head_info->total_length);

    ret = dump_match_log_strategy(head_info->file_name, &log_strategy);
    if (ret) {
        if (snprintf_s(file_name, MODEM_DUMP_FILE_NAME_LENGTH, (strlen(path) + strlen(head_info->file_name)), "%s%s",
                       path, head_info->file_name) < 0) {
            dump_error("er\n");
            return NULL;
        }
        fd = dump_open((char *)file_name, O_CREAT | O_RDWR, DUMP_LOG_FILE_AUTH);
    } else {
        log_strategy.data_head.filelength = total_length;
        if (snprintf_s(file_name, MODEM_DUMP_FILE_NAME_LENGTH,
                       (strlen(path) + strlen(log_strategy.data_head.dstfilename)), "%s%s", path,
                       log_strategy.data_head.dstfilename) < 0) {
            dump_error("er\n");
            return NULL;
        }
        if (dump_append_file(path, file_name, &log_strategy.data_head, sizeof(log_strategy.data_head), 0xFFFFFFFF)) {
            dump_error("add head err\n");
            return NULL;
        }
        fd = dump_open((char *)file_name, O_APPEND | O_RDWR, DUMP_LOG_FILE_AUTH);
    }
    return fd;
}

unsigned int dump_sec_odt_receive(odt_buffer_rw_s *odt_buff, u8 *writebuff, int total_length)
{
    void *buffer = NULL;
    void *rb_buffer = NULL;
    unsigned int rd_length = 0;

    buffer = (void *)phys_to_virt((uintptr_t)odt_buff->buffer);
    dump_sec_invalid_cache(buffer, odt_buff->size);
    if ((u32)total_length >= odt_buff->size) {
        rd_length = odt_buff->size;
        total_length -= odt_buff->size;
        if (memcpy_s(writebuff, g_sec_dump_drvdata.odt_dst_info.buf_len, buffer, rd_length) != EOK) {
            bsp_debug("err\n");
        }
    } else {
        rd_length = total_length;
        if (memcpy_s(writebuff, g_sec_dump_drvdata.odt_dst_info.buf_len, buffer, rd_length) != EOK) {
            bsp_debug("err\n");
        }
        goto recv_done;
    }

    /* 回卷长度有可能为0 */
    if ((odt_buff->rb_size) && (odt_buff->rb_buffer)) {
        rb_buffer = (void *)phys_to_virt((phys_addr_t)(uintptr_t)odt_buff->rb_buffer);
        dump_sec_invalid_cache(rb_buffer, odt_buff->rb_size);
        if (total_length >= odt_buff->rb_size) {
            if (memcpy_s((void *)(writebuff + rd_length), (g_sec_dump_drvdata.odt_dst_info.buf_len - rd_length),
                         rb_buffer, odt_buff->rb_size) != EOK) {
                bsp_debug("err\n");
            }
            rd_length += odt_buff->rb_size;
        } else if (total_length > 0) {
            if (memcpy_s((void *)(writebuff + rd_length), (g_sec_dump_drvdata.odt_dst_info.buf_len - rd_length),
                         rb_buffer, total_length) != EOK) {
                bsp_debug("err\n");
            }
            rd_length += total_length;
        }
    }
recv_done:
    /* first : read done */
    (void)bsp_odt_read_data_done(g_sec_dump_drvdata.odt_dst_info.dst_channel_id, rd_length);
    return rd_length;
}

int dump_sec_save_single_file_soc(const char *path, dump_ddr_trans_head_info_s *head_info)
{
    odt_buffer_rw_s odt_buff = {0};
    unsigned int rd_length = 0;
    unsigned int start, end;
    int wait_conter, sleep_gap;
    int ret, total_length;
    struct file *fd;
    u8 *writebuff = g_sec_dump_drvdata.odt_dst_info.temp_buf;
    total_length = (int)(head_info->total_length);

    start = bsp_get_slice_value();
    fd = dump_sec_get_file(path, head_info);
    if (fd == NULL) {
        dump_error("open %s  error!\n", head_info->file_name);
        return -1;
    }

    /* 根据数据总量和buffer大小，计算需要接收多少次数据 */
    wait_conter = total_length / g_sec_dump_drvdata.odt_dst_info.buf_len + 1;
    sleep_gap = (g_sec_dump_drvdata.odt_dst_info.buf_len / DUMP_SECMEM_WAIT_DATARATE_MS) + 1;
    while (total_length > 0) {
        ret = bsp_odt_get_read_buff(g_sec_dump_drvdata.odt_dst_info.dst_channel_id, &odt_buff);
        if (ret) {
            dump_close(fd);
            dump_error("get odt buffer error!\n");
            return -1;
        }

        if ((!(odt_buff.rb_size + odt_buff.size)) && (total_length > 0)) {
            dump_debug("odt buffer space is null,total len is 0x%x,wait\n", total_length);

            if (wait_conter > 0) {
                msleep(sleep_gap);
                wait_conter--;
                continue;
            } else {
                dump_close(fd);
                dump_error("odt buffer space is null!\n");
                return -1;
            }
        }
        rd_length = dump_sec_odt_receive(&odt_buff, writebuff, total_length);
        total_length -= rd_length;
        /* second : then write file ,need to keep time for odt encoder */
        ret = (u32)dump_write_sync(fd, writebuff, rd_length);
        if (ret != rd_length) {
            dump_error("write error %d,%d!\n", ret, rd_length);
        }
    }
    (void)dump_close(fd);
    end = bsp_get_slice_value();
    dump_error("%s save done! ,slice: 0x%x\n", head_info->file_name, (end - start));
    dump_sec_mask_file_saved(head_info);
    return 0;
}

/*lint -save -e574*/
int dump_sec_save_single_file(const char *path, dump_ddr_trans_head_info_s *head_info)
{
    if (head_info == NULL) {
        dump_debug("error\n");
        return -1;
    }

    if (BSP_ERROR == dump_sec_check_head_info(head_info)) {
        return -1;
    }
    dump_sec_debug(head_info);
    dump_sec_save_single_file_soc(path, head_info);
    return BSP_OK;
}

/*lint -restore +e574*/
void dump_sec_save_file(const char *path)
{
    odt_buffer_rw_s odt_buff;
    int ret = 0;
    u32 waitcount = 10;
    dump_ddr_trans_head_info_s head_info;

    /* 未使能安全dump，直接退出即可 */
    if (!g_sec_dump_drvdata.feature_on) {
        dump_print("sec dump not support\n");
        return;
    }

    if (g_sec_dump_drvdata.odt_dst_info.init_state) {
        dump_error("sec dump has not init\n");
        return;
    }
    dump_error("start to save sec log\n");
    if (memset_s(&odt_buff, sizeof(odt_buff), 0, sizeof(odt_buff)) != EOK) {
        dump_debug("err\n");
    }

    /* 保存过程中，如果单独复位流程启动，停止保存dump信息 */
    while (g_sec_dump_drvdata.dump_state == SEC_DUMP_STATE_SAVING) {
        ret = bsp_odt_get_read_buff(g_sec_dump_drvdata.odt_dst_info.dst_channel_id, &odt_buff);
        if (ret) {
            dump_error("no file need save\n");
            return;
        }

        if ((odt_buff.size + odt_buff.rb_size) < sizeof(dump_ddr_trans_head_info_s)) { /* 是否readdone执行 */
            (void)bsp_odt_read_data_done(g_sec_dump_drvdata.odt_dst_info.dst_channel_id,
                                         odt_buff.size + odt_buff.rb_size);
            dump_print("wait odt data!\n");
            if (waitcount--) {
                msleep(DUMP_SLEEP_5MS_NUM);
                continue;
            } else {
                dump_error("log save stop\n");
                return;
            }
        }

        /* odt寄存器已更新，但是可能还没写到ddr，导致读取到的数据不正确，这取决于上一包的处理速度；
         * 如果上一包drop了，那么容易触发该场景
         */
        msleep(1);
        /* we have already check odt rd length before, so just ignore the ret val */
        (void)dump_sec_odt_receive(&odt_buff, (u8 *)&head_info, sizeof(dump_ddr_trans_head_info_s));

        if (head_info.magic != DUMP_SEC_TRANS_FILE_MAGIC) { /* 是否readdone执行 */
            dump_error("file magic num error,readdone data 0x%x\n", head_info.magic);
            return;
        }

        (void)dump_sec_save_single_file(path, &head_info);
        waitcount = DUMP_SECDUMP_WAIT_CNTS;
    }
    return;
}

int dump_sec_trans_mem_init(void)
{
    void *virt_addr = NULL;
    dma_addr_t phy_addr = 0;

    if (g_sec_dump_drvdata.odt_dst_info.dst_buf != NULL)
        return 0;

    g_sec_dump_drvdata.odt_dst_info.temp_buf = (void *)vmalloc(g_sec_dump_drvdata.odt_dst_info.buf_len);
    if (g_sec_dump_drvdata.odt_dst_info.temp_buf == NULL) {
        dump_error("tempucBuff vmalloc fail\n");
        return -1;
    }

    if (g_dump_sec_pdev) {
        virt_addr = dma_alloc_coherent(&g_dump_sec_pdev->dev, g_sec_dump_drvdata.odt_dst_info.buf_len, &phy_addr,
                                       GFP_KERNEL);
    }

    if (virt_addr == NULL) {
        dump_error("dma_alloc_coherent error\n");
        return -1;
    }

    g_sec_dump_drvdata.odt_dst_info.dst_buf = virt_addr;
    g_sec_dump_drvdata.odt_dst_info.dst_buf_phy = (u8 *)(uintptr_t)phy_addr;
    return 0;
}

int dump_sec_dst_channel_free(void)
{
    /* 未使能安全dump，直接退出即可 */
    if (!g_sec_dump_drvdata.feature_on) {
        return BSP_OK;
    }

    bsp_odt_free_enc_dst_chan(g_sec_dump_drvdata.odt_dst_info.dst_channel_id);
    if (g_dump_sec_pdev) {
        dma_free_coherent(&g_dump_sec_pdev->dev, g_sec_dump_drvdata.odt_dst_info.buf_len,
                          g_sec_dump_drvdata.odt_dst_info.dst_buf,
                          (dma_addr_t)(uintptr_t)g_sec_dump_drvdata.odt_dst_info.dst_buf_phy);
    }
    g_sec_dump_drvdata.odt_dst_info.dst_buf = NULL;
    g_sec_dump_drvdata.odt_dst_info.dst_buf_phy = 0;
    if (g_sec_dump_drvdata.odt_dst_info.temp_buf != NULL) {
        vfree(g_sec_dump_drvdata.odt_dst_info.temp_buf);
        g_sec_dump_drvdata.odt_dst_info.temp_buf = NULL;
    }
    g_sec_dump_drvdata.odt_dst_info.init_state = 1;

    return BSP_OK;
}

int bsp_dump_sec_channel_free(drv_reset_cb_moment_e eparam, s32 usrdata)
{
    int count = DUMP_SECDUMP_WAIT_DUMP_IDLE_MAX_DELAY;
    unsigned long flags = 0;
    UNUSED(eparam);
    UNUSED(usrdata);

    if (!g_sec_dump_drvdata.feature_on) {
        return 0;
    }

    /* 防止单独复位与dump保存流程耦合 */
    spin_lock_irqsave(&g_sec_dump_drvdata.lock, flags);
    if (g_sec_dump_drvdata.dump_state == SEC_DUMP_STATE_IDLE) {
        spin_unlock_irqrestore(&g_sec_dump_drvdata.lock, flags);
        return 0;
    } else {
        g_sec_dump_drvdata.dump_state = SEC_DUMP_STATE_STOPPING;
    }
    spin_unlock_irqrestore(&g_sec_dump_drvdata.lock, flags);

    while ((g_sec_dump_drvdata.dump_state != SEC_DUMP_STATE_IDLE) && count){
        msleep(DUMP_SECDUMP_WAIT_DUMP_IDLE_SINGLE_TIMEMS);
        count--;
    }

    if (g_sec_dump_drvdata.dump_state == SEC_DUMP_STATE_IDLE) {
        return 0;
    } else {
        dump_error("wait secdump idle timeout, close froce\n");
    }

    /* 关闭源通道并清除配置信息，防止单独复位失败时，源通道bd描述符被覆盖后，还能打开源通道继续传输未知数据 */
    dump_sec_disable_trans(MODEM_SEC_DUMP_STOP_CP_CHANNEL_CMD);
    return 0;
}

void dump_sec_enable_trans(u32 cmd_type)
{
    int ret;

    /* 未使能安全dump，直接退出即可 */
    if (!g_sec_dump_drvdata.feature_on) {
        dump_print("sec dump not support\n");
        return;
    }
    bsp_odt_dst_channel_enable(ODT_CODER_DST_DUMP);
    dump_error("notify teeos enable odt channel\n");
    g_sec_dump_secos_packet->sec_dump_cmd = cmd_type;
    ret = dump_call_tee_service_ext(DUMP_TEE_CMD_SECDUMP, g_sec_dump_secos_packet, sizeof(dump_sec_secos_packet_s));
    if (ret) {
        dump_error("notify teeos error!\n");
    }
    msleep(2); /* 等待2ms */
}

void dump_sec_disable_trans(u32 cmd_type)
{
    int ret;
    u32 max_delay = DUMP_SECDUMP_WAIT_ODT_IDLE_MAX_DELAY;
    odt_buffer_rw_s odt_buff = {0};
    u32 stat;

    /* 未使能安全dump，直接退出即可 */
    if (!g_sec_dump_drvdata.feature_on) {
        dump_print("sec dump not support\n");
        return;
    }

    dump_error("notify teeos stop odt channel\n");

    g_sec_dump_secos_packet->sec_dump_cmd = cmd_type;
    ret = dump_call_tee_service_ext(DUMP_TEE_CMD_SECDUMP, g_sec_dump_secos_packet, sizeof(dump_sec_secos_packet_s));
    if (ret) {
        dump_error("notify teeos error!\n");
    }
    bsp_odt_dst_channel_disable(ODT_CODER_DST_DUMP);
    stat = bsp_odt_get_single_src_state(g_sec_dump_drvdata.odt_src_info.odt_cpsrc_chnl_id);
    while (stat && max_delay){
        /* 如果目的端之前有数据没处理，导致当前剩余目的buffer空间小于源端正在发送的bd包长，那么odt永远无法到达idle状态
         * 需要将目的端之前的数据丢掉，清理出足够的空间，接收odt fifo中的下一包bd数据
         */
        if ((bsp_odt_get_read_buff(g_sec_dump_drvdata.odt_dst_info.dst_channel_id, &odt_buff) == BSP_OK) &&
            (odt_buff.rb_size + odt_buff.size)) {
            bsp_odt_read_data_done(g_sec_dump_drvdata.odt_dst_info.dst_channel_id,
                                   odt_buff.size + odt_buff.rb_size);
        }
        msleep(DUMP_SECDUMP_WAIT_ODT_IDLE_SINGLE_TIMEMS);
        stat = bsp_odt_get_single_src_state(g_sec_dump_drvdata.odt_src_info.odt_cpsrc_chnl_id);
        max_delay--;
    }
    if (stat) {
        dump_error("wait odt src chan idle timeout\n");
        return;
    }
    /* free src channel cfg */
    g_sec_dump_secos_packet->sec_dump_cmd = 0;
    ret = dump_call_tee_service_ext(DUMP_TEE_CMD_SECDUMP, g_sec_dump_secos_packet, sizeof(dump_sec_secos_packet_s));
    if (ret) {
        dump_error("notify teeos error!\n");
    }
}

int dump_sec_channel_init(void)
{
    odt_dst_chan_cfg_s channel;

    if (dump_sec_trans_mem_init()) {
        return -1;
    }

    /* 扩大编码目的通道1阈值门限 */
    channel.encdst_thrh = 0x2000;

    channel.coder_dst_setbuf.output_start = g_sec_dump_drvdata.odt_dst_info.dst_buf_phy;

    channel.coder_dst_setbuf.output_end =
        (g_sec_dump_drvdata.odt_dst_info.dst_buf_phy + g_sec_dump_drvdata.odt_dst_info.buf_len) - 1;

    channel.coder_dst_setbuf.threshold = g_sec_dump_drvdata.odt_dst_info.threshold;

    channel.encdst_timeout_mode = g_sec_dump_drvdata.odt_dst_info.timeout_mode;

    if (bsp_odt_coder_set_dst_chan(g_sec_dump_drvdata.odt_dst_info.dst_channel_id, &channel)) {
        g_sec_dump_drvdata.odt_dst_info.init_state = 1;
        return -1;
    }
    bsp_odt_dst_trans_id_disable(ODT_CODER_DST_DUMP);
    /* disable encdst interupt */
    (void)bsp_odt_encdst_dsm_init(g_sec_dump_drvdata.odt_dst_info.dst_channel_id, ODT_DEST_DSM_DISABLE);

    g_sec_dump_drvdata.odt_dst_info.init_state = 0;
    dump_error("sec dump channel init ok!\n");

    return 0;
}

static inline void dump_sec_file_extra_info_probe(dump_sec_file_info_s *dump_file_info)
{
    dump_load_info_s *dump_load = NULL;
    (void)dump_get_load_info(dump_load);
    if ((dump_load != NULL) && (dump_file_info->dumpfile == DUMP_SEC_MDM_DDR)) {
        dump_file_info->save_offset = dump_load->mdm_ddr_saveoff;
    }
}

int dump_sec_get_bd_buffer(void)
{
    u32 size = g_sec_dump_drvdata.odt_src_info.dump_sec_bd_size + g_sec_dump_drvdata.odt_src_info.dump_sec_header_size;
    dma_addr_t phy_addr = 0;
    u8 *vaddr = NULL;

    if (g_dump_sec_pdev != NULL) {
        vaddr = dma_alloc_coherent(&g_dump_sec_pdev->dev, size, &phy_addr, GFP_KERNEL);
    }
    g_sec_dump_drvdata.odt_src_info.dump_sec_bd_vaddr = vaddr;
    if (vaddr == NULL) {
        return BSP_ERROR;
    }
    (void)memset_s(vaddr, size, 0 ,size);
    g_sec_dump_drvdata.odt_src_info.dump_sec_bd_paddr = (u64)(uintptr_t)phy_addr;
    g_sec_dump_secos_packet->dump_sec_bd_paddr = g_sec_dump_drvdata.odt_src_info.dump_sec_bd_paddr;
    g_sec_dump_drvdata.odt_src_info.dump_sec_header_vaddr = (dump_ddr_trans_head_info_s *)((uintptr_t)vaddr +
                                                            g_sec_dump_drvdata.odt_src_info.dump_sec_bd_size);
    return BSP_OK;
}

void dump_sec_free_bd_buffer(void)
{
    u32 size = g_sec_dump_drvdata.odt_src_info.dump_sec_bd_size + g_sec_dump_drvdata.odt_src_info.dump_sec_header_size;

    if (g_dump_sec_pdev != NULL && g_sec_dump_drvdata.odt_src_info.dump_sec_bd_vaddr != NULL) {
        dma_free_coherent(&g_dump_sec_pdev->dev, size, g_sec_dump_drvdata.odt_src_info.dump_sec_bd_vaddr,
                          (dma_addr_t)(uintptr_t)g_sec_dump_drvdata.odt_src_info.dump_sec_bd_paddr);
        g_sec_dump_drvdata.odt_src_info.dump_sec_bd_vaddr = NULL;
        g_sec_dump_drvdata.odt_src_info.dump_sec_bd_paddr = 0;
        g_sec_dump_secos_packet->dump_sec_bd_paddr = 0;
    }
}

int dump_save_default_sec_log_force(const char *dir_name)
{
    u32 i;
    dump_sec_file_info_s *file_info = g_sec_dump_secos_packet->file_list;
    int ret;

    /* 未使能安全dump，直接退出即可 */
    if (!g_sec_dump_drvdata.feature_on) {
        dump_print("sec dump not support\n");
        return BSP_OK;
    }
    if (dump_check_default_sec_dump()) {
        return BSP_OK;
    }
    if (file_info->name[0] == '\0') {
        return BSP_OK;
    }

    if (dump_sec_channel_init() != BSP_OK) {
        return BSP_ERROR;
    }

    for (i = 0; (i < SEC_DUMP_RETRANS_FILE_LIST_NUM) && file_info->name[0]; i++, file_info++) {
        dump_sec_file_extra_info_probe(file_info);
    }
    if (g_sec_dump_drvdata.odt_src_info.dump_sec_bd_size != 0) {
        if (dump_sec_get_bd_buffer() != BSP_OK) {
            dump_error("get bd_size failed.\n");
            dump_sec_dst_channel_free();
            return BSP_ERROR;
        }
    }

    g_sec_dump_secos_packet->sec_dump_cmd = MODEM_SEC_DUMP_RETRANS_LOG_CMD;
    ret = dump_call_tee_service_ext(DUMP_TEE_CMD_SECDUMP, g_sec_dump_secos_packet, sizeof(dump_sec_secos_packet_s));
    if (ret) {
        dump_error("dump_save_default_sec_log_force failed!\n");
        dump_sec_dst_channel_free();
        dump_sec_free_bd_buffer();
        return BSP_ERROR;
    }
    dump_sec_enable_trans(MODEM_SEC_DUMP_ENABLE_CP_CHANNEL_CMD);
    dump_sec_save_file(dir_name);
    dump_sec_disable_trans(MODEM_SEC_DUMP_STOP_CP_CHANNEL_CMD);
    dump_sec_free_bd_buffer();
    dump_sec_dst_channel_free();
    return BSP_OK;
}

int dump_sec_file_probe(device_node_s *dev_node)
{
    int ret;
    u32 i;
    dump_sec_file_info_s *file_info = g_sec_dump_secos_packet->file_list;
    const char *file_name = NULL;
    u32 file_id;
    u32 attr;

    if (bsp_dt_property_read_u32(dev_node, "file_id", &file_id)) {
        dump_error("file_id not found!\n");
        return BSP_ERROR;
    }

    for (i = 0; (i < SEC_DUMP_RETRANS_FILE_LIST_NUM) && file_info->name[0]; i++, file_info++) {
        // 覆盖原有配置信息
        if (file_id == file_info->dumpfile) {
            break;
        }
    };
    if (i == SEC_DUMP_RETRANS_FILE_LIST_NUM) {
        dump_error("file_info_filed not enough, file:%s will not retrans!\n", dev_node->name);
        return BSP_ERROR;
    }

    if (bsp_dt_property_read_u32(dev_node, "attribute", &attr)) {
        dump_error("attribute not found!\n");
        return BSP_ERROR;
    }

    if (!dump_file_is_secdump_attr(file_id)) {
        dump_debug("%d not support sec escape!\n", file_id);
        return BSP_OK;
    }
    file_info->dumpfile = file_id;
    file_info->attribute = attr;
    ret = bsp_dt_property_read_string(dev_node, "file_name", &file_name);
    if (ret != 0 || file_name == NULL) {
        dump_error("file_name not found!\n");
        return BSP_ERROR;
    }

    ret = (u32)strncpy_s(file_info->name, sizeof(file_info->name), file_name, strlen(file_name));
    if (ret != EOK) {
        dump_error("file_name strncpy filed!\n");
        return ret;
    }
    file_info->length = 0;
    file_info->phy_addr = 0;

    return BSP_OK;
}

void dump_sec_file_cfg_init(void)
{
    device_node_s *dev_node = NULL;
    device_node_s *child = NULL;
    dev_node = bsp_dt_find_node_by_path("/dump_file_cfg");
    if (dev_node == NULL) {
        dump_error("dump_file_cfg not found");
    }

    bsp_dt_for_each_child_of_node(dev_node, child)
    {
        (void)dump_sec_file_probe(child);
    }
    dev_node = bsp_dt_find_node_by_path("/dump_extra_flash");
    if (dev_node == NULL) {
        dump_print("no cust dump_extra_logs");
    }
    bsp_dt_for_each_child_of_node(dev_node, child)
    {
        (void)dump_sec_file_probe(child);
    }
}

int dump_sec_odt_cfg_init(void)
{
    device_node_s *dev_node = NULL;
    u32 dts_size = 0;
    dev_node = bsp_dt_find_node_by_path("/sec_dump/secdump_odt_cfg");
    if (dev_node == NULL) {
        dump_error("secdump_odt_cfg not found");
        return BSP_ERROR;
    }
    if (bsp_dt_property_read_u32(dev_node, "cp_src_chnl", &g_sec_dump_drvdata.odt_src_info.odt_cpsrc_chnl_id)) {
        dump_error("cpsrc not found!\n");
        return BSP_ERROR;
    }
    if (bsp_dt_property_read_u32(dev_node, "bd_size", &g_sec_dump_drvdata.odt_src_info.dump_sec_bd_size)) {
        dump_error("cpsrc not found!\n");
        return BSP_ERROR;
    }
    if (bsp_dt_property_read_u32(dev_node, "dst_buf_size", &dts_size)) {
        dump_error("no cust dst_buf_size, use default\n");
    } else {
        g_sec_dump_drvdata.odt_dst_info.buf_len = (size_t)dts_size;
    }
    g_sec_dump_drvdata.odt_src_info.dump_sec_header_size = SEC_DUMP_FILE_HEADER_SIZE;

    g_sec_dump_secos_packet->odt_base_addr = g_sec_dump_drvdata.odt_src_info.odt_base_addr;
    g_sec_dump_secos_packet->odt_cpsrc_chnl_id = g_sec_dump_drvdata.odt_src_info.odt_cpsrc_chnl_id;
    g_sec_dump_secos_packet->dump_sec_bd_size = g_sec_dump_drvdata.odt_src_info.dump_sec_bd_size;
    g_sec_dump_secos_packet->dump_sec_header_size = SEC_DUMP_FILE_HEADER_SIZE;
    g_sec_dump_secos_packet->odt_dst_channel_id = g_sec_dump_drvdata.odt_dst_info.dst_channel_id;

    return BSP_OK;
}

static int dump_sec_probe(struct platform_device *pdev)
{
    int ret;
    u8 *secos_pkt_paddr = NULL;
    u64 mask = (u64)DMA_BIT_MASK(64); /*lint !e598 !e648*/

    spin_lock_init(&g_sec_dump_drvdata.lock);
    dma_set_mask_and_coherent(&pdev->dev, mask); /*lint !e598 !e648*/
    g_dump_sec_pdev = pdev;

    g_sec_dump_debug = bsp_dump_register_field(DUMP_MODEMAP_SEC_DUMP, "sec_dump",
                                               DUMP_SEC_DEBUG_SIZE + DUMP_SEC_SECOS_SIZE, SEC_DUMP_DEBUG_VER);
    if (g_sec_dump_debug != NULL) {
        g_sec_dump_secos_packet = (dump_sec_secos_packet_s *)((uintptr_t)g_sec_dump_debug + DUMP_SEC_DEBUG_SIZE);
        if (memset_s(g_sec_dump_debug, DUMP_SEC_DEBUG_SIZE + DUMP_SEC_SECOS_SIZE, 0,
                     DUMP_SEC_DEBUG_SIZE + DUMP_SEC_SECOS_SIZE) != EOK) {
            dump_debug("err\n");
        }
    } else {
        dump_debug("get sec_dump field err\n");
        return BSP_ERROR;
    }
    secos_pkt_paddr = bsp_dump_get_field_phy_addr(DUMP_MODEMAP_SEC_DUMP);
    if (secos_pkt_paddr == NULL) {
        dump_debug("err\n");
        return BSP_ERROR;
    }
    g_sec_dump_drvdata.secos_packet_paddr = (u64)(uintptr_t)secos_pkt_paddr + DUMP_SEC_DEBUG_SIZE;

    dump_sec_file_cfg_init();
    ret = dump_sec_odt_cfg_init();
    if (ret != BSP_OK) {
        return ret;
    }

    g_sec_dump_drvdata.dump_state = SEC_DUMP_STATE_IDLE;
    g_sec_dump_drvdata.feature_on = true;

    dump_error("sec dump init ok!\n");

    return 0;
}

static int dump_sec_remove(struct platform_device *pdev)
{
    return 0;
}

static const struct of_device_id g_dump_sec_match_table[] = {
    { .compatible = "driver,sec_dump" },
    {},
};

struct platform_device g_dump_sec_device = {
    .name = "dump_sec",
    .id = -1,
};

static struct platform_driver g_dump_sec_driver = {
    .probe = dump_sec_probe,
    .remove = dump_sec_remove,
    .driver = {
        .name = "dump_sec",
        .of_match_table = g_dump_sec_match_table,
        .probe_type = PROBE_FORCE_SYNCHRONOUS,
    },
};

void dump_save_cp_sec_log_files(const char *dir_name)
{
    if (dump_sec_channel_init() != BSP_OK) {
        return;
    }
    dump_sec_enable_trans(MODEM_SEC_DUMP_ENABLE_CP_CHANNEL_CMD);
    dump_sec_save_file(dir_name);
    dump_sec_disable_trans(MODEM_SEC_DUMP_STOP_CP_CHANNEL_CMD);
    dump_sec_dst_channel_free();
}

void dump_save_sec_log_files(const char *dir_name)
{
    unsigned long flags = 0;
    const char *extra_log_path = NULL;

    /* 逃生和debug信息重新恢复为初始状态 */
    dump_mask_default_sec_dump_unsaved();
    dump_sec_debug_clear_all();

    spin_lock_irqsave(&g_sec_dump_drvdata.lock, flags);
    g_sec_dump_drvdata.dump_state = SEC_DUMP_STATE_SAVING;
    spin_unlock_irqrestore(&g_sec_dump_drvdata.lock, flags);

    dump_save_cp_sec_log_files(dir_name);
    if (g_sec_dump_drvdata.dump_state == SEC_DUMP_STATE_STOPPING) {
        goto out;
    }

    extra_log_path = dump_get_extra_logs_log_path(dir_name);
    /* 只有MBB定制了extra_log_path后，且创建目录失败才会为NULL */
    if (extra_log_path != NULL) {
        dump_save_default_sec_log_force(extra_log_path);
    } else {
        dump_error("dump extra log path null\n");
    }

out:
    spin_lock_irqsave(&g_sec_dump_drvdata.lock, flags);
    g_sec_dump_drvdata.dump_state = SEC_DUMP_STATE_IDLE;
    spin_unlock_irqrestore(&g_sec_dump_drvdata.lock, flags);
}

int dump_sec_log_init(void)
{
    int ret;
    device_node_s *node = bsp_dt_find_compatible_node(NULL, NULL, "driver,sec_dump");

    if (node == NULL) {
        dump_error("no sec_dump node");
        return BSP_OK;
    }

    if (bsp_dt_device_is_available(node) == false) {  // 如果为false时，自己创建
        ret = platform_device_register(&g_dump_sec_device);
        if (ret) {
            dump_error("platform_device_register fail");
            return ret;
        }
        of_dma_configure(&g_dump_sec_device.dev, NULL, true);
        dma_set_mask_and_coherent(&g_dump_sec_device.dev, DUMP_SEC_DEV_DMAMASK);
    }

    ret = platform_driver_register(&g_dump_sec_driver);

    bsp_dump_register_log_notifier(DUMP_SAVE_SEC_LOGS, dump_save_sec_log_files);

    dump_error("dump_sec_init");
    return ret;
}

