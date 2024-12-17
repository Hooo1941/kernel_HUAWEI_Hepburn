/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: efficent non-blocking ring buffer
 * Author: Huawei Dresden Research Center
 * Create: Tue Sep 15 10:13:59 2020
 */

#ifndef __RB_H
#define __RB_H
/*******************************************************************************
 * A formally verified and efficient ring buffer which provides almost
 * non-blocking multi-producer and speculative single-consumer. The C code
 * has been formally verified and optimized with VSync on weak memory models
 *
 * API list:
 * - rb_init
 * - rb_try_alloc_entry
 * - rb_commit_entry
 * - rb_write_entry
 * - rb_try_read_entry
 * - rb_read_entry
 * - rb_rewind_reader
 * - rb_get_write_offset
 *
 * Usage:
 *
 *   void *buf = malloc(size);
 *   struct rbuffer_s rb;
 *
 *   rb_init(&q, buf, size, blk_num, RB_RDWR);
 *
 *   // writer
 *   do {
 *        struct entry_s *entry = rb_try_alloc_entry(&rb, entry_size);
 *   } while(entry == NULL);
 *   void *data = entry_get_data();
 *   memcpy(data, src, entry_size);
 *   rb_commit_entry(&rb, entry);
 *
 *   // or use rb_write_entry directly
 *   #define TRY_TIME 100
 *   memcpy(user_buf, user_data, size);
 *   size_t sz = rb_write_entry(&rb, user_buf, size, TRY_TIME, NULL)
 *   if (sz == 0)
 *       return;
 *
 *   // reader
 *   vuint8_t my_buff[256];
 *   do {
 *        size_t size = rb_try_read_entry(&rb, my_buff, 256, NULL);
 *   } while (size == 0);
 *
 *   // or use rb_read_entry directly
 *   size_t size = rb_read_entry(&rb, my_buff, 256, NULL);
 *
 ******************************************************************************/
#include <linux/string.h>
#include <asm/vatomic.h>
#include <securec.h>

#define ALIGNMENT_MASK 0x7
#define RB_ALIGN_DOWN(a) ((a) & (~ALIGNMENT_MASK))
#define MIN_BLOCK_NUM 2
#define BACKWARD_STEP 1
#define ENTRY_ALIGN_BIT 2
#define ENTRY_ALIGN_MASK ((1U << ENTRY_ALIGN_BIT) - 1)
#define MIN_ALLOC_SIZE 8
#define MAX_ALLOC_SIZE(blksz)                                                  \
	(((blksz) - sizeof(struct block_head_s) - sizeof(struct entry_s)))
#define MIN_BLOCK_SIZE                                                         \
	(sizeof(struct block_head_s) + sizeof(struct entry_s) + MIN_ALLOC_SIZE)
#define BLOCK_DATA_SIZE(blksz) ((blksz) - sizeof(struct block_head_s))
#define ENTRY_INDEX_BITS 16
#define ENTRY_INDEX_MASK ((1U << ENTRY_INDEX_BITS) - 1)
#define WRITE_INDEX_SHIFT 1
#define WRITE_INDEX_MASK ((1U << WRITE_INDEX_SHIFT) - 1U)
#define RB_TRUE 1
#define RB_FALSE 0
#define RB_RDWR 0
#define RB_RDONLY 1
#define RB_MODE(flags) ((flags)&RB_RDONLY)
#define RB_DROP_NEW 2
#define RB_DROP(flags) ((flags)&RB_DROP_NEW)
#define ALIGN_DOWN_WITH_POWER_2(a)                                             \
	(1U << (32 - __builtin_clz((vuint32_t)(a)) - 1))
#ifdef VSYNC_VERIFICATION
#define rb_likely(x) (x)
#define rb_unlikely(x) (x)
#else
#define rb_likely(x) __builtin_expect(!!(x), 1)
#define rb_unlikely(x) __builtin_expect(!!(x), 0)
#endif

struct entry_s {
	vatomic_t blk_idx;
#ifdef VSYNC_VERIFICATION
	vatomic_t next;
	vatomic_t size;
#else
	vuint16_t next;
	vuint16_t size;
#endif
};

struct read_info_s {
	vuint64_t index;
	vuint32_t total;
	vuint32_t blk_read_cnt;
};

/* block index is 16 bit, so the block size is limited to 64k */
struct block_head_s {
	vatomic_t alloc_idx;
	vatomic_t commit_num;
};

