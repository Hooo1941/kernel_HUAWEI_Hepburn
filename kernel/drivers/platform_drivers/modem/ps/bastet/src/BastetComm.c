/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "product_config.h"
#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <asm/uaccess.h>
#include <linux/ioctl.h>
#include <linux/poll.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
#include <linux/uio.h>
#include <uapi/linux/uio.h>
#endif
#include <linux/etherdevice.h>
#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#endif
#include "adrv_sensorhub.h"
#include "bastet_comm.h"
#include "securec.h"
#include "mdrv_vcom_agent.h"
#include "mdrv_msg.h"
#include "msg_id.h"
#include "bastet_rnic_interface.h"
#include "bastet_private.h"


#define THIS_FILE_ID PS_FILE_ID_BST_COMM_C

dev_t g_bastet_modem_dev;
struct cdev g_bastet_modem_cdev;
struct class *bastet_modem_class;
struct msg_chn_hdl *g_bastet_msglite_chnhdl = NULL;

static struct bastet_modem_driver_data g_bastet_modem_data;

bool g_bastet_modem_dev_en;

static int bastet_modem_open(struct inode *inode, struct file *filp)
{
    spin_lock_bh(&g_bastet_modem_data.read_lock);
    if (g_bastet_modem_dev_en) {
        BST_PR_LOGE("bastet device has been opened");
        spin_unlock_bh(&g_bastet_modem_data.read_lock);
        return -EPERM;
    }

    g_bastet_modem_dev_en = true;

    spin_unlock_bh(&g_bastet_modem_data.read_lock);
    BST_PR_LOGI("success");

    return 0;
}

/*
 * Get modem id and rab id.
 */
int get_modem_rab_id_by_devname(struct bastet_modem_rab_id *info, const char *local_netdev_name)
{
    struct net_device *dev = NULL;
    struct bst_rnic_modem_info modemInfo;
    int rslt;

    if (info == NULL) {
        return -EINVAL;
    }

    dev = dev_get_by_name(&init_net, local_netdev_name);
    if (dev == NULL) {
        return -ENOENT;
    }

    rslt = rnic_bst_get_modem_info(dev, &modemInfo);
    if (rslt != 0) {
        BST_PR_LOGE("get modem rab id fail\n");
        return -EPERM;
    }
    dev_put(dev);

    if (modemInfo.ipv4_act != 1) {
        BST_PR_LOGE("Ipv4 pdp reg status inactive\n");
        return -EPERM;
    }

    info->modem_id = modemInfo.modem_id;
    /*
     * Bastet only running in IPv4 mode,
     * so, get IPv4 Pdp info
     */
    info->rab_id = modemInfo.ipv4_rab_id;

    return 0;
}

/*
 * Get ipv6 modem id and rab id.
 */
int get_ipv6_modem_rab_id_by_devname(struct bastet_modem_rab_id *info, const char *local_netdev_name)
{
    struct net_device *dev = NULL;
    int rslt;
    struct bst_rnic_modem_info modemInfo;

    if (memset_s(&modemInfo, sizeof(modemInfo), 0, sizeof(modemInfo))) {
        BST_PR_LOGE("memset failed\n");
        return -EINVAL;
    }
    if (info == NULL) {
        return -EINVAL;
    }

    dev = dev_get_by_name(&init_net, local_netdev_name);
    if (dev == NULL) {
        return -ENOENT;
    }

    rslt = rnic_bst_get_modem_info(dev, &modemInfo);
    if (rslt != 0) {
        BST_PR_LOGE("get ipv6 modem rab id fail\n");
        return -EPERM;
    }
    dev_put(dev);

    if (modemInfo.ipv6_act != 1) {
        BST_PR_LOGE("Ipv6 pdp reg status inactive\n");
        return -EPERM;
    }

    info->modem_id = modemInfo.modem_id;
    info->rab_id = modemInfo.ipv6_rab_id;

    return 0;
}


