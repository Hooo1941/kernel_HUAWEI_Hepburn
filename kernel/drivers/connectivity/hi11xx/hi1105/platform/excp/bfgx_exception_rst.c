/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: bfgx exception reset function
 * Author: @CompanyNameTag
 */

/* 头文件包含 */
#include "oal_ext_if.h"
#include "plat_debug.h"
#include "plat_pm.h"
#include "plat_pm_wlan.h"
#include "plat_firmware.h"
#include "plat_firmware_download.h"
#include "plat_uart.h"
#include "chr_errno.h"
#include "oam_rdr.h"
#include "oam_ext_if.h"
#include "securec.h"
#include "chr_user.h"
#include "bfgx_dev.h"
#include "bfgx_core.h"
#include "bfgx_data_parse.h"
#include "exception_common.h"
#include "wifi_exception_rst.h"
#include "bfgx_exception_rst.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_BFGX_EXCEPTION_RST_C

OAL_STATIC struct st_bfgx_reset_cmd g_bfgx_reset_msg[BFGX_BUTT] = {
    { BT_RESET_CMD_LEN,   { 0x04, 0xff, 0x01, 0xc7 }},
    { FM_RESET_CMD_LEN,   { 0xfb }},
    { GNSS_RESET_CMD_LEN, { 0xc7, 0x51, 0x0, 0x0, 0xa5, 0xa5, 0x0, 0x0, GNSS_SEPER_TAG_LAST }},
    { IR_RESET_CMD_LEN,   {0}},
    { NFC_RESET_CMD_LEN,  {0}},
    /*
     *              1字节             2字节           2字节
     *         [hci data type]      [opcode]        [length]
     */
    { SLE_RESET_CMD_LEN,  { 0xA2, 0xff, 0xff, 0x01, 0x00, 0xC7 }}
};

memdump_info_t g_bcpu_memdump_cfg;
memdump_info_t g_gcpu_memdump_cfg;

OAL_STATIC int32_t bfgx_status_recovery(struct ps_core_s *ps_core_d);

/* dfr skip ssi dump check */
void ssi_dump_for_dfr(unsigned long long module_set)
{
#ifdef PLATFORM_DEBUG_ENABLE
    ssi_dump_device_regs_handler(module_set, OAL_FALSE);
#endif
}

int32_t bfgx_excp_handle_before_rst(uint32_t exception_type, struct ps_core_s *ps_core_d)
{
    int32_t ret = EXCEPTION_SUCCESS;
    struct pm_drv_data *pm_data = NULL;
    unsigned long flags;

    pm_data = ps_core_d->pm_data;
    oal_spin_lock_irq_save(&pm_data->uart_state_spinlock, &flags);
    bfgx_dev_state_set(pm_data, BFGX_RESET);
    oal_spin_unlock_irq_restore(&pm_data->uart_state_spinlock, &flags);

    ret = cancel_work_sync(&pm_data->wkup_dev_work);
    ps_print_info("[%s]cancel wkup_dev_work ret %d \n", index2name(ps_core_d->uart_index), ret);
    ret = cancel_work_sync(&pm_data->send_disallow_msg_work);
    ps_print_info("[%s]cancel send_disallow_msg_work ret %d \n", index2name(ps_core_d->uart_index), ret);
    ret = cancel_work_sync(&pm_data->send_allow_sleep_work);
    ps_print_info("[%s]cancel send_allow_sleep_work ret %d \n", index2name(ps_core_d->uart_index), ret);

    del_timer_sync(&pm_data->bfg_timer);
    oal_atomic_set(&pm_data->bfg_timer_mod_cnt, 0);
    pm_data->bfg_timer_mod_cnt_pre = 0;
    del_timer_sync(&pm_data->dev_ack_timer);

    update_heart_beat_timer(ps_core_d, TIMER_DELETE);

    bfgx_print_subsys_state(pm_data->index);

    if (exception_type != get_panic_excp_by_uart_idx(pm_data->index)) {
        ps_uart_state_dump(ps_core_d);
    }

    return EXCEPTION_SUCCESS;
}