struct buffer_head_s {
	vatomic64_t cur_wblk;
	vatomic64_t cur_rblk;
	vatomic_t blk_read_cnt;
	vatomic_t blk_read_idx;
};

struct rbuffer_s {
	struct buffer_head_s *bh;
	vuint8_t *g_blk;
	size_t total_size;
	vuint16_t total_blks;
	vuint16_t blk_size;
	vuint16_t move_step;
	vuint16_t flags;
};

/*******************************************************************************
 *  Auxiliary functions
 ******************************************************************************/
static inline vuint16_t size_align(vuint16_t size)
{
	vuint16_t entry_size = size + sizeof(struct entry_s);
	return (entry_size + ENTRY_ALIGN_MASK) & (~ENTRY_ALIGN_MASK);
}

static inline vuint64_t get_write_idx(vuint64_t widx)
{
	return (widx >> WRITE_INDEX_SHIFT);
}

static inline vuint64_t get_busy_bit(vuint64_t widx)
{
	return (widx & WRITE_INDEX_MASK);
}

static inline vuint64_t set_busy_bit(vuint64_t widx)
{
	return (widx | WRITE_INDEX_MASK);
}

static inline int is_busy(vuint64_t widx)
{
	return get_busy_bit(widx);
}

static inline vuint64_t get_next_idx(vuint64_t widx)
{
	return get_write_idx(widx) + 1;
}

static inline vuint64_t get_next_write_idx(vuint64_t widx)
{
	vuint64_t idx = get_next_idx(widx);
	/* ready bit always 0 when get next */
	return idx << WRITE_INDEX_SHIFT;
}

static inline vuint16_t allocidx_of(vuint32_t idx)
{
	return (vuint16_t)(idx & ENTRY_INDEX_MASK);
}

static inline vuint16_t allocnum_of(vuint32_t idx)
{
	return (vuint16_t)(idx >> ENTRY_INDEX_BITS);
}

static inline vuint32_t get_alloc_idx(vuint16_t alloc, vuint16_t idx)
{
	return (vuint32_t)(((vuint32_t)alloc << ENTRY_INDEX_BITS) | idx);
}

static inline vuint16_t convert_idx(struct rbuffer_s *rb, vuint64_t idx)
{
	return (vuint16_t)(idx & (rb->total_blks - 1));
}

static inline struct block_head_s *idx_to_block(struct rbuffer_s *rb,
						vuint16_t idx)
{
	return (struct block_head_s *)(rb->g_blk + (idx * rb->blk_size));
}

static inline struct block_head_s *global_idx_to_block(struct rbuffer_s *rb,
						       vuint64_t index)
{
	vuint16_t idx = convert_idx(rb, index);
	return idx_to_block(rb, idx);
}

/*******************************************************************************
 *  Entry functions
 ******************************************************************************/
static inline vuint16_t entry_get_blk_idx(struct entry_s *entry)
{
	return vatomic_read_rlx(&entry->blk_idx);
}

static inline void entry_set_blk_idx(struct entry_s *entry, vuint16_t idx)
{
	vatomic_write_rlx(&entry->blk_idx, idx);
}

static inline vuint16_t entry_get_next(struct entry_s *entry)
{
#ifdef VSYNC_VERIFICATION
	return (vuint16_t)vatomic_read_rlx(&entry->next);
#else
	return entry->next;
#endif
}

static inline void entry_set_next(struct entry_s *entry, vuint16_t next)
{
#ifdef VSYNC_VERIFICATION
	vatomic_write_rlx(&entry->next, next);
#else
	entry->next = next;
#endif
}

static inline vuint16_t entry_get_size(struct entry_s *entry)
{
#ifdef VSYNC_VERIFICATION
	return (vuint16_t)vatomic_read_rlx(&entry->size);
#else
	return entry->size;
#endif
}

static inline void entry_set_size(struct entry_s *entry, vuint16_t size)
{
#ifdef VSYNC_VERIFICATION
	vatomic_write_rlx(&entry->size, size);
#else
	entry->size = size;
#endif
}

static inline void *entry_get_data(struct entry_s *entry)
{
	return (void *)(entry + 1);
}

static inline void entry_alloc(struct entry_s *entry, vuint16_t bidx,
			       vuint16_t next, vuint16_t size)
{
	entry_set_size(entry, size);
	entry_set_next(entry, next);
	entry_set_blk_idx(entry, bidx);
}