/*
 * this is main method to exchange data with user space,
 * including socket sync, get ip and port, adjust kernel flow
 * return "int" by standard.
 */
static long bastet_modem_ioctl(struct file *flip, unsigned int cmd, unsigned long arg)
{
    int rc = -EFAULT;
    void __user *argp = (void __user *)(uintptr_t)arg;
    char netdev_name_from[IFNAMSIZ];
    struct bastet_modem_rab_id info;

    if (NULL == argp) {
        BST_PR_LOGI("bastet_modem ioctl argp is null.\n");
        return rc;
    }

#ifdef CONFIG_BASTET_COMM
    __pm_stay_awake(&g_bastet_modem_data.bastet_wklock);
    switch (cmd) {
        case BST_MODEM_IOC_GET_MODEM_RAB_ID: {
            BST_PR_LOGI("bastet_modem ioctl get modem rab id.\n");
            if (copy_from_user(&netdev_name_from, argp, sizeof(netdev_name_from)))
                break;

            rc = get_modem_rab_id_by_devname(&info, netdev_name_from);
            if (rc < 0)
                break;

            if (copy_to_user(argp, &info, sizeof(info)))
                rc = -EFAULT;
            break;
        }
        case BST_MODEM_IOC_GET_MODEM_RESET: {
            BST_PR_LOGI("bastet_modem ioctl get modem reset status.\n");
            spin_lock_bh(&g_bastet_modem_data.read_lock);
            // get modem_reset_status and copy to user
            if (copy_to_user(argp, &g_bastet_modem_data.modem_reset_status,
                             sizeof(g_bastet_modem_data.modem_reset_status))) {
                rc = -EFAULT;
            } else {
                // reset the modem_reset_status after read
                g_bastet_modem_data.modem_reset_status = 0;
            }
            spin_unlock_bh(&g_bastet_modem_data.read_lock);
            break;
        }
        case BST_MODEM_IOC_GET_IPV6_MODEM_RAB_ID: {
            BST_PR_LOGI("bastet_modem ioctl get ipv6 modem rab id.\n");
            if (copy_from_user(&netdev_name_from, argp, sizeof(netdev_name_from)))
                break;

            rc = get_ipv6_modem_rab_id_by_devname(&info, netdev_name_from);
            if (rc < 0)
                break;

            if (copy_to_user(argp, &info, sizeof(info)))
                rc = -EFAULT;
            break;
        }
        default: {
            BST_PR_LOGE("unknown ioctl: %u", cmd);
            break;
        }
    }
    __pm_relax(&g_bastet_modem_data.bastet_wklock);
#endif

    return rc;
}

/* support of 32bit userspace on 64bit platforms */
#ifdef CONFIG_COMPAT
static long compat_bastet_modem_ioctl(struct file *flip, unsigned int cmd, unsigned long arg)
{
    return bastet_modem_ioctl(flip, cmd, (unsigned long)(uintptr_t)compat_ptr(arg));
}
#endif

#define BST_MAX_WRITE_PAYLOAD (256)
#define BST_MAX_QUEUE_SIZE (256)

