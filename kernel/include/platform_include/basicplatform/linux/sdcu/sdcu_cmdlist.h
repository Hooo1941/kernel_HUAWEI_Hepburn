/*
 * Copyright(C) 2021-2022 Hisilicon Technologies Co., Ltd. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __SDCU_CMDLIST_H__
#define __SDCU_CMDLIST_H__

#include <linux/types.h>
/*
 * The macro 'SDCU_DEBUG' is reserved for drv debugging.
 * When there are new software requirements or hardware evolutions,
 * uncomment the following definination to allow debugging prints in drv.
 * #define SDCU_DEBUG 1
 */

/*
 * Currently, SDCU hardware restricts that
 * one decompress-cmd can carry at most 512K src data.
 * Decompress-cmds with more than 512K src data will not be
 * handled as errors.
 */
#define DECOMP_IN_MAX_LEN (512 * 1024)
#define EXT_LEN_MASK 0x1FFE
#define EXT_LEN_SHIFT 1

enum comp_alg {
	COMP_ALG_PLZ,
	COMP_ALG_PLZFSE,
	COMP_ALG_MAX
};

enum addr_align_mode {
	ALIGN_4K,
	ALIGN_128B,
	ALIGN_256B,
	ALIGN_512B,
	ALIGN_MAX
};

union sdcu_ext_buf {
	struct _comp_out {
		u16 *out_len_buf; /* start va */
		uintptr_t base_phy; /* start pa */
		size_t len; /* buf_len */
	} comp_out;

	struct _decomp_in {
		u16 *len_to_4k;
		uintptr_t base_phy;
		size_t len; /* buf_len */
	} decomp_in;
};

struct addr_ent {
	uintptr_t start_addr; /* start phy addr */
	unsigned int addr_block_cnt; /* addr block cnt */
	/*
	 * align mode:
	 * 0b00: 4KB
	 * 0b01: 128B
	 * 0b10: 256B
	 * 0b11: 512B
	 */
	unsigned int align_mode;
};

struct cmd_address {
	struct addr_ent *addr_ents;
	int nents; /* valid nents in the addr_list and sg_addr_type list */
};

struct sdcu_cmd_buf {
	u32 *header_buf;
	size_t header_word;
	size_t header_len;
	dma_addr_t phy_header_buf;
	u32 *addr_in_buf;
	size_t addr_in_word;
	size_t addr_in_len;
	dma_addr_t phy_addr_in_buf;
	u32 *addr_out_buf;
	size_t addr_out_word;
	size_t addr_out_len;
	dma_addr_t phy_addr_out_buf;
};

#ifdef CONFIG_MM_SDCU_TEST
struct sdcu_req_ent;
typedef int (*sdcu_async_callback)(struct cmd_address *,
	struct cmd_address *, union sdcu_ext_buf *, struct sdcu_req_ent *);
#else
typedef int (*sdcu_async_callback)(struct cmd_address *,
	struct cmd_address *, union sdcu_ext_buf *, bool);
#endif

struct sdcu_req_ent {
	int finished; /* asyn finish status */
	int opcode;
	int alg;
	int sync_type; /* sync or async */
	u32 seq_id;
	u32 chnid;
	bool is_succ;
	int retry_times;
	struct sdcu_cmd_buf cmd_buf;
	union sdcu_ext_buf *ext_buf;
	struct list_head list_node;
	struct cmd_address *src;
	struct cmd_address *dst;
	sdcu_async_callback async_callback;
};

int sdcu_sync_compress(struct cmd_address *user_src,
	struct cmd_address *user_dst, int alg, union sdcu_ext_buf *user_ext);
int sdcu_sync_decompress(struct cmd_address *user_src,
	struct cmd_address *user_dst, int alg, union sdcu_ext_buf *user_ext);
int sdcu_sync_move(struct cmd_address *user_src, struct cmd_address *user_dst);
int sdcu_async_compress(struct cmd_address *user_src,
	struct cmd_address *user_dst, int alg, union sdcu_ext_buf *user_ext,
	sdcu_async_callback func);
int sdcu_async_decompress(struct cmd_address *user_src,
	struct cmd_address *user_dst, int alg, union sdcu_ext_buf *user_ext,
	sdcu_async_callback func);
int sdcu_async_move(struct cmd_address *user_src,
	struct cmd_address *user_dst, sdcu_async_callback func);

static inline void set_ext_len(u16 *ext, u16 len)
{
	*ext = len << EXT_LEN_SHIFT;
}

static inline u32 get_ext_len(u16 ext)
{
	return (ext & EXT_LEN_MASK) >> EXT_LEN_SHIFT;
}

static inline bool is_ext_valid(u16 ext)
{
	/* bit(0) == 1 means the dcmp operation reported failure */
	return (ext & BIT(0)) ? false : true;
}

#endif
