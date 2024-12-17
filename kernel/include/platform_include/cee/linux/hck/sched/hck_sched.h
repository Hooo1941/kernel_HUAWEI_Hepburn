#ifndef __HCK_SCHED__
#define __HCK_SCHED__

#include <linux/types.h>
#include <platform_include/basicplatform/linux/hck_vendor_hooks.h>

/*
 * Following tracepoints are not exported in tracefs and provide a
 * mechanism for vendor modules to hook and extend functionality
 */

struct task_struct;
struct file;

DECLARE_HCK_VH(vendor_init_task_qos,
	TP_PROTO(struct task_struct *p),
	TP_ARGS(p));

DECLARE_HCK_VH(vendor_sched_exit_qos_list,
	TP_PROTO(struct task_struct *p),
	TP_ARGS(p));

DECLARE_HCK_VH(vendor_do_qos_ctrl_ioctl,
	TP_PROTO(struct file *file, unsigned int cmd, unsigned long arg, long *ret),
	TP_ARGS(file, cmd, arg, ret));

DECLARE_HCK_VH(vendor_check_authorized,
	TP_PROTO(unsigned int func_id, unsigned int type, bool *ret),
	TP_ARGS(func_id, type, ret));

DECLARE_HCK_VH(vendor_wgcm_clear_child,
	TP_PROTO(struct task_struct *p),
	TP_ARGS(p));

DECLARE_HCK_VH(vendor_wgcm_do_exit,
	TP_PROTO(struct task_struct *tsk),
	TP_ARGS(tsk));

DECLARE_HCK_VH(vendor_wgcm_activate_task,
	TP_PROTO(struct task_struct *p),
	TP_ARGS(p));

DECLARE_HCK_VH(vendor_wgcm_deactivate_task,
	TP_PROTO(struct task_struct *p, int flags),
	TP_ARGS(p, flags));

DECLARE_HCK_VH(vendor_wgcm_ctl,
	TP_PROTO(unsigned long func_id, unsigned long arg, int *ret),
	TP_ARGS(func_id, arg, ret));

#endif /* __HCK_SCHED__ */
