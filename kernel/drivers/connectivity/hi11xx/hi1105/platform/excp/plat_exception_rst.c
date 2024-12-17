/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: External interface implementation for exception reset
 * Author: @CompanyNameTag
 */

/* 头文件包含 */
#include "plat_debug.h"
#include "plat_pm.h"
#include "plat_firmware.h"
#include "plat_firmware_download.h"
#include "plat_uart.h"
#include "oal_hcc_host_if.h"
#include "oam_rdr.h"
#include "oam_ext_if.h"
#include "bfgx_dev.h"
#include "bfgx_core.h"
#include "pcie_linux.h"
#include "pcie_chip.h"
#include "exception_common.h"
#include "bfgx_exception_rst.h"
#include "wifi_exception_rst.h"
#include "gt_exception_rst.h"
#include "mp13_exception_rst.h"
#include "mp15_exception_rst.h"
#include "mp16c_exception_rst.h"
#include "mp16_exception_rst.h"
#include "gf61_exception_rst.h"
#include "plat_exception_rst.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_PLAT_EXCEPTION_RST_C

typedef int32_t (*memdump_enquenue)(uint8_t *buf_ptr, uint16_t count);

/* 全局变量定义 */
OAL_STATIC struct st_exception_info *g_mpxx_exception_info = NULL;

OAL_STATIC excp_info_str_t g_excp_info_str_tab[] = {
    { .id = BUART_PANIC,            .name = "buart_panic" },
    { .id = GUART_PANIC,            .name = "guart_panic" },
    { .id = HCC_BUS_PANIC,          .name = "hcc_bus_panic" },
    { .id = HCC_BUS_TRANS_FAIL,     .name = "hcc_bus_trans_fail" },

    { .id = BUART_WAKEUP_FAIL,      .name = "buart_wakeup_fail" },
    { .id = GUART_WAKEUP_FAIL,      .name = "guart_wakeup_fail" },
    { .id = WIFI_WAKEUP_FAIL,       .name = "wifi_wakeup_fail" },
    { .id = GT_WAKEUP_FAIL,         .name = "gt_wakeup_fail" },

    { .id = BT_POWERON_FAIL,        .name = "bt_poweron_fail" },
    { .id = FM_POWERON_FAIL,        .name = "fm_poweron_fail" },
    { .id = GNSS_POWERON_FAIL,      .name = "gnss_poweron_fail" },
    { .id = IR_POWERON_FAIL,        .name = "ir_poweron_fail" },
    { .id = NFC_POWERON_FAIL,       .name = "nfc_poweron_fail" },
    { .id = WIFI_POWERON_FAIL,      .name = "wifi_poweron_fail" },
    { .id = SLE_POWERON_FAIL,       .name = "sle_poweron_fail" },

    { .id = BUART_BEATHEART_TIMEOUT, .name = "buart_beatheart_timeout" },
    { .id = GUART_BEATHEART_TIMEOUT, .name = "guart_beatheart_timeout" },
    { .id = BFGX_TIMER_TIMEOUT,     .name = "bfgx_timer_timeout" },
    { .id = BFGX_ARP_TIMEOUT,       .name = "bfgx_arp_timeout" },

    { .id = WIFI_WATCHDOG_TIMEOUT,  .name = "wifi_watchdog_timeout" },
    { .id = SDIO_DUMPBCPU_FAIL,     .name = "sdio_dumpbcpu_fail" },

    { .id = GT_WATCHDOG_TIMEOUT,    .name = "gt_watchdog_timeout" },
    { .id = CHIP_FATAL_ERROR,       .name = "chip_fatal_error" },
    { .id = GUGONG_POWER_FAIL,      .name = "gugong_power_fail" },

    { .id = BT_POWER_OFF_FAIL,        .name = "bt_power_off_fail" },
    { .id = FM_POWER_OFF_FAIL,        .name = "fm_power_off_fail" },
    { .id = GNSS_POWER_OFF_FAIL,      .name = "gnss_power_off_fail" },
    { .id = IR_POWER_OFF_FAIL,        .name = "ir_power_off_fail" },
    { .id = NFC_POWER_OFF_FAIL,       .name = "nfc_power_off_fail" },
    { .id = SLE_POWER_OFF_FAIL,       .name = "sle_power_off_fail" },
};

#ifdef PLATFORM_DEBUG_ENABLE
OAL_STATIC excp_test_cfg_t g_excp_test_cfg[EXCP_TEST_CFG_BUTT];
#endif

OAL_STATIC uint32_t g_uart_panic_type[UART_BUTT] = {
    BUART_PANIC,
    GUART_PANIC
};

OAL_STATIC uint32_t g_uart_wakeup_type[UART_BUTT] = {
    BUART_WAKEUP_FAIL,
    GUART_WAKEUP_FAIL
};

OAL_STATIC excp_chip_res_t *excp_chip_res_init(uint32_t chip_type);
OAL_STATIC void get_subsys_core_type(uint32_t exception_type, excp_record_t *excp_record);
OAL_STATIC void wait_memdump_complete(struct st_exception_info *excp_data);
OAL_STATIC void dfr_fail_power_off(excp_chip_res_t *chip_res);

/*
 * 函 数 名  : get_exception_info_reference
 * 功能描述  : 获得保存异常信息的全局变量
 */
struct st_exception_info *get_exception_info_reference(void)
{
    if (g_mpxx_exception_info == NULL) {
        ps_print_err("%s g_mpxx_exception_info is NULL\n", __func__);
    }

    return g_mpxx_exception_info;
}
EXPORT_SYMBOL_GPL(get_exception_info_reference);

OAL_STATIC char *excp_info_str_get(int32_t id)
{
    int32_t i = 0;
    for (i = 0; i < sizeof(g_excp_info_str_tab) / sizeof(excp_info_str_t); i++) {
        if (id == g_excp_info_str_tab[i].id) {
            return g_excp_info_str_tab[i].name;
        }
    }
    return NULL;
}

CHIP_CFG_FUNC g_excp_chip_res_cfg[BOARD_VERSION_MPXX_BUTT] = {
    NULL,
    get_mp13_chip_cfg,
    NULL,
    get_mp15_chip_cfg,
    get_mp16_chip_cfg,
    get_mp16c_chip_cfg,
    get_gf61_chip_cfg
};

excp_chip_res_t *g_excp_chip_res = NULL;

int32_t plat_get_excp_total_cnt(excp_cnt_t *excp_cnt)
{
    uint32_t i;
    excp_chip_res_t *chip_res = NULL;
    excp_core_res_t *core_res = NULL;
    struct st_exception_info *excp_data = get_exception_info_reference();

    if (oal_unlikely(excp_data == NULL || excp_data->excp_chip == NULL)) {
        ps_print_err("excp_data or excp_chip NULL\n");
        return -EXCEPTION_FAIL;
    }

    chip_res = excp_data->excp_chip;
    excp_cnt->chip_cnt = chip_res->chip_excp_cnt;
    for (i = 0; i < chip_res->subsys_cnt; i++) {
        excp_cnt->subsys_cnt += chip_res->subsys_attr[i].subsys_excp_cnt;
        core_res = chip_res->subsys_attr[i].core_attr;
        if (core_res == NULL) {
            continue;
        }

        excp_cnt->core_cnt += core_res->sla_excp_total_cnt;
    }
    return EXCEPTION_SUCCESS;
}

int32_t get_core_excp_info(char *buf, int32_t index, excp_subsys_res_t *subsys_res)
{
    int etype, ret;
    excp_core_res_t *core_res = subsys_res->core_attr;
    if (core_res == NULL) {
        return 0;
    }

    for (etype = 0; etype < EXCEPTION_TYPE_BOTTOM; etype++) {
        ret = snprintf_s(buf + index, PAGE_SIZE - index, PAGE_SIZE - index - 1,
                         "id:%-30sslave_core_rst:%-10u\n",
                         excp_info_str_get(etype),
                         core_res->sla_excp_stats[etype]);
        if (ret < 0) {
            return index;
        }
        index += ret;
    }
    return index;
}

int32_t core_excp_print(excp_subsys_res_t *subsys_res)
{
    int etype;
    excp_core_res_t *core_res = NULL;
    core_res = subsys_res->core_attr;
    if (core_res == NULL) {
        return OAL_FAIL;
    }

    for (etype = 0; etype < EXCEPTION_TYPE_BOTTOM; etype++) {
        ps_print_err("id:%-30sslave_core_rst:%-10d\n", excp_info_str_get(etype), core_res->sla_excp_stats[etype]);
    }
    return OAL_SUCC;
}


int32_t plat_get_dfr_sinfo(char *buf, int32_t index)
{
    int i = index;
    int etype, j, ret;
    excp_cnt_t excp_cnt = {0};
    excp_chip_res_t *chip_res = NULL;
    excp_subsys_res_t *subsys_res = NULL;
    struct st_exception_info *excp_data = get_exception_info_reference();

    if (oal_unlikely(excp_data == NULL || excp_data->excp_chip == NULL)) {
        ps_print_err("excp_data or excp_chip NULL\n");
        return -EXCEPTION_FAIL;
    }

    ret = snprintf_s(buf + i, PAGE_SIZE - i, PAGE_SIZE - i - 1, "==========dfr info:=========\n");
    if (ret < 0) {
        return i;
    }
    i += ret;

    plat_get_excp_total_cnt(&excp_cnt);
    ret = snprintf_s(buf + i, PAGE_SIZE - i, PAGE_SIZE - i - 1,
                     "total cnt:chip_rst=%u, subsys_rst=%u, core_rst=%u\n",
                     excp_cnt.chip_cnt, excp_cnt.subsys_cnt, excp_cnt.core_cnt);
    if (ret < 0) {
        return i;
    }
    i += ret;

    chip_res = excp_data->excp_chip;
    for (j = 0; j < chip_res->subsys_cnt; j++) {
        subsys_res = &chip_res->subsys_attr[j];
        ret = snprintf_s(buf + i, PAGE_SIZE - i, PAGE_SIZE - i - 1,
                         "==========%s: subsys_rst info:=========\n", subsys_res->subsys_cfg->subsys_name);
        if (ret < 0) {
            return i;
        }
        i += ret;

        i += get_core_excp_info(buf, i, subsys_res);
        for (etype = 0; etype < EXCEPTION_TYPE_BOTTOM; etype++) {
            ret = snprintf_s(buf + i, PAGE_SIZE - i, PAGE_SIZE - i - 1,
                             "id:%-30ssubsys_rst:%-10u\n",
                             excp_info_str_get(etype),
                             subsys_res->subsys_excp_stats[etype]);
            if (ret < 0) {
                return i;
        }
            i += ret;
        }
    }

    return i;
}
int32_t plat_print_dfr_info(void)
{
    int etype, j;
    excp_cnt_t excp_cnt = {0};
    excp_chip_res_t *chip_res = NULL;
    excp_subsys_res_t *subsys_res = NULL;
    struct st_exception_info *excp_data = get_exception_info_reference();

    if (oal_unlikely(excp_data == NULL || excp_data->excp_chip == NULL)) {
        ps_print_err("excp_data or excp_chip NULL\n");
        return -EXCEPTION_FAIL;
    }

    ps_print_info("[MPXX_DFR]========== dfr info:+++++++++++++++++++++++++++++++\n");
    plat_get_excp_total_cnt(&excp_cnt);
    ps_print_info("total cnt:chip_rst=%d, subsys_rst=%d, core_rst=%d\n",
                  excp_cnt.chip_cnt, excp_cnt.subsys_cnt, excp_cnt.core_cnt);

    chip_res = excp_data->excp_chip;
    for (j = 0; j < chip_res->subsys_cnt; j++) {
        subsys_res = &chip_res->subsys_attr[j];
        ps_print_info("==========%s: subsys_rst info:=========\n", subsys_res->subsys_cfg->subsys_name);
        core_excp_print(subsys_res);
        for (etype = 0; etype < EXCEPTION_TYPE_BOTTOM; etype++) {
            ps_print_info("id:%-30ssubsys_rst:%-10d\n", excp_info_str_get(etype),
                          subsys_res->subsys_excp_stats[etype]);
        }
    }

    return EXCEPTION_SUCCESS;
}