static inline void entry_commit(struct rbuffer_s *rb, struct entry_s *entry)
{
	vuint16_t bidx = entry_get_blk_idx(entry);
	struct block_head_s *blk = idx_to_block(rb, bidx);
	vatomic_inc_rel(&blk->commit_num);
}

static inline int entry_is_invalid(struct block_head_s *blk,
				   struct entry_s *entry, vuint16_t blk_size)
{
	if (rb_unlikely(entry == NULL)) {
		return RB_TRUE;
	}
	vuint16_t size = entry_get_size(entry);
	if (size < MIN_ALLOC_SIZE || size > MAX_ALLOC_SIZE(blk_size)) {
		return RB_TRUE;
	}
	vuint8_t *cur = (vuint8_t *)entry;
	if (cur > (vuint8_t *)blk + blk_size) {
		return RB_TRUE;
	}
	vuint8_t *next = cur + entry_get_next(entry);
	if (next > (vuint8_t *)blk + blk_size) {
		return RB_TRUE;
	}
	return RB_FALSE;
}

/*******************************************************************************
 *  block functions
 ******************************************************************************/
static inline vuint16_t block_atomic_alloc_index(vatomic_t *idx, vuint16_t offset, vuint16_t bound)
{
	vuint32_t cur, next;
	vuint32_t newidx = 0;
	int done = RB_FALSE;

	do {
		cur = vatomic_read_rlx(idx);
		if (allocidx_of(cur) >= bound) {
			return ENTRY_INDEX_MASK;
		}

		newidx = allocidx_of(cur) + offset;
		if (newidx > bound) {
			next = get_alloc_idx(allocnum_of(cur), bound);
			if (vatomic_cmpxchg_rlx(idx, cur, next) == cur) {
				return ENTRY_INDEX_MASK;
			}
		} else {
			next = get_alloc_idx(allocnum_of(cur) + 1,
					     (vuint16_t)newidx);
			done = (vatomic_cmpxchg_rlx(idx, cur, next) == cur);
		}
	} while (!done);

	return allocidx_of(cur);
}

static inline vuint16_t block_get_allocnum(struct block_head_s *blk)
{
	vuint32_t alloc_idx_num = vatomic_read_rlx(&blk->alloc_idx);
	return allocnum_of(alloc_idx_num);
}

static inline vuint8_t *block_get_buff(struct block_head_s *blk)
{
	return (vuint8_t *)(blk + 1);
}

static inline int block_is_ready(struct block_head_s *blk)
{
	vuint16_t commit_num = (vuint16_t)vatomic_read_acq(&blk->commit_num);
	/* Read commit_num must happen before reading alloc_num, because the writer
     * write alloc_num first, then commit the entry */
	vuint16_t alloc_num = block_get_allocnum(blk);
	if (alloc_num == commit_num) {
		return (int)alloc_num;
	}
	return -1;
}

static inline void block_reset_metadata(struct block_head_s *blk)
{
	vatomic_write_rlx(&blk->commit_num, 0);
	/* reset commit_num must happen before reseting alloc_idx, because once
     * alloc_idx is set to 0, the pending threads can start to alloc */
	vatomic_write_rel(&blk->alloc_idx, 0);
}

static inline void block_init(struct block_head_s *blk)
{
	block_reset_metadata(blk);
}

static inline struct entry_s *block_alloc_entry(struct rbuffer_s *rb,
						vuint64_t bidx, vuint16_t size)
{
	vuint16_t offset = size_align(size);

	struct block_head_s *blk = global_idx_to_block(rb, bidx);
	vuint16_t idx = block_atomic_alloc_index(&blk->alloc_idx, offset,
						 BLOCK_DATA_SIZE(rb->blk_size));
	if (idx <= BLOCK_DATA_SIZE(rb->blk_size) - sizeof(struct entry_s) -
			   MIN_ALLOC_SIZE) {
		struct entry_s *entry =
			(struct entry_s *)&(block_get_buff(blk)[idx]);
		vatomic_fence();
		entry_alloc(entry, convert_idx(rb, bidx), offset, size);
		return entry;
	} else {
		return NULL;
	}
}

static inline struct entry_s *block_get_first_entry(struct block_head_s *blk)
{
	return (struct entry_s *)(block_get_buff(blk));
}