/*lint -e429*/
int post_modem_packet(const void *info, unsigned int len)
{
    struct bastet_modem_packet *pkt = NULL;

    if (!g_bastet_modem_dev_en) {
        BST_PR_LOGE("bastet is not opened\n");
        return -ENOENT;
    }

    pkt = kmalloc(sizeof(struct bastet_modem_packet) + len, GFP_ATOMIC);
    if (NULL == pkt) {
        BST_PR_LOGE("failed to kmalloc\n");
        return -ENOMEM;
    }
    if (memset_s(pkt, sizeof(struct bastet_modem_packet) + len, 0,
                 sizeof(struct bastet_modem_packet) + len)) {
        BST_PR_LOGE("memset failed\n");
    }

    BST_PR_LOGI("bastet modem post packet len %d \n", len);

    pkt->data.cons = 0;
    pkt->data.len = len;
    if (NULL != info) {
        if (memcpy_s(pkt->data.value, BST_MAX_WRITE_PAYLOAD, info, len)) {
            BST_PR_LOGE("memcpy failed\n");
        }
    }

    spin_lock_bh(&g_bastet_modem_data.read_lock);
    if (g_bastet_modem_data.queuelen >= BST_MAX_QUEUE_SIZE) {
        BST_PR_LOGE("queuelen is exceeding the limits\n");
        spin_unlock_bh(&g_bastet_modem_data.read_lock);
        kfree(pkt);
        return -ENOMEM;
    } else {
        list_add_tail(&pkt->list, &g_bastet_modem_data.read_queue);
        g_bastet_modem_data.queuelen++;
    }
    spin_unlock_bh(&g_bastet_modem_data.read_lock);

    wake_up_interruptible_sync_poll(&g_bastet_modem_data.read_wait, POLLIN | POLLRDNORM | POLLRDBAND);

    return 0;
}
/*lint +e429*/
int bastet_msglite_recv(const struct msg_addr *src, void *data, unsigned len)
{
    BST_ACOM_MSG_STRU *pTmpMsg = NULL;
    int ret = 0;
    /*lint -e826*/
    if (data == NULL) {
        BST_PR_LOGE("data is empty\n");
        return -EINVAL;
    } else {
        pTmpMsg = (BST_ACOM_MSG_STRU *)data;
    }

    /*lint +e826*/
    if (len > BST_MAX_WRITE_PAYLOAD) {
        BST_PR_LOGI("bastet_msglite_recv: data length is larger than BST_MAX_WRITE_PAYLOAD.\n");
        len = BST_MAX_WRITE_PAYLOAD;
    }

    post_modem_packet((void *)((unsigned char *)data), len);
    return 0;
}

static int bastet_modem_packet_read(char __user *buf, size_t count)
{
    struct bastet_modem_packet *pkt = NULL;
    uint8_t *data = NULL;
    bool isfree = false;
    uint32_t len = 0;
    uint32_t size = 0;

    if (NULL == buf)
        return -EINVAL;

    BST_PR_LOGI("bastet_modem_packet_read ready to got data\n");

    spin_lock_bh(&g_bastet_modem_data.read_lock);
    if (list_empty(&g_bastet_modem_data.read_queue)) {
        spin_unlock_bh(&g_bastet_modem_data.read_lock);
        return -EAGAIN;
    }

    pkt = list_first_entry(&g_bastet_modem_data.read_queue, struct bastet_modem_packet, list);
    len = pkt->data.len;
    data = (uint8_t *)(&(pkt->data.value)); /*lint !e826 */

    if ((0 == pkt->data.cons) && (count > (size_t)len)) { /*lint !e574 */
        list_del(&pkt->list);
        size = len;
        isfree = true;
    } else if (((0 == pkt->data.cons) && (count <= (size_t)len))                        /*lint !e574 */
               || ((pkt->data.cons != 0) && (pkt->data.cons + count <= (size_t)len))) { /*lint !e574 */
        size = count;
        isfree = false;
    } else {
        list_del(&pkt->list);
        size = len - pkt->data.cons;
        isfree = true;
    }
    spin_unlock_bh(&g_bastet_modem_data.read_lock);

    BST_PR_LOGI("bastet_modem_packet_read got data done and copy to user\n");
    if (copy_to_user(buf, data + pkt->data.cons, size)) {
        pkt->data.cons = 0;
        if (isfree) {
            kfree(pkt);
            spin_lock_bh(&g_bastet_modem_data.read_lock);
            g_bastet_modem_data.queuelen--;
            spin_unlock_bh(&g_bastet_modem_data.read_lock);
        }
        return -EFAULT;
    }
    pkt->data.cons += size;

    if (isfree) {
        kfree(pkt);
        spin_lock_bh(&g_bastet_modem_data.read_lock);
        if (g_bastet_modem_data.queuelen > 0)
            g_bastet_modem_data.queuelen--;
        spin_unlock_bh(&g_bastet_modem_data.read_lock);
    }
    BST_PR_LOGI("bastet_modem_packet_read copy to user done and return size: %d\n", size);
    return size;
}

