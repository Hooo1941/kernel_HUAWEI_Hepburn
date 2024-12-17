/**
 * resize.c
 *
 * Copyright (c) 2015 Jaegeuk Kim <jaegeuk@kernel.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include "fsck.h"

static void migrate_ssa(struct f2fs_sb_info *sbi,
			struct f2fs_super_block *sb)
{
	int ret;
	__u64 head_addr = get_sb(ssa_blkaddr);
	size_t len = get_sb(segment_count_main) * sizeof(struct f2fs_summary_block);

	ret = dev_write(sbi->sum_blks, head_addr << F2FS_BLKSIZE_BITS, len, 0, 0, 0);
	ASSERT(ret >= 0);
}

static int build_new_sum_blocks(struct f2fs_sb_info *sbi,
					unsigned int segment_count_main)
{
	sbi->sum_blks = calloc(segment_count_main *
			sizeof(struct f2fs_summary_block), 1);
	if (!sbi->sum_blks) {
		MSG(0, "\tError: Calloc failed for build_new_sum_blocks!\n");
		return -ENOMEM;
	}
	return 0;
}

static void free_new_sit_sentries(struct f2fs_sb_info *sbi)
{
	unsigned int i;
	struct sit_info *sit_i = SM_I(sbi)->sit_info;

	for (i = 0; i < NEW_TOTAL_SEGS(sbi); i++)
		free(sit_i->new_sentries[i].cur_valid_map);

	free(sit_i->new_sentries);
}

static int build_new_sit_sentries(struct f2fs_sb_info *sbi,
				unsigned int segment_count_main)
{
	unsigned int start;
	struct sit_info *sit_i = SM_I(sbi)->sit_info;
	sit_i->new_sentries = calloc(segment_count_main * sizeof(struct seg_entry), 1);

	if (!sit_i->new_sentries) {
		MSG(0, "\tError: Calloc failed for build_new_sit_sentries!\n");
		return -ENOMEM;
	}

	for (start = 0; start < segment_count_main; start++) {
		sit_i->new_sentries[start].cur_valid_map
			= calloc(SIT_VBLOCK_MAP_SIZE, 1);
		if (!sit_i->new_sentries[start].cur_valid_map) {
			MSG(0, "\tError: Calloc failed for build_new_sit_sentries!\n");
			goto err;
		}
	}
	return 0;

err:
	while (start-- > 0)
		free(sit_i->new_sentries[start].cur_valid_map);
	free(sit_i->new_sentries);
	return -ENOMEM;
}

static struct seg_entry *get_new_seg_entry(struct f2fs_sb_info *sbi,
		unsigned int segno)
{
	struct sit_info *sit_i = SIT_I(sbi);
	return &sit_i->new_sentries[segno];
}

static block_t __next_free_blkaddr(struct f2fs_super_block *sb,
		struct curseg_info *curseg)
{
	return get_sb(main_blkaddr) +
		(curseg->segno << get_sb(log_blocks_per_seg)) +
		curseg->next_blkoff;
}

static void init_cur_seg(struct f2fs_sb_info *sbi, struct f2fs_super_block *sb)
{
	int i, ret;
	int n;
	struct curseg_info *curseg = NULL;
	u64 ssa_blk;
	struct sit_info *sit_i = SM_I(sbi)->sit_info;
	struct summary_footer *sum_footer = NULL;
	struct seg_entry *se = NULL;
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);

	set_write_stream_id(META_STREAM_ID, get_sb(segment0_blkaddr));
	for (i = 0; i < NO_CHECK_TYPE; i++) {
		curseg = CURSEG_I(sbi, i);

		cp->alloc_type[i] = curseg->alloc_type;
		if (i < CURSEG_HOT_NODE) {
			set_cp(cur_data_segno[i], -1);
		} else if (i < CURSEG_DATA_MOVE1) {
			n = i - CURSEG_HOT_NODE;
			set_cp(cur_node_segno[n], -1);
		} else {
			n = i - CURSEG_DATA_MOVE1;
			set_cp(cur_dmgc_segno[n], -1);
		}

		if (is_set_ckpt_flags(F2FS_CKPT(sbi), CP_DATAMOVE_FLAG) ||
				!IS_DM_SEG(i)) {
			/* update original SSA */
			ssa_blk = GET_SUM_BLKADDR(sbi, curseg->segno);
			ret = dev_write_block_no_sync(curseg->sum_blk, ssa_blk);
			ASSERT(ret >= 0);
		}

		if (sbi->active_logs == NR_UNIFY_CURSEG_TYPE &&
				(i == CURSEG_WARM_NODE || i == CURSEG_WARM_DATA)) {
			curseg->segno = NULL_SEGNO;
			curseg->next_blkoff = 0;
			continue;
		}

		curseg->segno = sit_i->next_free_section * c.segs_per_sec;
		sit_i->free_segments--;
		sit_i->next_free_section++;

		se = get_new_seg_entry(sbi, curseg->segno);

		if (IS_DM_SEG(i)) {
			se->type = curseg_dm_type[i - CURSEG_DATA_MOVE1];
		} else {
			se->type = i;
			ret = set_section_info(get_sb(main_blkaddr) +
				(curseg->segno << sbi->log_blocks_per_seg),
				TLC_MODE, get_stream_id(i));
			if (ret) {
				ERR_MSG("Errno:%d set section info[%d] to device failed\n",
					ret, get_stream_id(i));
				ASSERT(0);
			}
		}
		se->dirty = 1;

		curseg->next_blkoff = 0;
		curseg->alloc_type = LFS;

		/*
		 * section_size / curseg->max_cache_size must be an integer,
		 * otherwise, the address may not be continuous in
		 * curseg->seg_cache.
		 */
		curseg->max_cache_size = c.segs_per_sec *
					sbi->blocks_per_seg * F2FS_BLKSIZE;
		while (curseg->max_cache_size > MAX_CACHED_SIZE)
			curseg->max_cache_size = curseg->max_cache_size >> 1;
		curseg->cached_size = 0;
		curseg->cached_start_addr = __next_free_blkaddr(sb, curseg);
		curseg->seg_cache = calloc(curseg->max_cache_size, 1);
		if (!curseg->seg_cache) {
			MSG(0, "\tError: calloc failed for current segment cache!"
					" Cache will not be used later\n");
		}

		memset(curseg->sum_blk, 0, SUM_ENTRIES_SIZE);
		sum_footer = &(curseg->sum_blk->footer);
		if (IS_DATASEG(i) || IS_DM_SEG(i))
			SET_SUM_TYPE(sum_footer, SUM_TYPE_DATA);
		if (IS_NODESEG(i))
			SET_SUM_TYPE(sum_footer, SUM_TYPE_NODE);
	}
}

