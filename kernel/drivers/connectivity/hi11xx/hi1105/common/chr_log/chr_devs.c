/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: CHR system Function Implementation
 * Author: @CompanyNameTag
 */

#ifdef CONFIG_HI1102_PLAT_HW_CHR
/* 头文件包含 */
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/workqueue.h>
#include <linux/skbuff.h>
#include "chr_devs.h"
#include "board.h"
#include "bfgx_dev.h"
#include "bfgx_core.h"
#include "chr_errno.h"
#include "chr_user.h"
#include "oal_hcc_host_if.h"
#include "hw_bfg_ps.h"
#include "plat_pm.h"
#include "plat_exception_rst.h"
#include "bfgx_data_parse.h"
#include "securec.h"
#include "oam_dsm.h"

#define CHR_FILEID_OFFSET    16

typedef struct {
    uint32_t file_line;
    uint16_t chip_id;
    uint16_t param_num;
} chr_errlog_stru;

typedef struct {
    uint32_t errid;
    uint16_t errlen;
    uint32_t module;
} chr_inner_report_info;

typedef struct {
    uint32_t pc;
    uint32_t lr;
    uint8_t fault_reason;
    uint8_t fault_type;
} chr_inner_wifi_panic_info;

/* 函数声明 */
static int32_t chr_misc_open(struct inode *fd, struct file *fp);
static ssize_t chr_misc_read(struct file *fp, char __user *buff, size_t count, loff_t *loff);
static long chr_misc_ioctl(struct file *fp, uint32_t cmd, uintptr_t arg);
static int32_t chr_misc_release(struct inode *fd, struct file *fp);
static void chr_rx_errno_to_dispatch(uint32_t errno);
static int32_t chr_wifi_tx_handler(uint32_t errno);
static int32_t chr_bfg_dev_tx_handler(uint32_t errno);

/* 全局变量定义 */
static chr_event g_chr_event;
/* 本模块debug控制全局变量 */
static int32_t g_chr_log_enable = CHR_LOG_DISABLE;

static const struct file_operations g_chr_misc_fops = {
    .owner = THIS_MODULE,
    .open = chr_misc_open,
    .read = chr_misc_read,
    .release = chr_misc_release,
    .unlocked_ioctl = chr_misc_ioctl,
};

static struct miscdevice g_chr_misc_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = CHR_DEV_KMSG_PLAT,
    .fops = &g_chr_misc_fops,
};

/*
 * 函 数 名  : chr_misc_open
 * 功能描述  : 打开设备节点接口
 */
static int32_t chr_misc_open(struct inode *fd, struct file *fp)
{
    chr_dbg("chr %s open success\n", g_chr_misc_dev.name);
    return CHR_SUCC;
}

/*
 * 函 数 名  : chr_misc_read
 * 功能描述  : 读取设备节点接口
 */
static ssize_t chr_misc_read(struct file *fp, char __user *buff, size_t count, loff_t *loff)
{
    int32_t ret;
    uint32_t __user *puser = (uint32_t __user *)buff;
    struct sk_buff *skb = NULL;
    uint16_t data_len = 0;

    if (count < sizeof(chr_dev_exception_stru_para)) {
        chr_err("The user space buff is too small\n");
        return -CHR_EFAIL;
    }

    if (buff == NULL) {
        chr_err("chr %s read fail, user buff is NULL", g_chr_misc_dev.name);
        return -EAGAIN;
    }

    skb = skb_dequeue(&g_chr_event.errno_queue);
    if (skb == NULL) {
        if (fp->f_flags & O_NONBLOCK) {
            chr_dbg("Thread read chr with NONBLOCK mode\n");
            /* for no data with O_NONBOCK mode return 0 */
            return 0;
        } else {
            if (wait_event_interruptible(g_chr_event.errno_wait,
                                         (skb = skb_dequeue(&g_chr_event.errno_queue)) != NULL)) {
                if (skb != NULL) {
                    skb_queue_head(&g_chr_event.errno_queue, skb);
                }
                chr_dbg("Thread interrupt with signel\n");
                return -ERESTARTSYS;
            }
        }
    }

    data_len = min_t(size_t, skb->len, count);
    ret = copy_to_user(puser, skb->data, data_len);
    if (ret) {
        chr_warning("copy_to_user err!restore it, len=%d\n", data_len);
        skb_queue_head(&g_chr_event.errno_queue, skb);
        return -EFAULT;
    }

    /* have read count1 byte */
    skb_pull(skb, data_len);

    /* if skb->len = 0: read is over */
    if (skb->len == 0) { /* curr skb data have read to user */
        kfree_skb(skb);
    } else { /* if don,t read over; restore to skb queue */
        skb_queue_head(&g_chr_event.errno_queue, skb);
    }

    return data_len;
}

