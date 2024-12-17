/**
 * Copyright (c) @CompanyNameMagicTag 2022-2023. All rights reserved.
 *
 * Description: Common framework layer of the bfgx module
 * Author: @CompanyNameTag
 */

#include <linux/list.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>

#include "board.h"
#include "bfgx_gnss.h"
#include "bfgx_sle.h"
#include "bfgx_dev.h"
#include "hw_bfg_ps.h"
#include "plat_debug.h"
#include "plat_uart.h"
#include "board_bfgx.h"
#include "bfgx_cust.h"
#include "plat_pm.h"
#include "plat_cali.h"
#include "plat_parse_changid.h"
#include "bfgx_data_parse.h"
#include "plat_exception_rst.h"
#include "bfgx_core.h"

#define BFGX_HEART_BEAT_TIME      3000  /* bfgx心跳超时时间为3秒钟 */

STATIC LIST_HEAD(g_devlist);

STATIC int32_t register_misc_device(struct bfgx_dev_node *dev_node,
                                    struct bfgx_subsys_dev_desc *subsys_desc)
{
    int32_t ret;
    struct bfgx_subsys_driver_desc *drv_desc = NULL;

    drv_desc = subsys_desc->get_driver_desc();
    memset_s(&dev_node->c, sizeof(struct miscdevice), 0, sizeof(struct miscdevice));
    dev_node->c.name = subsys_desc->name;
    dev_node->c.fops = drv_desc->file_ops;
    dev_node->c.groups = drv_desc->groups;
    dev_node->c.minor = MISC_DYNAMIC_MINOR;

    ret = misc_register(&dev_node->c);
    if (ret != 0) {
        ps_print_err("fail to register miscdev %s ret %d\n", subsys_desc->name, ret);
        return ret;
    }

    return OAL_SUCC;
}

STATIC void _bfgx_dev_remove(struct ps_core_s *core_data)
{
    struct bfgx_dev_node *dev_node = NULL;
    struct bfgx_dev_node *dev_node_tmp = NULL;

    list_for_each_entry_safe(dev_node, dev_node_tmp, &g_devlist, node) {
        if (dev_node->core != core_data) {
            continue;
        }
        misc_deregister(&dev_node->c);
        list_del(&dev_node->node);
        kfree(dev_node);
    }
}

STATIC int32_t bfgx_bind_ps_core(struct ps_core_s *core_data,
                                 struct bfgx_subsys_dev_desc *subsys_desc,
                                 int32_t desc_num)
{
    int32_t i;
    struct bfgx_dev_node *dev_node = NULL;

    for (i = 0; i < desc_num; i++) {
        if (core_data->uart_index != subsys_desc[i].uart) {
            continue;
        }

        dev_node = (struct bfgx_dev_node *)kmalloc(sizeof(struct bfgx_dev_node), GFP_KERNEL);
        if (dev_node == NULL) {
            ps_print_err("alloc dev node mem fail\n");
            _bfgx_dev_remove(core_data);
            return -ENOMEM;
        }
        if (register_misc_device(dev_node, &subsys_desc[i]) != OAL_SUCC) {
            ps_print_err("index %d name %s device register fail\n", i, subsys_desc[i].name);
            _bfgx_dev_remove(core_data);
            return -OAL_FAIL;
        }
        dev_node->core = core_data;
        dev_node->open_cnt = (atomic_t)ATOMIC_INIT(0);
        dev_node->dev_data = subsys_desc[i].private_data;
        INIT_LIST_HEAD(&dev_node->node);
        list_add_tail(&dev_node->node, &g_devlist);
    }
    ps_print_info("bfgx device num[%d] init success\n", desc_num);

    return OAL_SUCC;
}

