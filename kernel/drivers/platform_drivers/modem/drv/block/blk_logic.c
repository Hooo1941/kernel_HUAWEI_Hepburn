/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <securec.h>
#include <mdrv_bmi_blk.h>
#include <bsp_print.h>
#include <bsp_partition.h>
#include <bsp_blk.h>
#include "blk_base.h"

#define THIS_MODU mod_blk

#define PB_HEADER_LENGTH 512
#define PB_HEADER_MAGIC 0x9048
#define PB_HEADER_MAGIC_DIFF 0xffff

#define EMMC_LOGIC_HANDLE_BLK 0x10
enum {
    BLK_READ = 0,
    BLK_WRITE,
};

enum {
    BLK_FLAG_UNMAPPED = 0,
    BLK_FLAG_MAPPED,
    BLK_FLAG_BAD,
};

struct pb_header {
    u16 magic;    // 初始化扫描分区时检查，用于确保此块数据合法性
    u16 version;  // 方案版本号，当前为0
    u32 lb_index; // 此物理块所属的逻辑块
    u64 sqnum;    // 块序列号，当多个pb的lb_index相同时，用于判断最新的pb。默认不会翻转。
};

struct lb_map {
    u32 pb_index;
    u32 mapped_flag; // 此逻辑块是否映射，即代表其pb_index的有效性
};

struct map_info {
    struct list_head list;   // 分区管理信息链表头，用于多个分区的map_info信息管理
    struct mutex part_mutex; // 读写访问互斥锁，每次读写操作都是互斥的
    const char *part_name; // 此管理信息所属的分区，用于读写操作时找到对应分区的map_info信息
    u32 phy_blksize;
    u32 logic_blksize;
    u64 logic_partsize;
    u32 lb_cnt;
    u32 pb_cnt;
    u32 latest_pb_index; // 当前最新有效的物理块，用于写操作找可写块时顺序查找下一个空闲物理块
    u64 latest_sqnum;      // 当前最新的序列号，用于写操作时更新序列号
    struct lb_map *lb_map; // 逻辑块映射表，用于读写时直接通过逻辑块好找到对应的物理块
    u32 *pb_mapped_flag;   // 物理块映射状态，用于写操作时找到空闲物理块
};

struct rw_ops_para {
    u64 rw_size;
    u64 start_offset;
    u64 data_len;
    u32 remainder;
    u8 *data_buf;
    u8 *tmp_buf;
};

struct blk_logic_info {
    struct list_head map_list;
    struct mutex map_list_mutex;
};

struct blk_logic_info g_blk_logic_info;
int blk_logic_write_multi_blk(struct blk_op_handler *bh, loff_t from, void *data_buf, u32 data_len)
{
    u32 write_len = 0;

    while (write_len < data_len) {
        if (blk_erase(bh, from + write_len)) {
            bsp_err("<%s> blk_erase err, from:0x%llx\n", __func__, from + write_len);
            return -1;
        }
        if (blk_write(bh, from + write_len, data_buf + write_len, bh->block_size)) {
            bsp_err("<%s> blk_write err, from:0x%llx, size:%d\n", __func__, from + write_len, bh->block_size);
            return -1;
        }
        write_len += bh->block_size;
    }
    return 0;
}

int blk_logic_read_multi_blk(struct blk_op_handler *bh, loff_t from, void *data_buf, u32 data_len)
{
    u32 size, left_len;
    u32 remainder = 0;
    u32 read_len = 0;
    div_u64_rem(from, bh->block_size, &remainder);

    while (read_len < data_len) {
        left_len = data_len - read_len;
        size = remainder + left_len < bh->block_size ? left_len : bh->block_size - remainder;
        if (blk_read(bh, from + read_len, data_buf + read_len, size)) {
            bsp_err("<%s> blk_read err, from:0x%llx, size:%d\n", __func__, from + read_len, size);
            return -1;
        }
        read_len += size;
    }
    return 0;
}

/* 此函数用于提升可靠性，正常分区中所有块的状态都在内部维护不需要再进一步检查，除非有其他用户不通过logic
 * block层操作此分区，导致数据被破坏 */
