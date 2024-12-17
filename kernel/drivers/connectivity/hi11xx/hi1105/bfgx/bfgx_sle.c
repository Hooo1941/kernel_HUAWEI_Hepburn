/**
 * Copyright (c) @CompanyNameMagicTag 2022-2023. All rights reserved.
 *
 * Description: bfgx sle functional module
 * Author: @CompanyNameTag
 */

/* Include Head file */
#include "bfgx_sle.h"
#include <linux/platform_device.h>
#include "plat_debug.h"
#include "plat_pm.h"
#include "plat_uart.h"
#include "bfgx_dev.h"
#include "bfgx_core.h"
#include "plat_exception_rst.h"
#include "bfgx_data_parse.h"
#include "plat_common_clk.h"
#include "bfgx_excp_dbg.h"

struct bfgx_sle_private_data {
    struct bt_data_combination data_combination;
};

STATIC struct platform_device *g_hw_ps_sle_device = NULL;
struct platform_device *get_sle_platform_device(void)
{
    return g_hw_ps_sle_device;
}

STATIC int32_t hw_sle_init(struct bfgx_dev_node *dev_node, struct ps_core_s *ps_core_d)
{
    int32_t ret;
    struct bfgx_sle_private_data *priv_data = NULL;
    if (oal_atomic_read(&dev_node->open_cnt) != 0) {
        ps_print_err("hwsle dev already opened\n");
        if (ps_core_d->pm_data->bt_fake_close_flag == OAL_TRUE) {
            return OAL_SUCC;
        }
        return -EINVAL;
    }

    priv_data = (struct bfgx_sle_private_data *)os_kzalloc_gfp(sizeof(struct bfgx_sle_private_data));
    if (priv_data == NULL) {
        ps_print_err("SLE private data memory alloc err\n");
        return -ENOMEM;
    }

    mutex_init(&priv_data->data_combination.comb_mutex);
    dev_node->dev_data = priv_data;
    oal_atomic_set(&dev_node->open_cnt, 1);

    ret = hw_bfgx_open(ps_core_d, BFGX_SLE);
    if (ret != 0) {
        oal_atomic_set(&dev_node->open_cnt, 0);
        mutex_destroy(&priv_data->data_combination.comb_mutex);
        os_mem_kfree(priv_data);
    }
    return ret;
}

STATIC int32_t hw_sle_open(struct inode *inode, struct file *filp)
{
    int32_t ret;
    struct bfgx_dev_node *dev_node = NULL;
    struct ps_core_s *ps_core_d = NULL;
    struct pm_top* pm_top_data = pm_get_top();

    if (unlikely((filp == NULL) || (filp->private_data == NULL))) {
        ps_print_err("%s param is error", __func__);
        return -EINVAL;
    }

    dev_node = oal_container_of((struct miscdevice *)filp->private_data,
                                struct bfgx_dev_node, c);
    ps_core_d = (struct ps_core_s *)dev_node->core;
    if (ps_core_d == NULL) {
        ps_print_err("%s ps core is null", __func__);
        return -EINVAL;
    }

    mutex_lock(&(pm_top_data->host_mutex));
    ret = hw_sle_init(dev_node, ps_core_d);
    mutex_unlock(&(pm_top_data->host_mutex));

    return ret;
}

STATIC uint32_t hw_sle_poll(struct file *filp, poll_table *wait)
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
    poll_wait(filp, &ps_core_d->bfgx_info[BFGX_SLE].rx_wait, wait);

    ps_print_dbg("%s, recive gnss me data\n", __func__);

    if (ps_core_d->bfgx_info[BFGX_SLE].rx_queue.qlen) { /* have data to read */
        mask |= POLLIN | POLLRDNORM;
    }

    return mask;
}