OAL_STATIC void plat_exception_dump_file_rotate_init(void)
{
    oal_wait_queue_init_head(&g_bcpu_memdump_cfg.dump_type_wait);
    skb_queue_head_init(&g_bcpu_memdump_cfg.dump_type_queue);
    skb_queue_head_init(&g_bcpu_memdump_cfg.quenue);
    oal_wait_queue_init_head(&g_wcpu_memdump_cfg.dump_type_wait);
    skb_queue_head_init(&g_wcpu_memdump_cfg.dump_type_queue);
    skb_queue_head_init(&g_wcpu_memdump_cfg.quenue);
    oal_wait_queue_init_head(&g_gcpu_memdump_cfg.dump_type_wait);
    skb_queue_head_init(&g_gcpu_memdump_cfg.dump_type_queue);
    skb_queue_head_init(&g_gcpu_memdump_cfg.quenue);
    oal_wait_queue_init_head(&g_gt_memdump_cfg.dump_type_wait);
    skb_queue_head_init(&g_gt_memdump_cfg.dump_type_queue);
    skb_queue_head_init(&g_gt_memdump_cfg.quenue);
    ps_print_info("plat exception dump file rotate init success\n");
}

/*
 * 函 数 名  : plat_exception_reset_handler
 * 功能描述  : 执行平台的复位恢复动作
 * 返 回 值  : 异常处理成功返回0，否则返回负值
 */
OAL_STATIC int32_t plat_exception_reset_handler(struct st_exception_info *exception_data)
{
    int32_t ret;
    ktime_t trans_time;
    unsigned long long total_time;

    wait_memdump_complete(exception_data);

    ret = chip_reset_main(&exception_data->record, exception_data->excp_chip);
    if (ret != EXCEPTION_SUCCESS) {
        return -EXCEPTION_FAIL;
    }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
    trans_time = ktime_sub(ktime_get(), exception_data->etype_info[exception_data->excetion_type].stime);
    total_time = (unsigned long long)ktime_to_ms(trans_time);
    if (exception_data->etype_info[exception_data->excetion_type].maxtime < total_time) {
        exception_data->etype_info[exception_data->excetion_type].maxtime = total_time;
        ps_print_warning("[MPXX_DFR]time update:%llu,exception_type:%d \n",
                         total_time, exception_data->excetion_type);
    }
#endif
    oam_warning_log2(0, OAM_SF_DFR,
                     "[MPXX_DFR]plat execption recovery success, current time [%d]ms, max time [%d]ms\n",
                     total_time, exception_data->etype_info[exception_data->excetion_type].maxtime);

    /* 每次dfr完成，显示历史dfr处理结果 */
    plat_print_dfr_info();
    return EXCEPTION_SUCCESS;
}

/*
 * 函 数 名  : plat_exception_reset_work
 * 功能描述  : 平台host异常处理work，判断产生异常的系统，调用相应的处理函数。
 *             这个函数会获取mutex，以避免并发处理。
 * 返 回 值  : 异常处理成功返回0，否则返回1
 */
OAL_STATIC void plat_exception_reset_work(oal_work_stru *work)
{
    struct pm_top* pm_top_data = pm_get_top();

    struct st_exception_info *pst_exception_data = get_exception_info_reference();
    if (pst_exception_data == NULL) {
        ps_print_err("[MPXX_DFR]get exception info reference is error\n");
        return;
    }

    ps_print_err("[MPXX_DFR] enter plat_exception_reset_work\n");
    mutex_lock(&(pm_top_data->host_mutex));

    if (plat_exception_reset_handler(pst_exception_data) != EXCEPTION_SUCCESS) {
        ps_print_err("[MPXX_DFR] platform reset handler fail\n");
    }

    atomic_set(&pst_exception_data->is_reseting_device, PLAT_EXCEPTION_RESET_IDLE);

    plat_bbox_msg_hander(pst_exception_data->subsys_type, pst_exception_data->excetion_type);
    mutex_unlock(&(pm_top_data->host_mutex));
}

/*
 * 函 数 名  : plat_exception_reset_init
 * 功能描述  : 平台异常处理模块初始化函数
 */
int32_t plat_exception_reset_init(void)
{
    struct st_exception_info *p_exception_data = NULL;

    p_exception_data = (struct st_exception_info *)kzalloc(sizeof(struct st_exception_info), GFP_KERNEL);
    if (p_exception_data == NULL) {
        ps_print_err("kzalloc p_exception_data is failed!\n");
        return -ENOMEM;
    }

    p_exception_data->wifi_callback = NULL;

    p_exception_data->subsys_type = SUBSYS_BOTTOM;
    p_exception_data->thread_type = BFGX_THREAD_BOTTOM;
    p_exception_data->excetion_type = EXCEPTION_TYPE_BOTTOM;

    p_exception_data->exception_reset_enable = OAL_FALSE;
    p_exception_data->subsystem_rst_en = DFR_TEST_DISABLE;

    atomic_set(&p_exception_data->is_reseting_device, PLAT_EXCEPTION_RESET_IDLE);

    /* 初始化异常处理workqueue和work */
    p_exception_data->plat_exception_rst_workqueue = create_singlethread_workqueue("plat_exception_reset_queue");
    if (p_exception_data->plat_exception_rst_workqueue == NULL) {
        kfree(p_exception_data);
        ps_print_err("create plat_exception workqueue failed\n");
        return -EINTR;
    }

    INIT_WORK(&p_exception_data->plat_exception_rst_work, plat_exception_reset_work);
    oal_wake_lock_init(&p_exception_data->plat_exception_rst_wlock, "mpxx_excep_rst_wlock");

    /* 初始化异常处理自旋锁 */
    oal_spin_lock_init(&p_exception_data->exception_spin_lock);

    /* 调试使用的变量初始化 */
    p_exception_data->sys_rst_download_dbg = OAL_FALSE;
    p_exception_data->sys_rst_uart_suspend = OAL_FALSE;

    g_mpxx_exception_info = p_exception_data;

    /* 芯片级异常资源初始化 */
    p_exception_data->excp_chip = excp_chip_res_init(get_mpxx_subchip_type());
    p_exception_data->record.core = CORE_BUTT;
    p_exception_data->record.subsys_type = EXCP_SYS_BUTT;
    p_exception_data->record.exception_type = EXCEPTION_TYPE_BOTTOM;

    /* 初始化dump文件轮替模块 */
    plat_exception_dump_file_rotate_init();

    hisi_conn_rdr_init();

    ps_print_suc("plat exception reset init success\n");
    return EXCEPTION_SUCCESS;
}

/*
 * 函 数 名  : plat_exception_reset_exit
 * 功能描述  : 平台异常处理模块退出清理函数
 */
int32_t plat_exception_reset_exit(void)
{
    struct st_exception_info *p_exception_data = NULL;

    p_exception_data = g_mpxx_exception_info;
    if (p_exception_data == NULL) {
        ps_print_err("g_mpxx_exception_info is NULL!\n");
        return -EXCEPTION_FAIL;
    }

    destroy_workqueue(p_exception_data->plat_exception_rst_workqueue);
    oal_wake_lock_exit(&p_exception_data->plat_exception_rst_wlock);
    kfree(p_exception_data);
    g_mpxx_exception_info = NULL;
    hisi_conn_rdr_exit();

    ps_print_suc("plat exception reset exit success\n");
    return EXCEPTION_SUCCESS;
}

int32_t in_plat_exception_reset(void)
{
    struct st_exception_info *pst_exception_data = get_exception_info_reference();

    return ((oal_atomic_read(&pst_exception_data->is_reseting_device) == PLAT_EXCEPTION_RESET_BUSY) ?
            OAL_TRUE : OAL_FALSE);
}

OAL_STATIC uint32_t plat_is_dfr_enable(void)
{
    struct st_exception_info *pst_exception_data = get_exception_info_reference();
    if (pst_exception_data == NULL) {
        ps_print_err("get exception info reference is error\n");
        return OAL_FALSE;
    }

    if (pst_exception_data->exception_reset_enable != OAL_TRUE) {
        ps_print_info("plat exception reset not enable!");
        return OAL_FALSE;
    }

    return OAL_TRUE;
}

OAL_STATIC int32_t plat_power_fail_excep_check(uint32_t subsys_type,
                                               uint32_t thread_type,
                                               uint32_t exception_type)
{
    if (subsys_type >= SUBSYS_BOTTOM) {
        ps_print_err("para subsys_type %u is error!\n", subsys_type);
        return -EXCEPTION_FAIL;
    }

    if (((subsys_type == SUBSYS_WIFI) && (thread_type >= WIFI_THREAD_BOTTOM)) ||
        ((subsys_type == SUBSYS_BFGX) && (thread_type >= BFGX_THREAD_BOTTOM))) {
        ps_print_err("para thread_type %u is error! subsys_type is %u\n", thread_type, subsys_type);
        return -EXCEPTION_FAIL;
    }

    if (exception_type >= EXCEPTION_TYPE_BOTTOM) {
        ps_print_err("para exception_type %u is error!\n", exception_type);
        return -EXCEPTION_FAIL;
    }

    if (plat_is_dfr_enable() == OAL_FALSE) {
        ps_print_err("dfr disabled\n");
        return -EXCEPTION_FAIL;
    }

    return EXCEPTION_SUCCESS;
}

/*
 * 函 数 名  : plat_power_fail_info_set
 * 功能描述  : bfgx open失败时，调用该函数设置exception info
 * 输入参数  : subsys_type:调用异常处理的子系统，WIFI或者BFGX
 *             thread_type:子系统中的子线程
 *             exception_type:异常的类型
 */
int32_t plat_power_fail_info_set(uint32_t subsys_type, uint32_t thread_type, uint32_t exception_type)
{
    struct st_exception_info *excp_data = NULL;
    unsigned long flag;

    if (plat_power_fail_excep_check(subsys_type, thread_type, exception_type) !=
        EXCEPTION_SUCCESS) {
        ps_print_err("check failed\n");
        return -EXCEPTION_FAIL;
    }

    excp_data = get_exception_info_reference();
    if (excp_data == NULL) {
        ps_print_err("get exception info reference is error\n");
        return -EXCEPTION_FAIL;
    }

    oal_spin_lock_irq_save(&excp_data->exception_spin_lock, &flag);

    if (in_plat_exception_reset() == OAL_FALSE) {
        get_subsys_core_type(exception_type, &excp_data->record);
        excp_data->subsys_type = subsys_type;
        excp_data->thread_type = thread_type;
        excp_data->excetion_type = exception_type;

        /* 当前异常没有处理完成之前，不允许处理新的异常 */
        ps_print_info("set PLAT_EXCEPTION_RESET_BUSY! subsys=%d, exception type=%d\n",
                      excp_data->subsys_type, excp_data->excetion_type);
        atomic_set(&excp_data->is_reseting_device, PLAT_EXCEPTION_RESET_BUSY);
    } else {
        ps_print_info("plat is processing exception! subsys=%d, exception type=%d\n",
                      excp_data->subsys_type, excp_data->excetion_type);
        oal_spin_unlock_irq_restore(&excp_data->exception_spin_lock, &flag);
        return -EXCEPTION_FAIL;
    }

    /* 增加统计信息 */
    excp_data->etype_info[exception_type].excp_cnt += 1;

    oal_spin_unlock_irq_restore(&excp_data->exception_spin_lock, &flag);

    return EXCEPTION_SUCCESS;
}