/* 不作限制的chr上报事件 */
static uint32_t g_chr_unlimit_errno[] = {
    CHR_WIFI_DISCONNECT_QUERY_EVENTID,
    CHR_WIFI_CONNECT_FAIL_QUERY_EVENTID,
    CHR_WIFI_WEB_FAIL_QUERY_EVENTID,
    CHR_WIFI_WEB_SLOW_QUERY_EVENTID,
    CHR_BT_CHIP_SOFT_ERROR_EVENTID,
    CHR_PLATFORM_EXCEPTION_EVENTID
};

/*
 * 函 数 名  : chr_report_frequency_limit
 * 功能描述  : 限制chr上报频率，两条chr的间隔至少10s
 * 返回值    : 两条chr的间隔超过10s，返回SUCC，否则返回FAILED
 */
static int32_t chr_report_frequency_limit(uint32_t errno)
{
#define CHR_REPORT_LIMIT_TIME 5 /* second */
    static unsigned long long chr_report_last_time = 0;
    static uint32_t old_errno = 0;
    unsigned long long chr_report_current_time;
    unsigned long long chr_report_interval_time;
    uint32_t index;
    uint32_t len = (uint32_t)(sizeof(g_chr_unlimit_errno) / sizeof(uint32_t));

    /* 跳过不限制的chr no */
    for (index = 0; index < len; index++) {
        if (errno == g_chr_unlimit_errno[index]) {
            return CHR_SUCC;
        }
    }

    chr_report_current_time = oal_current_kernel_time();
    chr_report_interval_time = chr_report_current_time - chr_report_last_time;

    if ((chr_report_interval_time > CHR_REPORT_LIMIT_TIME) ||
        (chr_report_last_time == 0) || (old_errno != errno)) {
        chr_report_last_time = chr_report_current_time;
        old_errno = errno;
        return CHR_SUCC;
    } else {
        return CHR_EFAIL;
    }
}

/*
 * 函 数 名  : chr_errno_convert_to_bfg_id
 * 功能描述  : 将CHR 错误号，转成bfg id号
 */
static int32_t chr_errno_convert_to_bfg_id(uint32_t errno)
{
    uint32_t chr_id;

    chr_id = errno / CHR_ID_MSK;

    if (chr_id == CHR_BT) {
        return BFGX_BT;
    } else if (chr_id == CHR_GNSS) {
        return BFGX_GNSS;
    } else {
        chr_err("unsupport errno [%d]\n", errno);
        return -EINVAL;
    }
}

static void chr_bfg_disallow_sleep_info(chr_inner_report_info* report)
{
    uint8_t* data = (uint8_t*)report + sizeof(chr_inner_report_info);
    // disallow sleep report携带2个判据
    chr_warning("[sys_id:0x%x]bfg_disallow_sleep_info: data0[%d], data1[%d]\n", report->module, data[0], data[1]);
}

static void chr_pmu_event_err_reg_info(chr_inner_report_info* report)
{
    uint8_t* data = (uint8_t*)report + sizeof(chr_inner_report_info);
    chr_warning("[sys_id:0x%x]pmu_irq_err_reg_info: irq_scp0[0x%x], irq_ocp0[0x%x], irq_ocp1[0x%x], "\
        "irq_irq0[0x%x], irq_ocp2[0x%x]\n",
        report->module, data[0], data[1], data[2], data[3], data[4]);  // (irq) 0:scp0 1:ocp0 2:ocp1 3:irq0 4:ocp2
    chr_warning("[sys_id:0x%x]pmu_event_err_reg_info: scp0_event[0x%x], ocp0_event[0x%x], ocp1_event[0x%x], "\
        "irq0_event[0x%x], ocp2_event[0x%x]\n",
        report->module, data[5], data[6], data[7], data[8], data[9]);  // (event) 5:scp0 6:ocp0 7:ocp1 8:irq0 9:ocp2
}

