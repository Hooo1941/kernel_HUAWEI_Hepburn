/**
 * mount.c
 *
 * Copyright (c) 2013 Samsung Electronics Co., Ltd.
 *             http://www.samsung.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include "fsck.h"
#include "xattr.h"
#include <locale.h>
#include <stdbool.h>
#include "quotaio_cache.h"
#ifdef HAVE_LINUX_POSIX_ACL_H
#include <linux/posix_acl.h>
#endif
#ifdef HAVE_SYS_ACL_H
#include <sys/acl.h>
#endif

#ifndef ACL_UNDEFINED_TAG
#define ACL_UNDEFINED_TAG	(0x00)
#define ACL_USER_OBJ		(0x01)
#define ACL_USER		(0x02)
#define ACL_GROUP_OBJ		(0x04)
#define ACL_GROUP		(0x08)
#define ACL_MASK		(0x10)
#define ACL_OTHER		(0x20)
#endif

u32 get_free_segments(struct f2fs_sb_info *sbi)
{
	u32 i, free_segs = 0;

	for (i = 0; i < TOTAL_SEGS(sbi); i++) {
		int devid = get_device_index(START_BLOCK(sbi, i));

		if (devid >= 0 && c.devices[devid].reserved)
			continue;

		struct seg_entry *se = get_seg_entry(sbi, i);
		if (se->valid_blocks == 0x0 && !IS_CUR_SEGNO(sbi, i) &&
				!IS_DM_SOURCE_SEGNO(sbi, i))
			free_segs++;
	}
	return free_segs;
}

void update_free_segments(struct f2fs_sb_info *sbi)
{
	char *progress = "-*|*-";
	static int i = 0;

	if (c.dbg_lv)
		return;

	MSG(0, "\r [ %c ] Free segments: 0x%x", progress[i % 5], get_free_segments(sbi));
	fflush(stdout);
	i++;
}

#if defined(HAVE_LINUX_POSIX_ACL_H) || defined(HAVE_SYS_ACL_H)
void print_acl(char *value, int size)
{
	struct f2fs_acl_header *hdr = (struct f2fs_acl_header *)value;
	struct f2fs_acl_entry *entry = (struct f2fs_acl_entry *)(hdr + 1);
	const char *end = value + size;
	int i, count;

	if (hdr->a_version != cpu_to_le32(F2FS_ACL_VERSION)) {
		MSG(0, "Invalid ACL version [0x%x : 0x%x]\n",
				le32_to_cpu(hdr->a_version), F2FS_ACL_VERSION);
		return;
	}

	count = f2fs_acl_count(size);
	if (count <= 0) {
		MSG(0, "Invalid ACL value size %d\n", size);
		return;
	}

	for (i = 0; i < count; i++) {
		if ((char *)entry > end) {
			MSG(0, "Invalid ACL entries count %d\n", count);
			return;
		}

		switch (le16_to_cpu(entry->e_tag)) {
		case ACL_USER_OBJ:
		case ACL_GROUP_OBJ:
		case ACL_MASK:
		case ACL_OTHER:
			MSG(0, "tag:0x%x perm:0x%x\n",
					le16_to_cpu(entry->e_tag),
					le16_to_cpu(entry->e_perm));
			entry = (struct f2fs_acl_entry *)((char *)entry +
					sizeof(struct f2fs_acl_entry_short));
			break;
		case ACL_USER:
			MSG(0, "tag:0x%x perm:0x%x uid:%u\n",
					le16_to_cpu(entry->e_tag),
					le16_to_cpu(entry->e_perm),
					le32_to_cpu(entry->e_id));
			entry = (struct f2fs_acl_entry *)((char *)entry +
					sizeof(struct f2fs_acl_entry));
			break;
		case ACL_GROUP:
			MSG(0, "tag:0x%x perm:0x%x gid:%u\n",
					le16_to_cpu(entry->e_tag),
					le16_to_cpu(entry->e_perm),
					le32_to_cpu(entry->e_id));
			entry = (struct f2fs_acl_entry *)((char *)entry +
					sizeof(struct f2fs_acl_entry));
			break;
		default:
			MSG(0, "Unknown ACL tag 0x%x\n",
					le16_to_cpu(entry->e_tag));
			return;
		}
	}
}
#else
#define print_acl(value, size) do {		\
	int i;					\
	for (i = 0; i < size; i++)		\
		MSG(0, "%02X", value[i]);	\
	MSG(0, "\n");				\
} while (0)
#endif

void print_xattr_entry(struct f2fs_xattr_entry *ent)
{
	char *value = (char *)(ent->e_name + le16_to_cpu(ent->e_name_len));
	struct fscrypt_context *ctx;
	int i;

	MSG(0, "\nxattr: e_name_index:%d e_name:", ent->e_name_index);
	for (i = 0; i < le16_to_cpu(ent->e_name_len); i++)
		MSG(0, "%c", ent->e_name[i]);
	MSG(0, " e_name_len:%d e_value_size:%d e_value:\n",
			ent->e_name_len, le16_to_cpu(ent->e_value_size));

	switch (ent->e_name_index) {
	case F2FS_XATTR_INDEX_POSIX_ACL_ACCESS:
	case F2FS_XATTR_INDEX_POSIX_ACL_DEFAULT:
		print_acl(value, le16_to_cpu(ent->e_value_size));
		break;
	case F2FS_XATTR_INDEX_USER:
	case F2FS_XATTR_INDEX_SECURITY:
	case F2FS_XATTR_INDEX_TRUSTED:
	case F2FS_XATTR_INDEX_LUSTRE:
		for (i = 0; i < le16_to_cpu(ent->e_value_size); i++)
			MSG(0, "%02X", value[i]);
		MSG(0, "\n");
		break;
	case F2FS_XATTR_INDEX_ENCRYPTION:
		ctx = (struct fscrypt_context *)value;
		MSG(0, "format: %d\n", ctx->format);
		MSG(0, "contents_encryption_mode: 0x%x\n", ctx->contents_encryption_mode);
		MSG(0, "filenames_encryption_mode: 0x%x\n", ctx->filenames_encryption_mode);
		MSG(0, "flags: 0x%x\n", ctx->flags);
		MSG(0, "master_key_descriptor: ");
		for (i = 0; i < FS_KEY_DESCRIPTOR_SIZE; i++)
			MSG(0, "%02X", ctx->master_key_descriptor[i]);
		MSG(0, "\nnonce: ");
		for (i = 0; i < FS_KEY_DERIVATION_NONCE_SIZE; i++)
			MSG(0, "%02X", ctx->nonce[i]);
		MSG(0, "\n");
		break;
	default:
		break;
	}
}

void print_inode_info(struct f2fs_sb_info *sbi,
			struct f2fs_node *node, int name)
{
	struct f2fs_inode *inode = &node->i;
	void *xattr_addr;
	struct f2fs_xattr_entry *ent;
	unsigned char en[F2FS_NAME_LEN + 1];
	unsigned int i = 0;
	u32 namelen = le32_to_cpu(inode->i_namelen);
	int enc_name = file_enc_name(inode);
	int ofs = __get_extra_isize(inode);

	namelen = convert_encrypted_name(inode->i_name, namelen, en, enc_name);
	en[namelen] = '\0';
	if (name && namelen) {
		inode->i_name[namelen] = '\0';
		MSG(0, " - File name         : %s%s\n", en,
				enc_name ? " <encrypted>" : "");
		setlocale(LC_ALL, "");
		MSG(0, " - File size         : %'llu (bytes)\n",
				le64_to_cpu(inode->i_size));
		return;
	}

	DISP_u32(inode, i_mode);
	DISP_u32(inode, i_advise);
	DISP_u32(inode, i_uid);
	DISP_u32(inode, i_gid);
	DISP_u32(inode, i_links);
	DISP_u64(inode, i_size);
	DISP_u64(inode, i_blocks);

	DISP_u64(inode, i_atime);
	DISP_u32(inode, i_atime_nsec);
	DISP_u64(inode, i_ctime);
	DISP_u32(inode, i_ctime_nsec);
	DISP_u64(inode, i_mtime);
	DISP_u32(inode, i_mtime_nsec);

	DISP_u32(inode, i_generation);
	DISP_u32(inode, i_current_depth);
	DISP_u32(inode, i_xattr_nid);
	DISP_u32(inode, i_flags);
	DISP_u32(inode, i_inline);
	DISP_u32(inode, i_pino);
	DISP_u32(inode, i_dir_level);

	if (namelen) {
		DISP_u32(inode, i_namelen);
		printf("%-30s\t\t[%s]\n", "i_name", en);
	}

	MSG(0, "i_ext: fofs:%x blkaddr:%x len:%x\n",
			le32_to_cpu(inode->i_ext.fofs),
			le32_to_cpu(inode->i_ext.blk_addr),
			le32_to_cpu(inode->i_ext.len));

	if (c.feature & cpu_to_le32(F2FS_FEATURE_EXTRA_ATTR)) {
		DISP_u16(inode, i_extra_isize);
		if (c.feature & cpu_to_le32(F2FS_FEATURE_FLEXIBLE_INLINE_XATTR))
			DISP_u16(inode, i_inline_xattr_size);
		if (c.feature & cpu_to_le32(F2FS_FEATURE_PRJQUOTA))
			DISP_u32(inode, i_projid);
		if (c.feature & cpu_to_le32(F2FS_FEATURE_INODE_CHKSUM))
			DISP_u32(inode, i_inode_checksum);
		if (c.feature & cpu_to_le32(F2FS_FEATURE_INODE_CRTIME)) {
			DISP_u64(inode, i_crtime);
			DISP_u32(inode, i_crtime_nsec);
		}
		if (c.feature & cpu_to_le32(F2FS_FEATURE_COMPRESSION)) {
			DISP_u64(inode, i_compr_blocks);
			DISP_u32(inode, i_compress_algrithm);
			DISP_u32(inode, i_log_cluster_size);
			DISP_u32(inode, i_padding);
		}
	}

	DISP_u32(inode, i_addr[ofs]);		/* Pointers to data blocks */
	DISP_u32(inode, i_addr[ofs + 1]);	/* Pointers to data blocks */
	DISP_u32(inode, i_addr[ofs + 2]);	/* Pointers to data blocks */
	DISP_u32(inode, i_addr[ofs + 3]);	/* Pointers to data blocks */

	for (i = ofs + 3; i < ADDRS_PER_INODE(inode); i++) {
		if (inode->i_addr[i] == 0x0)
			break;
		MSG(0, "i_addr[0x%x] points data block\t\t[0x%4x]\n",
				i, le32_to_cpu(inode->i_addr[i]));
	}

	DISP_u32(inode, i_nid[0]);	/* direct */
	DISP_u32(inode, i_nid[1]);	/* direct */
	DISP_u32(inode, i_nid[2]);	/* indirect */
	DISP_u32(inode, i_nid[3]);	/* indirect */
	DISP_u32(inode, i_nid[4]);	/* double indirect */

	xattr_addr = read_all_xattrs(sbi, node);
	list_for_each_xattr(ent, xattr_addr) {
		print_xattr_entry(ent);
	}
	free(xattr_addr);

	MSG(0, "\n");
}

void print_node_info(struct f2fs_sb_info *sbi,
			struct f2fs_node *node_block, int verbose)
{
	nid_t ino = le32_to_cpu(node_block->footer.ino);
	nid_t nid = le32_to_cpu(node_block->footer.nid);
	/* Is this inode? */
	if (ino == nid) {
		DBG(verbose, "Node ID [0x%x:%u] is inode\n", nid, nid);
		print_inode_info(sbi, node_block, verbose);
	} else {
		int i;
		u32 *dump_blk = (u32 *)node_block;
		DBG(verbose,
			"Node ID [0x%x:%u] is direct node or indirect node.\n",
								nid, nid);
		for (i = 0; i < NIDS_PER_BLOCK; i++)
			MSG(verbose, "[%d]\t\t\t[0x%8x : %u]\n",
						i, dump_blk[i], dump_blk[i]);
	}
}

static void DISP_label(u_int16_t *name)
{
	char buffer[MAX_VOLUME_NAME];

	utf16_to_utf8(buffer, name, MAX_VOLUME_NAME, MAX_VOLUME_NAME);
	if (c.layout)
		MSG(0, "%-30s %s\n", "Filesystem volume name:", buffer);
	else
		MSG(0, "%-30s" "\t\t[%s]\n", "volum_name", buffer);
}

void print_raw_sb_info(struct f2fs_super_block *sb)
{
	if (c.layout)
		goto printout;
	if (!c.dbg_lv)
		return;

	MSG(0, "\n");
	MSG(0, "+--------------------------------------------------------+\n");
	MSG(0, "| Super block                                            |\n");
	MSG(0, "+--------------------------------------------------------+\n");
printout:
	DISP_u32(sb, magic);
	DISP_u32(sb, major_ver);

	DISP_label(sb->volume_name);

	DISP_u32(sb, minor_ver);
	DISP_u32(sb, log_sectorsize);
	DISP_u32(sb, log_sectors_per_block);

	DISP_u32(sb, log_blocksize);
	DISP_u32(sb, log_blocks_per_seg);
	DISP_u32(sb, segs_per_sec);
	DISP_u32(sb, secs_per_zone);
	DISP_u32(sb, checksum_offset);
	DISP_u64(sb, block_count);

	DISP_u32(sb, section_count);
	DISP_u32(sb, segment_count);
	DISP_u32(sb, segment_count_ckpt);
	DISP_u32(sb, segment_count_sit);
	DISP_u32(sb, segment_count_nat);

	DISP_u32(sb, segment_count_ssa);
	DISP_u32(sb, segment_count_main);
	DISP_u32(sb, segment0_blkaddr);

	DISP_u32(sb, cp_blkaddr);
	DISP_u32(sb, sit_blkaddr);
	DISP_u32(sb, nat_blkaddr);
	DISP_u32(sb, ssa_blkaddr);
	DISP_u32(sb, main_blkaddr);

	DISP_u32(sb, root_ino);
	DISP_u32(sb, node_ino);
	DISP_u32(sb, meta_ino);
	DISP_u32(sb, cp_payload);
	DISP_u32(sb, crc);
	DISP("%-.256s", sb, version);
	MSG(0, "\n");
}

void print_ckpt_info(struct f2fs_sb_info *sbi)
{
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);

	if (c.layout)
		goto printout;
	if (!c.dbg_lv)
		return;

	MSG(0, "\n");
	MSG(0, "+--------------------------------------------------------+\n");
	MSG(0, "| Checkpoint                                             |\n");
	MSG(0, "+--------------------------------------------------------+\n");
printout:
	DISP_u64(cp, checkpoint_ver);
	DISP_u64(cp, user_block_count);
	DISP_u64(cp, valid_block_count);
	DISP_u32(cp, rsvd_segment_count);
	DISP_u32(cp, overprov_segment_count);
	DISP_u32(cp, free_segment_count);

	DISP_u32(cp, alloc_type[CURSEG_HOT_NODE]);
	DISP_u32(cp, alloc_type[CURSEG_WARM_NODE]);
	DISP_u32(cp, alloc_type[CURSEG_COLD_NODE]);
	DISP_u32(cp, alloc_type[CURSEG_DATA_MOVE1]);
	DISP_u32(cp, alloc_type[CURSEG_DATA_MOVE2]);
	DISP_u32(cp, cur_node_segno[0]);
	DISP_u32(cp, cur_node_segno[1]);
	DISP_u32(cp, cur_node_segno[2]);

	DISP_u32(cp, cur_node_blkoff[0]);
	DISP_u32(cp, cur_node_blkoff[1]);
	DISP_u32(cp, cur_node_blkoff[2]);

	DISP_u32(cp, cur_dmgc_segno[0]);
	DISP_u32(cp, cur_dmgc_segno[1]);

	DISP_u32(cp, cur_dmgc_blkoff[0]);
	DISP_u32(cp, cur_dmgc_blkoff[1]);

	DISP_u32(cp, alloc_type[CURSEG_HOT_DATA]);
	DISP_u32(cp, alloc_type[CURSEG_WARM_DATA]);
	DISP_u32(cp, alloc_type[CURSEG_COLD_DATA]);
	DISP_u32(cp, cur_data_segno[0]);
	DISP_u32(cp, cur_data_segno[1]);
	DISP_u32(cp, cur_data_segno[2]);

	DISP_u32(cp, cur_data_blkoff[0]);
	DISP_u32(cp, cur_data_blkoff[1]);
	DISP_u32(cp, cur_data_blkoff[2]);

	DISP_u32(cp, ckpt_flags);
	DISP_u32(cp, cp_pack_total_block_count);
	DISP_u32(cp, cp_pack_start_sum);
	DISP_u32(cp, valid_node_count);
	DISP_u32(cp, valid_inode_count);
	DISP_u32(cp, next_free_nid);
	DISP_u32(cp, sit_ver_bitmap_bytesize);
	DISP_u32(cp, nat_ver_bitmap_bytesize);
	DISP_u32(cp, checksum_offset);
	DISP_u64(cp, elapsed_time);

	DISP_u32(cp, sit_nat_version_bitmap[0]);
	MSG(0, "\n\n");
}

