/**
 * Copyright (c) @CompanyNameMagicTag 2022-2023. All rights reserved.
 *
 * Description: Create a log system file node and register the driver.
 * Author: @CompanyNameTag
 */

#include <linux/miscdevice.h>

#include "plat_pm.h"
#include "bfgx_dev.h"
#include "bfgx_core.h"
#include "bfgx_data_parse.h"
#include "bfgx_exception_rst.h"
#include "plat_exception_rst.h"
#include "plat_debug.h"

// oam log ioctl parameter
#define PLAT_CFG_IOC_MAGIC                   'z'
#define PLAT_DFR_CFG_CMD                     _IOW(PLAT_CFG_IOC_MAGIC, PLAT_DFR_CFG, int)
#define PLAT_BEATTIMER_TIMEOUT_RESET_CFG_CMD _IOW(PLAT_CFG_IOC_MAGIC, PLAT_BEATTIMER_TIMEOUT_RESET_CFG, int)


/*
 * Prototype    : hw_debug_open
 * Description  : functions called from above oam hal,when open debug file
 * input        : "/dev/hwbfgdbg"
 * output       : return 0 --> open is ok
 *              : return !0--> open is false
 */
STATIC int32_t hw_debug_open(struct inode *inode, struct file *filp)
{
    struct ps_core_s *ps_core_d = NULL;
    struct bfgx_dev_node *dev_node = NULL;

    if (unlikely((inode == NULL) || (filp == NULL) || (filp->private_data == NULL))) {
        ps_print_err("input parameter is NULL\n");
        return -EINVAL;
    }

    dev_node = oal_container_of((struct miscdevice *)filp->private_data,
                                struct bfgx_dev_node, c);
    ps_core_d = (struct ps_core_s *)dev_node->core;
    if (ps_core_d == NULL) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    oal_atomic_inc(&dev_node->open_cnt);
    ps_print_info("[%s]open cnt=%d\n", index2name(ps_core_d->uart_index), oal_atomic_read(&dev_node->open_cnt));
    oal_atomic_set(&ps_core_d->dbg_recv_dev_log, 1);
    oal_atomic_set(&ps_core_d->dbg_read_delay, DBG_READ_DEFAULT_TIME);

    return 0;
}

/*
 * Prototype    : hw_debug_write
 * Description  : functions called from above oam hal,write count data to buf
 */
#ifdef PLATFORM_DEBUG_ENABLE
STATIC int32_t hw_debug_write_check(struct ps_core_s *ps_core_d)
{
    if (is_bfgx_exception() == PLAT_EXCEPTION_RESET_BUSY) {
        ps_print_err("dfr is processing, skip.\n");
        return -EBUSY;
    }

    if (ps_core_chk_bfgx_active(ps_core_d) == false) {
        ps_print_err("bfg is closed, cant't write!!!\n");
        return -EINVAL;
    }
    /* _PRE_PM_TTY_OFF支持低功耗时关闭tty，唤醒后重新开启 */
#ifndef _PRE_PM_TTY_OFF
    if (ps_core_d->tty_have_open == false) {
        ps_print_err("%s,tty is closed skip!\n", __func__);
        return -EFAULT;
    }
#endif
    return OAL_SUCC;
}

STATIC ssize_t hw_debug_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    struct ps_core_s *ps_core_d = NULL;
    struct sk_buff *skb;
    uint16_t total_len;
    int32_t ret, prepare_result;

    ps_core_d = get_ps_core_from_miscdev(filp->private_data);
    if (unlikely((ps_core_d == NULL) || (buf == NULL))) {
        ps_print_err("ps_core_d & buf is NULL\n");
        return -EINVAL;
    }

    if (hw_debug_write_check(ps_core_d) < 0) {
        ps_print_err("check failed\n");
        return -EINVAL;
    }

    if (count > (DBG_TX_MAX_FRAME - PS_PKG_SIZE)) {
        ps_print_err("err: dbg packet is too large!\n");
        return -EINVAL;
    }

    /* if low queue num > MAX_NUM and don't write */
    if (oal_netbuf_list_len(&ps_core_d->tx_low_seq) > TX_LOW_QUE_MAX_NUM) {
        return 0;
    }

    ret = prepare_to_visit_node(ps_core_d, &prepare_result);
    if (ret < 0) {
        ps_print_err("prepare work FAIL\n");
        return ret;
    }
    /* modify expire time of uart idle timer */
    mod_timer(&ps_core_d->pm_data->bfg_timer, jiffies + msecs_to_jiffies(BT_SLEEP_TIME));
    oal_atomic_inc(&ps_core_d->pm_data->bfg_timer_mod_cnt);

    total_len = (uint16_t)(count + PS_PKG_SIZE);
    skb = ps_alloc_skb(total_len);
    if (skb == NULL) {
        ps_print_err("ps alloc skb mem fail\n");
        post_to_visit_node(ps_core_d, prepare_result);
        return -EFAULT;
    }

    if (copy_from_user(&skb->data[PS_HEAD_SIZE], buf, count)) {
        ps_print_err("copy_from_user from dbg is err\n");
        kfree_skb(skb);
        post_to_visit_node(ps_core_d, prepare_result);
        return -EFAULT;
    }
    ps_print_info("send %d byte data\n", total_len);
    ps_add_packet_head(skb->data, OML_MSG, total_len);
    ps_skb_enqueue(ps_core_d, skb, TX_LOW_QUEUE);
    ps_core_tx_work_add(ps_core_d);

    post_to_visit_node(ps_core_d, prepare_result);

    return count;
}
#endif