/*
 * blocked read, it will be waiting here until net device state is change.
 * standard arg is "const char __user *buf".
 */
/*lint -e666*/
static ssize_t bastet_modem_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
    int ret = 0;

    spin_lock_bh(&g_bastet_modem_data.read_lock);
    while (list_empty(&g_bastet_modem_data.read_queue)) {
        spin_unlock_bh(&g_bastet_modem_data.read_lock);
        ret = wait_event_interruptible(g_bastet_modem_data.read_wait, !list_empty(&g_bastet_modem_data.read_queue));
        if (ret)
            return ret;

        spin_lock_bh(&g_bastet_modem_data.read_lock);
    }
    spin_unlock_bh(&g_bastet_modem_data.read_lock);

    BST_PR_LOGI("bastet_modem_read got a data here\n");

    return bastet_modem_packet_read(buf, count);
}

static ssize_t bastet_modem_read_iter(struct kiocb *kiocb, struct iov_iter *iov_iter)
{
    int ret = 0;
    char __user *buf = iov_iter->kvec->iov_base;
    size_t count = iov_iter->count;
    if (buf == NULL) {
        BST_PR_LOGE("bastet_modem_read_iter: buf param error!");
        return -EFAULT;
    }
    spin_lock_bh(&g_bastet_modem_data.read_lock);
    while (list_empty(&g_bastet_modem_data.read_queue)) {
        spin_unlock_bh(&g_bastet_modem_data.read_lock);
        ret = wait_event_interruptible(g_bastet_modem_data.read_wait, !list_empty(&g_bastet_modem_data.read_queue));
        if (ret)
            return ret;

        spin_lock_bh(&g_bastet_modem_data.read_lock);
    }
    spin_unlock_bh(&g_bastet_modem_data.read_lock);

    BST_PR_LOGI("bastet_modem_read got a data here\n");

    return bastet_modem_packet_read(buf, count);
}

/* 功能描述：通过MsgLite将消息发送到C核，代替原来PS_SEND_MSG */
static u32 bst_msglite_send_to_modem(BST_ACOM_MSG_STRU *msg, u32 length)
{
    int ret;
    struct msg_addr dst = {
        .core = MSG_CORE_TSP, /* 接收方C核 */
        .chnid = MSG_CHN_BASTET,
    };
    ret = mdrv_msgchn_lite_sendto(g_bastet_msglite_chnhdl, &dst, msg, length);
    if (ret != 0) {
        BST_PR_LOGE("bst_msglite_send_to_modem: mdrv_msgchn_lite_sendto failed!\n");
        return -EINVAL;
    }
    BST_PR_LOGI("bst_msglite_send_to_modem: mdrv_msgchn_lite_sendto success!\n");
    return 0;
}

/*lint +e666*/
// #ifdef CONFIG_BASTET_COMM

static ssize_t bastet_modem_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
{
    /* u8 msg[BST_MAX_WRITE_PAYLOAD]; */
    u8 *msg = kmalloc(BST_MAX_WRITE_PAYLOAD, GFP_KERNEL);
    u32 ulLength = 0;
    u32 ret = 0;

    if (NULL == msg) {
        BST_PR_LOGE("msg alloc failed!");
        return -1;
    }

    if (memset_s(msg, BST_MAX_WRITE_PAYLOAD, 0, BST_MAX_WRITE_PAYLOAD)) {
        BST_PR_LOGE("memset failed");
    }
    if (NULL == buf) {
        BST_PR_LOGE("buf is null!");
        kfree(msg);
        return -1;
    }

    if ((count > BST_MAX_WRITE_PAYLOAD) || (count <= 0)) {
        BST_PR_LOGE("write length over BST_MAX_WRITE_PAYLOAD!");
        kfree(msg);
        return -EINVAL;
    }

    if (copy_from_user(msg, buf, count)) {
        BST_PR_LOGE("copy_from_user error");
        kfree(msg);
        return -EFAULT;
    }

    BST_PR_LOGI("bastet modem write count = %d\n", (int)count);
    if (bst_msglite_send_to_modem((BST_ACOM_MSG_STRU *)msg, count) != 0) {
        BST_PR_LOGE("bastet_modem_write: bst_msglite_send_to_modem failed!\n");
        kfree(msg);
        return -EFAULT;
    }
    kfree(msg);

    return count;
}

