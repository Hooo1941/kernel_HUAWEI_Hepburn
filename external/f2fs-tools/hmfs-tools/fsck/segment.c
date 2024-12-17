/**
 * segment.c
 *
 * Many parts of codes are copied from Linux kernel/fs/f2fs.
 *
 * Copyright (C) 2015 Huawei Ltd.
 * Witten by:
 *   Hou Pengyang <houpengyang@huawei.com>
 *   Liu Shuoran <liushuoran@huawei.com>
 *   Jaegeuk Kim <jaegeuk@kernel.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <limits.h>
#include "fsck.h"
#include "node.h"
#include "quotaio_cache.h"

const int curseg_dm_type[NR_CURSEG_DM_TYPE] = {CURSEG_HOT_DATA, CURSEG_COLD_DATA};

static void set_prefree(struct f2fs_sb_info *sbi, unsigned int segno)
{
	struct free_segmap_info *free_i = FREE_I(sbi);
	unsigned int secno = GET_SEC_FROM_SEG(sbi, segno);
	unsigned int start_segno = GET_SEG_FROM_SEC(sbi, secno);
	unsigned int next;
	struct seg_entry *se = get_seg_entry(sbi, segno);

	if (se->valid_blocks || IS_CURSEG(sbi, segno))
		return;

	if (test_and_clear_bit_le(segno, free_i->free_segmap))
		free_i->free_segments++;

	if (IS_CURSEC(sbi, secno)) {
		return;
	}
	next = find_next_bit_le(free_i->free_segmap,
			start_segno + sbi->segs_per_sec, start_segno);
	if (next >= start_segno + sbi->segs_per_sec) {
		set_bit_le(secno, free_i->prefree_secmap);
	}
}

void set_prefree_as_free_section(struct f2fs_sb_info *sbi)
{
	unsigned int secno;
	struct free_segmap_info *free_i = FREE_I(sbi);

	for (secno = 0; secno < MAIN_SECS(sbi); secno++) {
		if (test_bit_le(secno, free_i->prefree_secmap)) {
			clear_bit_le(secno, free_i->free_secmap);
			free_i->free_sections++;
			clear_bit_le(secno, free_i->prefree_secmap);
		}
	}
}

int set_block_free(struct f2fs_sb_info *sbi, u64 blk_addr)
{
	struct seg_entry *se = NULL;
	u64 offset;
	int type;
	unsigned int segno;

	/* update sit bitmap & valid_blocks && se->type */
	segno = GET_SEGNO(sbi, blk_addr);
	se = get_seg_entry(sbi, segno);
	offset = OFFSET_IN_SEG(sbi, blk_addr);
	type = se->type;
	se->valid_blocks--;
	f2fs_clear_bit(offset, (char *)se->cur_valid_map);
	se->dirty = 1;

	set_prefree(sbi, segno);
	return type;
}

bool has_not_enough_free_secs(struct f2fs_sb_info *sbi)
{
	struct free_segmap_info *free_i = FREE_I(sbi);

	return (free_i->free_sections <= min_fsck_free_section(sbi));
}

static bool check_ssa_valid(struct f2fs_sb_info *sbi, unsigned int segno,
		struct f2fs_summary_block *sum_blk, struct seg_entry *se)
{
	u64 ssa_blk;
	struct f2fs_summary *sum_entry = NULL;
	int ret;
	int i;
	block_t blk_addr;
	struct node_info ni;

	ssa_blk = GET_SUM_BLKADDR(sbi, segno);
	ret = dev_read_block(sum_blk, ssa_blk);
	ASSERT(ret >= 0);

	if (IS_SUM_NODE_SEG(sum_blk->footer) != IS_NODESEG(se->type) &&
	    IS_SUM_DATA_SEG(sum_blk->footer) != IS_DATASEG(se->type)) {
		return false;
	}

	for (i = 0; i < sbi->blocks_per_seg; i++) {
		if (!f2fs_test_bit(i, (const char *)se->cur_valid_map))
			continue;

		blk_addr = START_BLOCK(sbi, segno) + i;
		sum_entry = &(sum_blk->entries[i]);

		if (IS_NODESEG(se->type)) {
			if (le32_to_cpu(sum_entry->nid) >=
					F2FS_FSCK(sbi)->nr_nat_entries) {
				return false;
			}
			get_node_info(sbi, le32_to_cpu(sum_entry->nid), &ni);
			if (le32_to_cpu(sum_entry->nid) != ni.nid)
				return false;
		} else {
			if (is_valid_summary(sbi, sum_entry, blk_addr) != 1) {
				return false;
			}
		}
	}

	return true;
}

static int min_cost_cmp(const void *a, const void *b)
{
	return ((struct gc_victim_cost*)a)->cost -
		((struct gc_victim_cost*)b)->cost;
}

