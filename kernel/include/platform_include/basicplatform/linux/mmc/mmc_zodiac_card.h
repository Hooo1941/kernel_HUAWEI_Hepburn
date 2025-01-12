#ifndef MMC_ZODIAC_CARD_H
#define MMC_ZODIAC_CARD_H

#include <linux/kernel.h>
#include "queue.h"
#include <linux/device.h>

/*
 * There is one mmc_blk_data per slot.
 */
struct mmc_blk_data {
	spinlock_t lock;
	struct device *parent;
	struct gendisk *disk;
	struct mmc_queue queue;
	struct list_head part;
	struct list_head rpmbs;

	unsigned int flags;
#define MMC_BLK_CMD23 (1 << 0) /* Can do SET_BLOCK_COUNT for multiblock */
#define MMC_BLK_REL_WR (1 << 1)	/* MMC Reliable write support */
#define MMC_BLK_PACKED_CMD (1 << 2) /* MMC packed command support */
#define MMC_BLK_CMD_QUEUE (1 << 3) /* MMC command queue support */

	unsigned int usage;
	unsigned int read_only;
	unsigned int part_type;
	unsigned int name_idx;
	unsigned int reset_done;
#define MMC_BLK_READ            BIT(0)
#define MMC_BLK_WRITE           BIT(1)
#define MMC_BLK_DISCARD         BIT(2)
#define MMC_BLK_SECDISCARD      BIT(3)
#define MMC_BLK_CQE_RECOVERY    BIT(4)

	/*
	 * Only set in main mmc_blk_data associated
	 * with mmc_card with dev_set_drvdata, and keeps
	 * track of the current selected device partition.
	 */
	unsigned int part_curr;
	struct device_attribute force_ro;
	struct device_attribute power_ro_lock;
	int area_type;

	/* debugfs files (only in main mmc_blk_data) */
	struct dentry *status_dentry;
	struct dentry *ext_csd_dentry;
};

#define mmc_req_rel_wr(req)                                                    \
	((req->cmd_flags & REQ_FUA) && (rq_data_dir(req) == WRITE))

extern int mmc_blk_reset(struct mmc_blk_data *md, struct mmc_host *host, int type);
#ifdef CONFIG_HW_MMC_MAINTENANCE_DATA
extern void record_cmdq_rw_data(struct mmc_request *mrq)
#endif
extern int mmc_cmdq_discard_queue(struct mmc_host *host, u32 tasks);

struct mmc_blk_data *mmc_blk_get(struct gendisk *disk);
void mmc_blk_put(struct mmc_blk_data *md);
int mmc_blk_part_switch(struct mmc_card *card, unsigned int part_type);
int ioctl_rpmb_card_status_poll(struct mmc_card *card, u32 *status,
				       u32 retries_max);
extern void mmc_cmdq_error_work(struct work_struct *work);
extern enum blk_eh_timer_return mmc_blk_cmdq_req_timed_out(struct request *req);
extern int mmc_blk_cmdq_issue_rq(struct mmc_queue *mq, struct request *req);
extern void mmc_blk_cmdq_setup_queue(struct mmc_queue *mq, struct mmc_card *card);
extern void mmc_cmdq_task_info_init(struct mmc_card *card, struct request *req);

#ifdef CONFIG_MMC_MQ_CQ_HCI
extern int mmc_cmdq_get_tag(struct mmc_card *card, struct request *req);
extern int mmc_blk_cmdq_wait_for_rw(struct mmc_host *host, struct request *req);
extern struct request * mmc_blk_cmdq_mq_find_req(struct request_queue *q, int tag);
#endif

#endif