STATIC int32_t bfgx_dev_init(struct ps_core_s *core_data)
{
    int32_t ret;
    struct bfgx_prj_desc *prj_desc = NULL;

    if (is_bfgx_support() != OAL_TRUE) {
        /* don't support bfgx */
        ps_print_info("bfgx disabled, dev init bypass\n ");
        return OAL_SUCC;
    }

    if (core_data == NULL) {
        ps_print_err("ps core get fail\n");
        return -OAL_FAIL;
    }

    prj_desc = board_bfgx_get_prj_desc();
    if (prj_desc == NULL || prj_desc->subsys_desc == NULL) {
        ps_print_err("project desc get fail\n");
        return -OAL_FAIL;
    }

    ret = bfgx_bind_ps_core(core_data, prj_desc->subsys_desc, prj_desc->desc_num);
    if (ret < 0) {
        ps_print_err("bind core fail\n");
        return -OAL_FAIL;
    }

    return OAL_SUCC;
}

STATIC void bfgx_dev_remove(struct ps_core_s *core_data)
{
    if (is_bfgx_support() != OAL_TRUE) {
        /* don't support bfgx */
        ps_print_info("bfgx disabled, dev remove bypass\n ");
        return;
    }

    _bfgx_dev_remove(core_data);
}

struct ps_core_s *get_ps_core_from_miscdev(struct miscdevice *dev)
{
    struct bfgx_dev_node *dev_node = NULL;

    if (dev == NULL) {
        return NULL;
    }

    dev_node = oal_container_of(dev, struct bfgx_dev_node, c);

    return dev_node->core;
}

struct ps_core_s *bfgx_get_core_by_dev_name(const char *dev_name)
{
    struct bfgx_dev_node *dev_node = NULL;

    if (dev_name == NULL) {
        return NULL;
    }

    list_for_each_entry(dev_node, &g_devlist, node) {
        if (strcmp(dev_node->c.name, dev_name) == 0) {
            return dev_node->core;
        }
    }

    return NULL;
}

STATIC LIST_HEAD(g_ps_plat_list);

struct ps_core_value {
    const char *uart_name;
    uint32_t uart_index;
    uint32_t baud_rate;
    uint32_t ldisc_num;
    uint32_t flow_cntrl;
};

struct ps_core_dts_label {
    const char *host_wkup_gpio;
    const char *uart_name;
    const char *uart_index;
    const char *baud_rate;
    const char *ldisc_num;
    const char *flow_cntrl;
    const char *pm_switch;
};

struct ps_private_data {
    struct ps_core_value v;       // 默认属性值
    struct ps_core_dts_label dts; // DTS 属性
    struct device_node *of_node;  // 在驱动匹配的时候自动挂载
};

/*
 * Prototype    : ps_get_core_reference
 * Description  : reference the core's data,This references the per-PS
 *                platform device in the arch/xx/board-xx.c file..
 */
struct ps_core_s *ps_get_core_reference(uint32_t uart_index)
{
    struct ps_plat_s *ps_plat_d = NULL;

    list_for_each_entry(ps_plat_d, &g_ps_plat_list, node) {
        if (ps_plat_d->uart_index == uart_index) {
            return ps_plat_d->core_data;
        }
    }

    return NULL;
}

struct ps_core_s *ps_get_core_by_uart_name(const char *uart_name)
{
    struct ps_plat_s *ps_plat_d = NULL;

    if (uart_name == NULL) {
        return NULL;
    }

    list_for_each_entry(ps_plat_d, &g_ps_plat_list, node) {
        if (strcmp(uart_name, get_tty_name(ps_plat_d)) == 0) {
            return ps_plat_d->core_data;
        }
    }

    return NULL;
}

/*
 * Prototype    : ps_core_chk_bfgx_active
 * Description  : to chk wether or not bfg active
 */
bool ps_core_chk_bfgx_active(struct ps_core_s *ps_core_d)
{
    int32_t i = 0;
    for (i = 0; i < BFGX_BUTT; i++) {
        if (atomic_read(&ps_core_d->bfgx_info[i].subsys_state) != POWER_STATE_SHUTDOWN) {
            return true;
        }
    }

    return false;
}

/*
 * Prototype    : ps_core_show_bfgx_status
 * Description  : show bfgx actvie status
 */
