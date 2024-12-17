#ifndef IO_URING_TYPES_H
#define IO_URING_TYPES_H

#include <linux/blkdev.h>
#include <linux/task_work.h>
#include <linux/bitmap.h>
#include <linux/llist.h>
#include <uapi/linux/io_uring.h>
#include <platform_include/basicplatform/linux/hck/fs/hck_hyperframe.h>

#include "io-wq.h"

struct io_uring {
	u32 head ____cacheline_aligned_in_smp;
	u32 tail ____cacheline_aligned_in_smp;
};

/*
 * This data is shared with the application through the mmap at offsets
 * IORING_OFF_SQ_RING and IORING_OFF_CQ_RING.
 *
 * The offsets to the member fields are published through struct
 * io_sqring_offsets when calling io_uring_setup.
 */
struct io_rings {
	/*
	 * Head and tail offsets into the ring; the offsets need to be
	 * masked to get valid indices.
	 *
	 * The kernel controls head of the sq ring and the tail of the cq ring,
	 * and the application controls tail of the sq ring and the head of the
	 * cq ring.
	 */
	struct io_uring		sq, cq;
	/*
	 * Bitmasks to apply to head and tail offsets (constant, equals
	 * ring_entries - 1)
	 */
	u32			sq_ring_mask, cq_ring_mask;
	/* Ring sizes (constant, power of 2) */
	u32			sq_ring_entries, cq_ring_entries;
	/*
	 * Number of invalid entries dropped by the kernel due to
	 * invalid index stored in array
	 *
	 * Written by the kernel, shouldn't be modified by the
	 * application (i.e. get number of "new events" by comparing to
	 * cached value).
	 *
	 * After a new SQ head value was read by the application this
	 * counter includes all submissions that were dropped reaching
	 * the new SQ head (and possibly more).
	 */
	u32			sq_dropped;
	/*
	 * Runtime SQ flags
	 *
	 * Written by the kernel, shouldn't be modified by the
	 * application.
	 *
	 * The application needs a full memory barrier before checking
	 * for IORING_SQ_NEED_WAKEUP after updating the sq tail.
	 */
	u32			sq_flags;
	/*
	 * Runtime CQ flags
	 *
	 * Written by the application, shouldn't be modified by the
	 * kernel.
	 */
	u32                     cq_flags;
	/*
	 * Number of completion events lost because the queue was full;
	 * this should be avoided by the application by making sure
	 * there are not more requests pending than there is space in
	 * the completion queue.
	 *
	 * Written by the kernel, shouldn't be modified by the
	 * application (i.e. get number of "new events" by comparing to
	 * cached value).
	 *
	 * As completion events come in out of order this counter is not
	 * ordered with any other data.
	 */
	u32			cq_overflow;
	/*
	 * Ring buffer of completion events.
	 *
	 * The kernel writes completion events fresh every time they are
	 * produced, so the application is allowed to modify pending
	 * entries.
	 */
	struct io_uring_cqe	cqes[] ____cacheline_aligned_in_smp;
};

struct io_mapped_ubuf {
	u64		ubuf;
	size_t		len;
	struct		bio_vec *bvec;
	unsigned int	nr_bvecs;
	unsigned long	acct_pages;
};

struct fixed_file_table {
	struct file		**files;
};

struct fixed_file_ref_node {
	struct percpu_ref		refs;
	struct list_head		node;
	struct list_head		file_list;
	struct fixed_file_data		*file_data;
	struct llist_node		llist;
	bool				done;
};

struct fixed_file_data {
	struct fixed_file_table		*table;
	struct io_ring_ctx		*ctx;

	struct fixed_file_ref_node	*node;
	struct percpu_ref		refs;
	struct completion		done;
	struct list_head		ref_list;
	spinlock_t			lock;
};

struct io_buffer {
	struct list_head list;
	__u64 addr;
	__u32 len;
	__u16 bid;
};

struct io_restriction {
	DECLARE_BITMAP(register_op, IORING_REGISTER_LAST);
	DECLARE_BITMAP(sqe_op, IORING_OP_LAST);
	u8 sqe_flags_allowed;
	u8 sqe_flags_required;
	bool registered;
};

struct io_sq_data {
	refcount_t		refs;
	struct mutex		lock;

	/* ctx's that are using this sqd */
	struct list_head	ctx_list;
	struct list_head	ctx_new_list;
	struct mutex		ctx_lock;

	struct task_struct	*thread;
	struct wait_queue_head	wait;
};

