/**
 * Copyright (c) @CompanyNameMagicTag 2022-2023. All rights reserved.
 *
 * Description: bfgx gnss functional module
 * Author: @CompanyNameTag
 */

/* Include Head file */
#include "bfgx_gnss.h"
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include "plat_debug.h"
#include "plat_pm.h"
#include "plat_uart.h"
#include "bfgx_dev.h"
#include "bfgx_core.h"
#include "plat_exception_rst.h"
#include "bfgx_data_parse.h"
#include "plat_common_clk.h"
#include "bfgx_excp_dbg.h"

// GNSS
#define GNSS_SET_READ_TIME     1

/* gnss update para */
#define PLAT_GNSS_MAGIC             'w'
#define PLAT_GNSS_DCXO_SET_PARA_CMD _IOW(PLAT_GNSS_MAGIC, 1, int)
#define PLAT_GNSS_ABB_CLK_PARA_CMD  _IOW(PLAT_GNSS_MAGIC, 2, int)
#define PLAT_GNSS_SLEEP_VOTE_CMD    _IOW(PLAT_GNSS_MAGIC, 4, int)
#define PLAT_GNSS_FRC_AWAKE_CMD     _IOW(PLAT_GNSS_MAGIC, 5, int)
#define PLAT_GNSS_GPIO_CMD          _IOW(PLAT_GNSS_MAGIC, 6, int)

/* gnss abb clk control cmd */
#define GNSS_ABB_CLK_ENABLE  1
#define GNSS_ABB_CLK_DISABLE 0

/* gnss sleep vote */
#define AGREE_SLEEP     1
#define NOT_AGREE_SLEEP 0

/*
* 平台处理逻辑：
* sleep_vote ==1 && frc_awake == 0  :  sleep
* sleep_vote == 1 && frc_awake == 1:   work
* sleep_vote == 0 && frc_awake == 1:   work
* sleep_vote == 0 && frc_awake == 0 :  5s后无GNSS数据收发，视为gnss业务异常，补投票， sleep
*/
STATIC int32_t gnss_set_frc_awake(struct pm_drv_data *pm_data, uint32_t frc)
{
    int32_t ret;
    int32_t prepare_result;
    ret = prepare_to_visit_node(pm_data->ps_core_data, &prepare_result);
    if (ret < 0) {
        return ret;
    }
    ps_print_info("[%s]gnss_frc_awake %d!\n", index2name(pm_data->index), frc);

    pm_data->gnss_frc_awake = (uint32_t)(frc & 0x1);

    post_to_visit_node(pm_data->ps_core_data, prepare_result);
    return ret;
}

STATIC void gnss_lowpower_vote(struct pm_drv_data *pm_data, uint32_t vote)
{
    if (oal_unlikely(pm_data == NULL)) {
        return;
    }
    ps_print_info("[%s]gnss_lowpower_vote %d!\n", index2name(pm_data->index), vote);
    if (vote == GNSS_AGREE_SLEEP) {
        if (pm_data->bfgx_lowpower_enable == BFGX_PM_DISABLE) {
            ps_print_warning("[%s]gnss low power disabled!\n", index2name(pm_data->index));
            return;
        }
        if (pm_data->bfgx_dev_state_get(pm_data) == BFGX_SLEEP) {
            ps_print_warning("[%s]gnss proc: dev has been sleep, not allow dev slp\n", index2name(pm_data->index));
            return;
        }

        /* set the flag to 1 means gnss request sleep */
        atomic_set(&pm_data->gnss_sleep_flag, GNSS_AGREE_SLEEP);

        /* if bt and fm are both shutdown ,we will pull down gpio directly */
        if (!timer_pending(&pm_data->bfg_timer) && (gnss_get_lowpower_state(pm_data) == GNSS_AGREE_SLEEP)) {
            ps_print_info("[%s]gnss_lowpower_vote sumit sleep work!\n", index2name(pm_data->index));

            if (queue_work(pm_data->wkup_dev_workqueue, &pm_data->send_allow_sleep_work) != true) {
                ps_print_info("[%s]queue_work send_allow_sleep_work not return true\n", index2name(pm_data->index));
            }
        }
    } else if (vote == GNSS_NOT_AGREE_SLEEP) {
        atomic_set(&pm_data->gnss_sleep_flag, GNSS_NOT_AGREE_SLEEP);
    } else {
        ps_print_err("PLAT_GNSS_SLEEP_VOTE_CMD invalid arg !\n");
    }
}