void ps_core_show_bfgx_status(void)
{
    struct ps_plat_s *ps_plat_d = NULL;
    struct ps_core_s *ps_core_d = NULL;

    list_for_each_entry(ps_plat_d, &g_ps_plat_list, node) {
        ps_core_d = ps_plat_d->core_data;
        bfgx_print_subsys_state(ps_core_d->uart_index);
    }
}

/*
 * Prototype    : ps_core_single_subsys_active
 * Description  : bfgn other subsys all shutdown except parament subsys
 */
oal_bool_enum ps_core_single_subsys_active(uint8_t subsys)
{
    struct ps_plat_s *ps_plat_d = NULL;
    struct ps_core_s *ps_core_d = NULL;

    list_for_each_entry(ps_plat_d, &g_ps_plat_list, node) {
        ps_core_d = ps_plat_d->core_data;
        if (bfgx_other_subsys_all_shutdown(ps_core_d->pm_data, subsys) != OAL_TRUE) {
            return OAL_FALSE;
        }
    }
    return OAL_TRUE;
}

/*
 * Prototype    : ps_core_ir_only_mode
 * Description  : bfgn has more than one subsys in ir only mode
 */
oal_bool_enum ps_core_ir_only_mode(void)
{
    struct ps_plat_s *ps_plat_d = NULL;
    struct ps_core_s *ps_core_d = NULL;

    list_for_each_entry(ps_plat_d, &g_ps_plat_list, node) {
        ps_core_d = ps_plat_d->core_data;
        if (oal_atomic_read(&ps_core_d->ir_only) != 0) {
            return OAL_TRUE;
        }
    }
    return OAL_FALSE;
}

STATIC OAL_INLINE uint32_t get_ps_u32_by_dts_label(struct device_node *of_node,
                                                   const char *dts_label, uint32_t default_value)
{
    uint32_t value;
    if (dts_label != NULL && of_property_read_u32(of_node, dts_label, &value) == 0) {
        ps_print_info("node %s get dts [%s] value %u\n", of_node->name, dts_label, value);
        return value;
    }

    return default_value;
}

STATIC OAL_INLINE int32_t get_ps_gpio_type_by_dts_label(struct device_node *of_node,
                                                        const char *dts_label, int32_t index)
{
    int32_t gpio_nr, gpio_type;

    if (dts_label == NULL) {
        return GPIO_TYPE_BUFF;
    }

    gpio_nr = of_get_named_gpio(of_node, dts_label, 0);
    if (gpio_nr < 0) {
        return GPIO_TYPE_BUFF;
    }

    gpio_type = conn_get_gpio_type_by_number(gpio_nr);
    if (gpio_type < 0) {
        return GPIO_TYPE_BUFF;
    }

    return gpio_type;
}

STATIC uint32_t get_legancy_baudrate(uint32_t default_baudrate)
{
    uint32_t baudrate;

    if (!mpxx_is_asic()) {
        ps_print_info("fpga baudrate =%u\n", UART_BAUD_RATE_2M);
        return UART_BAUD_RATE_2M;
    }

    if (uart_baudrate_get_by_ini(&baudrate) == OAL_SUCC) {
        ps_print_info("ini cust baudrate =%u\n", baudrate);
        return baudrate;
    }

    if (get_mpxx_subchip_type() == BOARD_VERSION_MP13) {
        ps_print_info("chiptye[%d] baudrate %u\n", BOARD_VERSION_MP13, UART_BAUD_RATE_4M);
        return UART_BAUD_RATE_4M;
    }

    ps_print_info("use default baudrate=%u\n", default_baudrate);
    return default_baudrate;
}