int blk_logic_check_valid(struct blk_op_handler *bh, struct map_info *map_info, u32 pb_index, u32 lb_index)
{
    struct pb_header pbh;
    if (blk_isbad(bh, pb_index * map_info->phy_blksize)) {
        bsp_err("<%s> blk_isbad, pb_index:%d\n", __func__, pb_index);
        return -1;
    }
    if (blk_logic_read_multi_blk(bh, pb_index * map_info->phy_blksize, &pbh, sizeof(struct pb_header))) {
        bsp_err("<%s> read pb_header err, pb_index:%d\n", __func__, pb_index);
        return -1;
    }

    if (pbh.magic != PB_HEADER_MAGIC || pbh.lb_index != lb_index) {
        bsp_err("<%s> pb_header invalid, magic:0x%x, lb_index:%d, expect lb_index:%d\n", __func__, pbh.magic,
            pbh.lb_index, lb_index);
        return -1;
    }
    return 0;
}

int blk_logic_read_ops(struct blk_op_handler *bh, struct map_info *map_info, struct rw_ops_para *p)
{
    u32 lb_index, pb_index;

    lb_index = div_u64(p->start_offset, map_info->logic_blksize);

    if (map_info->lb_map[lb_index].mapped_flag != BLK_FLAG_MAPPED) {
        bsp_err("<%s> lb:%d is not mapped, cannot be read\n", __func__, lb_index);
        return BLK_LOGIC_NO_VALID_DATA;
    }

    pb_index = map_info->lb_map[lb_index].pb_index;
    if (blk_logic_check_valid(bh, map_info, pb_index, lb_index)) {
        bsp_err("<%s> check valid before read fail\n", __func__);
        return BLK_LOGIC_CHECK_VALID_FAIL;
    }
    if (blk_logic_read_multi_blk(bh, pb_index * map_info->phy_blksize + PB_HEADER_LENGTH + p->remainder, p->data_buf,
        p->rw_size)) {
        bsp_err("<%s> read err, pb_index:%d, size:%lld\n", __func__, pb_index, p->rw_size);
        return -1;
    }
    return 0;
}

static void blk_logic_init_pb_header(struct map_info *map_info, struct pb_header *pbh)
{
    pbh->magic = PB_HEADER_MAGIC;
    pbh->version = 0;
    map_info->latest_sqnum++;
    pbh->sqnum = map_info->latest_sqnum;
}

int blk_logic_find_new_pb(struct blk_op_handler *bh, struct map_info *map_info, u32 lb_index)
{
    u32 cur_pb_index, old_pb_index;
    for (cur_pb_index = map_info->latest_pb_index + 1; cur_pb_index != map_info->latest_pb_index; cur_pb_index++) {
        if (cur_pb_index >= map_info->pb_cnt) {
            cur_pb_index = 0;
        }
        if (map_info->pb_mapped_flag[cur_pb_index] == BLK_FLAG_UNMAPPED) {
            return cur_pb_index;
        }
    }
    /* 没有好的可用块了，并且这个块也没有对应的老块，无法原地刷新 */
    if (map_info->lb_map[lb_index].mapped_flag == BLK_FLAG_UNMAPPED) {
        bsp_err("<%s> lb_index:%d is unmapped, and cannot find available pb\n", __func__, lb_index);
        return -1;
    }
    old_pb_index = map_info->lb_map[lb_index].pb_index;
    bsp_info("<%s> could not found empty pb, return old_pb:%d\n", __func__, old_pb_index);
    return old_pb_index;
}

int blk_logic_read_old_pb(struct blk_op_handler *bh, struct map_info *map_info, u32 lb_index, struct rw_ops_para *p)
{
    u32 pb_index = map_info->lb_map[lb_index].pb_index;
    if (blk_logic_check_valid(bh, map_info, pb_index, lb_index)) {
        bsp_err("<%s> check valid before read old fail\n", __func__);
        return -1;
    }
    if (blk_logic_read_multi_blk(bh, pb_index * map_info->phy_blksize, p->tmp_buf, map_info->phy_blksize)) {
        bsp_err("<%s> read err, pb_index:%d, size:%d\n", __func__, pb_index, map_info->phy_blksize);
        return -1;
    }
    return 0;
}

int blk_logic_write_prepare_buf(struct blk_op_handler *bh, struct map_info *map_info, u32 lb_index,
    struct rw_ops_para *p)
{
    int ret;
    struct pb_header pbh;

