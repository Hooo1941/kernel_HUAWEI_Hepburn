/**
 * Copyright (c) @CompanyNameMagicTag 2024-2024. All rights reserved. \n
 *
 * Description: kernel osal port demo. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2024-01-04, Create file. \n
 */

#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/fs.h>
#include <linux/kthread.h>
#include <linux/version.h>
#include "tiot_osal.h"

unsigned long osal_msleep(unsigned int msecs)
{
    return msleep_interruptible(msecs);
}

static unsigned int osal_convert_gfp_flag(unsigned int osal_gfp_flag)
{
    unsigned int gfp_flag = 0;

    if (osal_gfp_flag & OSAL_GFP_KERNEL) {
        gfp_flag = GFP_KERNEL;
    } else if (osal_gfp_flag & OSAL_GFP_ATOMIC) {
        gfp_flag = GFP_ATOMIC;
    } else if (osal_gfp_flag & OSAL_GFP_DMA) {
        gfp_flag = GFP_DMA;
    } else {
        gfp_flag = GFP_KERNEL;
    }

    if (osal_gfp_flag & OSAL_GFP_ZERO) {
        gfp_flag |= __GFP_ZERO;
    }
    return gfp_flag;
}

void *osal_kmalloc(unsigned long size, unsigned int osal_gfp_flag)
{
    unsigned int gfp_flag = osal_convert_gfp_flag(osal_gfp_flag);
    return kmalloc(size, gfp_flag);
}

void osal_kfree(void *addr)
{
    if (addr != NULL) {
        kfree(addr);
    }
}

int osal_spin_lock_init(osal_spinlock *lock)
{
    spinlock_t *p = NULL;

    if (lock == NULL || lock->lock != NULL) {
        printk("parameter invalid!\n");
        return OSAL_FAILURE;
    }
    p = (spinlock_t *)kmalloc(sizeof(spinlock_t), GFP_KERNEL); // 8 is malloc size
    if (p == NULL) {
        printk("kmalloc error!\n");
        return OSAL_FAILURE;
    }
    spin_lock_init(p);
    lock->lock = p;
    return OSAL_SUCCESS;
}

void osal_spin_lock_irqsave(osal_spinlock *lock, unsigned long *flags)
{
    spinlock_t *p = NULL;
    unsigned long f;

    if ((lock == NULL) || (flags == NULL) || (lock->lock == NULL)) {
        printk("parameter invalid!\n");
        return;
    }

    p = (spinlock_t *)(lock->lock);
    spin_lock_irqsave(p, f);

    *flags = f;
}

void osal_spin_unlock_irqrestore(osal_spinlock *lock, unsigned long *flags)
{
    spinlock_t *p = NULL;

    if ((lock == NULL) || (flags == NULL) || (lock->lock == NULL)) {
        printk("parameter invalid!\n");
        return;
    }

    p = (spinlock_t *)(lock->lock);
    spin_unlock_irqrestore(p, *flags);
}

void osal_spin_lock_destroy(osal_spinlock *lock)
{
    spinlock_t *p = NULL;

    if (lock == NULL) {
        printk("parameter invalid!\n");
        return;
    }

    p = (spinlock_t *)(lock->lock);
    kfree(p);
    lock->lock = NULL;
}

int osal_mutex_init(osal_mutex *mutex)
{
    struct mutex *p = NULL;

    if (mutex == NULL || mutex->mutex != NULL) {
        printk("parameter invalid!\n");
        return OSAL_FAILURE;
    }

    p = kmalloc(sizeof(struct mutex), GFP_KERNEL);
    if (p == NULL) {
        printk("kmalloc error!\n");
        return OSAL_FAILURE;
    }

    mutex_init(p);
    mutex->mutex = p;

    return OSAL_SUCCESS;
}

int osal_mutex_lock(osal_mutex *mutex)
{
    struct mutex *p = NULL;

    if (mutex == NULL || mutex->mutex == NULL) {
        printk("parameter invalid!\n");
        return OSAL_FAILURE;
    }

    p = (struct mutex *)(mutex->mutex);
    mutex_lock(p);

    return OSAL_SUCCESS;
}

void osal_mutex_unlock(osal_mutex *mutex)
{
    struct mutex *p = NULL;

    if (mutex == NULL || mutex->mutex == NULL) {
        printk("parameter invalid!\n");
        return;
    }

    p = (struct mutex *)(mutex->mutex);

    mutex_unlock(p);
}

void osal_mutex_destroy(osal_mutex *mutex)
{
    struct mutex *p = NULL;

    if (mutex == NULL || mutex->mutex == NULL) {
        printk("parameter invalid!\n");
        return;
    }
    p = (struct mutex *)(mutex->mutex);
    kfree(p);
    mutex->mutex = NULL;
}