void print_cp_state(u32 flag)
{
	MSG(0, "Info: checkpoint state = %x : ", flag);
	if (flag & CP_QUOTA_NEED_FSCK_FLAG)
		MSG(0, "%s", " quota_need_fsck");
	if (flag & CP_LARGE_NAT_BITMAP_FLAG)
		MSG(0, "%s", " large_nat_bitmap");
	if (flag & CP_NOCRC_RECOVERY_FLAG)
		MSG(0, "%s", " allow_nocrc");
	if (flag & CP_TRIMMED_FLAG)
		MSG(0, "%s", " trimmed");
	if (flag & CP_CRC_RECOVERY_FLAG)
		MSG(0, "%s", " crc");
	if (flag & CP_FASTBOOT_FLAG)
		MSG(0, "%s", " fastboot");
	if (flag & CP_FSCK_FLAG)
		MSG(0, "%s", " fsck");
	if (flag & CP_ERROR_FLAG)
		MSG(0, "%s", " error");
	if (flag & CP_COMPACT_SUM_FLAG)
		MSG(0, "%s", " compacted_summary");
	if (flag & CP_ORPHAN_PRESENT_FLAG)
		MSG(0, "%s", " orphan_inodes");
	if (flag & CP_DISABLED_FLAG)
		MSG(0, "%s", " disabled");
	if (flag & CP_RESIZEFS_FLAG)
		MSG(0, "%s", " resizefs");
	if (flag & CP_UMOUNT_FLAG)
		MSG(0, "%s", " unmount");
	else
		MSG(0, "%s", " sudden-power-off");
	MSG(0, "\n");
}

void print_sb_state(struct f2fs_super_block *sb)
{
	__le32 f = sb->feature;
	int i;

	MSG(0, "Info: superblock features = %x : ", f);
	if (f & cpu_to_le32(F2FS_FEATURE_ENCRYPT)) {
		MSG(0, "%s", " encrypt");
	}
	if (f & cpu_to_le32(F2FS_FEATURE_VERITY)) {
		MSG(0, "%s", " verity");
	}
	if (f & cpu_to_le32(F2FS_FEATURE_BLKZONED)) {
		MSG(0, "%s", " blkzoned");
	}
	if (f & cpu_to_le32(F2FS_FEATURE_EXTRA_ATTR)) {
		MSG(0, "%s", " extra_attr");
	}
	if (f & cpu_to_le32(F2FS_FEATURE_PRJQUOTA)) {
		MSG(0, "%s", " project_quota");
	}
	if (f & cpu_to_le32(F2FS_FEATURE_INODE_CHKSUM)) {
		MSG(0, "%s", " inode_checksum");
	}
	if (f & cpu_to_le32(F2FS_FEATURE_FLEXIBLE_INLINE_XATTR)) {
		MSG(0, "%s", " flexible_inline_xattr");
	}
	if (f & cpu_to_le32(F2FS_FEATURE_QUOTA_INO)) {
		MSG(0, "%s", " quota_ino");
	}
	if (f & cpu_to_le32(F2FS_FEATURE_INODE_CRTIME)) {
		MSG(0, "%s", " inode_crtime");
	}
	if (f & cpu_to_le32(F2FS_FEATURE_LOST_FOUND)) {
		MSG(0, "%s", " lost_found");
	}
	if (f & cpu_to_le32(F2FS_FEATURE_SB_CHKSUM)) {
		MSG(0, "%s", " sb_checksum");
	}
	if (f & cpu_to_le32(F2FS_FEATURE_CASEFOLD)) {
		MSG(0, "%s", " casefold");
	}
	if (f & cpu_to_le32(F2FS_FEATURE_COMPRESSION)) {
		MSG(0, "%s", " compression");
	}
	MSG(0, "\n");
	MSG(0, "Info: superblock encrypt level = %d, salt = ",
					sb->encryption_level);
	for (i = 0; i < 16; i++)
		MSG(0, "%02x", sb->encrypt_pw_salt[i]);
	MSG(0, "\n");
}

static void set_sb_chksum(struct f2fs_super_block *sb)
{
	sb->feature |= cpu_to_le32(F2FS_FEATURE_SB_CHKSUM);
	set_sb(checksum_offset, SB_CHKSUM_OFFSET);
}

void update_superblock(struct f2fs_super_block *sb, int sb_mask)
{
	int addr, ret;
	u_int8_t *buf;
	u32 old_crc, new_crc;

	buf = calloc(BLOCK_SZ, 1);
	ASSERT(buf);

	if (get_sb(feature) & F2FS_FEATURE_SB_CHKSUM) {
		old_crc = get_sb(crc);
		new_crc = f2fs_cal_crc32(get_sb(magic), sb,
						SB_CHKSUM_OFFSET);
		set_sb(crc, new_crc);
		MSG(1, "Info: SB CRC is updated (0x%x -> 0x%x)\n",
							old_crc, new_crc);
	}

	memcpy(buf + F2FS_SUPER_OFFSET, sb, sizeof(*sb));
	for (addr = SB0_ADDR; addr < SB_MAX_ADDR; addr++) {
		if (SB_MASK(addr) & sb_mask) {
			set_write_stream_id(META_STREAM_ID, addr);
			ret = dev_write_block(buf, addr);
			ASSERT(ret >= 0);
		}
	}

	free(buf);
	DBG(0, "Info: Done to update superblock\n");
}

static inline int sanity_check_area_boundary(struct f2fs_super_block *sb,
							enum SB_ADDR sb_addr)
{
	u32 segment0_blkaddr = get_sb(segment0_blkaddr);
	u32 cp_blkaddr = get_sb(cp_blkaddr);
	u32 sit_blkaddr = get_sb(sit_blkaddr);
	u32 nat_blkaddr = get_sb(nat_blkaddr);
	u32 ssa_blkaddr = get_sb(ssa_blkaddr);
	u32 main_blkaddr = get_sb(main_blkaddr);
	u32 segment_count_ckpt = get_sb(segment_count_ckpt);
	u32 segment_count_sit = get_sb(segment_count_sit);
	u32 segment_count_nat = get_sb(segment_count_nat);
	u32 segment_count_ssa = get_sb(segment_count_ssa);
	u32 segment_count_main = get_sb(segment_count_main);
	u32 segment_count = get_sb(segment_count);
	u32 log_blocks_per_seg = get_sb(log_blocks_per_seg);
	u64 main_end_blkaddr = main_blkaddr +
				(segment_count_main << log_blocks_per_seg);
	u64 seg_end_blkaddr = segment0_blkaddr +
				(segment_count << log_blocks_per_seg);

	if (segment0_blkaddr != cp_blkaddr) {
		MSG(0, "\tMismatch segment0(%u) cp_blkaddr(%u)\n",
				segment0_blkaddr, cp_blkaddr);
		return -1;
	}

	if (cp_blkaddr + (segment_count_ckpt << log_blocks_per_seg) !=
							sit_blkaddr) {
		MSG(0, "\tWrong CP boundary, start(%u) end(%u) blocks(%u)\n",
			cp_blkaddr, sit_blkaddr,
			segment_count_ckpt << log_blocks_per_seg);
		return -1;
	}

	if (sit_blkaddr + (segment_count_sit << log_blocks_per_seg) !=
							nat_blkaddr) {
		MSG(0, "\tWrong SIT boundary, start(%u) end(%u) blocks(%u)\n",
			sit_blkaddr, nat_blkaddr,
			segment_count_sit << log_blocks_per_seg);
		return -1;
	}

	if (nat_blkaddr + (segment_count_nat << log_blocks_per_seg) !=
							ssa_blkaddr) {
		MSG(0, "\tWrong NAT boundary, start(%u) end(%u) blocks(%u)\n",
			nat_blkaddr, ssa_blkaddr,
			segment_count_nat << log_blocks_per_seg);
		return -1;
	}

	if (ssa_blkaddr + (segment_count_ssa << log_blocks_per_seg) !=
							main_blkaddr) {
		MSG(0, "\tWrong SSA boundary, start(%u) end(%u) blocks(%u)\n",
			ssa_blkaddr, main_blkaddr,
			segment_count_ssa << log_blocks_per_seg);
		return -1;
	}

	if (main_end_blkaddr > seg_end_blkaddr) {
		MSG(0, "\tWrong MAIN_AREA, start(%u) end(%u) block(%u)\n",
			main_blkaddr,
			segment0_blkaddr +
				(segment_count << log_blocks_per_seg),
			segment_count_main << log_blocks_per_seg);
		return -1;
	} else if (main_end_blkaddr < seg_end_blkaddr) {
		set_sb(segment_count, (main_end_blkaddr -
				segment0_blkaddr) >> log_blocks_per_seg);

		update_superblock(sb, SB_MASK(sb_addr));
		MSG(0, "Info: Fix alignment: start(%u) end(%u) block(%u)\n",
			main_blkaddr,
			segment0_blkaddr +
				(segment_count << log_blocks_per_seg),
			segment_count_main << log_blocks_per_seg);
	}
	return 0;
}

static int verify_sb_chksum(struct f2fs_super_block *sb)
{
	if (SB_CHKSUM_OFFSET != get_sb(checksum_offset)) {
		MSG(0, "\tInvalid SB CRC offset: %u\n",
					get_sb(checksum_offset));
		return -1;
	}
	if (f2fs_crc_valid(get_sb(magic), get_sb(crc), sb,
			get_sb(checksum_offset))) {
		MSG(0, "\tInvalid SB CRC: 0x%x\n", get_sb(crc));
		return -1;
	}
	return 0;
}

int sanity_check_raw_super(struct f2fs_super_block *sb, enum SB_ADDR sb_addr)
{
	unsigned int blocksize;

	if ((get_sb(feature) & F2FS_FEATURE_SB_CHKSUM) &&
					verify_sb_chksum(sb))
		return -1;

	if (c.func == FSCK && F2FS_SUPER_MAGIC == get_sb(magic)) {
		MSG(0, "Ignore first fsck before resize\n");
		exit(1);
	}

	/* RESIZE need compatible with F2FS, ignore check here */
	if (c.func != RESIZE && HMFS_SUPER_MAGIC != get_sb(magic)) {
		MSG(0, "HMFS Magic Mismatch, valid(0x%x) - read(0x%x)\n",
				HMFS_SUPER_MAGIC, get_sb(magic));
		return -1;
	}

	if (F2FS_BLKSIZE != PAGE_CACHE_SIZE)
		return -1;

	blocksize = 1 << get_sb(log_blocksize);
	if (F2FS_BLKSIZE != blocksize)
		return -1;

	/* check log blocks per segment */
	if (get_sb(log_blocks_per_seg) != 9)
		return -1;

	/* Currently, support 512/1024/2048/4096 bytes sector size */
	if (get_sb(log_sectorsize) > F2FS_MAX_LOG_SECTOR_SIZE ||
			get_sb(log_sectorsize) < F2FS_MIN_LOG_SECTOR_SIZE)
		return -1;

	if (get_sb(log_sectors_per_block) + get_sb(log_sectorsize) !=
						F2FS_MAX_LOG_SECTOR_SIZE)
		return -1;

	/* check reserved ino info */
	if (get_sb(node_ino) != 1 || get_sb(meta_ino) != 2 ||
					get_sb(root_ino) != 3)
		return -1;

	/* Check zoned block device feature */
	if (c.devices[0].zoned_model == F2FS_ZONED_HM &&
			!(sb->feature & cpu_to_le32(F2FS_FEATURE_BLKZONED))) {
		MSG(0, "\tMissing zoned block device feature\n");
		return -1;
	}

	if (get_sb(segment_count) > F2FS_MAX_SEGMENT)
		return -1;

	if (sanity_check_area_boundary(sb, sb_addr))
		return -1;
	return 0;
}

static int is_same_device(char *sb_dev_path, char *device_path)
{
	struct stat dev_stat, device_stat;

	ASSERT(sb_dev_path && device_path);

	if (!strcmp(sb_dev_path, device_path))
		return 0;

	if (stat(sb_dev_path, &dev_stat) || stat(device_path, &device_stat)) {
		MSG(0, "\t stat %s or %s failed\n", sb_dev_path, device_path);
		return -1;
	}

	if (S_ISBLK(dev_stat.st_mode) && S_ISBLK(device_stat.st_mode) &&
			dev_stat.st_rdev == device_stat.st_rdev)
		return 0;

	return -1;
}

/* check the already added device */
static int valid_sb_new_device(struct f2fs_super_block *sb, int ndevs)
{
	int i;

	ASSERT(ndevs > 0);

	for (i = 0; i < ndevs; i++) {
		if (!is_same_device((char *)sb->devs[i].path, c.device_to_add))
			return -1;
	}

	return 0;
}

/* write the new device path into sbi->raw_super. */
static int add_sb_new_device(struct f2fs_sb_info *sbi)
{
	int ndevs = 0;
	int err = 0;
	struct f2fs_super_block *sb = sbi->raw_super;

	if (!c.device_to_add)
		return 0;

	ASSERT(c.func == RESIZE);

	while (ndevs < MAX_DEVICES && sb->devs[ndevs].path[0])
		ndevs++;

	if (ndevs >= MAX_DEVICES) {
		MSG(0, "\tError: Too many devices:%s\n", c.device_to_add);
		err = -ENOTSUP;
		goto out;
	}

	if (!ndevs) {
		strncpy((char *)sb->devs[ndevs].path,
				c.devices[0].path, MAX_PATH_LEN);
		ndevs++;
	}

	/* add RESERVED_DEV(10G) for dynamic resize and
	 * c.device_to_add for common resize.
	 */
	if (ndevs == 1 && valid_sb_new_device(sb, ndevs) == 0) {
		/* mkfs single dev then resize with -c and -r */
		if (c.reserved_segs > 0) {
			strncpy((char *)sb->devs[ndevs].path,
					RESERVED_DEV, MAX_PATH_LEN);
			MSG(0, "Info: write the new device[%d] path :%s into sb"
				"c.reserved_segs=%d\n", ndevs,
				sb->devs[ndevs].path, c.reserved_segs);
			ndevs++;
		}
		strncpy((char *)sb->devs[ndevs].path,
				c.device_to_add, MAX_PATH_LEN);
		MSG(0, "Info: write the new device[%d] path :%s into sb\n",
						ndevs, sb->devs[ndevs].path);
	} else if (ndevs == 2 && valid_sb_new_device(sb, ndevs)) {
		/* already resize only with -c, we need set devs[1] RESERVED_DEV */
		if (c.reserved_segs > 0) {
			strncpy((char *)sb->devs[1].path,
					RESERVED_DEV, MAX_PATH_LEN);
			MSG(0, "Info: ndevs=2, write the new device[1] path :%s into sb\n",
						sb->devs[1].path);
			strncpy((char *)sb->devs[2].path,
					c.device_to_add, MAX_PATH_LEN);
			MSG(0, "Info: ndevs=2, write the new device[2] path :%s into sb\n",
						sb->devs[2].path);
			/* for now it is not supported; mainarea need to adjust */
			err = -ENOTSUP;
		} else {
			MSG(0, "Info: ndevs=2, Maybe write the already added device[1] "
				"path :%s into sb\n", c.device_to_add);
			err = -ENOTSUP;
		}
	} else if (ndevs == 3 && valid_sb_new_device(sb, ndevs)) {
		/* already resize with -c and -r, just do check */
		if (!strcmp((char *)sb->devs[1].path, RESERVED_DEV)) {
			MSG(0, "Info: ndevs=3, Maybe write the already added "
				"device[1]=%s, device[2]=%s into sb, "
				"c.device_to_add=%s\n", RESERVED_DEV,
				(char *)sb->devs[2].path, c.device_to_add);
			err = -ENOTSUP;
		} else {
			MSG(0, "Info: ndevs=3, unexpected path:device[1]=%s, "
				"device[2]=%s into sb, c.device_to_add=%s\n",
				(char *)sb->devs[1].path,
				(char *)sb->devs[2].path, c.device_to_add);
			err = -ENOTSUP;
		}
	} else if (valid_sb_new_device(sb, ndevs) == 0) {
		/* support common resize ,ignore '-c' c.reserved_segs */
		strncpy((char *)sb->devs[ndevs].path,
				c.device_to_add, MAX_PATH_LEN);
		MSG(0, "Info: write the new device[%d] path :%s into sb, "
			"ignore reserved_segs=%d\n", ndevs,
				sb->devs[ndevs].path, c.reserved_segs);
	} else {
		MSG(0, "Info: Maybe write the already added device[%d] "
			"path :%s into sb, reserved_segs=%d\n",
			ndevs, c.device_to_add, c.reserved_segs);
		err = -ENOTSUP;
	}

out:
	free(c.device_to_add);
	c.device_to_add = NULL;

	return err;
}

int validate_super_block(struct f2fs_sb_info *sbi, enum SB_ADDR sb_addr)
{
	char buf[F2FS_BLKSIZE];

	sbi->raw_super = malloc(sizeof(struct f2fs_super_block));
	ASSERT(sbi->raw_super);

	if (dev_read_block(buf, sb_addr))
		goto free;

	memcpy(sbi->raw_super, buf + F2FS_SUPER_OFFSET,
					sizeof(struct f2fs_super_block));

	if (!sanity_check_raw_super(sbi->raw_super, sb_addr)) {
		/* write the new device path into sbi->raw_super */
		if (add_sb_new_device(sbi)) {
			free(sbi->raw_super);
			sbi->raw_super = NULL;
			return -ENOTSUP;
		}

		/* get kernel version */
		if (c.kd >= 0) {
			if (dev_read_version(c.version, 0, VERSION_LEN)) {
				MSG(0, "Error: While reading kernel version\n");
				goto free;
			}
			get_kernel_version(c.version);
		} else {
			get_kernel_uname_version(c.version);
		}

		/* build sb version */
		memcpy(c.sb_version, sbi->raw_super->version, VERSION_LEN);
		get_kernel_version(c.sb_version);
		memcpy(c.init_version, sbi->raw_super->init_version, VERSION_LEN);
		get_kernel_version(c.init_version);

		MSG(0, "Info: MKFS version\n  \"%s\"\n", c.init_version);
		MSG(0, "Info: FSCK version\n  from \"%s\"\n    to \"%s\"\n",
					c.sb_version, c.version);
		if (cmp_kernel_version(c.sb_version, c.version)) {
			memcpy(sbi->raw_super->version,
						c.version, VERSION_LEN);
			set_sb_chksum(sbi->raw_super);
			update_superblock(sbi->raw_super, SB_MASK(sb_addr));

			c.auto_fix = 0;
			c.fix_on = 1;
		}
		print_sb_state(sbi->raw_super);
		return 0;
	}

free:
	free(sbi->raw_super);
	sbi->raw_super = NULL;
	MSG(0, "\tCan't find a valid HMFS superblock at 0x%x\n", sb_addr);

	return -EINVAL;
}

