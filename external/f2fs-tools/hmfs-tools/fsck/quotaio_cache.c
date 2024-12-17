/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * quotaio_cache.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "quotaio_cache.h"
#include <stdbool.h>

static struct tree_node *create_tree_node(nid_t nid)
{
	struct tree_node *tnode = (struct tree_node *)malloc(sizeof(struct tree_node));
	ASSERT(tnode);
	tnode->nc = (struct node_cache *)malloc(sizeof(struct node_cache));
	ASSERT(tnode->nc);
	tnode->nc->blk_buffer = (struct f2fs_node *)malloc(BLOCK_SZ);
	ASSERT(tnode->nc->blk_buffer);
	tnode->nc->nid = nid;
	tnode->nc->node_blkaddr = NULL_ADDR;
	tnode->nc->dirty = 0;
	tnode->left = tnode->right = NULL;

	return tnode;
}

static struct data_list *create_data_list(pgoff_t index)
{
	struct data_list *data_list = (struct data_list *)malloc(sizeof(struct data_list));
	ASSERT(data_list);
	data_list->dc = (struct data_cache *)malloc(sizeof(struct data_cache));
	ASSERT(data_list->dc);
	data_list->dc->blk_buffer = (char *)malloc(BLOCK_SZ);
	ASSERT(data_list->dc->blk_buffer);

	data_list->dc->index = index;
	data_list->dc->dirty = 0;
	data_list->next = NULL;

	return data_list;
}

static struct tree_node *search_tree_node(struct tree_node *root, nid_t nid)
{
	if (root != NULL) {
		if (root->nc->nid == nid)
			return root;
		else if (root->nc->nid > nid)
			return search_tree_node(root->left, nid);
		else
			return search_tree_node(root->right, nid);
	} else {
		return NULL;
	}
}

static struct data_list *search_data_list(struct data_list *head, pgoff_t index)
{
	struct data_list *dl = head;

	while (dl) {
		if (dl->dc->index == index) {
			return dl;
		} else if (dl->dc->index > index) {
			dl = dl->next;
		} else {
			return NULL;
		}
	}
	return NULL;
}

static struct tree_node *add_tree_node(struct tree_node **root, nid_t nid)
{
	struct tree_node *tnode = *root;

	if (tnode == NULL) {
		tnode = create_tree_node(nid);
		*root = tnode;
		return tnode;
	}
	if (nid == tnode->nc->nid)
		return tnode;
	if (nid < tnode->nc->nid) {
		return add_tree_node(&tnode->left, nid);
	} else {
		return add_tree_node(&tnode->right, nid);
	}
}

static struct data_list *add_data_list(struct data_list **head, pgoff_t index)
{
	struct data_list *pre = NULL;
	struct data_list *data_list_new = NULL;
	struct data_list *cur = *head;

	if (cur == NULL) {
		cur = create_data_list(index);
		*head = cur;
		return cur;
	}

	while((cur->dc->index > index) && (cur->next != NULL)) {
		pre = cur;
		cur = cur->next;
	}

	if (cur->dc->index == index)
		return cur;

	data_list_new = create_data_list(index);
	if (cur->dc->index < index) {
		if (cur == *head) {
			data_list_new->next = cur;
			*head = data_list_new;
		} else {
			pre->next = data_list_new;
			data_list_new->next = cur;
		}
	} else {
		cur->next = data_list_new;
		data_list_new->next = NULL;
	}
	return data_list_new;
}

static void destroy_tree_node(struct tree_node *root)
{
	if (root != NULL) {
		destroy_tree_node(root->left);
		destroy_tree_node(root->right);

		free(root->nc->blk_buffer);
		free(root->nc);
		free(root);
		root = NULL;
	}
}

static void destroy_tree_data(struct data_list *head)
{
	struct data_list *cur = head;
	struct data_list *next = NULL;

	while (cur != NULL) {
		next = cur->next;
		free(cur->dc->blk_buffer);
		free(cur->dc);
		free(cur);
		cur = next;
	}
}

