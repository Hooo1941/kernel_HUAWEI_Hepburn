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
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/slab.h>
#include <linux/export.h>
#include <linux/console.h>
#include <linux/delay.h>

#include <securec.h>
#include <bsp_print.h>

#include "bsp_nvim.h"

#include "pcie_tty.h"
#include "bsp_pcdev.h"
#include "mdrv_pcdev.h"
#include "pcie_cdev_dbg.h"

#ifndef THIS_MODU
#define THIS_MODU mod_ptty
#endif

static struct tty_driver *g_pcie_tty_driver;
#define MAX_PCIE_TTY_PORTS 1
#define PCIE_CONSOLE_IDX 0
#define WRITE_BUF_SIZE 8192
#define WRITE_LEFT_MIN 128
#define PCIE_TTY_NAME "uw_tty"
#define USB_CONSOLE_SHELL 0x40

#define PCIE_TTY_SPEED 9600
#define PCIE_TTY_DTS_POS 2

pcdev_wr_async_info_s g_ptty_last_rw_info;

#ifdef CONFIG_PCIE_CDEV_DBG_USB_SHELL

typedef NV_USB_FEATURE usb_feature_t;
typedef struct {
    /* usb nv structure ... */
    usb_feature_t usb_feature;

    /* stat counter */
    unsigned int stat_nv_read_fail;
    unsigned int stat_nv_init;
} ptty_usb_nv_t;

static ptty_usb_nv_t g_usb_nv = {
    .usb_feature = {0},
    .stat_nv_read_fail = 0,
    .stat_nv_init = 0
};

#endif

struct portctl {
    struct mutex lock; /* protect open/close */
    struct gs_port *port;
} g_ptty_ports[MAX_PCIE_TTY_PORTS];

struct pcie_tty_name_type_tbl g_pcie_tty_type_table[MAX_PCIE_TTY_PORTS] = {
    /* name             type(prot id) */
    { PCIE_TTY_NAME, IF_PROTOCOL_3G_GPS },
};

#define CONSOLE_BUFFER_SIZE (32 * 1024)

struct pcie_tty_console_ctx_info {
    struct gs_buf buf_info;
    spinlock_t lock;
    struct delayed_work write_work;
    struct delayed_work read_work;
};

static struct pcie_tty_console_ctx_info g_console_ctx;

/* -------------------------------------------------------------------------
 * Circular Buffer
 * gs_buf_alloc
 * Allocate a circular buffer and all associated memory.
 */
static int gs_buf_alloc(struct gs_buf *gb, unsigned size)
{
    if (size == 0 || size > CONSOLE_BUFFER_SIZE) {
        return -EINVAL;
    }
    gb->buf_buf = kmalloc(size, GFP_KERNEL);
    if (gb->buf_buf == NULL)
        return -ENOMEM;

    gb->buf_size = size;
    gb->buf_put = gb->buf_buf;
    gb->buf_get = gb->buf_buf;

    return 0;
}

/*
 * gs_buf_free
 * Free the buffer and all associated memory.
 */
static void gs_buf_free(struct gs_buf *gb)
{
    if (gb->buf_buf != NULL) {
        kfree(gb->buf_buf);
        gb->buf_buf = NULL;
    }
}

/*
 * gs_buf_data_avail
 * Return the number of bytes of data written into the circular
 * buffer.
 */
static unsigned gs_buf_data_avail(struct gs_buf *gb)
{
    return (gb->buf_size + gb->buf_put - gb->buf_get) % gb->buf_size;
}

/*
 * gs_buf_space_avail
 * Return the number of bytes of space available in the circular
 * buffer.
 */
static unsigned gs_buf_space_avail(struct gs_buf *gb, int line_state_on)
{
    unsigned len;
    len = (gb->buf_size + gb->buf_get - gb->buf_put - 1) % gb->buf_size;

    /*
     * if the left romm is not enough, and cosole is not connect,
     * overwrite the old data
     */
    if (!line_state_on && len < WRITE_LEFT_MIN) {
        gb->buf_put = gb->buf_buf;
        gb->buf_get = gb->buf_buf;
        len = gb->buf_size - 1;
    }
    return len;
}

