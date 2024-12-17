#ifndef __HCK_SCSI_UFS_GEAR_CTRL_H__
#define __HCK_SCSI_UFS_GEAR_CTRL_H__

#include <linux/types.h>
#include <platform_include/basicplatform/linux/hck_vendor_hooks.h>

struct request_queue;
struct Scsi_Host;
struct ufs_hba;

DECLARE_HCK_VH(scsi_dev_gear_ctrl_register,
		TP_PROTO(struct Scsi_Host *shost),
		TP_ARGS(shost));
DECLARE_HCK_VH(ufs_gear_ctrl_adjust_capbility,
		TP_PROTO(struct ufs_hba *hba),
		TP_ARGS(hba));
#endif
