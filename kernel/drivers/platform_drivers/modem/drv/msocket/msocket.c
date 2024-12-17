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

#include <linux/mm.h>
#include <linux/socket.h>
#include <linux/file.h>
#include <linux/net.h>
#include <linux/interrupt.h>
#include <linux/thread_info.h>
#include <linux/rcupdate.h>
#include <linux/netdevice.h>
#include <net/net_namespace.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/mutex.h>
#include <linux/init.h>
#include <linux/poll.h>
#include <linux/cache.h>
#include <linux/module.h>
#include <linux/highmem.h>
#include <linux/mount.h>
#include <linux/security.h>
#include <linux/syscalls.h>
#include <linux/compat.h>
#include <linux/kmod.h>
#include <linux/audit.h>
#include <linux/wireless.h>
#include <linux/nsproxy.h>
#include <linux/magic.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <asm/unistd.h>
#include <net/compat.h>
#include <net/wext.h>
#include <net/cls_cgroup.h>
#include <net/sock.h>
#include <linux/netfilter.h>
#include <linux/if_tun.h>
#include <linux/ipv6_route.h>
#include <linux/route.h>
#include <linux/sockios.h>
#include <linux/atalk.h>
#include <linux/time.h>
#include <linux/sched.h>
#include <linux/ktime.h>
#include <linux/wait.h>
#include <linux/kernel.h>
#include <linux/personality.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <asm/processor.h>
#include <asm/uaccess.h>
#include <linux/in.h>

#include <linux/inet.h>
#include <linux/version.h>

#include <securec.h>
#include <product_config.h>
#include <osl_types.h>
#include <mdrv_socket.h>
#include <bsp_print.h>
#include <bsp_dump.h>
#include "msocket.h"

static int g_socket_debug = DISABLE_SOCKET_DEBUG;

static struct hi_fs_table g_hifstable;

struct socket_dbg_table g_sockdbg;
struct ctl_table_header *g_hisock_ctlheader = NULL;
static struct file g_file_stub;

#define hi_get_sock(fd) g_hifstable.hi_socks[fd].sock

void hi_sock_show(void)
{
    bsp_err("sock:  %u", atomic_read(&g_sockdbg.socket_dbg_cnt.sock));
    bsp_err("bind:  %u", atomic_read(&g_sockdbg.socket_dbg_cnt.bind));
    bsp_err("listen:  %u", atomic_read(&g_sockdbg.socket_dbg_cnt.listen));
    bsp_err("select_in:  %u", atomic_read(&g_sockdbg.socket_dbg_cnt.select_in));
    bsp_err("select_out:  %u", atomic_read(&g_sockdbg.socket_dbg_cnt.select_out));
    bsp_err("accept_in:  %u", atomic_read(&g_sockdbg.socket_dbg_cnt.accept_in));
    bsp_err("accept_out:  %u", atomic_read(&g_sockdbg.socket_dbg_cnt.accept_out));
    bsp_err("send:  %u", atomic_read(&g_sockdbg.socket_dbg_cnt.send));
    bsp_err("recv_in:  %u", atomic_read(&g_sockdbg.socket_dbg_cnt.recv_in));
    bsp_err("recv_out:  %u", atomic_read(&g_sockdbg.socket_dbg_cnt.recv_out));
    bsp_err("shutdown:  %u", atomic_read(&g_sockdbg.socket_dbg_cnt.shutdown));
    bsp_err("close:  %u", atomic_read(&g_sockdbg.socket_dbg_cnt.close));
}

/* register a socket to table,set reference count to 1 */
void bsp_sock_fd_zero(sk_fd_set *set)
{
    int ret;

    if (set != NULL) {
        ret = memset_s(set, sizeof(sk_fd_set), 0, sizeof(*set));
        if (ret) {
            bsp_err("memset_s failed\n");
        }
    }
}

static int hi_sock_reg_fd(struct socket *sock)
{
    int i = -1;

    mutex_lock(&g_hifstable.hi_mutex);

    if (g_hifstable.used == HI_SOCKET_MAX) {
        bsp_err("sock_reg fail, sock used full\n");
        goto fail;
    }

    for (i = 0; i < HI_SOCKET_MAX; i++) {
        if (!atomic_read(&g_hifstable.hi_socks[i].cnt)) {
            break;
        }
    }

    if (i >= HI_SOCKET_MAX) {
        i = -1;
        goto fail;
    }

    sock->file = &g_file_stub;
    g_hifstable.hi_socks[i].sock = sock;
    atomic_set(&g_hifstable.hi_socks[i].cnt, 1);

    g_hifstable.used++;

    atomic_inc(&g_sockdbg.socket_dbg_cnt.sockregfd);
fail:
    mutex_unlock(&g_hifstable.hi_mutex);

    return i;
}

