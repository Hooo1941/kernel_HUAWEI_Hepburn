/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2018-2020. All rights reserved.
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/slab.h>
#include <linux/export.h>
#include <linux/netdevice.h>
#include <linux/syscalls.h>

#include <securec.h>
#include <bsp_print.h>

#include "bsp_pcdev.h"
#include "mdrv_pcdev.h"
#include "pcie_modem.h"
#include "mdrv_errno.h"
#include "bsp_dump.h"

#define PCDEV_MODEM_PREFIX "pmdm_"
#define PCDEV_MODEM_DRV_NAME "pcdev_mdm"
#define THIS_MODU mod_pmdm
#define PCDEV_MODEM_MAX_IOCTL_CB 16
#define PCDEV_MODEM_IOCTL_INVALID_INDEX (-1)

struct modem_ioctl_handler {
    unsigned int cmd;
    int (*modem_ioctl_cb)(void *handle, u32 cmd, void *para);
};

static struct u_modem_driver *g_modem_driver;
static struct pcdev_mdm_port_manager g_pcdev_modem_ports[PCDEV_MDM_COUNT];
unsigned int g_pmdm_port_num_err = 0;
unsigned int g_pmdm_drv_invalid = 0;

struct class *g_pmdm_class;

static int pcdev_mdm_get_port_num(struct inode *inode)
{
    int port_num;

    if (g_modem_driver == NULL) {
        g_pmdm_drv_invalid++;
        return -ENXIO;
    }

    port_num = inode->i_rdev - g_modem_driver->dev_no;

    if (port_num >= PCDEV_MDM_COUNT) {
        g_pmdm_port_num_err++;
        return -ENXIO;
    }

    return port_num;
}

static void pcdev_mdm_write_complete(char *pVirAddr, char *pPhyAddr, int size)
{
    struct gs_pcdev_mdm_port *port = g_pcdev_modem_ports[PCDEV_MDM_CONSOLE_IDX].port;
    struct sk_buff *skb = NULL;
    unsigned long flags;

    if (size >= 0) {
        port->pstats.tx_done++;
        port->pstats.tx_done_bytes += size;
    } else {
        port->pstats.tx_done_fail++;
    }

    spin_lock_irqsave(&port->port_lock, flags);
    skb = __skb_dequeue_tail(&port->skb_list);
    spin_unlock_irqrestore(&port->port_lock, flags);
    dev_kfree_skb_any(skb);
}

static ssize_t pcdev_mdm_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    return -ENOTSUPP;
}

static ssize_t pcdev_mdm_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    return -ENOTSUPP;
}

static int pcdev_mdm_write_base(struct gs_pcdev_mdm_port *port, struct sk_buff *skb)
{
    pcdev_wr_async_info_s rw_info;
    unsigned long flags;
    int ret;

    if (unlikely(skb == NULL || skb->len == 0)) {
        port->pstats.tx_inv_param++;
        return -EINVAL;
    }

    port->pstats.write_bytes += skb->len;

    rw_info.p_vaddr = skb->data;
    rw_info.p_paddr = 0;
    rw_info.size = skb->len;
    ret = bsp_pcdev_ioctl(port->fd, PCDEV_IOCTL_WRITE_ASYNC, &rw_info);
    if (!ret) {
        spin_lock_irqsave(&port->port_lock, flags);
        __skb_queue_head(&port->skb_list, skb);
        spin_unlock_irqrestore(&port->port_lock, flags);
    }

    return ret;
}