STATIC int32_t ps_plat_dts_init(struct ps_private_data *private_data, struct ps_plat_s *plat_d)
{
    struct ps_core_dts_label *dts_label = NULL;
    const char *uart_name = NULL;

    dts_label = &private_data->dts;

    ps_print_info("dev node name is %s\n", private_data->of_node->name);
    if (dts_label->uart_name != NULL &&
        of_property_read_string(private_data->of_node, dts_label->uart_name, &uart_name) == 0) {
        plat_d->uart_name = uart_name;
    } else {
        plat_d->uart_name = private_data->v.uart_name;
    }
    ps_print_info("ps uart name %s\n", plat_d->uart_name);

    plat_d->ldisc_num = get_ps_u32_by_dts_label(private_data->of_node,
                                                dts_label->ldisc_num, private_data->v.ldisc_num);
    if (plat_d->ldisc_num == 0) {
        return -EINVAL;
    }

    plat_d->uart_index = get_ps_u32_by_dts_label(private_data->of_node, dts_label->uart_index,
                                                 private_data->v.uart_index);
    if (ps_get_core_reference(plat_d->uart_index) != NULL) {
        ps_print_err("uart %s channel index[%d] already register\n", plat_d->uart_name, plat_d->uart_index);
        return -EINVAL;
    }
    plat_d->baud_rate = get_ps_u32_by_dts_label(private_data->of_node, dts_label->baud_rate,
                                                get_legancy_baudrate(private_data->v.baud_rate));
    plat_d->host_wkup_io_type = get_ps_gpio_type_by_dts_label(private_data->of_node, dts_label->host_wkup_gpio, 0);
    plat_d->pm_switch = get_ps_u32_by_dts_label(private_data->of_node, dts_label->pm_switch, 1);
    plat_d->flow_cntrl = get_ps_u32_by_dts_label(private_data->of_node, dts_label->flow_cntrl,
                                                 private_data->v.flow_cntrl);
    return OAL_SUCC;
}

STATIC struct ps_plat_s *ps_plat_init(struct platform_device *pdev)
{
    int32_t ret;
    struct ps_plat_s *ps_plat_d = NULL;
    struct ps_private_data *private_data = NULL;
    struct device_node *of_node = pdev->dev.of_node;

    private_data = (struct ps_private_data *)of_device_get_match_data(&pdev->dev);
    if (private_data == NULL) {
        ps_print_err("need ps private config\n");
        return NULL;
    }
    private_data->of_node = of_node;

    ps_plat_d = (struct ps_plat_s *)kzalloc(sizeof(struct ps_plat_s), GFP_KERNEL);
    if (ps_plat_d == NULL) {
        ps_print_err("no mem to allocate\n");
        return NULL;
    }

    pdev->dev.platform_data = ps_plat_d;
    dev_set_drvdata(&pdev->dev, ps_plat_d);

    ret = ps_plat_dts_init(private_data, ps_plat_d);
    if (ret < 0) {
        ps_print_err("dts init fail\n");
        return NULL;
    }

    INIT_LIST_HEAD(&ps_plat_d->node);
    list_add_tail(&ps_plat_d->node, &g_ps_plat_list);
    ps_print_info("ps_plat channel index[%d] init\n", ps_plat_d->uart_index);

    return ps_plat_d;
}

STATIC void ps_plat_deinit(struct ps_plat_s *plat_d)
{
    list_del(&plat_d->node);
    kfree(plat_d);
}

OAL_STATIC uint8_t g_beat_timeout_reset_enable = OAL_FALSE;

/*
 * 函 数 名  : plat_beat_timeout_reset_set
 * 功能描述  : beat_timer全局配置
 */
void plat_beat_timeout_reset_set(unsigned long arg)
{
    g_beat_timeout_reset_enable = (arg == OAL_FALSE) ? (OAL_FALSE) : (OAL_TRUE);
    ps_print_info("plat beat timer timeOut reset cfg set value = %ld\n", arg);
}

