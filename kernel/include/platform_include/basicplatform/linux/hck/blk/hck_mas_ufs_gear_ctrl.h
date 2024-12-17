#ifndef __HCK_MAS_UFS_GEAR_CTRL_H__
#define __HCK_MAS_UFS_GEAR_CTRL_H__

#include <linux/types.h>
#include <platform_include/basicplatform/linux/hck_vendor_hooks.h>

struct request_queue;
struct request;

DECLARE_HCK_VH(mas_queue_busy_change_gear,
		TP_PROTO(struct request_queue *q, struct request *req),
		TP_ARGS(q, req));
DECLARE_HCK_VH(mas_queue_ioback_change_gear,
		TP_PROTO(struct request_queue *q, struct request *req),
		TP_ARGS(q, req));
DECLARE_HCK_VH(mas_blk_fill_gear_info,
		TP_PROTO(char *info, int len),
		TP_ARGS(info, len));
DECLARE_HCK_VH(mas_blk_gear_ctrl_reinit,
		TP_PROTO(void *data),
		TP_ARGS(data));

#endif