static unsigned int get_new_segment(struct f2fs_sb_info *sbi,
					unsigned int old_seg)
{
	unsigned int new_seg;
	struct sit_info *sit_i = SM_I(sbi)->sit_info;

	if ((old_seg + 1) % c.segs_per_sec) {
		new_seg = old_seg + 1;
	} else {
		new_seg = sit_i->next_free_section * c.segs_per_sec;
		sit_i->next_free_section++;
	}

	sit_i->free_segments--;
	return new_seg;
}

static void new_curseg(struct f2fs_sb_info *sbi, struct curseg_info *curseg,
		int type, struct f2fs_super_block *new_sb)
{
	struct seg_entry *se = NULL;
	struct summary_footer *sum_footer = NULL;
	int ret;

	curseg->next_segno = get_new_segment(sbi, curseg->segno);
	if (!IS_DM_SEG(type) && !(curseg->next_segno % c.segs_per_sec)) {
		ret = set_section_info(get_newsb(main_blkaddr) +
			(curseg->next_segno << sbi->log_blocks_per_seg),
			TLC_MODE, get_stream_id(type));
		if (ret) {
			ERR_MSG("Errno:%d set section info[%d] to device failed\n",
				ret, get_stream_id(type));
			ASSERT(0);
		}
	}

	memcpy(sbi->sum_blks + curseg->segno, curseg->sum_blk,
				sizeof(struct f2fs_summary_block));

	curseg->segno = curseg->next_segno;
	curseg->next_blkoff = 0;
	curseg->alloc_type = LFS;
	memset(curseg->sum_blk, 0, SUM_ENTRIES_SIZE);

	sum_footer = &(curseg->sum_blk->footer);
	if (IS_DATASEG(type))
		SET_SUM_TYPE(sum_footer, SUM_TYPE_DATA);
	if (IS_NODESEG(type))
		SET_SUM_TYPE(sum_footer, SUM_TYPE_NODE);

	se = get_new_seg_entry(sbi, curseg->segno);
	se->type = type;
	se->dirty = 1;
}

static int flush_curseg_cache(struct f2fs_super_block *sb,
				struct curseg_info *curseg, int type)
{
	int ret = 0;
	__u64 head_addr = curseg->cached_start_addr;

	if (!curseg->seg_cache || curseg->cached_size == 0)
		return 0;

	MSG(0,"flush: head:%x, index:%d, type:%d\n", head_addr, curseg->cached_size, type);
	ret = dev_write(curseg->seg_cache, head_addr << F2FS_BLKSIZE_BITS,
				curseg->cached_size, 1, 1, get_stream_id(type));
	ASSERT(ret >= 0);

	curseg->cached_size = 0;
	curseg->cached_start_addr = __next_free_blkaddr(sb, curseg);
	memset(curseg->seg_cache, 0, curseg->max_cache_size);

	return ret;
}

static void block_write_to_cache(struct f2fs_sb_info *sbi, void *buf,
						block_t to, int type)
{
	int ret = 0;
	struct curseg_info *curseg = CURSEG_I(sbi, type);
	char *addr = curseg->seg_cache;

	if (!curseg->seg_cache) {
		set_write_stream_id(get_stream_id(type), to);
		ret = dev_write_block(buf, to);
		ASSERT(ret >= 0);
	} else {
		addr += curseg->cached_size;
		memcpy(addr, buf, F2FS_BLKSIZE);
		curseg->cached_size += F2FS_BLKSIZE;
		ASSERT(curseg->cached_size <= curseg->max_cache_size);
	}
}

static void allocate_data_block(struct f2fs_sb_info *sbi, block_t *new_blkaddr,
		struct f2fs_super_block *new_sb, void *buf,
		struct f2fs_summary *sum, int type)
{
	if (sbi->active_logs == NR_UNIFY_CURSEG_TYPE) {
		if (type == CURSEG_WARM_DATA)
			type = CURSEG_HOT_DATA;
		else if (type == CURSEG_WARM_NODE)
			type = CURSEG_HOT_NODE;
	}

	struct curseg_info *curseg = CURSEG_I(sbi, type);
	struct seg_entry *se = get_new_seg_entry(sbi, curseg->segno);
	*new_blkaddr = __next_free_blkaddr(new_sb, curseg);
	block_write_to_cache(sbi, buf, *new_blkaddr, type);
	add_sum_entry(curseg, sum);

	se->type = type;
	se->valid_blocks++;
	f2fs_set_bit(curseg->next_blkoff, (char *)se->cur_valid_map);
	se->dirty = 1;

	curseg->next_blkoff++;
	if (!__has_curseg_space(sbi, curseg)) {
		new_curseg(sbi, curseg, type, new_sb);
	}
	if (curseg->cached_size == curseg->max_cache_size) {
		flush_curseg_cache(new_sb, curseg, type);
	}
}