static void chr_wifi_pc_lr_reg_info(chr_inner_report_info* report, uint16_t len)
{
#if defined(CONFIG_HUAWEI_OHOS_DSM) || defined(CONFIG_HUAWEI_DSM)
    chr_inner_wifi_panic_info* data = NULL;
    int32_t ret;
    char buf[DSM_BUF_MAX_SIZE] = { 0 };

    data = (chr_inner_wifi_panic_info*)((uint8_t*)report + sizeof(chr_inner_report_info));

    ret = snprintf_s(buf, sizeof(buf), sizeof(buf) - 1, "wifi panic PC=0x%x LR=0x%x Type=0x%x Reason=0x%x",
                     data->pc, data->lr, data->fault_type, data->fault_reason);
    if (ret > 0) {
        ps_print_dbg("wifi panic message %s to server\n", buf);
        hw_mpxx_dsm_client_notify(SYSTEM_TYPE_PLATFORM, DSM_MPXX_HALT, "%s\n", buf);
    } else {
        ps_print_err("wifi panic message to server fail, ret = %d\n", ret);
    }
#endif
}

static int32_t chr_cust_id_message_proc(uint32_t errno, uint8_t *ptr_data, uint16_t len)
{
    chr_inner_report_info* report = (chr_inner_report_info*)ptr_data;

    if (len < sizeof(chr_inner_report_info) || len != report->errlen) {
        chr_err("chr errno len is wrong, errno len = %d!\n", len);
        return -EINVAL;
    }

    chr_warning("chr inner report[%d] info: errid=%d, len=%d, module=0x%x\n",
                errno, report->errid, report->errlen, report->module);
    switch (report->errid) {
        case CHR_PLAT_INNER_ERROR_BUS_FRC_ERR:
            declare_dft_trace_key_info("wlan_bus_pwr_frcon_fail", OAL_DFT_TRACE_FAIL);
            plat_exception_handler(SUBSYS_WIFI, THREAD_WIFI, CHIP_FATAL_ERROR);
            break;
        case CHR_PLAT_INNER_ERROR_BFG_DISALLOW_SLEEP:
            chr_bfg_disallow_sleep_info(report);
            break;
        case CHR_PLAT_INNER_ERROR_PMU_EVENT_REG_ERR:
            chr_pmu_event_err_reg_info(report);
            break;
        case CHR_PLAT_INNER_ERROR_WIFI_PC_LR:
            chr_wifi_pc_lr_reg_info(report, len);
            break;
        default:
            break;
    }
    return CHR_SUCC;
}

static int32_t chr_data_is_empty(uint8_t *ptr_data, uint16_t len)
{
    if (ptr_data == NULL) {
        // 异常信息不携带数据 len 应该为0
        if (len > 0) {
            chr_err("chr errno len = %d, but ptr_data is NULL pointer!\n", len);
            return CHR_EFAIL;
        }
    }
    return CHR_SUCC;
}

static int32_t chr_error_log_message_proc(uint32_t errno, uint8_t *ptr_data, uint16_t len)
{
    chr_errlog_stru* report = (chr_errlog_stru*)ptr_data;

    if (len < (uint16_t)sizeof(chr_errlog_stru)) {
        chr_err("chr errno len is wrong, errno len = %d!\n", len);
        return -EINVAL;
    }

    chr_warning("chr error log report[%d] info: file_id=%u, file_line=%u, chip_id=%u\n",
                errno, (report->file_line >> CHR_FILEID_OFFSET), (report->file_line & 0xFFFF), report->chip_id);
    return CHR_SUCC;
}