static int pcdev_mdm_get_read_buf(struct gs_pcdev_mdm_port *port, pcdev_wr_async_info_s *read_info)
{
    pcdev_wr_async_info_s rw_info;
    struct sk_buff *skb = NULL;
    char *skb_data = NULL;
    int status;

    status = bsp_pcdev_ioctl(port->fd, PCDEV_IOCTL_GET_RD_BUFF, &rw_info);
    if (status) {
        status = -EAGAIN;
        return status;
    }

    skb = dev_alloc_skb(rw_info.size);
    if (!skb) {
        bsp_err("<%s>dev_alloc_skb fail\n", __func__);
        return -ENOMEM;
    }
    skb_data = skb_put(skb, rw_info.size);
    status = memcpy_s(skb_data, skb->len, rw_info.p_vaddr, rw_info.size);
    if (status) {
        bsp_err("<%s>memcpy fail\n", __func__);
    }

    read_info->p_vaddr = (char *)skb;
    read_info->size = skb->len;

    status = bsp_pcdev_ioctl(port->fd, PCDEV_IOCTL_RETURN_BUFF, &rw_info);
    if (status) {
        bsp_err("<%s>ret buf fail\n", __func__);
    }
    return status;
}

static int pcdev_mdm_ret_read_buf(struct gs_pcdev_mdm_port *port, pcdev_wr_async_info_s *read_info)
{
#ifdef CONFIG_PCIE_CDEV
    struct sk_buff *free_skb = (struct sk_buff *)read_info->p_vaddr;
    dev_kfree_skb_any(free_skb);
#endif
    return 0;
}

static int ioctl_write_async_cmd(void *handle, u32 cmd, void *arg)
{
    struct gs_pcdev_mdm_port *port = container_of(handle, struct gs_pcdev_mdm_port, fd);
    pcdev_wr_async_info_s *rw_info = NULL;

    if (arg == 0) {
        bsp_err("gs_acm_modem_ioctl ACM_IOCTL_WRITE_ASYNC invalid param\n");
        return -EFAULT;
    }
    rw_info = (pcdev_wr_async_info_s *)arg;
    port->pstats.write_async_call++;
    return pcdev_mdm_write_base(port, (struct sk_buff *)rw_info->p_vaddr);
}

static int ioctl_get_rdbuf_cmd(void *handle, u32 cmd, void *arg)
{
    struct gs_pcdev_mdm_port *port = container_of(handle, struct gs_pcdev_mdm_port, fd);

    if (arg == 0) {
        bsp_err("gs_acm_modem_ioctl ACM_IOCTL_GET_RD_BUFF invalid param\n");
        return -EFAULT;
    }
    port->pstats.get_buf_call++;
    return pcdev_mdm_get_read_buf(port, (pcdev_wr_async_info_s *)arg);
}

static int ioctl_return_rdbuf_cmd(void *handle, u32 cmd, void *arg)
{
    struct gs_pcdev_mdm_port *port = container_of(handle, struct gs_pcdev_mdm_port, fd);

    if (arg == 0) {
    bsp_err("gs_acm_modem_ioctl ACM_IOCTL_RETURN_BUFF invalid param\n");
        return -EFAULT;
    }
    port->pstats.ret_buf_call++;
    return pcdev_mdm_ret_read_buf(port, (pcdev_wr_async_info_s *)arg);
}

static int ioctl_realloc_rdbuf_cmd(void *handle, u32 cmd, void *arg)
{
    struct gs_pcdev_mdm_port *port = container_of(handle, struct gs_pcdev_mdm_port, fd);
    pcdev_read_buf_info_s *rd_info = NULL;

    if (arg == 0) {
        bsp_err("gs_acm_modem_ioctl ACM_IOCTL_RETURN_BUFF invalid param\n");
        return -EFAULT;
    }
    rd_info = (pcdev_read_buf_info_s *)arg;
    port->read_buf_size = (((unsigned)rd_info->buf_size + MAX_MPS_SIZE - 1) / MAX_MPS_SIZE) * MAX_MPS_SIZE;
    rd_info->buf_size = port->read_buf_size;
    return bsp_pcdev_ioctl(port->fd, cmd, arg);
}

static int set_write_cb(void *handle, u32 cmd, void *arg)
{
    struct gs_pcdev_mdm_port *port = container_of(handle, struct gs_pcdev_mdm_port, fd);
    port->write_done_cb = (pcdev_write_done_cb_t)arg;
    return 0;
}