STATIC ssize_t hw_debug_read(struct file *filp, char __user *buf,
                             size_t count, loff_t *f_pos)
{
    struct ps_core_s *ps_core_d = NULL;
    struct sk_buff *skb = NULL;
    uint16_t count1 = 0;
    long timeout;
    unsigned long ret;

    ps_core_d = get_ps_core_from_miscdev(filp->private_data);
    if (unlikely((buf == NULL) || (ps_core_d == NULL))) {
        ps_print_err("input parameter is NULL\n");
        return -EINVAL;
    }

    if (skb_queue_len(&ps_core_d->rx_dbg_seq) == 0) { /* if no data, and wait timeout function */
        if (filp->f_flags & O_NONBLOCK) {  /* if O_NONBLOCK read and return */
            return -EAGAIN;
        }

        /* timeout function;when have data,can interrupt */
        timeout = oal_wait_event_interruptible_timeout_m(ps_core_d->rx_dbg_wait,
            (oal_netbuf_list_len(&ps_core_d->rx_dbg_seq) > 0),
            (long)msecs_to_jiffies(oal_atomic_read(&ps_core_d->dbg_read_delay)));
        if (!timeout) {
            ps_print_dbg("debug read time out!\n");
            return -ETIMEDOUT;
        }
    }

    /* pull skb data from skb queue */
    if ((skb = ps_skb_dequeue(ps_core_d, RX_DBG_QUEUE)) == NULL) {
        ps_print_dbg("dbg read no data!\n");
        return -ETIMEDOUT;
    }
    /* read min value from skb->len or count */
    count1 = min_t(size_t, skb->len, count);
    ret = copy_to_user(buf, skb->data, count1);
    if (ret != 0) {
        ps_print_err("[%s]debug copy_to_user is err, ret=%lu, count1=%d\n",
                     index2name(ps_core_d->uart_index), ret, count1);
        ps_restore_skbqueue(ps_core_d, skb, RX_DBG_QUEUE);
        return -EFAULT;
    }

    skb_pull(skb, count1);

    if (skb->len == 0) { /* curr skb data have read to user */
        kfree_skb(skb);
    } else { /* if don,t read over; restore to skb queue */
        ps_restore_skbqueue(ps_core_d, skb, RX_DBG_QUEUE);
    }

    return count1;
}

/*
 * 函 数 名  : plat_dfr_cfg_set
 * 功能描述  : dfr全局配置
 */
STATIC void plat_dfr_cfg_set(unsigned long arg)
{
    struct st_exception_info *pst_exception_data = get_exception_info_reference();
    if (pst_exception_data == NULL) {
        ps_print_err("get exception info reference is error\n");
        return;
    }

    pst_exception_data->exception_reset_enable = (arg == OAL_FALSE) ? (OAL_FALSE) : (OAL_TRUE);

    ps_print_info("plat dfr cfg set value = %ld\n", arg);
}

/*
 * Prototype    : hw_debug_ioctl
 * Description  : called by ini_plat_dfr_set
 */
STATIC long hw_debug_ioctl(struct file *file, uint32_t cmd, unsigned long arg)
{
    int32_t ret = 0;

    if (file == NULL) {
        ps_print_err("file is null\n");
        return -EINVAL;
    }

    switch (cmd) {
        case PLAT_DFR_CFG_CMD:
            plat_dfr_cfg_set(arg);
            break;
        case PLAT_BEATTIMER_TIMEOUT_RESET_CFG_CMD:
            plat_beat_timeout_reset_set(arg);
            break;
        default:
            ps_print_warning("hw_debug_ioctl cmd = %d not find\n", cmd);
            ret = -EINVAL;
            break;
    }

    return ret;
}

/*
 * Prototype    : hw_debug_release
 * Description  : called by oam func from hal when close debug inode
 * input        : have opened file handle
 * output       : return 0 --> close is ok
 *                return !0--> close is false
 */