static int hi_sock_unreg_fd(int fd)
{
    if (fd > HI_SOCKET_MAX - 1 || fd < 0) {
        bsp_err("sock_unreg fail,unreg fd is illegal !\n");
        return -1;
    }

    mutex_lock(&g_hifstable.hi_mutex);

    g_hifstable.used--;

    atomic_set(&g_hifstable.hi_socks[fd].cnt, 0);

    g_hifstable.hi_socks[fd].sock = 0;

    atomic_inc(&g_sockdbg.socket_dbg_cnt.sockunregfd);

    mutex_unlock(&g_hifstable.hi_mutex);

    return 1;
}

static int hi_sock_get_fd(int fd)
{
    int err = -1;

    if (fd > HI_SOCKET_MAX - 1 || fd < 0) {
        bsp_err("sock_getfd fail, fd = %d\n", fd);
        return err;
    }

    if (likely(atomic_inc_not_zero(&g_hifstable.hi_socks[fd].cnt)))
        err = 0;

    atomic_inc(&g_sockdbg.socket_dbg_cnt.sockgetfd);

    return err;
}

static int hi_sock_put_fd(int fd)
{
    struct socket *sock = NULL;

    int err = -1;

    if (fd > HI_SOCKET_MAX - 1 || fd < 0) {
        bsp_err("sock_putfd fail!fd = %d\n", fd);
        return err;
    }

    /* no use */
    if (unlikely(atomic_dec_and_test(&g_hifstable.hi_socks[fd].cnt))) {
        sock = g_hifstable.hi_socks[fd].sock;

        err = hi_sock_unreg_fd(fd);
        sock->file = NULL;

        sock_release(sock);
    }
    atomic_inc(&g_sockdbg.socket_dbg_cnt.sockputfd);

    return 1;
}

int bsp_socket(int family, int type, int protocol)
{
    struct socket *sock = NULL;
    int retval;

    atomic_inc(&g_sockdbg.socket_dbg_cnt.sock);

    retval = sock_create_kern(&init_net, family, type, protocol, &sock);
    if (retval < 0) {
        bsp_err("create sock fail!\n ");
        goto out;
    }

    retval = hi_sock_reg_fd(sock);
    if (retval < 0) {
        goto out_release;
    }

out:
    return retval;

out_release:
    sock_release(sock);
    return retval;
}

int bsp_bind(int fd, struct sockaddr *addr, int addrlen)
{
    struct socket *sock = NULL;

    int retval = -1;

    atomic_inc(&g_sockdbg.socket_dbg_cnt.bind);

    if (hi_sock_get_fd(fd) < 0) {
        return retval;
    }

    sock = hi_get_sock(fd);

    retval = kernel_bind(sock, addr, addrlen);
    if (retval < 0) {
        bsp_err("bind fail!\n ");
    }

    hi_sock_put_fd(fd);

    return retval;
}

int bsp_listen(int fd, int backlog)
{
    struct socket *sock = NULL;
    int retval = -1;

    atomic_inc(&g_sockdbg.socket_dbg_cnt.listen);

    if (hi_sock_get_fd(fd) < 0) {
        return retval;
    }

    sock = hi_get_sock(fd);

    retval = kernel_listen(sock, backlog);
    if (retval < 0) {
        bsp_err("listen fail !\n");
    }

    hi_sock_put_fd(fd);

    return retval;
}

static int hisk_get_newsocket(int fd, struct socket **aptsock, struct socket **newsock)
{
    int err = -1;
    int newfd;
    struct socket *sock = NULL;
    struct socket *newsk = NULL;
    struct sock *sk = NULL;

    atomic_inc(&g_sockdbg.socket_dbg_cnt.accept_in);

    if (hi_sock_get_fd(fd) < 0) {
        goto fail;
    }

    sock = hi_get_sock(fd);

    sk = sock->sk;
    err = sock_create_lite(sk->sk_family, sk->sk_type, sk->sk_protocol, &newsk);
    if (err < 0) {
        goto put_fd;
    }

    newfd = hi_sock_reg_fd(newsk);
    if (newfd < 0) {
        err = newfd;
        goto release_sock;
    }

    *aptsock = sock;
    *newsock = newsk;
    return newfd;

release_sock:
    sock_release(newsk);
put_fd:
    hi_sock_put_fd(fd);
fail:
    atomic_inc(&g_sockdbg.socket_dbg_cnt.accept_out);
    return err;
}