static int32_t chr_message_report_to_kmsg(uint32_t errno, uint8_t *ptr_data, uint16_t len)
{
    if (errno == CHR_CHIP_INNER_REPORT_EVENTID) {
        return chr_cust_id_message_proc(errno, ptr_data, len);
    }

    if (errno == CHR_ERROR_LOG_EVENTID) {
        return chr_error_log_message_proc(errno, ptr_data, len);
    }
    return CHR_SUCC;
}

static int32_t chr_message_report_to_server(uint32_t errno, uint16_t us_flag, uint8_t *ptr_data, uint16_t len)
{
    struct sk_buff *skb = NULL;
    uint16_t sk_len;
    int32_t ret;

    // sk_len 最大为0xffff, 长度过大会出现溢出
    if (unlikely(len > (USHRT_MAX - sizeof(chr_dev_exception_stru_para)))) {
        chr_warning("chr errno %x len %d over limit\n", errno, len);
        return -EINVAL;
    }

    if (chr_report_frequency_limit(errno)) {
        chr_warning("chr report limited, dispose errno=%d\n", errno);
        return CHR_SUCC;
    }

    if (skb_queue_len(&g_chr_event.errno_queue) > CHR_ERRNO_QUEUE_MAX_LEN) {
        chr_warning("chr errno queue is full, dispose errno=%d\n", errno);
        return CHR_SUCC;
    }

    /* for code run in interrupt context */
    sk_len = (uint16_t)sizeof(chr_dev_exception_stru_para) + len;
    skb = alloc_skb(sk_len, (oal_in_interrupt() || oal_in_atomic()) ? GFP_ATOMIC : GFP_KERNEL);
    if (skb == NULL) {
        chr_err("chr errno alloc skbuff failed! len=%d, errno=%d\n", sk_len, errno);
        return -ENOMEM;
    }

    skb_put(skb, sk_len);
    *(uint32_t *)skb->data = errno;
    *((uint16_t *)(skb->data + 4)) = len;  /* 偏移存放errno的前4个字节 */
    *((uint16_t *)(skb->data + 6)) = us_flag; /* 偏移存放errno加长度的前6个字节 */

    if ((len > 0) && (ptr_data != NULL)) {
        ret = memcpy_s(((uint8_t *)skb->data + sizeof(chr_dev_exception_stru_para)),
                       sk_len - sizeof(chr_dev_exception_stru_para), ptr_data, len);
        if (ret != EOK) {
            chr_err("memcpy_s error, destlen=%lu, srclen=%d\n ",
                    sk_len - sizeof(chr_dev_exception_stru_para), len);
            kfree_skb(skb);
            return -EINVAL;
        }
    }

    skb_queue_tail(&g_chr_event.errno_queue, skb);
    oal_wait_queue_wake_up_interrupt(&g_chr_event.errno_wait);

    chr_warning("chr_write_errno_to_queue success errno=%d\n", errno);

    return CHR_SUCC;
}

/*
 * 函 数 名  : chr_write_errno_to_queue
 * 功能描述  : 将异常码写入队列
 */
static int32_t chr_write_errno_to_queue(uint32_t errno, uint16_t us_flag, uint8_t *ptr_data, uint16_t len)
{
    int32_t ret;

    ret = chr_data_is_empty(ptr_data, len);
    if (ret != CHR_SUCC) {
        return -ENOMEM;
    }

    if ((errno == CHR_CHIP_INNER_REPORT_EVENTID) || (errno == CHR_ERROR_LOG_EVENTID)) {
        ret = chr_message_report_to_kmsg(errno, ptr_data, len);
        return ret;
    }

    ret = chr_message_report_to_server(errno, us_flag, ptr_data, len);
    if (ret != CHR_SUCC) {
        return ret;
    }

    return CHR_SUCC;
}