STATIC int32_t gnss_sepreted_skb_data_read(struct sk_buff_head *read_queue, struct ps_core_s *ps_core_d)
{
    struct sk_buff *skb = NULL;
    uint8_t seperate_tag;

    oal_spin_lock(&ps_core_d->bfgx_info[BFGX_GNSS].rx_lock);
    do {
        if ((skb = ps_skb_dequeue(ps_core_d, RX_GNSS_QUEUE)) == NULL) {
            oal_spin_unlock(&ps_core_d->bfgx_info[BFGX_GNSS].rx_lock);
            if (read_queue->qlen != 0) {
                // 没有找到last包，skb queue就空了
                ps_print_err("skb dequeue error, qlen=%x!\n", read_queue->qlen);
                return -EFAULT;
            } else {
                ps_print_info("gnss read no data!\n");
                return 0;
            }
        }

        seperate_tag = skb->data[skb->len - 1];
        if ((seperate_tag != GNSS_SEPER_TAG_INIT) && (seperate_tag != GNSS_SEPER_TAG_LAST)) {
            ps_print_err("seperate_tag=%x not support\n", seperate_tag);
            seperate_tag = GNSS_SEPER_TAG_LAST;
        }

        skb_queue_tail(read_queue, skb);
    } while (seperate_tag == GNSS_SEPER_TAG_INIT);
    oal_spin_unlock(&ps_core_d->bfgx_info[BFGX_GNSS].rx_lock);

    return 0;
}

STATIC int32_t gnss_read_data_to_user(struct sk_buff_head *read_queue, char __user *buf, size_t count)
{
    struct sk_buff *skb = NULL;
    int32_t data_len;
    int32_t copy_cnt;
    uint32_t ret;

    copy_cnt = 0;
    while (read_queue->qlen > 0) {
        skb = skb_dequeue(read_queue);
        if (unlikely(skb == NULL)) {
            ps_print_err("copy dequeue error, copy_cnt=%x\n", copy_cnt);
            return -EINVAL;
        }

        if (unlikely(skb->len <= 1)) {
            ps_print_err("skb len error,skb->len=%x,copy_cnt=%x,count=%x\n", skb->len, copy_cnt, (uint32_t)count);
            goto data_error;
        }

        data_len = (int32_t)(skb->len - 1);
        if (unlikely(data_len + copy_cnt > count)) {
            ps_print_err("copy total len error,skb->len=%x,tag=%x,copy_cnt=%x,read_cnt=%x\n",
                         skb->len, skb->data[skb->len - 1], copy_cnt, (uint32_t)count);
            goto data_error;
        }

        ret = copy_to_user(buf + copy_cnt, skb->data, data_len);
        if (unlikely(ret != 0)) {
            ps_print_err("copy_to_user err,ret=%x,dest=%p,src=%p,tag:%x,count1=%x,copy_cnt=%x,read_cnt=%x\n",
                         ret, buf + copy_cnt, skb->data, skb->data[skb->len - 1], data_len, copy_cnt, (uint32_t)count);
            goto data_error;
        }

        copy_cnt += data_len;
        kfree_skb(skb);
    }

    return copy_cnt;

data_error:
    kfree_skb(skb);
    return -EFAULT;
}

/*
 * Prototype    : hw_gnss_open
 * Description  : functions called from above gnss hal,when open gnss file
 * input        : "/dev/hwgnss"
 * output       : return 0 --> open is ok
 *                return !0--> open is false
 */