static inline void add_inode_blocks()
{
	struct tree_node *tnode = NULL;
	u64 blocks;

	tnode = search_tree_node(c.tree_node_root, c.cached_ino);
	ASSERT(tnode);
	blocks = le64_to_cpu(tnode->nc->blk_buffer->i.i_blocks);
	tnode->nc->blk_buffer->i.i_blocks = cpu_to_le64(blocks + 1);
	tnode->nc->dirty = 1;
}

static inline void set_inode_dirty()
{
	struct tree_node *tnode = NULL;

	tnode = search_tree_node(c.tree_node_root, c.cached_ino);
	ASSERT(tnode);
	tnode->nc->dirty = 1;
}

static void reset_data_blkaddr(struct data_cache *dc)
{
	__le32 *addr_array = NULL;
	struct f2fs_node *node_blk = NULL;
	unsigned int ofs_in_node = dc->ofs_in_node;
	struct tree_node *tnode = NULL;

	tnode = search_tree_node(c.tree_node_root, dc->nid);
	ASSERT(tnode);
	node_blk = tnode->nc->blk_buffer;
	addr_array = blkaddr_in_node(node_blk);
	addr_array[ofs_in_node] = cpu_to_le32(dc->data_blkaddr);
	tnode->nc->dirty = 1;
}

static void check_inode_ext(block_t blk_addr)
{
	struct tree_node *tnode = NULL;
	struct f2fs_node *inode_blk = NULL;
	block_t startaddr, endaddr;

	tnode = search_tree_node(c.tree_node_root, c.cached_ino);
	ASSERT(tnode);
	inode_blk = tnode->nc->blk_buffer;

	startaddr = le32_to_cpu(inode_blk->i.i_ext.blk_addr);
	endaddr = startaddr + le32_to_cpu(inode_blk->i.i_ext.len);
	if (blk_addr >= startaddr && blk_addr < endaddr) {
		inode_blk->i.i_ext.len = 0;
		tnode->nc->dirty = 1;
	}
}

static void block_write_to_cache(struct f2fs_sb_info *sbi, void *buf, int type)
{
	struct curseg_info *curseg = CURSEG_I(sbi, type);
	char *addr = curseg->seg_cache;

	if (!curseg->seg_cache) {
		return;
	} else {
		addr += curseg->cached_size;
		memcpy(addr, buf, F2FS_BLKSIZE);
		curseg->cached_size += F2FS_BLKSIZE;
		ASSERT(curseg->cached_size <= curseg->max_cache_size);
	}
}

static block_t __next_free_blkaddr(struct f2fs_super_block *sb,
					struct curseg_info *curseg)
{
	return get_sb(main_blkaddr) +
		(curseg->segno << get_sb(log_blocks_per_seg)) +
		curseg->next_blkoff;
}

static void reset_curseg_cache(struct f2fs_sb_info *sbi, int type)
{
	struct curseg_info *curseg = CURSEG_I(sbi, type);
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);

	curseg->cached_size = 0;
	curseg->cached_start_addr = __next_free_blkaddr(sb, curseg);
}

static int hmfs_write_data_page(struct f2fs_sb_info *sbi, struct data_cache *dc)
{
	int ret;
	struct f2fs_summary sum;
	struct node_info ni;
	struct tree_node *tnode = search_tree_node(c.tree_node_root, dc->nid);
	struct node_cache *node_cache = tnode->nc;
	unsigned int blkaddr;
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);
	block_t blk_addr = dc->data_blkaddr;
	int type = CURSEG_HOT_DATA;

	if (blk_addr == NULL_ADDR || blk_addr == NEW_ADDR) {
		if (!is_set_ckpt_flags(cp, CP_UMOUNT_FLAG)) {
			c.alloc_failed = 1;
			return -EINVAL;
		}

		get_node_info(sbi, dc->nid, &ni);
		set_summary(&sum, dc->nid, dc->ofs_in_node, ni.version);
		blkaddr = datablock_addr(node_cache->blk_buffer, dc->ofs_in_node);
		block_write_to_cache(sbi, dc->blk_buffer, type);
		ret = reserve_new_block(sbi, &dc->data_blkaddr, &sum, type, 0);
		if (ret) {
			c.alloc_failed = 1;
			return ret;
		}

		if (blkaddr == NULL_ADDR)
			add_inode_blocks();
		else if (blkaddr == NEW_ADDR)
			set_inode_dirty();
		reset_data_blkaddr(dc);
	} else {
		set_block_free(sbi, blk_addr);

		/* read/write SSA */
		get_sum_entry(sbi, blk_addr, &sum);
		block_write_to_cache(sbi, dc->blk_buffer, type);
		ret = reserve_new_block(sbi, &dc->data_blkaddr, &sum, type, 0);
		if (ret) {
			c.alloc_failed = 1;
			return ret;
		}

		reset_data_blkaddr(dc);
		check_inode_ext(blk_addr);
	}
	return ret;
}