static long chr_misc_errno_write(uint32_t __user *puser)
{
    uint32_t ret;
    uint8_t *pst_mem = NULL;
    chr_host_exception_stru chr_rx_data;

    ret = copy_from_user(&chr_rx_data, puser, sizeof(chr_host_exception_stru));
    if (ret) {
        chr_err("chr %s ioctl fail, get data from user fail ret = %u", g_chr_misc_dev.name, ret);
        return -EINVAL;
    }

    if (chr_rx_data.chr_len == 0) {
        chr_write_errno_to_queue(chr_rx_data.chr_errno, CHR_HOST, NULL, 0);
    } else {
        pst_mem = oal_memalloc(chr_rx_data.chr_len);
        if (pst_mem == NULL) {
            chr_err("chr mem alloc failed len %u\n", chr_rx_data.chr_len);
            return -EINVAL;
        }

        if (chr_rx_data.chr_ptr == NULL) {
            chr_err("chr input arg is invalid!\n");
            oal_free(pst_mem);
            return -EINVAL;
        }

        ret = copy_from_user(pst_mem, (void __user *)(chr_rx_data.chr_ptr), chr_rx_data.chr_len);
        if (ret) {
            chr_err("chr %s ioctl fail, get data from user fail ret = %u, errno=%u, chr_len=%u",
                    g_chr_misc_dev.name, ret, chr_rx_data.chr_errno, chr_rx_data.chr_len);
            oal_free(pst_mem);
            return -EINVAL;
        }

        chr_write_errno_to_queue(chr_rx_data.chr_errno, CHR_HOST, pst_mem, chr_rx_data.chr_len);
        oal_free(pst_mem);
    }

    return CHR_SUCC;
}
/*
 * 函 数 名  : chr_misc_ioctl
 * 功能描述  : 控制设备节点接口
 */
static long chr_misc_ioctl(struct file *fp, uint32_t cmd, uintptr_t arg)
{
    uint32_t OAL_USER *puser = (uint32_t OAL_USER *)arg;
    uint32_t ret;
    uint32_t value = 0;

    if (_IOC_TYPE(cmd) != CHR_MAGIC) {
        chr_err("chr %s ioctl fail, the type of cmd is error type is %d\n", g_chr_misc_dev.name, _IOC_TYPE(cmd));
        return -EINVAL;
    }

    if (_IOC_NR(cmd) > CHR_MAX_NR) {
        chr_err("chr %s ioctl fail, the nr of cmd is error, nr is %d\n", g_chr_misc_dev.name, _IOC_NR(cmd));
        return -EINVAL;
    }

    switch (cmd) {
        case CHR_ERRNO_WRITE:
            if (chr_misc_errno_write(puser) < 0) {
                return -EINVAL;
            }
            break;
        case CHR_ERRNO_ASK:
            ret = get_user(value, puser);
            if (ret) {
                chr_err("chr %s ioctl fail, get data from user fail", g_chr_misc_dev.name);
                return -EINVAL;
            }
            chr_rx_errno_to_dispatch(value);
            break;
        default:
            chr_warning("chr ioctl not support cmd=0x%x\n", cmd);
            return -EINVAL;
    }

    return CHR_SUCC;
}

/*
 * 函 数 名  : chr_misc_release
 * 功能描述  : 释放节点设备接口
 */
static int32_t chr_misc_release(struct inode *fd, struct file *fp)
{
    if (g_chr_log_enable != CHR_LOG_ENABLE) {
        chr_err("chr %s release fail, module is disable\n", g_chr_misc_dev.name);
        return -EBUSY;
    }
    chr_dbg("chr %s release success\n", g_chr_misc_dev.name);
    return CHR_SUCC;
}

int32_t chr_exception_para(uint32_t chr_errno, uint8_t *chr_ptr, uint16_t chr_len)
{
    if (g_chr_log_enable != CHR_LOG_ENABLE) {
        chr_dbg("chr throw exception fail, module is disable\n");
        return -CHR_EFAIL;
    }

    chr_write_errno_to_queue(chr_errno, CHR_HOST, chr_ptr, chr_len);
    return CHR_SUCC;
}

EXPORT_SYMBOL(chr_exception_para);
/* chr_flag 取值范围 chr_report_flags_enum */
int32_t chr_exception_para_q(uint32_t chr_errno, uint16_t chr_flag,
                             uint8_t *chr_ptr, uint16_t chr_len)
{
    if (g_chr_log_enable != CHR_LOG_ENABLE) {
        chr_dbg("chr throw exception fail, module is disable\n");
        return -CHR_EFAIL;
    }

    chr_write_errno_to_queue(chr_errno, chr_flag, chr_ptr, chr_len);
    return CHR_SUCC;
}