static unsigned int get_victim(struct f2fs_sb_info *sbi)
{
	unsigned int secno, segno, i;
	struct free_segmap_info *free_i = FREE_I(sbi);
	unsigned int start_segno;
	unsigned int end_segno;
	unsigned int victim_secno = (unsigned int)~0;
	struct seg_entry *se = NULL;
	int cost;
	struct f2fs_summary_block *sum_blk = NULL;
	struct gc_victim_cost *victim_cost = NULL;

	sum_blk = calloc(BLOCK_SZ, 1);
	ASSERT(sum_blk);
	victim_cost = calloc(sizeof(struct gc_victim_cost), MAIN_SECS(sbi));
	if (victim_cost == NULL) {
		ERR_MSG("Errno:victim_cost calloc failed\n");
		free(sum_blk);
		exit(-1);
	}
	for (secno = 0; secno < MAIN_SECS(sbi); secno++) {
		victim_cost[secno].secno = secno;
		victim_cost[secno].cost = INT_MAX;

		if (IS_CUR_SECNO(sbi, secno)) {
			continue;
		}

		if (!test_bit_le(secno, free_i->free_secmap) ||
			test_bit_le(secno, free_i->free_dm_secmap)) {
			continue;
		}

		start_segno = GET_SEG_FROM_SEC(sbi, secno);
		end_segno = GET_SEG_FROM_SEC(sbi, secno + 1);

		cost = 0;
		for (i = start_segno; i < end_segno; i++) {
			se = get_seg_entry(sbi, i);
			cost += se->valid_blocks;
		}
		victim_cost[secno].cost = cost;
	}

	qsort(victim_cost, MAIN_SECS(sbi),
			sizeof(struct gc_victim_cost), min_cost_cmp);

	for (i = 0; i < MAIN_SECS(sbi); i++) {
		secno = victim_cost[i].secno;
		if (IS_CUR_SECNO(sbi, secno)) {
			continue;
		}

		if (!test_bit_le(secno, free_i->free_secmap) ||
				test_bit_le(secno, free_i->free_dm_secmap)) {
			continue;
		}
		start_segno = GET_SEG_FROM_SEC(sbi, secno);
		end_segno = GET_SEG_FROM_SEC(sbi, secno + 1);
		for (segno = start_segno; segno < end_segno; segno++) {
			se = get_seg_entry(sbi, segno);
			if (se->valid_blocks == 0)
				continue;
			if (check_ssa_valid(sbi, segno, sum_blk, se) == false) {
				MSG(0, "Info: gc victim segment:%x is not valid\n", segno);
				break;
			}
		}
		if (segno == end_segno) {
			victim_secno = secno;
			break;
		}
	}
	free(sum_blk);
	free(victim_cost);

	return victim_secno;
}

void fsck_gc(struct f2fs_sb_info *sbi)
{
	struct seg_entry *se = NULL;
	block_t from;
	unsigned int i;
	int j, ret;
	unsigned int start_segno;
	unsigned int end_segno;
	unsigned int secno = get_victim(sbi);

	struct free_segmap_info *free_i = FREE_I(sbi);
	MSG(0, "Info: before gc, it has %d free segments/%d free sections.\n",
			free_i->free_segments, free_i->free_sections);

	if (secno < 0 || secno >= MAIN_SECS(sbi)) {
		return;
	}
	start_segno = GET_SEG_FROM_SEC(sbi, secno);
	end_segno = GET_SEG_FROM_SEC(sbi, secno + 1);

	void *raw = calloc(BLOCK_SZ, 1);
	ASSERT(raw);
	c.do_gc = 1;

	for (i = start_segno; i < end_segno; i++) {
		se = get_seg_entry(sbi, i);
		if (!se->valid_blocks)
			continue;

		for (j = 0; j < sbi->blocks_per_seg; j++) {
			if (!f2fs_test_bit(j, (const char *)se->cur_valid_map))
				continue;

			from = START_BLOCK(sbi, i) + j;
			ret = dev_read_block(raw, from);
			ASSERT(ret >= 0);

			ret = f2fs_write_block(sbi, raw, from, 0);
			ASSERT(ret >= 0);
		}
	}
	free(raw);

	set_write_stream_id(META_STREAM_ID, sbi->sm_info->seg0_blkaddr);
	write_curseg_info(sbi);
	flush_sit_entries(sbi);
	flush_nat_entries(sbi);
	write_checkpoint(sbi);
	set_prefree_as_free_section(sbi);
	c.do_gc = 0;
	MSG(0, "Info: after gc, it has %d free segments/%d free sections.\n",
			free_i->free_segments, free_i->free_sections);

}

void write_inode(struct f2fs_sb_info *sbi, u64 blkaddr, struct f2fs_node *inode)
{
	if (c.feature & cpu_to_le32(F2FS_FEATURE_INODE_CHKSUM))
		inode->i.i_inode_checksum =
			cpu_to_le32(f2fs_inode_chksum(inode));
	if (c.func == RESIZE) {
		ASSERT(dev_write_block(inode, blkaddr) >= 0);
	} else {
		ASSERT(f2fs_write_block(sbi, inode, blkaddr, 0) >= 0);
	}
}

static void sync_flash_mode(struct f2fs_sb_info *sbi,
		struct stor_pwron_info *stor_pwron_info)
{
	int type;
	unsigned int i;
	unsigned int mode;