/*
 * gs_buf_put
 * Copy data data from a user buffer and put it into the circular buffer.
 * Restrict to the amount of space available.
 * Return the number of bytes copied.
 */
static unsigned gs_buf_put(struct gs_buf *gb, const char *buf, unsigned count, int line_on)
{
    unsigned len;
    int ret;

    len = gs_buf_space_avail(gb, line_on);
    if (count > len) {
        count = len;
    }

    if (count == 0) {
        return 0;
    }

    len = gb->buf_buf + gb->buf_size - gb->buf_put;
    if (count > len) {
        ret = memcpy_s(gb->buf_put, len, buf, len);
        if (ret) {
            PCDEV_ERR("memset_s failed, line: %d \n", __LINE__);
        }
        ret = memcpy_s(gb->buf_buf, gb->buf_size, buf + len, count - len);
        if (ret) {
            PCDEV_ERR("memset_s failed, line: %d \n", __LINE__);
        }
        gb->buf_put = gb->buf_buf + count - len;
    } else {
        ret = memcpy_s(gb->buf_put, len, buf, count);
        if (ret) {
            PCDEV_ERR("memset_s failed, line: %d \n", __LINE__);
        }
        if (count < len) {
            gb->buf_put += count;
        } else {
            gb->buf_put = gb->buf_buf;
        }
    }

    return count;
}

/*
 * gs_buf_get
 * Get data from the circular buffer and copy to the given buffer.
 * Restrict to the amount of data available.
 * Return the number of bytes copied.
 */
static unsigned gs_buf_get(struct gs_buf *gb, char *buf, unsigned count)
{
    unsigned len;
    int ret;

    len = gs_buf_data_avail(gb);
    if (count > len) {
        count = len;
    }

    if (count == 0) {
        return 0;
    }

    len = gb->buf_buf + gb->buf_size - gb->buf_get;
    if (count > len) {
        ret = memcpy_s(buf, len, gb->buf_get, len);
        if (ret) {
            PCDEV_ERR("memset_s failed, line: %d \n", __LINE__);
        }
        ret = memcpy_s(buf + len, gb->buf_size, gb->buf_buf, count - len);
        if (ret) {
            PCDEV_ERR("memset_s failed, line: %d \n", __LINE__);
        }
        gb->buf_get = gb->buf_buf + count - len;
    } else {
        ret = memcpy_s(buf, len, gb->buf_get, count);
        if (ret) {
            PCDEV_ERR("memset_s failed, line: %d \n", __LINE__);
        }
        if (count < len) {
            gb->buf_get += count;
        } else {
            gb->buf_get = gb->buf_buf;
        }
    }

    return count;
}

/*
 * convert '\n' -> "\r\n"
 */
static int gs_char_cpy(char *d, const char *s, unsigned size, unsigned max, unsigned *dst_size)
{
    int src_pos = 0;
    int dst_pos = 0;

    /* max - 2 : reserve the 2 rooms for last "\r\n" */
    while (src_pos < (int)size && dst_pos < (int)max - PCIE_TTY_DTS_POS) {
        if (*(s + src_pos) == '\n') {
            *(d + dst_pos) = '\r';
            *(d + dst_pos + 1) = '\n';
            dst_pos += PCIE_TTY_DTS_POS;
            src_pos++;
        } else {
            *(d + dst_pos) = *(s + src_pos);
            src_pos++;
            dst_pos++;
        }
    }

    *dst_size = dst_pos;
    return src_pos;
}

