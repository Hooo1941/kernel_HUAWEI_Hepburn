#ifndef _MEMCHECK_H
#define _MEMCHECK_H

#include <linux/types.h>
#ifdef __KERNEL__
#ifdef CONFIG_DFX_MEMCHECK
#include <asm/ioctls.h>
#endif
#else /* __KERNEL__ */
#include <sys/types.h>
#include <sys/ioctl.h>
#endif /* __KERNEL__ */

#define IDX_JAVA 0
#define IDX_NATIVE 1
#define IDX_SLUB 2
#define IDX_VMALLOC 3
#define IDX_ION 4
#define IDX_GPU 5
#define IDX_ASHMEM 6
#define IDX_CMA 7
#define IDX_COUNT 8

#define MTYPE_NONE 0
#define MTYPE_JAVA (1 << IDX_JAVA)
#define MTYPE_NATIVE (1 << IDX_NATIVE)
#define MTYPE_PSS (MTYPE_JAVA | MTYPE_NATIVE)
#define MTYPE_SLUB (1 << IDX_SLUB)
#define MTYPE_VMALLOC (1 << IDX_VMALLOC)
#define MTYPE_ION (1 << IDX_ION)
#define MTYPE_GPU (1 << IDX_GPU)
#define MTYPE_ASHMEM (1 << IDX_ASHMEM)
#define MTYPE_CMA (1 << IDX_CMA)

#define MEMCHECK_CMD_INVALID		0xFFFFFFFF
#define MEMCHECK_MAGIC			0x5377FEFA
#define MEMCHECK_PID_INVALID		0xFFDEADFF
#define MEMCHECK_STACKINFO_MAXSIZE	(600 * 1024)
#define MEMCHECK_DETAILINFO_MAXSIZE	(100 * 1024)

#define __MEMCHECKIO			0xAF
#define LOGGER_MEMCHECK_PSS_READ	_IO(__MEMCHECKIO, 1)
#define LOGGER_MEMCHECK_COMMAND		_IO(__MEMCHECKIO, 2)
#define LOGGER_MEMCHECK_STACK_READ	_IO(__MEMCHECKIO, 3)
#define LOGGER_MEMCHECK_STACK_SAVE	_IO(__MEMCHECKIO, 4)
#define LOGGER_MEMCHECK_DETAIL_READ	_IO(__MEMCHECKIO, 5)
#define LOGGER_MEMCHECK_DETAIL_WRITE	_IO(__MEMCHECKIO, 6)
#define LOGGER_MEMCHECK_MIN		LOGGER_MEMCHECK_PSS_READ
#define LOGGER_MEMCHECK_MAX		LOGGER_MEMCHECK_DETAIL_WRITE

#if defined(CONFIG_DFX_OHOS) && !defined(MUSL_SIGNAL_JSHEAP_PRIV)
#define MUSL_SIGNAL_JSHEAP_PRIV		40
#endif
#define SIGNO_MEMCHECK			44
#define ADDR_JAVA_ENABLE		(1 << 0)
#define ADDR_JAVA_DISABLE		(1 << 1)
#define ADDR_JAVA_SAVE			(1 << 2)
#define ADDR_JAVA_CLEAR			(1 << 3)
#define ADDR_NATIVE_ENABLE		(1 << 4)
#define ADDR_NATIVE_DISABLE		(1 << 5)
#define ADDR_NATIVE_SAVE		(1 << 6)
#define ADDR_NATIVE_CLEAR		(1 << 7)
#define ADDR_NATIVE_DETAIL_INFO		(1 << 8)

/* do not change the order or insert any value before MEMCMD_CLEAR_LOG */
enum memcmd {
	MEMCMD_NONE,
	MEMCMD_ENABLE,
	MEMCMD_DISABLE,
	MEMCMD_SAVE_LOG,
	MEMCMD_CLEAR_LOG,
#ifdef CONFIG_DFX_OHOS
	MEMCMD_JS_HEAP_DUMP,
#endif
	MEMCMD_MAX
};

#ifdef __KERNEL__
#ifdef CONFIG_DFX_MEMCHECK
struct memstat_pss {
	u32 magic;
	u32 id;
	u32 type;
	u64 pss;
	u64 swap;
	u64 java_pss;
	u64 native_pss;
};

struct track_cmd {
	u32 magic;
	u32 id;
	u32 type;
	u64 timestamp;
	enum memcmd cmd;
};

struct detail_info {
	u32 magic;
	u32 id;
	u32 type;
	u64 size;
	u64 timestamp;
	char data[0];
};

struct stack_info {
	u32 magic;
	u32 type;
	u64 size;
	char data[0];
};
#endif /* CONFIG_DFX_MEMCHECK */
#else
struct memstat_pss {
	__u32 magic;
	__u32 id;
	__u32 type;
	__u64 pss;
	__u64 swap;
	__u64 java_pss;
	__u64 native_pss;
};

struct track_cmd {
	__u32 magic;
	__u32 id;
	__u32 type;
	__u64 timestamp;
	enum memcmd cmd;
};

struct detail_info {
	__u32 magic;
	__u32 id;
	__u32 type;
	__u64 size;
	__u64 timestamp;
	char data[0];
};

struct stack_info {
	__u32 magic;
	__u32 type;
	__u64 size;
	char data[0];
};
#endif /* __KERNEL__ */

#ifdef __KERNEL__
struct file;
#ifdef CONFIG_DFX_MEMCHECK
int proc_graphic_mem_operations(struct seq_file *m, struct pid_namespace *ns,
			    struct pid *pid, struct task_struct *task);
long memcheck_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
#else /* CONFIG_DFX_MEMCHECK */
static inline long memcheck_ioctl(struct file *file, unsigned int cmd,
				  unsigned long arg)
{
	return MEMCHECK_CMD_INVALID;
}

extern void seq_printf(struct seq_file *m, const char *f, ...);
static inline int proc_graphic_mem_operations(struct seq_file *m, struct pid_namespace *ns,
					  struct pid *pid, struct task_struct *task)
{
	seq_printf(m, "%lu\n", 0);
	return 0;
}
#endif /* CONFIG_DFX_MEMCHECK */
#endif /* __KERNEL__ */

#endif /* _MEMCHECK_H */