	for (i = 0; i < NR_CURSEG_TYPE; i++) {
		sbi->flash_mode[i] = TLC_MODE;
	}

	for (i = COLD_NODE_STREAM_ID; i < FS_STREAM_NR; i++) {
		type = CURSEG_T(i);
		if ((stor_pwron_info->io_slc_mode_status & (1 << i))) {
			mode = SLC_MODE;
		} else {
			mode = TLC_MODE;
		}

		sbi->flash_mode[type] = mode;
	}
	MSG(0, "Info: sync flash mode with device:");
	for (i = CURSEG_HOT_DATA; i < NO_CHECK_TYPE; i++) {
		MSG(0, "%d", sbi->flash_mode[i]);
		if (i + 1 < NO_CHECK_TYPE)
			MSG(0, "|");
	}
	MSG(0, "\n");
}

static void power_on_addr_check(struct f2fs_sb_info *sbi, u64 pos)
{
	unsigned int segno;

	if (pos < SM_I(sbi)->main_blkaddr) {
		return;
	}

	segno = GET_SEGNO(sbi, pos);
	if (segno >= TOTAL_SEGS(sbi)) {
		return;
	}

	if (!IS_FIRST_DATA_BLOCK_IN_SEC(sbi, pos)) {
		__set_test_and_inuse(sbi, segno);
	}
}

void sync_curseg_device_info(struct f2fs_sb_info *sbi)
{
	int err;
	int i;
	struct stor_pwron_info stor_pwron_info;
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
	unsigned int secno;
	unsigned int segno;
	unsigned int old_segno;
	unsigned int next_blkoff;
	u64 pos;
	struct curseg_info *curseg;
	int type;

	err = blk_device_curseg_info_sync(&stor_pwron_info);
	if (err) {
		ERR_MSG("Errno:%d get curseg info from device failed\n", err);
		return;
	}

	sync_flash_mode(sbi, &stor_pwron_info);

	/*
	 * Read the position of the device in advance and set it to
	 * be used to prevent the section from being allocated when
	 * a new section is needed.
	 */
	for (i = COLD_NODE_STREAM_ID; i < FS_STREAM_NR; i++) {
		pos = stor_pwron_info.dev_stream_addr[i];
		power_on_addr_check(sbi, pos);
	}
	for (i = 0; i < NR_CURSEG_DM_TYPE; i++) {
		pos = stor_pwron_info.dm_stream_addr[i];
		power_on_addr_check(sbi, pos);
	}

	for (i = COLD_NODE_STREAM_ID; i < FS_STREAM_NR; i++) {
		pos = stor_pwron_info.dev_stream_addr[i];

		/* stream has never been written */
		if (pos == 0)
			continue;

		err = clear_section_info(pos, i);
		if (err) {
			ERR_MSG("Errno:%d clear curseg info[%d] from device failed\n",
				err, i);
			ASSERT(0);
		}

		type = CURSEG_T(i);
		curseg = CURSEG_I(sbi, type);
		segno = GET_SEGNO(sbi, pos);
		next_blkoff = OFFSET_IN_SEG(sbi, pos);
		old_segno = curseg->segno;

		if (IS_FIRST_DATA_BLOCK_IN_SEC(sbi, pos)) {
			MSG(0, "sync info: stream[%d]'s section[%u,%u,%u] "
					"has been all written, need a new one\n",
			i, curseg->segno, curseg->next_blkoff, pos);

			get_new_curseg(sbi, 0, curseg, type, 1);
			fsck->chk.sit_free_segs--;
			set_cp(free_segment_count, get_cp(free_segment_count) - 1);
			c.need_write_cp = 1;
			set_prefree(sbi, old_segno);
		} else if (segno != curseg->segno ||
					next_blkoff != curseg->next_blkoff) {
			MSG(0, "sync info:stream[%d] is't same[%u,%u]->[%u,%u]\n",
					i, curseg->segno, curseg->next_blkoff,
							segno, next_blkoff);

			if (segno != curseg->segno) {
				err = f2fs_write_block(sbi, curseg->sum_blk,
					GET_SUM_BLKADDR(sbi, curseg->segno), 1);
				ASSERT(err >= 0);
				curseg->segno = segno;
				curseg->zone =
					GET_ZONE_FROM_SEG(sbi, curseg->segno);

				__set_test_and_inuse(sbi, segno);
				reset_curseg(sbi, type);
				fsck->chk.sit_free_segs--;
				set_cp(free_segment_count, get_cp(free_segment_count) - 1);
				set_prefree(sbi, old_segno);
			}

			curseg->next_blkoff = next_blkoff;

			if (type < CURSEG_HOT_NODE) {
				set_cp(cur_data_segno[type], curseg->segno);
				set_cp(cur_data_blkoff[type], curseg->next_blkoff);
			} else {
				int n = type - CURSEG_HOT_NODE;

				set_cp(cur_node_segno[n], curseg->segno);
				set_cp(cur_node_blkoff[n], curseg->next_blkoff);
			}
			c.need_write_cp = 1;
		}

		/*
		 * if pos is the first section block, we need not call
		 * set_section_info again. Because it has been called
		 * in get_new_curseg.
		 */
		if (!IS_FIRST_DATA_BLOCK_IN_SEC(sbi, pos)) {
			secno = GET_SEC_FROM_SEG(sbi, curseg->segno);
			err = set_section_info(START_BLOCK(sbi, GET_SEG_FROM_SEC(sbi, secno)),
					sbi->flash_mode[type], i);
			if (err) {
				ERR_MSG("Errno:%d set section info[%d] to device failed\n",
						err, i);
				ASSERT(0);
			}
		}
	}
	MSG(0, "Info: sync position with device(segno,next_blkoff):");
	for (i = CURSEG_HOT_DATA; i < NO_CHECK_TYPE; i++) {
		curseg = CURSEG_I(sbi, i);
		MSG(0, "(%u,%u)", curseg->segno, curseg->next_blkoff);
		if (i + 1 < NO_CHECK_TYPE)
			MSG(0, "|");
	}
	MSG(0, "\n");
}