int32_t update_heart_beat_timer(struct ps_core_s *ps_core_d, enum heart_beat_swith on)
{
    int32_t ret;

    if (ps_core_d == NULL) {
        ps_print_err("get ps core d fail\n");
        return -EINVAL;
    }

    if (on == TIMER_RESET) {
        ret = oal_timer_start(&ps_core_d->heartbeat_timer, BFGX_HEART_BEAT_TIME);
        atomic_set(&ps_core_d->heartbeat_flag, BFGX_NOT_RECV_BEAT_INFO);
        ps_print_info("[%s]reset beat timer, ret=%d, jiffers=%lu, expires=%lu\n",
                      index2name(ps_core_d->pm_data->index), ret, jiffies, ps_core_d->heartbeat_timer.expires);
    } else {
        ps_print_info("[%s]delete beat timer\n", index2name(ps_core_d->pm_data->index));
        del_timer_sync(&ps_core_d->heartbeat_timer);
    }

    return 0;
}

STATIC void heartbeat_timer_expire_dfr(struct ps_core_s *ps_core_d)
{
    ps_print_err("[%s]bfgx beat timer bring to reset work!\n", index2name(ps_core_d->pm_data->index));
    if (ps_core_d->uart_index == BUART) {
        plat_exception_handler(SUBSYS_BFGX, THREAD_IDLE, BUART_BEATHEART_TIMEOUT);
    } else if (ps_core_d->uart_index == GUART) {
        plat_exception_handler(SUBSYS_BFGX, THREAD_IDLE, GUART_BEATHEART_TIMEOUT);
    }
    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                         CHR_PLT_DRV_EVENT_DEV, CHR_PLAT_DRV_ERROR_BEAT_TIMEOUT);
#ifdef PLATFORM_DEBUG_ENABLE
    cancel_excp_test_en(HEARTBEATER_TIMEOUT_FAULT, (uintptr_t)ps_core_d);
#endif
}

/*
 * 函 数 名  : heartbeat_timer_expire
 * 功能描述  : bfgx心跳超时处理函数，该函数运行在软中断上下文中，不能有引起睡眠的操作
 * 输入参数  : uint64 data，不需要。加上是为了遵守内核的函数声明
 */
STATIC void heartbeat_timer_expire(oal_timeout_func_para_t arg)
{
    int ret;
    struct ps_core_s *ps_core_d = NULL;
    struct pm_drv_data *pm_data;

    ps_core_d = oal_get_timer_container(ps_core_d, arg, heartbeat_timer);
    if (ps_core_chk_bfgx_active(ps_core_d) == false) {
        ps_print_warning("ps core is closed\n");
        return;
    }
    /* bfgx睡眠时，没有心跳消息上报 */
    pm_data = ps_core_d->pm_data;
    if (pm_data->bfgx_dev_state_get(pm_data) != BFGX_ACTIVE) {
        ps_print_info("[%s]bfgx has sleep!\n", index2name(ps_core_d->pm_data->index));
        return;
    }

    if (atomic_read(&ps_core_d->heartbeat_flag) == BFGX_NOT_RECV_BEAT_INFO) {
        ps_print_info("[%s]timeout!  HZ=%d, jiffers=%lu, expires=%lu\n",
                      index2name(ps_core_d->pm_data->index), HZ, jiffies, ps_core_d->heartbeat_timer.expires);
        ps_uart_state_dump(ps_core_d);
        /* 异常超时，时间未到，重启定时器，连续尝试5次 */
        if (time_after(ps_core_d->heartbeat_timer.expires, jiffies) && (ps_core_d->heartbeat_cnt < 5)) {
            ps_core_d->heartbeat_cnt++;
            ret = oal_timer_start(&ps_core_d->heartbeat_timer, BFGX_HEART_BEAT_TIME);
            ps_print_err("[%s]error! timer is not expires and restart, ret=%d, jiffers=%lu, expires=%lu\n",
                         index2name(ps_core_d->pm_data->index), ret, jiffies, ps_core_d->heartbeat_timer.expires);
            return;
        }

        ps_core_d->heartbeat_cnt = 0;

        ps_print_err("###########[%s]host can not recvive bfgx beat info@@@@@@@@@@@@@@!\n",
                     index2name(ps_core_d->pm_data->index));
        declare_dft_trace_key_info("bfgx beat timeout", OAL_DFT_TRACE_EXCEP);
        (void)ssi_dump_err_regs(SSI_ERR_BFGX_HEART_TIMEOUT);

        if (g_beat_timeout_reset_enable == OAL_TRUE) {
            heartbeat_timer_expire_dfr(ps_core_d);
            return;
        }
    }

    atomic_set(&ps_core_d->heartbeat_flag, BFGX_NOT_RECV_BEAT_INFO);

    ps_core_d->heartbeat_cnt = 0;
    ret = oal_timer_start(&ps_core_d->heartbeat_timer, BFGX_HEART_BEAT_TIME);
    ps_print_info("[%s]reset beat timer, ret=%d, jiffers=%lu, expires=%lu\n",
                  index2name(ps_core_d->pm_data->index), ret, jiffies, ps_core_d->heartbeat_timer.expires);

    bfg_check_timer_work(pm_data);
}