static int hmfs_write_node_page(struct f2fs_sb_info *sbi, struct node_cache *nc)
{
	int ret;
	int type;
	nid_t nid;
	nid_t ino;
	struct f2fs_summary sum;
	struct node_info ni;
	block_t blk_addr = nc->node_blkaddr;
	block_t to;
	struct f2fs_node *node_blk = nc->blk_buffer;

	if (blk_addr == NULL_ADDR || blk_addr == NEW_ADDR) {
		nid = le32_to_cpu(node_blk->footer.nid);
		ino = le32_to_cpu(node_blk->footer.ino);

		type = CURSEG_COLD_NODE;
		if (IS_DNODE(node_blk)) {
			type = CURSEG_HOT_NODE;
		}
		get_node_info(sbi, nid, &ni);
		set_summary(&sum, nid, 0, ni.version);
		block_write_to_cache(sbi, nc->blk_buffer, type);
		ret = reserve_new_block(sbi, &blk_addr, &sum, type,
					IS_INODE(node_blk));
		if (ret) {
			c.alloc_failed = 1;
			return ret;
		}

		/* update nat info */
		update_nat_blkaddr(sbi, ino, nid, blk_addr);
	} else {
		ASSERT(blk_addr >= SM_I(sbi)->main_blkaddr);

		/* update sit bitmap & valid_blocks && se->type */
		type = set_block_free(sbi, blk_addr);

		/* read/write SSA */
		get_sum_entry(sbi, blk_addr, &sum);
		to = blk_addr;
		block_write_to_cache(sbi, nc->blk_buffer, type);
		ret = reserve_new_block(sbi, &to, &sum, type,
					IS_INODE(node_blk));
		if (ret) {
			c.alloc_failed = 1;
			return ret;
		}

		update_nat_blkaddr(sbi, 0, le32_to_cpu(sum.nid), to);
	}
	return ret;
}

static void hmfs_write_cached_node_pages(struct f2fs_sb_info *sbi, struct tree_node *root)
{
	if (root != NULL) {
		hmfs_write_cached_node_pages(sbi, root->left);
		if (root->nc->nid != c.cached_ino && root->nc->dirty) {
			hmfs_write_node_page(sbi, root->nc);
		}
		hmfs_write_cached_node_pages(sbi, root->right);
	}
}

static void hmfs_write_inode(struct f2fs_sb_info *sbi)
{
	struct tree_node *tnode = NULL;

	tnode = search_tree_node(c.tree_node_root, c.cached_ino);
	ASSERT(tnode);

	if (tnode->nc->dirty) {
		hmfs_write_node_page(sbi, tnode->nc);
	}
}

static void hmfs_write_cached_data_pages(struct f2fs_sb_info *sbi, struct data_list *head)
{
	struct data_list *cur = head;

	while (cur != NULL) {
		if (cur->dc->dirty) {
			hmfs_write_data_page(sbi, cur->dc);
		}
		cur = cur->next;
	}
}