STATIC ssize_t hw_sle_read(struct file *filp, char __user *buf,
                           size_t count, loff_t *f_pos)
{
    struct ps_core_s *ps_core_d = NULL;
    struct sk_buff *skb = NULL;
    uint16_t count1;

    ps_core_d = get_ps_core_from_miscdev(filp->private_data);
    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    if ((skb = ps_skb_dequeue(ps_core_d, RX_SLE_QUEUE)) == NULL) {
        return 0;
    }

    /* read min value from skb->len or count */
    count1 = min_t(size_t, skb->len, count);
    if (copy_to_user(buf, skb->data, count1)) {
        ps_print_err("copy_to_user is err!\n");
        ps_restore_skbqueue(ps_core_d, skb, RX_SLE_QUEUE);
        return -EFAULT;
    }

    /* have read count1 byte */
    skb_pull(skb, count1);

    /* if skb->len = 0: read is over */
    if (skb->len == 0) { /* curr skb data have read to user */
        kfree_skb(skb);
    } else { /* if don,t read over; restore to skb queue */
        ps_restore_skbqueue(ps_core_d, skb, RX_SLE_QUEUE);
    }

    return count1;
}

STATIC int32_t hw_sle_data_send(struct bt_data_combination *data_combination,
                                       struct ps_core_s *ps_core_d,
                                       const char __user *buf,
                                       size_t count)
{
    struct sk_buff *skb = NULL;
    uint16_t total_len;

    total_len = count + data_combination->len + sizeof(struct ps_packet_head) +
                sizeof(struct ps_packet_end);

    skb = ps_alloc_skb(total_len);
    if (skb == NULL) {
        ps_print_err("ps alloc skb mem fail\n");
        return -EFAULT;
    }

    if (copy_from_user(&skb->data[sizeof(struct ps_packet_head) +
                       data_combination->len], buf, count)) {
        ps_print_err("copy_from_user from bt is err\n");
        kfree_skb(skb);
        return -EFAULT;
    }

    if (data_combination->len == 1) {
        skb->data[sizeof(struct ps_packet_head)] = data_combination->type;
    }

    ps_add_packet_head(skb->data, SLE_MSG, total_len);
    ps_skb_enqueue(ps_core_d, skb, TX_HIGH_QUEUE);
    ps_core_tx_work_add(ps_core_d);

    ps_core_d->bfgx_info[BFGX_SLE].tx_pkt_num++;

    return count;
}

STATIC ssize_t priv_sle_write(struct file *filp, const char __user *buf, size_t count,
                              struct bt_data_combination *data_comb)
{
    struct ps_core_s *ps_core_d = NULL;
    ssize_t ret = 0;
    int32_t prepare_result;

    ps_core_d = get_ps_core_from_miscdev(filp->private_data);
    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    if (hw_bfgx_write_check(buf, ps_core_d, BFGX_SLE) < 0) {
        return -EINVAL;
    }

    if (count > SLE_TX_MAX_FRAME) {
        ps_print_err("bt skb len is too large!\n");
        return -EINVAL;
    }

    /* if high queue num > MAX_NUM and don't write */
    if (oal_netbuf_list_len(&ps_core_d->tx_high_seq) > TX_HIGH_QUE_MAX_NUM) {
        ps_print_err("sle tx high seqlen large than MAXNUM\n");
        return 0;
    }

    ret = prepare_to_visit_node(ps_core_d, &prepare_result);
    if (ret < 0) {
        ps_print_err("prepare work fail, bring to reset work\n");
        plat_exception_handler(SUBSYS_BFGX, THREAD_SLE, get_wakeup_excp_by_uart_idx(ps_core_d->pm_data->index));
        return ret;
    }

    oal_wake_lock_timeout(&ps_core_d->pm_data->bt_wake_lock, DEFAULT_WAKELOCK_TIMEOUT);

    /* modify expire time of uart idle timer */
    oal_atomic_inc(&ps_core_d->pm_data->bfg_timer_mod_cnt);
    mod_timer(&ps_core_d->pm_data->bfg_timer, jiffies + msecs_to_jiffies(SLE_SLEEP_TIME));

    ret = hw_sle_data_send(data_comb, ps_core_d, buf, count);