STATIC int32_t hw_gnss_open(struct inode *inode, struct file *filp)
{
    int32_t ret;
    struct pm_top* pm_top_data = pm_get_top();
    struct bfgx_dev_node *dev_node = NULL;
    struct ps_core_s *ps_core_d = NULL;

    if (oal_unlikely((filp == NULL) || (filp->private_data == NULL))) {
        ps_print_err("%s param is error", __func__);
        return -EINVAL;
    }

    dev_node = oal_container_of((struct miscdevice *)filp->private_data,
                                struct bfgx_dev_node, c);
    ps_core_d = (struct ps_core_s *)dev_node->core;
    if (oal_unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    mutex_lock(&(pm_top_data->host_mutex));
    if (oal_atomic_read(&dev_node->open_cnt) != 0) {
        mutex_unlock(&(pm_top_data->host_mutex));
        ps_print_err("hwgnss dev already opened\n");
        return -EBUSY;
    }
    oal_atomic_set(&dev_node->open_cnt, 1);
    oal_atomic_set(&ps_core_d->pm_data->gnss_sleep_flag, GNSS_NOT_AGREE_SLEEP);
    ret = hw_bfgx_open(ps_core_d, BFGX_GNSS);
    oal_atomic_set(&(ps_core_d->bfgx_info[BFGX_GNSS].read_delay), GNSS_READ_DEFAULT_TIME);

    if (ret != BFGX_POWER_SUCCESS) {
        oal_atomic_set(&ps_core_d->pm_data->gnss_sleep_flag, GNSS_AGREE_SLEEP);
        oal_atomic_set(&dev_node->open_cnt, 0);
    }

    mutex_unlock(&(pm_top_data->host_mutex));

    return ret;
}

/*
 * Prototype    : hw_gnss_poll
 * Description  : called by gnss func from hal;
 *                check whether or not allow read and write
 */
STATIC uint32_t hw_gnss_poll(struct file *filp, poll_table *wait)
{
    struct ps_core_s *ps_core_d = NULL;
    uint32_t mask = 0;

    ps_print_dbg("%s\n", __func__);

    ps_core_d = get_ps_core_from_miscdev(filp->private_data);
    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    /* push curr wait event to wait queue */
    poll_wait(filp, &ps_core_d->bfgx_info[BFGX_GNSS].rx_wait, wait);

    ps_print_dbg("%s, recive gnss data\n", __func__);

    if (ps_core_d->bfgx_info[BFGX_GNSS].rx_queue.qlen) { /* have data to read */
        mask |= POLLIN | POLLRDNORM;
    }

    return mask;
}

/*
 * Prototype    : hw_gnss_read
 * Description  : functions called from above gnss hal,read count data to buf
 */
STATIC ssize_t hw_gnss_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    struct sk_buff *skb = NULL;
    struct ps_core_s *ps_core_d = NULL;
    struct sk_buff_head read_queue;
    int32_t copy_cnt;

    ps_core_d = get_ps_core_from_miscdev(filp->private_data);
    if (unlikely(ps_core_d == NULL || buf == NULL)) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    skb_queue_head_init(&read_queue);

    copy_cnt = gnss_sepreted_skb_data_read(&read_queue, ps_core_d);
    if (copy_cnt < 0) {
        goto read_error;
    }

    copy_cnt = gnss_read_data_to_user(&read_queue, buf, count);
    if (copy_cnt < 0) {
        goto read_error;
    }

    return copy_cnt;

read_error:
    while ((skb = skb_dequeue(&read_queue)) != NULL) {
        ps_print_err("free skb: len=%x, tag=%x\n", skb->len, skb->data[skb->len - 1]);
        kfree_skb(skb);
    }

    return -EFAULT;
}

/*
 * Prototype    : hw_gnss_write
 * Description  : functions called from above gnss hal,write count data to buf
 */
STATIC ssize_t hw_gnss_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    struct ps_core_s *ps_core_d = NULL;
    int32_t ret = 0;
    struct pm_drv_data *pm_data = NULL;
    int32_t prepare_result;
    PS_PRINT_FUNCTION_NAME;

    ps_core_d = get_ps_core_from_miscdev(filp->private_data);
    if (oal_unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    pm_data = ps_core_d->pm_data;

    ret = hw_bfgx_write_check(buf, ps_core_d, BFGX_GNSS);
    if (ret < 0) {
        return ret;
    }

    if (count > GNSS_TX_MAX_FRAME) {
        ps_print_err("err:gnss packet is too large!\n");
        return -EINVAL;
    }

    /* if low queue num > MAX_NUM and don't write */
    if (oal_netbuf_list_len(&ps_core_d->tx_low_seq) > TX_LOW_QUE_MAX_NUM) {
        return 0;
    }

    atomic_set(&pm_data->gnss_sleep_flag, GNSS_NOT_AGREE_SLEEP);
    ret = prepare_to_visit_node(ps_core_d, &prepare_result);
    if (ret < 0) {
        atomic_set(&pm_data->gnss_sleep_flag, GNSS_AGREE_SLEEP);
        ps_print_err("prepare work fail, bring to reset work\n");
        plat_exception_handler(SUBSYS_BFGX, THREAD_GNSS, get_wakeup_excp_by_uart_idx(ps_core_d->pm_data->index));
        return ret;
    }

    oal_wake_lock_timeout(&pm_data->gnss_wake_lock, DEFAULT_WAKELOCK_TIMEOUT);

    /* to divide up packet function and tx to tty work */
    if (ps_h2d_tx_data(ps_core_d, BFGX_GNSS, buf, count) < 0) {
        ps_print_err("hw_gnss_write is err\n");
        atomic_set(&pm_data->gnss_sleep_flag, GNSS_AGREE_SLEEP);
        post_to_visit_node(ps_core_d, prepare_result);
        return -EFAULT;
    }

    ps_core_d->bfgx_info[BFGX_GNSS].tx_pkt_num++;

    post_to_visit_node(ps_core_d, prepare_result);

    return count;
}

/*
 * Prototype    : hw_gnss_ioctl
 * Description  : called by gnss func from hal when open power gpio or close power gpio
 */