static inline struct entry_s *block_get_next_entry(struct entry_s *entry)
{
	vuint8_t *cur = (vuint8_t *)entry;
	return (struct entry_s *)(cur + entry_get_next(entry));
}

static inline struct entry_s *block_fetch_entry(struct rbuffer_s *rb,
						struct block_head_s *rblk,
						vuint32_t idx)
{
	struct entry_s *entry = (struct entry_s *)&(block_get_buff(rblk)[idx]);
	if (entry_is_invalid(rblk, entry, rb->blk_size)) {
		return NULL;
	}
	return entry;
}

/*******************************************************************************
 *  ring buffer internal functions
 ******************************************************************************/
static inline int rb_is_read_conflict(struct rbuffer_s *rb, vuint64_t ridx)
{
	vuint64_t widxl = vatomic64_read_rlx(&rb->bh->cur_wblk);
	vuint64_t widx = get_write_idx(widxl);
	vuint16_t blk_num = rb->total_blks;

#ifdef EAGER_MOVE_BLOCK
	if (widx - ridx < blk_num) {
		return RB_FALSE;
	}
#else
	if (widx - ridx + get_busy_bit(widxl) < blk_num) {
		return RB_FALSE;
	}
#endif
	return RB_TRUE;
}

static inline void rb_update_reader(struct rbuffer_s *rb, vuint32_t idx,
				    vuint32_t cnt)
{
	vatomic_write_rlx(&rb->bh->blk_read_idx, idx);
	vatomic_write_rlx(&rb->bh->blk_read_cnt, cnt);
}

static inline void rb_reset_reader(struct rbuffer_s *rb)
{
	rb_update_reader(rb, 0, 0);
}

static inline void rb_advance_rblk_conflict(struct rbuffer_s *rb)
{
	vuint64_t widx = get_write_idx(vatomic64_read_rlx(&rb->bh->cur_wblk));
	rb_reset_reader(rb);
	vatomic64_write_rlx(&rb->bh->cur_rblk, widx - rb->move_step);
}

static inline void rb_advance_rblk_normal(struct rbuffer_s *rb, vuint64_t ridx)
{
	vuint64_t widx = get_write_idx(vatomic64_read_rlx(&rb->bh->cur_wblk));
	if (rb_likely(widx > ridx)) {
		rb_reset_reader(rb);
		vatomic64_write_rlx(&rb->bh->cur_rblk, ++ridx);
	}
}

static inline int rb_is_read_unsafe(struct rbuffer_s *rb, vuint64_t ridx)
{
	vuint64_t widxl = vatomic64_read_acq(&rb->bh->cur_wblk);
	vuint64_t widx = get_write_idx(widxl);

	if (rb_likely(widx != ridx)) {
		return RB_FALSE;
	} else {
		if (!is_busy(widxl)) {
			return RB_FALSE;
		}
	}
	return RB_TRUE;
}

static inline void rb_setup_readinfo(struct read_info_s *info, vuint64_t idx,
				     vuint32_t total, vuint32_t cnt)
{
	if (info == NULL) {
		return;
	}
	info->index = idx;
	info->total = total;
	info->blk_read_cnt = cnt;
}

/*******************************************************************************
 *  ring buffer api for users
 ******************************************************************************/
/* rb_init - init ring buffer
 *
 * Params:
 *   rb      - point to struct rbuffer_s object
 *   buf     - address of memory that used to store data.
 *             it must be align with 8.
 *   size    - size of buf
 *   blk_num - block number of ring buffer. blk_num should be
 *             power of 2. If not, rb_init will align down with
 *             power of 2, for example, if blk_num is 9, the
 *             actual blk_num is 8. User should be carefully with
 *             the blk_num to make block size greater than the
 *             maximum entry size.
 *   mode    - mode of ring buffer. Only RB_RDONLY and RB_RDWR are
 *             supported. Each read thread should create a ringbuffer
 *             with RB_RDONLY mode if want to multiple readers.
 * Returns:
 *   Success: 0
 *   Fail:    -1
 */
static int rb_init(struct rbuffer_s *rb, void *buf, size_t size,
		   vuint16_t blk_num, vuint32_t mode) __attribute__((unused));