    if (p->rw_size != map_info->logic_blksize) {
        /* 如果没有老块或者读老块失败就直接重新写一块数据 */
        if ((map_info->lb_map[lb_index].mapped_flag != BLK_FLAG_MAPPED) ||
            (blk_logic_read_old_pb(bh, map_info, lb_index, p))) {
            (void)memset_s(p->tmp_buf, map_info->phy_blksize, 0xff, map_info->phy_blksize);
        }
        ret = memcpy_s(p->tmp_buf + PB_HEADER_LENGTH + p->remainder,
            map_info->phy_blksize - PB_HEADER_LENGTH - p->remainder, p->data_buf, p->rw_size);
        if (ret) {
            bsp_err("<%s> memcpy_s err, ret:%d, size:%lld\n", __func__, ret, p->rw_size);
            return -1;
        }
    } else {
        ret =
            memcpy_s(p->tmp_buf + PB_HEADER_LENGTH, map_info->phy_blksize - PB_HEADER_LENGTH, p->data_buf, p->rw_size);
        if (ret) {
            bsp_err("<%s> memcpy_s err, ret:%d, size:%lld\n", __func__, ret, p->rw_size);
            return -1;
        }
    }

    blk_logic_init_pb_header(map_info, &pbh);
    pbh.lb_index = lb_index;
    ret = memcpy_s(p->tmp_buf, map_info->phy_blksize, &pbh, sizeof(pbh));
    if (ret) {
        bsp_err("<%s> memcpy_s pbh err, ret:%d\n", __func__, ret);
        return -1;
    }
    ret = memcpy_s(p->tmp_buf + map_info->phy_blksize - PB_HEADER_LENGTH, PB_HEADER_LENGTH, &pbh, sizeof(pbh));
    if (ret) {
        bsp_err("<%s> memcpy_s pbh_bk err, ret:%d\n", __func__, ret);
        return -1;
    }
    return 0;
}

int blk_logic_write_ops(struct blk_op_handler *bh, struct map_info *map_info, struct rw_ops_para *p)
{
    u32 lb_index, old_pb_index;
    int new_pb_index;

    lb_index = div_u64(p->start_offset, map_info->logic_blksize);

    if (blk_logic_write_prepare_buf(bh, map_info, lb_index, p)) {
        return BLK_LOGIC_WRITE_PREPARE_ERR;
    }

    while (1) {
        new_pb_index = blk_logic_find_new_pb(bh, map_info, lb_index);
        if (new_pb_index < 0) {
            return BLK_LOGIC_FIND_NEW_PB_ERR;
        }
        /* 映射表已经管理了坏块情况，不需要再查坏块，不需要考虑被其他手段标记坏块的情况 */
        if (blk_logic_write_multi_blk(bh, new_pb_index * map_info->phy_blksize, p->tmp_buf, map_info->phy_blksize)) {
            map_info->pb_mapped_flag[new_pb_index] = BLK_FLAG_BAD;
            bsp_err("<%s> write err, pb_index:%d, size:%d\n", __func__, new_pb_index, map_info->phy_blksize);
        } else {
            break;
        }
    }
    if (map_info->lb_map[lb_index].mapped_flag == BLK_FLAG_MAPPED) {
        old_pb_index = map_info->lb_map[lb_index].pb_index;
        map_info->pb_mapped_flag[old_pb_index] = BLK_FLAG_UNMAPPED;
    }
    map_info->lb_map[lb_index].pb_index = new_pb_index;
    map_info->lb_map[lb_index].mapped_flag = BLK_FLAG_MAPPED;
    map_info->pb_mapped_flag[new_pb_index] = BLK_FLAG_MAPPED;
    map_info->latest_pb_index = new_pb_index;

    return 0;
}

int blk_logic_read_write_ops(struct blk_op_handler *bh, struct map_info *map_info, struct rw_ops_para *p, u32 ops_type)
{
    int ret;
    int err = 0;
    while (p->data_len > 0) {
        div_u64_rem(p->start_offset, map_info->logic_blksize, &p->remainder);
        p->rw_size = (p->remainder + p->data_len < map_info->logic_blksize) ? p->data_len :
                                                                              (map_info->logic_blksize - p->remainder);

        mutex_lock(&map_info->part_mutex);
        if (ops_type == BLK_WRITE) {
            ret = blk_logic_write_ops(bh, map_info, p);
        } else {
            ret = blk_logic_read_ops(bh, map_info, p);
        }
        mutex_unlock(&map_info->part_mutex);

        if (ret == BLK_LOGIC_NO_VALID_DATA) {
            err = BLK_LOGIC_NO_VALID_DATA;
        }
        if (ret && ret != BLK_LOGIC_NO_VALID_DATA) {
            bsp_err("<%s> logic read write failed, ops %d\n", __func__, ops_type);
            return ret;
        }
        p->start_offset += p->rw_size;
        p->data_len = (p->data_len >= p->rw_size) ? p->data_len - p->rw_size : 0;
        p->data_buf += p->rw_size;
    }
    return err;
}