EXPORT_SYMBOL(chr_exception_para_q);

/*
 * 函 数 名  : chr_dev_exception_callback
 * 功能描述  : device异常回调接口
 */
void chr_dev_exception_callback(void *buff, uint16_t len)
{
    chr_dev_exception_stru_para *chr_dev_exception_p = NULL;
    chr_dev_exception_stru *chr_dev_exception = NULL;
    uint32_t chr_len;
    uint8_t *chr_data = NULL;

    if (g_chr_log_enable != CHR_LOG_ENABLE) {
        chr_dbg("chr throw exception fail, module is disable\n");
        return;
    }

    if (buff == NULL) {
        chr_warning("chr recv device errno fail, buff is NULL\n");
        return;
    }

    chr_dev_exception = (chr_dev_exception_stru *)buff;

    /* mode select */
    if ((chr_dev_exception->framehead == CHR_DEV_FRAME_START) && (chr_dev_exception->frametail == CHR_DEV_FRAME_END)) {
        /* old interface: chr upload has only errno */
        chr_len = (uint32_t)sizeof(chr_dev_exception_stru);

        if (len != chr_len) {
            chr_warning("chr recv device errno fail, len %d is unavailable,chr_len %d\n", (int32_t)len, chr_len);
            return;
        }

        chr_write_errno_to_queue(chr_dev_exception->error, CHR_DEVICE, NULL, 0);
    } else {
        /* new interface:chr upload eigher has data or not */
        chr_dev_exception_p = (chr_dev_exception_stru_para *)buff;
        chr_len = (uint32_t)sizeof(chr_dev_exception_stru_para) + chr_dev_exception_p->errlen;

        if (len != chr_len) {
            chr_warning("chr recv device errno fail, len %d is unavailable,chr_len %d\n", (int32_t)len, chr_len);
            return;
        }

        if (chr_dev_exception_p->errlen == 0) {
            chr_write_errno_to_queue(chr_dev_exception_p->error, chr_dev_exception_p->flag, NULL, 0);
        } else {
            chr_data = (uint8_t *)buff + sizeof(chr_dev_exception_stru_para);
            chr_write_errno_to_queue(chr_dev_exception_p->error, chr_dev_exception_p->flag,
                                     chr_data, chr_dev_exception_p->errlen);
        }
    }
}
EXPORT_SYMBOL(chr_dev_exception_callback);

static chr_callback_stru g_chr_get_wifi_info_callback;

void chr_host_callback_register(chr_get_wifi_info pfunc)
{
    if (pfunc == NULL) {
        chr_err("chr_host_callback_register::pfunc is null !");
        return;
    }

    g_chr_get_wifi_info_callback.chr_get_wifi_info = pfunc;

    return;
}

void chr_host_callback_unregister(void)
{
    g_chr_get_wifi_info_callback.chr_get_wifi_info = NULL;

    return;
}

EXPORT_SYMBOL(chr_host_callback_register);
EXPORT_SYMBOL(chr_host_callback_unregister);

/*
 * 函 数 名  : chr_rx_errno_to_dispatch
 * 功能描述  : 将接收到的errno进行解析并分配
 */
static void chr_rx_errno_to_dispatch(uint32_t errno)
{
    uint32_t chr_num;
    chr_num = errno / CHR_ID_MSK;
    switch (chr_num) {
        case CHR_WIFI:

            if (chr_wifi_tx_handler(errno) != CHR_SUCC) {
                chr_info("wifi tx failed,0x%x", errno);
            }
            break;
        case CHR_BT:
        case CHR_GNSS:
            if (chr_bfg_dev_tx_handler(errno) != CHR_SUCC) {
                chr_info("bt/gnss tx failed,0x%x", errno);
            }
            break;
        default:
            chr_err("rcv error num 0x%x", errno);
    }
}

/*
 * 函 数 名  : chr_wifi_dev_tx_handler
 * 功能描述  : 通过hcc通道将errno下发到wifi device
 */