static int rb_init(struct rbuffer_s *rb, void *buf, size_t size,
		   vuint16_t blk_num, vuint32_t mode)
{
	if (rb == NULL || buf == NULL) {
		return -1;
	}
	/* The buf address should be aligned with 8, because
     * we have vatomic64 at the head of buf */
	if (((unsigned long)(vuintptr_t)buf & ALIGNMENT_MASK) != 0) {
		return -1;
	}
	if (size < sizeof(struct buffer_head_s)) {
		return -1;
	}
	struct buffer_head_s *bh = (struct buffer_head_s *)buf;
	vuint8_t *g_blk = (vuint8_t *)(bh + 1);
	rb->flags = mode;
	rb->g_blk = g_blk;
	rb->bh = bh;

	size_t blks_total_size = size - sizeof(struct buffer_head_s);
	rb->total_size = blks_total_size;
	vuint16_t total_blks =
		blk_num < MIN_BLOCK_NUM ? MIN_BLOCK_NUM : blk_num;

	/* make total blocks align down with power of 2, so we can have a
     * quick computation of the block index by using ">>" instead of "%".
     * For example, if the blks is 9, then the actual block number is 8 */
	total_blks = ALIGN_DOWN_WITH_POWER_2(total_blks);
	if (blks_total_size < total_blks * MIN_BLOCK_SIZE) {
		return -1;
	}
	vuint32_t blk_size = blks_total_size / total_blks;
	if (blk_size >= (1U << ENTRY_INDEX_BITS)) {
		return -1;
	}

	/* The block size should be aligned with 8 */
	blk_size = RB_ALIGN_DOWN(blk_size);

	if (blk_size < MIN_BLOCK_SIZE) {
		return -1;
	}

	rb->total_blks = total_blks;
	rb->blk_size = (vuint16_t)blk_size;
	rb->move_step = total_blks - BACKWARD_STEP;
	if (RB_MODE(mode) == RB_RDONLY) {
		return 0;
	}

	/* only creaters need reset read index */
	vatomic_write_rlx(&rb->bh->blk_read_cnt, 0);
	vatomic_write_rlx(&rb->bh->blk_read_idx, 0);
	vatomic64_write_rlx(&rb->bh->cur_rblk, 0);

	int i;
	for (i = 0; i < total_blks; i++) {
		block_init((struct block_head_s *)(g_blk + i * blk_size));
	}

	vatomic64_init(&bh->cur_wblk, 0);
	return 0;
}

/* rb_try_alloc_entry - try to alloc an entry from ring buffer
 *
 * Params:
 *   rb   - pointer to ring buffer object
 *   size - the desired size of the entry.
 *
 * Returns:
 *   Success: pointer of the entry.
 *   Fail:    NULL. If there is no enough space in the block, it will try to
 *   move the wblk to next block and return NULL. And when other thread is
 * trying to move wblk to next block and hold the ready bit, it also return
 * NULL. Also return NULL when trying to move to next block and next block is
 * not ready(there is still some thread writing on the block). Once return NULL,
 * the caller should try again.
 */
static struct entry_s *rb_try_alloc_entry(struct rbuffer_s *rb, size_t size)
	__attribute__((unused));
static struct entry_s *rb_try_alloc_entry(struct rbuffer_s *rb, size_t size)
{
	if (rb_unlikely(rb == NULL || rb->bh == NULL || size < MIN_ALLOC_SIZE ||
			size > MAX_ALLOC_SIZE(rb->blk_size))) {
		return NULL;
	}
	struct buffer_head_s *bh = rb->bh;

	vuint64_t wblk = vatomic64_read_rlx(&bh->cur_wblk);
	if (!is_busy(wblk)) {
		struct entry_s *entry = block_alloc_entry(
			rb, get_write_idx(wblk), (vuint16_t)size);
		if (entry != NULL) {
			return entry;
		}

		vuint64_t newblk = get_next_write_idx(wblk);
		if (RB_DROP(rb->flags) &&
		    get_write_idx(newblk) - vatomic64_read_acq(&bh->cur_rblk) >=
			    rb->total_blks) {
			return NULL;
		}
		struct block_head_s *nblk =
			global_idx_to_block(rb, get_write_idx(newblk));
		int rc = block_is_ready(nblk);
		if (rb_unlikely(rc < 0)) {
			return NULL;
		}

		if (wblk != vatomic64_read_rlx(&bh->cur_wblk)) {
			return NULL;
		}
#ifdef EAGER_MOVE_BLOCK
		vuint64_t tmp_wblk = set_busy_bit(newblk);
#else
		vuint64_t tmp_wblk = set_busy_bit(wblk);
#endif
		if (vatomic64_cmpxchg_rlx(&bh->cur_wblk, wblk, tmp_wblk) ==
		    wblk) {
			if (rb_likely(rc > 0)) {
				block_reset_metadata(nblk);
			}
			/* block_reset_metadata must happen before write wblk */
			vatomic64_write_rel(&bh->cur_wblk, newblk);
		}
	}
	return NULL;
}

