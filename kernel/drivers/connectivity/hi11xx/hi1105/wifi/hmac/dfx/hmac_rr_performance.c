/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : power managemet
 * 作    者 : wifi
 * 创建日期 : 2019年8月28日
 */

#ifdef _PRE_WLAN_RR_PERFORMENCE_DEBUG
#include "hmac_rr_performance.h"

/* 保存RR性能host打点 */
hmac_perform_host_timestamp_t g_host_rr_perform_timestamp;

/*
 * 功能描述  : 获得ktime时间戳
 * 1.日    期  : 2020年2月11日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint64_t hmac_get_ktimestamp(void)
{
    ktime_t ktime;
    uint64_t time_us;

    ktime = ktime_get_real();
    time_us = (uint64_t)ktime_to_us(ktime);

    return time_us;
}
/*
 * 功能描述  : RR性能检测发送流程wal to hmac位置打点
 * 1.日    期  : 2020年2月11日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_rr_tx_w2h_timestamp(void)
{
    if (g_host_rr_perform_timestamp.rr_switch == OAL_SWITCH_ON) {
        g_host_rr_perform_timestamp.tx_w2hmac_timestamp = hmac_get_ktimestamp();
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_rr_tx_w2h_timestamp::tx_w2hmac_timestamp = %llu}\n",
                         g_host_rr_perform_timestamp.tx_w2hmac_timestamp);
    }
}
/*
 * 功能描述  :RR性能检测发送流程hmac to dmac位置打点
 * 1.日    期  : 2020年2月11日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_rr_tx_h2d_timestamp(void)
{
    if (g_host_rr_perform_timestamp.rr_switch == OAL_SWITCH_ON) {
        g_host_rr_perform_timestamp.tx_hmac2d_timestamp = hmac_get_ktimestamp();
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_rr_tx_h2d_timestamp::tx_hmac2d_timestamp = %llu}\r\n",
                         g_host_rr_perform_timestamp.tx_hmac2d_timestamp);
    }
}
/*
 * 功能描述  :RR性能检测接收流程dmac to hmac位置打点
 * 1.日    期  : 2020年2月11日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_rr_rx_d2h_timestamp(void)
{
    if (g_host_rr_perform_timestamp.rr_switch == OAL_SWITCH_ON) {
        g_host_rr_perform_timestamp.rx_d2hmac_timestamp = hmac_get_ktimestamp();
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_rr_rx_d2h_timestamp::rx_d2hmac_timestamp = %llu}\r\n",
                         g_host_rr_perform_timestamp.rx_d2hmac_timestamp);
    }
}
/*
 * 功能描述  :RR性能检测接收流程dmac to hmac位置打点
 * 1.日    期  : 2020年2月11日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_rr_rx_h2w_timestamp(void)
{
    if (g_host_rr_perform_timestamp.rr_switch == OAL_SWITCH_ON) {
        g_host_rr_perform_timestamp.rx_hmac2w_timestamp = hmac_get_ktimestamp();
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_rr_rx_h2w_timestamp::rx_hmac2w_timestamp = %llu}\r\n",
                         g_host_rr_perform_timestamp.rx_hmac2w_timestamp);
    }
}
#endif