static int hisk_get_upeeraddr(struct socket *sock, struct sockaddr *upskaddr, int *upaddrlen)
{
    int err, len;
    struct sockaddr_storage address;

    if (upskaddr == NULL) {
       bsp_err("para error, upskaddr = NULL\n");
        return 0;
    }
    /* kernel 5.10: return value is actually length of sock name, len < 0 means get name failed */
    len = sock->ops->getname(sock, (struct sockaddr *)&address, 0x2);
    if (len < 0) {
        bsp_err("kernel getname fail, len=%d\n", len);
        return len;
    }

    if (*upaddrlen < len) {
        bsp_err("addrlen is longer than given len, len=%d, upaddrlen=%d\n", len, *upaddrlen);
        return -1;
    }

    err = memcpy_s(upskaddr, *upaddrlen, &address, len);
    if (err != EOK) {
        bsp_err("memcpy_s failed!\n");
        return err;
    }
    *upaddrlen = len;

    return 0;
}

int bsp_accept(int fd, struct sockaddr *upeer_sockaddr, int *upeer_addrlen)
{
    int err;
    int newfd;
    struct socket *newsock = NULL;
    struct socket *sock = NULL;

    newfd = hisk_get_newsocket(fd, &sock, &newsock);
    if (newfd < 0) {
        bsp_err("newsock get fail\n");
        return newfd;
    }

    err = sock->ops->accept(sock, newsock, O_NONBLOCK, true);
    if (err < 0) {
        bsp_err("kernel accept fail!\n");
        goto release_newfd;
    }

    newsock->ops = sock->ops;
    err = hisk_get_upeeraddr(newsock, upeer_sockaddr, upeer_addrlen);
    if (err < 0) {
        bsp_err("hisk get upeer address fail\n");
        goto release_newfd;
    }

    hi_sock_put_fd(fd);
    atomic_inc(&g_sockdbg.socket_dbg_cnt.accept_out);
    return newfd;

release_newfd:
    hi_sock_unreg_fd(newfd);
    sock_release(newsock);
    hi_sock_put_fd(fd);
    atomic_inc(&g_sockdbg.socket_dbg_cnt.accept_out);
    return err;
}

int bsp_connect(int fd, struct sockaddr *addr, int addrlen)
{
    struct socket *sock = NULL;
    int err = -1;

    atomic_inc(&g_sockdbg.socket_dbg_cnt.connect_in);

    if (hi_sock_get_fd(fd) < 0) {
        atomic_inc(&g_sockdbg.socket_dbg_cnt.connect_out);
        return err;
    }

    sock = hi_get_sock(fd);

    err = kernel_connect(sock, addr, addrlen, 0);
    if (err < 0) {
        bsp_err("kernel connect fail!\n");
    }

    hi_sock_put_fd(fd);
    atomic_inc(&g_sockdbg.socket_dbg_cnt.connect_out);

    return err;
}

static int hi_poll_select_copy_remaining(struct timespec64 *end_time, unsigned char *p, int timeval, int ret)
{
    struct timespec64 rts;
    struct __kernel_timex_timeval rtv;
    int err;

    if (!p) {
        return ret;
    }

    if (current->personality & STICKY_TIMEOUTS) {
        goto sticky;
    }

    if (!end_time->tv_sec && !end_time->tv_nsec) {
        return ret;
    }

    ktime_get_real_ts64(&rts);
    rts = timespec64_sub(*end_time, rts);
    if (rts.tv_sec < 0) {
        rts.tv_sec = rts.tv_nsec = 0;
    }

    if (timeval) {
        if (sizeof(rtv) > sizeof(rtv.tv_sec) + sizeof(rtv.tv_usec)) {
            err = memset_s(&rtv, sizeof(rtv), 0, sizeof(rtv));
            if (err != EOK) {
                bsp_err("memset_s fail\n");
                return ret;
            }
        }
        rtv.tv_sec = rts.tv_sec;
        rtv.tv_usec = rts.tv_nsec / NSEC_PER_USEC;

        err = memcpy_s(p, sizeof(struct __kernel_timex_timeval), &rtv, sizeof(rtv));
        if (err != EOK) {
            bsp_err("memcpy_s fail\n");
            return ret;
        }
    }

sticky:
    if (ret == -ERESTARTNOHAND)
        ret = -EINTR;
    return ret;
}

static int __hi_estimate_accuracy(struct timespec64 *tv)
{
    int slack;
    int divfactor = 1000;

    if (tv->tv_sec < 0)
        return 0;

    if (task_nice(current) > 0)
        divfactor = divfactor / 0x5;

    if (tv->tv_sec > HI_MAX_SLACK / (NSEC_PER_SEC / divfactor))
        return HI_MAX_SLACK;

    slack = tv->tv_nsec / divfactor;
    slack += tv->tv_sec * (NSEC_PER_SEC / divfactor);

    if (slack > HI_MAX_SLACK)
        return HI_MAX_SLACK;

    return slack;
}

static int hi_estimate_accuracy(struct timespec64 *tv)
{
    unsigned int ret;
    struct timespec64 now;

    ktime_get_real_ts64(&now);
    now = timespec64_sub(*tv, now);
    ret = __hi_estimate_accuracy(&now);
    if (ret < current->timer_slack_ns)
        return current->timer_slack_ns;

    return ret;
}