/*
 * 函 数 名  : plat_power_fail_process_done
 * 功能描述  : bfgx open失败时，异常处理完成
 */
void plat_power_fail_process_done(uint32_t excp_type)
{
    struct st_exception_info *pst_exception_data = get_exception_info_reference();
    if (oal_unlikely(pst_exception_data == NULL)) {
        ps_print_err("get exception info reference is error\n");
        return;
    }

    atomic_set(&pst_exception_data->is_reseting_device, PLAT_EXCEPTION_RESET_IDLE);
    oam_warning_log1(0, OAM_SF_DFR, "[MPXX_DFR] plat open fail process done, excp_type=%d", excp_type);
    return;
}

/*
 * 函 数 名  : plat_exception_handler
 * 功能描述  : 平台host异常处理总入口函数，填充异常信息，并触发异常处理work
 * 输入参数  : subsys_type:调用异常处理的子系统，WIFI或者BFGX
 *             thread_type:子系统中的子线程
 *             exception_type:异常的类型
 * 返 回 值  : 异常处理成功返回0，否则返回1
 */
int32_t plat_exception_handler(uint32_t subsys_type, uint32_t thread_type, uint32_t exception_type)
{
    struct st_exception_info *excp_data = NULL;
    unsigned long flag;
#ifdef _PRE_CONFIG_GPIO_TO_SSI_DEBUG
    int32_t timeout;
#endif

    if (plat_power_fail_excep_check(subsys_type, thread_type, exception_type) != EXCEPTION_SUCCESS) {
        return -EXCEPTION_FAIL;
    }

    excp_data = get_exception_info_reference();
    if (oal_unlikely(excp_data == NULL || excp_data->excp_chip == NULL)) {
        ps_print_err("[MPXX_DFR] excp_data or excp_chip is NULL\n");
        return -EXCEPTION_FAIL;
    }

    /* 这里只能用spin lock，因为该函数会被心跳超时函数调用，心跳超时函数属于软中断，不允许睡眠 */
    oal_spin_lock_irq_save(&excp_data->exception_spin_lock, &flag);
    if (in_plat_exception_reset() == OAL_FALSE) {
        get_subsys_core_type(exception_type, &excp_data->record);
        excp_data->subsys_type = subsys_type;
        excp_data->thread_type = thread_type;
        excp_data->excetion_type = exception_type;
        /* 当前异常没有处理完成之前，不允许处理新的异常 */
        atomic_set(&excp_data->is_reseting_device, PLAT_EXCEPTION_RESET_BUSY);
    } else {
        ps_print_info("plat is processing exception! subsys=%d, exception type=%d\n",
                      excp_data->subsys_type, excp_data->excetion_type);
        oal_spin_unlock_irq_restore(&excp_data->exception_spin_lock, &flag);
        return EXCEPTION_SUCCESS;
    }
    oal_spin_unlock_irq_restore(&excp_data->exception_spin_lock, &flag);

#ifdef _PRE_CONFIG_GPIO_TO_SSI_DEBUG
    /* 等待SSI操作完成，防止NOC */
    timeout = wait_for_ssi_idle_timeout(MPXX_DFR_WAIT_SSI_IDLE_MS);
    if (timeout <= 0) {
        ps_print_err("[MPXX_DFR]wait for ssi idle failed\n");
        atomic_set(&excp_data->is_reseting_device, PLAT_EXCEPTION_RESET_IDLE);
        return EXCEPTION_FAIL;
    }
    ps_print_info("[MPXX_DFR]wait for ssi idle cost time:%dms\n", MPXX_DFR_WAIT_SSI_IDLE_MS - timeout);
#endif

    excp_data->etype_info[excp_data->excetion_type].stime = ktime_get();
    ps_print_warning("[MPXX_DFR]plat start doing exception! subsys=%d, exception type=%d\n",
                     excp_data->subsys_type, excp_data->excetion_type);

    oal_wake_lock_timeout(&excp_data->plat_exception_rst_wlock, 10 * MSEC_PER_SEC); /* 处理异常，持锁10秒 */
    /* 触发异常处理worker */
    queue_work(excp_data->plat_exception_rst_workqueue, &excp_data->plat_exception_rst_work);

    return EXCEPTION_SUCCESS;
}

EXPORT_SYMBOL(plat_exception_handler);

#ifdef PLATFORM_DEBUG_ENABLE
int32_t is_dfr_test_en(enum excp_test_cfg_em excp_cfg, uintptr_t data)
{
    if (excp_cfg >= EXCP_TEST_CFG_BUTT || g_excp_test_cfg[excp_cfg].data != data) {
        return DFR_TEST_DISABLE;
    }

    if (g_excp_test_cfg[excp_cfg].enable == DFR_TEST_ENABLE) {
        return DFR_TEST_ENABLE;
    }
    return DFR_TEST_DISABLE;
}

void set_excp_test_en(enum excp_test_cfg_em excp_cfg, uintptr_t data)
{
    if (excp_cfg >= EXCP_TEST_CFG_BUTT) {
        return;
    }

    g_excp_test_cfg[excp_cfg].enable = DFR_TEST_ENABLE;
    g_excp_test_cfg[excp_cfg].data = data;
}

void cancel_excp_test_en(enum excp_test_cfg_em excp_cfg, uintptr_t data)
{
    if (excp_cfg >= EXCP_TEST_CFG_BUTT) {
        return;
    }

    g_excp_test_cfg[excp_cfg].enable = DFR_TEST_DISABLE;
    g_excp_test_cfg[excp_cfg].data = data;
}
#endif

STATIC void oal_print_wcpu_reg(uint32_t *pst_buf, uint32_t ul_size)
{
    int32_t i = 0;
    int32_t remain = (int32_t)ul_size; /* per dword */
    if (ul_size) {
        oam_error_log0(0, OAM_SF_ANY, "print wcpu registers:");
    }

    forever_loop() {
        if (remain >= 4) { /* 当需要打印的32bit寄存器个数大于等于4时，一次性打印4个 */
            oam_error_log4(0, OAM_SF_ANY, "wcpu_reg: %x %x %x %x",
                           *(pst_buf + i + 0), *(pst_buf + i + 1),
                           *(pst_buf + i + 2), *(pst_buf + i + 3)); /* 指针向后移动2,3 */
            i += 4; /* 打印变量+4 */
            remain -= 4; /* 寄存器个数-4 */
        } else if (remain >= 3) { /* 当需要打印的32bit寄存器个数等于3时，一次性打印3个 */
            oam_error_log3(0, OAM_SF_ANY, "wcpu_reg: %x %x %x",
                           *(pst_buf + i + 0), *(pst_buf + i + 1),
                           *(pst_buf + i + 2)); /* 指针向后移动2 */
            i += 3; /* 打印变量+3 */
            remain -= 3; /* 寄存器个数-3 */
        } else if (remain >= 2) { /* 当需要打印的32bit寄存器个数等于2时，一次性打印2个 */
            oam_error_log2(0, OAM_SF_ANY, "wcpu_reg: %x %x",
                           *(pst_buf + i + 0), *(pst_buf + i + 1));
            i += 2; /* 打印变量+2 */
            remain -= 2; /* 寄存器个数-2 */
        } else if (remain >= 1) { /* 当需要打印的32bit寄存器个数等于1时，一次性打印1个 */
            oam_error_log1(0, OAM_SF_ANY, "wcpu_reg: %x",
                           *(pst_buf + i + 0));
            i += 1; /* 打印变量+1 */
            remain -= 1; /* 寄存器个数-1 */
        } else {
            break;
        }
    }
}

STATIC int32_t recv_device_memdump(uint8_t *data_buf, int32_t len, memdump_enquenue func)
{
    int32_t l_ret = -EFAIL;
    uint8_t retry = 3;
    int32_t lenbuf = 0;

    if (data_buf == NULL) {
        ps_print_err("data_buf is NULL\n");
        return -EFAIL;
    }

    ps_print_dbg("expect recv len is [%d]\n", len);

    while (len > lenbuf) {
        l_ret = read_msg(data_buf + lenbuf, len - lenbuf);
        if (l_ret > 0) {
            lenbuf += l_ret;
        } else {
            retry--;
            lenbuf = 0;
            if (retry == 0) {
                l_ret = -EFAIL;
                ps_print_err("time out\n");
                break;
            }
        }
    }

    if (len <= lenbuf) {
        func(data_buf, len);
    }

    return l_ret;
}

OAL_STATIC int32_t sdio_read_device_mem(struct st_wifi_dump_mem_info *pst_mem_dump_info, uint8_t *data_buf,
                                        uint32_t data_buf_len, memdump_enquenue func)
{
    uint8_t buf_tx[SEND_BUF_LEN];
    int32_t ret = 0;
    uint32_t size = 0;
    uint32_t offset;
    uint32_t remainder = pst_mem_dump_info->size;

    offset = 0;
    while (remainder > 0) {
        memset_s(buf_tx, SEND_BUF_LEN, 0, SEND_BUF_LEN);

        size = min(remainder, data_buf_len);
        ret = snprintf_s(buf_tx, sizeof(buf_tx), sizeof(buf_tx) - 1, "%s%c0x%lx%c%u%c",
                         RMEM_CMD_KEYWORD,
                         COMPART_KEYWORD,
                         pst_mem_dump_info->mem_addr + offset,
                         COMPART_KEYWORD,
                         size,
                         COMPART_KEYWORD);
        if (ret < 0) {
            ps_print_err("log str format err line[%d]\n", __LINE__);
            break;
        }
        ps_print_dbg("read mem cmd:[%s]\n", buf_tx);
        send_msg(buf_tx, os_str_len(buf_tx));

        ret = recv_device_memdump(data_buf, size, func);
        if (ret < 0) {
            ps_print_err("wifi mem dump fail, filename is [%s],ret=%d\n", pst_mem_dump_info->file_name, ret);
            break;
        }

#ifdef CONFIG_PRINTK
        if (offset == 0) {
            int8_t *pst_file_name = (pst_mem_dump_info->file_name ?
                                       ((int8_t *)pst_mem_dump_info->file_name) : (int8_t *)"default: ");
            if (!oal_strcmp("wifi_device_panic_mem", pst_file_name) && (size > CPU_PANIC_MEMDUMP_SIZE)) {
                oal_print_hex_dump(data_buf, CPU_PANIC_MEMDUMP_SIZE, HEX_DUMP_GROUP_SIZE, pst_file_name);
                /* 因寄存器宽度是4字节，所以这里入参要除4 */
                oal_print_wcpu_reg((uint32_t *)(data_buf), CPU_PANIC_MEMDUMP_SIZE / 4);
            }
        }
#endif

        offset += size;

        remainder -= size;
    }

    return ret;
}

STATIC void cpu_trace_mem_config(hcc_bus *hi_bus)
{
    int32_t ret;
    oal_pcie_linux_res *pst_pci_lres = NULL;

    pst_pci_lres = (oal_pcie_linux_res *)hi_bus->data;
    ret = oal_pcie_slave_soc_config(pst_pci_lres, PCI_CPUTRACE);
    if (ret != OAL_SUCC) {
        ps_print_info("cputrace not config\n");
    }
}