static int get_new_sb(struct f2fs_sb_info *sbi, struct f2fs_super_block *sb)
{
	u_int32_t zone_size_bytes;
	u_int64_t zone_align_start_offset, segment_align_start_offset;
	u_int32_t blocks_for_sit, blocks_for_nat, blocks_for_ssa;
	u_int32_t sit_segments, nat_segments;
	u_int32_t total_meta_blocks, total_meta_segments;
	u_int32_t total_valid_blks_available;
	u_int32_t sit_bitmap_size, max_sit_bitmap_size;
	u_int32_t max_nat_bitmap_size, max_nat_segments;
	u_int32_t log_sectorsize, log_sectors_per_block;
	u_int64_t blk_size_bytes;
	struct sit_info *sit_i = SM_I(sbi)->sit_info;

	/* We need to update sector size */
	log_sectorsize = log_base_2(c.sector_size);
	log_sectors_per_block = log_base_2(c.sectors_per_blk);
	set_sb(log_sectorsize, log_sectorsize);
	set_sb(log_sectors_per_block, log_sectors_per_block);
	set_sb(segs_per_sec, c.segs_per_sec);
	blk_size_bytes = 1 << get_sb(log_blocksize);

	u_int32_t segment_size_bytes = 1 << (get_sb(log_blocksize) +
					get_sb(log_blocks_per_seg));
	u_int32_t blks_per_seg = 1 << get_sb(log_blocks_per_seg);
	u_int32_t segs_per_zone = get_sb(segs_per_sec) * get_sb(secs_per_zone);

	set_sb(block_count, c.target_sectors >>
				get_sb(log_sectors_per_block));

	zone_size_bytes = segment_size_bytes * segs_per_zone;
	segment_align_start_offset =
		((u_int64_t) c.start_sector * DEFAULT_SECTOR_SIZE +
		2 * F2FS_BLKSIZE + segment_size_bytes - 1) /
		segment_size_bytes * segment_size_bytes -
		(u_int64_t) c.start_sector * DEFAULT_SECTOR_SIZE;

	set_sb(segment0_blkaddr, segment_align_start_offset / blk_size_bytes);
	sb->cp_blkaddr = sb->segment0_blkaddr;
	set_sb(sit_blkaddr, get_sb(segment0_blkaddr) +
			get_sb(segment_count_ckpt) * c.blks_per_seg);

	set_sb(segment_count, (c.target_sectors * c.sector_size -
				segment_align_start_offset) / segment_size_bytes);

	if (c.safe_resize)
		goto safe_resize;

	blocks_for_sit = SIZE_ALIGN(get_sb(segment_count), SIT_ENTRY_PER_BLOCK);
	sit_segments = SEG_ALIGN(blocks_for_sit);
	set_sb(segment_count_sit, sit_segments * 2);
	set_sb(nat_blkaddr, get_sb(sit_blkaddr) +
				get_sb(segment_count_sit) * blks_per_seg);

	total_valid_blks_available = (get_sb(segment_count) -
			(get_sb(segment_count_ckpt) +
			get_sb(segment_count_sit))) * blks_per_seg;
	blocks_for_nat = SIZE_ALIGN(total_valid_blks_available,
					NAT_ENTRY_PER_BLOCK);

	if (c.large_nat_bitmap) {
		nat_segments = SEG_ALIGN(blocks_for_nat) *
						DEFAULT_NAT_ENTRY_RATIO / 100;
		set_sb(segment_count_nat, nat_segments ? nat_segments : 1);

		max_nat_bitmap_size = (get_sb(segment_count_nat) <<
						get_sb(log_blocks_per_seg)) / 8;
		set_sb(segment_count_nat, get_sb(segment_count_nat) * 2);
	} else {
		set_sb(segment_count_nat, SEG_ALIGN(blocks_for_nat));
		max_nat_bitmap_size = 0;
	}

	sit_bitmap_size = ((get_sb(segment_count_sit) / 2) <<
				get_sb(log_blocks_per_seg)) / 8;
	if (sit_bitmap_size > MAX_SIT_BITMAP_SIZE)
		max_sit_bitmap_size = MAX_SIT_BITMAP_SIZE;
	else
		max_sit_bitmap_size = sit_bitmap_size;

	if (c.large_nat_bitmap) {
		/* use cp_payload if free space of f2fs_checkpoint is not enough */
		if (max_sit_bitmap_size + max_nat_bitmap_size >
						MAX_BITMAP_SIZE_IN_CKPT) {
			u_int32_t diff =  max_sit_bitmap_size +
						max_nat_bitmap_size -
						MAX_BITMAP_SIZE_IN_CKPT;
			set_sb(cp_payload, F2FS_BLK_ALIGN(diff));
		} else {
			set_sb(cp_payload, 0);
		}
	} else {
		/*
		 * It should be reserved minimum 1 segment for nat.
		 * When sit is too large, we should expand cp area.
		 * It requires more pages for cp.
		 */
		if (max_sit_bitmap_size > MAX_SIT_BITMAP_SIZE_IN_CKPT) {
			max_nat_bitmap_size = CP_CHKSUM_OFFSET - sizeof(struct f2fs_checkpoint) + 1;
			set_sb(cp_payload, F2FS_BLK_ALIGN(max_sit_bitmap_size));
		} else {
			max_nat_bitmap_size = CP_CHKSUM_OFFSET - sizeof(struct f2fs_checkpoint) + 1
				- max_sit_bitmap_size;
			set_sb(cp_payload, 0);
		}

		max_nat_segments = (max_nat_bitmap_size * 8) >>
					get_sb(log_blocks_per_seg);

		if (get_sb(segment_count_nat) > max_nat_segments)
			set_sb(segment_count_nat, max_nat_segments);

		set_sb(segment_count_nat, get_sb(segment_count_nat) * 2);
	}

	set_sb(ssa_blkaddr, get_sb(nat_blkaddr) +
				get_sb(segment_count_nat) * blks_per_seg);

	total_valid_blks_available = (get_sb(segment_count) -
			(get_sb(segment_count_ckpt) +
			get_sb(segment_count_sit) +
			get_sb(segment_count_nat))) * blks_per_seg;

	blocks_for_ssa = total_valid_blks_available / blks_per_seg + 1;

	set_sb(segment_count_ssa, SEG_ALIGN(blocks_for_ssa));
	total_meta_blocks = (get_sb(segment_count_ckpt) +
			     get_sb(segment_count_sit) +
			     get_sb(segment_count_nat) +
		             get_sb(segment_count_ssa)) * c.blks_per_seg;
	/* main_blkaddr should zone_aligned */
	zone_align_start_offset =
		((u_int64_t) c.start_sector * DEFAULT_SECTOR_SIZE +
		(get_sb(segment0_blkaddr) + total_meta_blocks) *
		F2FS_BLKSIZE + zone_size_bytes - 1) /
		zone_size_bytes * zone_size_bytes -
		(u_int64_t) c.start_sector * DEFAULT_SECTOR_SIZE;
	set_sb(main_blkaddr, zone_align_start_offset / blk_size_bytes);
	MSG(0, "Info: main_blkaddr = %x\n", get_sb(main_blkaddr));

	set_sb(segment_count_ssa,
		SEG_ALIGN(get_sb(main_blkaddr) - get_sb(ssa_blkaddr)));

safe_resize:
	total_meta_segments = get_sb(segment_count_ckpt) +
		get_sb(segment_count_sit) +
		get_sb(segment_count_nat) +
		get_sb(segment_count_ssa);

	set_sb(segment_count_main, get_sb(segment_count) -
			total_meta_segments);
	set_sb(section_count, get_sb(segment_count_main) /
						get_sb(segs_per_sec));
	set_sb(segment_count_main, get_sb(section_count) *
						get_sb(segs_per_sec));
	/* Segment_count is recalculated because some segments at the tail are truncated. */
	set_sb(segment_count, total_meta_segments + get_sb(segment_count_main));

	sit_i->free_segments = get_sb(segment_count_main);
	SM_I(sbi)->new_main_segments = get_sb(segment_count_main);

	/* Let's determine the best reserved and overprovisioned space */
	c.new_overprovision = get_best_overprovision(sb);

	c.original_reserved_segments =
		(2 * (100 / c.new_overprovision + 1) + NR_CURSEG_TYPE + 1);
	c.new_reserved_segments = HMFS_RSV_SEC * c.segs_per_sec;

	if ((get_sb(segment_count_main) - 2) < c.new_reserved_segments ||
		get_sb(segment_count_main) * blks_per_seg >
						get_sb(block_count)) {
		MSG(0, "\tError: Device size is not sufficient for HMFS volume, "
			"more segment needed =%u",
			c.new_reserved_segments -
			(get_sb(segment_count_main) - 2));
		return -1;
	}
	return 0;
}

