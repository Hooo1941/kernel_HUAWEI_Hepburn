/**
 * Copyright (c) @CompanyNameMagicTag 2022-2023. All rights reserved.
 *
 * Description: Create a BT system file node and register the driver
 * Author: @CompanyNameTag
 */

#include <linux/miscdevice.h>
#include <linux/reboot.h>
#include <linux/fs.h>

#include "plat_pm.h"
#include "bfgx_dev.h"
#include "bfgx_core.h"
#include "bfgx_data_parse.h"
#include "plat_exception_rst.h"
#include "hw_bfg_ps.h"
#include "plat_debug.h"
#include "bfgx_excp_dbg.h"
#include "bfgx_bt.h"
#ifdef _PRE_CONFIG_HISI_110X_BLUEZ
#include "bfgx_bluez.h"
#endif

#ifndef _PRE_CONFIG_HISI_110X_BLUEZ
struct bfgx_bt_private_data {
    struct bt_data_combination data_combination;
};

STATIC int32_t hw_bt_init(struct bfgx_dev_node *dev_node, struct ps_core_s *ps_core_d)
{
    int32_t ret;
    struct bfgx_bt_private_data *private_data = NULL;

    if (oal_atomic_read(&dev_node->open_cnt) != 0) {
        ps_print_err("hwbt dev already opened\n");
        if (ps_core_d->pm_data->bt_fake_close_flag == OAL_TRUE) {
            return OAL_SUCC;
        }
        return -EINVAL;
    }

    private_data = (struct bfgx_bt_private_data *)os_kzalloc_gfp(sizeof(struct bfgx_bt_private_data));
    if (private_data == NULL) {
        ps_print_err("BT private data memory alloc err\n");
        return -ENOMEM;
    }

    mutex_init(&private_data->data_combination.comb_mutex);
    dev_node->dev_data = private_data;
    oal_atomic_set(&dev_node->open_cnt, 1);

    ret = hw_bfgx_open(ps_core_d, BFGX_BT);
    if (ret != 0) {
        oal_atomic_set(&dev_node->open_cnt, 0);
        mutex_destroy(&private_data->data_combination.comb_mutex);
        os_mem_kfree(private_data);
    }
    return ret;
}

/*
 * Prototype    : hw_bt_open
 * Description  : functions called from above bt hal,when open bt file
 * input        : "/dev/hwbt"
 * output       : return 0 --> open is ok
 *              : return !0--> open is false
 */
STATIC int32_t hw_bt_open(struct inode *inode, struct file *filp)
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
    ret = hw_bt_init(dev_node, ps_core_d);
    mutex_unlock(&(pm_top_data->host_mutex));

    return ret;
}

/*
 * Prototype    : hw_bt_read
 * Description  : functions called from above bt hal,read count data to buf
 * input        : file handle, buf, count
 * output       : return size --> actual read byte size
 */
STATIC ssize_t hw_bt_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    struct ps_core_s *ps_core_d = NULL;
    struct sk_buff *skb = NULL;

    uint16_t count1;

    ps_core_d = (struct ps_core_s *)get_ps_core_from_miscdev(filp->private_data);
    if (ps_core_d == NULL || buf == NULL) {
        ps_print_err("ps core & buf is null\n");
        return -EINVAL;
    }

    if ((skb = ps_skb_dequeue(ps_core_d, RX_BT_QUEUE)) == NULL) {
        ps_print_warning("bt read skb queue is null!\n");
        return 0;
    }

    /* read min value from skb->len or count */
    count1 = min_t(size_t, skb->len, count);
    if (copy_to_user(buf, skb->data, count1)) {
        ps_print_err("copy_to_user is err!\n");
        ps_restore_skbqueue(ps_core_d, skb, RX_BT_QUEUE);
        return -EFAULT;
    }

    /* have read count1 byte */
    skb_pull(skb, count1);

    /* if skb->len = 0: read is over */
    if (skb->len == 0) { /* curr skb data have read to user */
        kfree_skb(skb);
    } else { /* if don,t read over; restore to skb queue */
        ps_restore_skbqueue(ps_core_d, skb, RX_BT_QUEUE);
    }

    return count1;
}

STATIC ssize_t priv_bt_data_send(const char __user *buf, size_t count,
                                 struct ps_core_s *ps_core_d, struct bt_data_combination *data_comb)
{
    uint16_t total_len = count + data_comb->len + sizeof(struct ps_packet_head) + sizeof(struct ps_packet_end);