    post_to_visit_node(ps_core_d, prepare_result);
    return ret;
}

STATIC ssize_t hw_sle_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    ssize_t ret = 0;
    uint8_t type = 0;
    uint8_t __user *puser = (uint8_t __user *)buf;
    struct bfgx_dev_node *dev_node = NULL;
    struct bfgx_sle_private_data *priv_data = NULL;
    struct bt_data_combination *data_comb = NULL;

    dev_node = oal_container_of((struct miscdevice *)filp->private_data,
                                struct bfgx_dev_node, c);
    priv_data = dev_node->dev_data;
    data_comb = &priv_data->data_combination;
    mutex_lock(&data_comb->comb_mutex);

    /* 数据流不允许连续下发两次type */
    if ((data_comb->len == BT_TYPE_DATA_LEN) && (count == BT_TYPE_DATA_LEN)) {
        data_comb->len = 0;
        mutex_unlock(&(data_comb->comb_mutex));
        ps_print_err("two consecutive type write!\n");
        return -EFAULT;
    }

    /* BT数据分两次下发，先发数据类型，长度固定为1Byte，然后发数据，需要在驱动中组合起来发给device */
    if ((count == BT_TYPE_DATA_LEN) && (puser != NULL)) {
        get_user(type, puser);
        data_comb->type = type;
        data_comb->len = count;
        mutex_unlock(&(data_comb->comb_mutex));
        return count;
    }

    ret = priv_sle_write(filp, buf, count, data_comb);

    data_comb->len = 0;
    mutex_unlock(&(data_comb->comb_mutex));
    return ret;
}

STATIC int32_t hw_sle_release(struct inode *inode, struct file *filp)
{
    int32_t ret;
    struct ps_core_s *ps_core_d = NULL;
    struct bfgx_dev_node *dev_node = NULL;
    struct bfgx_sle_private_data *priv_data = NULL;
    struct pm_top* pm_top_data = pm_get_top();
    struct bt_data_combination *data_comb = NULL;

    if (unlikely((filp == NULL) || (filp->private_data == NULL))) {
        ps_print_err("%s param is error", __func__);
        return -EINVAL;
    }

    dev_node = oal_container_of((struct miscdevice *)filp->private_data,
                                struct bfgx_dev_node, c);
    priv_data = dev_node->dev_data;
    data_comb = &priv_data->data_combination;

    ps_core_d = (struct ps_core_s *)dev_node->core;
    if (ps_core_d == NULL) {
        ps_print_err("%s ps core is null", __func__);
        return -EINVAL;
    }

    mutex_lock(&(pm_top_data->host_mutex));
    ret = hw_bfgx_close(ps_core_d, BFGX_SLE);
    oal_atomic_set(&dev_node->open_cnt, 0);
    mutex_destroy(&data_comb->comb_mutex);
    os_mem_kfree(dev_node->dev_data);
    mutex_unlock(&(pm_top_data->host_mutex));

    return ret;
}

STATIC const struct file_operations g_default_sle_fops = {
    .owner = THIS_MODULE,
    .open = hw_sle_open,
    .write = hw_sle_write,
    .read = hw_sle_read,
    .poll = hw_sle_poll,
    .unlocked_ioctl = NULL,
    .release = hw_sle_release,
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

// for directory /sys/devices/virtual/misc/hwsle
STATIC struct attribute *g_hwsle_attrs[] = {
#ifdef PLATFORM_DEBUG_ENABLE
    &dev_attr_excp_dbg.attr,
#endif
    NULL
};
ATTRIBUTE_GROUPS(g_hwsle);

STATIC struct bfgx_subsys_driver_desc g_default_sle_driver_desc = {
    .file_ops = &g_default_sle_fops,
    .groups = g_hwsle_groups
};

struct bfgx_subsys_driver_desc *get_sle_default_drv_desc(void)
{
    return &g_default_sle_driver_desc;
}

MODULE_DESCRIPTION("Public serial Driver for huawei sle chips");
MODULE_LICENSE("GPL");