int blk_logic_set_map_info(struct blk_op_handler *bh, struct map_info *map_info)
{
    u32 remainder = 0;
    int ret;

    if (bsp_boot_mode_is_emmc()) {
        map_info->phy_blksize = EMMC_LOGIC_HANDLE_BLK * bh->block_size;
        map_info->pb_cnt = div_u64(bh->part_size, map_info->phy_blksize);
        map_info->logic_blksize = map_info->phy_blksize - 2U * PB_HEADER_LENGTH;
        map_info->lb_cnt = map_info->pb_cnt - 1;
        map_info->logic_partsize = map_info->lb_cnt * map_info->logic_blksize;
    } else {
        map_info->phy_blksize = bh->block_size;
        map_info->pb_cnt = div_u64(bh->part_size, map_info->phy_blksize);
        map_info->logic_blksize = map_info->phy_blksize - 2U * PB_HEADER_LENGTH;
        ret = bsp_partition_get_partition_logic_len(bh->part_name);
        if (ret <= 0) {
            bsp_err("<%s> part %s logic size err\n", __func__, bh->part_name);
            return -1;
        }
        map_info->logic_partsize = (u32)ret;
        map_info->lb_cnt = div_u64_rem(map_info->logic_partsize, map_info->logic_blksize, &remainder);
        if (remainder) {
            map_info->lb_cnt++;
        }
        if (map_info->lb_cnt > map_info->pb_cnt) {
            bsp_err("<%s> part lb_cnt(%d) > pb_cnt(%d)\n", __func__, map_info->lb_cnt, map_info->pb_cnt);
            return -1;
        }
    }
    return 0;
}

struct map_info *blk_logic_alloc_map(struct blk_op_handler *bh)
{
    struct map_info *map_info = NULL;

    map_info = kzalloc(sizeof(struct map_info), GFP_KERNEL);
    if (map_info == NULL) {
        bsp_err("<%s> kzalloc failed\n", __func__);
        return NULL;
    }
    map_info->part_name = bh->part_name;
    if (blk_logic_set_map_info(bh, map_info)) {
        goto err;
    }
    map_info->lb_map = kzalloc(sizeof(struct lb_map) * map_info->lb_cnt, GFP_KERNEL);
    if (map_info->lb_map == NULL) {
        bsp_err("<%s> kzalloc failed, lb_cnt:%d\n", __func__, map_info->lb_cnt);
        goto err;
    }
    map_info->pb_mapped_flag = kzalloc(sizeof(u32) * map_info->pb_cnt, GFP_KERNEL);
    if (map_info->pb_mapped_flag == NULL) {
        bsp_err("<%s> kzalloc failed, pb_cnt:%d\n", __func__, map_info->pb_cnt);
        kfree(map_info->lb_map);
        goto err;
    }
    map_info->latest_pb_index = 0;
    mutex_init(&map_info->part_mutex);

    list_add(&map_info->list, &g_blk_logic_info.map_list);

    return map_info;
err:
    kfree(map_info);
    return NULL;
}

void blk_logic_free_map(struct map_info *map_info)
{
    list_del(&map_info->list);
    kfree(map_info->pb_mapped_flag);
    kfree(map_info->lb_map);
    kfree(map_info);
}

int blk_logic_scan(struct blk_op_handler *bh, struct map_info *map_info, struct pb_header *pbh,
    struct pb_header *pbh_bk)
{
    int ret;
    u32 cur_pb_index;