    struct sk_buff *skb = ps_alloc_skb(total_len);
    if (skb == NULL) {
        ps_print_err("ps alloc skb mem fail\n");
        return -EFAULT;
    }

    if (copy_from_user(&skb->data[sizeof(struct ps_packet_head) + data_comb->len], buf, count)) {
        ps_print_err("copy_from_user from bt is err\n");
        kfree_skb(skb);
        return -EFAULT;
    }

    if (data_comb->len == BT_TYPE_DATA_LEN) {
        skb->data[sizeof(struct ps_packet_head)] = data_comb->type;
    }

    ps_add_packet_head(skb->data, BT_MSG, total_len);
    ps_skb_enqueue(ps_core_d, skb, TX_HIGH_QUEUE);
    ps_core_tx_work_add(ps_core_d);

    ps_core_d->bfgx_info[BFGX_BT].tx_pkt_num++;

    return count;
}

STATIC ssize_t priv_bt_write(struct file *filp, const char __user *buf, size_t count,
                             struct bt_data_combination *data_comb)
{
    ssize_t ret = 0;
    int32_t prepare_result;
    struct ps_core_s *ps_core_d = NULL;

    ps_core_d = (struct ps_core_s *)get_ps_core_from_miscdev(filp->private_data);

    if (hw_bfgx_write_check(buf, ps_core_d, BFGX_BT) < 0) {
        return -EINVAL;
    }

    if (count > BT_TX_MAX_FRAME) {
        ps_print_err("bt skb len is too large!\n");
        return -EINVAL;
    }

    /* if high queue num > MAX_NUM and don't write */
    if (oal_netbuf_list_len(&ps_core_d->tx_high_seq) > TX_HIGH_QUE_MAX_NUM) {
        ps_print_err("bt tx high seqlen large than MAXNUM\n");
        return 0;
    }

    ret = prepare_to_visit_node(ps_core_d, &prepare_result);
    if (ret < 0) {
        ps_print_err("prepare work fail, bring to reset work\n");
        plat_exception_handler(SUBSYS_BFGX, THREAD_BT, get_wakeup_excp_by_uart_idx(ps_core_d->pm_data->index));
        return ret;
    }

    oal_wake_lock_timeout(&ps_core_d->pm_data->bt_wake_lock, DEFAULT_WAKELOCK_TIMEOUT);

    /* modify expire time of uart idle timer */
    oal_atomic_inc(&ps_core_d->pm_data->bfg_timer_mod_cnt);
    mod_timer(&ps_core_d->pm_data->bfg_timer, jiffies + msecs_to_jiffies(BT_SLEEP_TIME));

    ret = priv_bt_data_send(buf, count, ps_core_d, data_comb);

    post_to_visit_node(ps_core_d, prepare_result);

    return ret;
}

/*
 * Prototype    : hw_bt_write
 * Description  : functions called from above bt hal,write count data to buf
 * input        : file handle, buf, count
 * output       : return size --> actual write byte size
 */
STATIC ssize_t hw_bt_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    ssize_t ret = 0;
    uint8_t type = 0;
    uint8_t __user *puser = (uint8_t __user *)buf;
    struct bfgx_dev_node *dev_node = NULL;
    struct bfgx_bt_private_data *private_data = NULL;
    struct bt_data_combination *data_combination = NULL;

    dev_node = oal_container_of((struct miscdevice *)filp->private_data,
                                struct bfgx_dev_node, c);
    private_data = dev_node->dev_data;
    data_combination = &private_data->data_combination;
    mutex_lock(&data_combination->comb_mutex);

    /* 数据流不允许连续下发两次type */
    if ((data_combination->len == BT_TYPE_DATA_LEN) && (count == BT_TYPE_DATA_LEN)) {
        data_combination->len = 0;
        mutex_unlock(&private_data->data_combination.comb_mutex);
        ps_print_err("two consecutive type write!\n");
        return -EFAULT;
    }

    /* BT数据分两次下发，先发数据类型，长度固定为1Byte，然后发数据，需要在驱动中组合起来发给device */
    if ((count == BT_TYPE_DATA_LEN) && (puser != NULL)) {
        /* 暂存type, 待与下一包数据一起组合发送 */
        get_user(type, puser);
        data_combination->type = type;
        data_combination->len = count;
        mutex_unlock(&private_data->data_combination.comb_mutex);
        return count;
    }

    /* 组合发送 */
    ret = priv_bt_write(filp, buf, count, data_combination);

    /* 设置为初始值 */
    data_combination->len = 0;

    mutex_unlock(&private_data->data_combination.comb_mutex);
    return ret;
}