static inline void hi_wait_key_set(poll_table *wait, unsigned int in, unsigned int out, unsigned int bit)
{
    if (wait) {
        wait->_key = HI_POLLEX_SET;
        if (in & bit)
            wait->_key |= HI_POLLIN_SET;
        if (out & bit)
            wait->_key |= HI_POLLOUT_SET;
    }
}

int __hi_pollwake(wait_queue_entry_t *wait, unsigned mode, int sync, void *key)
{
    struct hi_poll_wqueues *pwq = wait->private;
    DECLARE_WAITQUEUE(dummy_wait, pwq->polling_task);

    smp_wmb();
    pwq->triggered = 1;

    return default_wake_function(&dummy_wait, mode, sync, key);
}

int hi_pollwake(wait_queue_entry_t *wait, unsigned mode, int sync, void *key)
{
    struct hi_poll_table_entry *entry;

    entry = container_of(wait, struct hi_poll_table_entry, wait);

    if ((unsigned long long)(uintptr_t)key && !((unsigned long long)(uintptr_t)key & entry->key)) {
        return 0;
    }
    return __hi_pollwake(wait, mode, sync, key);
}

#define HISK_POLL_MAX 32
struct hi_poll_table_entry *hi_poll_get_entry(struct hi_poll_wqueues *p)
{
    if (p->inline_index < HISK_POLL_MAX) {
        return p->inline_entries + p->inline_index++;
    } else {
        return NULL;
    }
}

void __hi_pollwait(struct file *filp, wait_queue_head_t *wait_address, poll_table *p)
{
    struct hi_poll_wqueues *pwq = container_of(p, struct hi_poll_wqueues, pt);
    struct hi_poll_table_entry *entry = hi_poll_get_entry(pwq);

    if (!entry) {
        return;
    }
    entry->wait_address = wait_address;

    entry->key = p->_key;

    init_waitqueue_func_entry(&entry->wait, hi_pollwake);
    entry->wait.private = pwq;
    add_wait_queue(wait_address, &entry->wait);

    return;
}

void hi_poll_initwait(struct hi_poll_wqueues *pwq)
{
    init_poll_funcptr(&pwq->pt, __hi_pollwait);
    pwq->polling_task = current;
    pwq->triggered = 0;
    pwq->error = 0;
    pwq->inline_index = 0;
}

int hi_poll_schedule_timeout(struct hi_poll_wqueues *pwq, int state, ktime_t *expires, unsigned int slack)
{
    int rc = -EINTR;
    /*lint -e446 -e666*/
    set_current_state(state);
    /*lint +e446 +e666*/
    if (!pwq->triggered) {
        rc = schedule_hrtimeout_range(expires, slack, HRTIMER_MODE_ABS);
    }

    __set_current_state(TASK_RUNNING);
    smp_store_mb(pwq->triggered, 0);
    return rc;
}

static void hi_free_poll_entry(struct hi_poll_table_entry *entry)
{
    remove_wait_queue(entry->wait_address, &entry->wait);
}

void hi_poll_freewait(struct hi_poll_wqueues *pwq)
{
    int i;
    for (i = 0; i < pwq->inline_index; i++)
        hi_free_poll_entry(pwq->inline_entries + i);
}

static void batch_put_fd(int n, int *getfd)
{
    int i;
    for (i = 0; i < n; i++) {
        while (getfd[i]) {
            hi_sock_put_fd(i);
            getfd[i]--;
        }
    }
    return;
}

static void hi_do_select_traverse(int n, struct hi_fd_set_bits *fds, int *retval, poll_table *wait, int *getfd)
{
    unsigned int i, mask, all_bits;
    unsigned int bit = 1;
    unsigned int res_in = 0;
    unsigned int res_out = 0;
    unsigned int res_ex = 0;
    struct socket *sock = NULL;

    unsigned int in = fds->in;
    unsigned int out = fds->out;
    unsigned int ex = fds->ex;

    unsigned int *rinp = &(fds->res_in);
    unsigned int *routp = &(fds->res_out);
    unsigned int *rexp = &(fds->res_ex);

    for (i = 0; i < (unsigned int)n; i++) {
        bit = 1UL << i;

        all_bits = in | out | ex;

        if (((all_bits & bit) == 0) || (hi_sock_get_fd(i) < 0)) {
            continue;
        }

        getfd[i]++;

        sock = hi_get_sock(i);

        mask = DEFAULT_POLLMASK;

        hi_wait_key_set(wait, in, out, bit);
        if (sock->ops->poll) {
            mask = sock->ops->poll(&g_file_stub, sock, wait);
        }

        if ((mask & HI_POLLIN_SET) && (in & bit)) {
            res_in |= bit;
            *rinp = res_in;
            (*retval)++;
            wait = NULL;
        }
        if ((mask & HI_POLLOUT_SET) && (out & bit)) {
            res_out |= bit;
            *routp = res_out;
            (*retval)++;
            wait = NULL;
        }
        if ((mask & HI_POLLEX_SET) && (ex & bit)) {
            res_ex |= bit;
            *rexp = res_ex;
            (*retval)++;
            wait = NULL;
        }

        cond_resched();
    }
}