int32_t bfgx_reset_pre_proc(struct pm_drv_data *pm_data, excp_record_t *excp_record)
{
    struct ps_core_s *ps_core_d = NULL;

    if (oal_unlikely(pm_data == NULL || pm_data->ps_core_data == NULL)) {
        ps_print_err("%s: subsys_res is NULL\n", __func__);
        return -EXCEPTION_FAIL;
    }

    ps_core_d = pm_data->ps_core_data;
    if (oal_unlikely(ps_core_d == NULL)) {
        ps_print_err("%s: ps_core_d is NULL\n", __func__);
        return -EXCEPTION_FAIL;
    }

    ps_print_err("uart[%s]\n", index2name(ps_core_d->pm_data->index));

    if (bfgx_excp_handle_before_rst(excp_record->exception_type, ps_core_d) < 0) {
        ps_print_err("bfgx excep before rst failed\n");
        return -EXCEPTION_FAIL;
    }

    if (release_tty_drv(ps_core_d) != EXCEPTION_SUCCESS) {
        ps_print_err("close tty is err!\n");
        if (ps_core_d->tty_have_open == true) {
            return -EXCEPTION_FAIL;
        }
    }

    atomic_set(&pm_data->bfg_needwait_devboot_flag, NEED_SET_FLAG);
    oal_reinit_completion(pm_data->dev_bootok_ack_comp);
    return EXCEPTION_SUCCESS;
}

int32_t get_bfgx_subsys_state(excp_comm_t *node_ptr)
{
    struct pm_drv_data *pm_drv_d = NULL;
    excp_subsys_res_t *subsys_res = oal_container_of(node_ptr, excp_subsys_res_t, node);

    if (oal_unlikely(subsys_res == NULL)) {
        ps_print_err("subsys_res is NULL"NEWLINE);
        return POWER_STATE_SHUTDOWN;
    }

    pm_drv_d = (struct pm_drv_data*)subsys_res->subsys_pm;
    if (ps_core_chk_bfgx_active(pm_drv_d->ps_core_data) == false) {
        return POWER_STATE_SHUTDOWN;
    }

    return POWER_STATE_OPEN;
}

int32_t bfgx_reset_pre_do(excp_comm_t *node_ptr, excp_record_t *excp_record)
{
    excp_subsys_res_t *subsys_res = oal_container_of(node_ptr, excp_subsys_res_t, node);

    if (oal_unlikely(subsys_res == NULL)) {
        ps_print_err("%s: subsys_res is NULL\n", __func__);
        return -EXCEPTION_FAIL;
    }

    return bfgx_reset_pre_proc((struct pm_drv_data*)subsys_res->subsys_pm, excp_record);
}

int32_t bfgx_master_core_reset_pre_do(excp_comm_t *node_ptr, excp_record_t *excp_record)
{
    excp_core_res_t *core_attr = oal_container_of(node_ptr, excp_core_res_t, node);

    return bfgx_reset_pre_proc(core_attr->master_pm, excp_record);
}

int32_t bfgx_slave_core_reset_pre_do(excp_comm_t *node_ptr, excp_record_t *excp_record)
{
    int32_t ret;
    excp_sla_res_t *sla_core_res = oal_container_of(node_ptr, excp_sla_res_t, node);

    if (oal_unlikely(sla_core_res->sla_core_pm == NULL || sla_core_res->sla_cfg == NULL)) {
        ps_print_err("sla_core_pm or sla_cfg is NULL"NEWLINE);
        return -EXCEPTION_FAIL;
    }

    ret = bfgx_reset_pre_proc((struct pm_drv_data*)sla_core_res->sla_core_pm, excp_record);
    return ret;
}

int32_t bfgx_subsys_power_reset(excp_comm_t *node_ptr)
{
    int32_t ret;

    oal_reference(node_ptr);
    ret = board_sys_disable(B_SYS);
    if (ret != OAL_SUCC) {
        return ret;
    }
    mdelay(BFGX_SUBSYS_RST_DELAY);
    return board_sys_enable(B_SYS);
}