int hmfs_grab_node_from_cache(struct f2fs_node *buf, nid_t nid, block_t blk_addr)
{
	int ret;
	struct tree_node *tnode = search_tree_node(c.tree_node_root, nid);

	if (tnode != NULL) {
		memcpy(buf, tnode->nc->blk_buffer, BLOCK_SZ);
		ret = BLOCK_SZ;
	} else {
		ret = dev_read_block(buf, blk_addr);
		ASSERT(ret >= 0);
		tnode = add_tree_node(&c.tree_node_root, nid);
		memcpy(tnode->nc->blk_buffer, buf, BLOCK_SZ);
		tnode->nc->node_blkaddr = blk_addr;
	}
	return ret;
}

int hmfs_write_node_to_cache(struct f2fs_node *node_blk, block_t blk_addr)
{
	nid_t nid = le32_to_cpu(node_blk->footer.nid);
	struct tree_node *tnode = search_tree_node(c.tree_node_root, nid);

	if (tnode == NULL) {
		tnode = add_tree_node(&c.tree_node_root, nid);
	}

	memcpy(tnode->nc->blk_buffer, node_blk, BLOCK_SZ);
	tnode->nc->node_blkaddr = blk_addr;
	tnode->nc->dirty = 1;

	return BLOCK_SZ;
}

void hmfs_grab_data_from_cache(void *blk_buf, pgoff_t index, block_t blk_addr)
{
	int ret = 0;
	struct data_list *dl = search_data_list(c.data_list_head, index);

	if (dl != NULL) {
		memcpy(blk_buf, dl->dc->blk_buffer, BLOCK_SZ);
	} else if (blk_addr != NULL_ADDR && blk_addr != NEW_ADDR) {
		ret = dev_read_block(blk_buf, blk_addr);
		ASSERT(ret >= 0);
		dl = add_data_list(&c.data_list_head, index);
		memcpy(dl->dc->blk_buffer, blk_buf, BLOCK_SZ);
		dl->dc->data_blkaddr = blk_addr;
	}
}

void hmfs_write_data_to_cache(char *blk_buf, pgoff_t index,
				struct dnode_of_data *dn, block_t blk_addr)
{
	struct data_list *dl = search_data_list(c.data_list_head, index);

	if (dl == NULL)
		dl = add_data_list(&c.data_list_head, index);

	memcpy(dl->dc->blk_buffer, blk_buf, BLOCK_SZ);
	dl->dc->data_blkaddr = blk_addr;
	dl->dc->nid = dn->nid;
	dl->dc->ofs_in_node = dn->ofs_in_node;
	dl->dc->dirty = 1;

	if (c.cached_ino == dn->nid)
		dn->idirty = 1;
	else
		dn->ndirty = 1;
}

static void init_curseg_cache(struct f2fs_sb_info *sbi, int type)
{
	struct curseg_info *curseg = CURSEG_I(sbi, type);
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);

	curseg->max_cache_size = sbi->blocks_per_seg * F2FS_BLKSIZE;
	curseg->cached_size = 0;
	curseg->cached_start_addr = __next_free_blkaddr(sb, curseg);
	curseg->seg_cache = calloc(curseg->max_cache_size, 1);
	ASSERT(curseg->seg_cache);
}

static void clean_curseg_cache(struct f2fs_sb_info *sbi, int type)
{
	struct curseg_info *curseg = CURSEG_I(sbi, type);

	curseg->cached_size = 0;
	free(curseg->seg_cache);
}

int flush_curseg_cached_blocks(struct f2fs_sb_info *sbi, int type)
{
	int ret;
	struct curseg_info *curseg = CURSEG_I(sbi, type);
	__u64 head_addr = curseg->cached_start_addr;
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);

	if (!curseg->seg_cache || curseg->cached_size == 0)
		return 0;

	MSG(0, "Info: flush quota cache: head:%x, size:%d, type:%d\n",
			head_addr, curseg->cached_size, type);
	ret = dev_write(curseg->seg_cache, head_addr << F2FS_BLKSIZE_BITS,
				curseg->cached_size, 1, 1, get_stream_id(type));

	curseg->cached_size = 0;
	curseg->cached_start_addr = __next_free_blkaddr(sb, curseg);
	memset(curseg->seg_cache, 0, curseg->max_cache_size);

	return ret;
}