int hi_do_select(int n, struct hi_fd_set_bits *fds, struct timespec64 *end_time)
{
    ktime_t expire;
    ktime_t *to = NULL;
    struct hi_poll_wqueues *table = NULL;
    poll_table *wait = NULL;
    int retval;
    int timed_out = 0;
    unsigned int slack = 0;
    int getfd[HI_SOCKET_MAX] = {0};

    table = (struct hi_poll_wqueues *)kzalloc(sizeof(struct hi_poll_wqueues), GFP_KERNEL);
    if (!table) {
        return -ENOMEM;
    }

    hi_poll_initwait(table);
    wait = &table->pt;

    if (end_time && !end_time->tv_sec && !end_time->tv_nsec) {
        wait = NULL;
        timed_out = 1;
    }

    if (end_time && !timed_out) {
        slack = hi_estimate_accuracy(end_time);
    }

    retval = 0;
    for (;;) {
        hi_do_select_traverse(n, fds, &retval, wait, getfd);

        wait = NULL;
        if (retval || timed_out) {
            break;
        }

        if (table->error) {
            retval = table->error;
            break;
        }

        if (end_time && !to) {
            expire = timespec64_to_ktime(*end_time);
            to = &expire;
        }

        if (!hi_poll_schedule_timeout(table, TASK_INTERRUPTIBLE, to, slack))
            timed_out = 1;
    }

    hi_poll_freewait(table);

    kfree((const void *)table);
    table = NULL;

    batch_put_fd(n, getfd);

    return retval;
}

int hi_core_sys_select(int n, sk_fd_set *inp, sk_fd_set *outp, sk_fd_set *exp_core, struct timespec64 *end_time)
{
    int ret;
    struct hi_fd_set_bits fd_set_bits_socket;

    ret = -EINVAL;
    if (n < 0) {
        return ret;
    }

    if (n > HI_SOCKET_MAX) {
        n = HI_SOCKET_MAX;
    }

    /* default 0 */
    ret = memset_s(&fd_set_bits_socket, sizeof(fd_set_bits_socket), 0, sizeof(fd_set_bits_socket));
    if (ret != EOK) {
        bsp_err("memset_s fail\n");
        return ret;
    }

    if (inp) {
        fd_set_bits_socket.in = inp->fds_bits;
    }
    if (outp) {
        fd_set_bits_socket.out = outp->fds_bits;
    }
    if (exp_core) {
        fd_set_bits_socket.ex = exp_core->fds_bits;
    }

    /* do */
    ret = hi_do_select(n, &fd_set_bits_socket, end_time);

    if (inp)
        inp->fds_bits = fd_set_bits_socket.res_in;
    if (outp)
        outp->fds_bits = fd_set_bits_socket.res_out;
    if (exp_core)
        exp_core->fds_bits = fd_set_bits_socket.res_ex;

    return ret;
}

struct timespec64 hi_timespec_add_safe(const struct timespec64 lhs, const struct timespec64 rhs)
{
    struct timespec64 res;

    set_normalized_timespec64(&res, lhs.tv_sec + rhs.tv_sec, lhs.tv_nsec + rhs.tv_nsec);

    if (res.tv_sec < lhs.tv_sec || res.tv_sec < rhs.tv_sec)
        res.tv_sec = TIME64_MAX;

    return res;
}

int hi_poll_select_set_timeout(struct timespec64 *to, int sec, int nsec)
{
    struct timespec64 ts = {
        .tv_sec = sec,
        .tv_nsec = nsec
    };

    if (!timespec64_valid(&ts))
        return -EINVAL;

    if (!sec && !nsec) {
        to->tv_sec = to->tv_nsec = 0;
    } else {
        ktime_get_real_ts64(to);
        *to = hi_timespec_add_safe(*to, ts);
    }
    return 0;
}

int bsp_select(int n, sk_fd_set *inp, sk_fd_set *outp, sk_fd_set *exp_fdset, struct __kernel_timex_timeval *tvp)
{
    struct timespec64 end_time, *to = NULL;
    struct __kernel_timex_timeval tv;
    int ret, err;

    atomic_inc(&g_sockdbg.socket_dbg_cnt.select_in);

    if (tvp) {
        err = memcpy_s(&tv, sizeof(struct __kernel_timex_timeval), tvp, sizeof(tv));
        if (err != EOK) {
            bsp_err("memcpy_s fail\n");
            atomic_inc(&g_sockdbg.socket_dbg_cnt.select_out);
            return -1;
        }
        to = &end_time;

        if (hi_poll_select_set_timeout(to, tv.tv_sec + (tv.tv_usec / USEC_PER_SEC),
            (tv.tv_usec % USEC_PER_SEC) * NSEC_PER_USEC)) {
            atomic_inc(&g_sockdbg.socket_dbg_cnt.select_out);
            return -EINVAL;
        }
    }

    ret = hi_core_sys_select(n, inp, outp, exp_fdset, to);
    ret = hi_poll_select_copy_remaining(&end_time, (unsigned char *)tvp, 1, ret);

    atomic_inc(&g_sockdbg.socket_dbg_cnt.select_out);

    return ret;
}