STATIC void bfgx_excp_close_for_all_cores(excp_subsys_res_t *subsys_res)
{
    uint32_t i;
    excp_core_res_t *core_attr = NULL;
    excp_sla_res_t *sla_core = NULL;

    if (subsys_res->core_attr == NULL) {
        hw_bfgx_close_for_excp((struct pm_drv_data*)subsys_res->subsys_pm, OAL_TRUE);
        return;
    }

    core_attr = subsys_res->core_attr;
    for (i = 0; i < core_attr->sla_core_cnt; i++) {
        sla_core = &core_attr->sla_cores[i];
        if (sla_core->sla_core_pm != NULL) {
            hw_bfgx_close_for_excp((struct pm_drv_data*)sla_core->sla_core_pm, OAL_FALSE);
        }
    }
    hw_bfgx_close_for_excp((struct pm_drv_data*)core_attr->master_pm, OAL_TRUE);
}

void bfgx_subsys_power_off_proc(excp_comm_t *node_ptr, uint8_t flag, uint32_t sys)
{
    excp_subsys_res_t *subsys_res = NULL;

    if (flag == OAL_FALSE) { // 之前未上电，直接拉en, 确保所有系统下电
        board_power_off(sys);
        return;
    }

    subsys_res = oal_container_of(node_ptr, excp_subsys_res_t, node);
    bfgx_excp_close_for_all_cores(subsys_res);
}

int32_t bfgx_firmware_download_do(excp_comm_t *node_ptr)
{
    int32_t ret;
    struct pm_top* pm_top_data = pm_get_top();
    excp_subsys_res_t *subsys_res = oal_container_of(node_ptr, excp_subsys_res_t, node);

    if (mpxx_firmware_download_mode() == MODE_COMBO) {
        return EXCEPTION_SUCCESS;
    }

    /* MODE_SEPARATE depends on WiFi, pcie */
    if (wlan_is_shutdown()) {
        if (board_sys_enable(W_SYS) != SUCC) {
            return -EXCEPTION_FAIL;
        }
    }

    ret = firmware_download_function(subsys_res->subsys_cfg->which_cfg, hcc_get_bus(HCC_EP_WIFI_DEV));
    if (ret != SUCCESS) {
        ps_print_err("%s: %s download firmware fail!\n", __func__, subsys_res->subsys_cfg->subsys_name);
    }

    if (wlan_is_shutdown()) {
        hcc_bus_disable_state(pm_top_data->wlan_pm_info->pst_bus, OAL_BUS_STATE_ALL);
        board_sys_disable(W_SYS);
    }
    return ret;
}

OAL_STATIC int32_t bfgx_download_fail_proc(struct pm_drv_data *pm_data)
{
    if (oal_unlikely(pm_data == NULL)) {
        ps_print_err("%s: pm_data is NULL!\n", __func__);
        return -EXCEPTION_FAIL;
    }

    atomic_set(&pm_data->bfg_needwait_devboot_flag, NONEED_SET_FLAG);
    return EXCEPTION_SUCCESS;
}

int32_t bfgx_download_fail_do(excp_comm_t *node_ptr)
{
    excp_subsys_res_t *subsys_res = oal_container_of(node_ptr, excp_subsys_res_t, node);

    if (oal_unlikely(subsys_res == NULL)) {
        ps_print_err("%s: subsys_res is NULL!\n", __func__);
        return -EXCEPTION_FAIL;
    }

    return bfgx_download_fail_proc((struct pm_drv_data*)subsys_res->subsys_pm);
}

int32_t bfgx_master_core_download_fail_do(excp_comm_t *node_ptr)
{
    excp_core_res_t *core_attr = oal_container_of(node_ptr, excp_core_res_t, node);

    if (oal_unlikely(core_attr->master_pm == NULL)) {
        ps_print_err("%s: master_pm is NULL!\n", __func__);
        return -EXCEPTION_FAIL;
    }

    return bfgx_download_fail_proc((struct pm_drv_data*)core_attr->master_pm);
}

