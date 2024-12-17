/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description: TIoT osal interface. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-05-31, Create file. \n
 */
#ifndef TIOT_OSAL_H
#define TIOT_OSAL_H

#ifdef CONFIG_OSAL_DEFINED
#include "soc_osal.h"
#else
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @defgroup tiot_common_interface_osal TIoT Osal Interface
 * @ingroup  tiot_common_interface
 * @{
 */

#define OSAL_SUCCESS 0
#define OSAL_FAILURE (-1)
#define OSAL_EOVERFLOW (-75)

#define OSAL_WAIT_FOREVER 0xFFFFFFFF
#define OSAL_WAIT_CONDITION_TRUE 1

#define OSAL_GFP_ZERO (0x1)
#define OSAL_GFP_ATOMIC (0x1 << 1)
#define OSAL_GFP_DMA (0x1 << 2)
#define OSAL_GFP_KERNEL (0x1 << 3)

#define OSAL_O_RDONLY 00000000
#define OSAL_O_WRONLY 00000001
#define OSAL_O_RDWR 00000002
#define OSAL_O_ACCMODE 00000003

#define OSAL_O_CREAT 00000100
#define OSAL_O_EXCL 00000200
#define OSAL_O_TRUNC 00001000
#define OSAL_O_APPEND 00002000
#define OSAL_O_CLOEXEC 02000000

#define OSAL_SEEK_SET 0
#define OSAL_SEEK_CUR 1
#define OSAL_SEEK_END 2

/* Osal string. */
#ifdef __KERNEL__
#define osal_strtol(cp, endp, base)    simple_strtol(cp, endp, base)
#else
#define osal_strtol(cp, endp, base)    strtol(cp, endp, base)
#endif

typedef struct {
    void *lock;
} osal_spinlock;

typedef struct {
    void *mutex;
} osal_mutex;

typedef struct {
    void *wait;
} osal_wait;

typedef struct {
    void *task;
} osal_task;

typedef int (*osal_kthread_handler)(void *data);

/* return value is a bool type */
typedef int (*osal_wait_condition_func)(const void *param);

/**
 * @ingroup osal_timer
 * @brief sleep waiting for signals.
 *
 * @par Description:
 * sleep waiting for signals.
 *
 * @param msecs [in] Time in milliseconds to sleep for.
 *
 * @return Returns 0 when the timer has expired otherwise the remaining time in ms will be returned.
 *
 * @par Support System:
 * linux liteos freertos.
 */
unsigned long osal_msleep(unsigned int msecs);

/**
 * @ingroup osal_addr
 * @brief Alloc dynamic memory.
 *
 * @par Description:
 * This API is used to alloc a memory block of which the size is specified.
 *
 * @param size [in] How many bytes of memory are required.
 * @param osal_gfp_flag [in] The type of memory to alloc. This parameter is not used in liteos and freertos.
 * In linux, it must include one of the following access modes: OSAL_GFP_ATOMIC, OSAL_GFP_DMA, OSAL_GFP_KERNEL.
 * OSAL_GFP_ZERO can be bitwise-or'd in flags, Then, the memory is set to zero.
 *
 * @par Support System:
 * linux liteos freertos.
 */
void *osal_kmalloc(unsigned long size, unsigned int osal_gfp_flag);

/**
 * @ingroup osal_addr
 * @brief Free dynamic memory.
 *
 * @par Description:
 * This API is used to free specified dynamic memory that has been allocd and update module mem used.
 *
 * @param addr [in] Starting address of the memory block to be freed.
 *
 * @par Support System:
 * linux liteos freertos.
 */
void osal_kfree(void *addr);

/**
 * @ingroup osal_spinlock
 * @brief Initialize a spin lock.
 *
 * @par Description:
 * This API is used to initialization of spin_lock.
 *
 * @param lock [out] the lock to be initialized.
 *
 * @attention
 * must be free with osal_spin_lock_destroy, other wise will lead to memory leak;
 *
 * @return OSAL_SUCCESS/OSAL_FAILURE
 *
 * @par Support System:
 * linux liteos.
 */
int osal_spin_lock_init(osal_spinlock *lock);

/**
 * @ingroup osal_spinlock
 * @brief acquire the spin_lock.
 *
 * @par Description:
 * Saves the current IRQ status of the CPU, obtains the specified spin_lock, and disables the interrupts of the CPU.
 *
 * @param lock [in] the lock to be acquired.
 *
 * @par Support System:
 * linux liteos freertos.
 */
void osal_spin_lock_irqsave(osal_spinlock *lock, unsigned long *flags);

/**
 * @ingroup osal_spinlock
 * @brief release the spin_lock.
 *
 * @par Description:
 * Releases the specified spin_lock and restores the interrupt status of the CPU, and enables the interrupts of the CPU.
 *
 * @param lock [in] the lock to be released.
 *
 * @par Support System:
 * linux liteos freertos.
 */
void osal_spin_unlock_irqrestore(osal_spinlock *lock, unsigned long *flags);

/**
 * @ingroup osal_spinlock
 * @brief Destroy the spin_lock.
 *
 * @par Description:
 * Destroy the spin_lock.
 *
 * @param lock [in] The lock to be destroyed.
 *
 * @attention
 * must be called when kmod exit, other wise will lead to memory leak;
 * this API will free memory, lock must be from osal_spin_lock_init returns
 *
 * @par Support System:
 * linux liteos.
 */
void osal_spin_lock_destroy(osal_spinlock *lock);

/**
 * @ingroup osal_mutex
 * @brief Initialize the mutex.
 *
 * @par Description:
 * This API is used to initialize the mutex.
 *
 * @param mutex [in] the mutex to be initialized.
 *
 * @return OSAL_FAILURE/OSAL_SUCCESS
 *
 * @par Support System:
 * linux liteos freertos.
 */
int osal_mutex_init(osal_mutex *mutex);

/**
 * @ingroup osal_mutex
 * @brief Acquire the mutex.
 *
 * @par Description:
 * Acquire the mutex. Lock the mutex exclusively for this task.
 * If the mutex is not available right now, it will sleep until it can get it.
 *
 * @param mutex [in] the mutex to be initialized.
 *
 * @return OSAL_FAILURE/OSAL_SUCCESS
 *
 * @note
 * The mutex must later on be released by the same task that acquired it. Recursive locking is not allowed.
 * The task may not exit without first unlocking the mutex.
 * Also, kernel memory where the mutex resides must not be freed with the mutex still locked.
 * The mutex must first be initialized (or statically defined) before it can be locked.
 * memset()-ing the mutex to 0 is not allowed.
 *
 * @par Support System:
 * linux liteos freertos.
 * @par System Differ: Only LiteOS supports the nested lock feature.
 */
int osal_mutex_lock(osal_mutex *mutex);

/**
 * @ingroup osal_mutex
 * @brief Release the mutex.
 *
 * @par Description:
 * Release the mutex. Unlock a mutex that has been locked by this task previously.
 * This function must not be used in interrupt context. Unlocking of a not locked mutex is not allowed.
 *
 * @param mutex [in] the mutex to be released.
 *
 * @par Support System:
 * linux liteos freertos.
 */
void osal_mutex_unlock(osal_mutex *mutex);

/**
 * @ingroup osal_mutex
 * @brief Destroy the mutex.
 *
 * @par Description:
 * This API is used to destroy the mutex.
 *
 * @param mutex [in] The mutex to be destroyed.
 *
 * @attention
 * must be called when kmod exit, otherwise will lead to memory leak; osal_mutex_destroy will free memory,
 * caller should clear pointer to be NULL after called.
 *
 * @par Support System:
 * linux liteos freertos.
 */
void osal_mutex_destroy(osal_mutex *mutex);

/**
 * @ingroup osal_wait
 * @brief Initialize a waiting queue.
 *
 * @par Description:
 * This API is used to Initialize a waiting queue.
 *
 * @param wait [out] The wait queue to be initialized.
 *
 * @par Support System:
 * linux liteos.
 */
int osal_wait_init(osal_wait *wait);

/**
 * @ingroup osal_wait
 * @brief sleep until a condition gets true or a timeout elapses.
 *
 * @par Description:
 * sleep until a condition gets true or a timeout elapses.
 * The process is put to sleep (TASK_INTERRUPTIBLE) until the @func returned true or a signal is received.
 * Return value of the @func is checked each time the waitqueue @wait is woken up.
 *
 * @param wait [in] the waitqueue to wait on.
 * @param wait [in] a C expression for the event to wait for.
 * @param wait [in] timeout, in ms.
 *
 * @par Support System:
 * linux liteos.
 */
int osal_wait_timeout_interruptible(osal_wait *wait, osal_wait_condition_func func, const void *param,
    unsigned long ms);

/**
 * @ingroup osal_wait
 * @brief wake up threads blocked on a waitqueue.
 *
 * @par Description:
 * Wake-up wait queue,pair to @wait_event_interruptible.
 *
 * @param wait [in] The wait to be wake up.
 *
 * @par Support System:
 * linux liteos.
 */
void osal_wait_wakeup_interruptible(osal_wait *wait); // same as osal_wait_wakeup on liteos

/**
 * @ingroup osal_wait
 * @brief to destroy the wait.
 *
 * @par Description:
 * This API is used to destroy the wait.
 *
 * @param wait [in] The wait to be destroyed.
 *
 * @attention this api may free memory, wait should be from osal_complete_init.
 * @par Support System:
 * linux liteos.
 */
void osal_wait_destroy(osal_wait *wait);

#ifdef CONFIG_XFER_TX_SUPPORT_TASK
/**
 * @ingroup osal_task
 *
 * @brief Provides an interface for creating a thread and invokes kthread_run for creating a kernel thread.
 *
 * @par Description:
 * Provides an interface for creating a thread and invokes kthread_run for creating a kernel thread.
 *
 * @param stack_size [in] Size of the stack space of the thread. 0 means default.
 * @param handler [in] Functions to be processed by the thread
 * @param data [in] Function handler data.
 * @param name [in] Thread name displayed.
 *
 * @retval   osal_task*  If the thread is successfully created, the pointer of the thread is returned.
 * @retval   NULL        If the thread fails to be created, NULL is returned.
 *
 * @par Support System:
 * linux liteos freertos.
 */
osal_task *osal_kthread_create(osal_kthread_handler handler, void *data, const char *name, unsigned int stack_size);

/**
 * @ingroup osal_task
 *
 * @brief Used to destroy the threads you created.
 *
 * @par Description:
 * Stops a specified kernel thread.
 *
 * @attention osal_kthread_destroy will free memory of task, caller should clear pointer to be NULL.
 * Note that if you want to destroy a thread, the thread cannot end before calling this function,
 * Or something terrible will happen.
 * When you call the Kthread_stop function, the thread function cannot be finished, otherwise it will oops.
 * @task may be free in api, @task should be from osal_kthread_create.
 *
 * @param task  [in] Threads you want to destroy.
 * @param stop_flag [in] Indicates whether the current thread exits. If the value of stop_flag is 0,
 * The current thread does not exit. The stop flag is not 0.
 *
 * @par Support System:
 * linux liteos freertos.
 */
void osal_kthread_destroy(osal_task *task, unsigned int stop_flag);
#endif

#ifndef CONFIG_FILE_BY_ARRAY
/**
 * @ingroup osal_fileops
 * @brief open file and return file pointer.
 * @param file [in] the filename of file
 * @param flag [in] the opreation flag example: OSAL_O_CREAT
 * @param mode [in] the mode example: OSAL_O_RDONLY
 * @return void* / null
 *
 * @par Support System:
 * linux liteos seliteos freertos.
 */
void *osal_klib_fopen(const char *file, int flags, int mode);

/**
 * @ingroup osal_fileops
 * @brief close file.
 * @param filp [in] the result of osal_klib_fopen
 * @par Support System:
 * linux liteos seliteos freertos.
 */
void osal_klib_fclose(void *filp);

/**
 * @ingroup osal_fileops
 * @brief Kernel read function.
 * @param buf [out] the buffer you want to read in
 * @param size [in] the size of buffer
 * @param filp [in] the result of osal_klib_fopen
 * @return The length that has been read.
 *
 * @par Support System:
 * linux liteos seliteos freertos.
 */
int osal_klib_fread(char *buf, unsigned long size, void *filp);

/**
 * @ingroup osal_fileops
 * @brief Kernel file seek function.
 * @param offset [in] the new position of file
 * @param whence [in] the method of lseek,example:OSAL_SEEK_SET
 * @param filp [in] the result of osal_klib_fopen
 * @par Support System:
 * linux liteos seliteos.
 */
int osal_klib_fseek(long long offset, int whence, void *filp);
#endif

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* CONFIG_OSAL_DEFINED */

#endif