STATIC int32_t hw_debug_release(struct inode *inode, struct file *filp)
{
    struct ps_core_s *ps_core_d = NULL;
    struct bfgx_dev_node *dev_node = NULL;

    if (unlikely((filp == NULL) || (filp->private_data == NULL))) {
        ps_print_err("input parameter is NULL\n");
        return -EINVAL;
    }

    dev_node = oal_container_of((struct miscdevice *)filp->private_data,
                                struct bfgx_dev_node, c);
    ps_core_d = dev_node->core;
    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    oal_atomic_dec(&dev_node->open_cnt);
    ps_print_info("[%s]open cnt=%d", index2name(ps_core_d->uart_index), oal_atomic_read(&dev_node->open_cnt));
    if (oal_atomic_read(&dev_node->open_cnt) == 0) {
        /* wake up bt dbg wait queue */
        oal_wait_queue_wake_up_interrupt(&ps_core_d->rx_dbg_wait);
        atomic_set(&ps_core_d->dbg_recv_dev_log, 0);

        /* kfree have rx dbg skb */
        ps_kfree_skb(ps_core_d, RX_DBG_QUEUE);
    }

    return 0;
}

STATIC const struct file_operations g_hw_default_debug_fops = {
    .owner = THIS_MODULE,
    .open = hw_debug_open,
#ifdef PLATFORM_DEBUG_ENABLE
    .write = hw_debug_write,
#endif
    .read = hw_debug_read,
    .unlocked_ioctl = hw_debug_ioctl,
    .release = hw_debug_release,
};

STATIC struct bfgx_subsys_driver_desc g_default_bfgxdbg_driver_desc = {
    .file_ops = &g_hw_default_debug_fops
};

struct bfgx_subsys_driver_desc *get_bfgxdbg_default_drv_desc(void)
{
    return &g_default_bfgxdbg_driver_desc;
}

// memdump ioctl parameter
#define PLAT_BFGX_EXCP_CFG_IOC_MAGIC        'b'
#define PLAT_BFGX_DUMP_FILE_READ_CMD        _IOW(PLAT_BFGX_EXCP_CFG_IOC_MAGIC, PLAT_BFGX_DUMP_FILE_READ, int)
#define PLAT_ME_DUMP_FILE_READ_CMD          _IOW(PLAT_BFGX_EXCP_CFG_IOC_MAGIC, PLAT_ME_DUMP_FILE_READ, int)
#define PLAT_WIFI_EXCP_CFG_IOC_MAGIC        'w'
#define PLAT_WIFI_DUMP_FILE_READ_CMD        _IOW(PLAT_WIFI_EXCP_CFG_IOC_MAGIC, PLAT_WIFI_DUMP_FILE_READ, int)
#define DFR_HAL_GNSS_CFG_CMD                _IOW(PLAT_BFGX_EXCP_CFG_IOC_MAGIC, DFR_HAL_GNSS, int)
#define DFR_HAL_BT_CFG_CMD                  _IOW(PLAT_BFGX_EXCP_CFG_IOC_MAGIC, DFR_HAL_BT, int)
#define DFR_HAL_FM_CFG_CMD                  _IOW(PLAT_BFGX_EXCP_CFG_IOC_MAGIC, DFR_HAL_FM, int)
#define PLAT_GT_EXCP_CFG_IOC_MAIC           'g'
#define PLAT_GT_DUMP_FILE_READ_CMD          _IOW(PLAT_GT_EXCP_CFG_IOC_MAIC, PLAT_GT_DUMP_FILE_READ, int)

STATIC int32_t hw_excp_read_mem(struct file *filp, char __user *buf,
                          size_t count, loff_t *f_pos, memdump_info_t *memdump_t)
{
    struct sk_buff *skb = NULL;
    uint16_t count1;

    if (unlikely((buf == NULL) || (filp == NULL))) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }
    if ((skb = skb_dequeue(&memdump_t->quenue)) == NULL) {
        return 0;
    }

    /* read min value from skb->len or count */
    count1 = min_t(size_t, skb->len, count);
    if (copy_to_user(buf, skb->data, count1)) {
        ps_print_err("copy_to_user is err!\n");
        skb_queue_head(&memdump_t->quenue, skb);
        return -EFAULT;
    }

    /* have read count1 byte */
    skb_pull(skb, count1);

    /* if skb->len = 0: read is over */
    if (skb->len == 0) { /* curr skb data have read to user */
        kfree_skb(skb);
    } else { /* if don,t read over; restore to skb queue */
        skb_queue_head(&memdump_t->quenue, skb);
    }

    return count1;
}

STATIC ssize_t hw_excp_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    struct bfgx_dev_node *dev_node = NULL;

    if (filp == NULL || filp->private_data == NULL) {
        return -EINVAL;
    }
    dev_node = oal_container_of((struct miscdevice *)filp->private_data,
                                struct bfgx_dev_node, c);
    if (dev_node->dev_data == NULL) {
        ps_print_err("hwexcep need memdump cfg\n");
        return -EINVAL;
    }
    return hw_excp_read_mem(filp, buf, count, f_pos, dev_node->dev_data);
}