block_t next_free_blkaddr(struct f2fs_sb_info *sbi, int type)
{
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	struct curseg_info *curseg = CURSEG_I(sbi, type);

	return SM_I(sbi)->main_blkaddr +
		(curseg->segno << get_sb(log_blocks_per_seg)) +
		curseg->next_blkoff;
}

int reserve_new_block(struct f2fs_sb_info *sbi, block_t *to,
			struct f2fs_summary *sum, int type, bool is_inode)
{
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
	struct seg_entry *se;
	u64 blkaddr, offset;
	u64 old_blkaddr = *to;
	bool is_node = IS_NODESEG(type);
	int left;

	if (old_blkaddr == NULL_ADDR) {
		if (c.func == FSCK) {
			if (fsck->chk.valid_blk_cnt >= sbi->user_block_count) {
				ERR_MSG("Not enough space");
				return -ENOSPC;
			}
			if (is_node && fsck->chk.valid_node_cnt >=
					sbi->total_valid_node_count) {
				ERR_MSG("Not enough space for node block");
				return -ENOSPC;
			}
		} else {
			if (sbi->total_valid_block_count >=
					sbi->user_block_count) {
				ERR_MSG("Not enough space");
				return -ENOSPC;
			}
			if (is_node && sbi->total_valid_node_count >=
						sbi->total_node_count) {
				ERR_MSG("Not enough space for node block");
				return -ENOSPC;
			}
		}
	}

	if (c.invertion) {
		blkaddr = ((SM_I(sbi)->main_segments - 1) <<
			sbi->log_blocks_per_seg) + SM_I(sbi)->main_blkaddr; /*lint !e647*/
		left = 1;
	} else {
		blkaddr = SM_I(sbi)->main_blkaddr;
		left = 0;
	}

	if (find_next_free_block(sbi, &blkaddr, left, type, sum)) {
		ERR_MSG("Can't find free block");
		ASSERT(0);
	}

	se = get_seg_entry(sbi, GET_SEGNO(sbi, blkaddr));
	offset = OFFSET_IN_SEG(sbi, blkaddr);
	se->type = type;
	se->orig_type = type;
	se->valid_blocks++;
	f2fs_set_bit(offset, (char *)se->cur_valid_map);
	se->dirty = 1;

	if (old_blkaddr == NULL_ADDR) {
		sbi->total_valid_block_count++;
		if (is_node) {
			sbi->total_valid_node_count++;
			if (is_inode)
				sbi->total_valid_inode_count++;
		}
		if (c.func == FSCK) {
			fsck->chk.valid_blk_cnt++;
			if (is_node) {
				fsck->chk.valid_node_cnt++;
				if (is_inode)
					fsck->chk.valid_inode_cnt++;
			}
			f2fs_set_main_bitmap(sbi, blkaddr, type);
			f2fs_set_sit_bitmap(sbi, blkaddr);
		}
	} else {
		if (c.func == FSCK) {
			if (f2fs_test_main_bitmap(sbi, old_blkaddr) == 1) {
				f2fs_set_main_bitmap(sbi, blkaddr, type);
			}
			f2fs_set_sit_bitmap(sbi, blkaddr);
			f2fs_clear_main_bitmap(sbi, old_blkaddr);
			f2fs_clear_sit_bitmap(sbi, old_blkaddr);
		}
	}

	/* read/write SSA */
	*to = (block_t)blkaddr;

	return 0;
}