int init_sb_info(struct f2fs_sb_info *sbi)
{
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	u64 total_sectors;
	int i;

	sbi->log_sectors_per_block = get_sb(log_sectors_per_block);
	sbi->log_blocksize = get_sb(log_blocksize);
	sbi->blocksize = 1 << sbi->log_blocksize;
	sbi->log_blocks_per_seg = get_sb(log_blocks_per_seg);
	sbi->blocks_per_seg = 1 << sbi->log_blocks_per_seg;
	sbi->segs_per_sec = get_sb(segs_per_sec);
	sbi->secs_per_zone = get_sb(secs_per_zone);
	sbi->total_sections = get_sb(section_count);
	sbi->total_node_count = (get_sb(segment_count_nat) / 2) *
				sbi->blocks_per_seg * NAT_ENTRY_PER_BLOCK;
	sbi->root_ino_num = get_sb(root_ino);
	sbi->node_ino_num = get_sb(node_ino);
	sbi->meta_ino_num = get_sb(meta_ino);
	sbi->cur_victim_sec = NULL_SEGNO;

	for (i = 0; i < MAX_DEVICES; i++) {
		if (!sb->devs[i].path[0])
			break;

		if (i) {
			c.devices[i].path = strdup((char *)sb->devs[i].path);
			if (!strcmp(c.devices[i].path, RESERVED_DEV))
				c.devices[i].reserved = 1;
			if (get_device_info(i))
				ASSERT(0);
		} else {
			ASSERT(!is_same_device((char *)sb->devs[i].path,
						(char *)c.devices[i].path));
		}

		if (c.func == RESIZE) {
			if (i) {
				if (i == 1 && c.reserved_segs > 0)
					set_sb(devs[i].total_segments,
							c.reserved_segs);
				else
					set_sb(devs[i].total_segments,
						c.devices[i].total_sectors /
							(c.sectors_per_blk *
							c.blks_per_seg)); /*lint !e647 !e712 !e744*/
			} else {
				set_sb(devs[i].total_segments,
					(c.devices[i].total_sectors /
					c.sectors_per_blk -
					get_sb(segment0_blkaddr)) /
					c.blks_per_seg); /*lint !e712 !e737 !e744*/
			}
		}

		c.devices[i].total_segments =
			le32_to_cpu(sb->devs[i].total_segments);
		if (i)
			c.devices[i].start_blkaddr =
				c.devices[i - 1].end_blkaddr + 1;
		c.devices[i].end_blkaddr = c.devices[i].start_blkaddr +
			c.devices[i].total_segments *
			c.blks_per_seg - 1;
		if (i == 0)
			c.devices[i].end_blkaddr += get_sb(segment0_blkaddr);

		c.ndevs = i + 1;
		MSG(0, "Info: Device[%d] : %s blkaddr = %"PRIx64"--%"PRIx64"\n",
				i, c.devices[i].path,
				c.devices[i].start_blkaddr,
				c.devices[i].end_blkaddr);
	}

	total_sectors = get_sb(block_count) << sbi->log_sectors_per_block;
	MSG(0, "Info: total FS sectors = %"PRIu64" (%"PRIu64" MB)\n",
				total_sectors, total_sectors >>
						(20 - get_sb(log_sectorsize)));
	return 0;
}

void clear_extra_flag(struct f2fs_sb_info *sbi, unsigned int flag)
{
	struct extra_flags_block *ef_blk;
	unsigned long long cp_blkaddr;
	int ret;

	cp_blkaddr = le32_to_cpu(sbi->raw_super->cp_blkaddr);
	ef_blk = calloc(PAGE_SIZE, 1);
	if (!ef_blk) {
		ERR_MSG("failed to alloc extra_flags_block\n");
		return;
	}
	if (dev_read_block(ef_blk, cp_blkaddr + sbi->blocks_per_seg - 1) < 0) {
		ERR_MSG("failed to read extra_flags_block\n");
		goto free;
	}

	switch (flag) {
	case EXTRA_NEED_FSCK_FLAG:
		if (!ef_blk->need_fsck)
			goto free;
		ef_blk->need_fsck = 0;
		break;
	default:
		ERR_MSG("unknown extra flag 0x%x\n", flag);
		goto free;
	}

	/* do not use crc for now */
	ret = f2fs_write_block(sbi, ef_blk, cp_blkaddr + sbi->blocks_per_seg - 1, 1);
	if (ret < 0)
		ERR_MSG("failed to write extra_flags_block\n");
free:
	free(ef_blk);
}

static void check_extra_flag(struct f2fs_sb_info *sbi, unsigned int flag)
{
	struct extra_flags_block *ef_blk;
	unsigned long long cp_blkaddr;

	cp_blkaddr = le32_to_cpu(sbi->raw_super->cp_blkaddr);
	ef_blk = calloc(PAGE_SIZE, 1);
	if (!ef_blk)
		return;
	if (dev_read_block(ef_blk, cp_blkaddr + sbi->blocks_per_seg - 1) < 0)
		goto free;

	/* do not use crc for now */
	switch (flag) {
	case EXTRA_NEED_FSCK_FLAG:
		if (le32_to_cpu(ef_blk->need_fsck) == flag) {
			ASSERT_MSG("EXTRA_NEED_FSCK_FLAG is set");
			c.fix_on = 1;
		}
		break;
	default:
		break;
	}

free:
	free(ef_blk);
}

void *validate_checkpoint(struct f2fs_sb_info *sbi, block_t cp_addr,
				unsigned long long *version)
{
	void *cp_page_1, *cp_page_2;
	struct f2fs_checkpoint *cp;
	struct f2fs_super_block *sb = sbi->raw_super;
	unsigned long blk_size = sbi->blocksize;
	unsigned long long cur_version = 0, pre_version = 0;
	unsigned int crc = 0;
	size_t crc_offset;

	/* Read the 1st cp block in this CP pack */
	cp_page_1 = malloc(PAGE_SIZE);
	ASSERT(cp_page_1);

	if (dev_read_block(cp_page_1, cp_addr) < 0)
		goto invalid_cp1;

	cp = (struct f2fs_checkpoint *)cp_page_1;
	crc_offset = get_cp(checksum_offset);
	if (crc_offset > (blk_size - sizeof(__le32)))
		goto invalid_cp1;

	crc = le32_to_cpu(*(__le32 *)((unsigned char *)cp + crc_offset));
	if (f2fs_crc_valid(get_sb(magic), crc, cp, crc_offset))
		goto invalid_cp1;

	if (get_cp(cp_pack_total_block_count) > sbi->blocks_per_seg)
		goto invalid_cp1;

	pre_version = get_cp(checkpoint_ver);

	/* Read the 2nd cp block in this CP pack */
	cp_page_2 = malloc(PAGE_SIZE);
	ASSERT(cp_page_2);

	cp_addr += get_cp(cp_pack_total_block_count) - 1;

	if (dev_read_block(cp_page_2, cp_addr) < 0)
		goto invalid_cp2;

	cp = (struct f2fs_checkpoint *)cp_page_2;
	crc_offset = get_cp(checksum_offset);
	if (crc_offset > (blk_size - sizeof(__le32)))
		goto invalid_cp2;

	crc = le32_to_cpu(*(__le32 *)((unsigned char *)cp + crc_offset));
	if (f2fs_crc_valid(get_sb(magic), crc, cp, crc_offset))
		goto invalid_cp2;

	cur_version = get_cp(checkpoint_ver);

	if (cur_version == pre_version) {
		*version = cur_version;
		free(cp_page_2);
		return cp_page_1;
	}

invalid_cp2:
	free(cp_page_2);
invalid_cp1:
	free(cp_page_1);
	return NULL;
}

int get_valid_checkpoint(struct f2fs_sb_info *sbi)
{
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	void *cp1, *cp2, *cur_page;
	unsigned long blk_size = sbi->blocksize;
	unsigned long long cp1_version = 0, cp2_version = 0, version;
	unsigned long long cp_start_blk_no;
	unsigned int cp_payload, cp_blks;
	int ret;

	cp_payload = get_sb(cp_payload);
	if (cp_payload > F2FS_BLK_ALIGN(MAX_SIT_BITMAP_SIZE))
		return -EINVAL;

	cp_blks = 1 + cp_payload;
	sbi->ckpt = malloc(cp_blks * blk_size);
	if (!sbi->ckpt)
		return -ENOMEM;
	/*
	 * Finding out valid cp block involves read both
	 * sets( cp pack1 and cp pack 2)
	 */
	cp_start_blk_no = get_sb(cp_blkaddr);
	cp1 = validate_checkpoint(sbi, cp_start_blk_no, &cp1_version);
	check_extra_flag(sbi, EXTRA_NEED_FSCK_FLAG);

	/* The second checkpoint pack should start at the next segment */
	cp_start_blk_no += 1 << get_sb(log_blocks_per_seg);
	cp2 = validate_checkpoint(sbi, cp_start_blk_no, &cp2_version);

	if (cp1 && cp2) {
		if (ver_after(cp2_version, cp1_version)) {
			cur_page = cp2;
			sbi->cur_cp = 2;
			version = cp2_version;
		} else {
			cur_page = cp1;
			sbi->cur_cp = 1;
			version = cp1_version;
		}
	} else if (cp1) {
		cur_page = cp1;
		sbi->cur_cp = 1;
		version = cp1_version;
	} else if (cp2) {
		cur_page = cp2;
		sbi->cur_cp = 2;
		version = cp2_version;
	} else
		goto fail_no_cp;

	MSG(0, "Info: CKPT version = %llx\n", version);

	memcpy(sbi->ckpt, cur_page, blk_size);

	if (cp_blks > 1) {
		unsigned int i;
		unsigned long long cp_blk_no;

		cp_blk_no = get_sb(cp_blkaddr);
		if (cur_page == cp2)
			cp_blk_no += 1 << get_sb(log_blocks_per_seg);

		/* copy sit bitmap */
		for (i = 1; i < cp_blks; i++) {
			unsigned char *ckpt = (unsigned char *)sbi->ckpt;
			ret = dev_read_block(cur_page, cp_blk_no + i);
			ASSERT(ret >= 0);
			memcpy(ckpt + i * blk_size, cur_page, blk_size);
		}
	}
	if (cp1)
		free(cp1);
	if (cp2)
		free(cp2);
	return 0;

fail_no_cp:
	free(sbi->ckpt);
	sbi->ckpt = NULL;
	return -EINVAL;
}

/*
 * For a return value of 1, caller should further check for c.fix_on state
 * and take appropriate action.
 */
static int f2fs_should_proceed(struct f2fs_super_block *sb, u32 flag)
{
	if (c.auto_fix || c.preen_mode) {
		if (flag & CP_FSCK_FLAG ||
			flag & CP_QUOTA_NEED_FSCK_FLAG ||
			(exist_qf_ino(sb) && (flag & CP_ERROR_FLAG))) {
			c.fix_on = 1;
		} else if (!c.preen_mode) {
			print_cp_state(flag);
			return 0;
		}
	}
	return 1;
}

int sanity_check_ckpt(struct f2fs_sb_info *sbi)
{
	unsigned int total, fsmeta;
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);
	unsigned int flag = get_cp(ckpt_flags);
	unsigned int ovp_segments, reserved_segments;
	unsigned int main_segs, blocks_per_seg;
	unsigned int sit_segs, nat_segs;
	unsigned int sit_bitmap_size, nat_bitmap_size;
	unsigned int log_blocks_per_seg;
	unsigned int segment_count_main;
	unsigned int cp_pack_start_sum, cp_payload;
	block_t user_block_count;
	int i;

	total = get_sb(segment_count);
	fsmeta = get_sb(segment_count_ckpt);
	sit_segs = get_sb(segment_count_sit);
	fsmeta += sit_segs;
	nat_segs = get_sb(segment_count_nat);
	fsmeta += nat_segs;
	fsmeta += get_cp(rsvd_segment_count);
	fsmeta += get_sb(segment_count_ssa);

	if (fsmeta >= total)
		return 1;

	ovp_segments = get_cp(overprov_segment_count);
	reserved_segments = get_cp(rsvd_segment_count);

	if (fsmeta < F2FS_MIN_SEGMENT || ovp_segments == 0 ||
					reserved_segments == 0) {
		MSG(0, "\tWrong layout: check mkfs.hmfs version\n");
		return 1;
	}

	user_block_count = get_cp(user_block_count);
	segment_count_main = get_sb(segment_count_main);
	log_blocks_per_seg = get_sb(log_blocks_per_seg);
	if (!user_block_count || user_block_count >=
			segment_count_main << log_blocks_per_seg) {
		ASSERT_MSG("\tWrong user_block_count(%u)\n", user_block_count);

		if (!f2fs_should_proceed(sb, flag))
			return 1;
		if (!c.fix_on)
			return 1;

		if (flag & (CP_FSCK_FLAG | CP_RESIZEFS_FLAG)) {
			u32 valid_user_block_cnt;
			u32 seg_cnt_main = get_sb(segment_count) -
					(get_sb(segment_count_ckpt) +
					 get_sb(segment_count_sit) +
					 get_sb(segment_count_nat) +
					 get_sb(segment_count_ssa));

			/* validate segment_count_main in sb first */
			if (seg_cnt_main != get_sb(segment_count_main)) {
				MSG(0, "Inconsistent segment_cnt_main %u in sb\n",
						segment_count_main << log_blocks_per_seg);
				return 1;
			}
			valid_user_block_cnt = ((get_sb(segment_count_main) -
						get_cp(overprov_segment_count)) * c.blks_per_seg);
			MSG(0, "Info: Fix wrong user_block_count in CP: (%u) -> (%u)\n",
					user_block_count, valid_user_block_cnt);
			set_cp(user_block_count, valid_user_block_cnt);
			c.bug_on = 1;
		}
	}
	if (sbi->total_sections * sbi->segs_per_sec != segment_count_main) {
		ASSERT_MSG("\tWrong total sections(%u)\n", sbi->total_sections);
		return 1;
	}

	main_segs = get_sb(segment_count_main);
	blocks_per_seg = sbi->blocks_per_seg;
	print_ckpt_info(sbi);

	for (i = 0; i < NR_CURSEG_NODE_TYPE; i++) {
		if ((sbi->active_logs == NR_UNIFY_CURSEG_TYPE) &&
				((i + CURSEG_HOT_NODE) == CURSEG_WARM_NODE)) {
			if (get_cp(cur_node_segno[i]) == NULL_SEGNO) {
				continue;
			} else if (c.func == FSCK) {
				set_cp(cur_node_segno[i], NULL_SEGNO);
				c.fix_on = 1;
				continue;
			}
		}
		if (get_cp(cur_node_segno[i]) >= main_segs ||
			get_cp(cur_node_blkoff[i]) >= blocks_per_seg)
			return 1;
	}
	for (i = 0; i < NR_CURSEG_DATA_TYPE; i++) {
		if ((sbi->active_logs == NR_UNIFY_CURSEG_TYPE) &&
				(i == CURSEG_WARM_DATA)) {
			if (get_cp(cur_data_segno[i]) == NULL_SEGNO) {
				continue;
			} else if (c.func == FSCK) {
				set_cp(cur_data_segno[i], NULL_SEGNO);
				c.fix_on = 1;
				continue;
			}
		}
		if (get_cp(cur_data_segno[i]) >= main_segs ||
			get_cp(cur_data_blkoff[i]) >= blocks_per_seg)
			return 1;
	}
	if (is_set_ckpt_flags(F2FS_CKPT(sbi), CP_DATAMOVE_FLAG)) {
		for (i = 0; i < NR_CURSEG_DM_TYPE; i++) {
			if (get_cp(cur_dmgc_segno[i]) >= main_segs ||
					get_cp(cur_dmgc_blkoff[i]) >= blocks_per_seg)
				return 1;
		}
	}

	sit_bitmap_size = get_cp(sit_ver_bitmap_bytesize);
	nat_bitmap_size = get_cp(nat_ver_bitmap_bytesize);

	if (sit_bitmap_size != ((sit_segs / 2) << log_blocks_per_seg) / 8 ||
		nat_bitmap_size != ((nat_segs / 2) << log_blocks_per_seg) / 8) {
		MSG(0, "\tWrong bitmap size: sit(%u), nat(%u)\n",
				sit_bitmap_size, nat_bitmap_size);
		return 1;
	}

	cp_pack_start_sum = __start_sum_addr(sbi);
	cp_payload = __cp_payload(sbi);
	if (cp_pack_start_sum < cp_payload + 1 ||
		cp_pack_start_sum > blocks_per_seg - 1 -
			NR_CURSEG_TYPE) {
		MSG(0, "\tWrong cp_pack_start_sum(%u) or cp_payload(%u)\n",
			cp_pack_start_sum, cp_payload);
		if ((get_sb(feature) & F2FS_FEATURE_SB_CHKSUM))
			return 1;
		set_sb(cp_payload, cp_pack_start_sum - 1);
		update_superblock(sb, SB_MASK_ALL);
	}

	return 0;
}

pgoff_t current_nat_addr(struct f2fs_sb_info *sbi, nid_t start, int *pack)
{
	struct f2fs_nm_info *nm_i = NM_I(sbi);
	pgoff_t block_off;
	pgoff_t block_addr;
	int seg_off;

	block_off = NAT_BLOCK_OFFSET(start);
	seg_off = block_off >> sbi->log_blocks_per_seg;

	block_addr = (pgoff_t)(nm_i->nat_blkaddr +
			(seg_off << sbi->log_blocks_per_seg << 1) +
			(block_off & ((1 << sbi->log_blocks_per_seg) -1)));
	if (pack)
		*pack = 1;

	if (f2fs_test_bit(block_off, nm_i->nat_bitmap)) {
		block_addr += sbi->blocks_per_seg;
		if (pack)
			*pack = 2;
	}

	return block_addr;
}