int osal_wait_init(osal_wait *wait)
{
    wait_queue_head_t *wq = NULL;
    if (wait == NULL || wait->wait != NULL) {
        printk("parameter invalid!\n");
        return OSAL_FAILURE;
    }
    wq = (wait_queue_head_t *)kmalloc(sizeof(wait_queue_head_t), GFP_ATOMIC);
    if (wq == NULL) {
        printk("kmalloc error!\n");
        return OSAL_FAILURE;
    }
    init_waitqueue_head(wq);
    wait->wait = wq;
    return OSAL_SUCCESS;
}

int osal_wait_timeout_interruptible(osal_wait *wait, osal_wait_condition_func func, const void *param,
    unsigned long ms)
{
    wait_queue_head_t *wq = NULL;

    if (wait == NULL || wait->wait == NULL) {
        printk("parameter invalid!\n");
        return OSAL_FAILURE;
    }

    wq = (wait_queue_head_t *)(wait->wait);

    if (func == NULL) {
        return wait_event_interruptible_timeout((*wq), OSAL_WAIT_CONDITION_TRUE, msecs_to_jiffies(ms));
    }

    return wait_event_interruptible_timeout((*wq), func(param), msecs_to_jiffies(ms));
}

void osal_wait_wakeup_interruptible(osal_wait *wait)
{
    wait_queue_head_t *wq = NULL;

    if (wait == NULL || wait->wait == NULL) {
        printk("wait is NULL!\n");
        return;
    }

    wq = (wait_queue_head_t *)(wait->wait);

    wake_up_interruptible(wq);
}

void osal_wait_destroy(osal_wait *wait)
{
    wait_queue_head_t *wq = NULL;

    if (wait == NULL || wait->wait == NULL) {
        printk("wait is NULL!\n");
        return;
    }

    wq = (wait_queue_head_t *)(wait->wait);
    kfree(wq);
    wait->wait = NULL;
}

#ifdef CONFIG_XFER_TX_SUPPORT_TASK
/* 不需要发送线程可不实现。 */
osal_task *osal_kthread_create(osal_kthread_handler handler, void *data, const char *name, unsigned int stack_size)
{
    struct task_struct *k = NULL;
    osal_task *p = NULL;

    if (handler == NULL) {
        printk("task handler is null!\n");
        return NULL;
    }

    p = (osal_task *)kzalloc(sizeof(osal_task), GFP_KERNEL);
    if (p == NULL) {
        printk("kmalloc error!\n");
        return NULL;
    }

    k = kthread_run(handler, data, name);
    if (IS_ERR(k)) {
        printk("kthread create error! k = %#x!\n", k);
        kfree(p);
        p = NULL;
        return NULL;
    }
    p->task = k;
    return p;
}

void osal_kthread_destroy(osal_task *task, unsigned int stop_flag)
{
    if (task == NULL || task->task == NULL) {
        printk("parameter invalid!\n");
        return;
    }
    /* note: When you call the Kthread_stop function, the thread function cannot be finished, otherwise it will oops. */
    if (stop_flag != 0 && task->task != NULL) {
        kthread_stop((struct task_struct *)(task->task));
    }
    task->task = NULL;
    kfree(task);
}
#endif

#ifndef CONFIG_FILE_BY_ARRAY
static struct file *klib_fopen(const char *file, int flags, int mode)
{
    struct file *filp = filp_open(file, flags, mode);
    return (IS_ERR(filp)) ? NULL : filp;
}

static void klib_fclose(struct file *filp)
{
    if (filp != NULL) {
        filp_close(filp, NULL);
    }
    return;
}

static int klib_fwrite(const char *buf, unsigned long size, struct file *filp)
{
    int writelen;

    writelen = kernel_write(filp, buf, size, &filp->f_pos);
    return writelen;
}

static int klib_fread(char *buf, unsigned long size, struct file *filp)
{
    int readlen;
    readlen = kernel_read(filp, buf, size, &filp->f_pos);

    return readlen;
}

void *osal_klib_fopen(const char *file, int flags, int mode)
{
    if (file == NULL) {
        printk("file NULL!\n");
        return NULL;
    }

    return (void *)klib_fopen(file, flags, mode);
}

void osal_klib_fclose(void *filp)
{
    if (filp == NULL) {
        printk("filp NULL!\n");
        return;
    }

    klib_fclose((struct file *)filp);
}

int osal_klib_fread(char *buf, unsigned long size, void *filp)
{
    if ((buf == NULL) || (filp == NULL)) {
        printk("buf&filp NULL!\n");
        return OSAL_FAILURE;
    }

    return klib_fread(buf, size, (struct file *)filp);
}

int osal_klib_fseek(long long offset, int whence, void *filp)
{
    int ret;
    loff_t res;

    if (filp == NULL) {
        printk("filp NULL!\n");
        return OSAL_FAILURE;
    }

    res = vfs_llseek(filp, offset, whence);
    ret = (int)res;
    if (res != (loff_t)ret) {
        ret = OSAL_EOVERFLOW;
    }

    return ret;
}
#endif