static void __migrate_main_segment(struct f2fs_sb_info *sbi,
		struct f2fs_super_block *new_sb, enum SEG_TYPE type)
{
	struct seg_entry *se = NULL;
	block_t to;
	unsigned int i;
	int j, ret;
	struct f2fs_summary sum;
	struct f2fs_summary_block *sum_blk = NULL;
	int ret_type;
	__u64 blk_addr;
	void *segment_raw = calloc(F2FS_BLKSIZE * sbi->blocks_per_seg, 1);

	ASSERT(segment_raw != NULL);

	for (i = 0; i < TOTAL_SEGS(sbi); i++) {
		se = get_seg_entry(sbi, i);
		if (!se->valid_blocks)
			continue;
		if (get_seg_type(se->type) != type)
			continue;

		sum_blk = get_sum_block(sbi, i, &ret_type);
		blk_addr = START_BLOCK(sbi, i);
		ret = dev_read(segment_raw, blk_addr << F2FS_BLKSIZE_BITS,
						F2FS_BLKSIZE * sbi->blocks_per_seg);
		ASSERT(ret >= 0);

		for (j = 0; j < sbi->blocks_per_seg; j++) {
			if (!f2fs_test_bit(j, (const char *)se->cur_valid_map))
				continue;

			memcpy(&sum, &(sum_blk->entries[j]), sizeof(struct f2fs_summary));

			allocate_data_block(sbi, &to, new_sb,
					segment_raw + j * F2FS_BLKSIZE, &sum, se->type);
			set_write_stream_id(META_STREAM_ID, sbi->sm_info->seg0_blkaddr);
			if (type == SEG_DATA) {
				update_data_blkaddr(sbi, le32_to_cpu(sum.nid),
						le16_to_cpu(sum.ofs_in_node), to);
			} else {
				update_nat_blkaddr(sbi, 0,
						le32_to_cpu(sum.nid), to);
			}
		}
		if (type == SEG_TYPE_NODE || type == SEG_TYPE_DATA ||
				type == SEG_TYPE_MAX)
			free(sum_blk);
	}

	free(segment_raw);
}

static void migrate_main(struct f2fs_sb_info *sbi,
			struct f2fs_super_block *new_sb)
{
	/* first migreate data segment */
	__migrate_main_segment(sbi, new_sb, SEG_DATA);
	__migrate_main_segment(sbi, new_sb, SEG_NODE);
}

static void migrate_curseg(struct f2fs_sb_info *sbi,
			struct f2fs_super_block *new_sb)
{
	int i;
	struct curseg_info *curseg = NULL;

	for (i = 0; i < NO_CHECK_TYPE; i++) {
		curseg = CURSEG_I(sbi, i);
		if (curseg->segno == NULL_SEGNO) {
			continue;
		}
		flush_curseg_cache(new_sb, curseg, i);

		if (curseg->seg_cache)
			free(curseg->seg_cache);

		if (curseg->next_blkoff == 0)
			continue;

		memcpy(sbi->sum_blks + curseg->segno, curseg->sum_blk,
					sizeof(struct f2fs_summary_block));
	}
}

static int shrink_nats(struct f2fs_sb_info *sbi,
				struct f2fs_super_block *new_sb)
{
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	struct f2fs_nm_info *nm_i = NM_I(sbi);
	block_t old_nat_blkaddr = get_sb(nat_blkaddr);
	unsigned int nat_blocks;
	void *nat_block, *zero_block;
	int nid, ret, new_max_nid;
	pgoff_t block_off;
	pgoff_t block_addr;
	int seg_off;

	nat_block = malloc(BLOCK_SZ);
	ASSERT(nat_block);
	zero_block = calloc(BLOCK_SZ, 1);
	ASSERT(zero_block);

	nat_blocks = get_newsb(segment_count_nat) >> 1;
	nat_blocks = nat_blocks << get_sb(log_blocks_per_seg);
	new_max_nid = NAT_ENTRY_PER_BLOCK * nat_blocks;

	for (nid = nm_i->max_nid - 1; nid > new_max_nid; nid -= NAT_ENTRY_PER_BLOCK) {
		block_off = nid / NAT_ENTRY_PER_BLOCK;
		seg_off = block_off >> sbi->log_blocks_per_seg;
		block_addr = (pgoff_t)(old_nat_blkaddr +
				(seg_off << sbi->log_blocks_per_seg << 1) +
				(block_off & ((1 << sbi->log_blocks_per_seg) - 1)));

		if (f2fs_test_bit(block_off, nm_i->nat_bitmap))
			block_addr += sbi->blocks_per_seg;

		ret = dev_read_block(nat_block, block_addr);
		ASSERT(ret >= 0);

		if (memcmp(zero_block, nat_block, BLOCK_SZ)) {
			ret = -1;
			goto not_avail;
		}
	}
	ret = 0;
	nm_i->max_nid = new_max_nid;
not_avail:
	free(nat_block);
	free(zero_block);
	return ret;
}

static void migrate_nat(struct f2fs_sb_info *sbi,
			struct f2fs_super_block *new_sb)
{
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	struct f2fs_nm_info *nm_i = NM_I(sbi);
	block_t old_nat_blkaddr = get_sb(nat_blkaddr);
	block_t new_nat_blkaddr = get_newsb(nat_blkaddr);
	unsigned int nat_blocks;
	void *nat_block;
	int nid, ret, new_max_nid;
	pgoff_t block_off;
	pgoff_t block_addr;
	int seg_off;

	nat_block = malloc(BLOCK_SZ);
	ASSERT(nat_block);

	for (nid = nm_i->max_nid - 1; nid >= 0; nid -= NAT_ENTRY_PER_BLOCK) {
		block_off = nid / NAT_ENTRY_PER_BLOCK;
		seg_off = block_off >> sbi->log_blocks_per_seg;
		block_addr = (pgoff_t)(old_nat_blkaddr +
				(seg_off << sbi->log_blocks_per_seg << 1) +
				(block_off & ((1 << sbi->log_blocks_per_seg) - 1)));

		/* move to set #0 */
		if (f2fs_test_bit(block_off, nm_i->nat_bitmap)) {
			block_addr += sbi->blocks_per_seg;
			f2fs_clear_bit(block_off, nm_i->nat_bitmap);
		}

		ret = dev_read_block(nat_block, block_addr);
		ASSERT(ret >= 0);

		block_addr = (pgoff_t)(new_nat_blkaddr +
				(seg_off << sbi->log_blocks_per_seg << 1) +
				(block_off & ((1 << sbi->log_blocks_per_seg) - 1)));

		/* new bitmap should be zeros */
		ret = dev_write_block_no_sync(nat_block, block_addr);
		ASSERT(ret >= 0);
	}
	/* zero out newly assigned nids */
	memset(nat_block, 0, BLOCK_SZ);
	nat_blocks = get_newsb(segment_count_nat) >> 1;
	nat_blocks = nat_blocks << get_sb(log_blocks_per_seg);
	new_max_nid = NAT_ENTRY_PER_BLOCK * nat_blocks;