pgoff_t next_nat_addr(struct f2fs_sb_info *sbi, pgoff_t block_addr)
{
	struct f2fs_nm_info *nm_i = NM_I(sbi);

	block_addr -= nm_i->nat_blkaddr;
	block_addr ^= 1 << sbi->log_blocks_per_seg;
	return block_addr + nm_i->nat_blkaddr;
}

pgoff_t next_sit_addr(struct f2fs_sb_info *sbi, pgoff_t block_addr)
{
	struct sit_info *sit_i = SIT_I(sbi);
	block_addr -= sit_i->sit_base_addr;
	if (block_addr < sit_i->sit_blocks)
		block_addr += sit_i->sit_blocks;
	else
		block_addr -= sit_i->sit_blocks;

	return block_addr + sit_i->sit_base_addr;
}

static inline void set_to_next_nat(struct f2fs_sb_info *sbi, nid_t nid)
{
	struct f2fs_nm_info *nm_i = NM_I(sbi);
	unsigned int block_off = NAT_BLOCK_OFFSET(nid);

	f2fs_change_bit(block_off, nm_i->nat_bitmap);
}

static inline void set_to_next_sit(struct f2fs_sb_info *sbi, unsigned int start)
{
	struct sit_info *sit_i = SIT_I(sbi);
	unsigned int block_off = SIT_BLOCK_OFFSET(sit_i, start);

	f2fs_change_bit(block_off, sit_i->sit_bitmap);
}

static int hmfs_init_nid_bitmap(struct f2fs_sb_info *sbi)
{
	struct f2fs_nm_info *nm_i = NM_I(sbi);
	int nid_bitmap_size = (nm_i->max_nid + BITS_PER_BYTE - 1) / BITS_PER_BYTE;
	struct curseg_info *curseg = CURSEG_I(sbi, CURSEG_HOT_DATA);
	struct f2fs_journal *journal = curseg->journal;
	struct f2fs_nat_block *nat_block;
	block_t start_blk;
	nid_t nid;
	int i;
	unsigned long nat_journal_entries;

	if (!(c.func == SLOAD || c.func == FSCK))
		return 0;

	nm_i->nid_bitmap = (char *)calloc(nid_bitmap_size, 1);
	if (!nm_i->nid_bitmap)
		return -ENOMEM;

	/* arbitrarily set 0 bit */
	f2fs_set_bit(0, nm_i->nid_bitmap);

	nat_block = malloc(F2FS_BLKSIZE);
	if (!nat_block) {
		free(nm_i->nid_bitmap);
		return -ENOMEM;
	}

	for (nid = 0; nid < nm_i->max_nid; nid++) {
		if (!(nid % NAT_ENTRY_PER_BLOCK)) {
			int ret;

			start_blk = current_nat_addr(sbi, nid, NULL);
			ret = dev_read_block(nat_block, start_blk);
			ASSERT(ret >= 0);
		}

		if (nat_block->entries[nid % NAT_ENTRY_PER_BLOCK].block_addr)
			f2fs_set_bit(nid, nm_i->nid_bitmap);
	}

	nat_journal_entries = NAT_JOURNAL_ENTRIES;
	if (is_set_ckpt_flags(F2FS_CKPT(sbi), CP_APPEND_NAT_FLAG)) {
		nat_journal_entries += NAT_APPEND_JOURNAL_ENTRIES;
	}

	if (nats_in_cursum(journal) > nat_journal_entries) {
		MSG(0, "\tError: hmfs_init_nid_bitmap truncate n_nats(%u) to "
			"NAT_JOURNAL_ENTRIES(%lu)\n",
			nats_in_cursum(journal), nat_journal_entries);
		journal->n_nats = cpu_to_le16(nat_journal_entries);
	}

	for (i = 0; i < nats_in_cursum(journal); i++) {
		block_t addr;

		addr = le32_to_cpu(nat_in_journal(journal, i).block_addr);
		if (!IS_VALID_BLK_ADDR(sbi, addr)) {
			MSG(0, "\tError: hmfs_init_nid_bitmap: addr(%u) is invalid!!!\n", addr);
			journal->n_nats = cpu_to_le16(i);
			continue;
		}

		nid = le32_to_cpu(nid_in_journal(journal, i));
		if (!IS_VALID_NID(sbi, nid)) {
			MSG(0, "\tError: hmfs_init_nid_bitmap: nid(%u) is invalid!!!\n", nid);
			journal->n_nats = cpu_to_le16(i);
			continue;
		}
		if (addr != NULL_ADDR)
			f2fs_set_bit(nid, nm_i->nid_bitmap);
	}
	free(nat_block);
	return 0;
}

int init_node_manager(struct f2fs_sb_info *sbi)
{
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);
	struct f2fs_nm_info *nm_i = NM_I(sbi);
	unsigned char *version_bitmap;
	unsigned int nat_segs;

	nm_i->nat_blkaddr = get_sb(nat_blkaddr);

	/* segment_count_nat includes pair segment so divide to 2. */
	nat_segs = get_sb(segment_count_nat) >> 1;
	nm_i->nat_blocks = nat_segs << get_sb(log_blocks_per_seg);
	nm_i->max_nid = NAT_ENTRY_PER_BLOCK * nm_i->nat_blocks;
	nm_i->fcnt = 0;
	nm_i->nat_cnt = 0;
	nm_i->init_scan_nid = get_cp(next_free_nid);
	nm_i->next_scan_nid = get_cp(next_free_nid);

	nm_i->bitmap_size = __bitmap_size(sbi, NAT_BITMAP);

	nm_i->nat_bitmap = malloc(nm_i->bitmap_size);
	if (!nm_i->nat_bitmap)
		return -ENOMEM;
	version_bitmap = __bitmap_ptr(sbi, NAT_BITMAP);
	if (!version_bitmap)
		return -EFAULT;

	/* copy version bitmap */
	memcpy(nm_i->nat_bitmap, version_bitmap, nm_i->bitmap_size);
	return hmfs_init_nid_bitmap(sbi);
}

int build_node_manager(struct f2fs_sb_info *sbi)
{
	int err;
	sbi->nm_info = malloc(sizeof(struct f2fs_nm_info));
	if (!sbi->nm_info)
		return -ENOMEM;

	err = init_node_manager(sbi);
	if (err)
		return err;

	return 0;
}

int build_sit_info(struct f2fs_sb_info *sbi)
{
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);
	struct sit_info *sit_i;
	unsigned int sit_segs;
	int start;
	char *src_bitmap, *dst_bitmap;
	unsigned int bitmap_size;

	sit_i = malloc(sizeof(struct sit_info));
	if (!sit_i) {
		MSG(1, "\tError: Malloc failed for build_sit_info!\n");
		return -ENOMEM;
	}

	SM_I(sbi)->sit_info = sit_i;

	sit_i->sentries = calloc(TOTAL_SEGS(sbi) * sizeof(struct seg_entry), 1);
	if (!sit_i->sentries) {
		MSG(1, "\tError: Calloc failed for build_sit_info!\n");
		goto free_sit_info;
	}

	for (start = 0; start < TOTAL_SEGS(sbi); start++) {
		sit_i->sentries[start].cur_valid_map
			= calloc(SIT_VBLOCK_MAP_SIZE, 1);
		if (!sit_i->sentries[start].cur_valid_map) {
			MSG(1, "\tError: Calloc failed for build_sit_info!!\n");
			goto free_validity_maps;
		}
	}

	sit_segs = get_sb(segment_count_sit) >> 1;
	bitmap_size = __bitmap_size(sbi, SIT_BITMAP);
	src_bitmap = __bitmap_ptr(sbi, SIT_BITMAP);

	dst_bitmap = malloc(bitmap_size);
	if (!dst_bitmap) {
		MSG(1, "\tError: Malloc failed for build_sit_info!!\n");
		goto free_validity_maps;
	}

	memcpy(dst_bitmap, src_bitmap, bitmap_size);

	sit_i->sit_base_addr = get_sb(sit_blkaddr);
	sit_i->sit_blocks = sit_segs << sbi->log_blocks_per_seg;
	sit_i->written_valid_blocks = get_cp(valid_block_count);
	sit_i->sit_bitmap = dst_bitmap;
	sit_i->bitmap_size = bitmap_size;
	sit_i->dirty_sentries = 0;
	sit_i->sents_per_block = SIT_ENTRY_PER_BLOCK;
	sit_i->elapsed_time = get_cp(elapsed_time);
	return 0;

free_validity_maps:
	for (--start ; start >= 0; --start)
		free(sit_i->sentries[start].cur_valid_map);
	free(sit_i->sentries);

free_sit_info:
	free(sit_i);

	return -ENOMEM;
}

void reset_curseg(struct f2fs_sb_info *sbi, int type)
{
	struct curseg_info *curseg = CURSEG_I(sbi, type);
	struct summary_footer *sum_footer;
	struct seg_entry *se;

	sum_footer = &(curseg->sum_blk->footer);
	memset(sum_footer, 0, sizeof(struct summary_footer));
	if (IS_DATASEG(type))
		SET_SUM_TYPE(sum_footer, SUM_TYPE_DATA);
	if (IS_NODESEG(type))
		SET_SUM_TYPE(sum_footer, SUM_TYPE_NODE);
	se = get_seg_entry(sbi, curseg->segno);
	se->type = type;
	se->orig_type = type;
	se->dirty = 1;
}

static void read_compacted_summaries(struct f2fs_sb_info *sbi)
{
	struct curseg_info *curseg;
	unsigned int i, j, offset;
	block_t start;
	char *kaddr;
	int ret;

	start = start_sum_block(sbi);

	kaddr = (char *)malloc(PAGE_SIZE);
	ASSERT(kaddr);

	ret = dev_read_block(kaddr, start++);
	ASSERT(ret >= 0);

	curseg = CURSEG_I(sbi, CURSEG_HOT_DATA);
	memcpy(&curseg->journal->n_nats, kaddr, SUM_JOURNAL_SIZE);

	curseg = CURSEG_I(sbi, CURSEG_COLD_DATA);
	memcpy(&curseg->journal->n_sits, kaddr + SUM_JOURNAL_SIZE,
						SUM_JOURNAL_SIZE);

	offset = 2 * SUM_JOURNAL_SIZE;
	for (i = CURSEG_HOT_DATA; i <= CURSEG_COLD_DATA; i++) {
		unsigned short blk_off;
		curseg = CURSEG_I(sbi, i);

		if (curseg->segno == NULL_SEGNO &&
			(sbi->active_logs == NR_UNIFY_CURSEG_TYPE) &&
			(i == CURSEG_WARM_DATA)) {
			continue;
		}

		reset_curseg(sbi, i);
		if (curseg->alloc_type == SSR)
			blk_off = sbi->blocks_per_seg;
		else
			blk_off = curseg->next_blkoff;

		ASSERT(blk_off <= ENTRIES_IN_SUM);

		for (j = 0; j < blk_off; j++) {
			struct f2fs_summary *s;
			s = (struct f2fs_summary *)(kaddr + offset);
			curseg->sum_blk->entries[j] = *s;
			offset += SUMMARY_SIZE;
			if (offset + SUMMARY_SIZE <=
					PAGE_CACHE_SIZE - SUM_FOOTER_SIZE)
				continue;
			memset(kaddr, 0, PAGE_SIZE);
			ret = dev_read_block(kaddr, start++);
			ASSERT(ret >= 0);
			offset = 0;
		}
	}
	free(kaddr);
}

static void restore_node_summary(struct f2fs_sb_info *sbi,
		unsigned int segno, struct f2fs_summary_block *sum_blk)
{
	struct f2fs_node *node_blk;
	struct f2fs_summary *sum_entry;
	block_t addr;
	unsigned int i;
	int ret;

	node_blk = malloc(F2FS_BLKSIZE);
	ASSERT(node_blk);

	/* scan the node segment */
	addr = START_BLOCK(sbi, segno);
	sum_entry = &sum_blk->entries[0];

	for (i = 0; i < sbi->blocks_per_seg; i++, sum_entry++) {
		ret = dev_read_block(node_blk, addr);
		ASSERT(ret >= 0);
		sum_entry->nid = node_blk->footer.nid;
		addr++;
	}
	free(node_blk);
}

static void read_normal_summaries(struct f2fs_sb_info *sbi, int type)
{
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);
	struct f2fs_summary_block *sum_blk;
	struct curseg_info *curseg;
	unsigned int segno = 0;
	block_t blk_addr = 0;
	int ret;

	curseg = CURSEG_I(sbi, type);
	if (curseg->segno == NULL_SEGNO &&
			(sbi->active_logs == NR_UNIFY_CURSEG_TYPE) &&
			(type == CURSEG_WARM_NODE || type == CURSEG_WARM_DATA))
		return;

	if (IS_DATASEG(type)) {
		segno = get_cp(cur_data_segno[type]);
		if (is_set_ckpt_flags(cp, CP_UMOUNT_FLAG))
			blk_addr = sum_blk_addr(sbi, NR_CURSEG_TYPE, type);
		else
			blk_addr = sum_blk_addr(sbi, NR_CURSEG_DATA_TYPE + NR_CURSEG_DM_TYPE, type);
		if(!is_set_ckpt_flags(cp, CP_DATAMOVE_FLAG))
				blk_addr += NR_CURSEG_DM_TYPE;
	} else if (IS_NODESEG(type)){
		segno = get_cp(cur_node_segno[type - CURSEG_HOT_NODE]);
		if (is_set_ckpt_flags(cp, CP_UMOUNT_FLAG)) {
			blk_addr = sum_blk_addr(sbi, NR_CURSEG_NODE_TYPE + NR_CURSEG_DM_TYPE,
					type - CURSEG_HOT_NODE);
			if(!is_set_ckpt_flags(cp, CP_DATAMOVE_FLAG))
				blk_addr += NR_CURSEG_DM_TYPE;
		} else {
			blk_addr = GET_SUM_BLKADDR(sbi, segno);
		}
	} else {
		segno = get_cp(cur_dmgc_segno[type - CURSEG_DATA_MOVE1]);
		blk_addr = sum_blk_addr(sbi, NR_CURSEG_DM_TYPE,
				type - CURSEG_DATA_MOVE1);
		ASSERT(is_set_ckpt_flags(cp, CP_DATAMOVE_FLAG));
	}

	sum_blk = (struct f2fs_summary_block *)malloc(PAGE_SIZE);
	ASSERT(sum_blk);

	ret = dev_read_block(sum_blk, blk_addr);
	ASSERT(ret >= 0);

	if (IS_NODESEG(type) && !is_set_ckpt_flags(cp, CP_UMOUNT_FLAG))
		restore_node_summary(sbi, segno, sum_blk);

	memcpy(curseg->sum_blk, sum_blk, PAGE_CACHE_SIZE);
	/* copy node journal */
	memcpy(curseg->journal, &sum_blk->journal, SUM_JOURNAL_SIZE);
	reset_curseg(sbi, type);
	free(sum_blk);
}

static void read_append_journal(struct f2fs_sb_info *sbi)
{
	struct curseg_info *curseg;
	char *kaddr = NULL;
	block_t start_blk;
	int ret;
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);

	start_blk = __start_cp_addr(sbi) +
		    le32_to_cpu(F2FS_CKPT(sbi)->cp_pack_total_block_count);

	kaddr = (char *)malloc(PAGE_SIZE);
	ASSERT(kaddr);

	if (is_set_ckpt_flags(cp, CP_APPEND_NAT_FLAG)) {
		curseg = CURSEG_I(sbi, CURSEG_HOT_DATA);
		memset(kaddr, 0, PAGE_SIZE); //lint !e668
		ret = dev_read_block(kaddr, start_blk++);
		ASSERT(ret >= 0);
		memcpy((char *)curseg->journal + SUM_JOURNAL_SIZE - NAT_JOURNAL_RESERVED,
		       kaddr,
		       NAT_APPEND_JOURNAL_ENTRIES *
		       sizeof(struct nat_journal_entry));
	}

	if (is_set_ckpt_flags(cp, CP_APPEND_SIT_FLAG)) {
		curseg = CURSEG_I(sbi, CURSEG_COLD_DATA);
		memset(kaddr, 0, PAGE_SIZE); //lint !e668
		ret = dev_read_block(kaddr, start_blk);
		ASSERT(ret >= 0);
		memcpy((char *)curseg->journal + SUM_JOURNAL_SIZE - SIT_JOURNAL_RESERVED,
		       kaddr,
		       SIT_APPEND_JOURNAL_ENTRIES *
		       sizeof(struct sit_journal_entry));
	}
	free(kaddr);
}

static void restore_curseg_summaries(struct f2fs_sb_info *sbi)
{
	int type = CURSEG_HOT_DATA;

	if (is_set_ckpt_flags(F2FS_CKPT(sbi), CP_COMPACT_SUM_FLAG)) {
		read_compacted_summaries(sbi);
		type = CURSEG_HOT_NODE;
	}

	for (; type < NR_CURSEG_TYPE; type++) {
		if (!is_set_ckpt_flags(F2FS_CKPT(sbi),CP_DATAMOVE_FLAG) &&
						IS_DM_SEG(type))
			continue;
		read_normal_summaries(sbi, type);
	}

	if (is_set_ckpt_flags(F2FS_CKPT(sbi),CP_APPEND_NAT_FLAG) ||
		is_set_ckpt_flags(F2FS_CKPT(sbi),CP_APPEND_SIT_FLAG)) {
		read_append_journal(sbi);
	}
}