int gs_pcie_console_write(struct gs_port *port, const char *buf, unsigned count)
{
    unsigned long flags;
    unsigned trans_size;
    int status;
    pcdev_wr_async_info_s rw_info;
    char *s_buf = NULL;

    spin_lock_irqsave(&port->port_lock, flags);
    if (unlikely(port->fd == NULL)) {
        port->pstats.fd_null++;
        spin_unlock_irqrestore(&port->port_lock, flags);
        return -ENODEV;
    }

    if (unlikely(!port->port.count)) {
        port->pstats.pcie_console_write_count_null++;
        spin_unlock_irqrestore(&port->port_lock, flags);
        return -EINVAL;
    }

    s_buf = kmalloc(port->maxpacket, GFP_ATOMIC);
    if (s_buf == NULL) {
        status = -ENOMEM;
        goto console_write_exit;
    }
    count = gs_char_cpy(s_buf, (char *)buf, count, port->maxpacket, &trans_size);

    spin_unlock_irqrestore(&port->port_lock, flags);

    rw_info.p_vaddr = s_buf;
    rw_info.size = trans_size;
    rw_info.p_paddr = 0;
    port->pstats.pcie_console_write++;
    status = bsp_pcdev_ioctl(port->fd, PCDEV_IOCTL_WRITE_ASYNC, &rw_info);
    if (status) {
        port->pstats.pcie_console_write_fail++;
        goto console_write_exit;
    }
    port->pstats.pcie_console_write_len += count;
    port->write_started++;
    status = count;

console_write_exit:
    return status;
}

static void gs_console_write(struct console *co, const char *buf, unsigned count)
{
    struct gs_port *port = g_ptty_ports[PCIE_CONSOLE_IDX].port;
    unsigned long flags;
    int no_data = 0;

    if (count == 0 || port == NULL)
        return;

    port->pstats.console_write++;

    /* write the data to usb console buffer */
    spin_lock_irqsave(&g_console_ctx.lock, flags);
    if (gs_buf_space_avail(&g_console_ctx.buf_info, 1) < count) {
        port->pstats.console_write_no_room++;
        no_data = 1;
    }
    (void)gs_buf_put(&g_console_ctx.buf_info, buf, count, 1);
    spin_unlock_irqrestore(&g_console_ctx.lock, flags);

    /* we can delay a little time to get the more msg */
    if (!no_data) {
        schedule_delayed_work(&g_console_ctx.write_work, 1);
    }
    return;
}

static struct tty_driver *gs_console_device(struct console *co, int *index)
{
    struct tty_driver **p = NULL;

    if (co == NULL || index == NULL) {
        bsp_err("co or index is null\n");
        return NULL;
    }

    p = (struct tty_driver **)co->data;
    if (!*p) {
        return NULL;
    }

    *index = co->index;
    return *p;
}

static int gs_console_write_to_pcie(const char *buf, unsigned count)
{
    struct gs_port *port = g_ptty_ports[PCIE_CONSOLE_IDX].port;

    if (count == 0 || port == NULL) {
        return 0;
    }

    port->pstats.console_write_to_pcie++;

    while (count) {
        int ret;
        ret = gs_pcie_console_write(port, buf, count);
        if (ret < 0) {
            return ret;
        }
        buf += ret;
        count -= ret;
    }
    return 0;
}

static void gs_console_write_work(struct work_struct *work)
{
    unsigned long flags;
    unsigned count;
    unsigned len;
    struct gs_buf *gb = &g_console_ctx.buf_info;
    struct gs_port *port = g_ptty_ports[PCIE_CONSOLE_IDX].port;

    if (port == NULL)
        return;

    port->pstats.console_write_work++;

    spin_lock_irqsave(&g_console_ctx.lock, flags);
    count = gs_buf_data_avail(gb);
    if (count == 0) {
        spin_unlock_irqrestore(&g_console_ctx.lock, flags);
        goto write_exit;
    }

    if (count > port->pstats.console_data_max) {
        port->pstats.console_data_max = count;
    }
    len = gb->buf_buf + gb->buf_size - gb->buf_get;
    spin_unlock_irqrestore(&g_console_ctx.lock, flags);
    if (count > len) {
        (void)gs_console_write_to_pcie(gb->buf_get, len);
        (void)gs_console_write_to_pcie(gb->buf_buf, count - len);
        gb->buf_get = gb->buf_buf + count - len;
    } else {
        (void)gs_console_write_to_pcie(gb->buf_get, count);
        if (count < len) {
            gb->buf_get += count;
        } else {
            gb->buf_get = gb->buf_buf;
        }
    }

write_exit:
    return;
}