static int32_t chr_wifi_dev_tx_handler(uint32_t errno)
{
    struct hcc_transfer_param st_hcc_transfer_param = {0};
    struct hcc_handler *hcc = hcc_get_handler(HCC_EP_WIFI_DEV);
    oal_netbuf_stru *pst_netbuf = NULL;
    int32_t l_ret;

    if (hcc == NULL) {
        oal_io_print("chr_wifi_dev_tx_handler::hcc is null\n");
        return -CHR_EFAIL;
    }

    pst_netbuf = hcc_netbuf_alloc(sizeof(uint32_t));
    if (pst_netbuf == NULL) {
        oal_io_print("hwifi alloc skb fail.\n");
        return -CHR_EFAIL;
    }

    l_ret = memcpy_s(oal_netbuf_put(pst_netbuf, sizeof(uint32_t)), sizeof(uint32_t),
                     &errno, sizeof(uint32_t));
    if (l_ret != EOK) {
        oal_io_print("chr_wifi errno copy failed\n");
        oal_netbuf_free(pst_netbuf);
        return -CHR_EFAIL;
    }

    hcc_hdr_param_init(&st_hcc_transfer_param,
                       HCC_ACTION_TYPE_CHR,
                       0,
                       0,
                       HCC_FC_NONE,
                       DATA_HI_QUEUE);
    l_ret = hcc_tx(hcc, pst_netbuf, &st_hcc_transfer_param);
    if (l_ret != CHR_SUCC) {
        oal_io_print("chr_wifi_dev_tx_handler::hcc tx is fail,ret=%d\n", l_ret);
        oal_netbuf_free(pst_netbuf);
        return -CHR_EFAIL;
    }

    return CHR_SUCC;
}

/*
 * 函 数 名  : chr_host_tx_handler
 * 功能描述  : 调用回调接口将errno传给hmac
 */
static int32_t chr_host_tx_handler(uint32_t errno)
{
    if (g_chr_get_wifi_info_callback.chr_get_wifi_info == NULL) {
        oal_io_print("{chr_host_tx_handler:: callback is null!}");
        return CHR_EFAIL;
    }
    if (g_chr_get_wifi_info_callback.chr_get_wifi_info(errno) != CHR_SUCC) {
        oal_io_print("{chr_host_tx_handler:: tx faild, errno = %u !}", errno);
        return CHR_EFAIL;
    }

    return CHR_SUCC;
}

static int32_t chr_wifi_tx_handler(uint32_t errno)
{
    int32_t ret1;
    int32_t ret2;

    if (wlan_pm_is_poweron() == OAL_FALSE) {
        chr_info("handle error[%d] fail, wifi device not active\n", errno);
        return -CHR_EFAIL;
    }

    ret1 = chr_host_tx_handler(errno);
    ret2 = chr_wifi_dev_tx_handler(errno);
    if (ret1 != CHR_SUCC || ret2 != CHR_SUCC) {
        chr_info("wifi tx failed,errno[%u],host tx ret1[%d],device tx ret2[%d]", errno, ret1, ret2);
        return -CHR_EFAIL;
    }

    chr_info("tx is succ,errno %u\n", errno);

    return CHR_SUCC;
}

static const char *get_chr_dev_name(int32_t bfgx_subsys)
{
    if (bfgx_subsys == BFGX_BT) {
        return HW_BT_DEV_NAME;
    } else if (bfgx_subsys == BFGX_GNSS) {
        return HW_GNSS_DEV_NAME;
    }

    return NULL;
}

static struct ps_core_s *get_valid_chr_ps_core(uint32_t errno)
{
    int32_t bfgx_subsys;
    struct ps_core_s *ps_core_d = NULL;
    const char *dev_name = NULL;

    bfgx_subsys = chr_errno_convert_to_bfg_id(errno);
    if (bfgx_subsys < 0) {
        return NULL;
    }

    dev_name = get_chr_dev_name(bfgx_subsys);
    if (dev_name == NULL) {
        chr_err("get dev name is NULL\n");
        return NULL;
    }

    ps_core_d = bfgx_get_core_by_dev_name(dev_name);
    if (unlikely((ps_core_d == NULL) || (ps_core_d->pm_data == NULL))) {
        chr_err("ps_core_d is NULL\n");
        return NULL;
    }