STATIC int32_t dump_wifi_mem(struct st_wifi_dump_mem_info *mem_dump_info, uint32_t count)
{
    uint32_t i;
    int32_t ret = -EFAIL;
    uint8_t *data_buf = NULL;
    uint8_t buff[DEVICE_MEM_DUMP_MAX_BUFF_SIZE];
    uint32_t *pcount = (uint32_t *)&buff[0];
    uint32_t sdio_transfer_limit = hcc_get_max_trans_size(hcc_get_handler(HCC_EP_WIFI_DEV));

    sdio_transfer_limit = oal_min(PAGE_SIZE, sdio_transfer_limit);
    if ((mem_dump_info == NULL) || (sdio_transfer_limit == 0)) {
        ps_print_err("pst_wifi_dump_info is NULL, or sdio_transfer_limit is 0\n");
        return -EFAIL;
    }

    do {
        ps_print_info("try to malloc mem dump buf len is [%d]\n", sdio_transfer_limit);
        data_buf = (uint8_t *)os_kmalloc_gfp(sdio_transfer_limit);
        if (data_buf == NULL) {
            ps_print_warning("malloc mem  len [%d] fail, continue to try in a smaller size\n", sdio_transfer_limit);
            sdio_transfer_limit = sdio_transfer_limit >> 1;
        }
    } while ((data_buf == NULL) && (sdio_transfer_limit >= MIN_FIRMWARE_FILE_TX_BUF_LEN));

    if (data_buf == NULL) {
        ps_print_err("data_buf KMALLOC failed\n");
        return -ENOMEM;
    }

    ps_print_info("mem dump data buf len is [%d]\n", sdio_transfer_limit);

    wifi_notice_hal_memdump();

    for (i = 0; i < count; i++) {
        *pcount = mem_dump_info[i].size;
        ps_print_info("mem dump data size [%d]==> [%d]\n", *pcount, mem_dump_info[i].size);
        wifi_memdump_enquenue(buff, 4); /* 后续申请sk_buff的大小为4 */
        ret = sdio_read_device_mem(&mem_dump_info[i], data_buf, sdio_transfer_limit, wifi_memdump_enquenue);
        if (ret < 0) {
            break;
        }
    }
    wifi_memdump_finish();

    os_mem_kfree(data_buf);
    return ret;
}

/*
 * 函 数 名  : wifi_device_mem_dump
 * 功能描述  : firmware加载时读取wifi的内存
 */
int32_t wifi_device_mem_dump(hcc_bus *hi_bus, int32_t excep_type)
{
    int32_t ret;
    struct st_wifi_dump_mem_info *pst_mem_dump_info = hi_bus->mem_info;
    uint32_t count = hi_bus->mem_size;
    if (!ps_is_device_log_enable()) {
        return 0;
    }

    if (excep_type != HCC_BUS_PANIC && hi_bus->bus_type == HCC_BUS_PCIE) {
        cpu_trace_mem_config(hi_bus);
    }

    ret = dump_wifi_mem(pst_mem_dump_info, count);

    return ret;
}

int32_t gt_device_mem_dump(hcc_bus *hi_bus, int32_t excep_type)
{
    int32_t ret = -EFAIL;
    uint32_t i;
    uint8_t *data_buf = NULL;
    uint8_t buff[DEVICE_MEM_DUMP_MAX_BUFF_SIZE];
    uint32_t *pcount = (uint32_t *)&buff[0];
    uint32_t sdio_transfer_limit = hcc_get_max_trans_size(hcc_get_handler(HCC_EP_GT_DEV));
    struct st_wifi_dump_mem_info *mem_dump_info = hi_bus->mem_info;
    uint32_t count = hi_bus->mem_size;

    if (!ps_is_device_log_enable()) {
        return 0;
    }

    sdio_transfer_limit = oal_min(PAGE_SIZE, sdio_transfer_limit);
    if ((mem_dump_info == NULL) || (sdio_transfer_limit == 0)) {
        ps_print_err("pst_wifi_dump_info is NULL, or sdio_transfer_limit is 0\n");
        return -EFAIL;
    }

    do {
        ps_print_info("try to malloc mem dump buf len is [%d]\n", sdio_transfer_limit);
        data_buf = (uint8_t *)os_kmalloc_gfp(sdio_transfer_limit);
        if (data_buf == NULL) {
            ps_print_warning("malloc mem  len [%d] fail, continue to try in a smaller size\n", sdio_transfer_limit);
            sdio_transfer_limit = sdio_transfer_limit >> 1;
        }
    } while ((data_buf == NULL) && (sdio_transfer_limit >= MIN_FIRMWARE_FILE_TX_BUF_LEN));

    if (data_buf == NULL) {
        ps_print_err("data_buf KMALLOC failed\n");
        return -ENOMEM;
    }

    ps_print_info("mem dump data buf len is [%d]\n", sdio_transfer_limit);

    gt_notice_hal_memdump();

    for (i = 0; i < count; i++) {
        *pcount = mem_dump_info[i].size;
        ps_print_info("mem dump data size [%d]==> [%d]\n", *pcount, mem_dump_info[i].size);
        gt_memdump_enquenue(buff, 4); /* 后续申请sk_buff的大小为4 */
        ret = sdio_read_device_mem(&mem_dump_info[i], data_buf, sdio_transfer_limit, gt_memdump_enquenue);
        if (ret < 0) {
            break;
        }
    }
    gt_memdump_finish();

    os_mem_kfree(data_buf);

    return ret;
}

uint32_t get_panic_excp_by_uart_idx(uint32_t uart_idx)
{
    if (oal_unlikely(uart_idx >= UART_BUTT)) {
        ps_print_err("%s: invalid uart_idx"NEWLINE, __func__);
        return BUART_PANIC;
    }
    return g_uart_panic_type[uart_idx];
}

uint32_t get_wakeup_excp_by_uart_idx(uint32_t uart_idx)
{
    if (oal_unlikely(uart_idx >= UART_BUTT)) {
        ps_print_err("%s: invalid uart_idx"NEWLINE, __func__);
        return BUART_WAKEUP_FAIL;
    }
    return g_uart_wakeup_type[uart_idx];
}

OAL_STATIC excp_chip_cfg_t *get_excp_chip_cfg(uint32_t chip_type)
{
    if (chip_type >= BOARD_VERSION_MPXX_BUTT) {
        return NULL;
    }

    if (oal_unlikely(g_excp_chip_res_cfg[chip_type] == NULL)) {
        ps_print_err("%s: chip_res_cfg func is NULL"NEWLINE, __func__);
        return NULL;
    }

    return g_excp_chip_res_cfg[chip_type]();
}

uint32_t get_poweron_fail_excp(uint32_t sys)
{
    excp_chip_cfg_t *chip_cfg = NULL;

    chip_cfg = get_excp_chip_cfg(get_mpxx_subchip_type());
    if (oal_unlikely(chip_cfg == NULL || chip_cfg->bfgx_poweron_fail == NULL)) {
        ps_print_err("%s: chip_cfg or chip_cfg->bfgx_poweron_fail NULL"NEWLINE, __func__);
        return EXCEPTION_TYPE_BOTTOM;
    }

    if (oal_unlikely(sys >= chip_cfg->poweron_fail_size)) {
        ps_print_err("invalid sys=%d"NEWLINE, sys);
        return EXCEPTION_TYPE_BOTTOM;
    }

    return chip_cfg->bfgx_poweron_fail[sys];
}

uint32_t get_poweroff_fail_excp(uint32_t sys)
{
    excp_chip_cfg_t *chip_cfg = NULL;

    chip_cfg = get_excp_chip_cfg(get_mpxx_subchip_type());
    if (oal_unlikely(chip_cfg == NULL || chip_cfg->bfgx_poweroff_fail == NULL)) {
        ps_print_err("%s: chip_cfg or chip_cfg->bfgx_poweroff_fail NULL"NEWLINE, __func__);
        return EXCEPTION_TYPE_BOTTOM;
    }

    if (oal_unlikely(sys >= chip_cfg->poweroff_fail_size)) {
        ps_print_err("invalid sys=%d"NEWLINE, sys);
        return EXCEPTION_TYPE_BOTTOM;
    }

    return chip_cfg->bfgx_poweroff_fail[sys];
}

OAL_STATIC int32_t excp_core_res_init(excp_core_res_t *core, excp_core_cfg_t *core_cfg)
{
    uint32_t i;

    core->master_cfg = core_cfg;
    core->sla_core_cnt = core_cfg->sla_core_cnt;

    core->sla_cores = oal_memalloc(sizeof(excp_sla_res_t) * core_cfg->sla_core_cnt);
    if (core->sla_cores == NULL) {
        ps_print_err("core->sla_cores alloc fail"NEWLINE);
        return OAL_FAIL;
    }
    memset_s(core->sla_cores, sizeof(excp_sla_res_t) * core_cfg->sla_core_cnt,
             0, sizeof(excp_sla_res_t) * core_cfg->sla_core_cnt);

    core->sla_excp_arr = oal_memalloc(sizeof(uint32_t) * core_cfg->sla_core_cnt);
    if (core->sla_excp_arr == NULL) {
        ps_print_err("core->sla_excp_arr alloc fail"NEWLINE);
        oal_free(core->sla_cores);
        return OAL_FAIL;
    }
    core->sla_excp_size = core_cfg->sla_core_cnt;
    memset_s(core->sla_excp_arr, sizeof(uint32_t) * core_cfg->sla_core_cnt,
             0, sizeof(uint32_t) * core_cfg->sla_core_cnt);

    if (oal_unlikely(core_cfg->sla_core_cfg == NULL)) {
        ps_print_err("master_core=%s, sla_core_cfg is NULL"NEWLINE, core_cfg->master_core_name);
        oal_free(core->sla_excp_arr);
        oal_free(core->sla_cores);
        return OAL_FAIL;
    }

    for (i = 0; i < core_cfg->sla_core_cnt; i++) {
        core->sla_cores[i].sla_cfg = &core_cfg->sla_core_cfg[i];
    }

    ps_print_info("%s: master_core_name=%s, init succ"NEWLINE, __func__, core_cfg->master_core_name);
    return OAL_SUCC;
}

OAL_STATIC int32_t excp_core_res_deinit(excp_core_res_t *core)
{
    oal_free(core->sla_cores);
    core->sla_cores = NULL;
    oal_free(core);
    return OAL_SUCC;
}

OAL_STATIC int32_t excp_subsys_res_init(excp_subsys_res_t *subsys, excp_subsys_cfg_t *subsys_cfg)
{
    int32_t ret;
    if (oal_unlikely(subsys_cfg == NULL)) {
        ps_print_err("subsys_cfg is NULL"NEWLINE);
        return OAL_FAIL;
    }

    subsys->subsys_cfg = subsys_cfg;

    if (subsys_cfg->core_cfg != NULL) {
        subsys->core_attr = oal_memalloc(sizeof(excp_core_res_t));
        if (subsys->core_attr == NULL) {
            ps_print_err("subsys->core_attr alloc fail"NEWLINE);
            return OAL_FAIL;
        }
        memset_s(subsys->core_attr, sizeof(excp_core_res_t), 0, sizeof(excp_core_res_t));

        ret = excp_core_res_init(subsys->core_attr, subsys_cfg->core_cfg);
        if (ret != OAL_SUCC) {
            ps_print_err("excp_core_res_init fail"NEWLINE);
            oal_free(subsys->core_attr);
            return OAL_FAIL;
        }
    }
    ps_print_info("%s: subsys_name=%s, init succ"NEWLINE, __func__, subsys_cfg->subsys_name);
    return OAL_SUCC;
}

excp_chip_res_t *get_excp_chip_res(void)
{
    return g_excp_chip_res;
}