/* rb_commit_entry - commit an entry
 *
 * Params:
 *  rb    - pointer to ring buffer object
 *  entry - the entry to be commited
 *
 * Returns:
 *  Success: committed entries
 *  Fail: 0
 *
 * Note:
 *  This api may not be used by user directly, so has unused attribute
 */
static void rb_commit_entry(struct rbuffer_s *rb, struct entry_s *entry)
	__attribute__((unused));
static void rb_commit_entry(struct rbuffer_s *rb, struct entry_s *entry)
{
	if (rb_unlikely(rb == NULL || rb->bh == NULL || entry == NULL)) {
		return;
	}
	entry_commit(rb, entry);
}

/* rb_write_entry - write an entry to ring buffer
 *
 * Params:
 *  rb        - pointer to ring buffer object
 *  buf       - the data buf
 *  size      - the size of data
 *  try_time  - try times on trying allocte entry
 *  committed - pointer to a vuint64_t, it's used to return current committed
 * entries
 *
 * Returns:
 *  Success: entry size
 *  Fail:    0
 *
 * Note:
 *  This api may not be used by user,so has unused attribute
 *
 */
static size_t rb_write_entry(struct rbuffer_s *rb, void *buf, size_t size,
			     vuint32_t try_time) __attribute__((unused));
static size_t rb_write_entry(struct rbuffer_s *rb, void *buf, size_t size,
			     vuint32_t try_time)
{
	int ret;
	if (rb_unlikely(rb == NULL || buf == NULL)) {
		return 0;
	}

	vuint32_t cnt = 0;
	struct entry_s *entry = NULL;

	do {
		entry = rb_try_alloc_entry(rb, size);
		cnt++;
	} while (entry == NULL && cnt < try_time);

	if (entry == NULL) {
		return 0;
	}

	void *dst = entry_get_data(entry);
	ret = memcpy_s(dst, size, buf, size);
	if (ret != 0) {
		/* Mark entry as invalid when memcpy failed. */
		entry->size = 0;
	}
	entry_commit(rb, entry);
	return ret == 0 ? size : 0;
}

/* rb_try_read_entry - try to read an entry from ring buffer
 *
 * Params:
 *   rb        - pointer to ring buffer object.
 *   user_buf  - buf to store entry data, provided by user.
 *   buff_size - the size of user_buff, the buf should be big enough to store
 * the entry info      - address to read_info_s object. The reader info contains
 * the total entries of current block, and the index of current entry.
 *
 *  Returns:
 *   Success: size of the entry.
 *   Fail:    0 when fails to read valid entry, and the content in buf is
 * invalid
 */
static size_t rb_try_read_entry(struct rbuffer_s *rb, void *user_buf,
				size_t buff_size, struct read_info_s *info)
	__attribute__((unused));
static size_t rb_try_read_entry(struct rbuffer_s *rb, void *user_buf,
				size_t buff_size, struct read_info_s *info)
{
	if (rb_unlikely(rb == NULL || user_buf == NULL || rb->bh == NULL)) {
		return 0;
	}

	vuint64_t ridx = vatomic64_read_rlx(&rb->bh->cur_rblk);
	struct block_head_s *rblk = global_idx_to_block(rb, ridx);
	vuint32_t read_cnt, read_idx;

	/* Check whether current rblk is overwritten */
	if (rb_is_read_conflict(rb, ridx)) {
		rb_advance_rblk_conflict(rb);
		return 0;
	}

	if (rb_is_read_unsafe(rb, ridx)) {
		return 0;
	}

	/* Check all writers are done */
	int alloc_num = block_is_ready(rblk);
	if (alloc_num < 0) {
		return 0;
	}
	/* Check if there are entries remain in the block */
	read_cnt = vatomic_read_rlx(&rb->bh->blk_read_cnt);
	if (read_cnt == (vuint32_t)alloc_num) {
		rb_advance_rblk_normal(rb, ridx);
		return 0;
	}

	read_idx = vatomic_read_rlx(&rb->bh->blk_read_idx);
	struct entry_s *entry = block_fetch_entry(rb, rblk, read_idx);
	if (entry != NULL) {
		size_t sz = (size_t)entry_get_size(entry);

		if (rb_unlikely(buff_size < sz)) {
			return 0;
		}

		if (memcpy_s(user_buf, buff_size, entry_get_data(entry), sz) != 0) {
			return 0;
		}

		vuint16_t next_offset = entry_get_next(entry);

		vatomic_fence_acq();

		if (!rb_is_read_conflict(rb, ridx)) {
			rb_update_reader(rb, read_idx + next_offset,
					 ++read_cnt);
			rb_setup_readinfo(info, ridx, (vuint32_t)alloc_num,
					  read_cnt);
			if (read_cnt == (vuint32_t)alloc_num) {
				rb_advance_rblk_normal(rb, ridx);
			}
			return sz;
		} else {
			rb_advance_rblk_conflict(rb);
		}
	}
	return 0;
}