static ssize_t bastet_modem_write_iter(struct kiocb *kiocb, struct iov_iter *iov_iter)
{
    /* u8 msg[BST_MAX_WRITE_PAYLOAD]; */
    u8 *msg = kmalloc(BST_MAX_WRITE_PAYLOAD, GFP_KERNEL);

    void *buf = iov_iter->kvec->iov_base;
    size_t count = iov_iter->count;

    if (msg == NULL) {
        BST_PR_LOGE("msg alloc failed!");
        return -1;
    }

    if (memset_s(msg, BST_MAX_WRITE_PAYLOAD, 0, BST_MAX_WRITE_PAYLOAD)) {
        BST_PR_LOGE("memset failed");
    }
    if (buf == NULL) {
        BST_PR_LOGE("buf is null!");
        kfree(msg);
        return -1;
    }

    if ((count > BST_MAX_WRITE_PAYLOAD) || (count <= 0)) {
        BST_PR_LOGE("write length over BST_MAX_WRITE_PAYLOAD!");
        kfree(msg);
        return -EINVAL;
    }

    if (memcpy_s(msg, BST_MAX_WRITE_PAYLOAD, buf, (u32)count)) {
        BST_PR_LOGE("buf memcpy failed\n");
    }

    BST_PR_LOGI("bastet modem write count = %d\n", (int)count);

    if (bst_msglite_send_to_modem((BST_ACOM_MSG_STRU *)msg, count) != 0) {
        BST_PR_LOGE("bastet_modem_write_iter: bst_msglite_send_to_modem failed!\n");
        kfree(msg);
        return -EFAULT;
    }
    kfree(msg);

    return count;
}

static unsigned int bastet_modem_poll(struct file *file, poll_table *wait)
{
    unsigned int mask;

    poll_wait(file, &g_bastet_modem_data.read_wait, wait);
    mask = !list_empty(&g_bastet_modem_data.read_queue) ? (POLLIN | POLLRDNORM) : 0;

    return mask;
}

static int bastet_modem_release(struct inode *inode, struct file *filp)
{
    struct list_head *p = NULL, *n = NULL;
    struct bastet_modem_packet *pkt = NULL;

    spin_lock_bh(&g_bastet_modem_data.read_lock);

    if (list_empty(&g_bastet_modem_data.read_queue))
        goto out_release;

    list_for_each_safe(p, n, &g_bastet_modem_data.read_queue)
    {
        pkt = list_entry(p, struct bastet_modem_packet, list);
        list_del(&pkt->list);
        kfree(pkt);
    }

out_release:
    g_bastet_modem_dev_en = false;
    spin_unlock_bh(&g_bastet_modem_data.read_lock);
    BST_PR_LOGI("success");

    return 0;
}

static const struct file_operations bastet_modem_dev_fops = {
    .owner = THIS_MODULE,
    .open = bastet_modem_open,
    .unlocked_ioctl = bastet_modem_ioctl,
#ifdef CONFIG_COMPAT
    .compat_ioctl = compat_bastet_modem_ioctl,
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
    .read = bastet_modem_read,
#else
    .read_iter = bastet_modem_read_iter,
#endif

#ifdef CONFIG_BASTET_COMM

// 区分A核消息发到sensorhub还是发到modem
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
    .write = bastet_modem_write,
#else
    .write_iter = bastet_modem_write_iter,
#endif

#endif
    .poll = bastet_modem_poll,
    .release = bastet_modem_release,
};

