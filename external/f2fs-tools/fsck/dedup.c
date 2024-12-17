/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * dedup.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "dedup.h"
#include "quotaio.h"

static void add_into_dedup_inner_list(struct f2fs_sb_info *sbi, nid_t nid,
						bool is_valid, u32 link_cnt)
{
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
	struct dedup_inner_node *node = NULL, *tmp = NULL, *prev = NULL;

	node = calloc(sizeof(struct dedup_inner_node), 1);
	ASSERT(node != NULL);

	node->nid = nid;
	node->links = link_cnt;
	node->actual_links = 0;
	node->is_valid = is_valid;
	node->next = NULL;

	if (fsck->dedup_inner_list_head == NULL) {
		fsck->dedup_inner_list_head = node;
		goto out;
	}

	tmp = fsck->dedup_inner_list_head;

	/* Find insertion position */
	while (tmp && (nid < tmp->nid)) {
		ASSERT(tmp->nid != nid);
		prev = tmp;
		tmp = tmp->next;
	}

	if (tmp == fsck->dedup_inner_list_head) {
		node->next = tmp;
		fsck->dedup_inner_list_head = node;
	} else {
		prev->next = node;
		node->next = tmp;
	}
out:
	DBG(2, "inner ino[0x%x] has dedup links [0x%x]\n", nid, link_cnt);
}

static bool inode_dedup_feature_check(struct f2fs_node *node_blk)
{
	if (c.feature & cpu_to_le32(F2FS_FEATURE_DEDUP) &&
		f2fs_has_extra_isize(&node_blk->i) &&
		F2FS_FITS_IN_INODE(le16_to_cpu(node_blk->i.i_extra_isize),
			i_inner_ino, sizeof(node_blk->i.i_extra_isize))) {
		return true;
	}
	return false;
}

bool f2fs_is_deduped_inode(struct f2fs_node *node_blk)
{
	if (node_blk->footer.nid == node_blk->footer.ino &&
		inode_dedup_feature_check(node_blk) &&
		node_blk->i.i_dedup_flags & F2FS_DEDUPED_FL) {
		return true;
	}
	return false;
}

bool f2fs_is_inner_inode(struct f2fs_node *node_blk)
{
	if (node_blk->footer.nid == node_blk->footer.ino &&
		inode_dedup_feature_check(node_blk) &&
		node_blk->i.i_dedup_flags & F2FS_INNER_FL) {
		return true;
	}
	return false;
}

bool f2fs_is_out_inode(struct f2fs_node *node_blk)
{
	if (f2fs_is_deduped_inode(node_blk) &&
			!f2fs_is_inner_inode(node_blk)) {
		return true;
	}
	return false;
}

static bool f2fs_is_revoke_inode(struct f2fs_node *node_blk)
{
	if (f2fs_is_deduped_inode(node_blk) &&
		node_blk->i.i_dedup_flags & F2FS_REVOKE_FL) {
		return true;
	}
	return false;
}

static bool f2fs_is_deduping_inode(struct f2fs_node *node_blk)
{
	if (f2fs_is_deduped_inode(node_blk) &&
		node_blk->i.i_dedup_flags & F2FS_DOING_DEDUP_FL) {
		return true;
	}
	return false;
}

bool f2fs_is_unstable_dedup_inode(struct f2fs_node *node_blk)
{
	if (f2fs_is_revoke_inode(node_blk) ||
		f2fs_is_deduping_inode(node_blk)) {
		return true;
	}
	return false;
}

nid_t f2fs_get_dedup_inner_ino(struct f2fs_node *node_blk)
{
	return node_blk->i.i_inner_ino;
}

static struct dedup_inner_node *find_dedup_inner_node(struct f2fs_sb_info *sbi, nid_t nid)
{
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
	struct dedup_inner_node *node = NULL;

	if (fsck->dedup_inner_list_head == NULL) {
		return NULL;
	}

	node = fsck->dedup_inner_list_head;

	while (node && (nid < node->nid)) {
		node = node->next;
	}

	if (node == NULL || (nid != node->nid)) {
		return NULL;
	}

	return node;
}

void f2fs_inc_inner_actual_links(struct f2fs_sb_info *sbi, nid_t inner_ino)
{
	struct dedup_inner_node *node = find_dedup_inner_node(sbi, inner_ino);

	if (node != NULL) {
		node->actual_links++;
	}
}