static int gs_console_setup(struct console *co, char *options)
{
    struct gs_port *port = g_ptty_ports[PCIE_CONSOLE_IDX].port;
    if (port == NULL) {
        return -ENODEV;
    }

    port->pstats.console_setup++;

    if (gs_buf_alloc(&g_console_ctx.buf_info, CONSOLE_BUFFER_SIZE)) {
        port->pstats.console_setup_buf_alloc_fail++;
        return -ENOMEM;
    }
    spin_lock_init(&g_console_ctx.lock);
    INIT_DELAYED_WORK(&g_console_ctx.write_work, gs_console_write_work);
    return 0;
}

struct console g_pcie_tty_console = {
    .name = PCIE_TTY_NAME,
    .write = gs_console_write,
    .device = gs_console_device,
    .setup = gs_console_setup,
    .flags = CON_PRINTBUFFER,
    .index = -1,
    .data = &g_pcie_tty_driver,
};

/*
 * gs_send_packet
 * If there is data to send, a packet is built in the given
 * buffer and the size is returned.  If there is no data to
 * send, 0 is returned.
 * Called with port_lock held.
 */
static unsigned gs_send_packet(struct gs_port *port, char *packet, unsigned size)
{
    unsigned len;

    len = gs_buf_data_avail(&port->port_write_buf);
    if (len < size) {
        size = len;
    }
    if (size != 0) {
        size = gs_buf_get(&port->port_write_buf, packet, size);
    }
    return size;
}

/*
 * pcie_tty_start_tx
 * This function finds available write requests, calls
 * gs_send_packet to fill these packets with data, and
 * continues until either there are no more write requests
 * available or no more data to send.  This function is
 * run whenever data arrives or write requests are available.
 * Context: caller owns port_lock; port_usb is non-null.
 */
static int pcie_tty_start_tx(struct gs_port *port)
{
    int len;
    int status = 0;
    bool do_tty_wake = false;
    char *buf = NULL;
    pcdev_wr_async_info_s rw_info;
    unsigned long flags;

    port->pstats.start_tx++;

    do {
        buf = kmalloc(port->maxpacket, GFP_ATOMIC);
        len = gs_send_packet(port, buf, port->maxpacket);
        if (len == 0) {
            kfree(buf);
            break;
        }
        do_tty_wake = true;
        rw_info.p_vaddr = buf;
        rw_info.size = len;
        rw_info.p_paddr = 0;
        port->pstats.pcdev_write++;
        spin_unlock_irqrestore(&port->port_lock, flags);
        status = bsp_pcdev_ioctl(port->fd, PCDEV_IOCTL_WRITE_ASYNC, &rw_info);
        spin_lock_irqsave(&port->port_lock, flags);
        if (status) {
            port->pstats.pcdev_write_fail++;
        } else {
            port->write_started++;
            port->pstats.pcdev_write_len += len;
        }
    } while (!status);

    if (do_tty_wake && port->port.tty)
        tty_wakeup(port->port.tty);
    return status;
}

static void pcie_tty_write_complete(char *pVirAddr, char *pPhyAddr, int size)
{
    struct gs_port *port = g_ptty_ports[PCIE_CONSOLE_IDX].port;
    unsigned long flags;

    spin_lock_irqsave(&port->port_lock, flags);
    port->write_started--;
    port->pstats.pcdev_write_complete++;

    if (pVirAddr != NULL) {
        kfree(pVirAddr);
    }

    /* normal completion */
    pcie_tty_start_tx(port);

    spin_unlock_irqrestore(&port->port_lock, flags);
}

static void pcie_tty_read_complete(void)
{
    struct gs_port *port = port = g_ptty_ports[PCIE_CONSOLE_IDX].port;
    unsigned long flags;

    port->pstats.pcdev_read_complete++;

    spin_lock_irqsave(&port->port_lock, flags);
    /* we start rx in connect, so if usr not open the dev, drop the rx data */
    if (port->port.count) {
        port->read_complete++;
    }
    spin_unlock_irqrestore(&port->port_lock, flags);

    schedule_delayed_work(&g_console_ctx.read_work, 0);
}