static int build_curseg(struct f2fs_sb_info *sbi)
{
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);
	struct curseg_info *array;
	unsigned short blk_off;
	unsigned int segno;
	int i;
	unsigned int append;

	array = malloc(sizeof(*array) * NR_CURSEG_TYPE);
	if (!array) {
		MSG(1, "\tError: Malloc failed for build_curseg!\n");
		return -ENOMEM;
	}

	SM_I(sbi)->curseg_array = array;

	for (i = 0; i < NR_CURSEG_TYPE; i++) {
		append = 0;
		if (i == CURSEG_HOT_DATA && is_set_ckpt_flags(cp,CP_APPEND_NAT_FLAG)) {
			append = NAT_APPEND_JOURNAL_ENTRIES *
				 sizeof(struct nat_journal_entry) -
				 NAT_JOURNAL_RESERVED;
		}
		if (i == CURSEG_COLD_DATA && is_set_ckpt_flags(cp,CP_APPEND_SIT_FLAG)) {
			append = SIT_APPEND_JOURNAL_ENTRIES *
				 sizeof(struct sit_journal_entry) -
				 SIT_JOURNAL_RESERVED;
		}
		array[i].sum_blk = calloc(PAGE_CACHE_SIZE, 1);
		if (!array[i].sum_blk) {
			MSG(1, "\tError: Calloc failed for build_curseg!!\n");
			goto seg_cleanup;
		}
		append = (append + PAGE_CACHE_SIZE - 1) & (~(PAGE_CACHE_SIZE - 1));
		array[i].journal = malloc(sizeof(struct f2fs_journal) + append);
		ASSERT(array[i].journal);
		memset(array[i].journal, 0, sizeof(struct f2fs_journal) + append);

		if (i <= CURSEG_COLD_DATA) {
			blk_off = get_cp(cur_data_blkoff[i]);
			segno = get_cp(cur_data_segno[i]);
		}
		if (i >= CURSEG_HOT_NODE && i <= CURSEG_COLD_NODE) {
			blk_off = get_cp(cur_node_blkoff[i - CURSEG_HOT_NODE]);
			segno = get_cp(cur_node_segno[i - CURSEG_HOT_NODE]);
		}
		if (IS_DM_SEG(i)) {
			if (is_set_ckpt_flags(F2FS_CKPT(sbi),CP_DATAMOVE_FLAG)) {
				blk_off = get_cp(cur_dmgc_blkoff[i - CURSEG_DATA_MOVE1]);
				segno = get_cp(cur_dmgc_segno[i - CURSEG_DATA_MOVE1]);
			} else {
				blk_off = 0;
				segno = NULL_SEGNO;
				goto seg_set;
			}
		}
		if ((sbi->active_logs == NR_UNIFY_CURSEG_TYPE) &&
				(i == CURSEG_WARM_NODE || i == CURSEG_WARM_DATA) &&
				segno == NULL_SEGNO) {
			blk_off = 0;
		} else {
			ASSERT(segno < TOTAL_SEGS(sbi));
			ASSERT(blk_off < DEFAULT_BLOCKS_PER_SEGMENT);
		}

seg_set:
		array[i].segno = segno;
		array[i].zone = GET_ZONENO_FROM_SEGNO(sbi, segno);
		array[i].next_segno = NULL_SEGNO;
		array[i].next_blkoff = blk_off;
		array[i].alloc_type = cp->alloc_type[i];
	}
	restore_curseg_summaries(sbi);
	return 0;

seg_cleanup:
	for(--i ; i >=0; --i)
		free(array[i].sum_blk);
	free(array);

	return -ENOMEM;
}

static inline void check_seg_range(struct f2fs_sb_info *sbi, unsigned int segno)
{
	unsigned int end_segno = SM_I(sbi)->segment_count - 1;
	ASSERT(segno <= end_segno);
}

pgoff_t current_sit_addr(struct f2fs_sb_info *sbi, unsigned int segno)
{
	struct sit_info *sit_i = SIT_I(sbi);
	unsigned int offset = SIT_BLOCK_OFFSET(sit_i, segno);
	block_t blk_addr = sit_i->sit_base_addr + offset;

	check_seg_range(sbi, segno);
	/* calculate sit block address */
	if (f2fs_test_bit(offset, sit_i->sit_bitmap))
		blk_addr += sit_i->sit_blocks;

	return blk_addr;
}

void get_current_sit_page(struct f2fs_sb_info *sbi,
			unsigned int segno, struct f2fs_sit_block *sit_blk)
{
	struct sit_info *sit_i = SIT_I(sbi);
	unsigned int offset = SIT_BLOCK_OFFSET(sit_i, segno);
	block_t blk_addr = sit_i->sit_base_addr + offset;
	int ret;

	check_seg_range(sbi, segno);

	/* calculate sit block address */
	if (f2fs_test_bit(offset, sit_i->sit_bitmap))
		blk_addr += sit_i->sit_blocks;

	ret = dev_read_block(sit_blk, blk_addr);
	ASSERT(ret >= 0);
}

void rewrite_current_sit_page(struct f2fs_sb_info *sbi,
			unsigned int segno, struct f2fs_sit_block *sit_blk)
{
	struct sit_info *sit_i = SIT_I(sbi);
	unsigned int offset = SIT_BLOCK_OFFSET(sit_i, segno);
	block_t blk_addr = sit_i->sit_base_addr + offset;
	int ret;

	/* calculate sit block address */
	if (f2fs_test_bit(offset, sit_i->sit_bitmap))
		blk_addr += sit_i->sit_blocks;

	ret = dev_write_block_no_sync(sit_blk, blk_addr);
	ASSERT(ret >= 0);
}

void check_block_count(struct f2fs_sb_info *sbi,
		unsigned int segno, struct f2fs_sit_entry *raw_sit)
{
	struct f2fs_sm_info *sm_info = SM_I(sbi);
	unsigned int end_segno = sm_info->segment_count - 1;
	int valid_blocks = 0;
	unsigned int i;

	/* check segment usage */
	if (GET_SIT_VBLOCKS(raw_sit) > sbi->blocks_per_seg) {
		DMD_ADD_ERROR(LOG_TYP_FSCK, PR_INVALID_SIT_VBLOCKS);
		ASSERT_MSG("Invalid SIT vblocks: segno=0x%x, %u",
				segno, GET_SIT_VBLOCKS(raw_sit));
	}

	/* check boundary of a given segment number */
	if (segno > end_segno) {
		DMD_ADD_ERROR(LOG_TYP_FSCK, PR_INVALID_SEGNO);
		ASSERT_MSG("Invalid SEGNO: 0x%x", segno);
	}

	/* check bitmap with valid block count */
	for (i = 0; i < SIT_VBLOCK_MAP_SIZE; i++)
		valid_blocks += get_bits_in_byte(raw_sit->valid_map[i]);

	if (GET_SIT_VBLOCKS(raw_sit) != valid_blocks) {
		DMD_ADD_ERROR(LOG_TYP_FSCK, PR_SIT_VBLOCKS_IS_ERROR);
		ASSERT_MSG("Wrong SIT valid blocks: segno=0x%x, %u vs. %u",
				segno, GET_SIT_VBLOCKS(raw_sit), valid_blocks);
	}

	if (GET_SIT_TYPE(raw_sit) >= NO_CHECK_TYPE) {
		DMD_ADD_ERROR(LOG_TYP_FSCK, PR_INVALID_SIT_TYPE);
		ASSERT_MSG("Wrong SIT type: segno=0x%x, %u",
				segno, GET_SIT_TYPE(raw_sit));
	}
}

void seg_info_from_raw_sit(struct seg_entry *se,
		struct f2fs_sit_entry *raw_sit)
{
	se->valid_blocks = GET_SIT_VBLOCKS(raw_sit);
	memcpy(se->cur_valid_map, raw_sit->valid_map, SIT_VBLOCK_MAP_SIZE);
	se->type = GET_SIT_TYPE(raw_sit);
	se->orig_type = GET_SIT_TYPE(raw_sit);
	se->mtime = le64_to_cpu(raw_sit->mtime);
}

struct seg_entry *get_seg_entry(struct f2fs_sb_info *sbi,
		unsigned int segno)
{
	struct sit_info *sit_i = SIT_I(sbi);
	return &sit_i->sentries[segno];
}

struct f2fs_summary_block *get_sum_block(struct f2fs_sb_info *sbi,
				unsigned int segno, int *ret_type)
{
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);
	struct f2fs_summary_block *sum_blk;
	struct curseg_info *curseg;
	int type, ret;
	u64 ssa_blk;

	*ret_type= SEG_TYPE_MAX;

	ssa_blk = GET_SUM_BLKADDR(sbi, segno);

	for (type = 0; type < NR_CURSEG_NODE_TYPE; type++) {
		if (segno == get_cp(cur_node_segno[type])) {
			curseg = CURSEG_I(sbi, CURSEG_HOT_NODE + type);
			if (!IS_SUM_NODE_SEG(curseg->sum_blk->footer)) {
				ASSERT_MSG("segno [0x%x] indicates a data "
						"segment, but should be node",
						segno);
				*ret_type = -SEG_TYPE_CUR_NODE;
			} else {
				*ret_type = SEG_TYPE_CUR_NODE;
			}
			return curseg->sum_blk;
		}
	}

	for (type = 0; type < NR_CURSEG_DATA_TYPE; type++) {
		if (segno == get_cp(cur_data_segno[type])) {
			curseg = CURSEG_I(sbi, type);
			if (IS_SUM_NODE_SEG(curseg->sum_blk->footer)) {
				ASSERT_MSG("segno [0x%x] indicates a node "
						"segment, but should be data",
						segno);
				*ret_type = -SEG_TYPE_CUR_DATA;
			} else {
				*ret_type = SEG_TYPE_CUR_DATA;
			}
			return curseg->sum_blk;
		}
	}

	for (type = 0; type < NR_CURSEG_DM_TYPE; type++) {
		if (segno == get_cp(cur_dmgc_segno[type])) {
			curseg = CURSEG_I(sbi, CURSEG_DATA_MOVE1 + type);
			if (IS_SUM_NODE_SEG(curseg->sum_blk->footer)) {
				ASSERT_MSG("segno [0x%x] indicates a node "
						"segment, but should be data of datamove",
						segno);
				*ret_type = -SEG_TYPE_CUR_DATA;
			} else {
				*ret_type = SEG_TYPE_CUR_DATA;
			}
			return curseg->sum_blk;
		}
	}

	sum_blk = calloc(BLOCK_SZ, 1);
	ASSERT(sum_blk);

	ret = dev_read_block(sum_blk, ssa_blk);
	ASSERT(ret >= 0);

	if (IS_SUM_NODE_SEG(sum_blk->footer))
		*ret_type = SEG_TYPE_NODE;
	else if (IS_SUM_DATA_SEG(sum_blk->footer))
		*ret_type = SEG_TYPE_DATA;

	return sum_blk;
}

int get_sum_entry(struct f2fs_sb_info *sbi, u32 blk_addr,
				struct f2fs_summary *sum_entry)
{
	struct f2fs_summary_block *sum_blk;
	u32 segno, offset;
	int type;

	segno = GET_SEGNO(sbi, blk_addr);
	offset = OFFSET_IN_SEG(sbi, blk_addr);

	sum_blk = get_sum_block(sbi, segno, &type);
	memcpy(sum_entry, &(sum_blk->entries[offset]),
				sizeof(struct f2fs_summary));
	if (type == SEG_TYPE_NODE || type == SEG_TYPE_DATA ||
					type == SEG_TYPE_MAX)
		free(sum_blk);
	return type;
}

static void get_nat_entry(struct f2fs_sb_info *sbi, nid_t nid,
				struct f2fs_nat_entry *raw_nat)
{
	struct f2fs_nat_block *nat_block;
	pgoff_t block_addr;
	int entry_off;
	int ret;

	if (lookup_nat_in_journal(sbi, nid, raw_nat) >= 0)
		return;

	nat_block = (struct f2fs_nat_block *)calloc(BLOCK_SZ, 1);
	ASSERT(nat_block);

	entry_off = nid % NAT_ENTRY_PER_BLOCK;
	block_addr = current_nat_addr(sbi, nid, NULL);

	ret = dev_read_block(nat_block, block_addr);
	ASSERT(ret >= 0);

	memcpy(raw_nat, &nat_block->entries[entry_off],
					sizeof(struct f2fs_nat_entry));
	free(nat_block);
}

void update_data_blkaddr(struct f2fs_sb_info *sbi, nid_t nid,
				u16 ofs_in_node, block_t newaddr)
{
	struct f2fs_node *node_blk = NULL;
	struct node_info ni;
	block_t oldaddr, startaddr, endaddr;
	int ret;

	node_blk = (struct f2fs_node *)calloc(BLOCK_SZ, 1);
	ASSERT(node_blk);

	get_node_info(sbi, nid, &ni);

	/* read node_block */
	ret = dev_read_block(node_blk, ni.blk_addr);
	ASSERT(ret >= 0);

	/* check its block address */
	if (node_blk->footer.nid == node_blk->footer.ino) {
		int ofs = get_extra_isize(node_blk);

		oldaddr = le32_to_cpu(node_blk->i.i_addr[ofs + ofs_in_node]);
		node_blk->i.i_addr[ofs + ofs_in_node] = cpu_to_le32(newaddr);
		if (c.feature & cpu_to_le32(F2FS_FEATURE_INODE_CHKSUM))
			node_blk->i.i_inode_checksum =
				cpu_to_le32(f2fs_inode_chksum(node_blk));
	} else {
		oldaddr = le32_to_cpu(node_blk->dn.addr[ofs_in_node]);
		node_blk->dn.addr[ofs_in_node] = cpu_to_le32(newaddr);
	}

	if (c.func == RESIZE) {
		ret = dev_write_block(node_blk, ni.blk_addr);
	} else {
		ret = f2fs_write_block(sbi, node_blk, ni.blk_addr, 0);
	}
	ASSERT(ret >= 0);

	get_node_info(sbi, le32_to_cpu(node_blk->footer.ino), &ni);

	/* check extent cache entry */
	if (node_blk->footer.nid != node_blk->footer.ino) {
		/* read inode block */
		ret = dev_read_block(node_blk, ni.blk_addr);
		ASSERT(ret >= 0);
	}

	startaddr = le32_to_cpu(node_blk->i.i_ext.blk_addr);
	endaddr = startaddr + le32_to_cpu(node_blk->i.i_ext.len);
	if (oldaddr >= startaddr && oldaddr < endaddr) {
		node_blk->i.i_ext.len = 0;

		/* update inode block */
		write_inode(sbi, ni.blk_addr, node_blk);
	}
	free(node_blk);
}

void update_nat_blkaddr(struct f2fs_sb_info *sbi, nid_t ino,
					nid_t nid, block_t newaddr)
{
	struct f2fs_nat_block *nat_block;
	pgoff_t block_addr;
	int entry_off;
	int ret;

	if (c.func == FSCK) {
		if (nid >= F2FS_FSCK(sbi)->nr_nat_entries)
			return;
		if (ino)
			F2FS_FSCK(sbi)->entries[nid].ino = cpu_to_le32(ino);

		F2FS_FSCK(sbi)->entries[nid].block_addr =
						cpu_to_le32(newaddr);
		f2fs_set_bit(nid, F2FS_FSCK(sbi)->nat_area_dirty_bitmap);
		return;
	}

	nat_block = (struct f2fs_nat_block *)calloc(BLOCK_SZ, 1);
	ASSERT(nat_block);

	entry_off = nid % NAT_ENTRY_PER_BLOCK;
	block_addr = current_nat_addr(sbi, nid, NULL);

	ret = dev_read_block(nat_block, block_addr);
	ASSERT(ret >= 0);

	if (ino)
		nat_block->entries[entry_off].ino = cpu_to_le32(ino);
	nat_block->entries[entry_off].block_addr = cpu_to_le32(newaddr);

	if (c.func == RESIZE) {
		ret = dev_write_block(nat_block, block_addr);
	} else {
		ret = f2fs_write_block(sbi, nat_block, block_addr, 1);
	}
	ASSERT(ret >= 0);
	free(nat_block);
}

void get_node_info(struct f2fs_sb_info *sbi, nid_t nid, struct node_info *ni)
{
	struct f2fs_nat_entry raw_nat;

	ni->nid = nid;
	if (c.func == FSCK) {
		ASSERT(nid < F2FS_FSCK(sbi)->nr_nat_entries);
		node_info_from_raw_nat(ni, &(F2FS_FSCK(sbi)->entries[nid]));
		if (ni->blk_addr)
			return;
		/* nat entry is not cached, read it */
	}

	get_nat_entry(sbi, nid, &raw_nat);
	node_info_from_raw_nat(ni, &raw_nat);

	if (c.func == FSCK) {
		F2FS_FSCK(sbi)->entries[nid] = raw_nat;
	}
}

