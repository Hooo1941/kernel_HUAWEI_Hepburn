#ifndef __HCK_UFS_MAS_MCQ_H__
#define __HCK_UFS_MAS_MCQ_H__

#include <linux/types.h>
#include <platform_include/basicplatform/linux/hck_vendor_hooks.h>

struct Scsi_Host;
struct request_queue;

DECLARE_HCK_VH(ufs_mcq_cpu_sche_disable,
		TP_PROTO(const struct request_queue *q),
		TP_ARGS(q));
DECLARE_HCK_VH(ufs_mcq_cpu_sche_enable,
		TP_PROTO(const struct request_queue *q),
		TP_ARGS(q));
DECLARE_HCK_VH(dfa_mcq_set_en,
		TP_PROTO(struct Scsi_Host *shost),
		TP_ARGS(shost));
#endif