static void bastet_data_init(void)
{
    spin_lock_init(&g_bastet_modem_data.read_lock);
    INIT_LIST_HEAD(&g_bastet_modem_data.read_queue);
    init_waitqueue_head(&g_bastet_modem_data.read_wait);
    g_bastet_modem_data.modem_reset_status = 0;
    g_bastet_modem_data.queuelen = 0;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
    g_bastet_modem_data.bastet_wklock = *(wakeup_source_register(NULL, "bastet_wakelock"));
    if (&(g_bastet_modem_data.bastet_wklock) == NULL) {
        BST_PR_LOGE("bastet_data_init: bastet wakelock register err!\n");
    }
#else
    wakeup_source_init(&g_bastet_modem_data.bastet_wklock, "bastet_wakelock");
#endif
}

static int bastet_open_msglitechn(void)
{
    struct msgchn_attr attr = {0};

    mdrv_msgchn_attr_init(&attr);
    attr.chnid = MSG_CHN_BASTET; // 通道id
    attr.coremask = MSG_CORE_MASK(MSG_CORE_TSP); // 接收C核消息
    attr.lite_notify = bastet_msglite_recv; /* 回调函数中接收C核数据 */

    g_bastet_msglite_chnhdl = mdrv_msgchn_lite_open(&attr); // 修改返回值
    if (g_bastet_msglite_chnhdl == NULL) {
        BST_PR_LOGE("bastet_open_msglitechn fail");
        return -EINVAL;
    }

    return 0;
}

static void bastet_comm_init(void)
{
    bastet_open_msglitechn();
}

int bastet_modem_driver_init(void)
{
    int ret = 0;
    struct device *dev = NULL;

    BST_PR_LOGE("bastet modem init");
    bastet_data_init();
#ifdef CONFIG_BASTET_COMM
    bastet_comm_init();
    BST_PR_LOGE("bastet modem comm init");

#endif

    /* register bastet major and minor number */
    ret = alloc_chrdev_region(&g_bastet_modem_dev, BST_MODEM_FIRST_MINOR, BST_MODEM_DEVICES_NUMBER, BASTET_MODEM_NAME);
    if (ret) {
        BST_PR_LOGE("alloc_chrdev_region error");
        goto fail_region;
    }

    cdev_init(&g_bastet_modem_cdev, &bastet_modem_dev_fops);
    g_bastet_modem_cdev.owner = THIS_MODULE;

    ret = cdev_add(&g_bastet_modem_cdev, g_bastet_modem_dev, BST_MODEM_DEVICES_NUMBER);
    if (ret) {
        BST_PR_LOGE("cdev_add error");
        goto fail_cdev_add;
    }

    bastet_modem_class = class_create(THIS_MODULE, BASTET_MODEM_NAME);
    if (IS_ERR(bastet_modem_class)) {
        BST_PR_LOGE("class_create error");
        goto fail_class_create;
    }

    dev = device_create(bastet_modem_class, NULL, g_bastet_modem_dev, NULL, BASTET_MODEM_NAME);
    if (IS_ERR(dev)) {
        BST_PR_LOGE("device_create error");
        goto fail_device_create;
    }
    return 0;

fail_device_create:
    class_destroy(bastet_modem_class);
fail_class_create:
    cdev_del(&g_bastet_modem_cdev);
fail_cdev_add:
    unregister_chrdev_region(g_bastet_modem_dev, BST_MODEM_DEVICES_NUMBER);
fail_region:

    return ret;
}

void __exit bastet_modem_driver_exit(void)
{
    if (NULL != bastet_modem_class) {
        device_destroy(bastet_modem_class, g_bastet_modem_dev);
        class_destroy(bastet_modem_class);
    }
    cdev_del(&g_bastet_modem_cdev);
    unregister_chrdev_region(g_bastet_modem_dev, BST_MODEM_DEVICES_NUMBER);
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
module_init(bastet_modem_driver_init);
module_exit(bastet_modem_driver_exit);

MODULE_AUTHOR("huawei.com");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Bastet Modem driver");
#endif