	DBG(1, "Write NAT block: %x->%x, max_nid=%x->%x\n",
			old_nat_blkaddr, new_nat_blkaddr,
			get_sb(segment_count_nat),
			get_newsb(segment_count_nat));

	for (nid = nm_i->max_nid; nid < new_max_nid;
				nid += NAT_ENTRY_PER_BLOCK) {
		block_off = nid / NAT_ENTRY_PER_BLOCK;
		seg_off = block_off >> sbi->log_blocks_per_seg;
		block_addr = (pgoff_t)(new_nat_blkaddr +
				(seg_off << sbi->log_blocks_per_seg << 1) +
				(block_off & ((1 << sbi->log_blocks_per_seg) - 1)));
		ret = dev_write_block_no_sync(nat_block, block_addr);
		ASSERT(ret >= 0);
		DBG(3, "Write NAT: %lx\n", block_addr);
	}
	free(nat_block);
	DBG(0, "Info: Done to migrate NAT blocks: nat_blkaddr = 0x%x -> 0x%x\n",
			old_nat_blkaddr, new_nat_blkaddr);
}

static void migrate_sit(struct f2fs_sb_info *sbi,
		struct f2fs_super_block *new_sb, unsigned int offset)
{
	struct sit_info *sit_i = SIT_I(sbi);
	unsigned int ofs = 0, pre_ofs = 0;
	unsigned int segno, index;
	struct f2fs_sit_block *sit_blk = calloc(BLOCK_SZ, 1);
	block_t sit_blks = get_newsb(segment_count_sit) <<
						(sbi->log_blocks_per_seg - 1);
	struct seg_entry *se;
	block_t blk_addr = 0;
	int ret;

	ASSERT(sit_blk);

	/* initialize with zeros */
	for (index = 0; index < sit_blks; index++) {
		ret = dev_write_block_no_sync(sit_blk, get_newsb(sit_blkaddr) + index);
		ASSERT(ret >= 0);
		DBG(3, "Write zero sit: %x\n", get_newsb(sit_blkaddr) + index);
	}

	for (segno = 0; segno < NEW_TOTAL_SEGS(sbi); segno++) {
		struct f2fs_sit_entry *sit;

		se = get_new_seg_entry(sbi, segno);
		if (segno < offset) {
			ASSERT(se->valid_blocks == 0);
			continue;
		}

		ofs = SIT_BLOCK_OFFSET(sit_i, segno - offset);

		if (ofs != pre_ofs) {
			blk_addr = get_newsb(sit_blkaddr) + pre_ofs;
			ret = dev_write_block_no_sync(sit_blk, blk_addr);
			ASSERT(ret >= 0);
			DBG(1, "Write valid sit: %x\n", blk_addr);

			pre_ofs = ofs;
			memset(sit_blk, 0, BLOCK_SZ);
		}

		sit = &sit_blk->entries[SIT_ENTRY_OFFSET(sit_i, segno - offset)];
		memcpy(sit->valid_map, se->cur_valid_map, SIT_VBLOCK_MAP_SIZE);
		sit->vblocks = cpu_to_le16((se->type << SIT_VBLOCKS_SHIFT) |
							se->valid_blocks);
	}
	blk_addr = get_newsb(sit_blkaddr) + ofs;
	ret = dev_write_block_no_sync(sit_blk, blk_addr);
	DBG(1, "Write valid sit: %x\n", blk_addr);
	ASSERT(ret >= 0);

	free(sit_blk);
	DBG(0, "Info: Done to restore new SIT blocks: 0x%x\n",
					get_newsb(sit_blkaddr));
}