OAL_STATIC excp_chip_res_t *excp_chip_res_init(uint32_t chip_type)
{
    int32_t ret;
    uint32_t i;
    excp_chip_res_t *chip_res = NULL;
    excp_chip_cfg_t *chip_cfg = get_excp_chip_cfg(chip_type);

    if (oal_unlikely(chip_cfg == NULL)) {
        ps_print_err("chip_cfg is NULL, chip_type=%d"NEWLINE, chip_type);
        return NULL;
    }

    chip_res = oal_memalloc(sizeof(excp_chip_res_t));
    if (chip_res == NULL) {
        ps_print_err("chip_res alloc fail"NEWLINE);
        return NULL;
    }
    memset_s(chip_res, sizeof(excp_chip_res_t), 0, sizeof(excp_chip_res_t));

    chip_res->chip_cfg = chip_cfg;
    chip_res->subsys_cnt = chip_cfg->subsys_cnt;
    chip_res->subsys_attr = oal_memalloc(sizeof(excp_subsys_res_t) * chip_cfg->subsys_cnt);
    if (chip_res->subsys_attr == NULL) {
        ps_print_err("subsys_attr alloc fail"NEWLINE);
        oal_free(chip_res);
        return NULL;
    }
    memset_s(chip_res->subsys_attr, sizeof(excp_subsys_res_t) * chip_cfg->subsys_cnt,
             0, sizeof(excp_subsys_res_t) * chip_cfg->subsys_cnt);
    for (i = 0; i < chip_cfg->subsys_cnt; i++) {
        ret = excp_subsys_res_init(&chip_res->subsys_attr[i], &chip_cfg->subsys_cfg[i]);
        if (ret != OAL_SUCC) {
            ps_print_err("excp_subsys_res_init fail"NEWLINE);
            oal_free(chip_res->subsys_attr);
            oal_free(chip_res);
            return NULL;
        }
    }

    g_excp_chip_res = chip_res;
    ps_print_info("%s: chip_type=%d, chip_name=%s, init succ"NEWLINE, __func__, chip_type, chip_cfg->chip_name);
    return chip_res;
}

int32_t excp_chip_res_deinit(excp_chip_res_t *chip_res, uint32_t chip_type)
{
    uint32_t i;
    excp_chip_cfg_t *chip_cfg = get_excp_chip_cfg(chip_type);

    if (oal_unlikely(chip_cfg == NULL)) {
        ps_print_err("chip_cfg is NULL, chip_type=%d"NEWLINE, chip_type);
        return OAL_FAIL;
    }

    for (i = 0; i < chip_cfg->subsys_cnt; i++) {
        excp_core_res_deinit(chip_res->subsys_attr[i].core_attr);
        chip_res->subsys_attr[i].core_attr = NULL;
    }

    oal_free(chip_res->subsys_attr);
    chip_res->subsys_attr = NULL;
    oal_free(chip_res);
    chip_res = NULL;
    return OAL_SUCC;
}

OAL_STATIC uint32_t excp_find_core_idx(uint32_t core_type, excp_core_res_t *core_res)
{
    uint32_t i;
    excp_sla_core_cfg_t *sla_cfg = NULL;

    for (i = 0; i < core_res->sla_core_cnt; i++) {
        sla_cfg = core_res->sla_cores[i].sla_cfg;
        if (sla_cfg->sla_core_type == core_type) {
            break;
        }
    }

    return i;
}

OAL_STATIC uint32_t reset_firmware_proc(reset_handler_t *reset_handler, excp_comm_t *node_ptr)
{
    if (reset_handler->firmware_download_pre != NULL) {
        if (reset_handler->firmware_download_pre(node_ptr) != EXCEPTION_SUCCESS) {
            ps_print_err("firmware_download_pre failed"NEWLINE);
            return -EXCEPTION_FAIL;
        }
    }

    if (reset_handler->firmware_download_do != NULL) {
        if (reset_handler->firmware_download_do(node_ptr) != EXCEPTION_SUCCESS) {
            if (reset_handler->firmware_download_fail_do != NULL) {
                reset_handler->firmware_download_fail_do(node_ptr);
            }
            ps_print_err("firmware_download_do failed"NEWLINE);
            return -EXCEPTION_FAIL;
        }
    }

    if (reset_handler->firmware_download_post != NULL) {
        if (reset_handler->firmware_download_post(node_ptr) != EXCEPTION_SUCCESS) {
            ps_print_err("firmware_download_post failed"NEWLINE);
            return -EXCEPTION_FAIL;
        }
    }

    return EXCEPTION_SUCCESS;
}

int32_t core_reset_main(excp_record_t *excp_record, excp_core_res_t *core_res)
{
    excp_sla_res_t *sla_core = NULL;
    excp_sla_core_cfg_t *sla_cfg = NULL;
    uint32_t core_idx;

    if (oal_unlikely(core_res == NULL || core_res->master_cfg == NULL || core_res->sla_cores == NULL)) {
        ps_print_err("core_res, master_cfg or sla_cores is NULL"NEWLINE);
        return -EXCEPTION_FAIL;
    }

    core_idx = excp_find_core_idx(excp_record->core, core_res);
    if (oal_unlikely(core_idx == core_res->sla_core_cnt)) {
        return -EXCEPTION_FAIL;
    }

    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                         CHR_PLT_DRV_EVENT_RST, CHR_PLAT_DRV_ERROR_CORE_RST);

    sla_cfg = core_res->sla_cores[core_idx].sla_cfg;
    ps_print_info("reset slave_core=%s, core_type=%d"NEWLINE, sla_cfg->sla_core_name, sla_cfg->sla_core_type);
    if (oal_unlikely(sla_cfg->sla_reset_func == NULL)) {
        ps_print_err("sla_reset_func is NULL"NEWLINE);
        return -EXCEPTION_FAIL;
    }

    sla_core = &core_res->sla_cores[core_idx];

    if (sla_cfg->sla_reset_func->pre_do != NULL) {
        if (sla_cfg->sla_reset_func->pre_do(&sla_core->node, excp_record) != EXCEPTION_SUCCESS) {
            ps_print_err("%s: pre_do failed"NEWLINE, __func__);
            return -EXCEPTION_FAIL;
        }
    }

    if (sla_cfg->sla_reset_func->power_reset != NULL) {
        if (sla_cfg->sla_reset_func->power_reset(&sla_core->node) != EXCEPTION_SUCCESS) {
            ps_print_err("%s: power_reset failed"NEWLINE, __func__);
            return -EXCEPTION_FAIL;
        }
    }

    if (reset_firmware_proc(sla_cfg->sla_reset_func, &sla_core->node) != EXCEPTION_SUCCESS) {
        ps_print_err("%s: reset_firmware_proc failed"NEWLINE, __func__);
        return -EXCEPTION_FAIL;
    }

    if (sla_cfg->sla_reset_func->post_do != NULL) {
        if (sla_cfg->sla_reset_func->post_do(&sla_core->node, excp_record) != EXCEPTION_SUCCESS) {
            ps_print_err("%s: post_do failed"NEWLINE, __func__);
            return -EXCEPTION_FAIL;
        }
    }

    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                         CHR_PLT_DRV_EVENT_RST, CHR_PLAT_DRV_ERROR_CORE_RST_SUCC);
    oam_warning_log1(0, OAM_SF_DFR, "[MPXX_DFR] core reset success, core=%d\n", sla_cfg->sla_core_type);
    return EXCEPTION_SUCCESS;
}

int32_t subsys_reset_main(excp_record_t *excp_record, excp_subsys_res_t *subsys)
{
    int32_t ret;
    reset_handler_t* subsys_rst_handler = NULL;
    excp_subsys_cfg_t *subsys_cfg = NULL;

    if (oal_unlikely(subsys == NULL || subsys->subsys_cfg == NULL || subsys->subsys_cfg->subsys_reset_func == NULL)) {
        ps_print_err("subsys, subsys_cfg or subsys_reset_func is NULL"NEWLINE);
        return -EXCEPTION_FAIL;
    }

    subsys_cfg = subsys->subsys_cfg;
    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                         CHR_PLT_DRV_EVENT_RST, CHR_PLAT_DRV_ERROR_SUB_SYSTEM_RST);
    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                         CHR_PLT_DRV_EVENT_RST, subsys_cfg->chr_id);

    ret = core_reset_main(excp_record, subsys->core_attr);
    if (ret == EXCEPTION_SUCCESS) {
        return ret;
    }

    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                         CHR_PLT_DRV_EVENT_RST, CHR_PLAT_DRV_ERROR_CORE_RST_FAIL);
    oam_warning_log0(0, OAM_SF_DFR, "[MPXX_DFR] try subsys reset");
    ps_print_info("rst subsys=%s, subsys_type=%d\n", subsys_cfg->subsys_name, subsys_cfg->subsys_type);

    subsys_rst_handler = subsys_cfg->subsys_reset_func;
    if (subsys_rst_handler->pre_do != NULL) {
        if (subsys_rst_handler->pre_do(&subsys->node, excp_record) != EXCEPTION_SUCCESS) {
            ps_print_err("%s: pre_do failed"NEWLINE, __func__);
            return -EXCEPTION_FAIL;
        }
    }

    if (subsys_rst_handler->power_reset != NULL) {
        if (subsys_rst_handler->power_reset(&subsys->node) != EXCEPTION_SUCCESS) {
            ps_print_err("%s: power_reset failed"NEWLINE, __func__);
            return -EXCEPTION_FAIL;
        }
    }

    if (reset_firmware_proc(subsys_rst_handler, &subsys->node) != EXCEPTION_SUCCESS) {
        ps_print_err("%s: reset_firmware_proc failed"NEWLINE, __func__);
        return -EXCEPTION_FAIL;
    }

    if (subsys_rst_handler->post_do != NULL) {
        if (subsys_rst_handler->post_do(&subsys->node, excp_record) != EXCEPTION_SUCCESS) {
            ps_print_err("%s: post_do failed"NEWLINE, __func__);
            return -EXCEPTION_FAIL;
        }
    }

    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                         CHR_PLT_DRV_EVENT_RST, CHR_PLAT_DRV_ERROR_SUB_SYSTEM_RST_SUCC);
    oam_warning_log1(0, OAM_SF_DFR, "[MPXX_DFR] subsys reset success, subsys=%d", subsys->subsys_cfg->subsys_type);
    return EXCEPTION_SUCCESS;
}

OAL_STATIC uint32_t excp_find_subsys_idx(uint32_t subsys_type, excp_chip_res_t *chip_res)
{
    uint32_t i;
    excp_subsys_cfg_t *subsys_cfg = NULL;

    for (i = 0; i < chip_res->subsys_cnt; i++) {
        subsys_cfg = chip_res->subsys_attr[i].subsys_cfg;
        if (subsys_cfg->subsys_type == subsys_type) {
            break;
        }
    }

    return i;
}

OAL_STATIC void wait_memdump_complete(struct st_exception_info *excp_data)
{
    uint32_t i;
    uint32_t bus_id = UART_BUTT;
    excp_chip_res_t *chip_res = NULL;
    struct ps_core_s *ps_core_d = NULL;

    if (oal_unlikely(excp_data == NULL || excp_data->excp_chip == NULL || excp_data->excp_chip->chip_cfg == NULL)) {
        ps_print_err("excp_data, excp_chip or chip_cfg is NULL\n");
        return;
    }

    chip_res = excp_data->excp_chip;
    if (oal_unlikely(chip_res->chip_cfg->panic_table == NULL)) {
        ps_print_err("panic_table is NULL\n");
        return;
    }

    for (i = 0; i < chip_res->chip_cfg->panic_table_size; i++) {
        if (chip_res->chip_cfg->panic_table[i].exception_type == excp_data->record.exception_type) {
            bus_id = chip_res->chip_cfg->panic_table[i].bus_id;
            break;
        }
    }

    if (i == chip_res->chip_cfg->panic_table_size) {
        ps_print_info("cannot find match bus_id, do nothing\n");
        return;
    }

    ps_core_d = ps_get_core_reference(bus_id);
    bfgx_dump_stack(ps_core_d);
}