struct modem_ioctl_handler g_modem_ioctl_handler_table[PCDEV_MODEM_MAX_IOCTL_CB] = {
    { PCDEV_IOCTL_SET_READ_CB, bsp_pcdev_ioctl },
    { PCDEV_IOCTL_SET_WRITE_CB, set_write_cb },
    { PCDEV_IOCTL_SET_EVT_CB, bsp_pcdev_ioctl },
    { PCDEV_IOCTL_SEND_EVT, bsp_pcdev_ioctl },
    { PCDEV_IOCTL_SEND_MSC_READ, bsp_pcdev_ioctl },
    { PCDEV_IOCTL_SEND_REL_IND, bsp_pcdev_ioctl },
    { PCDEV_IOCTL_WRITE_ASYNC, ioctl_write_async_cmd },
    { PCDEV_IOCTL_GET_RD_BUFF, ioctl_get_rdbuf_cmd },
    { PCDEV_IOCTL_RETURN_BUFF, ioctl_return_rdbuf_cmd },
    { PCDEV_IOCTL_RELLOC_READ_BUFF, ioctl_realloc_rdbuf_cmd },
    { PCDEV_IOCTL_SET_FREE_CB, NULL },
    { PCDEV_IOCTL_WRITE_DO_COPY, NULL },
    { PCDEV_MODEM_IOCTL_SET_MSC_READ_CB, bsp_pcdev_ioctl },
    { PCDEV_MODEM_IOCTL_MSC_WRITE_CMD, bsp_pcdev_ioctl },
    { PCDEV_MODEM_IOCTL_SET_REL_IND_CB, bsp_pcdev_ioctl },
    { PCDEV_MODEM_IOCTL_SET_MSC_WRITE_CB, bsp_pcdev_ioctl },
};

static int mdm_ioctl_switch(struct gs_pcdev_mdm_port *port, unsigned int cmd, unsigned long arg)
{
    struct modem_ioctl_handler *ioctl_handler_table = &g_modem_ioctl_handler_table[0];
    int status = 0;
    int index = PCDEV_MODEM_IOCTL_INVALID_INDEX;
    int i;

    for (i = 0; i < PCDEV_MODEM_MAX_IOCTL_CB; i++) {
        if (ioctl_handler_table[i].cmd == cmd) {
            index = i;
            break;
        }
    }

    if (index < 0 || index >= PCDEV_MODEM_MAX_IOCTL_CB) {
        return PCDEV_MODEM_IOCTL_INVALID_INDEX;
    }

    if (ioctl_handler_table[index].modem_ioctl_cb != NULL) {
        if (cmd == PCDEV_IOCTL_SEND_REL_IND) {
            status = ioctl_handler_table[index].modem_ioctl_cb(port->fd,
                PCDEV_IOCTL_SEND_MSC_READ, (void *)(uintptr_t)arg);
        } else {
            status = ioctl_handler_table[index].modem_ioctl_cb(port->fd, cmd, (void *)(uintptr_t)arg);
        }
    }

    return status;
}

long pcdev_mdm_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct inode *inode = NULL;
    struct gs_pcdev_mdm_port *port = NULL;
    int status;
    unsigned long flags;

    if (file == NULL || arg == 0) {
        bsp_err("file is null or arg is 0\n");
        return -ESHUTDOWN;
    }

    inode = file->f_path.dentry->d_inode;
    if (get_fs() != KERNEL_DS) {
        bsp_err("can't support in usr space\n");
        return -ENOTSUPP;
    }

    status = pcdev_mdm_get_port_num(inode);
    if (status < 0) {
        bsp_err("gs_acm_modem_get_port_num err, status=%d\n", status);
        return status;
    }

    port = g_pcdev_modem_ports[status].port;
    if (port == NULL) {
        bsp_err("modem is disconnect\n");
        return -ESHUTDOWN;
    }

    spin_lock_irqsave(&port->port_lock, flags);
    if (!port->open_count) {
        bsp_err("<%s>modem is not open\n", __func__);
        spin_unlock_irqrestore(&port->port_lock, flags);
        return -ENODEV;
    }
    spin_unlock_irqrestore(&port->port_lock, flags);

    return mdm_ioctl_switch(port, cmd, arg);
}