bool f2fs_sanity_check_dedup_inner_nid(struct f2fs_sb_info *sbi, nid_t inner_ino)
{
	struct dedup_inner_node *dedup_inner_node = NULL;
	u32 i_links;
	bool is_inner_inode_valid = true;
	struct node_info ni;
	struct f2fs_node *node_blk = NULL;
	u32 blk_cnt;
	struct f2fs_compr_blk_cnt cbc;

	if (!IS_VALID_NID(sbi, inner_ino)) {
		return false;
	}

	dedup_inner_node = find_dedup_inner_node(sbi, inner_ino);
	/* have already checked, only need check once */
	if (dedup_inner_node) {
		return dedup_inner_node->is_valid;
	}

	node_blk = (struct f2fs_node *)calloc(BLOCK_SZ, 1);
	ASSERT(node_blk != NULL);

	if (sanity_check_nid(sbi, inner_ino, node_blk, F2FS_FT_DEDUP_INNER, TYPE_INODE, &ni)) {
		is_inner_inode_valid = false;
		i_links = 0;
		goto out;
	}

	blk_cnt = 1;
	cbc.cnt = 0;
	cbc.cheader_pgofs = CHEADER_PGOFS_NONE;
	fsck_chk_inode_blk(sbi, inner_ino, F2FS_FT_DEDUP_INNER, node_blk, &blk_cnt, &cbc, &ni, NULL);
	i_links = le32_to_cpu(node_blk->i.i_links);
out:
	add_into_dedup_inner_list(sbi, inner_ino, is_inner_inode_valid, i_links);
	free(node_blk);
	return is_inner_inode_valid;
}

static void drop_node_blk(struct f2fs_sb_info *sbi, nid_t nid, enum NODE_TYPE ntype);
static void drop_inode_blk(struct f2fs_sb_info *sbi, struct f2fs_node *node_blk)
{
	unsigned int idx = 0;
	int ofs;
	block_t blkaddr;
	nid_t i_nid;
	enum NODE_TYPE ntype;
	struct node_info xattr_ni;
	nid_t x_nid = le32_to_cpu(node_blk->i.i_xattr_nid);
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);

	/* drop xattr addr */
	if (x_nid != 0 && IS_VALID_NID(sbi, x_nid)) {
		get_node_info(sbi, x_nid, &xattr_ni);

		if (f2fs_test_main_bitmap(sbi, xattr_ni.blk_addr) != 0) {
			f2fs_clear_main_bitmap(sbi, xattr_ni.blk_addr);
			fsck->chk.valid_blk_cnt--;
			fsck->chk.valid_node_cnt--;
		}
	}

	ofs = get_extra_isize(node_blk);
	/* drop data blocks in inode */
	for (idx = 0; idx < ADDRS_PER_INODE(&node_blk->i); idx++) {
		blkaddr = le32_to_cpu(node_blk->i.i_addr[ofs + idx]);

		if (IS_VALID_BLK_ADDR(sbi, blkaddr)) {
			if (blkaddr == NULL_ADDR) {
				continue;
			}

			if (blkaddr == NEW_ADDR) {
				fsck->chk.valid_blk_cnt--;
			} else {
				if (f2fs_test_main_bitmap(sbi, blkaddr) != 0) {
					f2fs_clear_main_bitmap(sbi, blkaddr);
					fsck->chk.valid_blk_cnt--;
				}
			}
		}
	}

	/* drop node blocks in inode */
	for (idx = 0; idx < 5; idx++) {
		i_nid = le32_to_cpu(node_blk->i.i_nid[idx]);

		if (idx == 0 || idx == 1) {
			ntype = TYPE_DIRECT_NODE;
		} else if (idx == 2 || idx == 3) {
			ntype = TYPE_INDIRECT_NODE;
		} else if (idx == 4) {
			ntype = TYPE_DOUBLE_INDIRECT_NODE;
		} else {
			ASSERT(0);
		}

		if (i_nid == 0x0) {
			continue;
		}
		drop_node_blk(sbi, i_nid, ntype);
	}
}

static void drop_dnode_blk(struct f2fs_sb_info *sbi, struct f2fs_node *node_blk)
{
	int idx;
	block_t blkaddr;
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);

	for (idx = 0; idx < ADDRS_PER_BLOCK(&node_blk->i); idx++) {
		blkaddr = le32_to_cpu(node_blk->dn.addr[idx]);

		if (IS_VALID_BLK_ADDR(sbi, blkaddr)) {
			if (blkaddr == NULL_ADDR) {
				continue;
			}

			if (blkaddr == NEW_ADDR) {
				fsck->chk.valid_blk_cnt--;
			} else {
				if (f2fs_test_main_bitmap(sbi, blkaddr) != 0) {
					f2fs_clear_main_bitmap(sbi, blkaddr);
					fsck->chk.valid_blk_cnt--;
				}
			}
		}
	}
}

