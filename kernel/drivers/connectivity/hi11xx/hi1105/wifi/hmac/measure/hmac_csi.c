/*
* Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
* 功能说明   : CSI功能
* 作者       : wifi
* 创建日期   : 2020年11月17日
*/
/* 1 头文件包含 */
#include "oal_kernel_file.h"
#include "mac_data.h"
#include "hmac_resource.h"
#include "hmac_host_tx_data.h"
#include "hmac_tx_data.h"
#include "hmac_config.h"
#include "plat_pm_wlan.h"
#include "hmac_csi.h"
#include "hmac_ftm.h"
#include "host_hal_ext_if.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CSI_C

#include "securec.h"
#include "securectype.h"

#ifdef _PRE_WLAN_FEATURE_CSI
void hmac_proc_location_print_timestamp(oal_file *file)
{
    oal_timespec_stru time;
    oal_time_stru local_time;
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    oal_do_gettimeofday(&time);
#endif
    oal_get_real_time_m(&local_time);

    oal_kernel_file_print(file, "%04d-%02d-%02d-", local_time.tm_year + BASE_YEAR,
        local_time.tm_mon + 1, local_time.tm_mday);
    oal_kernel_file_print(file, "%02d-%02d-%02d-%08d : ", local_time.tm_hour,
        local_time.tm_min, local_time.tm_sec, oal_get_usec(&time));
}

/*
 * 功能描述  : hmac ftm/csi数据处理接口
 * 1.日    期  : 2020年11月19日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_csi_handle(hal_host_location_isr_stru *ftm_csi_isr)
{
    return;
}

/*
 * 功能描述  : 06 host配置ack缓存参数
 * 1.日    期  : 2020年11月17日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_host_csi_config(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    return hal_host_csi_config(mac_vap, len, param);
}

#endif

#ifdef _PRE_WLAN_FEATURE_FTM
/*
 * 功能描述  : host ftm/csi数据处理接口
 * 1.日    期  : 2020年11月19日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_rx_location_data_event(frw_event_mem_stru *event_mem)
{
    hal_host_location_isr_stru *ftm_csi_isr = NULL;
    uint32_t ftm_csi_flag;

    if (oal_unlikely(event_mem == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    ftm_csi_isr = (hal_host_location_isr_stru *)frw_get_event_stru(event_mem)->auc_event_data;

    ftm_csi_flag = ftm_csi_isr->ftm_csi_info;

#ifdef _PRE_WLAN_FEATURE_CSI
    if (ftm_csi_flag & BIT0) {  /* CSI 中断 */
        hmac_csi_handle(ftm_csi_isr);
    }
#endif
    return OAL_SUCC;
}
#endif