static int mdm_open_process(int port_num)
{
    struct gs_pcdev_mdm_port *port = NULL;
    int status;
    unsigned long flags;

    do {
        mutex_lock(&g_pcdev_modem_ports[port_num].open_close_lock);
        port = g_pcdev_modem_ports[port_num].port;
        if (port == NULL) {
            mutex_unlock(&g_pcdev_modem_ports[port_num].open_close_lock);
            bsp_err("<%s>port is null\n", __func__);
            return -ENODEV;
        } else {
            spin_lock_irqsave(&port->port_lock, flags);

            /* already open?  Great. */
            if (port->open_count) {
                status = 0;
                port->open_count++;

                /* currently opening/closing? wait ... */
            } else if (port->openclose) {
                status = -EBUSY;

                /* ... else we do the work */
            } else {
                status = -EAGAIN;
                port->openclose = true;
            }
            spin_unlock_irqrestore(&port->port_lock, flags);
        }
        mutex_unlock(&g_pcdev_modem_ports[port_num].open_close_lock);

        switch (status) {
            default:
                /* fully handled */
                return status;
            case -EAGAIN:
                /* must do the work */
                break;
            case -EBUSY:
                /* wait for EAGAIN task to finish */
                msleep(10); /* sleep 10 miliseconds */
                /*
                 * REVISIT could have a waitchannel here, if
                 * concurrent open performance is important
                 */
                break;
            default:
                break;
        }
    } while (status != -EAGAIN);

    spin_lock_irqsave(&port->port_lock, flags);
    port->open_count = 1;
    port->openclose = false;

    spin_unlock_irqrestore(&port->port_lock, flags);
    return 0;
}

static int pcdev_mdm_open(struct inode *inode, struct file *filp)
{
    int port_num;
    int ret;

    if (get_fs() != KERNEL_DS) {
        bsp_err("can't support in usr space\n");
        return -ENOTSUPP;
    }

    port_num = pcdev_mdm_get_port_num(inode);
    if (port_num < 0) {
        return port_num;
    }

    ret = mdm_open_process(port_num);
    if (ret) {
        return ret;
    }

    bsp_err("<%s> success\n", __func__);
    return 0;
}

static int pcdev_mdm_close(struct inode *inode, struct file *file)
{
    struct gs_pcdev_mdm_port *port = NULL;
    unsigned long flags;
    int status;

    if (inode == NULL || file == NULL) {
        bsp_err("inodr or file is NULL\n");
        return -EBADHANDLE;
    }

    if (get_fs() != KERNEL_DS) {
        bsp_err("can't support in usr space\n");
        return -ENOTSUPP;
    }

    status = pcdev_mdm_get_port_num(inode);
    if (status < 0) {
        return status;
    }
    port = g_pcdev_modem_ports[status].port;

    spin_lock_irqsave(&port->port_lock, flags);

    if (port->open_count != 1) {
        if (port->open_count == 0) {
            WARN_ON(1);
            spin_unlock_irqrestore(&port->port_lock, flags);
            return -EBADF;
        } else {
            --port->open_count;
        }
        goto exit;
    }

    port->open_count = 0;

exit:
    spin_unlock_irqrestore(&port->port_lock, flags);
    return 0;
}

static const struct file_operations g_pcdev_modem_fops = {
    .read = pcdev_mdm_read,
    .write = pcdev_mdm_write,
    .unlocked_ioctl = pcdev_mdm_ioctl,
    .open = pcdev_mdm_open,
    .release = pcdev_mdm_close,
};

void pcie_modem_init_cb(void)
{
    struct gs_pcdev_mdm_port *port;

    port = g_pcdev_modem_ports[PCDEV_MDM_CONSOLE_IDX].port;
    if (port == NULL) {
        bsp_err("<%s> port is null\n", __func__);
        return;
    }

    port->fd = bsp_pcdev_open(pcdev_modem);
    if (port->fd == NULL) {
        bsp_err("pcie_tty_open open failed\n");
        return;
    }

    bsp_pcdev_ioctl(port->fd, PCDEV_IOCTL_SET_WRITE_CB, pcdev_mdm_write_complete);

    return;
}