struct io_ring_ctx {
	struct {
		struct percpu_ref	refs;
	} ____cacheline_aligned_in_smp;

	struct {
		unsigned int		flags;
		unsigned int		compat: 1;
		unsigned int		limit_mem: 1;
		unsigned int		cq_overflow_flushed: 1;
		unsigned int		drain_next: 1;
		unsigned int		eventfd_async: 1;
		unsigned int		restricted: 1;
		unsigned int		sqo_dead: 1;

		/*
		 * Ring buffer of indices into array of io_uring_sqe, which is
		 * mmapped by the application using the IORING_OFF_SQES offset.
		 *
		 * This indirection could e.g. be used to assign fixed
		 * io_uring_sqe entries to operations and only submit them to
		 * the queue when needed.
		 *
		 * The kernel modifies neither the indices array nor the entries
		 * array.
		 */
		u32			*sq_array;
		unsigned		cached_sq_head;
		unsigned		sq_entries;
		unsigned		sq_mask;
		unsigned		sq_thread_idle;
		unsigned		cached_sq_dropped;
		unsigned		cached_cq_overflow;
		unsigned long		sq_check_overflow;

		struct list_head	defer_list;
		struct list_head	timeout_list;
		struct list_head	cq_overflow_list;

		struct io_uring_sqe	*sq_sqes;
	} ____cacheline_aligned_in_smp;

	struct io_rings	*rings;

	/* IO offload */
	struct io_wq		*io_wq;

	/*
	 * For SQPOLL usage - we hold a reference to the parent task, so we
	 * have access to the ->files
	 */
	struct task_struct	*sqo_task;

	/* Only used for accounting purposes */
	struct mm_struct	*mm_account;

#ifdef CONFIG_BLK_CGROUP
	struct cgroup_subsys_state	*sqo_blkcg_css;
#endif

	struct io_sq_data	*sq_data; /* if using sq thread polling */

	struct wait_queue_head	sqo_sq_wait;
	struct wait_queue_entry	sqo_wait_entry;
	struct list_head	sqd_list;

	/*
	 * If used, fixed file set. Writers must ensure that ->refs is dead,
	 * readers must ensure that ->refs is alive as long as the file* is
	 * used. Only updated through io_uring_register(2).
	 */
	struct fixed_file_data	*file_data;
	unsigned		nr_user_files;

	/* if used, fixed mapped user buffers */
	unsigned		nr_user_bufs;
	struct io_mapped_ubuf	*user_bufs;

	struct user_struct	*user;

	const struct cred	*creds;

#ifdef CONFIG_AUDIT
	kuid_t			loginuid;
	unsigned int		sessionid;
#endif

	struct completion	ref_comp;
	struct completion	sq_thread_comp;

	/* if all else fails... */
	struct io_kiocb		*fallback_req;

#if defined(CONFIG_UNIX)
	struct socket		*ring_sock;
#endif

	struct idr		io_buffer_idr;

	struct idr		personality_idr;

	struct {
		unsigned		cached_cq_tail;
		unsigned		cq_entries;
		unsigned		cq_mask;
		atomic_t		cq_timeouts;
		unsigned		cq_last_tm_flush;
		unsigned long		cq_check_overflow;
		struct wait_queue_head	cq_wait;
		struct fasync_struct	*cq_fasync;
		struct eventfd_ctx	*cq_ev_fd;
	} ____cacheline_aligned_in_smp;

	struct {
		struct mutex		uring_lock;
		wait_queue_head_t	wait;
	} ____cacheline_aligned_in_smp;

	struct {
		spinlock_t		completion_lock;

		/*
		 * ->iopoll_list is protected by the ctx->uring_lock for
		 * io_uring instances that don't use IORING_SETUP_SQPOLL.
		 * For SQPOLL, only the single threaded io_sq_thread() will
		 * manipulate the list, hence no extra locking is needed there.
		 */
		struct list_head	iopoll_list;
		struct hlist_head	*cancel_hash;
		unsigned		cancel_hash_bits;
		bool			poll_multi_file;

		spinlock_t		inflight_lock;
		struct list_head	inflight_list;
	} ____cacheline_aligned_in_smp;

	struct delayed_work		file_put_work;
	struct llist_head		file_put_llist;

	struct work_struct		exit_work;
	struct io_restriction		restrictions;
#ifdef CONFIG_HYPERFRAME
	void			*buf;
#endif
};

/*
 * First field must be the file pointer in all the
 * iocb unions! See also 'struct kiocb' in <linux/fs.h>
 */