/* rb_read_entry - read an entry from ring buffer
 *
 * Params:
 *   rb        - pointer to ring buffer object.
 *   user_buf  - buf to store entry data, provided by user.
 *   buff_size - the size of user_buff, the buf should be big enough to store
 * the entry info      - address to read_info_s object. The reader info contains
 * the total entries of current block, and the index of current entry.
 *
 *  Returns:
 *   Success: size of the entry.
 *   Fail:    0 input parameter is invalid
 *  Notes: This API will keep trying to read an vlaid entry. It returns only
 * when read out a valid entry from ring buffer. Be careful when using this API,
 * make sure there is some writer writing the ring buffer.
 */
static size_t rb_read_entry(struct rbuffer_s *rb, void *buf, size_t buff_size,
			    struct read_info_s *info) __attribute__((unused));
static size_t rb_read_entry(struct rbuffer_s *rb, void *buf, size_t buff_size,
			    struct read_info_s *info)
{
	if (rb_unlikely(rb == NULL || buf == NULL || rb->bh == NULL)) {
		return 0;
	}

	size_t size = 0;
	do {
		size = rb_try_read_entry(rb, buf, buff_size, info);
	} while (size == 0);

	return size;
}

/* rb_rewind_reader - move reader pointer to the oldest block.
 *
 * Params:
 *  rb - pointer to ring buffer object
 *
 * Notes:
 *  This api may not be used, so have a unused attribute
 */
static void rb_rewind_reader(struct rbuffer_s *rb) __attribute__((unused));
static void rb_rewind_reader(struct rbuffer_s *rb)
{
	if (rb_unlikely(rb == NULL || rb->bh == NULL)) {
		return;
	}
	vuint64_t widx = get_write_idx(vatomic64_read_rlx(&rb->bh->cur_wblk));
	rb_reset_reader(rb);
	if (rb_likely(widx >= rb->total_blks)) {
		vatomic64_write_rlx(&rb->bh->cur_rblk,
				    widx - rb->total_blks + 1);
	} else {
		vatomic64_write_rlx(&rb->bh->cur_rblk, 0);
	}
}

/* rb_get_write_offset - get write offset from entry
 *
 * Params:
 *  rb         - pointer to ring buffer object
 *  entry      - pointer to entry
 *  entry_size - entry size
 *
 * Returns:
 *  Success: the write offset of current entry
 *  Fail:    -1
 */
static int rb_get_write_offset(struct rbuffer_s *rb, struct entry_s *entry,
			       size_t entry_size) __attribute__((unused));
static int rb_get_write_offset(struct rbuffer_s *rb, struct entry_s *entry,
			       size_t entry_size)
{
	if (rb_unlikely(rb == NULL || entry == NULL)) {
		return -1;
	}
	vuint64_t ridx = vatomic64_read_rlx(&rb->bh->cur_rblk);
	if (rb_is_read_conflict(rb, ridx)) {
		return rb->total_size;
	} else {
		vuint8_t *cur_read = (vuint8_t *)global_idx_to_block(rb, ridx) +
				     sizeof(struct block_head_s) +
				     vatomic_read_rlx(&rb->bh->blk_read_idx);
		vuint8_t *cur_write =
			(vuint8_t *)entry + entry_size + sizeof(struct entry_s);
		if (cur_write >= cur_read) {
			return cur_write - cur_read;
		} else {
			return rb->total_size - (cur_read - cur_write);
		}
	}
}
#endif