static unsigned int get_required_blocks(struct data_list *head)
{
	unsigned int dirty_pages = 0;
	struct data_list *cur = head;

	while (cur != NULL) {
		if (cur->dc->dirty) {
			dirty_pages++;
		}
		cur = cur->next;
	}
	return dirty_pages;
}

static unsigned int get_free_block_count(struct f2fs_sb_info *sbi, int type)
{
	struct curseg_info *curseg = NULL;
	unsigned int cur_secno;
	u32 start;
	u32 end;

	curseg = CURSEG_I(sbi, type);
	cur_secno = GET_SEC_FROM_SEG(sbi, curseg->segno);
	start = START_BLOCK(sbi, curseg->segno) + curseg->next_blkoff;
	end = START_BLOCK(sbi, GET_SEG_FROM_SEC(sbi, cur_secno + 1));

	return end - start;
}

static bool has_not_enough_free_blocks(struct f2fs_sb_info *sbi,
				unsigned int required_blocks)
{
	struct free_segmap_info *free_i = FREE_I(sbi);
	unsigned int free_blocks;
	unsigned int min_free_sec = min_fsck_free_section(sbi);


	if (free_i->free_sections < min_free_sec)
		return true;

	free_blocks = ((free_i->free_sections - min_free_sec) *
		BLKS_PER_SEC(sbi)) + get_free_block_count(sbi, CURSEG_HOT_DATA);

	if (free_blocks < required_blocks)
		return true;
	else
		return false;
}

static int free_enough_blocks(struct f2fs_sb_info *sbi, struct data_list *head)
{
	int i = 0;
	unsigned int required_blocks = get_required_blocks(head);

check_free_secs:
	/*
	 * One more section may not be generated in one GC, so we
	 * need to do gc many times until we get enough free blocks.
	 * If gc times are more than MAX_GC_TIMES, there may be some error.
	 */
	if (has_not_enough_free_blocks(sbi, required_blocks)) {
		fsck_gc(sbi);
		i++;
		if (i > MAX_GC_TIMES) {
			DBG(0, "[fsck] gc times more then %d\n", MAX_GC_TIMES);
			return 1;
		}
		goto check_free_secs;
	}
	return 0;
}

void hmfs_quota_cache_write_back(struct f2fs_sb_info *sbi)
{
	unsigned int required_blocks = get_required_blocks(c.data_list_head);
	if (has_not_enough_free_blocks(sbi, required_blocks)) {
		MSG(0, "Info: has not enough free blocks, quota fixing aborted.");
		return;
	}

	c.do_gc = 1;
	init_curseg_cache(sbi, CURSEG_HOT_DATA);
	init_curseg_cache(sbi, CURSEG_COLD_DATA);
	hmfs_write_cached_data_pages(sbi, c.data_list_head);
	flush_curseg_cached_blocks(sbi, CURSEG_HOT_DATA);
	flush_curseg_cached_blocks(sbi, CURSEG_COLD_DATA);
	clean_curseg_cache(sbi, CURSEG_HOT_DATA);
	clean_curseg_cache(sbi, CURSEG_COLD_DATA);

	init_curseg_cache(sbi, CURSEG_HOT_NODE);
	init_curseg_cache(sbi, CURSEG_COLD_NODE);
	hmfs_write_cached_node_pages(sbi, c.tree_node_root);
	hmfs_write_inode(sbi);
	flush_curseg_cached_blocks(sbi, CURSEG_HOT_NODE);
	flush_curseg_cached_blocks(sbi, CURSEG_COLD_NODE);
	clean_curseg_cache(sbi, CURSEG_HOT_NODE);
	clean_curseg_cache(sbi, CURSEG_COLD_NODE);

	c.do_gc = 0;
}

void hmfs_quota_cache_init(nid_t ino)
{
	c.cached_ino = ino;
	c.tree_node_root = NULL;
	c.data_list_head = NULL;
}

void hmfs_quota_cache_clean()
{
	c.cached_ino = 0;
	destroy_tree_node(c.tree_node_root);
	destroy_tree_data(c.data_list_head);
}