int32_t bfgx_slave_core_download_fail_do(excp_comm_t *node_ptr)
{
    int32_t ret;
    excp_sla_res_t *sla_core_res = oal_container_of(node_ptr, excp_sla_res_t, node);

    if (oal_unlikely(sla_core_res->sla_core_pm == NULL)) {
        ps_print_err("sla_core_pm is NULL"NEWLINE);
        return -EXCEPTION_FAIL;
    }

    ret = bfgx_download_fail_proc((struct pm_drv_data*)sla_core_res->sla_core_pm);
    return ret;
}

OAL_STATIC int32_t bfgx_firmware_download_post_proc(struct pm_drv_data *pm_data)
{
    unsigned long timeleft;
    int32_t gpio_level, gpio_type;

    if (oal_unlikely(pm_data == NULL || pm_data->ps_core_data == NULL)) {
        ps_print_err("%s: subsys_res is NULL\n", __func__);
        return -EXCEPTION_FAIL;
    }

    if (open_tty_drv(pm_data->ps_core_data) != EXCEPTION_SUCCESS) {
        ps_print_err("open tty is err!\n");
        return -EXCEPTION_FAIL;
    }

    if (conn_gpio_valid(pm_data->ps_plat_data->host_wkup_io_type) == OAL_FALSE) {
        pm_data->bfg_wake_lock(pm_data);
        msleep(WAIT_BFGX_BOOTOK_TIME);
        bfgx_dev_state_set(pm_data, BFGX_ACTIVE);
        bfg_disallow_msg_work_add(pm_data);
    }

    timeleft = wait_for_completion_timeout(&pm_data->dev_bootok_ack_comp, msecs_to_jiffies(WAIT_BFGX_BOOTOK_TIME));
    if (!timeleft) {
        gpio_type = pm_data->ps_plat_data->host_wkup_io_type;
        gpio_level = conn_get_gpio_level(gpio_type);
        oam_error_log2(0, OAM_SF_DFR,
                       "[MPXX_DFR] wait bfgx boot ok timeout, gpio_type=%d, gpio_level=%d", gpio_type, gpio_level);
        atomic_set(&pm_data->bfg_needwait_devboot_flag, NONEED_SET_FLAG);
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_RST, CHR_PLAT_DRV_ERROR_BFGX_SYSTEM_RST_BOOTUP);
        ssi_dump_for_dfr(SSI_ERR_BFGX_OPEN_FAIL);
        // 恢复过程中有异常panic异常上报，这里等待内存导出
        wait_bfgx_memdump_complete(pm_data->ps_core_data);
        return -EXCEPTION_FAIL;
    }

    atomic_set(&pm_data->bfg_needwait_devboot_flag, NONEED_SET_FLAG);
    return EXCEPTION_SUCCESS;
}


int32_t bfgx_firmware_download_post(excp_comm_t *node_ptr)
{
    excp_subsys_res_t *subsys_res = oal_container_of(node_ptr, excp_subsys_res_t, node);

    if (oal_unlikely(subsys_res == NULL)) {
        ps_print_err("%s: subsys_res is NULL\n", __func__);
        return -EXCEPTION_FAIL;
    }

    return bfgx_firmware_download_post_proc((struct pm_drv_data*)subsys_res->subsys_pm);
}

int32_t bfgx_master_core_firmware_download_post(excp_comm_t *node_ptr)
{
    excp_core_res_t *core_attr = oal_container_of(node_ptr, excp_core_res_t, node);

    if (oal_unlikely(core_attr->master_pm == NULL)) {
        ps_print_err("%s: master_pm is NULL!\n", __func__);
        return -EXCEPTION_FAIL;
    }

    return bfgx_firmware_download_post_proc((struct pm_drv_data*)core_attr->master_pm);
}

OAL_STATIC int32_t bfgx_slave_core_firmware_download_post(excp_comm_t *node_ptr)
{
    int32_t ret;
    excp_sla_res_t *sla_core_res = oal_container_of(node_ptr, excp_sla_res_t, node);

    if (oal_unlikely(sla_core_res->sla_core_pm == NULL || sla_core_res->sla_cfg == NULL)) {
        ps_print_err("sla_core_pm or sla_cfg is NULL"NEWLINE);
        return -EXCEPTION_FAIL;
    }

    ret = bfgx_firmware_download_post_proc((struct pm_drv_data*)sla_core_res->sla_core_pm);
    return ret;
}