void pcie_modem_disable_cb(void)
{
    struct gs_pcdev_mdm_port *port;

    port = g_pcdev_modem_ports[PCDEV_MDM_CONSOLE_IDX].port;

    bsp_pcdev_close(port->fd);
    return;
}

static int pcdev_modem_port_alloc(unsigned port_num)
{
    struct gs_pcdev_mdm_port *port;

    port = kzalloc(sizeof(struct gs_pcdev_mdm_port), GFP_KERNEL);
    if (port == NULL)
        return -ENOMEM;

    spin_lock_init(&port->port_lock);
    skb_queue_head_init(&port->skb_list);

    port->port_num = port_num;
    port->read_buf_size = PCDEV_MDM_DFT_RD_BUF_SIZE;

    g_pcdev_modem_ports[port_num].port = port;
    g_pcdev_modem_ports[port_num].is_alloc = 1;
    return 0;
}

static struct u_modem_driver *pcdev_modem_alloc_driver(int lines)
{
    struct u_modem_driver *driver;

    driver = kzalloc(sizeof(struct u_modem_driver), GFP_KERNEL);
    if (driver != NULL) {
        kref_init(&driver->kref);
        driver->num = lines;
    }
    return driver;
}

static int pcdev_modem_register_driver(struct u_modem_driver *driver)
{
    int error;
    dev_t dev;

    error = alloc_chrdev_region(&dev, driver->minor_start, driver->num, driver->name);
    if (error < 0) {
        return error;
    }

    driver->major = MAJOR(dev);
    driver->minor_start = MINOR(dev);

    cdev_init(&driver->cdev, &g_pcdev_modem_fops);
    driver->cdev.owner = driver->owner;

    error = cdev_add(&driver->cdev, dev, driver->num);
    if (error) {
        unregister_chrdev_region(dev, driver->num);
        return error;
    }
    driver->dev_no = dev;

    return 0;
}

static void pcdev_modem_unregister_driver(struct u_modem_driver *driver)
{
    cdev_del(&driver->cdev);
    unregister_chrdev_region(g_modem_driver->dev_no, g_modem_driver->num);
    return;
}

static struct device *pcdev_modem_register_device(struct u_modem_driver *driver, unsigned index, struct device *device)
{
    dev_t dev = MKDEV(driver->major, driver->minor_start) + index;

    if ((int)index >= driver->num) {
        bsp_err("Attempt to register invalid tty line number (%d).\n", index);
        return ERR_PTR(-EINVAL);
    }

    return device_create(g_pmdm_class, device, dev, NULL, "pcdev_mdm");
}

static void pcdev_modem_unregister_device(struct u_modem_driver *driver, unsigned index)
{
    device_destroy(g_pmdm_class, MKDEV(driver->major, driver->minor_start) + index);
}

static int pcdev_modem_register_devices(void)
{
    int i;
    struct device *cdev = NULL;
    g_pmdm_class = class_create(THIS_MODULE, "g_pmdm_class");
    if (IS_ERR(g_pmdm_class)) {
        return PTR_ERR(g_pmdm_class);
    }

    /* register devices ... */
    for (i = 0; i < PCDEV_MDM_COUNT; i++) {
        /* register devices ... */
        cdev = pcdev_modem_register_device(g_modem_driver, i, NULL);
        if (IS_ERR(cdev)) {
            bsp_err("no classdev for port %d, err %ld\n", i, PTR_ERR(cdev));
            goto setup_fail;
        }
        g_pcdev_modem_ports[i].cdev = cdev;
    }
    return 0;

setup_fail:
    while (i > 0) {
        i--;
        /* start sysfs and /dev/ttyGS* node removal */
        if (g_pcdev_modem_ports[i].cdev) {
            pcdev_modem_unregister_device(g_modem_driver, i);
            g_pcdev_modem_ports[i].cdev = NULL;
        }
    }

    return -ENXIO;
}