void check_sit_nat_journal(struct f2fs_sb_info *sbi)
{
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	struct curseg_info *cold_data_curseg = CURSEG_I(sbi, CURSEG_COLD_DATA);
	struct curseg_info *hot_data_curseg = CURSEG_I(sbi, CURSEG_HOT_DATA);
	struct f2fs_journal *sit_journal = cold_data_curseg->journal;
	struct f2fs_journal *nat_journal = hot_data_curseg->journal;
	unsigned int i, segno, sit_journal_num, nat_journal_num, sit_journal_entries, nat_journal_entries;
	u32 nid, nr_nat_blks, max_nid;
	nr_nat_blks = ((get_sb(segment_count_nat) / 2) <<
				sbi->log_blocks_per_seg);
	max_nid = nr_nat_blks * NAT_ENTRY_PER_BLOCK;

	sit_journal_num = sits_in_cursum(sit_journal);
	nat_journal_num = nats_in_cursum(nat_journal);
	sit_journal_entries = SIT_JOURNAL_ENTRIES;
	nat_journal_entries = NAT_JOURNAL_ENTRIES;

	if (is_set_ckpt_flags(F2FS_CKPT(sbi),CP_APPEND_SIT_FLAG)) {
		sit_journal_entries += SIT_APPEND_JOURNAL_ENTRIES;
	}

	if (is_set_ckpt_flags(F2FS_CKPT(sbi),CP_APPEND_NAT_FLAG)) {
		nat_journal_entries += NAT_APPEND_JOURNAL_ENTRIES;
	}

	/*check sit journal value*/
	if (sit_journal_num > sit_journal_entries) {
		ASSERT_MSG("Invalid sit journal num: %u", sit_journal_num);
		sit_journal->n_sits = cpu_to_le16(sit_journal_entries);
	}

	for (i = 0; i < sits_in_cursum(sit_journal); i++) {
		segno = le32_to_cpu(segno_in_journal(sit_journal, i));
		if (segno > (TOTAL_SEGS(sbi) - 1)) {
			ASSERT_MSG("Invalid SEGNO: 0x%x", segno);
		}
	}

	/*check nat journal value*/
	if (nat_journal_num > nat_journal_entries) {
		ASSERT_MSG("Invalid nat journal num: %u", nat_journal_num);
		nat_journal->n_nats = cpu_to_le16(nat_journal_entries);
	}

	for (i = 0; i < nats_in_cursum(nat_journal); i++) {
		nid = le32_to_cpu(nid_in_journal(nat_journal, i));
		if (nid >= max_nid ) {
			ASSERT_MSG("Invalid nid: 0%x", nid);
			nid_in_journal(nat_journal, i) = 0;
			memset(&nat_in_journal(nat_journal, i), 0,
				sizeof(struct f2fs_nat_entry));
			FIX_MSG("Remove nid [0x%x] in nat journal\n", nid);
		}
	}
}

void set_free(struct f2fs_sb_info *sbi, unsigned int segno)
{
	struct free_segmap_info *free_i = FREE_I(sbi);
	unsigned int secno = GET_SEC_FROM_SEG(sbi, segno);
	unsigned int start_segno = GET_SEG_FROM_SEC(sbi, secno);
	unsigned int next;

	clear_bit_le(segno, free_i->free_segmap);
	free_i->free_segments++;

	if (IS_CURSEC(sbi, secno)) {
		return;
	}
	next = find_next_bit_le(free_i->free_segmap,
			start_segno + sbi->segs_per_sec, start_segno);
	if (next >= start_segno + sbi->segs_per_sec) {
		clear_bit_le(secno, free_i->free_secmap);
		free_i->free_sections++;
	}
}

static int init_free_segmap(struct f2fs_sb_info *sbi)
{
	struct free_segmap_info *free_i;
	unsigned int bitmap_size, sec_bitmap_size;

	/* allocate memory for free segmap information */
	free_i = malloc(sizeof(struct free_segmap_info));
	if (!free_i) {
		return -ENOMEM;
	}

	bitmap_size = (TOTAL_SEGS(sbi) + 7) / 8;
	free_i->free_segmap = malloc(bitmap_size);
	if (!free_i->free_segmap) {
		goto free_i_clean;
	}

	sec_bitmap_size = (MAIN_SECS(sbi) + 7) / 8;
	free_i->free_secmap = malloc(sec_bitmap_size);
	if (!free_i->free_secmap) {
		goto free_segmap_clean;
	}
	free_i->prefree_secmap = malloc(sec_bitmap_size);
	if (!free_i->prefree_secmap) {
		goto free_secmap_clean;
	}
	free_i->free_dm_secmap = malloc(sec_bitmap_size);
	if (!free_i->free_dm_secmap) {
		goto prefree_secmap_clean;
	}
	free_i->free_dm_segmap = malloc(bitmap_size);
	if (!free_i->free_dm_segmap) {
		goto prefree_segmap_clean;
	}
	memset(free_i->free_dm_secmap, 0, sec_bitmap_size);
	memset(free_i->free_dm_segmap, 0, bitmap_size);

	/* set all segments as dirty temporarily */
	memset(free_i->free_segmap, 0xff, bitmap_size);
	memset(free_i->free_secmap, 0xff, sec_bitmap_size);
	memset(free_i->prefree_secmap, 0, sec_bitmap_size);

	/* init free segmap information */
	free_i->free_segments = 0;
	free_i->free_sections = 0;

	SM_I(sbi)->free_info = free_i;
	return 0;

prefree_segmap_clean:
	free(free_i->free_dm_secmap);
prefree_secmap_clean:
	free(free_i->prefree_secmap);
free_secmap_clean:
	free(free_i->free_secmap);
free_segmap_clean:
	free(free_i->free_segmap);
free_i_clean:
	free(free_i);

	return -ENOMEM;
}


static int build_free_segmap(struct f2fs_sb_info *sbi)
{
	unsigned int start;
	int type, ret;
	struct curseg_info *curseg_t = NULL;

	ret = init_free_segmap(sbi);
	if (ret)
		return ret;

	for (start = 0; start < TOTAL_SEGS(sbi); start++) {
		struct seg_entry *sentry = get_seg_entry(sbi, start);

		if (!sentry->valid_blocks)
			set_free(sbi, start);
	}

	/* set use the current segments */
	for (type = CURSEG_HOT_DATA; type < NR_CURSEG_TYPE; type++) {
		if (!is_set_ckpt_flags(F2FS_CKPT(sbi), CP_DATAMOVE_FLAG) &&
				IS_DM_SEG(type)) {
			continue;
		}

		curseg_t = CURSEG_I(sbi, type);
		if (curseg_t->segno == NULL_SEGNO &&
				(sbi->active_logs == NR_UNIFY_CURSEG_TYPE) &&
				(type == CURSEG_WARM_NODE || type == CURSEG_WARM_DATA))
			continue;

		__set_test_and_inuse(sbi, curseg_t->segno);
	}

	return 0;
}

static int build_sit_entries(struct f2fs_sb_info *sbi)
{
	struct sit_info *sit_i = SIT_I(sbi);
	struct curseg_info *curseg = CURSEG_I(sbi, CURSEG_COLD_DATA);
	struct f2fs_journal *journal = curseg->journal;
	struct f2fs_sit_block *sit_blk;
	struct seg_entry *se;
	struct f2fs_sit_entry sit;
	unsigned int i, segno;

	sit_blk = calloc(BLOCK_SZ, 1);
	if (!sit_blk) {
		MSG(1, "\tError: Calloc failed for build_sit_entries!\n");
		return -ENOMEM;
	}

	for (segno = 0; segno < TOTAL_SEGS(sbi); segno++) {
		se = &sit_i->sentries[segno];

		get_current_sit_page(sbi, segno, sit_blk);
		sit = sit_blk->entries[SIT_ENTRY_OFFSET(sit_i, segno)];

		check_block_count(sbi, segno, &sit);
		seg_info_from_raw_sit(se, &sit);
	}

	free(sit_blk);
	for (i = 0; i < sits_in_cursum(journal); i++) {
		segno = le32_to_cpu(segno_in_journal(journal, i));
		if (segno >= TOTAL_SEGS(sbi))
			continue;
		se = &sit_i->sentries[segno];
		sit = sit_in_journal(journal, i);

		check_block_count(sbi, segno, &sit);
		seg_info_from_raw_sit(se, &sit);

		/* if do fsck we need to flush sit journal */
		se->dirty = 1;
	}
	return 0;
}

static int build_segment_manager(struct f2fs_sb_info *sbi)
{
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);
	struct f2fs_sm_info *sm_info;

	sm_info = malloc(sizeof(struct f2fs_sm_info));
	if (!sm_info) {
		MSG(1, "\tError: Malloc failed for build_segment_manager!\n");
		return -ENOMEM;
	}

	/* init sm info */
	sbi->sm_info = sm_info;
	sm_info->seg0_blkaddr = get_sb(segment0_blkaddr);
	sm_info->main_blkaddr = get_sb(main_blkaddr);
	sm_info->segment_count = get_sb(segment_count);
	sm_info->reserved_segments = get_cp(rsvd_segment_count);
	sm_info->ovp_segments = get_cp(overprov_segment_count);
	sm_info->main_segments = get_sb(segment_count_main);
	sm_info->ssa_blkaddr = get_sb(ssa_blkaddr);

	if (build_sit_info(sbi) || build_curseg(sbi)) {
		free(sm_info);
		return -ENOMEM;
	}

	check_sit_nat_journal(sbi);

	if (build_sit_entries(sbi)) {
		free(sm_info);
		return -ENOMEM;
	}

	return build_free_segmap(sbi);
}

void build_sit_area_bitmap(struct f2fs_sb_info *sbi)
{
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
	struct f2fs_sm_info *sm_i = SM_I(sbi);
	unsigned int segno = 0;
	char *ptr = NULL;
	u32 sum_vblocks = 0;
	u32 free_segs = 0;
	struct seg_entry *se;
	int devid = 0;

	fsck->sit_area_bitmap_sz = sm_i->main_segments * SIT_VBLOCK_MAP_SIZE;
	fsck->sit_area_bitmap = calloc(1, fsck->sit_area_bitmap_sz);
	ASSERT(fsck->sit_area_bitmap);
	ptr = fsck->sit_area_bitmap;

	ASSERT(fsck->sit_area_bitmap_sz == fsck->main_area_bitmap_sz);

	for (segno = 0; segno < TOTAL_SEGS(sbi); segno++) {
		devid = get_device_index(START_BLOCK(sbi, segno));
		if (devid >= 0 && c.devices[devid].reserved) {
			ptr += SIT_VBLOCK_MAP_SIZE;
			continue;
		}

		se = get_seg_entry(sbi, segno);

		memcpy(ptr, se->cur_valid_map, SIT_VBLOCK_MAP_SIZE);
		ptr += SIT_VBLOCK_MAP_SIZE;

		if (se->valid_blocks == 0x0) {
			if (le32_to_cpu(sbi->ckpt->cur_node_segno[0]) == segno ||
				le32_to_cpu(sbi->ckpt->cur_data_segno[0]) == segno ||
				le32_to_cpu(sbi->ckpt->cur_node_segno[1]) == segno ||
				le32_to_cpu(sbi->ckpt->cur_data_segno[1]) == segno ||
				le32_to_cpu(sbi->ckpt->cur_node_segno[2]) == segno ||
				le32_to_cpu(sbi->ckpt->cur_data_segno[2]) == segno) {
				continue;
			} else if (is_set_ckpt_flags(F2FS_CKPT(sbi), CP_DATAMOVE_FLAG) &&
				(le32_to_cpu(sbi->ckpt->cur_dmgc_segno[0]) == segno ||
				le32_to_cpu(sbi->ckpt->cur_dmgc_segno[1]) == segno)) {
				continue;
			} else if (IS_DM_SOURCE_SEGNO(sbi, segno)) {
				continue;
			} else {
				free_segs++;
			}
		} else {
			sum_vblocks += se->valid_blocks;
		}
	}
	fsck->chk.sit_valid_blocks = sum_vblocks;
	fsck->chk.sit_free_segs = free_segs;

	DBG(1, "Blocks [0x%x : %d] Free Segs [0x%x : %d]\n\n",
			sum_vblocks, sum_vblocks,
			free_segs, free_segs);
}

static void rewrite_sit_area_block(struct f2fs_sb_info *sbi, char *ptr,
		unsigned int start_segno, struct f2fs_sit_block *sit_blk)
{
	struct f2fs_sit_entry *sit;
	struct seg_entry *se;
	struct sit_info *sit_i = SIT_I(sbi);
	u16 valid_blocks;
	u16 type;
	int i;
	unsigned int segno;
	pgoff_t next_block_addr;
	pgoff_t cur_block_addr;

	get_current_sit_page(sbi, start_segno, sit_blk);
	for (segno = start_segno; segno < start_segno + SIT_ENTRY_PER_BLOCK &&
			segno < TOTAL_SEGS(sbi); segno++) {
		valid_blocks = 0;
		sit = &sit_blk->entries[SIT_ENTRY_OFFSET(sit_i, segno)];
		memcpy(sit->valid_map, ptr, SIT_VBLOCK_MAP_SIZE);

		/* update valid block count */
		for (i = 0; i < SIT_VBLOCK_MAP_SIZE; i++)
			valid_blocks += get_bits_in_byte(sit->valid_map[i]);

		se = get_seg_entry(sbi, segno);
		memcpy(se->cur_valid_map, ptr, SIT_VBLOCK_MAP_SIZE);
		se->valid_blocks = valid_blocks;
		type = se->type;
		if (type >= NO_CHECK_TYPE) {
			ASSERT_MSG("Invalide type and valid blocks=%x,%x",
					segno, valid_blocks);
			type = 0;
		}
		sit->vblocks = cpu_to_le16((type << SIT_VBLOCKS_SHIFT) |
								valid_blocks);
		ptr += SIT_VBLOCK_MAP_SIZE;
	}
	cur_block_addr = current_sit_addr(sbi, start_segno);
	next_block_addr = next_sit_addr(sbi, cur_block_addr);
	f2fs_write_block(sbi, sit_blk, next_block_addr, true);
	set_to_next_sit(sbi, start_segno);
}

void rewrite_sit_area_bitmap(struct f2fs_sb_info *sbi)
{
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
	struct curseg_info *curseg = CURSEG_I(sbi, CURSEG_COLD_DATA);
	struct f2fs_sit_block *sit_blk;
	unsigned int segno = 0;
	struct f2fs_summary_block *sum = curseg->sum_blk;
	char *ptr = NULL;

	sit_blk = calloc(BLOCK_SZ, 1);
	ASSERT(sit_blk);
	/* remove sit journal */
	sum->journal.n_sits = 0;
	curseg->journal->n_sits = 0;

	ptr = fsck->main_area_bitmap;

	for (segno = 0; segno < TOTAL_SEGS(sbi); segno += SIT_ENTRY_PER_BLOCK) {
		rewrite_sit_area_block(sbi, ptr, segno, sit_blk);
		ptr += SIT_VBLOCK_MAP_SIZE * SIT_ENTRY_PER_BLOCK;
	}

	free(sit_blk);
}

static int flush_sit_journal_entries(struct f2fs_sb_info *sbi)
{
	struct curseg_info *curseg = CURSEG_I(sbi, CURSEG_COLD_DATA);
	struct f2fs_journal *journal = curseg->journal;
	struct sit_info *sit_i = SIT_I(sbi);
	struct f2fs_sit_block *sit_blk;
	unsigned int segno;
	int i;

	sit_blk = calloc(BLOCK_SZ, 1);
	ASSERT(sit_blk);
	for (i = 0; i < sits_in_cursum(journal); i++) {
		struct f2fs_sit_entry *sit;
		struct seg_entry *se;

		segno = segno_in_journal(journal, i);
		if (segno >= TOTAL_SEGS(sbi))
			continue;
		se = get_seg_entry(sbi, segno);

		get_current_sit_page(sbi, segno, sit_blk);
		sit = &sit_blk->entries[SIT_ENTRY_OFFSET(sit_i, segno)];

		memcpy(sit->valid_map, se->cur_valid_map, SIT_VBLOCK_MAP_SIZE);
		sit->vblocks = cpu_to_le16((se->type << SIT_VBLOCKS_SHIFT) |
							se->valid_blocks);
		sit->mtime = cpu_to_le64(se->mtime);

		rewrite_current_sit_page(sbi, segno, sit_blk);
	}

	free(sit_blk);
	journal->n_sits = 0;
	return i;
}

static int flush_nat_journal_entries(struct f2fs_sb_info *sbi)
{
	struct curseg_info *curseg = CURSEG_I(sbi, CURSEG_HOT_DATA);
	struct f2fs_journal *journal = curseg->journal;
	struct f2fs_nat_block *nat_block;
	pgoff_t block_addr;
	int entry_off;
	nid_t nid;
	int ret;
	int i = 0;

	nat_block = (struct f2fs_nat_block *)calloc(BLOCK_SZ, 1);
	ASSERT(nat_block);
next:
	if (i >= nats_in_cursum(journal)) {
		free(nat_block);
		journal->n_nats = 0;
		return i;
	}

	nid = le32_to_cpu(nid_in_journal(journal, i));

	entry_off = nid % NAT_ENTRY_PER_BLOCK;
	block_addr = current_nat_addr(sbi, nid, NULL);

	ret = dev_read_block(nat_block, block_addr);
	ASSERT(ret >= 0);

	memcpy(&nat_block->entries[entry_off], &nat_in_journal(journal, i),
					sizeof(struct f2fs_nat_entry));

	ret = dev_write_block_no_sync(nat_block, block_addr);
	ASSERT(ret >= 0);
	i++;
	goto next;
}

/*
 * flush journal entries in no sync mode, callers need to handle
 * following things:
 *
 * - set write stream id for meta area before calling
 * - sync data after calling back
 */
void flush_journal_entries(struct f2fs_sb_info *sbi)
{
	int n_nats, n_sits;

	n_nats = flush_nat_journal_entries(sbi);
	n_sits = flush_sit_journal_entries(sbi);

	if (n_nats || n_sits)
		write_checkpoints(sbi);
}

static void flush_sit_block(struct f2fs_sb_info *sbi, unsigned int start_segno,
		struct f2fs_sit_block *sit_blk)
{
	unsigned int segno;
	struct sit_info *sit_i = SIT_I(sbi);
	struct seg_entry *se = NULL;
	bool need_flush = false;
	struct f2fs_sit_entry *sit = NULL;
	pgoff_t next_block_addr;
	pgoff_t cur_block_addr;

	for (segno = start_segno; segno < start_segno + SIT_ENTRY_PER_BLOCK &&
			segno < TOTAL_SEGS(sbi); segno++) {
		se = get_seg_entry(sbi, segno);
		if (!se->dirty)
			continue;
		if (need_flush == false) {
			get_current_sit_page(sbi, start_segno, sit_blk);
			need_flush = true;
		}

		sit = &sit_blk->entries[SIT_ENTRY_OFFSET(sit_i, segno)];
		memcpy(sit->valid_map, se->cur_valid_map, SIT_VBLOCK_MAP_SIZE);
		sit->vblocks = cpu_to_le16((se->type << SIT_VBLOCKS_SHIFT) |
							se->valid_blocks);
		se->dirty = 0;
	}
	if (need_flush) {
		cur_block_addr = current_sit_addr(sbi, start_segno);
		next_block_addr = next_sit_addr(sbi, cur_block_addr);
		f2fs_write_block(sbi, sit_blk, next_block_addr, true);
		set_to_next_sit(sbi, start_segno);
	}
}