int bsp_skclose(int fd)
{
    int ret;

    atomic_inc(&g_sockdbg.socket_dbg_cnt.close);

    ret = hi_sock_put_fd(fd);

    return ret;
}

int bsp_send(int fd, void *buff, size_t len, int flags)
{
    struct socket *sock = NULL;
    struct msghdr msg;
    struct kvec iov;
    int err = -1;

    atomic_inc(&g_sockdbg.socket_dbg_cnt.send);

    if (hi_sock_get_fd(fd) < 0) {
        goto out;
    }

    if (buff == NULL) {
        bsp_err("send buff is NULL\n");
        return -1;
    }
    sock = hi_get_sock(fd);

    sock->file = &g_file_stub;
    sock->file->f_flags = 0;
    sock->file->f_flags |= (unsigned int)flags;

    iov.iov_base = buff;
    iov.iov_len = len;

    msg.msg_name = NULL;
    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    msg.msg_namelen = 0;
    msg.msg_flags = flags;

    err = kernel_sendmsg(sock, &msg, &iov, 1, len);

    hi_sock_put_fd(fd);

out:
    return err;
}

int bsp_recv(int fd, void *ubuf, size_t size, int flags)
{
    struct socket *sock = NULL;
    struct kvec iov;
    struct msghdr msg;
    int err = -1;

    atomic_inc(&g_sockdbg.socket_dbg_cnt.recv_in);

    if (hi_sock_get_fd(fd) < 0) {
        goto out;
    }

    if (ubuf == NULL) {
        bsp_err("recv buff is NULL\n");
        atomic_inc(&g_sockdbg.socket_dbg_cnt.recv_out);
        return -1;
    }
    sock = hi_get_sock(fd);

    sock->file = &g_file_stub;
    sock->file->f_flags = 0;
    sock->file->f_flags |= (unsigned int)flags;

    iov.iov_len = size;
    iov.iov_base = ubuf;

    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    msg.msg_name = NULL;
    msg.msg_namelen = 0;

    err = kernel_recvmsg(sock, &msg, &iov, 1, size, flags);

    hi_sock_put_fd(fd);

out:
    atomic_inc(&g_sockdbg.socket_dbg_cnt.recv_out);
    return err;
}

int bsp_sendto(struct hisk_info *hiskinfo, unsigned flags, struct sockaddr *addr, int addr_len)
{
    struct socket *sock = NULL;
    struct sockaddr_storage address;
    struct msghdr msg;
    struct kvec iov;
    int err = -1;

    atomic_inc(&g_sockdbg.socket_dbg_cnt.sendto);

    if (hi_sock_get_fd(hiskinfo->fd) < 0) {
        goto out;
    }

    sock = hi_get_sock(hiskinfo->fd);

    sock->file = &g_file_stub;
    iov.iov_base = hiskinfo->buff;
    iov.iov_len = hiskinfo->len;
    msg.msg_name = NULL;
    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    msg.msg_namelen = 0;
    if (addr) {
        err = memcpy_s((void *)&address, sizeof(address), (void *)addr, sizeof(struct sockaddr));
        if (err != EOK) {
            bsp_err("memcpy_s fail\n");
            goto put_fd;
        }
        msg.msg_name = (struct sockaddr *)&address;
        msg.msg_namelen = addr_len;
    }
    msg.msg_flags = flags;

    err = kernel_sendmsg(sock, &msg, &iov, 1, hiskinfo->len);
put_fd:
    hi_sock_put_fd(hiskinfo->fd);

out:
    return err;
}

