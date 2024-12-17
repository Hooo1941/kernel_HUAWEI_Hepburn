/**
 * Copyright (c) @CompanyNameMagicTag 2022-2023. All rights reserved.
 *
 * Description: Create an IR system file node and register the driver
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

/*
 * Prototype    : hw_ir_open
 * Description  : open ir device
 */
STATIC int32_t hw_ir_open(struct inode *inode, struct file *filp)
{
    int32_t ret;
    struct pm_top* pm_top_data = pm_get_top();
    struct ps_core_s *ps_core_d = NULL;

    if (unlikely(inode == NULL)) {
        ps_print_err("%s param is error", __func__);
        return -EINVAL;
    }

    ps_core_d = get_ps_core_from_miscdev(filp->private_data);
    if (ps_core_d == NULL) {
        ps_print_err("%s ps core is null", __func__);
        return -EINVAL;
    }

    mutex_lock(&(pm_top_data->host_mutex));
    ret = hw_bfgx_open(ps_core_d, BFGX_IR);
    mutex_unlock(&(pm_top_data->host_mutex));

    return ret;
}

/*
 * Prototype    : hw_ir_read
 * Description  : read ir node data
 */
STATIC ssize_t hw_ir_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    uint16_t ret_count;
    struct sk_buff *skb = NULL;
    struct ps_core_s *ps_core_d = NULL;

    PS_PRINT_FUNCTION_NAME;
    if (unlikely(buf == NULL)) {
        ps_print_err("buf is NULL\n");
        return -EINVAL;
    }

    ps_core_d = get_ps_core_from_miscdev(filp->private_data);
    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    if ((skb = ps_skb_dequeue(ps_core_d, RX_IR_QUEUE)) == NULL) {
        ps_print_dbg("ir read skb queue is null!\n");
        return 0;
    }

    ret_count = min_t(size_t, skb->len, count);
    if (copy_to_user(buf, skb->data, ret_count)) {
        ps_print_err("copy_to_user is err!\n");
        ps_restore_skbqueue(ps_core_d, skb, RX_IR_QUEUE);
        return -EFAULT;
    }

    skb_pull(skb, ret_count);

    if (skb->len == 0) {
        kfree_skb(skb);
    } else {
        ps_restore_skbqueue(ps_core_d, skb, RX_IR_QUEUE);
    }

    return ret_count;
}

/*
 * Prototype    : hw_ir_write
 * Description  : write data to ir node
 */
STATIC ssize_t hw_ir_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    struct ps_core_s *ps_core_d = NULL;
    int32_t ret = 0;
    int32_t prepare_result;

    PS_PRINT_FUNCTION_NAME;

    ps_core_d = get_ps_core_from_miscdev(filp->private_data);
    ret = hw_bfgx_write_check(buf, ps_core_d, BFGX_IR);
    if (ret < 0) {
        return ret;
    }

    if (count > IR_TX_MAX_FRAME) {
        ps_print_err("IR skb len is too large!\n");
        return -EINVAL;
    }

    if (oal_netbuf_list_len(&ps_core_d->tx_low_seq) > TX_LOW_QUE_MAX_NUM) {
        return 0;
    }

    ret = prepare_to_visit_node(ps_core_d, &prepare_result);
    if (ret < 0) {
        ps_print_err("prepare work fail, bring to reset work\n");
        plat_exception_handler(SUBSYS_BFGX, THREAD_IR, get_wakeup_excp_by_uart_idx(ps_core_d->pm_data->index));
        return ret;
    }

    if (oal_atomic_read(&ps_core_d->ir_only) == 0) {
        /* modify expire time of uart idle timer */
        mod_timer(&ps_core_d->pm_data->bfg_timer, jiffies + msecs_to_jiffies(BT_SLEEP_TIME));
        oal_atomic_inc(&ps_core_d->pm_data->bfg_timer_mod_cnt);
    }

    /* to divide up packet function and tx to tty work */
    if (ps_h2d_tx_data(ps_core_d, BFGX_IR, buf, count) < 0) {
        ps_print_err("hw_ir_write is err\n");
        post_to_visit_node(ps_core_d, prepare_result);
        return -EFAULT;
    }

    ps_core_d->bfgx_info[BFGX_IR].tx_pkt_num++;

    post_to_visit_node(ps_core_d, prepare_result);

    ps_print_info("no.[%d] send data %d Byte\n", (uint32_t)ps_core_d->bfgx_info[BFGX_IR].tx_pkt_num, (uint32_t)count);

    return count;
}

STATIC int32_t hw_ir_release(struct inode *inode, struct file *filp)
{
    int32_t ret;
    struct ps_core_s *ps_core_d = NULL;
    struct pm_top* pm_top_data = pm_get_top();

    ps_core_d = get_ps_core_from_miscdev(filp->private_data);
    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    mutex_lock(&(pm_top_data->host_mutex));
    ret = hw_bfgx_close(ps_core_d, BFGX_IR);
    mutex_unlock(&(pm_top_data->host_mutex));

    return ret;
}

STATIC const struct file_operations g_default_ir_fops = {
    .owner = THIS_MODULE,
    .open = hw_ir_open,
    .write = hw_ir_write,
    .read = hw_ir_read,
    .release = hw_ir_release,
};

STATIC struct bfgx_subsys_driver_desc g_default_ir_driver_desc = {
    .file_ops = &g_default_ir_fops
};

struct bfgx_subsys_driver_desc *get_ir_default_drv_desc(void)
{
    return &g_default_ir_driver_desc;
}

MODULE_DESCRIPTION("Public serial Driver for huawei ir chips");
MODULE_LICENSE("GPL");