/*
 * flush sit entries in no sync mode, callers need to handle following things:
 *
 * - set write stream id for meta area before calling
 * - sync data after calling back
 */
void flush_sit_entries(struct f2fs_sb_info *sbi)
{
	struct f2fs_sit_block *sit_blk;
	unsigned int segno = 0;
	struct curseg_info *curseg = CURSEG_I(sbi, CURSEG_COLD_DATA);

	/* remove sit journal */
	curseg->journal->n_sits = 0;

	sit_blk = calloc(BLOCK_SZ, 1);
	ASSERT(sit_blk);
	/* update free segments */
	for (segno = 0; segno < TOTAL_SEGS(sbi);
			segno += SIT_ENTRY_PER_BLOCK) {
		flush_sit_block(sbi, segno, sit_blk);
	}

	free(sit_blk);
}

static void get_current_nat_block(struct f2fs_sb_info *sbi,
			struct f2fs_nat_block *nat_block, nid_t nid)
{
	pgoff_t block_addr;
	int ret;

	block_addr = current_nat_addr(sbi, nid, NULL);
	ret = dev_read_block(nat_block, block_addr);
	ASSERT(ret >= 0);
}

static void flush_nat_block(struct f2fs_sb_info *sbi, nid_t start_nid,
				struct f2fs_nat_block *nat_block)
{
	bool need_flush = false;
	int entry_off;
	nid_t nid;
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
	pgoff_t next_block_addr;
	pgoff_t cur_block_addr;

	for (nid = start_nid; nid < start_nid + NAT_ENTRY_PER_BLOCK &&
					nid < fsck->nr_nat_entries; nid++) {
		if (f2fs_test_bit(nid, fsck->nat_area_dirty_bitmap) == 0)
			continue;
		if (need_flush == false) {
			get_current_nat_block(sbi, nat_block, start_nid);
			need_flush = true;
		}
		entry_off = nid % NAT_ENTRY_PER_BLOCK;
		nat_block->entries[entry_off] = fsck->entries[nid];
		f2fs_clear_bit(nid, fsck->nat_area_dirty_bitmap);
	}
	if (need_flush) {
		cur_block_addr = current_nat_addr(sbi, start_nid, NULL);
		next_block_addr = next_nat_addr(sbi, cur_block_addr);
		f2fs_write_block(sbi, nat_block, next_block_addr, true);
		set_to_next_nat(sbi, start_nid);
	}
}

void flush_nat_entries(struct f2fs_sb_info *sbi)
{
	struct f2fs_nat_block *nat_block = NULL;
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
	struct curseg_info *curseg = CURSEG_I(sbi, CURSEG_HOT_DATA);
	nid_t nid;

	nat_block = (struct f2fs_nat_block *)calloc(BLOCK_SZ, 1);
	ASSERT(nat_block);
	/* remove sit journal */
	curseg->journal->n_nats = 0;

	for (nid = 0; nid < fsck->nr_nat_entries;
			nid += NAT_ENTRY_PER_BLOCK) {
		flush_nat_block(sbi, nid, nat_block);
	}
	free(nat_block);
}

int find_next_free_block(struct f2fs_sb_info *sbi, u64 *to, int left,
		int type, struct f2fs_summary *sum)
{
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	struct curseg_info *curseg = CURSEG_I(sbi, type);

	add_sum_entry(curseg, sum);

	*to = SM_I(sbi)->main_blkaddr +
		(curseg->segno << get_sb(log_blocks_per_seg)) +
		curseg->next_blkoff;

	curseg->next_blkoff++;

	if (!__has_curseg_space(sbi, curseg)) {
		get_new_curseg(sbi, left, curseg, type, 0);
		flush_curseg_cached_blocks(sbi, type);
	}

	c.fix_on = 1;

	return 0;
}

void zero_journal_entries(struct f2fs_sb_info *sbi)
{
	int i;

	for (i = 0; i < NO_CHECK_TYPE; i++)
		CURSEG_I(sbi, i)->journal->n_nats = 0;
}

void write_curseg_info(struct f2fs_sb_info *sbi)
{
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);
	int i;

	for (i = 0; i < NO_CHECK_TYPE; i++) {
		cp->alloc_type[i] = CURSEG_I(sbi, i)->alloc_type;
		if (i < CURSEG_HOT_NODE) {
			set_cp(cur_data_segno[i], CURSEG_I(sbi, i)->segno);
			set_cp(cur_data_blkoff[i],
					CURSEG_I(sbi, i)->next_blkoff);
		} else if (i >= CURSEG_HOT_NODE && i <= CURSEG_COLD_NODE) {
			int n = i - CURSEG_HOT_NODE;

			set_cp(cur_node_segno[n], CURSEG_I(sbi, i)->segno);
			set_cp(cur_node_blkoff[n],
					CURSEG_I(sbi, i)->next_blkoff);
		} else {
			int n = i - CURSEG_DATA_MOVE1;
			set_cp(cur_dmgc_segno[n], CURSEG_I(sbi, i)->segno);
			set_cp(cur_dmgc_blkoff[n],
					CURSEG_I(sbi, i)->next_blkoff);
		}
	}
}

int lookup_nat_in_journal(struct f2fs_sb_info *sbi, u32 nid,
					struct f2fs_nat_entry *raw_nat)
{
	struct curseg_info *curseg = CURSEG_I(sbi, CURSEG_HOT_DATA);
	struct f2fs_journal *journal = curseg->journal;
	int i = 0;

	for (i = 0; i < nats_in_cursum(journal); i++) {
		if (le32_to_cpu(nid_in_journal(journal, i)) == nid) {
			memcpy(raw_nat, &nat_in_journal(journal, i),
						sizeof(struct f2fs_nat_entry));
			DBG(3, "==> Found nid [0x%x] in nat cache\n", nid);
			return i;
		}
	}
	return -1;
}

void nullify_nat_entry(struct f2fs_sb_info *sbi, u32 nid)
{
	struct curseg_info *curseg = CURSEG_I(sbi, CURSEG_HOT_DATA);
	struct f2fs_journal *journal = curseg->journal;
	int i = 0;

	/* check in journal */
	for (i = 0; i < nats_in_cursum(journal); i++) {
		if (le32_to_cpu(nid_in_journal(journal, i)) == nid) {
			memset(&nat_in_journal(journal, i), 0,
					sizeof(struct f2fs_nat_entry));
			FIX_MSG("Remove nid [0x%x] in nat journal", nid);
			return;
		}
	}

	if (nid == F2FS_NODE_INO(sbi) || nid == F2FS_META_INO(sbi)) {
		FIX_MSG("nid [0x%x] block_addr= 0x%x -> 0x1", nid,
			le32_to_cpu(F2FS_FSCK(sbi)->entries[nid].block_addr));
		F2FS_FSCK(sbi)->entries[nid].block_addr = cpu_to_le32(0x1);
	} else {
		memset(&F2FS_FSCK(sbi)->entries[nid], 0,
					sizeof(struct f2fs_nat_entry));
		MSG(1, "Remove nid [0x%x] in NAT\n", nid);
	}
	f2fs_set_bit(nid, F2FS_FSCK(sbi)->nat_area_dirty_bitmap);
}

void duplicate_checkpoint(struct f2fs_sb_info *sbi)
{
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	unsigned long long dst, src;
	void *buf;
	unsigned int seg_size = 1 << get_sb(log_blocks_per_seg);
	int ret;

	if (sbi->cp_backuped)
		return;

	buf = malloc(F2FS_BLKSIZE * seg_size);
	ASSERT(buf);

	if (sbi->cur_cp == 1) {
		src = get_sb(cp_blkaddr);
		dst = src + seg_size;
	} else {
		dst = get_sb(cp_blkaddr);
		src = dst + seg_size;
	}

	ret = dev_read(buf, src << F2FS_BLKSIZE_BITS,
				seg_size << F2FS_BLKSIZE_BITS);
	ASSERT(ret >= 0);

	/* need not sync, will call f2fs_fsync_device immediately */
	ret = dev_write(buf, dst << F2FS_BLKSIZE_BITS,
				seg_size << F2FS_BLKSIZE_BITS, 0, 0, 0);
	ASSERT(ret >= 0);

	free(buf);

	ret = f2fs_fsync_device();
	ASSERT(ret >= 0);

	sbi->cp_backuped = 1;
	/* repair checkpoint at CP #0 position */
	sbi->cur_cp = CP_VER_FIRST;

	MSG(0, "Info: Duplicate valid checkpoint to mirror position "
		"%llu -> %llu\n", src, dst);
}

void write_checkpoint(struct f2fs_sb_info *sbi)
{
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	unsigned char *orphan_block;
	block_t orphan_blks = 0;
	unsigned long long cp_blk_no;
	unsigned long long orig_cp_blk_no;
	unsigned long long orphan_blk_addr;
	u32 flags = CP_UMOUNT_FLAG;
	int i, ret;
	u_int32_t crc = 0;
	struct f2fs_journal *nat_journal = CURSEG_I(sbi, CURSEG_HOT_DATA)->journal;
	struct f2fs_journal *sit_journal = CURSEG_I(sbi, CURSEG_COLD_DATA)->journal;
	u32 nat_block = 0;
	unsigned long long cp_ver = get_cp(checkpoint_ver);

	if (is_set_ckpt_flags(cp, CP_ORPHAN_PRESENT_FLAG)) {
		orphan_blks = __start_sum_addr(sbi) - 1;
		flags |= CP_ORPHAN_PRESENT_FLAG;
	}
	if (is_set_ckpt_flags(cp, CP_TRIMMED_FLAG))
		flags |= CP_TRIMMED_FLAG;
	if (is_set_ckpt_flags(cp, CP_DISABLED_FLAG))
		flags |= CP_DISABLED_FLAG;

	/* if has append nat or sit block,we need set append flag */
	if (nats_in_cursum(nat_journal) > NAT_JOURNAL_ENTRIES) {
		flags |= CP_APPEND_NAT_FLAG;
	}

	if (sits_in_cursum(sit_journal) > SIT_JOURNAL_ENTRIES) {
		flags |= CP_APPEND_SIT_FLAG;
	}

	if(is_set_ckpt_flags(cp, CP_DATAMOVE_FLAG)) {
		flags |= CP_DATAMOVE_FLAG;
	}

	set_cp(ckpt_flags, flags);

	set_cp(free_segment_count, get_free_segments(sbi));
	if (c.func == FSCK && c.fsck_stat >= VERIFY) {
		struct f2fs_fsck *fsck = F2FS_FSCK(sbi);

		set_cp(valid_block_count, fsck->chk.valid_blk_cnt);
		set_cp(valid_node_count, fsck->chk.valid_node_cnt);
		set_cp(valid_inode_count, fsck->chk.valid_inode_cnt);
	} else {
		set_cp(valid_block_count, sbi->total_valid_block_count);
		set_cp(valid_node_count, sbi->total_valid_node_count);
		set_cp(valid_inode_count, sbi->total_valid_inode_count);
	}

	if (flags & CP_DATAMOVE_FLAG)
		set_cp(cp_pack_total_block_count, 10 + orphan_blks + get_sb(cp_payload));
	else
		set_cp(cp_pack_total_block_count, 8 + orphan_blks + get_sb(cp_payload));

	set_cp(checkpoint_ver, cp_ver + 1);
	get_sit_bitmap(sbi, __bitmap_ptr(sbi, SIT_BITMAP));
	get_nat_bitmap(sbi, __bitmap_ptr(sbi, NAT_BITMAP));

	crc = f2fs_cal_crc32(HMFS_SUPER_MAGIC, cp, CP_CHKSUM_OFFSET);
	*((__le32 *)((unsigned char *)cp + CP_CHKSUM_OFFSET)) = cpu_to_le32(crc);

	cp_blk_no = __next_cp_addr(sbi);

	/* write the first cp */
	ret = dev_write_block_no_sync(cp, cp_blk_no++);
	ASSERT(ret >= 0);

	/* write the payload */
	for (i = 0; i < get_sb(cp_payload); ++i) {
		ret = dev_write_block_no_sync(((unsigned char *)cp) +
				(i + 1) * F2FS_BLKSIZE, cp_blk_no++);
		ASSERT(ret >= 0);
	}

	/* write the orphan blocks */
	orig_cp_blk_no = __start_cp_addr(sbi);
	orphan_blk_addr = orig_cp_blk_no + get_sb(cp_payload) + 1;
	orphan_block = (unsigned char*)calloc(F2FS_BLKSIZE, 1);
	ASSERT(orphan_block != NULL);
	for (i = 0; i < orphan_blks; ++i) {
		ret = dev_read_block(orphan_block, orphan_blk_addr + i);
		ASSERT(ret >= 0);
		ret = dev_write_block_no_sync(orphan_block, cp_blk_no++);
		ASSERT(ret >= 0);
	}
	free(orphan_block);

	/* update summary blocks having nullified journal entries */
	for (i = 0; i < NO_CHECK_TYPE; i++) {
		struct curseg_info *curseg = CURSEG_I(sbi, i);
		u64 ssa_blk;

		if (!(flags & CP_DATAMOVE_FLAG) && IS_DM_SEG(i))
			continue;

		if ((i == CURSEG_HOT_DATA) || (i == CURSEG_COLD_DATA)) {
			memcpy(&curseg->sum_blk->journal, curseg->journal,
				sizeof(struct f2fs_journal));
		}
		ret = dev_write_block_no_sync(curseg->sum_blk, cp_blk_no++);
		ASSERT(ret >= 0);

		/* update original SSA too */
		ssa_blk = GET_SUM_BLKADDR(sbi, curseg->segno);
		ret = dev_write_block_no_sync(curseg->sum_blk, ssa_blk);
		ASSERT(ret >= 0);
	}

	/* write nat append block */
	if (flags & CP_APPEND_NAT_FLAG) {
		struct curseg_info *curseg = CURSEG_I(sbi, CURSEG_HOT_DATA);
		block_t nat_blk =  __next_cp_addr(sbi) +
			le32_to_cpu(F2FS_CKPT(sbi)->cp_pack_total_block_count);
		ret = dev_write_block_no_sync((char *)curseg->journal +
				SUM_JOURNAL_SIZE - NAT_JOURNAL_RESERVED, nat_blk);
		nat_block = 1;
		ASSERT(ret >= 0);
	}

	/* write sit append block */
	if (flags & CP_APPEND_SIT_FLAG) {
		struct curseg_info *curseg = CURSEG_I(sbi, CURSEG_COLD_DATA);
		block_t sit_blk =  __next_cp_addr(sbi) +
			le32_to_cpu(F2FS_CKPT(sbi)->cp_pack_total_block_count) + nat_block;
		ret = dev_write_block_no_sync((char *)curseg->journal +
				SUM_JOURNAL_SIZE - SIT_JOURNAL_RESERVED, sit_blk);
		ASSERT(ret >= 0);
	}

	if (flags & CP_DATAMOVE_FLAG) {
		block_t dm_cache_blk =  __next_cp_addr(sbi) +
			le32_to_cpu(F2FS_CKPT(sbi)->cp_pack_total_block_count);
		if (flags & CP_APPEND_NAT_FLAG)
			dm_cache_blk++;
		if (flags & CP_APPEND_SIT_FLAG)
			dm_cache_blk++;

		ASSERT(sbi->dm_cache_blocks);
		char *dm_cache_block;
		for (i = 0; i < c.num_dm_blocks; i++) {
			dm_cache_block = sbi->dm_cache_blocks + i * PAGE_SIZE;
			ret = dev_write_block_no_sync(dm_cache_block, dm_cache_blk++);
			ASSERT(ret >= 0);
		}
	}

	/* in case of sudden power off */
	ret = f2fs_fsync_device();
	ASSERT(ret >= 0);

	/* write the last cp */
	ret = dev_write_block_no_sync(cp, cp_blk_no++);
	ASSERT(ret >= 0);

	ret = f2fs_fsync_device();
	ASSERT(ret >= 0);

	__set_cp_next_pack(sbi);
}

void write_checkpoints(struct f2fs_sb_info *sbi)
{
	/* copy valid checkpoint to its mirror position */
	duplicate_checkpoint(sbi);
	write_checkpoint(sbi);
}