int bsp_recvfrom(struct hisk_info *hiskinfo, unsigned flags, struct sockaddr *addr, int *addr_len)
{
    struct socket *sock = NULL;
    struct kvec iov;
    struct msghdr msg;
    struct sockaddr_storage address;
    int err = -1;
    int ret;

    atomic_inc(&g_sockdbg.socket_dbg_cnt.recvfrom_in);

    if (hi_sock_get_fd(hiskinfo->fd) < 0) {
        goto out;
    }

    sock = hi_get_sock(hiskinfo->fd);
    sock->file = &g_file_stub;
    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    iov.iov_len = hiskinfo->len;
    iov.iov_base = hiskinfo->buff;
    msg.msg_name = (struct sockaddr *)&address;
    msg.msg_namelen = sizeof(address);

    err = kernel_recvmsg(sock, &msg, &iov, 1, hiskinfo->len, flags);
    if ((err >= 0) && (addr)) {
        if (msg.msg_namelen > (unsigned int)(*addr_len)) {
            bsp_err("recvfrom addrlen is longer than given\n");
            err = -1;
        } else {
            ret = memcpy_s(addr, *addr_len, (struct sockaddr *)&address, msg.msg_namelen);
            if (ret != EOK) {
                bsp_err("memcpy_s fail\n");
                err = -1;
            }
            *addr_len = msg.msg_namelen;
        }
    }

    hi_sock_put_fd(hiskinfo->fd);
out:
    atomic_inc(&g_sockdbg.socket_dbg_cnt.recvfrom_out);
    return err;
}

int bsp_getsockname(int fd, struct sockaddr *addr, int *addrlen)
{
    struct socket *sock = NULL;
    int err = -1;

    atomic_inc(&g_sockdbg.socket_dbg_cnt.getsockname);
    if (hi_sock_get_fd(fd) < 0) {
        goto out;
    }

    sock = hi_get_sock(fd);

    err = kernel_getsockname(sock, addr);
    if (err < 0) {
        bsp_err("kernel getsockname fail\n");
    }

    hi_sock_put_fd(fd);
out:
    return err;
}

int bsp_getpeername(int fd, struct sockaddr *addr, int *addrlen)
{
    struct socket *sock = NULL;
    int ret = -1;

    atomic_inc(&g_sockdbg.socket_dbg_cnt.getpeername);
    if (hi_sock_get_fd(fd) < 0) {
        return ret;
    }

    sock = hi_get_sock(fd);

    ret = kernel_getpeername(sock, addr);
    hi_sock_put_fd(fd);
    if (ret < 0) {
        bsp_err("kernel getperrname fail:%d\n", ret);
        return ret;
    }

    *addrlen = ret;
    return 0;
}

int bsp_getsockopt(int fd, int level, int optname, char *optval, int *optlen)
{
    return 0;

}

int bsp_setsockopt(int fd, int level, int optname, char *optval, unsigned int optlen)
{
    return 0;
}

int bsp_ioctl(int fd, int cmd, unsigned int arg)
{
    return 0;
}

int bsp_shutdown(int fd, enum sock_shutdown_cmd how)
{
    struct socket *sock = NULL;
    int err = -1;

    atomic_inc(&g_sockdbg.socket_dbg_cnt.shutdown);

    if (hi_sock_get_fd(fd) < 0) {
        goto out;
    }

    sock = hi_get_sock(fd);

    err = kernel_sock_shutdown(sock, how);
    if (err < 0) {
        bsp_err("kernel shutdown fail\n");
    }

    hi_sock_put_fd(fd);
out:
    return err;
}

unsigned int bsp_inet_addr(const char *str)
{
    atomic_inc(&g_sockdbg.socket_dbg_cnt.inet_addr);
    return in_aton(str);
}