STATIC int32_t plat_excp_dump_rotate_cmd_read(unsigned long arg, memdump_info_t *memdump_info)
{
    uint32_t OAL_USER *puser = (uint32_t OAL_USER *)(uintptr_t)arg;
    struct sk_buff *skb = NULL;

    if (!oal_access_write_ok(puser, (unsigned long)sizeof(uint32_t))) {
        ps_print_err("address can not write\n");
        return -EINVAL;
    }

    if (wait_event_interruptible(memdump_info->dump_type_wait, (skb_queue_len(&memdump_info->dump_type_queue)) > 0)) {
        return -EINVAL;
    }

    skb = skb_dequeue(&memdump_info->dump_type_queue);
    if (skb == NULL) {
        ps_print_warning("skb is NULL\n");
        return -EINVAL;
    }

    if (copy_to_user(puser, skb->data, sizeof(uint32_t))) {
        ps_print_warning("copy_to_user err!restore it, len=%d,arg=%ld\n", (int32_t)sizeof(uint32_t), arg);
        skb_queue_head(&memdump_info->dump_type_queue, skb);
        return -EINVAL;
    }

    ps_print_info("read rotate cmd [%d] from queue\n", *(uint32_t *)skb->data);

    skb_pull(skb, skb->len);
    kfree_skb(skb);

    return 0;
}

STATIC int32_t arm_timeout_submit(enum bfgx_thread_enum subs)
{
#define DFR_SUBMIT_LIMIT_TIME 300 /* second */
    static unsigned long long dfr_submit_last_time = 0;
    unsigned long long dfr_submit_current_time;
    unsigned long long dfr_submit_interval_time;

    if (subs >= BFGX_THREAD_BOTTOM) {
        return -EINVAL;
    }

    ps_print_info("[subs id:%d]arm timeout trigger", subs);

    dfr_submit_current_time = oal_current_kernel_time();
    dfr_submit_interval_time = dfr_submit_current_time - dfr_submit_last_time;

    /* 5分钟内最多触发一次 */
    if ((dfr_submit_interval_time > DFR_SUBMIT_LIMIT_TIME) || (dfr_submit_last_time == 0)) {
        dfr_submit_last_time = dfr_submit_current_time;
        plat_exception_handler(SUBSYS_BFGX, subs, BFGX_ARP_TIMEOUT);
        return 0;
    } else {
        ps_print_err("[subs id:%d]arm timeout cnt max than limit", subs);
        return -EAGAIN;
    }
}

STATIC long hw_excp_ioctl(struct file *filp, uint32_t cmd, unsigned long arg)
{
    int32_t ret = 0;

    struct bfgx_dev_node *dev_node = NULL;
    struct miscdevice *miscdev = NULL;

    if (filp == NULL || filp->private_data == NULL) {
        return -EINVAL;
    }

    miscdev = (struct miscdevice *)filp->private_data;
    dev_node = oal_container_of(miscdev, struct bfgx_dev_node, c);
    if (dev_node->dev_data == NULL) {
        ps_print_err("hwexcep[%s] need memdump cfg\n", miscdev->name);
        return -EINVAL;
    }

    switch (cmd) {
        case PLAT_BFGX_DUMP_FILE_READ_CMD:
        case PLAT_ME_DUMP_FILE_READ_CMD:
        case PLAT_WIFI_DUMP_FILE_READ_CMD:
        case PLAT_GT_DUMP_FILE_READ_CMD:
            ret = plat_excp_dump_rotate_cmd_read(arg, dev_node->dev_data);;
            break;
        case DFR_HAL_GNSS_CFG_CMD:
            ret = arm_timeout_submit(THREAD_GNSS);
            break;
        case DFR_HAL_BT_CFG_CMD:
            ret = arm_timeout_submit(THREAD_BT);
            break;
        case DFR_HAL_FM_CFG_CMD:
            ret = arm_timeout_submit(THREAD_FM);
            break;
        default:
            ps_print_warning("hw_debug_ioctl cmd = %d not find\n", cmd);
            ret = -EINVAL;
    }

    return ret;
}

STATIC const struct file_operations g_hw_excp_fops = {
    .owner = THIS_MODULE,
    .read = hw_excp_read,
    .unlocked_ioctl = hw_excp_ioctl,
};

STATIC struct bfgx_subsys_driver_desc g_default_excp_driver_desc = {
    .file_ops = &g_hw_excp_fops
};

struct bfgx_subsys_driver_desc *get_excp_default_drv_desc(void)
{
    return &g_default_excp_driver_desc;
}

MODULE_DESCRIPTION("Public serial Driver for huawei bfgx debug chips");
MODULE_LICENSE("GPL");