OAL_STATIC int32_t bfgx_reset_post_proc(struct pm_drv_data *pm_data)
{
    if (oal_unlikely(pm_data == NULL || pm_data->ps_core_data == NULL)) {
        ps_print_err("pm_data or ps_core_data is NULL\n");
        return -EXCEPTION_FAIL;
    }

    bfgx_pm_feature_set(pm_data);

    if (bfgx_status_recovery(pm_data->ps_core_data) != EXCEPTION_SUCCESS) {
        oam_error_log0(0, OAM_SF_DFR, "[MPXX_DFR] bfgx recovery status failed");
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_RST, CHR_PLAT_DRV_ERROR_BFGX_SYSTEM_RST_RECOVERY);
        return -EXCEPTION_FAIL;
    }

    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                         CHR_PLT_DRV_EVENT_RST, CHR_PLAT_DRV_ERROR_BFGX_SYSTEM_DFR_SUCC);
    return EXCEPTION_SUCCESS;
}


int32_t bfgx_reset_post_do(excp_comm_t *node_ptr, excp_record_t *excp_record)
{
    int32_t ret;
    excp_subsys_res_t *subsys_res = oal_container_of(node_ptr, excp_subsys_res_t, node);

    if (oal_unlikely(subsys_res == NULL)) {
        ps_print_err("%s: subsys_res is NULL\n", __func__);
        return -EXCEPTION_FAIL;
    }

    ret = bfgx_reset_post_proc((struct pm_drv_data*)subsys_res->subsys_pm);
    if (ret != EXCEPTION_SUCCESS) {
        return -EXCEPTION_FAIL;
    }

    subsys_res->subsys_excp_stats[excp_record->exception_type]++;
    subsys_res->subsys_excp_cnt++;

    return EXCEPTION_SUCCESS;
}

int32_t bfgx_master_core_reset_post_do(excp_comm_t *node_ptr, excp_record_t *excp_record)
{
    excp_core_res_t *core_attr = oal_container_of(node_ptr, excp_core_res_t, node);

    if (oal_unlikely(core_attr->master_pm == NULL)) {
        ps_print_err("%s: master_pm is NULL!\n", __func__);
        return -EXCEPTION_FAIL;
    }

    return bfgx_reset_post_proc((struct pm_drv_data*)core_attr->master_pm);
}

int32_t bfgx_slave_core_reset_post_do(excp_comm_t *node_ptr, excp_record_t *excp_record)
{
    int32_t ret;
    excp_sla_res_t *sla_core_res = oal_container_of(node_ptr, excp_sla_res_t, node);

    if (oal_unlikely(sla_core_res->sla_core_pm == NULL || sla_core_res->sla_cfg == NULL)) {
        ps_print_err("sla_core_pm or sla_cfg is NULL"NEWLINE);
        return -EXCEPTION_FAIL;
    }

    ret = bfgx_slave_core_firmware_download_post(node_ptr);
    if (ret != EXCEPTION_SUCCESS) {
        ps_print_err("bfgx_slave_core_firmware_download_post fail"NEWLINE);
        return ret;
    }

    ret = bfgx_reset_post_proc((struct pm_drv_data*)sla_core_res->sla_core_pm);
    return ret;
}

/*
 * 函 数 名  : bfgx_dump_stack
 * 功能描述  : 心跳超时时，尝试通过uart读栈，不保证一定能成功，因为此时uart可能不通
 */