struct io_poll_iocb {
	struct file			*file;
	union {
		struct wait_queue_head	*head;
		u64			addr;
	};
	__poll_t			events;
	bool				done;
	bool				canceled;
	struct wait_queue_entry		wait;
};

struct io_close {
	struct file			*file;
	int				fd;
};

struct io_timeout_data {
	struct io_kiocb			*req;
	struct hrtimer			timer;
	struct timespec64		ts;
	enum hrtimer_mode		mode;
};

struct io_accept {
	struct file			*file;
	struct sockaddr __user		*addr;
	int __user			*addr_len;
	int				flags;
	unsigned long			nofile;
};

struct io_sync {
	struct file			*file;
	loff_t				len;
	loff_t				off;
	int				flags;
	int				mode;
};

struct io_cancel {
	struct file			*file;
	u64				addr;
};

struct io_timeout {
	struct file			*file;
	u32				off;
	u32				target_seq;
	struct list_head		list;
};

struct io_timeout_rem {
	struct file			*file;
	u64				addr;
};

struct io_rw {
	/* NOTE: kiocb has the file as the first member, so don't do it here */
	struct kiocb			kiocb;
	u64				addr;
	u64				len;
};

struct io_connect {
	struct file			*file;
	struct sockaddr __user		*addr;
	int				addr_len;
};

struct io_sr_msg {
	struct file			*file;
	union {
		struct user_msghdr __user *umsg;
		void __user		*buf;
	};
	int				msg_flags;
	int				bgid;
	size_t				len;
	struct io_buffer		*kbuf;
};

struct io_open {
	struct file			*file;
	int				dfd;
	bool				ignore_nonblock;
	struct filename			*filename;
	struct open_how			how;
	unsigned long			nofile;
};

struct io_files_update {
	struct file			*file;
	u64				arg;
	u32				nr_args;
	u32				offset;
};

struct io_fadvise {
	struct file			*file;
	u64				offset;
	u32				len;
	u32				advice;
};

struct io_madvise {
	struct file			*file;
	u64				addr;
	u32				len;
	u32				advice;
};

struct io_epoll {
	struct file			*file;
	int				epfd;
	int				op;
	int				fd;
	struct epoll_event		event;
};

struct io_splice {
	struct file			*file_out;
	struct file			*file_in;
	loff_t				off_out;
	loff_t				off_in;
	u64				len;
	unsigned int			flags;
};

struct io_provide_buf {
	struct file			*file;
	__u64				addr;
	__u32				len;
	__u32				bgid;
	__u16				nbufs;
	__u16				bid;
};

struct io_statx {
	struct file			*file;
	int				dfd;
	unsigned int			mask;
	unsigned int			flags;
	const char __user		*filename;
	struct statx __user		*buffer;
};

struct io_completion {
	struct file			*file;
	struct list_head		list;
	u32				cflags;
};

enum {
	REQ_F_FIXED_FILE_BIT	= IOSQE_FIXED_FILE_BIT,
	REQ_F_IO_DRAIN_BIT	= IOSQE_IO_DRAIN_BIT,
	REQ_F_LINK_BIT		= IOSQE_IO_LINK_BIT,
	REQ_F_HARDLINK_BIT	= IOSQE_IO_HARDLINK_BIT,
	REQ_F_FORCE_ASYNC_BIT	= IOSQE_ASYNC_BIT,
	REQ_F_BUFFER_SELECT_BIT	= IOSQE_BUFFER_SELECT_BIT,

	REQ_F_LINK_HEAD_BIT,
	REQ_F_FAIL_LINK_BIT,
	REQ_F_INFLIGHT_BIT,
	REQ_F_CUR_POS_BIT,
	REQ_F_NOWAIT_BIT,
	REQ_F_LINK_TIMEOUT_BIT,
	REQ_F_ISREG_BIT,
	REQ_F_NEED_CLEANUP_BIT,
	REQ_F_POLLED_BIT,
	REQ_F_BUFFER_SELECTED_BIT,
	REQ_F_NO_FILE_TABLE_BIT,
	REQ_F_WORK_INITIALIZED_BIT,
	REQ_F_LTIMEOUT_ACTIVE_BIT,

	/* not a real bit, just to check we're not overflowing the space */
	__REQ_F_LAST_BIT,
};