static void rebuild_checkpoint(struct f2fs_sb_info *sbi,
			struct f2fs_super_block *new_sb, unsigned int offset)
{
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);
	unsigned long long cp_ver = get_cp(checkpoint_ver);
	struct f2fs_checkpoint *new_cp;
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	struct sit_info *sit_i = SM_I(sbi)->sit_info;
	block_t orphan_blks = 0;
	block_t new_cp_blk_no, old_cp_blk_no, new_cp_blk_no2;
	u_int32_t crc = 0;
	void *buf = NULL;
	char *dm_cache_block = NULL;
	int i, ret;
	int extra_ovp = get_device_overprovision(get_newsb(cp_blkaddr));
	int total_ovp;
	int hmfs_min_op_sec;
	u_int32_t flags;

	new_cp = calloc(BLOCK_SZ, 1);
	ASSERT(new_cp);

	buf = malloc(BLOCK_SZ);
	ASSERT(buf);

	/* ovp / free segments */
	set_cp(rsvd_segment_count, c.new_reserved_segments);

	/* copy from mkfs */
	set_cp(overprov_segment_count, (get_newsb(segment_count_main) -
		c.original_reserved_segments) * c.new_overprovision / 100);

	total_ovp = (get_cp(overprov_segment_count) +
			c.original_reserved_segments + extra_ovp) /
			c.segs_per_sec * c.segs_per_sec;

	hmfs_min_op_sec = hmfs_get_min_op_sec(HMFS_RSV_SEC);
	if (total_ovp < hmfs_min_op_sec * c.segs_per_sec)
		total_ovp = hmfs_min_op_sec * c.segs_per_sec;
	set_cp(overprov_segment_count, total_ovp);

	set_cp(free_segment_count, sit_i->free_segments - c.reserved_segs);
	set_cp(user_block_count, ((get_newsb(segment_count_main) -
			get_cp(overprov_segment_count) - c.reserved_segs) * c.blks_per_seg));

	if (is_set_ckpt_flags(cp, CP_ORPHAN_PRESENT_FLAG))
		orphan_blks = __start_sum_addr(sbi) - 1 - get_sb(cp_payload);

	set_cp(cp_pack_start_sum, 1 + get_newsb(cp_payload) + orphan_blks);
	set_cp(cp_pack_total_block_count, 2 + NR_CURSEG_TYPE + orphan_blks +
			get_newsb(cp_payload));
	flags = get_cp(ckpt_flags) | CP_DATAMOVE_FLAG;
	set_cp(ckpt_flags, flags);
	/* cur->segno - offset */
	for (i = 0; i < NO_CHECK_TYPE; i++) {
		if (i < CURSEG_HOT_NODE) {
			set_cp(cur_data_segno[i],
					CURSEG_I(sbi, i)->segno - offset);
			set_cp(cur_data_blkoff[i],
					CURSEG_I(sbi, i)->next_blkoff);
		} else if (i < CURSEG_DATA_MOVE1) {
			int n = i - CURSEG_HOT_NODE;

			set_cp(cur_node_segno[n],
					CURSEG_I(sbi, i)->segno - offset);
			set_cp(cur_node_blkoff[n],
					CURSEG_I(sbi, i)->next_blkoff);
		} else {
			int n = i - CURSEG_DATA_MOVE1;
			set_cp(cur_dmgc_segno[n],
					CURSEG_I(sbi, i)->segno - offset);
			set_cp(cur_dmgc_blkoff[n],
					CURSEG_I(sbi, i)->next_blkoff);
		}
	}

	/* sit / nat ver bitmap bytesize */
	set_cp(sit_ver_bitmap_bytesize,
			((get_newsb(segment_count_sit) / 2) <<
			get_newsb(log_blocks_per_seg)) / 8);
	set_cp(nat_ver_bitmap_bytesize,
			((get_newsb(segment_count_nat) / 2) <<
			get_newsb(log_blocks_per_seg)) / 8);

	memcpy(new_cp, cp, (unsigned char *)cp->sit_nat_version_bitmap -
						(unsigned char *)cp);
	new_cp->checkpoint_ver = cpu_to_le64(cp_ver + 1);

	crc = f2fs_cal_crc32(HMFS_SUPER_MAGIC, new_cp, CP_CHKSUM_OFFSET);
	*((__le32 *)((unsigned char *)new_cp + CP_CHKSUM_OFFSET)) =
							cpu_to_le32(crc);

	/* Write a new checkpoint in the other set */
	old_cp_blk_no = get_sb(cp_blkaddr);
	new_cp_blk_no2 = new_cp_blk_no = get_newsb(cp_blkaddr);
	if (sbi->cur_cp == 2) {
		old_cp_blk_no += 1 << get_sb(log_blocks_per_seg);
		new_cp_blk_no2 += 1 << get_sb(log_blocks_per_seg);
	} else {
		new_cp_blk_no += 1 << get_sb(log_blocks_per_seg);
	}

	/* write first cp */
	ret = dev_write_block_no_sync(new_cp, new_cp_blk_no++);
	ASSERT(ret >= 0);

	memset(buf, 0, BLOCK_SZ);
	for (i = 0; i < get_newsb(cp_payload); i++) {
		ret = dev_write_block_no_sync(buf, new_cp_blk_no++);
		ASSERT(ret >= 0);
	}

	for (i = 0; i < orphan_blks; i++) {
		block_t orphan_blk_no = old_cp_blk_no + 1 + get_sb(cp_payload);

		ret = dev_read_block(buf, orphan_blk_no++);
		ASSERT(ret >= 0);

		ret = dev_write_block_no_sync(buf, new_cp_blk_no++);
		ASSERT(ret >= 0);
	}

	/* update summary blocks having nullified journal entries */
	for (i = 0; i < NO_CHECK_TYPE; i++) {
		struct curseg_info *curseg = CURSEG_I(sbi, i);

		ret = dev_write_block_no_sync(curseg->sum_blk, new_cp_blk_no++);
		ASSERT(ret >= 0);
	}

	/* write the last cp */
	ret = dev_write_block_no_sync(new_cp, new_cp_blk_no++);
	ASSERT(ret >= 0);

	/* write for datamove */
	if (sbi->dm_cache_blocks == NULL) {
		memset(buf, 0, BLOCK_SZ);
		ret = dev_write_block_no_sync(buf, new_cp_blk_no++);
		ASSERT(ret >= 0);
	} else {
		for (i = 0; i < c.num_dm_blocks; i++) {
			dm_cache_block = sbi->dm_cache_blocks + i * PAGE_SIZE;
			ret = dev_write_block_no_sync(dm_cache_block, new_cp_blk_no++);
			ASSERT(ret >= 0);
		}
	}

	/* disable old checkpoint */
	crc = f2fs_cal_crc32(HMFS_SUPER_MAGIC, cp, CP_CHKSUM_OFFSET);
	*((__le32 *)((unsigned char *)cp + CP_CHKSUM_OFFSET)) = cpu_to_le32(crc);
	old_cp_blk_no = new_cp_blk_no2  + get_cp(cp_pack_total_block_count) - 1;
	ret = dev_write_block_no_sync(cp, old_cp_blk_no);
	ASSERT(ret >= 0);
	ret = dev_write_block_no_sync(cp, new_cp_blk_no2);
	ASSERT(ret >= 0);

	free(buf);
	free(new_cp);
	DBG(0, "Info: Done to rebuild checkpoint blocks\n");
}

static void update_checkpoint(struct f2fs_sb_info *sbi,
		struct f2fs_super_block *new_sb, unsigned int expand_secs)
{
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);
	unsigned long long cp_ver = get_cp(checkpoint_ver);
	struct f2fs_checkpoint *new_cp;
	block_t new_cp_blk_addr;
	u_int32_t crc = 0;
	int ret;

	new_cp = malloc(BLOCK_SZ);
	ASSERT(new_cp);

	/* copy valid checkpoint to its mirror position */
	duplicate_checkpoint(sbi);
	set_cp(free_segment_count, get_cp(free_segment_count) +
				expand_secs * c.segs_per_sec);
	set_cp(user_block_count, get_cp(user_block_count) +
			expand_secs * c.segs_per_sec * c.blks_per_seg);

	memcpy(new_cp, cp, BLOCK_SZ);
	new_cp->checkpoint_ver = cpu_to_le64(cp_ver + 1);
	crc = f2fs_cal_crc32(HMFS_SUPER_MAGIC, new_cp, CP_CHKSUM_OFFSET);
	*((__le32 *)((unsigned char *)new_cp + CP_CHKSUM_OFFSET)) =
							cpu_to_le32(crc);
	/* write first cp */
	new_cp_blk_addr = get_newsb(cp_blkaddr);
	ret = dev_write_block_no_sync(new_cp, new_cp_blk_addr);
	ASSERT(ret >= 0);
	/* write the last cp */
	new_cp_blk_addr += get_cp(cp_pack_total_block_count) - 1;
	ret = dev_write_block_no_sync(new_cp, new_cp_blk_addr);
	ASSERT(ret >= 0);

	free(new_cp);
	DBG(0, "Info: Done to rebuild checkpoint blocks\n");
}