void bfgx_dump_stack(struct ps_core_s *ps_core_d)
{
    unsigned long timeleft;
    uint32_t exception_type;
    struct st_exception_info *pst_exception_data = get_exception_info_reference();

    if (oal_unlikely(pst_exception_data == NULL || ps_core_d == NULL || ps_core_d->pm_data == NULL)) {
        ps_print_err("exception_data, ps_core_d, or pm_data is NULL\n");
        return;
    }

    /* bfg未open则跳过等待memdump操作完成 */
    if (ps_core_chk_bfgx_active(ps_core_d) == OAL_FALSE) {
        ps_print_warning("ps core is closed\n");
        return;
    }

    exception_type = pst_exception_data->excetion_type;
    if (exception_type != get_panic_excp_by_uart_idx(ps_core_d->pm_data->index)) {
        return;
    }

    oal_reinit_completion(ps_core_d->wait_read_bfgx_stack);

    /* 等待读栈操作完成 */
    timeleft = wait_for_completion_timeout(&ps_core_d->wait_read_bfgx_stack,
                                           msecs_to_jiffies(WAIT_BFGX_READ_STACK_TIME));
    if (!timeleft) {
        ps_uart_state_dump(ps_core_d);
        atomic_set(&ps_core_d->is_memdump_runing, 0);
        ssi_dump_err_regs(SSI_ERR_BFGX_OPEN_FAIL);
        ps_print_err("read bfgx stack failed!\n");
    } else {
        ps_print_info("read bfgx stack success!\n");
    }

    return;
}

OAL_STATIC int32_t bfgx_reset_cmd_send(uint32_t subsys, struct ps_core_s *ps_core_d)
{
    int32_t ret;
    struct st_bfgx_data *pst_bfgx_data = NULL;

    pst_bfgx_data = &ps_core_d->bfgx_info[subsys];

    ret = ps_push_skb_queue(ps_core_d, g_bfgx_reset_msg[subsys].cmd,
                            g_bfgx_reset_msg[subsys].len,
                            g_bfgx_rx_queue[subsys]);
    if (ret != EXCEPTION_SUCCESS) {
        ps_print_err("push %s reset cmd to skb fail\n", service_get_name(subsys));
        return -EXCEPTION_FAIL;
    }

    oal_wait_queue_wake_up_interrupt(&pst_bfgx_data->rx_wait);

    return EXCEPTION_SUCCESS;
}

#ifdef _PRE_H2D_GPIO_WKUP
OAL_STATIC int32_t bfgx_gpio_wkup_mode_recovery(struct ps_core_s *ps_core_d)
{
    struct pm_drv_data *pm_data = NULL;
    int32_t ret;

    pm_data = ps_core_d->pm_data;
    if (unlikely(pm_data == NULL)) {
        ps_print_err("pm_data is NULL!\n");
        return -EXCEPTION_FAIL;
    }

    atomic_set(&pm_data->cfg_wkup_dev_flag, 0);
    ret = host_send_cfg_gpio_wkup_dev_msg(pm_data);
    if (ret != SUCCESS) {
        ps_print_err("bfgx_gpio_wkup_mode_recovery fail.\n");
        return -EXCEPTION_FAIL;
    }

    return EXCEPTION_SUCCESS;
}
#endif
/*
 * 函 数 名  : bfgx_status_recovery
 * 功能描述  : 全系统复位以后，恢复bfgin业务函数
 */
OAL_STATIC int32_t bfgx_status_recovery(struct ps_core_s *ps_core_d)
{
    uint32_t i;
    struct st_bfgx_data *pst_bfgx_data = NULL;
    int32_t prepare_result;

    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is null\n");
        return -EXCEPTION_FAIL;
    }
#ifdef _PRE_H2D_GPIO_WKUP
    if (bfgx_gpio_wkup_mode_recovery(ps_core_d) != EXCEPTION_SUCCESS) {
        return -EXCEPTION_FAIL;
    }