enum {
	/* ctx owns file */
	REQ_F_FIXED_FILE	= BIT(REQ_F_FIXED_FILE_BIT),
	/* drain existing IO first */
	REQ_F_IO_DRAIN		= BIT(REQ_F_IO_DRAIN_BIT),
	/* linked sqes */
	REQ_F_LINK		= BIT(REQ_F_LINK_BIT),
	/* doesn't sever on completion < 0 */
	REQ_F_HARDLINK		= BIT(REQ_F_HARDLINK_BIT),
	/* IOSQE_ASYNC */
	REQ_F_FORCE_ASYNC	= BIT(REQ_F_FORCE_ASYNC_BIT),
	/* IOSQE_BUFFER_SELECT */
	REQ_F_BUFFER_SELECT	= BIT(REQ_F_BUFFER_SELECT_BIT),

	/* head of a link */
	REQ_F_LINK_HEAD		= BIT(REQ_F_LINK_HEAD_BIT),
	/* fail rest of links */
	REQ_F_FAIL_LINK		= BIT(REQ_F_FAIL_LINK_BIT),
	/* on inflight list */
	REQ_F_INFLIGHT		= BIT(REQ_F_INFLIGHT_BIT),
	/* read/write uses file position */
	REQ_F_CUR_POS		= BIT(REQ_F_CUR_POS_BIT),
	/* must not punt to workers */
	REQ_F_NOWAIT		= BIT(REQ_F_NOWAIT_BIT),
	/* has or had linked timeout */
	REQ_F_LINK_TIMEOUT	= BIT(REQ_F_LINK_TIMEOUT_BIT),
	/* regular file */
	REQ_F_ISREG		= BIT(REQ_F_ISREG_BIT),
	/* needs cleanup */
	REQ_F_NEED_CLEANUP	= BIT(REQ_F_NEED_CLEANUP_BIT),
	/* already went through poll handler */
	REQ_F_POLLED		= BIT(REQ_F_POLLED_BIT),
	/* buffer already selected */
	REQ_F_BUFFER_SELECTED	= BIT(REQ_F_BUFFER_SELECTED_BIT),
	/* doesn't need file table for this request */
	REQ_F_NO_FILE_TABLE	= BIT(REQ_F_NO_FILE_TABLE_BIT),
	/* io_wq_work is initialized */
	REQ_F_WORK_INITIALIZED	= BIT(REQ_F_WORK_INITIALIZED_BIT),
	/* linked timeout is active, i.e. prepared by link's head */
	REQ_F_LTIMEOUT_ACTIVE	= BIT(REQ_F_LTIMEOUT_ACTIVE_BIT),
};

struct async_poll {
	struct io_poll_iocb	poll;
	struct io_poll_iocb	*double_poll;
};

/*
 * NOTE! Each of the iocb union members has the file pointer
 * as the first entry in their struct definition. So you can
 * access the file pointer through any of the sub-structs,
 * or directly as just 'ki_filp' in this struct.
 */
struct io_kiocb {
	union {
		struct file		*file;
		struct io_rw		rw;
		struct io_poll_iocb	poll;
		struct io_accept	accept;
		struct io_sync		sync;
		struct io_cancel	cancel;
		struct io_timeout	timeout;
		struct io_timeout_rem	timeout_rem;
		struct io_connect	connect;
		struct io_sr_msg	sr_msg;
		struct io_open		open;
		struct io_close		close;
		struct io_files_update	files_update;
		struct io_fadvise	fadvise;
		struct io_madvise	madvise;
		struct io_epoll		epoll;
		struct io_splice	splice;
		struct io_provide_buf	pbuf;
		struct io_statx		statx;
		/* use only after cleaning per-op data, see io_clean_op() */
		struct io_completion	compl;
	};

	/* opcode allocated if it needs to store data for async defer */
	void				*async_data;
	u8				opcode;
	/* polled IO has completed */
	u8				iopoll_completed;

	u16				buf_index;
	u32				result;

	struct io_ring_ctx		*ctx;
	unsigned int			flags;
	refcount_t			refs;
	struct task_struct		*task;
	u64				user_data;

	struct list_head		link_list;

	/*
	 * 1. used with ctx->iopoll_list with reads/writes
	 * 2. to track reqs with ->files (see io_op_def::file_table)
	 */
	struct list_head		inflight_entry;

	struct list_head		iopoll_entry;

	struct percpu_ref		*fixed_file_refs;
	struct callback_head		task_work;
	/* for polled requests, i.e. IORING_OP_POLL_ADD and async armed poll */
	struct hlist_node		hash_node;
	struct async_poll		*apoll;
	struct io_wq_work		work;
#ifdef CONFIG_HYPERFRAME
	void			*buf;
#endif
};

#endif