STATIC long hw_gnss_ioctl(struct file *filp, uint32_t cmd, unsigned long arg)
{
    int32_t OAL_USER *puser;
    int32_t coeff_para[COEFF_NUM];
    struct ps_core_s *ps_core_d = NULL;

    PS_PRINT_FUNCTION_NAME;

    ps_core_d = get_ps_core_from_miscdev(filp->private_data);
    if (unlikely((ps_core_d == NULL))) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    switch (cmd) {
        case GNSS_SET_READ_TIME:
            if (arg < GNSS_MAX_READ_TIME) { /* set timeout for gnss read function */
                oal_atomic_set(&(ps_core_d->bfgx_info[BFGX_GNSS].read_delay), arg);
            } else {
                ps_print_err("arg is too large!\n");
                return -EINVAL;
            }
            break;
        case PLAT_GNSS_DCXO_SET_PARA_CMD:
            puser = (int32_t __user *)(uintptr_t)arg;
            if (copy_from_user(coeff_para, puser, COEFF_NUM * sizeof(int32_t))) {
                ps_print_err("[dcxo] get gnss update para error\n");
                return -EINVAL;
            }
            update_dcxo_coeff(coeff_para, COEFF_NUM * sizeof(int32_t));
            break;
        case PLAT_GNSS_SLEEP_VOTE_CMD:
            gnss_lowpower_vote(ps_core_d->pm_data, arg);
            break;
        case PLAT_GNSS_FRC_AWAKE_CMD:
            return gnss_set_frc_awake(ps_core_d->pm_data, arg);
        case PLAT_GNSS_GPIO_CMD:
            return conn_user_gpio_config((uint8_t __user *)(uintptr_t)arg);
        default:
            ps_print_warning("cmd = %d not find\n", cmd);
            return -EINVAL;
    }

    return 0;
}

/*
 * Prototype    : hw_gnss_release
 * Description  : called by gnss func from hal when close gnss inode
 */
STATIC int32_t hw_gnss_release(struct inode *inode, struct file *filp)
{
    int32_t ret;
    struct bfgx_dev_node *dev_node = NULL;
    struct pm_top* pm_top_data = pm_get_top();
    struct ps_core_s *ps_core_d = NULL;

    if (oal_unlikely(inode == NULL)) {
        ps_print_err("%s param is error", __func__);
        return -EINVAL;
    }

    if (oal_unlikely((filp == NULL) || (filp->private_data == NULL))) {
        ps_print_err("%s param is error", __func__);
        return -EINVAL;
    }
    dev_node = oal_container_of((struct miscdevice *)filp->private_data,
                                struct bfgx_dev_node, c);
    ps_core_d = (struct ps_core_s *)dev_node->core;
    if (ps_core_d == NULL) {
        ps_print_err("ps_core_d is NULL!\n");
        return -EINVAL;
    }

    mutex_lock(&(pm_top_data->host_mutex));
    ret = hw_bfgx_close(ps_core_d, BFGX_GNSS);
    oal_wake_unlock_force(&ps_core_d->pm_data->gnss_wake_lock);
    atomic_set(&ps_core_d->pm_data->gnss_sleep_flag, GNSS_AGREE_SLEEP);
    ps_core_d->pm_data->gnss_frc_awake = 0;
    oal_atomic_set(&dev_node->open_cnt, 0);
    mutex_unlock(&(pm_top_data->host_mutex));
    return ret;
}

STATIC const struct file_operations g_default_gnss_fops = {
    .owner = THIS_MODULE,
    .open = hw_gnss_open,
    .write = hw_gnss_write,
    .read = hw_gnss_read,
    .poll = hw_gnss_poll,
    .unlocked_ioctl = hw_gnss_ioctl,
    .release = hw_gnss_release,
};

#ifdef PLATFORM_DEBUG_ENABLE
STATIC ssize_t excp_dbg_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return bfgx_excp_dbg_show_comm(buf);
}

STATIC ssize_t excp_dbg_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    return bfgx_excp_dbg_store_comm(dev, buf, count);
}
STATIC DEVICE_ATTR_RW(excp_dbg);
#endif

// for directory /sys/devices/virtual/misc/hwgnss
STATIC struct attribute *g_hwgnss_attrs[] = {
#ifdef PLATFORM_DEBUG_ENABLE
    &dev_attr_excp_dbg.attr,
#endif
    NULL
};
ATTRIBUTE_GROUPS(g_hwgnss);

STATIC struct bfgx_subsys_driver_desc g_default_gnss_driver_desc = {
    .file_ops = &g_default_gnss_fops,
    .groups = g_hwgnss_groups
};

struct bfgx_subsys_driver_desc *get_gnss_default_drv_desc(void)
{
    return &g_default_gnss_driver_desc;
}

MODULE_DESCRIPTION("Public serial Driver for huawei GNSS chips");
MODULE_LICENSE("GPL");