int f2fs_write_data_block(struct f2fs_sb_info *sbi, void *block,
		block_t blk_addr, struct dnode_of_data *dn, int type)
{
	int ret;
	struct f2fs_summary sum;
	struct node_info ni;
	unsigned int blkaddr = datablock_addr(dn->node_blk, dn->ofs_in_node);
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);
	block_t startaddr, endaddr;

	if (blk_addr == NULL_ADDR || blk_addr == NEW_ADDR) {

		if (!is_set_ckpt_flags(cp, CP_UMOUNT_FLAG)) {
			c.alloc_failed = 1;
			return -EINVAL;
		}

		get_node_info(sbi, dn->nid, &ni);
		set_summary(&sum, dn->nid, dn->ofs_in_node, ni.version);
		ret = reserve_new_block(sbi, &dn->data_blkaddr, &sum, type, 0);
		if (ret) {
			c.alloc_failed = 1;
			return ret;
		}

		if (blkaddr == NULL_ADDR)
			inc_inode_blocks(dn);
		else if (blkaddr == NEW_ADDR)
			dn->idirty = 1;
		set_data_blkaddr(dn);

		set_write_stream_id(get_stream_id(type), dn->data_blkaddr);
		ret = dev_write_block(block, dn->data_blkaddr);
		ASSERT(ret >= 0);
	} else {
		type = set_block_free(sbi, blk_addr);

		/* read/write SSA */
		get_sum_entry(sbi, blk_addr, &sum);
		dn->data_blkaddr = blk_addr;
		ret = reserve_new_block(sbi, &dn->data_blkaddr, &sum, type, 0);
		if (ret) {
			c.alloc_failed = 1;
			return ret;
		}

		set_data_blkaddr(dn);
		startaddr = le32_to_cpu(dn->inode_blk->i.i_ext.blk_addr);
		endaddr = startaddr + le32_to_cpu(dn->inode_blk->i.i_ext.len);
		if (blk_addr >= startaddr && blk_addr < endaddr) {
			dn->inode_blk->i.i_ext.len = 0;
			dn->idirty = 1;
		}

		set_write_stream_id(get_stream_id(type), dn->data_blkaddr);
		ret = dev_write_block(block, dn->data_blkaddr);
		ASSERT(ret >= 0);
	}

	return ret;
}

u64 f2fs_read(struct f2fs_sb_info *sbi, nid_t ino, u8 *buffer,
					u64 count, pgoff_t offset)
{
	struct dnode_of_data dn;
	struct node_info ni;
	struct f2fs_node *inode;
	char *blk_buffer;
	u64 filesize;
	u64 off_in_blk;
	u64 len_in_blk;
	u64 read_count;
	u64 remained_blkentries;
	block_t blkaddr;
	void *index_node = NULL;

	memset(&dn, 0, sizeof(dn));

	/* Memory allocation for block buffer and inode. */
	blk_buffer = calloc(BLOCK_SZ, 2);
	ASSERT(blk_buffer);
	inode = (struct f2fs_node*)(blk_buffer + BLOCK_SZ);

	/* Read inode */
	get_node_info(sbi, ino, &ni);
	if (c.cached_ino == ino)
		hmfs_grab_node_from_cache(inode, ino, ni.blk_addr);
	else
		ASSERT(dev_read_block(inode, ni.blk_addr) >= 0);
	ASSERT(!S_ISDIR(le16_to_cpu(inode->i.i_mode)));
	ASSERT(!S_ISLNK(le16_to_cpu(inode->i.i_mode)));

	/* Adjust count with file length. */
	filesize = le64_to_cpu(inode->i.i_size);
	if (offset > filesize)
		count = 0;
	else if (count + offset > filesize)
		count = filesize - offset;

	/* Main loop for file blocks */
	read_count = remained_blkentries = 0;
	while (count > 0) {
		if (remained_blkentries == 0) {
			set_new_dnode(&dn, inode, NULL, ino);
			get_dnode_of_data(sbi, &dn, F2FS_BYTES_TO_BLK(offset),
					LOOKUP_NODE);
			if (index_node)
				free(index_node);
			index_node = (dn.node_blk == dn.inode_blk) ?
							NULL : dn.node_blk;
			remained_blkentries = ADDRS_PER_PAGE(sbi,
						dn.node_blk, dn.inode_blk);
		}
		ASSERT(remained_blkentries > 0);

		blkaddr = datablock_addr(dn.node_blk, dn.ofs_in_node);
		if (!IS_IN_MAIN_AREA(sbi, blkaddr))
			blkaddr = NULL_ADDR;
		if ((c.cached_ino != ino) &&
			(blkaddr == NULL_ADDR || blkaddr == NEW_ADDR))
			break;

		off_in_blk = offset % BLOCK_SZ;
		len_in_blk = BLOCK_SZ - off_in_blk;
		if (len_in_blk > count)
			len_in_blk = count;

		/* Read data from single block. */
		if (len_in_blk < BLOCK_SZ) {
			if (c.cached_ino == ino)
				hmfs_grab_data_from_cache(blk_buffer,
						F2FS_BYTES_TO_BLK(offset), blkaddr);
			else
				ASSERT(dev_read_block(blk_buffer, blkaddr) >= 0);
			memcpy(buffer, blk_buffer + off_in_blk, len_in_blk);
		} else {
			/* Direct read */
			if (c.cached_ino == ino)
				hmfs_grab_data_from_cache(buffer,
						F2FS_BYTES_TO_BLK(offset), blkaddr);
			else
				ASSERT(dev_read_block(buffer, blkaddr) >= 0);
		}

		offset += len_in_blk;
		count -= len_in_blk;
		buffer += len_in_blk;
		read_count += len_in_blk;

		dn.ofs_in_node++;
		remained_blkentries--;
	}
	if (index_node)
		free(index_node);
	free(blk_buffer);

	return read_count;
}