static int rx_push_process(struct gs_port *port, pcdev_wr_async_info_s *rw_info,
    bool *do_push, unsigned long flags)
{
    char *packet = NULL;
    unsigned size, n;
    int count;
    int status;

    n = port->pstats.n_read;
    if (n) {
        port->pstats.last_rw_info_read++;
        packet = g_ptty_last_rw_info.p_vaddr + n;
        size = g_ptty_last_rw_info.size - n;
        rw_info->p_vaddr = g_ptty_last_rw_info.p_vaddr;
        rw_info->size = g_ptty_last_rw_info.size;
    } else {
        if (port->fd == NULL) {
            port->pstats.fd_null++;
            return -1;
        }

        port->pstats.pcdev_get_buf++;
        spin_unlock_irqrestore(&port->port_lock, flags);
        status = bsp_pcdev_ioctl(port->fd, PCDEV_IOCTL_GET_RD_BUFF, (void *)rw_info);
        spin_lock_irqsave(&port->port_lock, flags);
        if (status) {
            port->pstats.pcdev_get_buf_fail++;
            return -1;
        }
        packet = rw_info->p_vaddr;
        size = rw_info->size;
    }

    /* we may have pushed part of this packet already... */
    count = tty_insert_flip_string(&port->port, packet, size);
    if (count) {
        *do_push = true;
    }

    if (count != size) {
        /* stop pushing; TTY layer can't handle more */
        port->pstats.cannt_push++;
        port->pstats.n_read += count;
        g_ptty_last_rw_info.p_vaddr = rw_info->p_vaddr;
        g_ptty_last_rw_info.size = rw_info->size;
        bsp_err("%d: rx block %d/%d\n", port->port_num, count, rw_info->size);
        return -1;
    }
    return 0;
}

static void gs_rx_push_do_push(struct gs_port *port, struct tty_struct *tty, bool do_push)
{
    if (port->read_complete && (tty != NULL)) {
        if (!test_bit(TTY_THROTTLED, &tty->flags)) {
            if (do_push) {
                schedule_delayed_work(&g_console_ctx.read_work, 0);
            } else {
                bsp_err("%d: RX not scheduled?\n", port->port_num);
            }
        }
    }
}

static void gs_rx_push(struct work_struct *work)
{
    struct tty_struct *tty = NULL;
    bool do_push = false;
    int status;
    pcdev_wr_async_info_s rw_info;
    unsigned long flags;
    struct gs_port *port = g_ptty_ports[PCIE_CONSOLE_IDX].port;

    if (port == NULL)
        return;

    port->pstats.rx_push++;

    /* hand any queued data to the tty */
    spin_lock_irqsave(&port->port_lock, flags);
    tty = port->port.tty;
    while (port->read_complete) {
        status = rx_push_process(port, &rw_info, &do_push, flags);
        if (status) {
            break;
        }

        port->pstats.n_read = 0;
        spin_unlock_irqrestore(&port->port_lock, flags);
        status = bsp_pcdev_ioctl(port->fd, PCDEV_IOCTL_RETURN_BUFF, (void *)&rw_info);
        spin_lock_irqsave(&port->port_lock, flags);
        if (status) {
            port->pstats.pcdev_ret_buf_fail++;
        } else {
            port->pstats.pcdev_ret_buf++;
        }
        port->read_complete--;
    }

    /*
     * Push from tty to ldisc; without low_latency set this is handled by
     * a workqueue, so we won't get callbacks and can hold port_lock
     */
    if (do_push) {
        tty_flip_buffer_push(&port->port);
    }

    gs_rx_push_do_push(port, tty, do_push);
    spin_unlock_irqrestore(&port->port_lock, flags);
}