static int f2fs_resize_grow_safe(struct f2fs_sb_info *sbi)
{
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	struct f2fs_super_block new_sb_raw;
	struct f2fs_super_block *new_sb = &new_sb_raw;
	struct stor_dev_mapping_partition mp;
	unsigned int expand_secs;
	int err = 0;
	u_int64_t blk_size_bytes;
	u_int64_t segment0_start_offset;
	u_int32_t segment_size_bytes;
	unsigned int segment_count, segment_count_main;
	unsigned int segment_count_meta;

	c.dbg_lv = 1;
	MSG(0, "before f2fs_resize_grow_safe\n");
	print_raw_sb_info(sb);
	print_ckpt_info(sbi);
	c.dbg_lv = 0;

	blk_size_bytes = 1 << get_sb(log_blocksize);
	segment_size_bytes = 1 << (get_sb(log_blocksize) +
					get_sb(log_blocks_per_seg));
	segment0_start_offset = get_sb(segment0_blkaddr) * blk_size_bytes;

	memcpy(new_sb, F2FS_RAW_SUPER(sbi), sizeof(*new_sb));
	set_newsb(block_count, c.target_sectors >>
				get_newsb(log_sectors_per_block));
	segment_count = (c.target_sectors * c.sector_size -
			segment0_start_offset) / segment_size_bytes;
	segment_count_meta = get_newsb(segment_count_ckpt) +
		get_newsb(segment_count_sit) +
		get_newsb(segment_count_nat) +
		get_newsb(segment_count_ssa);

	segment_count_main = segment_count - segment_count_meta;
	set_newsb(section_count, segment_count_main / c.segs_per_sec);
	set_newsb(segment_count_main, get_newsb(section_count) * c.segs_per_sec);
	set_newsb(segment_count, get_newsb(segment_count_main) +
			segment_count_meta);

	expand_secs = get_newsb(section_count) - get_sb(section_count);
	set_write_stream_id(META_STREAM_ID, get_sb(segment0_blkaddr));
	update_checkpoint(sbi, new_sb, expand_secs);

	err = get_device_segment_mapping_pos(&mp);
	if (err < 0) {
		MSG(0, "\tError: Failed to get device segment mapping pos!!!\n");
		return err;
	}
	if (c.ndevs > 1) {
		MSG(0, "\tError: Not support multi-devices resizing!!!\n");
		return -1;
	} else {
		unsigned int main_start_blk, main_start_sec;

		main_start_blk = get_newsb(main_blkaddr) +
				c.start_sector / DEFAULT_SECTORS_PER_BLOCK;
		main_start_sec = main_start_blk / c.blks_per_seg /
							c.segs_per_sec;
		if (main_start_sec * c.segs_per_sec * c.blks_per_seg !=
							main_start_blk) {
			MSG(0, "\tError: main start phy_addr[%u] is not "
					"section aligned!!!\n", main_start_blk);
			return -1;
		}

		mp.partion_size[PARTITION_TYPE_USER0] += expand_secs;
		mp.partion_size[PARTITION_TYPE_META1] -= expand_secs;
		if (mp.partion_size[PARTITION_TYPE_META1] == 0)
			mp.partion_start[PARTITION_TYPE_META1] = 0;
		else
			mp.partion_start[PARTITION_TYPE_META1] += expand_secs;

		/* Make sure 2M-mapping region is consistent with main area. */
		if (mp.partion_start[PARTITION_TYPE_USER0] != main_start_sec) {
			MSG(0, "\tError: user0 start %u, expect %u!!!\n",
					mp.partion_start[PARTITION_TYPE_USER0],
					main_start_sec);
			return -1;
		}
		if (mp.partion_size[PARTITION_TYPE_USER0] !=
						get_newsb(section_count)) {
			MSG(0, "\tError: user0 size %u, expect %u!!!\n",
					mp.partion_size[PARTITION_TYPE_USER0],
					get_newsb(section_count));
			return -1;
		}
		/* Make sure there is only one 2M-mapping region. */
		if (mp.partion_start[PARTITION_TYPE_USER1] != 0 ||
				mp.partion_size[PARTITION_TYPE_USER1] != 0) {
			MSG(0, "\tError: user1 %u-%u, expect 0-0!!!\n",
					mp.partion_start[PARTITION_TYPE_USER1],
					mp.partion_size[PARTITION_TYPE_USER1]);
			return -1;
		}
	}

	err = set_device_segment_mapping_pos(&mp);
	if (err < 0) {
		MSG(0, "\tError: Failed to set device segment mapping start pos!!!\n");
		return err;
	}

	/*
	 * Update super block after remapping expanded main area, in case of
	 * accidental power cut.
	 */
	set_newsb(magic, HMFS_SUPER_MAGIC);
	update_superblock(new_sb, SB_MASK_ALL);

	c.dbg_lv = 1;
	MSG(0, "after f2fs_resize_grow_safe\n");
	print_raw_sb_info(new_sb);
	print_ckpt_info(sbi);
	c.dbg_lv = 0;

	return err;
}