void hisock_dump_callback(void)
{
    unsigned int i;
    unsigned int *ptr = NULL;
    atomic_t *val = (atomic_t *)&g_sockdbg.socket_dbg_cnt;
    unsigned int loop = g_sockdbg.socket_dump.dump_size / sizeof(atomic_t);

    if (g_sockdbg.socket_dump.dump_reg) {
        ptr = (unsigned int *)g_sockdbg.socket_dump.dump_reg;
        for (i = 0; i < loop; i++) {
            *ptr++ = atomic_read(val++);
        }
    }
}
void hisock_om_dump_init(void)
{
    g_sockdbg.socket_dump.dump_reg =
        (char *)bsp_dump_register_field(DUMP_MODEMAP_HISOCK_DUMP, "HISOCK", HISOCK_DUMP_SIZE, 0);
    if (!g_sockdbg.socket_dump.dump_reg) {
        bsp_err("hisock backup mem for dump alloc failed\n");
        return;
    }
    if (!bsp_dump_register_hook("HISOCK", hisock_dump_callback)) {
        bsp_err("bsp_dump_register_hook failed\n");
        return;
    }
    g_sockdbg.socket_dump.dump_size = sizeof(g_sockdbg.socket_dbg_cnt);
    if (g_sockdbg.socket_dump.dump_size > HISOCK_DUMP_SIZE) {
        bsp_err("save dump more than 1k, only save 1k dump\n");
        g_sockdbg.socket_dump.dump_size = HISOCK_DUMP_SIZE;
        return;
    }

    return;
}
void hi_sockdbg_init(void)
{
    atomic_set(&g_sockdbg.socket_dbg_cnt.sock, 0);
    atomic_set(&g_sockdbg.socket_dbg_cnt.bind, 0);
    atomic_set(&g_sockdbg.socket_dbg_cnt.listen, 0);
    atomic_set(&g_sockdbg.socket_dbg_cnt.accept_in, 0);
    atomic_set(&g_sockdbg.socket_dbg_cnt.accept_out, 0);
    atomic_set(&g_sockdbg.socket_dbg_cnt.connect_in, 0);
    atomic_set(&g_sockdbg.socket_dbg_cnt.connect_out, 0);
    atomic_set(&g_sockdbg.socket_dbg_cnt.select_in, 0);
    atomic_set(&g_sockdbg.socket_dbg_cnt.select_out, 0);
    atomic_set(&g_sockdbg.socket_dbg_cnt.send, 0);
    atomic_set(&g_sockdbg.socket_dbg_cnt.recv_in, 0);
    atomic_set(&g_sockdbg.socket_dbg_cnt.recv_out, 0);
    atomic_set(&g_sockdbg.socket_dbg_cnt.sendto, 0);
    atomic_set(&g_sockdbg.socket_dbg_cnt.recvfrom_in, 0);
    atomic_set(&g_sockdbg.socket_dbg_cnt.recvfrom_out, 0);
    atomic_set(&g_sockdbg.socket_dbg_cnt.getsockname, 0);
    atomic_set(&g_sockdbg.socket_dbg_cnt.getpeername, 0);
    atomic_set(&g_sockdbg.socket_dbg_cnt.getsockopt, 0);
    atomic_set(&g_sockdbg.socket_dbg_cnt.setsockopt, 0);
    atomic_set(&g_sockdbg.socket_dbg_cnt.ioctl, 0);
    atomic_set(&g_sockdbg.socket_dbg_cnt.shutdown, 0);
    atomic_set(&g_sockdbg.socket_dbg_cnt.inet_addr, 0);
    atomic_set(&g_sockdbg.socket_dbg_cnt.close, 0);

    atomic_set(&g_sockdbg.socket_dbg_cnt.sockregfd, 0);
    atomic_set(&g_sockdbg.socket_dbg_cnt.sockunregfd, 0);
    atomic_set(&g_sockdbg.socket_dbg_cnt.sockgetfd, 0);
    atomic_set(&g_sockdbg.socket_dbg_cnt.sockputfd, 0);
}

static int hisocket_proc_show(struct seq_file *m, void *v)
{
    int i;
    int ref_cnt;

    seq_printf(m, "open socket number %d\n", g_hifstable.used);

    for (i = 0; i < HI_SOCKET_MAX; i++) {
        ref_cnt = atomic_read(&g_hifstable.hi_socks[i].cnt);
        if (ref_cnt) {
            seq_printf(m, "socket[%d] OPENED, Reference %d \n", i, ref_cnt);
        } else {
            seq_printf(m, "socket[%d] NULL\n", i);
        }
    }

    return 0;
}

static int hisocket_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, hisocket_proc_show, NULL);
}

static const struct proc_ops g_hisocket_proc_fops = {
    .proc_open = hisocket_proc_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release
};

static struct ctl_path g_kernel_socket_debug_path[] = {
    {
        .procname = "net",
    },
    {},
};

#define HISK_DBG_MODE 0644
static struct ctl_table g_kernel_socket_debug_table[] = {
    {
        .procname = "kernel_socket_debug",
        .data = &g_socket_debug,
        .maxlen = sizeof(int),
        .mode = HISK_DBG_MODE,
        .proc_handler = proc_dointvec,
    },
    {},
};

int om_socket_init(void)
{
    int i;
    int ret;

    ret = memset_s(&g_file_stub, sizeof(g_file_stub), 0, sizeof(g_file_stub));
    if (ret != EOK) {
        bsp_err("memset_s fail\n");
        return -1;
    }

    g_hifstable.used = 0;
    mutex_init(&g_hifstable.hi_mutex);

    for (i = 0; i < HI_SOCKET_MAX; i++) {
        atomic_set(&g_hifstable.hi_socks[i].cnt, 0);
        g_hifstable.hi_socks[i].sock = NULL;
    }

    hi_sockdbg_init();

    hisock_om_dump_init();
    proc_create("hisocket", 0, NULL, &g_hisocket_proc_fops);

    g_hisock_ctlheader = register_sysctl_paths(g_kernel_socket_debug_path, g_kernel_socket_debug_table);
    if (g_hisock_ctlheader == NULL) {
        bsp_err("register_sysctl_paths fail\n");
        return -1;
    }

    return 0;
}

void __exit om_socket_exit(void)
{
    if (g_hisock_ctlheader != NULL) {
        unregister_sysctl_table(g_hisock_ctlheader);
    }
    remove_proc_entry("hisocket", NULL);
    return;
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
MODULE_LICENSE("GPL");
module_init(om_socket_init);
module_exit(om_socket_exit);
#endif