static int pcie_tty_open(struct tty_struct *tty, struct file *file)
{
    int port_num;
    struct gs_port *port = NULL;
    int status = 0;
    unsigned long flags;

    if (tty == NULL || tty->index >= MAX_PCIE_TTY_PORTS) {
        bsp_err("tty is null or invalid index\n");
        return -1;
    }

    port_num = tty->index;
    port = g_ptty_ports[port_num].port;

    spin_lock_irqsave(&port->port_lock, flags);

    port->pstats.open_count++;
    if (port->pstats.openclose == true) {
        bsp_err("pcie_tty_open already open\n");
        spin_unlock_irqrestore(&port->port_lock, flags);
        return 0;
    }

    /* allocate circular buffer on first open */
    if (port->port_write_buf.buf_buf == NULL) {
        status = gs_buf_alloc(&port->port_write_buf, WRITE_BUF_SIZE);
        if (status) {
            bsp_err("gs_open: ttyGS%d no buffer\n", port->port_num);
            port->pstats.openclose = false;
            goto exit_unlock_port;
        }
    }

    /*
     * REVISIT if REMOVED (ports[].port NULL), abort the open
     * to let rmmod work faster (but this way isn't wrong).
     * REVISIT maybe wait for "carrier detect" 
     */
    tty->driver_data = port;
    port->port.tty = tty;

    port->port.count = 1;
    port->pstats.openclose = true;

exit_unlock_port:
    spin_unlock_irqrestore(&port->port_lock, flags);
    return status;
}

static void pcie_tty_close(struct tty_struct *tty, struct file *file)
{
    struct gs_port *port = tty->driver_data;
    unsigned long flags;

    spin_lock_irqsave(&port->port_lock, flags);
    port->pstats.close_count++;
    if (port->pstats.openclose == false) {
        spin_unlock_irqrestore(&port->port_lock, flags);
        return;
    }

    port->port.count = 0;
    port->pstats.openclose = false;

    gs_buf_free(&port->port_write_buf);

    tty->driver_data = NULL;
    port->port.tty = NULL;

    spin_unlock_irqrestore(&port->port_lock, flags);
    return;
}

static int pcie_tty_write(struct tty_struct *tty, const unsigned char *buf, int count)
{
    struct gs_port *port = tty->driver_data;
    unsigned long flags;
    int status;
    int i = 1000000;

    port->pstats.tty_write++;

    spin_lock_irqsave(&port->port_lock, flags);
    if (unlikely(!port->port.count)) {
        port->pstats.tty_write_count_null++;
        spin_unlock_irqrestore(&port->port_lock, flags);
        return -EINVAL;
    }

    if (count > port->pstats.tty_write_max) {
        port->pstats.tty_write_max = count;
    }

    if (count) {
        count = gs_buf_put(&port->port_write_buf, buf, count, 1);
    }

    if (port->fd != NULL) {
        status = pcie_tty_start_tx(port);
        while (i) {
            i--;
        }
    } else {
        port->pstats.fd_null++;
    }

    spin_unlock_irqrestore(&port->port_lock, flags);

    return count;
}

static int gs_put_char(struct tty_struct *tty, unsigned char ch)
{
    struct gs_port *port = tty->driver_data;
    unsigned long flags;
    int status;

    spin_lock_irqsave(&port->port_lock, flags);
    status = gs_buf_put(&port->port_write_buf, &ch, 1, 1);
    spin_unlock_irqrestore(&port->port_lock, flags);

    return status;
}

static void gs_flush_chars(struct tty_struct *tty)
{
    struct gs_port *port = tty->driver_data;
    unsigned long flags;

    spin_lock_irqsave(&port->port_lock, flags);
    pcie_tty_start_tx(port);
    spin_unlock_irqrestore(&port->port_lock, flags);
}

static int gs_write_room(struct tty_struct *tty)
{
    struct gs_port *port = tty->driver_data;
    unsigned long flags;
    /*
     * if usb not connect, return room available;
     * to avoid thread in sleep
     */
    int room = WRITE_BUF_SIZE - 1;

    spin_lock_irqsave(&port->port_lock, flags);
    if (port->pstats.openclose) {
        room = gs_buf_space_avail(&port->port_write_buf, 1);
    }
    spin_unlock_irqrestore(&port->port_lock, flags);

    return room;
}

static int gs_chars_in_buffer(struct tty_struct *tty)
{
    struct gs_port *port = tty->driver_data;
    unsigned long flags;
    int chars;

    spin_lock_irqsave(&port->port_lock, flags);
    chars = gs_buf_data_avail(&port->port_write_buf);
    spin_unlock_irqrestore(&port->port_lock, flags);

    return chars;
}

