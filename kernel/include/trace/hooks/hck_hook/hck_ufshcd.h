/* SPDX-License-Identifier: GPL-2.0 */
#undef TRACE_SYSTEM
#define TRACE_SYSTEM hck_ufshcd
#define TRACE_INCLUDE_PATH trace/hooks/hck_hook
#if !defined(_TRACE_HCK_HOOK_UFSHCD_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_HCK_HOOK_UFSHCD_H
#include <linux/tracepoint.h>
#include <trace/hooks/hck_hook/hck_vendor_hooks.h>

/*
 * Following tracepoints are not exported in tracefs and provide a
 * mechanism for vendor modules to hook and extend functionality
 */

struct ufs_hba;
struct Scsi_Host;
struct scsi_cmnd;

DECLARE_HCK_HOOK(hck_vh_hufs_uic_hibern8_enter,
		TP_PROTO(struct ufs_hba *hba, int *ret),
		TP_ARGS(hba, ret));
DECLARE_HCK_HOOK(hck_vh_hufs_queuecommand,
		TP_PROTO(struct Scsi_Host *host, struct scsi_cmnd *cmd, int *err),
		TP_ARGS(host, cmd, err));

#endif /* _TRACE_HCK_HOOK_UFSHCD_H */
/* This part must be outside protection */
#include <trace/define_trace.h>