static void drop_idnode_blk(struct f2fs_sb_info *sbi, struct f2fs_node *node_blk)
{
	int i = 0;
	nid_t nid;

	for (i = 0; i < NIDS_PER_BLOCK; i++) {
		nid = le32_to_cpu(node_blk->in.nid[i]);
		drop_node_blk(sbi, nid, TYPE_DIRECT_NODE);
	}
}

static void drop_didnode_blk(struct f2fs_sb_info *sbi, struct f2fs_node *node_blk)
{
	int i = 0;
	nid_t nid;

	for (i = 0; i < NIDS_PER_BLOCK; i++) {
		nid = le32_to_cpu(node_blk->in.nid[i]);
		drop_node_blk(sbi, nid, TYPE_INDIRECT_NODE);
	}
}

static void drop_node_blk(struct f2fs_sb_info *sbi, nid_t nid, enum NODE_TYPE ntype)
{
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
	struct node_info ni;
	struct f2fs_node *node_blk = NULL;
	int ret;

	if (nid == 0 || !IS_VALID_NID(sbi, nid)) {
		return;
	}

	get_node_info(sbi, nid, &ni);
	node_blk = (struct f2fs_node *)calloc(BLOCK_SZ, 1);
	ASSERT(node_blk != NULL);

	ret = dev_read_block(node_blk, ni.blk_addr);
	ASSERT(ret >= 0);

	if (f2fs_test_main_bitmap(sbi, ni.blk_addr) != 0) {
		f2fs_clear_main_bitmap(sbi, ni.blk_addr);
		fsck->chk.valid_blk_cnt--;
		fsck->chk.valid_node_cnt--;

		if (ntype == TYPE_INODE) {
			fsck->chk.valid_inode_cnt--;
		}
	}

	if (ntype == TYPE_INODE) {
		drop_inode_blk(sbi, node_blk);
	} else {
		switch (ntype) {
			case TYPE_DIRECT_NODE:
				drop_dnode_blk(sbi, node_blk);
				break;
			case TYPE_INDIRECT_NODE:
				drop_idnode_blk(sbi, node_blk);
				break;
			case TYPE_DOUBLE_INDIRECT_NODE:
				drop_didnode_blk(sbi, node_blk);
				break;
			default:
				ASSERT(0);
		}
	}
	free(node_blk);
}

void f2fs_fix_dedup_inner_list(struct f2fs_sb_info *sbi)
{
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
	struct dedup_inner_node *tmp, *node;
	struct f2fs_node *node_blk = NULL;
	struct node_info ni;
	int ret;

	if (fsck->dedup_inner_list_head == NULL) {
		return;
	}
	node = fsck->dedup_inner_list_head;

	node_blk = (struct f2fs_node *)calloc(BLOCK_SZ, 1);
	ASSERT(node_blk != NULL);
	while (node) {
		if (node->is_valid) {
			get_node_info(sbi, node->nid, &ni);
			if (node->actual_links == 0) {
				ASSERT_MSG("Inner inode: 0x%x i_links= 0x%x -> 0x%x",
					node->nid, node->links, node->actual_links);
				drop_node_blk(sbi, node->nid, TYPE_INODE);
			} else {
				ret = dev_read_block(node_blk, ni.blk_addr);
				ASSERT(ret >= 0);
				if (node->links != node->actual_links && c.fix_on &&
					f2fs_dev_is_writable()) {
					node_blk->i.i_links = cpu_to_le32(node->actual_links);
					FIX_MSG("Inner inode: 0x%x i_links= 0x%x -> 0x%x",
						node->nid, node->links, node->actual_links);

					ret = dev_write_block(node_blk, ni.blk_addr);
					ASSERT(ret >= 0);
				}
				quota_add_inode_usage(fsck->qctx, node->nid, &node_blk->i);
			}
		}
		tmp = node;
		node = node->next;
		free(tmp);
	}
	free(node_blk);
}

void f2fs_check_dedup_extent_info(struct child_info *child)
{
	struct extent_info *ei = &child->ei;

	if (!ei->len) {
		return;
	}
	child->state |= FSCK_UNMATCHED_EXTENT;
}