    for (cur_pb_index = 0; cur_pb_index < map_info->pb_cnt; cur_pb_index++) {
        ret = blk_isbad(bh, cur_pb_index * map_info->phy_blksize);
        if (ret) {
            bsp_err("<%s> blk isbad, ret:%d, pb_index:%d\n", __func__, ret, cur_pb_index);
            map_info->pb_mapped_flag[cur_pb_index] = BLK_FLAG_BAD;
            continue;
        }
        if (blk_logic_read_multi_blk(bh, cur_pb_index * map_info->phy_blksize, pbh + cur_pb_index,
            sizeof(struct pb_header))) {
            bsp_err("<%s> read pb_header err, cur_pb_index:%d\n", __func__, cur_pb_index);
            pbh[cur_pb_index].magic = PB_HEADER_MAGIC_DIFF;
            continue;
        }
        if (blk_logic_read_multi_blk(bh, (cur_pb_index + 1) * map_info->phy_blksize - PB_HEADER_LENGTH, pbh_bk,
            sizeof(struct pb_header))) {
            bsp_err("<%s> read pb_header_bk err, cur_pb_index:%d\n", __func__, cur_pb_index);
            pbh[cur_pb_index].magic = PB_HEADER_MAGIC_DIFF;
            continue;
        }
        if (memcmp(pbh + cur_pb_index, pbh_bk, sizeof(struct pb_header))) {
            bsp_err("<%s> pb_header diff, cur_pb_index:%d\n", __func__, cur_pb_index);
            pbh[cur_pb_index].magic = PB_HEADER_MAGIC_DIFF;
            continue;
        }
    }
    return 0;
}

void blk_logic_fill_map(struct map_info *map_info, struct pb_header *pbh)
{
    u32 cur_pb_index, cur_lb_index, pb_index_max;
    u32 found_flag;
    u64 sqnum_max;

    for (cur_lb_index = 0; cur_lb_index < map_info->lb_cnt; cur_lb_index++) {
        sqnum_max = 0;
        pb_index_max = 0;
        found_flag = 0;
        for (cur_pb_index = 0; cur_pb_index < map_info->pb_cnt; cur_pb_index++) {
            if (map_info->pb_mapped_flag[cur_pb_index] == BLK_FLAG_BAD) {
                continue;
            }
            if (pbh[cur_pb_index].lb_index != cur_lb_index) {
                continue;
            }
            if (pbh[cur_pb_index].magic != PB_HEADER_MAGIC) {
                continue;
            }
            found_flag = 1;
            if (pbh[cur_pb_index].sqnum > sqnum_max) {
                sqnum_max = pbh[cur_pb_index].sqnum;
                pb_index_max = cur_pb_index;
            }
        }
        if (!found_flag) {
            continue;
        }
        map_info->lb_map[cur_lb_index].pb_index = pb_index_max;
        map_info->lb_map[cur_lb_index].mapped_flag = BLK_FLAG_MAPPED;
        map_info->pb_mapped_flag[pb_index_max] = BLK_FLAG_MAPPED;
        bsp_info("<%s> lb_index:%d, pb_index:%d\n", __func__, cur_lb_index, pb_index_max);
        if (sqnum_max > map_info->latest_sqnum) {
            map_info->latest_sqnum = sqnum_max;
            map_info->latest_pb_index = pb_index_max;
        }
    }
    return;
}

int blk_logic_init_map(struct map_info *map_info, struct blk_op_handler *bh)
{
    struct pb_header *pbh = NULL;
    struct pb_header *pbh_bk = NULL;

    pbh = kzalloc(sizeof(struct pb_header) * (map_info->pb_cnt + 1), GFP_KERNEL);
    if (pbh == NULL) {
        bsp_err("<%s> kzalloc failed, pb_cnt:%d\n", __func__, map_info->pb_cnt);
        return -1;
    }
    pbh_bk = &pbh[map_info->pb_cnt];

    if (blk_logic_scan(bh, map_info, pbh, pbh_bk)) {
        kfree(pbh);
        return -1;
    }
    blk_logic_fill_map(map_info, pbh);
    kfree(pbh);
    return 0;
}

struct map_info *blk_logic_get_map_info(struct blk_op_handler *bh)
{
    struct list_head *p = NULL;
    struct list_head *n = NULL;
    struct map_info *map_info = NULL;

    mutex_lock(&g_blk_logic_info.map_list_mutex);

    list_for_each_safe(p, n, &g_blk_logic_info.map_list)
    {
        map_info = list_entry(p, struct map_info, list);
        if (!strcmp(map_info->part_name, bh->part_name)) {
            goto exit;
        }
    }
    map_info = blk_logic_alloc_map(bh);
    if (map_info == NULL) {
        goto err;
    }