STATIC int32_t ps_core_debug_res_init(struct ps_plat_s *ps_plat_d)
{
    struct ps_core_s *ps_core_d = NULL;

    ps_core_d = (struct ps_core_s *)(ps_plat_d->core_data);
    oal_timer_init(&ps_core_d->heartbeat_timer, BFGX_HEART_BEAT_TIME,
                   heartbeat_timer_expire, (uintptr_t)ps_core_d);
    atomic_set(&ps_core_d->heartbeat_flag, BFGX_NOT_RECV_BEAT_INFO);
    ps_core_d->heartbeat_cnt = 0;
    ps_core_d->heartbeat_dbg_flag = 1;

    /* memdump变量初始化 */
    atomic_set(&ps_core_d->is_memdump_runing, 0);
    /* 初始化bfgx读栈完成量 */
    init_completion(&ps_core_d->wait_read_bfgx_stack);
    return 0;
}

STATIC void ps_core_debug_res_remove(struct ps_plat_s *ps_plat_d)
{
    struct ps_core_s *ps_core_d = NULL;

    ps_core_d = (struct ps_core_s *)ps_plat_d->core_data;
    oal_timer_delete(&ps_core_d->heartbeat_timer);
}

/*
 * Prototype    : ps_core_init
 * Description  : init ps_core_d struct and kzalloc memery
 */
STATIC int32_t ps_core_init(struct ps_plat_s *ps_plat_d)
{
    struct ps_core_s *ps_core_d = NULL;
    int32_t ret;

    ps_core_d = kzalloc(sizeof(struct ps_core_s), GFP_KERNEL);
    if (ps_core_d == NULL) {
        ps_print_err("ps core allocation failed\n");
        return -ENOMEM;
    }
     /* refer to itself */
    ps_plat_d->core_data = ps_core_d;
    ps_core_d->ps_plat = ps_plat_d;
    ps_core_d->uart_index = ps_plat_d->uart_index;

    ret = ps_core_transfer_init(ps_core_d);
    if (ret) {
        kfree(ps_core_d);
        ps_print_err("ps core transfer init fail\n");
        return -EFAULT;
    }

    ret = ps_pm_register(ps_core_d);
    if (ret) {
        ps_core_transfer_remove(ps_core_d);
        kfree(ps_core_d);
        ps_print_err("error registering ps_pm\n");
        return -EFAULT;
    }

    ret = bfgx_dev_init(ps_core_d);
    if (ret != 0) {
        ps_pm_unregister(ps_core_d);
        ps_core_transfer_remove(ps_core_d);
        kfree(ps_core_d);
        return -EINVAL;
    }

    return 0;
}

/*
 * Prototype    : ps_core_exit
 * Description  : release have init ps_core_d struct and kfree memery:
 */
STATIC void ps_core_deinit(struct ps_plat_s *ps_plat_d)
{
    PS_PRINT_FUNCTION_NAME;

    bfgx_dev_remove(ps_plat_d->core_data);
    /* unregister pm */
    ps_pm_unregister(ps_plat_d->core_data);
    /* free pscore tarnsfer data */
    ps_core_transfer_remove(ps_plat_d->core_data);
    kfree(ps_plat_d->core_data);
}