u64 f2fs_write(struct f2fs_sb_info *sbi, nid_t ino, u8 *buffer,
					u64 count, pgoff_t offset)
{
	struct dnode_of_data dn;
	struct node_info ni;
	struct f2fs_node *inode;
	char *blk_buffer;
	u64 off_in_blk;
	u64 len_in_blk;
	u64 written_count;
	u64 remained_blkentries;
	block_t blkaddr;
	void* index_node = NULL;
	int idirty = 0;
	int err;

	/* Memory allocation for block buffer and inode. */
	blk_buffer = calloc(BLOCK_SZ, 2);
	ASSERT(blk_buffer);
	inode = (struct f2fs_node*)(blk_buffer + BLOCK_SZ);

	/* Read inode */
	get_node_info(sbi, ino, &ni);
	if (c.cached_ino == ino)
		hmfs_grab_node_from_cache(inode, ino, ni.blk_addr);
	else
		ASSERT(dev_read_block(inode, ni.blk_addr) >= 0);
	ASSERT(!S_ISDIR(le16_to_cpu(inode->i.i_mode)));
	ASSERT(!S_ISLNK(le16_to_cpu(inode->i.i_mode)));

	/* Main loop for file blocks */
	written_count = remained_blkentries = 0;
	while (count > 0) {
		if (remained_blkentries == 0) {
			set_new_dnode(&dn, inode, NULL, ino);
			err = get_dnode_of_data(sbi, &dn,
					F2FS_BYTES_TO_BLK(offset), ALLOC_NODE);
			if (err)
				break;
			idirty |= dn.idirty;
			if (index_node)
				free(index_node);
			index_node = (dn.node_blk == dn.inode_blk) ?
							NULL : dn.node_blk;
			remained_blkentries = ADDRS_PER_PAGE(sbi,
						dn.node_blk, dn.inode_blk);
		}
		ASSERT(remained_blkentries > 0);

		blkaddr = datablock_addr(dn.node_blk, dn.ofs_in_node);
		if (!IS_IN_MAIN_AREA(sbi, blkaddr))
			blkaddr = NULL_ADDR;
		if (blkaddr == NULL_ADDR || blkaddr == NEW_ADDR) {
			memset(blk_buffer, 0, BLOCK_SZ);
		}

		off_in_blk = offset % BLOCK_SZ;
		len_in_blk = BLOCK_SZ - off_in_blk;
		if (len_in_blk > count)
			len_in_blk = count;

		/* Write data to single block. */
		if (len_in_blk < BLOCK_SZ) {
			if (c.cached_ino == ino) {
				hmfs_grab_data_from_cache(blk_buffer,
						F2FS_BYTES_TO_BLK(offset), blkaddr);
			} else if (blkaddr != NULL_ADDR && blkaddr != NEW_ADDR) {
				ASSERT(dev_read_block(blk_buffer, blkaddr) >= 0);
			}
			memcpy(blk_buffer + off_in_blk, buffer, len_in_blk);

			if (c.cached_ino == ino)
				hmfs_write_data_to_cache(blk_buffer, F2FS_BYTES_TO_BLK(offset), &dn, blkaddr);
			else
				ASSERT(f2fs_write_data_block(sbi, blk_buffer,
							blkaddr, &dn, CURSEG_HOT_DATA) >= 0);
		} else {
			/* Direct write */
			if (c.cached_ino == ino)
				hmfs_write_data_to_cache(blk_buffer, F2FS_BYTES_TO_BLK(offset), &dn, blkaddr);
			else
				ASSERT(f2fs_write_data_block(sbi, blk_buffer,
							blkaddr, &dn, CURSEG_HOT_DATA) >= 0);
		}

		offset += len_in_blk;
		count -= len_in_blk;
		buffer += len_in_blk;
		written_count += len_in_blk;

		dn.ofs_in_node++;
		if ((--remained_blkentries == 0 || count == 0) && (dn.ndirty)) {
			if (c.cached_ino == ino)
				hmfs_write_node_to_cache(dn.node_blk, dn.node_blkaddr);
			else
				ASSERT(f2fs_write_node_block(sbi, dn.node_blk,
						dn.node_blkaddr) >= 0);
		}
	}
	if (offset > le64_to_cpu(inode->i.i_size)) {
		inode->i.i_size = cpu_to_le64(offset);
		idirty = 1;
	}
	if (idirty || dn.idirty) {
		ASSERT(inode == dn.inode_blk);
		/* ni.blk_addr may have been changed, needs to be geted again */
		get_node_info(sbi, ino, &ni);
		if (c.cached_ino == ino)
			hmfs_write_node_to_cache(inode, ni.blk_addr);
		else
			write_inode(sbi, ni.blk_addr, inode);
	}
	if (index_node)
		free(index_node);
	free(blk_buffer);

	return written_count;
}

/*
 * This function updates the buf from blk_addr.
 * update in place if the blk_addr is in meta area,
 * else allocate a new block.
 */