/*
 * Prototype    : hw_bt_poll
 * Description  : called by bt func from hal;
 *                check whether or not allow read and write
 */
STATIC uint32_t hw_bt_poll(struct file *filp, poll_table *wait)
{
    uint32_t mask = 0;
    struct ps_core_s *ps_core_d = NULL;

    PS_PRINT_FUNCTION_NAME;
    ps_core_d = (struct ps_core_s *)get_ps_core_from_miscdev(filp->private_data);
    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    /* push curr wait event to wait queue */
    poll_wait(filp, &ps_core_d->bfgx_info[BFGX_BT].rx_wait, wait);

    if (ps_core_d->bfgx_info[BFGX_BT].rx_queue.qlen) { /* have data to read */
        mask |= POLLIN | POLLRDNORM;
    }

    return mask;
}

/*
 * Prototype    : hw_bt_ioctl
 * Description  : called by bt func from hal; default not use
 */
STATIC long hw_bt_ioctl(struct file *filp, uint32_t cmd, unsigned long arg)
{
    struct ps_core_s *ps_core_d = NULL;
    struct pm_top* pm_top_data = pm_get_top();

    ps_print_info("cmd is %d,arg is %lu", cmd, arg);
    ps_core_d = (struct ps_core_s *)get_ps_core_from_miscdev(filp->private_data);
    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }
    mutex_lock(&(pm_top_data->host_mutex));

    /* hal下发cmd,控制不实际关闭close bt */
    if (cmd == BT_IOCTL_FAKE_CLOSE_CMD) {
        if (arg == BT_FAKE_CLOSE) {
            ps_core_d->pm_data->bt_fake_close_flag = OAL_TRUE;
        } else {
            ps_core_d->pm_data->bt_fake_close_flag = OAL_FALSE;
        }
    }

    mutex_unlock(&(pm_top_data->host_mutex));

    return 0;
}

STATIC int32_t hw_bt_fake_close(struct ps_core_s *ps_core_d)
{
    int32_t ret;
    int32_t prepare_result;

    ret = prepare_to_visit_node(ps_core_d, &prepare_result);
    if (ret < 0) {
        /* 唤醒失败DFR恢复 */
        ps_print_err("wakeup device FAIL\n");
        plat_exception_handler(SUBSYS_BFGX, THREAD_BT, get_wakeup_excp_by_uart_idx(ps_core_d->pm_data->index));
        return ret;
    }

    ps_tx_sys_cmd(ps_core_d, SYS_MSG, PL_FAKE_CLOSE_BT_EVT);

    /* modify expire time of uart idle timer */
    mod_timer(&ps_core_d->pm_data->bfg_timer, jiffies + msecs_to_jiffies(BT_SLEEP_TIME));
    oal_atomic_inc(&ps_core_d->pm_data->bfg_timer_mod_cnt);

    post_to_visit_node(ps_core_d, prepare_result);
    return 0;
}

/*
 * Prototype    : hw_bt_release
 * Description  : called by bt func from hal when close bt inode
 * input        : "/dev/hwbt"
 * output       : return 0 --> close is ok
 *                return !0--> close is false
 */
STATIC int32_t hw_bt_release(struct inode *inode, struct file *filp)
{
    int32_t ret;
    struct ps_core_s *ps_core_d = NULL;
    struct bfgx_dev_node *dev_node = NULL;
    struct bfgx_bt_private_data *private_data = NULL;
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

    /* hal下发cmd,控制不实际关闭close bt */
    if (ps_core_d->pm_data->bt_fake_close_flag == OAL_TRUE) {
        hw_bt_fake_close(ps_core_d);
        mutex_unlock(&(pm_top_data->host_mutex));
        ps_print_info("bt_fake_close_flag is %u ,not really close bt!",
                      ps_core_d->pm_data->bt_fake_close_flag);
        return BFGX_POWER_SUCCESS;
    }

    ret = hw_bfgx_close(ps_core_d, BFGX_BT);
    oal_wake_unlock_force(&ps_core_d->pm_data->bt_wake_lock);
    oal_atomic_set(&dev_node->open_cnt, 0);

    private_data = dev_node->dev_data;
    mutex_destroy(&private_data->data_combination.comb_mutex);
    os_mem_kfree(dev_node->dev_data);
    dev_node->dev_data = NULL;
    mutex_unlock(&(pm_top_data->host_mutex));

    return ret;
}
#endif