static int f2fs_resize_grow(struct f2fs_sb_info *sbi)
{
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	struct f2fs_super_block new_sb_raw;
	struct f2fs_super_block *new_sb = &new_sb_raw;
	block_t start_migrate_blk, new_main_blkaddr;
	struct stor_dev_mapping_partition mp;
	unsigned int old_segment_count_main, new_free_segment_count;
	struct sit_info *sit_i = SM_I(sbi)->sit_info;
	int err = 0;

	/* flush NAT/SIT journal entries */
	set_write_stream_id(META_STREAM_ID, sbi->sm_info->seg0_blkaddr);
	flush_journal_entries(sbi);
	ASSERT(f2fs_fsync_device() >= 0);

	memcpy(new_sb, F2FS_RAW_SUPER(sbi), sizeof(*new_sb));
	if (get_new_sb(sbi, new_sb))
		return -1;

	/* check nat availability */
	if (get_sb(segment_count_nat) > get_newsb(segment_count_nat)) {
		err = shrink_nats(sbi, new_sb);
		if (err) {
			MSG(0, "\tError: Failed to shrink NATs\n");
			return err;
		}
	}

	err = build_new_sit_sentries(sbi, get_newsb(segment_count_main));
	if (err) {
		return err;
	}

	err = build_new_sum_blocks(sbi, get_newsb(segment_count_main));
	if (err) {
		goto err_sum;
	}

	c.dbg_lv = 1;
	print_ckpt_info(sbi);
	c.dbg_lv = 0;

	start_migrate_blk = (get_sb(segment_count_main) <<
			get_sb(log_blocks_per_seg)) + get_sb(main_blkaddr);
	new_main_blkaddr = get_newsb(main_blkaddr);
	if (start_migrate_blk < new_main_blkaddr) {
		start_migrate_blk = new_main_blkaddr;
	}


	old_segment_count_main = get_sb(segment_count_main);

	sit_i->next_free_section = SEC_ALIGN(start_migrate_blk - new_main_blkaddr);

	MSG(0, "\tnew ssa_start blkaddr:%d\n", get_newsb(ssa_blkaddr));
	MSG(0, "\told ssa_blkaddr:%d\n", get_sb(ssa_blkaddr) + get_sb(segment_count_ssa));
	MSG(0, "\told main_blkaddr:%d\n", get_sb(main_blkaddr));

	new_free_segment_count =
		(get_newsb(section_count) - sit_i->next_free_section) *
		c.segs_per_sec;
	if (new_free_segment_count < old_segment_count_main) {
		MSG(0, "\tError: no enough space for migrating main\n");
		err = -EINVAL;
		goto err_out;
	}

	err = get_device_segment_mapping_pos(&mp);
	if (err < 0) {
		MSG(0, "\tError: Failed to get device segment mapping pos!!!\n");
		err = -EINVAL;
		goto err_out;
	}

	if (c.ndevs > 1) {
		MSG(0, "\tError: Not support multi-devices resizing!!!\n");
		err = -EINVAL;
		goto err_out;
	} else {
		unsigned int start_migrate_pos;

		start_migrate_pos = (start_migrate_blk + c.start_sector /
					DEFAULT_SECTORS_PER_BLOCK +
					c.blks_per_seg * c.segs_per_sec - 1) /
					c.blks_per_seg / c.segs_per_sec;
		mp.partion_start[PARTITION_TYPE_USER0] = start_migrate_pos;
		mp.partion_size[PARTITION_TYPE_USER0] =
			mp.partion_size[PARTITION_TYPE_META0] +
			mp.partion_size[PARTITION_TYPE_USER0] -
			start_migrate_pos;
		mp.partion_size[PARTITION_TYPE_META0] = start_migrate_pos;

		if(check_device_segment_meta1_mapping_pos(&mp)) {
			err = -EINVAL;
			goto err_out;
		}
		/* Make sure there is only one 2M-mapping region. */
		if (mp.partion_start[PARTITION_TYPE_USER1] != 0 ||
				mp.partion_size[PARTITION_TYPE_USER1] != 0) {
			MSG(0, "\tError: user1 %u-%u, expect 0-0!!!\n",
					mp.partion_start[PARTITION_TYPE_USER1],
					mp.partion_size[PARTITION_TYPE_USER1]);
			err = -EINVAL;
			goto err_out;
		}
	}

	err = set_device_segment_mapping_pos(&mp);
	if (err < 0) {
		MSG(0, "\tError: Failed to set device segment mapping start pos!!!\n");
		goto err_out;
	}

	err = device_ftl_reset();
	if (err < 0) {
		MSG(0, "\tError: Failed to reset device ftl !!!\n");
		goto err_out;
	}

	set_write_stream_id(META_STREAM_ID, get_sb(segment0_blkaddr));

	init_cur_seg(sbi, new_sb);

	/* move whole data region */
	migrate_main(sbi, new_sb);
	migrate_curseg(sbi, new_sb);

	set_write_stream_id(META_STREAM_ID, get_sb(segment0_blkaddr));
	migrate_ssa(sbi, new_sb);
	migrate_nat(sbi, new_sb);
	migrate_sit(sbi, new_sb, 0);
	rebuild_checkpoint(sbi, new_sb, 0);

	if (c.ndevs > 1) {
		MSG(0, "\tError: Not support multi-devices resizing!!!\n");
		err = -EINVAL;
		goto err_out;
	} else {
		unsigned int main_start_sec;

		main_start_sec = (get_newsb(main_blkaddr) + c.start_sector /
					DEFAULT_SECTORS_PER_BLOCK +
					c.blks_per_seg * c.segs_per_sec - 1) /
					c.blks_per_seg / c.segs_per_sec;
		mp.partion_start[PARTITION_TYPE_USER0] = main_start_sec;
		mp.partion_size[PARTITION_TYPE_USER0] =
				mp.partion_size[PARTITION_TYPE_META0] +
				mp.partion_size[PARTITION_TYPE_USER0] -
				main_start_sec;
		mp.partion_size[PARTITION_TYPE_META0] = main_start_sec;

		/* Make sure 2M-mapping region is consistent with main area */
		if (mp.partion_size[PARTITION_TYPE_USER0] !=
						get_newsb(section_count)) {
			err = -EINVAL;
			MSG(0, "\tError: user0 size %u, expect %u!!!\n",
					mp.partion_size[PARTITION_TYPE_USER0],
					get_newsb(section_count));
			goto err_out;
		}
		if(check_device_segment_meta1_mapping_pos(&mp)) {
			err = -EINVAL;
			goto err_out;
		}
	}

	err = set_device_segment_mapping_pos(&mp);
	if (err < 0) {
		MSG(0, "\tError: Failed to set device segment mapping start pos!!!\n");
		goto err_out;
	}

	/*
	 * Update super block after remapping expanded main area, in case of
	 * accidental power cut.
	 */
	set_newsb(magic, HMFS_SUPER_MAGIC);
	update_superblock(new_sb, SB_MASK_ALL);

	c.dbg_lv = 1;
	print_raw_sb_info(sb);
	print_raw_sb_info(new_sb);
	print_ckpt_info(sbi);
	c.dbg_lv = 0;

err_out:
	free(sbi->sum_blks);
err_sum:
	free_new_sit_sentries(sbi);

	return err;
}

int f2fs_resize(struct f2fs_sb_info *sbi)
{
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);

	/* may different sector size */
	if ((c.target_sectors * c.sector_size >>
			get_sb(log_blocksize)) < get_sb(block_count))
		if (!c.safe_resize) {
			ASSERT_MSG("Nothing to resize, now only supports resizing with safe resize flag\n");
			return -1;
		} else {
			ASSERT_MSG("not support shrink now!\n");
			return -1;
		}
	else if ((c.target_sectors * c.sector_size >>
			get_sb(log_blocksize)) > get_sb(block_count)) {
		if (c.safe_resize || get_sb(segment_count) / c.segs_per_sec >
					c.total_sectors / c.sectors_per_blk /
					c.blks_per_seg / c.segs_per_sec / 2) {
			/* Do safe resize when fs size exceeds 1/2 dev size */
			return f2fs_resize_grow_safe(sbi);
		} else if (c.target_sectors < c.total_sectors) {
			ASSERT_MSG("safe resize is needed when target size"
						"belows total size!\n");
			return -1;
		} else {
			return f2fs_resize_grow(sbi);
		}
	} else {
		MSG(0, "Nothing to resize.\n");
		return -1;
	}
}