OAL_STATIC int32_t chip_reset_proc(excp_record_t *excp_record, excp_chip_res_t *chip_res)
{
    reset_handler_t* chip_reset_handler = chip_res->chip_cfg->chip_reset_func;

    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                         CHR_PLT_DRV_EVENT_RST, CHR_PLAT_DRV_ERROR_ALL_SYSTEM_RST);

    ps_print_info("rst chip=%s, chip_type=%d\n", chip_res->chip_cfg->chip_name, chip_res->chip_cfg->chip_type);

    if (oal_unlikely(chip_reset_handler == NULL)) {
        ps_print_err("%s: chip_reset_handler is NULL"NEWLINE, __func__);
        return -EXCEPTION_FAIL;
    }

    if (chip_reset_handler->pre_do != NULL) {
        if (chip_reset_handler->pre_do(&chip_res->node, excp_record) != EXCEPTION_SUCCESS) {
            oam_error_log0(0, OAM_SF_DFR, "[MPXX_DFR] chip_reset_proc: pre_do failed");
            return -EXCEPTION_FAIL;
        }
    }

    if (chip_reset_handler->power_reset != NULL) {
        if (chip_reset_handler->power_reset(&chip_res->node) != EXCEPTION_SUCCESS) {
            oam_error_log0(0, OAM_SF_DFR, "[MPXX_DFR] chip_reset_proc: power_reset failed");
            return -EXCEPTION_FAIL;
        }
    }

    if (reset_firmware_proc(chip_reset_handler, &chip_res->node) != EXCEPTION_SUCCESS) {
        oam_error_log0(0, OAM_SF_DFR, "[MPXX_DFR] chip_reset_proc: reset_firmware_proc failed");
        return -EXCEPTION_FAIL;
    }

    if (chip_reset_handler->post_do != NULL) {
        if (chip_reset_handler->post_do(&chip_res->node, excp_record) != EXCEPTION_SUCCESS) {
            oam_error_log0(0, OAM_SF_DFR, "[MPXX_DFR] chip_reset_proc: post_do failed");
            return -EXCEPTION_FAIL;
        }
    }

    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                         CHR_PLT_DRV_EVENT_RST, CHR_PLAT_DRV_ERROR_ALL_SYSTEM_RST_SUCC);
    oam_warning_log1(0, OAM_SF_DFR, "[MPXX_DFR] chip reset success, chip=%d", chip_res->chip_cfg->chip_type);
    return EXCEPTION_SUCCESS;
}

OAL_STATIC void excp_all_subsys_state_print(excp_chip_res_t *chip_res)
{
    uint32_t i;
    int32_t state;
    excp_subsys_res_t *subsys_res = NULL;
    excp_subsys_cfg_t *subsys_cfg = NULL;
    struct pm_drv_data *pm_drv_d = NULL;

    for (i = 0; i < chip_res->subsys_cnt; i++) {
        subsys_res = &chip_res->subsys_attr[i];
        subsys_cfg = subsys_res->subsys_cfg;

        if (oal_unlikely(subsys_cfg->subsys_reset_func->get_power_state == NULL)) {
            ps_print_err("get_power_state is NULL"NEWLINE);
            return;
        }

        state = subsys_cfg->subsys_reset_func->get_power_state(&subsys_res->node);
        ps_print_info("subsys=%s:%d"NEWLINE, subsys_cfg->subsys_name, state);
        if (subsys_cfg->subsys_type != EXCP_W_SYS) {
            pm_drv_d = (struct pm_drv_data*)subsys_res->subsys_pm;
            bfgx_print_subsys_state(pm_drv_d->index);
        }
    }
}

OAL_STATIC uint32_t ir_mode_need_dfr(uint32_t subsys_type, excp_chip_res_t *chip_res)
{
    uint32_t ret = OAL_TRUE;
    excp_chip_cfg_t *chip_cfg = chip_res->chip_cfg;

    if (chip_cfg->chip_reset_func != NULL && chip_cfg->chip_reset_func->is_ir_mode_need_dfr != NULL) {
        ret = chip_cfg->chip_reset_func->is_ir_mode_need_dfr(subsys_type);
    }

    if (ps_core_ir_only_mode() == OAL_TRUE) {
        hw_bfgx_close(bfgx_get_core_by_dev_name(HW_IR_DEV_NAME), BFGX_IR);
        ps_print_warning("exit ir only mode\n");
    }
    return ret;
}

int32_t chip_reset_main(excp_record_t *excp_record, excp_chip_res_t *chip_res)
{
    int32_t ret;
    uint32_t subsys_idx;

    if (oal_unlikely(chip_res == NULL || chip_res->chip_cfg == NULL || chip_res->subsys_attr == NULL)) {
        ps_print_err("chip_res, chip_cfg or subsys_attr is NULL"NEWLINE);
        return -EXCEPTION_FAIL;
    }

    oam_warning_log3(0, OAM_SF_DFR, "[MPXX_DFR] enter chip_reset_main, excp_type=%d, subsys_type=%d, core=%d",
                     excp_record->exception_type, excp_record->subsys_type, excp_record->core);

    if (ir_mode_need_dfr(excp_record->subsys_type, chip_res) != OAL_TRUE) {
        ps_print_warning("ir noly mode, skip dfr"NEWLINE);
        return EXCEPTION_SUCCESS;
    }

    excp_all_subsys_state_print(chip_res);
    if (excp_record->subsys_type == EXCP_ALL_SYS) {
        ret = chip_reset_proc(excp_record, chip_res);
        if (ret != EXCEPTION_SUCCESS) {
            chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                                 CHR_PLT_DRV_EVENT_RST, CHR_PLAT_DRV_ERROR_ALL_SYSTEM_RST_FAIL);
            dfr_fail_power_off(chip_res);
        }
        return ret;
    }

    subsys_idx = excp_find_subsys_idx(excp_record->subsys_type, chip_res);
    if (oal_unlikely(subsys_idx == chip_res->subsys_cnt)) {
        ps_print_err("%s: invalid subsys_type=%d"NEWLINE, __func__, excp_record->subsys_type);
        return -EXCEPTION_FAIL;
    }

    if (is_subsystem_rst_enable() != DFR_TEST_ENABLE) {
        ret = subsys_reset_main(excp_record, &chip_res->subsys_attr[subsys_idx]);
        if (ret == EXCEPTION_SUCCESS) {
            return ret;
        }
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_RST, CHR_PLAT_DRV_ERROR_SUB_SYSTEM_RST_FAIL);
        oam_warning_log0(0, OAM_SF_DFR, "[MPXX_DFR] try chip reset");
    }

    excp_record->subsys_type = EXCP_ALL_SYS;
    ret = chip_reset_proc(excp_record, chip_res);
    if (ret != EXCEPTION_SUCCESS) {
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_RST, CHR_PLAT_DRV_ERROR_ALL_SYSTEM_RST_FAIL);
        dfr_fail_power_off(chip_res);
    }
    return ret;
}


OAL_STATIC uint32_t excp_is_single_subsys(excp_chip_res_t *chip_res)
{
    uint32_t i;
    uint32_t open_cnt = 0;
    excp_subsys_res_t *subsys_res = NULL;
    excp_subsys_cfg_t *subsys_cfg = NULL;

    for (i = 0; i < chip_res->subsys_cnt; i++) {
        subsys_res = &chip_res->subsys_attr[i];
        subsys_cfg = subsys_res->subsys_cfg;
        if (oal_unlikely(subsys_cfg->subsys_reset_func->get_power_state == NULL)) {
            ps_print_err("get_power_state is NULL"NEWLINE);
            return OAL_FALSE;
        }

        if (subsys_cfg->subsys_reset_func->get_power_state(&subsys_res->node) == POWER_STATE_OPEN) {
            open_cnt++;
        }
    }

    return open_cnt == 1 ? OAL_TRUE : OAL_FALSE;
}

OAL_STATIC uint32_t get_chip_force_reset_status(excp_chip_cfg_t *chip_cfg)
{
    if ((chip_cfg->chip_reset_func->is_chip_force_reset != NULL) &&
        (chip_cfg->chip_reset_func->is_chip_force_reset() == OAL_TRUE)) {
        ps_print_info("chip force reset, all exception will drf all subsys!"NEWLINE);
        return OAL_TRUE;
    } else {
        return OAL_FALSE;
    }
}

OAL_STATIC void get_subsys_core_type(uint32_t exception_type, excp_record_t *excp_record)
{
    excp_chip_res_t *chip_res = get_excp_chip_res();
    excp_chip_cfg_t *chip_cfg = NULL;
    excp_search_table_t *excp_callback = NULL;

    if (oal_unlikely(chip_res == NULL)) {
        ps_print_err("%s: chip_res is NULL"NEWLINE, __func__);
        return;
    }

    chip_cfg = chip_res->chip_cfg;
    if (oal_unlikely(chip_cfg == NULL || chip_cfg->excp_search == NULL || chip_cfg->chip_reset_func == NULL)) {
        ps_print_err("chip_cfg, excp_search or chip_reset_func is NULL"NEWLINE);
        return;
    }

    if (oal_unlikely(exception_type >= chip_cfg->excp_search_size)) {
        ps_print_err("invalid exception_type=%d"NEWLINE, exception_type);
        return;
    }

    excp_record->exception_type = exception_type;
    excp_callback = &chip_cfg->excp_search[exception_type];
    if (oal_unlikely(exception_type != excp_callback->exception_type)) {
        ps_print_err("%s: invalid exception_type=%d, callback_type=%d"NEWLINE, __func__, exception_type,
                     excp_callback->exception_type);
        return;
    }

    if ((excp_is_single_subsys(chip_res) == OAL_TRUE) || (get_chip_force_reset_status(chip_cfg) == OAL_TRUE)) {
        ps_print_info("single subsys"NEWLINE);
        excp_record->subsys_type = EXCP_ALL_SYS;
        excp_record->core = ALL_CORE;
    } else if (excp_callback->get_subsys_core_func != NULL) {
        excp_callback->get_subsys_core_func(excp_record);
    } else {
        excp_record->subsys_type = excp_callback->subsys_type;
        excp_record->core = excp_callback->core_type;
    }

    ps_print_info("exception_type=%d, subsys_type=%d, core_type=%d"NEWLINE, excp_record->exception_type,
                  excp_record->subsys_type, excp_record->core);
}


OAL_STATIC int32_t cores_handler_check(excp_core_res_t *core_attr)
{
    uint32_t i;
    excp_sla_res_t *sla_core = NULL;
    excp_sla_core_cfg_t *sla_cfg = NULL;

    if (oal_unlikely(core_attr == NULL || core_attr->master_cfg == NULL ||
        core_attr->master_cfg->master_reset_func == NULL)) {
        ps_print_info("core_attr, master_cfg or master_reset_func is NULL\n");
        return -EXCEPTION_FAIL;
    }

    for (i = 0; i < core_attr->sla_core_cnt; i++) {
        sla_core = &core_attr->sla_cores[i];
        sla_cfg = sla_core->sla_cfg;
        if (oal_unlikely(sla_cfg == NULL || sla_cfg->sla_reset_func == NULL)) {
            ps_print_err("sla_cfg or sla_reset_func is NULL\n");
            return -EXCEPTION_FAIL;
        }
    }
    return EXCEPTION_SUCCESS;
}