void build_nat_area_bitmap(struct f2fs_sb_info *sbi)
{
	struct curseg_info *curseg = CURSEG_I(sbi, CURSEG_HOT_DATA);
	struct f2fs_journal *journal = curseg->journal;
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	struct f2fs_nm_info *nm_i = NM_I(sbi);
	struct f2fs_nat_block *nat_block;
	struct node_info ni;
	u32 nid, nr_nat_blks;
	pgoff_t block_off;
	pgoff_t block_addr;
	int seg_off;
	int ret;
	unsigned int i;

	nat_block = (struct f2fs_nat_block *)calloc(BLOCK_SZ, 1);
	ASSERT(nat_block);

	/* Alloc & build nat entry bitmap */
	nr_nat_blks = (get_sb(segment_count_nat) / 2) <<
					sbi->log_blocks_per_seg;

	fsck->nr_nat_entries = nr_nat_blks * NAT_ENTRY_PER_BLOCK;
	fsck->nat_area_bitmap_sz = (fsck->nr_nat_entries + 7) / 8;
	fsck->nat_area_bitmap = calloc(fsck->nat_area_bitmap_sz, 1);
	ASSERT(fsck->nat_area_bitmap);
	fsck->nat_area_dirty_bitmap = calloc(fsck->nat_area_bitmap_sz, 1);
	ASSERT(fsck->nat_area_dirty_bitmap);

	fsck->entries = calloc(sizeof(struct f2fs_nat_entry),
					fsck->nr_nat_entries);
	ASSERT(fsck->entries);

	for (block_off = 0; block_off < nr_nat_blks; block_off++) {

		seg_off = block_off >> sbi->log_blocks_per_seg;
		block_addr = (pgoff_t)(nm_i->nat_blkaddr +
			(seg_off << sbi->log_blocks_per_seg << 1) +
			(block_off & ((1 << sbi->log_blocks_per_seg) - 1)));

		if (f2fs_test_bit(block_off, nm_i->nat_bitmap))
			block_addr += sbi->blocks_per_seg;

		ret = dev_read_block(nat_block, block_addr);
		ASSERT(ret >= 0);

		nid = block_off * NAT_ENTRY_PER_BLOCK;
		for (i = 0; i < NAT_ENTRY_PER_BLOCK; i++) {
			ni.nid = nid + i;

			if ((nid + i) == F2FS_NODE_INO(sbi) ||
					(nid + i) == F2FS_META_INO(sbi)) {
				/*
				 * block_addr of node/meta inode should be 0x1.
				 * Set this bit, and fsck_verify will fix it.
				 */
				if (le32_to_cpu(nat_block->entries[i].block_addr) != 0x1) {
					DMD_ADD_ERROR(LOG_TYP_FSCK, PR_INVALID_NAT_ENTRY1_ENTRY2);
					ASSERT_MSG("\tError: ino[0x%x] block_addr[0x%x] is invalid\n",
							nid + i, le32_to_cpu(nat_block->entries[i].block_addr));
					f2fs_set_bit(nid + i, fsck->nat_area_bitmap);
				}
				continue;
			}

			node_info_from_raw_nat(&ni, &nat_block->entries[i]);
			if (ni.blk_addr == 0x0)
				continue;
			if (!IS_VALID_BLK_ADDR(sbi, ni.blk_addr)) {
				ASSERT_MSG("\tError: ino[0x%x] block_addr[0x%x] is invalid\n",
							nid + i, ni.blk_addr);
				ni.blk_addr = 0;
			}
			if (ni.ino == 0x0) {
				DMD_ADD_ERROR(LOG_TYP_FSCK, PR_INVALID_INO_OR_BLKADDR);
				ASSERT_MSG("\tError: ino[0x%8x] or blk_addr[0x%16x]"
					" is invalid\n", ni.ino, ni.blk_addr);
			}
			if (ni.ino == (nid + i)) {
				fsck->nat_valid_inode_cnt++;
				DBG(3, "ino[0x%8x] maybe is inode\n", ni.ino);
			}
			if (nid + i == 0) {
				/*
				 * nat entry [0] must be null.  If
				 * it is corrupted, set its bit in
				 * nat_area_bitmap, fsck_verify will
				 * nullify it
				 */
				DMD_ADD_ERROR(LOG_TYP_FSCK, PR_INVALID_NAT_ENTRY0);
				ASSERT_MSG("Invalid nat entry[0]: "
					"blk_addr[0x%x]\n", ni.blk_addr);
				fsck->chk.valid_nat_entry_cnt--;
			}

			DBG(3, "nid[0x%8x] addr[0x%16x] ino[0x%8x]\n",
				nid + i, ni.blk_addr, ni.ino);
			f2fs_set_bit(nid + i, fsck->nat_area_bitmap);
			fsck->chk.valid_nat_entry_cnt++;

			fsck->entries[nid + i] = nat_block->entries[i];
		}
	}

	/* Traverse nat journal, update the corresponding entries */
	for (i = 0; i < nats_in_cursum(journal); i++) {
		struct f2fs_nat_entry raw_nat;
		nid = le32_to_cpu(nid_in_journal(journal, i));
		ni.nid = nid;

		DBG(3, "==> Found nid [0x%x] in nat cache, update it\n", nid);

		if (nid == 0)
			continue;
		/* Clear the original bit and count */
		if (fsck->entries[nid].block_addr != 0x0) {
			fsck->chk.valid_nat_entry_cnt--;
			f2fs_clear_bit(nid, fsck->nat_area_bitmap);
			if (fsck->entries[nid].ino == nid)
				fsck->nat_valid_inode_cnt--;
		}

		/* Use nat entries in journal */
		memcpy(&raw_nat, &nat_in_journal(journal, i),
					sizeof(struct f2fs_nat_entry));
		node_info_from_raw_nat(&ni, &raw_nat);
		if (ni.blk_addr != 0x0) {
			if (ni.ino == 0x0) {
				DMD_ADD_ERROR(LOG_TYP_FSCK, PR_INVALID_INO_OR_BLKADDR);
				ASSERT_MSG("\tError: ino[0x%8x] or blk_addr[0x%16x]"
					" is invalid\n", ni.ino, ni.blk_addr);
			}
			if (!IS_VALID_BLK_ADDR(sbi, ni.blk_addr)) {
				ASSERT_MSG("\tError: ino[0x%x] block_addr[0x%x] "
						"(in journal) is invalid\n",
						ni.ino, ni.blk_addr);
				ni.blk_addr = 0;
			}
			if (ni.ino == nid) {
				fsck->nat_valid_inode_cnt++;
				DBG(3, "ino[0x%8x] maybe is inode\n", ni.ino);
			}
			f2fs_set_bit(nid, fsck->nat_area_bitmap);
			fsck->chk.valid_nat_entry_cnt++;
			DBG(3, "nid[0x%x] in nat cache\n", nid);
		}
		fsck->entries[nid] = raw_nat;

		/* if do fsck we need to flush nat journal */
		f2fs_set_bit(nid, fsck->nat_area_dirty_bitmap);
	}
	free(nat_block);

	DBG(1, "valid nat entries (block_addr != 0x0) [0x%8x : %u]\n",
			fsck->chk.valid_nat_entry_cnt,
			fsck->chk.valid_nat_entry_cnt);
}

static int check_sector_size(struct f2fs_super_block *sb)
{
	u_int32_t log_sectorsize, log_sectors_per_block;

	log_sectorsize = log_base_2(c.sector_size);
	log_sectors_per_block = log_base_2(c.sectors_per_blk);

	if (log_sectorsize == get_sb(log_sectorsize) &&
			log_sectors_per_block == get_sb(log_sectors_per_block))
		return 0;

	set_sb(log_sectorsize, log_sectorsize);
	set_sb(log_sectors_per_block, log_sectors_per_block);

	update_superblock(sb, SB_MASK_ALL);
	return 0;
}

static int tune_sb_features(struct f2fs_sb_info *sbi)
{
	int sb_changed = 0;
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);

	if (!(sb->feature & cpu_to_le32(F2FS_FEATURE_ENCRYPT)) &&
			c.feature & cpu_to_le32(F2FS_FEATURE_ENCRYPT)) {
		sb->feature |= cpu_to_le32(F2FS_FEATURE_ENCRYPT);
		MSG(0, "Info: Set Encryption feature\n");
		sb_changed = 1;
	}
	/* TODO: quota needs to allocate inode numbers */

	if (!(sb->feature & cpu_to_le32(F2FS_FEATURE_CASEFOLD)) &&
			c.feature & cpu_to_le32(F2FS_FEATURE_CASEFOLD)) {
		if (!c.s_encoding) {
			ERR_MSG("ERROR: Must specify encoding to enable casefolding.\n");
			return -1;
		}
		sb->feature |= cpu_to_le32(F2FS_FEATURE_CASEFOLD);
		MSG(0, "Info: Set Casefold feature\n");
		sb_changed = 1;
	}

	c.feature = sb->feature;
	if (!sb_changed)
		return 0;

	update_superblock(sb, SB_MASK_ALL);
	return 0;
}

static int read_dm_blocks(struct f2fs_sb_info *sbi)
{
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);
	block_t start_blk;
	int ret, num_dm_entry;
	char *dm_cache_block = NULL;
	struct hmfs_datamove_block dm_head;
	int i;

	start_blk = __start_cp_addr(sbi) +
		le32_to_cpu(F2FS_CKPT(sbi)->cp_pack_total_block_count);

	if (is_set_ckpt_flags(cp, CP_APPEND_NAT_FLAG))
		start_blk++;
	if (is_set_ckpt_flags(cp, CP_APPEND_SIT_FLAG))
		start_blk++;

	ret = dev_read_block(&dm_head, start_blk++);
	ASSERT(ret >= 0);

	num_dm_entry = le32_to_cpu(dm_head.head.count);
	c.dm_set = malloc(sizeof(struct hmfs_dm_set));
	ASSERT(c.dm_set);
	c.dm_set->dm_entry_index = 0;
	c.dm_set->start = 0;
	c.dm_set->end = 0;
	c.dm_set->dm_entry = malloc(num_dm_entry * sizeof(struct hmfs_dm_entry));
	ASSERT(c.dm_set->dm_entry);

	c.num_dm_blocks = (num_dm_entry * sizeof(struct hmfs_datamove_entry) +
			DM_HEAD_RESV_SIZE - 1) / PAGE_SIZE + 1;
	if (c.num_dm_blocks > MAX_DM_CACHE_BLOCKS) {
		MSG(0, "num_dm_blocks:%d > MAX_DM_CACHE_BLOCKS, entries:%d\n",
						c.num_dm_blocks, num_dm_entry);
		return -1;
	}
	sbi->dm_cache_blocks = malloc(PAGE_SIZE * c.num_dm_blocks);
	ASSERT(sbi->dm_cache_blocks);
	memcpy(sbi->dm_cache_blocks, &dm_head,
			sizeof(struct hmfs_datamove_block));

	for (i = 1; i < c.num_dm_blocks; i++) {
		dm_cache_block = sbi->dm_cache_blocks + i * PAGE_SIZE;
		ret = dev_read_block(dm_cache_block, start_blk++);
		ASSERT(ret >= 0);
	}

	return ret;
}

static void insert_dm_entry(struct f2fs_sb_info *sbi,
			struct hmfs_datamove_entry *hmfs_dm_entry)
{
	struct hmfs_dm_entry dm_entry;
	struct free_segmap_info *free_i = FREE_I(sbi);
	unsigned int segno;

	dm_entry.src_blkaddr = le32_to_cpu(hmfs_dm_entry->src_blkaddr);
	dm_entry.dst_blkaddr = le32_to_cpu(hmfs_dm_entry->dst_blkaddr);
	segno = GET_SEGNO(sbi, dm_entry.src_blkaddr);
	ASSERT(segno < TOTAL_SEGS(sbi));

	/* Prevent the segment from being reallocated as a new segment. */
	__set_test_and_inuse(sbi, segno);
	/* Prevent the section from being GC */
	set_bit_le(GET_SEC_FROM_SEG(sbi, segno), free_i->free_dm_secmap);
	/* Prevents the DM source segment from being considered free. */
	set_bit_le(segno, free_i->free_dm_segmap);

	memcpy(c.dm_set->dm_entry + c.dm_set->dm_entry_index, &dm_entry,
				sizeof(struct hmfs_dm_entry));
	c.dm_set->dm_entry_index++;

	if (c.dm_set->start == 0) {
		c.dm_set->start = dm_entry.dst_blkaddr;
	}
	c.dm_set->end = dm_entry.dst_blkaddr;
}

static void build_dm_entries(struct f2fs_sb_info *sbi)
{
	int num_dm_entry, dm_page_index, offset;
	struct hmfs_datamove_entry *hmfs_dm_entry = NULL;
	char *dm_cache_block = NULL;
	struct hmfs_datamove_block *dm_head = NULL;
	int i;

	dm_head = (struct hmfs_datamove_block *)sbi->dm_cache_blocks;
	num_dm_entry = le32_to_cpu(dm_head->head.count);
	dm_page_index = 1;
	offset = DM_HEAD_RESV_SIZE;
	dm_cache_block = sbi->dm_cache_blocks;
	for (i = 0; i < num_dm_entry; i++) {
		hmfs_dm_entry = (struct hmfs_datamove_entry *)
					(dm_cache_block + offset);
		insert_dm_entry(sbi, hmfs_dm_entry);

		offset += sizeof(struct hmfs_datamove_entry);
		if (offset + sizeof(struct hmfs_datamove_entry) <= PAGE_SIZE)
			continue;

		dm_cache_block = sbi->dm_cache_blocks + dm_page_index * PAGE_SIZE;
		dm_page_index++;
		offset = 0;
	}

	/*
	 * Makesure that the addresses are consecutive.
	 * This is applicable only to the single-headed scenario.
	 * If the multi-headed scenario is used, extension is required.
	 */
	if (num_dm_entry > 0) {
		ASSERT((c.dm_set->end - c.dm_set->start + 1) ==
					c.dm_set->dm_entry_index);
	}
}

static void read_dm_cache_block(struct f2fs_sb_info *sbi)
{
	int ret;

	if(!is_set_ckpt_flags(F2FS_CKPT(sbi), CP_DATAMOVE_FLAG)) {
		sbi->dm_cache_blocks = NULL;
		return;
	}

	ret = read_dm_blocks(sbi);
	if (ret < 0) {
		sbi->dm_cache_blocks = NULL;
		return;
	}

	build_dm_entries(sbi);
}

int f2fs_do_mount(struct f2fs_sb_info *sbi)
{
	struct f2fs_checkpoint *cp = NULL;
	struct f2fs_super_block *sb = NULL;
	int ret;

	c.bug_on = 0;

	sbi->active_logs = c.active_logs;
	ret = validate_super_block(sbi, SB0_ADDR);
	if (ret == -ENOTSUP)
		return -1;
	if (ret) {
		ret = validate_super_block(sbi, SB1_ADDR);
		if (ret) {
			DMD_ADD_ERROR(LOG_TYP_FSCK, PR_INVALID_SUPER_BLOCK);
			return -1;
		}
	}
	sb = F2FS_RAW_SUPER(sbi);

	ret = check_sector_size(sb);
	if (ret)
		return -1;

	print_raw_sb_info(sb);

	init_sb_info(sbi);

	ret = get_valid_checkpoint(sbi);
	if (ret) {
		DMD_ADD_ERROR(LOG_TYP_FSCK, PR_INVALID_CHECKPOINT);
		ERR_MSG("Can't find valid checkpoint\n");
		return -1;
	}

	if (sanity_check_ckpt(sbi)) {
		DMD_ADD_ERROR(LOG_TYP_FSCK, PR_POLLUTE_CHECKPOINT);
		ERR_MSG("Checkpoint is polluted\n");
		return -1;
	}
	cp = F2FS_CKPT(sbi);

	print_ckpt_info(sbi);

	if (c.quota_fix) {
		if (get_cp(ckpt_flags) & CP_QUOTA_NEED_FSCK_FLAG)
			c.fix_on = 1;
	}

	if (!f2fs_should_proceed(sb, get_cp(ckpt_flags)))
		return 1;

	if (c.layout)
		return 1;

	if (tune_sb_features(sbi))
		return -1;

	/* precompute checksum seed for metadata */
	if (c.feature & cpu_to_le32(F2FS_FEATURE_INODE_CHKSUM))
		c.chksum_seed = f2fs_cal_crc32(~0, sb->uuid, sizeof(sb->uuid));

	sbi->total_valid_node_count = get_cp(valid_node_count);
	sbi->total_valid_inode_count = get_cp(valid_inode_count);
	sbi->user_block_count = get_cp(user_block_count);
	sbi->total_valid_block_count = get_cp(valid_block_count);
	sbi->last_valid_block_count = sbi->total_valid_block_count;
	sbi->alloc_valid_block_count = 0;

	if (build_segment_manager(sbi)) {
		DMD_ADD_ERROR(LOG_TYP_FSCK, PR_BUILD_SEGMENT_MANAGER_ERROR);
		ERR_MSG("build_segment_manager failed\n");
		return -1;
	}

	if (build_node_manager(sbi)) {
		DMD_ADD_ERROR(LOG_TYP_FSCK, PR_BUILD_NODE_MANAGER_ERROR);
		ERR_MSG("build_node_manager failed\n");
		return -1;
	}

	read_dm_cache_block(sbi);
	return 0;
}

void f2fs_do_umount(struct f2fs_sb_info *sbi)
{
	struct sit_info *sit_i = SIT_I(sbi);
	struct f2fs_sm_info *sm_i = SM_I(sbi);
	struct f2fs_nm_info *nm_i = NM_I(sbi);
	struct free_segmap_info *free_i = FREE_I(sbi);

	unsigned int i;

	/* free nm_info */
	if (c.func == SLOAD || c.func == FSCK)
		free(nm_i->nid_bitmap);
	free(nm_i->nat_bitmap);
	free(sbi->nm_info);

	/* free free_i */
	free(free_i->free_segmap);
	free(free_i->free_secmap);
	free(free_i->prefree_secmap);
	free(free_i->free_dm_secmap);
	free(free_i->free_dm_segmap);
	free(sm_i->free_info);

	/* free sit_info */
	for (i = 0; i < TOTAL_SEGS(sbi); i++)
		free(sit_i->sentries[i].cur_valid_map);

	free(sit_i->sentries);
	free(sit_i->sit_bitmap);
	free(sm_i->sit_info);

	/* free sm_info */
	for (i = 0; i < NR_CURSEG_TYPE; i++) {
		free(sm_i->curseg_array[i].sum_blk);
		free(sm_i->curseg_array[i].journal);
	}

	if (sbi->dm_cache_blocks)
		free(sbi->dm_cache_blocks);
	if (c.dm_set != NULL) {
		if (c.dm_set->dm_entry)
			free(c.dm_set->dm_entry);
		free(c.dm_set);
	}

	free(sm_i->curseg_array);
	free(sbi->sm_info);

	free(sbi->ckpt);
	sbi->ckpt = NULL;
	free(sbi->raw_super);
	sbi->raw_super = NULL;
}