int f2fs_write_block(struct f2fs_sb_info *sbi, void *buf,
					u64 blk_addr, bool is_meta)
{
	struct f2fs_sm_info *sm_i = SM_I(sbi);
	struct f2fs_summary sum;
	block_t to;
	int ret, type;
	bool is_inode = 0;

	if (is_meta) {
		ASSERT(blk_addr < sm_i->main_blkaddr);
		set_write_stream_id(META_STREAM_ID, blk_addr);
		ret = dev_write_block(buf, blk_addr);
		ASSERT(ret >= 0);
	} else {
		if (blk_addr < sm_i->main_blkaddr) {
			DBG(0, "blk_addr:%ld < sm_i->main_blkaddr %ld\n",
						blk_addr, sm_i->main_blkaddr);
		}
		ASSERT(blk_addr >= sm_i->main_blkaddr);

		/* update sit bitmap & valid_blocks && se->type */
		type = set_block_free(sbi, blk_addr);
		if (IS_NODESEG(type))
			is_inode = IS_INODE(buf);

		/* read/write SSA */
		get_sum_entry(sbi, blk_addr, &sum);
		to = blk_addr;
		ret = reserve_new_block(sbi, &to, &sum, type, is_inode);
		if (ret) {
			c.alloc_failed = 1;
			return ret;
		}

		if (IS_NODESEG(type)) {
			struct f2fs_node *node_blk = buf;

			node_blk->footer.next_blkaddr =
				cpu_to_le32(next_free_blkaddr(sbi, type));
		}

		set_write_stream_id(get_stream_id(type), to);
		ret = dev_write_block(buf, to);
		ASSERT(ret >= 0);

		/* if data block, read node and update node block */
		if (IS_DATASEG(type))
			update_data_blkaddr(sbi, le32_to_cpu(sum.nid),
					le16_to_cpu(sum.ofs_in_node), to);
		else
			update_nat_blkaddr(sbi, 0, le32_to_cpu(sum.nid), to);

		DBG(1, "f2fs write %s block type:%d %"PRIx64" -> %"PRIx64"\n",
				IS_DATASEG(type) ? "data" : "node", type,
				blk_addr, to);
	}
	return 0;
}

/* This function updates only inode->i.i_size */
void f2fs_filesize_update(struct f2fs_sb_info *sbi, nid_t ino, u64 filesize)
{
	struct node_info ni;
	struct f2fs_node *inode;

	inode = calloc(BLOCK_SZ, 1);
	ASSERT(inode);
	get_node_info(sbi, ino, &ni);

	if (c.cached_ino == ino)
		hmfs_grab_node_from_cache(inode, ino, ni.blk_addr);
	else
		ASSERT(dev_read_block(inode, ni.blk_addr) >= 0);
	ASSERT(!S_ISDIR(le16_to_cpu(inode->i.i_mode)));
	ASSERT(!S_ISLNK(le16_to_cpu(inode->i.i_mode)));

	inode->i.i_size = cpu_to_le64(filesize);

	if (c.cached_ino == ino)
		hmfs_write_node_to_cache(inode, ni.blk_addr);
	else
		write_inode(sbi, ni.blk_addr, inode);
	free(inode);
}

int f2fs_build_file(struct f2fs_sb_info *sbi, struct dentry *de)
{
	int fd, n;
	pgoff_t off = 0;
	u8 buffer[BLOCK_SZ];

	if (de->ino == 0)
		return -1;

	fd = open(de->full_path, O_RDONLY);
	if (fd < 0) {
		MSG(0, "Skip: Fail to open %s\n", de->full_path);
		return -1;
	}

	/* We disable inline_data here, for old kernels don't support it */
	if (0) {
		struct node_info ni;
		struct f2fs_node *node_blk;
		int ret;

		get_node_info(sbi, de->ino, &ni);

		node_blk = calloc(BLOCK_SZ, 1);
		ASSERT(node_blk);

		ret = dev_read_block(node_blk, ni.blk_addr);
		ASSERT(ret >= 0);

		node_blk->i.i_inline |= F2FS_INLINE_DATA;
		node_blk->i.i_inline |= F2FS_DATA_EXIST;

		if (c.feature & cpu_to_le32(F2FS_FEATURE_EXTRA_ATTR)) {
			node_blk->i.i_inline |= HMFS_EXTRA_ATTR;
			node_blk->i.i_extra_isize =
					cpu_to_le16(calc_extra_isize());
		}
		n = read(fd, buffer, BLOCK_SZ);
		ASSERT((unsigned long)n == de->size);
		memcpy(inline_data_addr(node_blk), buffer, de->size);
		node_blk->i.i_size = cpu_to_le64(de->size);
		write_inode(sbi, ni.blk_addr, node_blk);
		free(node_blk);
	} else {
		while ((n = read(fd, buffer, BLOCK_SZ)) > 0) {
			f2fs_write(sbi, de->ino, buffer, n, off);
			off += n;
		}
	}

	close(fd);
	if (n < 0)
		return -1;

	update_free_segments(sbi);

	MSG(1, "Info: Create %s -> %s\n"
		"  -- ino=%x, type=%x, mode=%x, uid=%x, "
		"gid=%x, cap=%"PRIx64", size=%lu, pino=%x\n",
		de->full_path, de->path,
		de->ino, de->file_type, de->mode,
		de->uid, de->gid, de->capabilities, de->size, de->pino);
	return 0;
}