STATIC int32_t ps_probe(struct platform_device *pdev)
{
    struct ps_plat_s *ps_plat_d = NULL;
    int32_t ret;

    ps_plat_d = ps_plat_init(pdev);
    if (ps_plat_d == NULL) {
        ps_print_err("ps plat init fail\n");
        return -EINVAL;
    }
    ret = ps_core_init(ps_plat_d);
    if (ret != 0) {
        ps_print_err("ps core init failed\n");
        goto err_ps_core_init;
    }
    ret = plat_uart_init(ps_plat_d->ldisc_num);
    if (ret) {
        ps_print_err("uart init failed\n");
        goto err_uart_init;
    }
    ret = ps_core_debug_res_init(ps_plat_d);
    if (ret < 0) {
        goto err_ps_core_debug_res_init;
    }
    /* 开机bfgx校准 */
    bt_cali_data_init(ps_plat_d->core_data);
    ps_print_suc("%s is success!\n", __func__);
    return 0;

err_ps_core_debug_res_init:
    plat_uart_exit(ps_plat_d->ldisc_num);
err_uart_init:
    ps_core_deinit(ps_plat_d);
err_ps_core_init:
    ps_plat_deinit(ps_plat_d);
    return -EINVAL;
}

/*
 * Prototype    : ps_suspend
 * Description  : called by kernel when kernel goto suspend
 */
STATIC int32_t ps_suspend(struct platform_device *pdev, oal_pm_message_t state)
{
    ps_print_info("ps_suspend %s event[%d]\n", pdev->name, state.event);
    return 0;
}

/*
 * Prototype    : ps_resume
 * Description  : called by kernel when kernel resume from suspend
 */
STATIC int32_t ps_resume(struct platform_device *pdev)
{
    ps_print_info("ps_resume %s\n", pdev->name);
    return 0;
}

/*
 * Prototype    : ps_remove
 * Description  : called when user applation rmmod driver
 */
STATIC int32_t ps_remove(struct platform_device *pdev)
{
    struct ps_plat_s *ps_plat_d = NULL;

    ps_plat_d = dev_get_drvdata(&pdev->dev);
    if (ps_plat_d == NULL) {
        ps_print_err("ps_plat_d is null\n");
        return -EFAULT;
    }

    bt_cali_data_exit(ps_plat_d->core_data);
    ps_core_debug_res_remove(ps_plat_d);
    /* TTY ldisc cleanup */
    plat_uart_exit(ps_plat_d->ldisc_num);
    ps_core_deinit(ps_plat_d);
    ps_plat_deinit(ps_plat_d);

    return 0;
}

#ifdef _PRE_CONFIG_USE_DTS
STATIC struct ps_private_data bfgx_ps_private_data = {
    .v = {
#ifdef _PRE_PRODUCT_HI1620S_KUNPENG
        .uart_name = "/dev/ttySC_SPI0",
#elif defined(_PRE_PRODUCT_HI3751_V900)
        .uart_name = "/dev/ttyAMA3",
#else
        .uart_name = "/dev/ttyAMA4",
#endif
        .uart_index = BUART,
        .flow_cntrl = FLOW_CTRL_ENABLE,
        .ldisc_num = N_HW_BFG,
        .baud_rate = UART_BAUD_RATE_8M
    },
    .dts = {
        .host_wkup_gpio = DTS_PROP_MPXX_GPIO_BFGX_WAKEUP_HOST,
        .uart_name = DTS_PROP_MPXX_UART_POART,
        .pm_switch = DTS_NODE_MPXX_PM_SWITCH
    }
};

/*
 * Prototype    : sle_ps_private_data
 * Description  : sle drivers data
 */