/* undo side effects of setting TTY_THROTTLED */
static void gs_unthrottle(struct tty_struct *tty)
{
    struct gs_port *port = NULL;
    unsigned long flags;

    if (tty == NULL) {
        bsp_err("tty is null\n");
        return;
    }

    port = tty->driver_data;
    spin_lock_irqsave(&port->port_lock, flags);
    if (port->pstats.openclose) {
        /*
         * Kickstart read queue processing.  We don't do xon/xoff,
         * rts/cts, or other handshaking with the host, but if the
         * read queue backs up enough we'll be NAKing OUT packets.
         */
        schedule_delayed_work(&g_console_ctx.read_work, 0);
        bsp_err("%d: unthrottle\n", port->port_num);
    }
    spin_unlock_irqrestore(&port->port_lock, flags);
}

static const struct tty_operations g_pcie_tty_ops = {
    .open = pcie_tty_open,
    .close = pcie_tty_close,
    .write = pcie_tty_write,
    .put_char = gs_put_char,
    .flush_chars = gs_flush_chars,
    .write_room = gs_write_room,
    .chars_in_buffer = gs_chars_in_buffer,
    .unthrottle = gs_unthrottle,
};

static int pcie_tty_port_alloc(unsigned port_num)
{
    struct gs_port *port = NULL;
    int ret = 0;

    bsp_info(":<%s> in\n", __func__);

    mutex_lock(&g_ptty_ports[port_num].lock);
    if (g_ptty_ports[port_num].port) {
        ret = -EBUSY;
        goto out;
    }

    port = kzalloc(sizeof(struct gs_port), GFP_KERNEL);
    if (port == NULL) {
        ret = -ENOMEM;
        goto out;
    }

    tty_port_init(&port->port);
    spin_lock_init(&port->port_lock);
    INIT_DELAYED_WORK(&g_console_ctx.read_work, gs_rx_push);

    port->port_num = port_num;
    port->maxpacket = WRITE_BUF_SIZE;
    port->name = g_pcie_tty_type_table[port_num].name;

    g_ptty_ports[port_num].port = port;
out:
    mutex_unlock(&g_ptty_ports[port_num].lock);
    return ret;
}

static void pcie_tty_free_port(struct gs_port *port)
{
    bsp_info(":<%s> in\n", __func__);
    tty_port_destroy(&port->port);
    kfree(port);
}

#ifdef CONFIG_PCIE_CDEV_DBG_USB_SHELL

static int bsp_usb_is_support_shell(void)
{
    return (int)(!!(g_usb_nv.usb_feature.flags & USB_CONSOLE_SHELL));
}

static inline int gs_acm_is_console_enable(void)
{
    return bsp_usb_is_support_shell();
}

static void bsp_usb_nv_init(void)
{
    if (g_usb_nv.stat_nv_init) {
        return;
    }

    /*
     * read needed usb nv items from nv
     * ignore the nv_read return value,
     * if fail, use the default value
     */
    if (bsp_nvm_read(NV_ID_DRV_USB_FEATURE, (u8 *)&g_usb_nv.usb_feature, sizeof(g_usb_nv.usb_feature))) {
        g_usb_nv.stat_nv_read_fail++;
    }

    /* SLT default is super speed */
    g_usb_nv.stat_nv_init++;
}

#else

static inline int gs_acm_is_console_enable(void)
{
    return 0;
}

static void bsp_usb_nv_init(void)
{

}

#endif