static void get_new_segment(struct f2fs_sb_info *sbi,
				unsigned int *newseg, int dir,
				int type, int new_sec)
{
	struct free_segmap_info *free_i = FREE_I(sbi);
	unsigned int segno, secno;
	unsigned int hint = GET_SEC_FROM_SEG(sbi, *newseg);
	unsigned int left_start = hint;
	int go_left = 0;
	unsigned int mode = sbi->flash_mode[type];
	int baseline_segs = mode ? SLC_END_SEG_IN_SEC(sbi) : sbi->segs_per_sec;

	if (!new_sec && ((*newseg + 1) % baseline_segs)) {
		segno = *newseg + 1;
		goto got_it;
	}

	secno = find_next_zero_bit_le(free_i->free_secmap,
					MAIN_SECS(sbi), hint);
	if (secno >= MAIN_SECS(sbi)) {
		if (dir == ALLOC_RIGHT) {
			secno = find_next_zero_bit_le(free_i->free_secmap,
							MAIN_SECS(sbi), 0);
			ASSERT(secno < MAIN_SECS(sbi));
		} else {
			go_left = 1;
			left_start = hint - 1;
		}
	}
	if (go_left == 0)
		goto got_secno;

	while (test_bit_le(left_start, free_i->free_secmap)) {
		if (left_start > 0) {
			left_start--;
			continue;
		}
		left_start = find_next_zero_bit_le(free_i->free_secmap,
							MAIN_SECS(sbi), 0);
		ASSERT(left_start < MAIN_SECS(sbi));
		break;
	}
	secno = left_start;

got_secno:
	segno = GET_SEG_FROM_SEC(sbi, secno);
	sbi->flash_mode[type] = TLC_MODE;
got_it:
	/* set it as dirty segment in free segmap */
	ASSERT(test_bit_le(segno, free_i->free_segmap) == 0);
	__set_inuse(sbi, segno);
	*newseg = segno;
}


void get_new_curseg(struct f2fs_sb_info *sbi, int left,
		struct curseg_info *curseg, int type, int new_sec)
{
	unsigned int segno = curseg->segno;
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);
	int ret;

	ASSERT(f2fs_write_block(sbi, curseg->sum_blk, GET_SUM_BLKADDR(sbi, segno), 1) >= 0);

	get_new_segment(sbi, &segno, left, type, new_sec);
	if (!IS_DM_SEG(type) && !(segno % sbi->segs_per_sec)) {
		ret = set_section_info(START_BLOCK(sbi, segno), TLC_MODE, get_stream_id(type));
		if (ret) {
			ERR_MSG("Errno:%d set section info[%d] to device failed\n",
				ret, get_stream_id(type));
			ASSERT(0);
		}
	}

	curseg->segno = segno;
	curseg->zone = GET_ZONE_FROM_SEG(sbi, curseg->segno);
	curseg->next_blkoff = 0;
	curseg->alloc_type = LFS;
	curseg->next_segno = NULL_SEGNO;

	if (type < CURSEG_HOT_NODE) {
		set_cp(cur_data_segno[type], curseg->segno);
		set_cp(cur_data_blkoff[type], curseg->next_blkoff);
	} else {
		int n = type - CURSEG_HOT_NODE;

		set_cp(cur_node_segno[n], curseg->segno);
		set_cp(cur_node_blkoff[n], curseg->next_blkoff);
	}

	reset_curseg(sbi, type);
}

void add_sum_entry(struct curseg_info *curseg,
					struct f2fs_summary *sum)
{
	void *addr = curseg->sum_blk;

	addr += curseg->next_blkoff * sizeof(struct f2fs_summary);
	memcpy(addr, sum, sizeof(struct f2fs_summary));
}

unsigned int get_stream_id(unsigned char type)
{
	if (type == CURSEG_HOT_DATA || type == CURSEG_WARM_DATA)
		return HOT_DATA_STREAM_ID;
	else if (type == CURSEG_COLD_DATA)
		return COLD_DATA_STREAM_ID;
	else if (type == CURSEG_HOT_NODE || type == CURSEG_WARM_NODE)
		return HOT_NODE_STREAM_ID;
	else if (type == CURSEG_COLD_NODE)
		return COLD_NODE_STREAM_ID;
	else
		return META_STREAM_ID;
}

void free_enough_sections(struct f2fs_sb_info *sbi)
{
	int i = 0;

check_free_secs:
	/*
	 * One more section may not be generated in one GC.
	 * So we do gc until we get one more section. If gc times
	 * are more than MAX_GC_TIMES, there may be some error, exit(1)
	 */
	if (has_not_enough_free_secs(sbi)) {
		fsck_gc(sbi);
		i++;
		if (i > MAX_GC_TIMES) {
			MSG(0, "[fsck] gc times more then %d", MAX_GC_TIMES);
			ASSERT(0);
		}
		goto check_free_secs;
	}
}