STATIC const struct file_operations g_default_bt_fops = {
    .owner = THIS_MODULE,
    .open = hw_bt_open,
    .write = hw_bt_write,
    .read = hw_bt_read,
    .poll = hw_bt_poll,
    .unlocked_ioctl = hw_bt_ioctl,
    .release = hw_bt_release,
};

#ifdef PLATFORM_DEBUG_ENABLE
STATIC ssize_t uart_switch_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct miscdevice *miscdev = dev_get_drvdata(dev);
    struct ps_core_s *ps_core_d = NULL;
    struct ps_plat_s *ps_plat_d = NULL;

    ps_core_d = get_ps_core_from_miscdev(miscdev);
    ps_plat_d = (struct ps_plat_s *)ps_core_d->ps_plat;

    ps_print_info("core name is %s\n", ps_plat_d->uart_name);
    return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "dev[%s] use uart %s\n",
                      dev_name(miscdev->this_device), ps_plat_d->uart_name);
}

STATIC ssize_t uart_switch_store(struct device *dev, struct device_attribute *attr,
                                 const char *buf, size_t count)
{
    struct miscdevice *miscdev = dev_get_drvdata(dev);
    struct bfgx_dev_node *dev_node = NULL;
    struct ps_core_s *ps_core_d = NULL;
    struct ps_plat_s *ps_plat_d = NULL;
    char subsys_name[SUBSYS_NAME_SIZE] = {0};

    ps_core_d = get_ps_core_from_miscdev(miscdev);
    ps_plat_d = (struct ps_plat_s *)ps_core_d->ps_plat;

    if (ps_core_chk_bfgx_active(ps_core_d) == true) {
        ps_print_err("must close current subsys %s first\n", dev_name(miscdev->this_device));
        return -EINVAL;
    }

    if (sscanf_s(buf, "%s", subsys_name, SUBSYS_NAME_SIZE - 1) != 1) {
        ps_print_err("input subsys name error\n");
        return -EINVAL;
    }

    ps_core_d = bfgx_get_core_by_dev_name(subsys_name);
    if (ps_core_d == NULL) {
        ps_print_err("not support current subys %s\n", subsys_name);
        return -EINVAL;
    }

    if (ps_core_chk_bfgx_active(ps_core_d) == true) {
        ps_print_err("must close switch-to subsys %s\n", subsys_name);
        return -EINVAL;
    }

    ps_plat_d = (struct ps_plat_s *)ps_core_d->ps_plat;
    dev_node = oal_container_of(miscdev, struct bfgx_dev_node, c);
    dev_node->core = ps_core_d;

    ps_print_info("dev %s uart set to %s\n", dev_name(miscdev->this_device), ps_plat_d->uart_name);

    return count;
}

STATIC ssize_t excp_dbg_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return bfgx_excp_dbg_show_comm(buf);
}

STATIC ssize_t excp_dbg_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    return bfgx_excp_dbg_store_comm(dev, buf, count);
}

STATIC DEVICE_ATTR_RW(uart_switch);
STATIC DEVICE_ATTR_RW(excp_dbg);
#endif

// for directory /sys/devices/virtual/misc/hwbt
STATIC struct attribute *g_hwbt_attrs[] = {
#ifdef PLATFORM_DEBUG_ENABLE
    &dev_attr_uart_switch.attr,
    &dev_attr_excp_dbg.attr,
#endif
    NULL
};
ATTRIBUTE_GROUPS(g_hwbt);

STATIC struct bfgx_subsys_driver_desc g_default_bt_driver_desc = {
    .file_ops = &g_default_bt_fops,
    .groups = g_hwbt_groups
};

struct bfgx_subsys_driver_desc *get_bt_default_drv_desc(void)
{
    return &g_default_bt_driver_desc;
}

MODULE_DESCRIPTION("Public serial Driver for huawei hwbt chips");
MODULE_LICENSE("GPL");