void tty_drvier_init(void)
{
    g_pcie_tty_driver = alloc_tty_driver(MAX_PCIE_TTY_PORTS);
    if (g_pcie_tty_driver == NULL) {
        return;
    }

    g_pcie_tty_driver->driver_name = "g_pcie_tty";
    g_pcie_tty_driver->name = g_pcie_tty_type_table[PCIE_CONSOLE_IDX].name;

    g_pcie_tty_driver->type = TTY_DRIVER_TYPE_SERIAL;
    g_pcie_tty_driver->subtype = SERIAL_TYPE_NORMAL;
    g_pcie_tty_driver->flags = TTY_DRIVER_REAL_RAW | TTY_DRIVER_DYNAMIC_DEV;
    g_pcie_tty_driver->init_termios = tty_std_termios;

    /*
     * 9600-8-N-1 ... matches defaults expected by "usbser.sys" on
     * MS-Windows.  Otherwise, most of these flags shouldn't affect
     * anything unless we were to actually hook up to a serial line.
     */
    g_pcie_tty_driver->init_termios.c_cflag = B9600 | CS8 | CREAD | HUPCL | CLOCAL;
    g_pcie_tty_driver->init_termios.c_ispeed = PCIE_TTY_SPEED;
    g_pcie_tty_driver->init_termios.c_ospeed = PCIE_TTY_SPEED;
}

int pcie_tty_init(void)
{
    unsigned i;
    int ret;
    struct device *tty_dev = NULL;
    struct gs_port *port = NULL;

    bsp_err("[init]start\n");

    memset_s(g_ptty_ports, sizeof(struct portctl) * MAX_PCIE_TTY_PORTS, 0, sizeof(struct portctl) * MAX_PCIE_TTY_PORTS);
    tty_drvier_init();

    tty_set_operations(g_pcie_tty_driver, &g_pcie_tty_ops);
    for (i = 0; i < MAX_PCIE_TTY_PORTS; i++)
        mutex_init(&g_ptty_ports[i].lock);

    /* export the driver ... */
    ret = tty_register_driver(g_pcie_tty_driver);
    if (ret) {
        bsp_err(":<%s> cannot register, err %d\n", __func__, ret);
        goto fail;
    }

    ret = pcie_tty_port_alloc(PCIE_CONSOLE_IDX);
    if (ret) {
        goto fail;
    }

    port = g_ptty_ports[PCIE_CONSOLE_IDX].port;

    bsp_info(":<%s> pcie_tty_port_alloc succ\n", __func__);

    tty_dev = tty_port_register_device(&port->port, g_pcie_tty_driver, PCIE_CONSOLE_IDX, NULL);
    if (IS_ERR(tty_dev)) {
        bsp_err("%s: failed to register tty for port %d, err %ld\n", __func__, PCIE_CONSOLE_IDX, PTR_ERR(tty_dev));

        ret = PTR_ERR(tty_dev);
        g_ptty_ports[PCIE_CONSOLE_IDX].port = NULL;
        pcie_tty_free_port(port);
        goto fail;
    }

    bsp_usb_nv_init();

    if (gs_acm_is_console_enable()) {
        /* reg from uart shell to usb shell */
        bsp_info(":<%s> register_console\n", __func__);

        register_console(&g_pcie_tty_console);
        console_start(&g_pcie_tty_console);
    }

    bsp_err("[init]ok\n");

    return ret;

fail:
    put_tty_driver(g_pcie_tty_driver);
    g_pcie_tty_driver = NULL;
    return ret;
}

void pcie_tty_init_cb(void)
{
    struct gs_port *port = NULL;
    bsp_info(":<%s> in\n", __func__);

    port = g_ptty_ports[PCIE_CONSOLE_IDX].port;

    port->fd = bsp_pcdev_open(pcdev_a_shell);
    if (port->fd == NULL) {
        bsp_err("pcie_tty_open open failed\n");
        return;
    }

    bsp_pcdev_ioctl(port->fd, PCDEV_IOCTL_SET_WRITE_CB, pcie_tty_write_complete);
    bsp_pcdev_ioctl(port->fd, PCDEV_IOCTL_SET_READ_CB, pcie_tty_read_complete);
    return;
}

void pcie_tty_disable_cb(void)
{
    struct gs_port *port;

    port = g_ptty_ports[PCIE_CONSOLE_IDX].port;

    bsp_pcdev_close(port->fd);
    return;
}

static int ptty_init(void)
{
    bsp_info(":<%s> in\n", __func__);
    bsp_pcdev_reg_enablecb(pcie_tty_init_cb);
    bsp_pcdev_reg_disablecb(pcie_tty_disable_cb);
    pcie_tty_init();
    return 0;
}
module_init(ptty_init);