int32_t subsys_all_core_pre_proc(excp_comm_t *node_ptr, excp_record_t *excp_record)
{
    uint32_t i;
    int32_t ret = -EXCEPTION_FAIL;
    excp_subsys_res_t *subsys_res = oal_container_of(node_ptr, excp_subsys_res_t, node);
    excp_core_res_t *core_attr = NULL;
    excp_sla_res_t *sla_core = NULL;
    excp_sla_core_cfg_t *sla_cfg = NULL;
    excp_core_cfg_t *master_cfg = NULL;

    /* 在pre_do处理检查一次，后面加载，post_do等不再判断 */
    if (cores_handler_check(subsys_res->core_attr) != EXCEPTION_SUCCESS) {
        return ret;
    }

    core_attr = subsys_res->core_attr;
    master_cfg = core_attr->master_cfg;
    ps_print_info("master_core=%s, subsys_all_core_pre_proc start\n", master_cfg->master_core_name);

    for (i = 0; i < core_attr->sla_core_cnt; i++) {
        sla_core = &core_attr->sla_cores[i];
        sla_cfg = sla_core->sla_cfg;

        if (sla_cfg->sla_reset_func->get_power_state != NULL) {
            if (sla_cfg->sla_reset_func->get_power_state(&sla_core->node) == POWER_STATE_SHUTDOWN) {
                continue;
            }
        }

        if (sla_cfg->sla_reset_func->pre_do != NULL) {
            ps_print_info("sla_core=%s pre_do\n", sla_cfg->sla_core_name);
            ret = sla_cfg->sla_reset_func->pre_do(&sla_core->node, excp_record);
            if (ret != EXCEPTION_SUCCESS) {
                return ret;
            }
        }
    }

    if (master_cfg->master_reset_func->pre_do != NULL) {
        ps_print_info("master_core=%s pre_do\n", master_cfg->master_core_name);
        ret = master_cfg->master_reset_func->pre_do(&core_attr->node, excp_record);
    }

    return ret;
}

int32_t subsys_all_core_download_fail_do(excp_comm_t *node_ptr)
{
    uint32_t i;
    int32_t ret = -EXCEPTION_FAIL;
    excp_subsys_res_t *subsys_res = oal_container_of(node_ptr, excp_subsys_res_t, node);
    excp_core_res_t *core_attr = subsys_res->core_attr;
    excp_core_cfg_t *master_cfg = core_attr->master_cfg;
    excp_sla_res_t *sla_core = NULL;
    excp_sla_core_cfg_t *sla_cfg = NULL;

    ps_print_info("master_core=%s, subsys_all_core_download_fail_do start\n", master_cfg->master_core_name);

    for (i = 0; i < core_attr->sla_core_cnt; i++) {
        sla_core = &core_attr->sla_cores[i];
        sla_cfg = sla_core->sla_cfg;

        if (sla_cfg->sla_reset_func->get_power_state != NULL) {
            if (sla_cfg->sla_reset_func->get_power_state(&sla_core->node) == POWER_STATE_SHUTDOWN) {
                continue;
            }
        }

        if (sla_cfg->sla_reset_func->firmware_download_fail_do != NULL) {
            ps_print_info("sla_core=%s firmware_download_fail_do\n", sla_cfg->sla_core_name);
            ret = sla_cfg->sla_reset_func->firmware_download_fail_do(&sla_core->node);
            if (ret != EXCEPTION_SUCCESS) {
                return ret;
            }
        }
    }

    if (master_cfg->master_reset_func->firmware_download_fail_do != NULL) {
        ps_print_info("master_core=%s firmware_download_fail_do\n", master_cfg->master_core_name);
        ret = master_cfg->master_reset_func->firmware_download_fail_do(&core_attr->node);
    }

    return ret;
}

int32_t subsys_all_core_firmware_download_post(excp_comm_t *node_ptr)
{
    uint32_t i;
    int32_t ret = -EXCEPTION_FAIL;
    excp_subsys_res_t *subsys_res = oal_container_of(node_ptr, excp_subsys_res_t, node);
    excp_core_res_t *core_attr = subsys_res->core_attr;
    excp_core_cfg_t *master_cfg = core_attr->master_cfg;
    excp_sla_res_t *sla_core = NULL;
    excp_sla_core_cfg_t *sla_cfg = NULL;

    ps_print_info("master_core=%s, subsys_all_core_firmware_download_post start\n", master_cfg->master_core_name);

    if (master_cfg->master_reset_func->firmware_download_post != NULL) {
        ps_print_info("master_core=%s firmware_download_post\n", master_cfg->master_core_name);
        ret = master_cfg->master_reset_func->firmware_download_post(&core_attr->node);
    }

    for (i = 0; i < core_attr->sla_core_cnt; i++) {
        sla_core = &core_attr->sla_cores[i];
        sla_cfg = sla_core->sla_cfg;

        if (sla_cfg->sla_reset_func->get_power_state != NULL) {
            if (sla_cfg->sla_reset_func->get_power_state(&sla_core->node) == POWER_STATE_SHUTDOWN) {
                continue;
            }
        }

        if (sla_cfg->sla_reset_func->firmware_download_post != NULL) {
            ps_print_info("sla_core=%s firmware_download_post\n", sla_cfg->sla_core_name);
            ret = sla_cfg->sla_reset_func->firmware_download_post(&sla_core->node);
            if (ret != EXCEPTION_SUCCESS) {
                return ret;
            }
        }
    }

    return ret;
}

int32_t subsys_all_core_reset_post_do(excp_comm_t *node_ptr, excp_record_t *excp_record)
{
    uint32_t i;
    int32_t ret = -EXCEPTION_FAIL;
    excp_subsys_res_t *subsys_res = oal_container_of(node_ptr, excp_subsys_res_t, node);
    excp_core_res_t *core_attr = subsys_res->core_attr;
    excp_core_cfg_t *master_cfg = core_attr->master_cfg;
    excp_sla_res_t *sla_core = NULL;
    excp_sla_core_cfg_t *sla_cfg = NULL;

    ps_print_info("master_core=%s, subsys_all_core_reset_post_do start\n", master_cfg->master_core_name);

    if (master_cfg->master_reset_func->post_do != NULL) {
        ps_print_info("master_core=%s post_do\n", master_cfg->master_core_name);
        ret = master_cfg->master_reset_func->post_do(&core_attr->node, excp_record);
        if (ret != EXCEPTION_SUCCESS) {
            return ret;
        }
    }

    for (i = 0; i < core_attr->sla_core_cnt; i++) {
        sla_core = &core_attr->sla_cores[i];
        sla_cfg = sla_core->sla_cfg;

        if (sla_cfg->sla_reset_func->get_power_state != NULL) {
            if (sla_cfg->sla_reset_func->get_power_state(&sla_core->node) == POWER_STATE_SHUTDOWN) {
                continue;
            }
        }

        if (sla_cfg->sla_reset_func->post_do != NULL) {
            ps_print_info("sla_core=%s post_do\n", sla_cfg->sla_core_name);
            ret = sla_cfg->sla_reset_func->post_do(&sla_core->node, excp_record);
            if (ret != EXCEPTION_SUCCESS) {
                return ret;
            }
            core_attr->sla_excp_stats[excp_record->exception_type]++;
            core_attr->sla_excp_arr[i]++;
            core_attr->sla_excp_total_cnt++;
        }
    }

    subsys_res->subsys_excp_stats[excp_record->exception_type]++;
    subsys_res->subsys_excp_cnt++;

    return ret;
}

int32_t chip_mpxx_pre_do(excp_comm_t *node_ptr, excp_record_t *excp_record)
{
    uint32_t i;
    int32_t ret = EXCEPTION_SUCCESS;
    excp_subsys_res_t *subsys_res = NULL;
    excp_subsys_cfg_t *subsys_cfg = NULL;
    excp_chip_res_t *chip_res = oal_container_of(node_ptr, excp_chip_res_t, node);

    for (i = 0; i < chip_res->subsys_cnt; i++) {
        subsys_res = &chip_res->subsys_attr[i];
        subsys_cfg = subsys_res->subsys_cfg;

        if (oal_unlikely(subsys_cfg->subsys_reset_func->get_power_state == NULL)) {
            ps_print_err("get_power_state is NULL"NEWLINE);
            return -EXCEPTION_FAIL;
        }
        if (subsys_cfg->subsys_reset_func->get_power_state(&subsys_res->node) == POWER_STATE_OPEN) {
            ps_print_info("subsys=%s"NEWLINE, subsys_cfg->subsys_name);
            if (subsys_cfg->subsys_reset_func->pre_do != NULL) {
                ret = subsys_cfg->subsys_reset_func->pre_do(&subsys_res->node, excp_record);
            }
            if (ret != EXCEPTION_SUCCESS) {
                break;
            }
        }
    }
    return ret;
}

int32_t chip_mpxx_power_reset(excp_comm_t *node_ptr)
{
    oal_reference(node_ptr);
    return board_power_reset(EXCP_SYS_BUTT); /* 该入参暂未使用 */
}

int32_t chip_mpxx_firmware_download_pre(excp_comm_t *node_ptr)
{
    uint32_t i;
    int32_t ret = EXCEPTION_SUCCESS;
    excp_subsys_res_t *subsys_res = NULL;
    excp_subsys_cfg_t *subsys_cfg = NULL;
    excp_chip_res_t *chip_res = oal_container_of(node_ptr, excp_chip_res_t, node);

    for (i = 0; i < chip_res->subsys_cnt; i++) {
        subsys_res = &chip_res->subsys_attr[i];
        subsys_cfg = subsys_res->subsys_cfg;
        if (subsys_cfg->subsys_reset_func->get_power_state(&subsys_res->node) == POWER_STATE_OPEN) {
            ps_print_info("subsys=%s"NEWLINE, subsys_cfg->subsys_name);
            if (subsys_cfg->subsys_reset_func->firmware_download_pre != NULL) {
                ret = subsys_cfg->subsys_reset_func->firmware_download_pre(&subsys_res->node);
            }
            if (ret != EXCEPTION_SUCCESS) {
                return -EXCEPTION_FAIL;
            }
        }
    }
    return EXCEPTION_SUCCESS;
}

int32_t chip_mpxx_firmware_download_post(excp_comm_t *node_ptr)
{
    uint32_t i;
    int32_t ret = EXCEPTION_SUCCESS;
    excp_subsys_res_t *subsys_res = NULL;
    excp_subsys_cfg_t *subsys_cfg = NULL;
    excp_chip_res_t *chip_res = oal_container_of(node_ptr, excp_chip_res_t, node);

    for (i = 0; i < chip_res->subsys_cnt; i++) {
        subsys_res = &chip_res->subsys_attr[i];
        subsys_cfg = subsys_res->subsys_cfg;
        if (subsys_cfg->subsys_reset_func->get_power_state(&subsys_res->node) == POWER_STATE_OPEN) {
            ps_print_info("subsys=%s"NEWLINE, subsys_cfg->subsys_name);
            if (subsys_cfg->subsys_reset_func->firmware_download_post != NULL) {
                ret = subsys_cfg->subsys_reset_func->firmware_download_post(&subsys_res->node);
            }
            if (ret != EXCEPTION_SUCCESS) {
                return -EXCEPTION_FAIL;
            }
        }
    }

    return EXCEPTION_SUCCESS;
}

