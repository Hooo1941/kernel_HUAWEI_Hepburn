

#ifndef __OAL_OS_H__
#define __OAL_OS_H__

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/sched.h>

typedef struct mutex oal_mutex_t;
typedef spinlock_t oal_spinlock_t;
typedef wait_queue_head_t oal_wait_queue_head_t;

#define unused(var)   \
	do {              \
		(void)(var);  \
	} while (0)

#define oal_spin_lock_irqsave spin_lock_irqsave
#define oal_spin_unlock_irqrestore spin_unlock_irqrestore

static inline void oal_msleep(uint32_t time)
{
	msleep(time);
}
static inline void oal_udelay(uint32_t time)
{
	udelay(time);
}

static inline void oal_mutex_init(oal_mutex_t *lock)
{
	mutex_init(lock);
}
static inline void oal_mutex_destroy(oal_mutex_t *lock)
{
	mutex_destroy(lock);
}
static inline void oal_mutex_lock(oal_mutex_t *lock)
{
	mutex_lock(lock);
}
static inline void oal_mutex_unlock(oal_mutex_t *lock)
{
	mutex_unlock(lock);
}

static inline void oal_init_waitqueue_head(oal_wait_queue_head_t *wq)
{
	init_waitqueue_head(wq);
}

#define oal_delete_waitqueue_head(wq)

#define NFC_OS_EVENT_IS_OK
#ifdef NFC_OS_EVENT_IS_OK
// linux实现中_condition是表达式，必须用宏实现
#define oal_wait_event_interruptible_timeout(_st_wq, _condition, timeout) \
({                                                                        \
	int ret_ = 0;                                                         \
	if ((timeout) == 0) {                                                 \
		ret_ = wait_event_interruptible(_st_wq, _condition);              \
	} else {                                                              \
		ret_ = (wait_event_interruptible_timeout(_st_wq, _condition, msecs_to_jiffies(timeout)) > 0) ? 0 : -1; \
	}                                                                     \
	ret_;                                                                 \
})
#else
extern int board_get_dev_wkup_host_status(void);
static inline int oal_wait_event_interruptible_timeout(oal_wait_queue_head_t wq, int condition, unsigned long timeout)
{
	unused(wq);
	unused(timeout);
	unused(condition);
	while (board_get_dev_wkup_host_status() == 0) {
		oal_msleep(1);
	}
	return 0;
}
#endif

#define oal_wkup_event(x) \
	wake_up(x)

static inline void *oal_malloc(size_t len)
{
	return kmalloc(len, (GFP_KERNEL | __GFP_NOWARN));
}

static inline void oal_free(void *data)
{
	return kfree(data);
}


#endif