static void pcdev_modem_unregister_devices(void)
{
    int i;

    for (i = 0; i < PCDEV_MDM_COUNT; i++) {
        if (g_pcdev_modem_ports[i].cdev) {
            pcdev_modem_unregister_device(g_modem_driver, i);
            g_pcdev_modem_ports[i].cdev = NULL;
        }
    }
    return;
}

static int pcie_modem_init(void)
{
    unsigned i;
    int status;
    bsp_err("[init]start\n");
    g_modem_driver = pcdev_modem_alloc_driver(PCDEV_MDM_COUNT);
    if (g_modem_driver == NULL)
        return -ENOMEM;

    g_modem_driver->owner = THIS_MODULE;
    g_modem_driver->driver_name = PCDEV_MODEM_DRV_NAME;
    g_modem_driver->name = PCDEV_MODEM_PREFIX;

    /* register the driver ... */
    status = pcdev_modem_register_driver(g_modem_driver);
    if (status) {
        bsp_err("cannot register, err %d\n", status);
        goto out1;
    }

    g_modem_driver->pmdm_work_queue = create_singlethread_workqueue("pmdm");
    if (g_modem_driver->pmdm_work_queue == NULL) {
        status = -ENOMEM;
        goto out1;
    }

    status = pcdev_modem_register_devices();
    if (status) {
        bsp_err("cannot register, err %d\n", status);
        goto out1;
    }

    /* alloc and init each port */
    for (i = 0; i < PCDEV_MDM_COUNT; i++) {
        mutex_init(&g_pcdev_modem_ports[i].open_close_lock);
        status = pcdev_modem_port_alloc(i);
        if (status) {
            goto out2;
        }
    }

    bsp_err("[init]ok\n");

    return status;

out2:
    pcdev_modem_unregister_devices();
out1:
    if (g_modem_driver->pmdm_work_queue != NULL)
        destroy_workqueue(g_modem_driver->pmdm_work_queue);

    pcdev_modem_unregister_driver(g_modem_driver);

    if (g_modem_driver != NULL) {
        kfree(g_modem_driver);
        g_modem_driver = NULL;
    }

    return status;
}

int pmodem_init(void)
{
    int ret;
    bsp_err("<%s>in\n", __func__);
    ret = pcie_modem_init();
    bsp_pcdev_reg_enablecb(pcie_modem_init_cb);
    bsp_pcdev_reg_disablecb(pcie_modem_disable_cb);
    return ret;
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
module_init(pmodem_init);
#endif

void pmdm_port_dump(void)
{
    struct gs_pcdev_mdm_port *port = g_pcdev_modem_ports[PCDEV_MDM_CONSOLE_IDX].port;

    if (port == NULL) {
        return;
    }

    bsp_err("fd                     %llx\n", (unsigned long)port->fd);
    bsp_err("port_num               %d\n", port->port_num);
    bsp_err("open_count             %d\n", port->open_count);
    bsp_err("openclose              %d\n", port->openclose);
    bsp_err("write_done_cb          %s\n", ((port->write_done_cb == NULL) ? ("NULL") : ("SET")));
    bsp_err("read_buf_size          %d\n", port->read_buf_size);

    bsp_err("write_async_call       %ld\n", port->pstats.write_async_call);
    bsp_err("write_bytes            %ld\n", port->pstats.write_bytes);
    bsp_err("tx_inv_param           %ld\n", port->pstats.tx_inv_param);
    bsp_err("tx_done                %ld\n", port->pstats.tx_done);
    bsp_err("tx_done_bytes          %ld\n", port->pstats.tx_done_bytes);
    bsp_err("tx_done_fail           %ld\n", port->pstats.tx_done_fail);

    bsp_err("get_buf_call           %ld\n", port->pstats.get_buf_call);
    bsp_err("ret_buf_call           %ld\n", port->pstats.ret_buf_call);
}

void st_pmdm_open_test(void)
{
    unsigned int port_n = pcdev_modem;
    void *pcdev_filep = NULL;

    pcdev_filep = bsp_pcdev_open(port_n);
    if (pcdev_filep == NULL) {
        bsp_err("port(%d) fail \n", port_n);
        return;
    }

    bsp_pcdev_close(pcdev_filep);
    return;
}