OAL_STATIC void dfr_fail_power_off(excp_chip_res_t *chip_res)
{
    uint32_t i;
    excp_subsys_res_t *subsys_res = NULL;
    excp_subsys_cfg_t *subsys_cfg = NULL;
#ifdef PLATFORM_DEBUG_ENABLE
    struct st_exception_info *pst_exception_data = get_exception_info_reference();
    if (pst_exception_data == NULL) {
        ps_print_err("[MPXX_DFR]get exception info reference is error\n");
        return;
    }
#endif

    /* 下电 */
    for (i = chip_res->subsys_cnt; i > 0; i--) {
        subsys_res = &chip_res->subsys_attr[i - 1];
        subsys_cfg = subsys_res->subsys_cfg;
        ps_print_info("subsys=%s"NEWLINE, subsys_cfg->subsys_name);
        if (subsys_cfg->subsys_reset_func->power_off == NULL) {
            ps_print_warning("power_off_func is NULL"NEWLINE);
            continue;
        }

        if (subsys_cfg->subsys_reset_func->get_power_state(&subsys_res->node) == POWER_STATE_OPEN) {
            subsys_cfg->subsys_reset_func->power_off(&subsys_res->node, OAL_TRUE);
        } else {
            subsys_cfg->subsys_reset_func->power_off(&subsys_res->node, OAL_FALSE);
        }
    }

#ifdef PLATFORM_DEBUG_ENABLE
    if (pst_exception_data->sys_rst_uart_suspend == OAL_TRUE) {
        ps_print_info("[dfr_test] uart_suspend clean\n");
        pst_exception_data->sys_rst_uart_suspend = OAL_FALSE;
    }
#endif

    declare_dft_trace_key_info("dfr_fail_power_off", OAL_DFT_TRACE_FAIL);
}

OAL_STATIC void download_power_off(excp_chip_res_t *chip_res)
{
    uint32_t i;
    uint32_t off_cnt = 0;
    excp_subsys_res_t *subsys_res = NULL;
    excp_subsys_cfg_t *subsys_cfg = NULL;
    excp_chip_cfg_t *chip_cfg = NULL;

    chip_cfg = chip_res->chip_cfg;
    /* 下电 */
    for (i = chip_res->subsys_cnt; i > 0; i--) {
        subsys_res = &chip_res->subsys_attr[i - 1];
        subsys_cfg = subsys_res->subsys_cfg;
        if (subsys_cfg->subsys_reset_func->get_power_state(&subsys_res->node) == POWER_STATE_SHUTDOWN) {
            ps_print_info("subsys=%s"NEWLINE, subsys_cfg->subsys_name);
            off_cnt++;
            if (subsys_cfg->subsys_reset_func->power_off != NULL) {
                subsys_cfg->subsys_reset_func->power_off(&subsys_res->node, OAL_FALSE);
            }
        }
    }

    if (off_cnt == chip_res->subsys_cnt) {
        board_chip_power_off();
    }
}

OAL_STATIC int32_t firmware_download_mode_separate(excp_chip_res_t *chip_res)
{
    uint32_t i;
    int32_t ret = EXCEPTION_SUCCESS;
    excp_subsys_res_t *subsys_res = NULL;
    struct pm_top* pm_top_data = pm_get_top();
    excp_subsys_cfg_t *subsys_cfg = NULL;

    /* 先下载bfgx, 然后下载wifi */
    for (i = chip_res->subsys_cnt; i > 0; i--) {
        subsys_res = &chip_res->subsys_attr[i - 1];
        subsys_cfg = subsys_res->subsys_cfg;
        if (subsys_cfg->subsys_reset_func->get_power_state(&subsys_res->node) == POWER_STATE_OPEN) {
            ps_print_info("subsys=%s"NEWLINE, subsys_cfg->subsys_name);
            ret = firmware_download_function(subsys_cfg->which_cfg, hcc_get_bus(HCC_EP_WIFI_DEV));
            /* sdio场景连续下载bin, 下载完后重新给sdio上电 */
            if (subsys_cfg->subsys_type != EXCP_W_SYS) {
                hcc_bus_disable_state(pm_top_data->wlan_pm_info->pst_bus, OAL_BUS_STATE_ALL);
                wifi_power_reset(NULL);
            }
            if (ret != EXCEPTION_SUCCESS) {
                ps_print_err("%s: firmware_download failed!\n", __func__);
                break;
            }
        }
    }

    return ret;
}


int32_t chip_mpxx_firmware_download_do(excp_comm_t *node_ptr)
{
    int32_t ret = EXCEPTION_SUCCESS;
    excp_chip_cfg_t *chip_cfg = NULL;
    excp_chip_res_t *chip_res = oal_container_of(node_ptr, excp_chip_res_t, node);
    struct st_exception_info *excp_data = get_exception_info_reference();

    if (unlikely(chip_res == NULL || excp_data == NULL)) {
        ps_print_err("chip_res or excp_data is NULL\n");
        return -EXCEPTION_FAIL;
    }

    chip_cfg = chip_res->chip_cfg;
    if (mpxx_firmware_download_mode() == MODE_COMBO) {
        ps_print_info("chip_mpxx_firmware_download_do MODE_COMBO\n");
        if (chip_cfg->chip_reset_func->firmware_download_mode_combo != NULL) {
            ret = chip_cfg->chip_reset_func->firmware_download_mode_combo(node_ptr);
        } else {
            ps_print_info("firmware_download_mode_combo is NULL\n");
        }
    } else {
        ret = firmware_download_mode_separate(chip_res);
    }

    download_power_off(chip_res);
#ifdef PLATFORM_DEBUG_ENABLE
    if (excp_data->sys_rst_download_dbg == OAL_TRUE) {
        excp_data->sys_rst_download_dbg = OAL_FALSE;
        ps_print_info("[dfr_test] firmware_download_do fail\n");
        return -EXCEPTION_FAIL;
    }
#endif
    return ret;
}

int32_t chip_mpxx_post_do(excp_comm_t *node_ptr, excp_record_t *excp_record)
{
    uint32_t i;
    int32_t ret = EXCEPTION_SUCCESS;
    excp_subsys_res_t *subsys_res = NULL;
    excp_subsys_cfg_t *subsys_cfg = NULL;
    excp_chip_res_t *chip_res = oal_container_of(node_ptr, excp_chip_res_t, node);

    for (i = 0; i < chip_res->subsys_cnt; i++) {
        subsys_res = &chip_res->subsys_attr[i];
        subsys_cfg = subsys_res->subsys_cfg;
        if (subsys_cfg->subsys_reset_func->get_power_state(&subsys_res->node) == POWER_STATE_OPEN) {
            ps_print_info("subsys=%s"NEWLINE, subsys_cfg->subsys_name);
            if (subsys_cfg->subsys_reset_func->post_do != NULL) {
                ret = subsys_cfg->subsys_reset_func->post_do(&subsys_res->node, excp_record);
            }
            if (ret != EXCEPTION_SUCCESS) {
                return -EXCEPTION_FAIL;
            }
        }
    }

    chip_res->chip_excp_stats[excp_record->exception_type]++;
    chip_res->chip_excp_cnt++;

    return EXCEPTION_SUCCESS;
}

/* 对于一个子系统下有多个bus(如shengkuo buart & guart)，须注册从核的pm data */
OAL_STATIC void get_slave_core_pm_data(excp_subsys_res_t *subsys)
{
    uint32_t i;
    excp_core_res_t *core_attr = NULL;
    excp_sla_res_t *sla_core = NULL;

    if (subsys->subsys_cfg->slave_pm_flag == OAL_FALSE) {
        return;
    }

    core_attr = subsys->core_attr;
    if (oal_unlikely(core_attr == NULL || core_attr->sla_cores == NULL)) {
        ps_print_info("core_attr, sla_cores or master_cfg is NULL\n");
        return;
    }

    for (i = 0; i < core_attr->sla_core_cnt; i++) {
        sla_core = &core_attr->sla_cores[i];
        if (oal_unlikely(sla_core->sla_cfg == NULL)) {
            ps_print_err("sla_cfg is NULL\n");
            return;
        }

        sla_core->sla_core_pm = pm_get_drvdata(sla_core->sla_cfg->bus_id);
        ps_print_info("i=%d, sla_cfg->bus_id=%d, core_name=%s\n", i, sla_core->sla_cfg->bus_id,
                      sla_core->sla_cfg->sla_core_name);
    }

    core_attr->master_pm = pm_get_drvdata(subsys->subsys_cfg->bus_id);
    ps_print_info("subsys_cfg->bus_id=%d\n", subsys->subsys_cfg->bus_id);
    ps_print_info("get_slave_core_pm_data succ\n");
}

void excp_pm_data_register(void)
{
    uint32_t i;
    excp_chip_res_t *excp_chip = NULL;
    excp_subsys_res_t *subsys = NULL;
    struct st_exception_info *exception_data = get_exception_info_reference();

    if (oal_unlikely(exception_data == NULL || exception_data->excp_chip == NULL)) {
        ps_print_err("%s:exception info reference is NULL\n", __func__);
        return;
    }

    excp_chip = exception_data->excp_chip;
    if (oal_unlikely(excp_chip->subsys_attr == NULL)) {
        ps_print_err("subsys_attr is NULL\n");
        return;
    }

    for (i = 0; i < excp_chip->subsys_cnt; i++) {
        subsys = &excp_chip->subsys_attr[i];
        get_slave_core_pm_data(subsys);
        if (oal_unlikely(subsys->subsys_cfg == NULL)) {
            ps_print_err("subsys_cfg is NULL\n");
            return;
        }
        if (subsys->subsys_cfg->bus_id >= UART_BUTT) {
            subsys->subsys_pm = wlan_pm_get_drv();
            continue;
        }

        subsys->subsys_pm = pm_get_drvdata(subsys->subsys_cfg->bus_id);
    }

    ps_print_info("excp_pm_data_register succ\n");
}

int dfr_notifier_chain_register(struct notifier_block *nb)
{
    struct st_exception_info *excp_data = get_exception_info_reference();

    if (oal_unlikely(oal_any_null_ptr3(excp_data, excp_data->excp_chip, nb))) {
        ps_print_err("exception info reference is NULL\n");
        return OAL_FAIL;
    }

    return blocking_notifier_chain_register(&excp_data->excp_chip->nh, nb);
}

int dfr_notifier_chain_unregister(struct notifier_block *nb)
{
    struct st_exception_info *excp_data = get_exception_info_reference();

    if (oal_unlikely(oal_any_null_ptr3(excp_data, excp_data->excp_chip, nb))) {
        ps_print_err("exception info reference is NULL\n");
        return OAL_FAIL;
    }

    return blocking_notifier_chain_unregister(&excp_data->excp_chip->nh, nb);
}


OAL_STATIC uint32_t get_excp_sys_map(uint32_t sys)
{
    excp_chip_cfg_t *chip_cfg = NULL;

    chip_cfg = get_excp_chip_cfg(get_mpxx_subchip_type());
    if (oal_unlikely(chip_cfg == NULL || chip_cfg->excp_sys_map == NULL)) {
        ps_print_err("%s: chip_cfg or chip_cfg->excp_sys_map NULL"NEWLINE, __func__);
        return SYS_BUTT;
    }

    if (oal_unlikely(sys >= chip_cfg->sys_map_size)) {
        ps_print_info("invalid excp sys=%d"NEWLINE, sys);
        return SYS_BUTT;
    }

    return chip_cfg->excp_sys_map[sys];
}


int dfr_notifier_chain_call(unsigned long excp_type)
{
    uint32_t map_sys;
    struct st_exception_info *excp_data = get_exception_info_reference();

    if (oal_unlikely(excp_data == NULL || excp_data->excp_chip == NULL)) {
        ps_print_err("exception info reference is NULL\n");
        return OAL_FAIL;
    }

    get_subsys_core_type(excp_type, &excp_data->record);
    map_sys = get_excp_sys_map(excp_data->record.subsys_type);

    return blocking_notifier_call_chain(&excp_data->excp_chip->nh, map_sys, NULL);
}

EXPORT_SYMBOL_GPL(dfr_notifier_chain_register);
EXPORT_SYMBOL_GPL(dfr_notifier_chain_unregister);