    if (blk_logic_init_map(map_info, bh)) {
        blk_logic_free_map(map_info);
        goto err;
    }
exit:
    mutex_unlock(&g_blk_logic_info.map_list_mutex);

    return map_info;
err:
    mutex_unlock(&g_blk_logic_info.map_list_mutex);
    return NULL;
}

int blk_logic_read_write(const char *part_name, struct rw_ops_para *p, u32 ops_type)
{
    int ret;
    struct map_info *map_info = NULL;
    struct blk_op_handler *bh = NULL;

    bh = blk_open(part_name);
    if (bh == NULL) {
        bsp_err("<%s> open err\n", __func__);
        return BLK_LOGIG_OPEN_PART_ERR;
    }
    map_info = blk_logic_get_map_info(bh);
    if (map_info == NULL) {
        blk_close(bh);
        return BLK_LOGIC_GET_MAP_ERR;
    }

    if (p->data_buf == NULL || p->start_offset > map_info->logic_partsize || p->data_len > map_info->logic_partsize ||
        p->start_offset + p->data_len > map_info->logic_partsize) {
        bsp_err("<%s> para err, start:0x%llx, data_len:0x%llx, logic_partsize:0x%llx, n", __func__, p->start_offset,
            p->data_len, map_info->logic_partsize);
        blk_close(bh);
        return BLK_LOGIC_PARAM_ERR;
    }

    if (ops_type == BLK_WRITE) {
        p->tmp_buf = (unsigned char *)vmalloc(map_info->phy_blksize);
        if (p->tmp_buf == NULL) {
            bsp_err("<%s> vmalloc failed\n", __func__);
            blk_close(bh);
            return BLK_LOGIC_ALLOC_ERR;
        }
    }

    ret = blk_logic_read_write_ops(bh, map_info, p, ops_type);

    if (ops_type == BLK_WRITE) {
        vfree(p->tmp_buf);
    }
    blk_close(bh);
    return ret;
}

int bsp_blk_logic_write(const char *part_name, loff_t part_offset, const void *data_buf, size_t data_len)
{
    int ret;
    struct rw_ops_para ops_para = { 0 };

    ops_para.start_offset = part_offset;
    ops_para.data_len = data_len;
    ops_para.data_buf = (void *)data_buf;
    ret = blk_logic_read_write(part_name, &ops_para, BLK_WRITE);
    return ret;
}

int mdrv_blk_logic_write(const char *part_name, unsigned long long part_offset, void *data_buf, unsigned long long data_len)
{
    return bsp_blk_logic_write(part_name, part_offset, data_buf, data_len);
}

int bsp_blk_logic_read(const char *part_name, loff_t part_offset, void *data_buf, size_t data_len)
{
    int ret;
    struct rw_ops_para ops_para = { 0 };

    ops_para.start_offset = part_offset;
    ops_para.data_len = data_len;
    ops_para.data_buf = (void *)data_buf;

    ret = blk_logic_read_write(part_name, &ops_para, BLK_READ);
    return ret;
}

int mdrv_blk_logic_read(const char *part_name, unsigned long long part_offset, void *data_buf, unsigned long long data_len)
{
    return bsp_blk_logic_read(part_name, part_offset, data_buf, data_len);
}

int blk_logic_init(void)
{
    INIT_LIST_HEAD(&g_blk_logic_info.map_list);
    mutex_init(&g_blk_logic_info.map_list_mutex);
    return 0;
}

void blk_logic_map_clear(void)
{
    struct list_head *p = NULL;
    struct list_head *n = NULL;
    struct map_info *map_info = NULL;
    list_for_each_safe(p, n, &g_blk_logic_info.map_list)
    {
        map_info = list_entry(p, struct map_info, list);
        blk_logic_free_map(map_info);
    }
}
fs_initcall(blk_logic_init);

EXPORT_SYMBOL_GPL(mdrv_blk_logic_read);
EXPORT_SYMBOL_GPL(mdrv_blk_logic_write);
EXPORT_SYMBOL_GPL(bsp_blk_logic_write);
EXPORT_SYMBOL_GPL(bsp_blk_logic_read);
EXPORT_SYMBOL_GPL(blk_logic_map_clear);