    if (ps_bfg_subsys_active(ps_core_d, bfgx_subsys) == false) {
        chr_info("handle error[%d] fail, bfgx device not active\n", errno);
        return NULL;
    }

    /* if high queue num > MAX_NUM and don't write */
    if (oal_netbuf_list_len(&ps_core_d->tx_high_seq) > TX_HIGH_QUE_MAX_NUM) {
        chr_err("bt tx high seqlen large than MAXNUM\n");
        return NULL;
    }

    return ps_core_d;
}

/*
 * 函 数 名  : chr_bfg_dev_tx_handler
 * 功能描述  : 利用uart通道将errno传给bfg
 */
static int32_t chr_bfg_dev_tx_handler(uint32_t errno)
{
    struct sk_buff *skb = NULL;
    uint16_t sk_len;
    int32_t ret;
    int32_t prepare_result;
    struct ps_core_s *ps_core_d = NULL;

    ps_core_d = get_valid_chr_ps_core(errno);
    if (ps_core_d == NULL) {
        chr_info("chr dev tx abort, errno[%u]\n", errno);
        return -CHR_EFAIL;
    }

    ret = prepare_to_visit_node(ps_core_d, &prepare_result);
    if (ret != CHR_SUCC) {
        chr_err("prepare work fail, bring to reset work\n");
        plat_exception_handler(SUBSYS_BFGX, THREAD_BT, get_wakeup_excp_by_uart_idx(ps_core_d->pm_data->index));
        return ret;
    }

    /* modify expire time of uart idle timer */
    mod_timer(&ps_core_d->pm_data->bfg_timer, jiffies + msecs_to_jiffies(BT_SLEEP_TIME));
    oal_atomic_inc(&ps_core_d->pm_data->bfg_timer_mod_cnt);

    /* alloc skb buf */
    sk_len = (uint16_t)(sizeof(uint32_t) + sizeof(struct ps_packet_head) + sizeof(struct ps_packet_end));
    skb = alloc_skb(sk_len, (oal_in_interrupt() || oal_in_atomic()) ? GFP_ATOMIC : GFP_KERNEL);
    if (skb == NULL) {
        chr_err("alloc skbuff failed! len=%d, errno=0x%x\n", sk_len, errno);
        post_to_visit_node(ps_core_d, prepare_result);
        return -CHR_EFAIL;
    }
    skb_put(skb, sk_len);
    /* skb data init,reuse the type of mem_dump to prevent the change of rom */
    ps_add_packet_head(skb->data, MEM_DUMP, sk_len);
    /* put errno into skb_data */
    *(uint32_t *)(skb->data + sizeof(struct ps_packet_head)) = errno;
    ps_skb_enqueue(ps_core_d, skb, TX_HIGH_QUEUE);

    ps_core_tx_work_add(ps_core_d);

    post_to_visit_node(ps_core_d, prepare_result);
    chr_info("tx is succ,errno %u\n", errno);

    return CHR_SUCC;
}

int32_t chr_miscdevs_init(void)
{
    int32_t ret;

    oal_wait_queue_init_head(&g_chr_event.errno_wait);
    skb_queue_head_init(&g_chr_event.errno_queue);

    ret = misc_register(&g_chr_misc_dev);
    if (ret != CHR_SUCC) {
        chr_err("chr module init fail\n");
        return -CHR_EFAIL;
    }
    g_chr_log_enable = CHR_LOG_ENABLE;
    chr_info("chr module init succ\n");

    return CHR_SUCC;
}

void chr_miscdevs_exit(void)
{
    if (g_chr_log_enable != CHR_LOG_ENABLE) {
        chr_info("chr module is diabled\n");
        return;
    }

    misc_deregister(&g_chr_misc_dev);
    g_chr_log_enable = CHR_LOG_DISABLE;
    chr_info("chr module exit succ\n");
}

MODULE_AUTHOR("Hisilicon platform Driver Group");
MODULE_DESCRIPTION("Hisilicon chr log driver");
MODULE_LICENSE("GPL");
#endif