#endif
    if (prepare_to_visit_node(ps_core_d, &prepare_result) != EXCEPTION_SUCCESS) {
        ps_print_err("prepare work FAIL\n");
        return -EXCEPTION_FAIL;
    }

    for (i = 0; i < BFGX_BUTT; i++) {
        pst_bfgx_data = &ps_core_d->bfgx_info[i];
        if (atomic_read(&pst_bfgx_data->subsys_state) == POWER_STATE_SHUTDOWN) {
            continue;
        }

        ps_kfree_skb(ps_core_d, g_bfgx_rx_queue[i]);

        ps_print_info("[MPXX_DFR]subsys %d open cmd send\n", i);
        if (bfgx_open_cmd_send(ps_core_d, i) != EXCEPTION_SUCCESS) {
            oam_error_log0(0, OAM_SF_DFR, "[MPXX_DFR] bfgx open cmd fail");
            ssi_dump_for_dfr(SSI_ERR_BFGX_OPEN_FAIL);
            post_to_visit_node(ps_core_d, prepare_result);
            return -EXCEPTION_FAIL;
        }

        ps_print_info("[MPXX_DFR]subsys %d reset cmd send\n", i);
        if (bfgx_reset_cmd_send(i, ps_core_d) != EXCEPTION_SUCCESS) {
            oam_error_log0(0, OAM_SF_DFR, "[MPXX_DFR] bfgx reset cmd send fail");
            post_to_visit_node(ps_core_d, prepare_result);
            return -EXCEPTION_FAIL;
        }
    }

    post_to_visit_node(ps_core_d, prepare_result);

    return EXCEPTION_SUCCESS;
}

int32_t wait_bfgx_memdump_complete(struct ps_core_s *ps_core_d)
{
    unsigned long timeleft;

    if (atomic_read(&ps_core_d->is_memdump_runing) == 0) {
        return EXCEPTION_SUCCESS;
    }

    oal_reinit_completion(ps_core_d->wait_read_bfgx_stack);
    /* wait for read stack completion */
    timeleft = wait_for_completion_timeout(&ps_core_d->wait_read_bfgx_stack,
                                           msecs_to_jiffies(WAIT_BFGX_READ_STACK_TIME));
    if (!timeleft) {
        atomic_set(&ps_core_d->is_memdump_runing, 0);
        ps_print_err("[%s]read bfgx stack failed!\n", index2name(ps_core_d->pm_data->index));
        return -EXCEPTION_FAIL;
    }

    ps_print_info("[%s]read bfgx stack success!\n", index2name(ps_core_d->pm_data->index));
    return EXCEPTION_SUCCESS;
}

int32_t is_bfgx_exception(void)
{
    int32_t is_exception;
    unsigned long flag;

    struct st_exception_info *pst_exception_data = get_exception_info_reference();
    if (pst_exception_data == NULL) {
        ps_print_err("get exception info reference is error\n");
        return 0;
    }

    oal_spin_lock_irq_save(&pst_exception_data->exception_spin_lock, &flag);
    if (in_plat_exception_reset() == OAL_TRUE) {
        is_exception = PLAT_EXCEPTION_RESET_BUSY;
    } else {
        is_exception = PLAT_EXCEPTION_RESET_IDLE;
    }
    oal_spin_unlock_irq_restore(&pst_exception_data->exception_spin_lock, &flag);

    return is_exception;
}

void bfgx_memdump_finish(struct ps_core_s *ps_core_d)
{
    int8_t index = ps_core_d->uart_index;

    if (index == BUART) {
        g_bcpu_memdump_cfg.is_working = 0;
    } else if (index == GUART) {
        g_gcpu_memdump_cfg.is_working = 0;
    } else {
        ps_print_err("unknown dev index:%d\n", index);
    }
}

int32_t bfgx_notice_hal_memdump(struct ps_core_s *ps_core_d)
{
    int8_t index = ps_core_d->uart_index;

    if (index == BUART) {
        return notice_hal_memdump(&g_bcpu_memdump_cfg, CMD_READM_BFGX_UART);
    } else if (index == GUART) {
        return notice_hal_memdump(&g_gcpu_memdump_cfg, CMD_READM_ME_UART);
    } else {
        ps_print_err("unknown dev index:%d\n", index);
        return -EINVAL;
    }
}

int32_t bfgx_memdump_enquenue(struct ps_core_s *ps_core_d, uint8_t *buf_ptr, uint16_t count)
{
    int8_t index = ps_core_d->uart_index;

    if (index == BUART) {
        return excp_memdump_queue(buf_ptr, count, &g_bcpu_memdump_cfg);
    } else if (index == GUART) {
        return excp_memdump_queue(buf_ptr, count, &g_gcpu_memdump_cfg);
    } else {
        ps_print_err("unknown dev index:%d\n", index);
        return -EINVAL;
    }
}