STATIC struct ps_private_data sle_ps_private_data = {
    .v = {
        .uart_name = "/dev/ttyAMA5",
        .uart_index = GUART,
        .flow_cntrl = FLOW_CTRL_ENABLE,
        .ldisc_num = N_HW_BFG - 1,
        .baud_rate = UART_BAUD_RATE_6M
    },
    .dts = {
        .host_wkup_gpio = DTS_PROP_GPIO_SLE_WAKEUP_HOST,
        .uart_name = DTS_PROP_MPXX_SLEUART_POART,
        .pm_switch = DTS_NODE_MPXX_PM_SWITCH
    }
};

STATIC struct ps_private_data me_ps_private_data = {
    .v = {
        .uart_name = "/dev/ttyAMA5",
        .uart_index = GUART,
        .flow_cntrl = FLOW_CTRL_ENABLE,
        .ldisc_num = N_HW_BFG - 1,
        .baud_rate = UART_BAUD_RATE_6M
    },
    .dts = {
        .host_wkup_gpio = DTS_PROP_MPXX_GPIO_GCPU_WAKEUP_HOST,
        .uart_name = DTS_PROP_MPXX_GUART_POART,
        .pm_switch = DTS_NODE_MPXX_PM_SWITCH
    }
};

STATIC struct of_device_id g_mpxx_ps_match_table[] = {
    {
        .compatible = DTS_NODE_MPXX_BFGX,
        .data = &bfgx_ps_private_data,
    },
    {
        .compatible = DTS_NODE_MPXX_SLE,
        .data = &sle_ps_private_data,
    },
    {
        .compatible = DTS_NODE_MPXX_ME,
        .data = &me_ps_private_data,
    },
    {}
};
#endif

/*  platform_driver struct for PS module */
STATIC struct platform_driver g_ps_platform_driver = {
    .probe = ps_probe,
    .remove = ps_remove,
    .suspend = ps_suspend,
    .resume = ps_resume,
    .driver = {
        .name = "hisi_bfgx",
        .owner = THIS_MODULE,
#ifdef _PRE_CONFIG_USE_DTS
        .of_match_table = g_mpxx_ps_match_table,
#endif
    },
};

#ifdef _PRE_PRODUCT_HI1620S_KUNPENG
struct platform_device  g_ps_platform_device  = {
    .name          = "hisi_bfgx",
    .id          = 0,
};
#endif

STATIC int32_t hw_ps_common_module_init(void)
{
    int32_t ret;

    ret = plat_read_changid();
    if (ret < 0) {
        ps_print_err("plat_cust_init failed\n");
    }

    ret = bfgx_customize_init();
    if (ret < 0) {
        ps_print_err("bfgx_customize_init failed\n");
        goto err_bfgx_custmoize_exit;
    }

    ret  = bfgx_user_ctrl_init();
    if (ret < 0) {
        ps_print_err("bfgx_user_ctrl_init failed\n");
        goto err_user_ctrl_init;
    }

    return OAL_SUCC;
err_user_ctrl_init:
    bfgx_user_ctrl_exit();
err_bfgx_custmoize_exit:
    plat_free_changid_buffer();
    return -EFAULT;
}

STATIC void hw_ps_common_module_remove(void)
{
    bfgx_user_ctrl_exit();
    ps_print_info("sysfs user ctrl removed\n");
    plat_free_changid_buffer();
}

int32_t hw_ps_init(void)
{
    int32_t ret;

    PS_PRINT_FUNCTION_NAME;

    ret = hw_ps_common_module_init();
    if (ret < 0) {
        ps_print_err("common module init fail\n");
        return ret;
    }

#ifdef _PRE_PRODUCT_HI1620S_KUNPENG
    platform_device_register(&g_ps_platform_device);
#endif

    st_tty_recv = ps_core_recv;
    ret = platform_driver_register(&g_ps_platform_driver);
    if (ret) {
        ps_print_err("Unable to register platform bfgx driver.\n");
    }

    return ret;
}

void hw_ps_exit(void)
{
    platform_driver_unregister(&g_ps_platform_driver);
    hw_ps_common_module_remove();
}
