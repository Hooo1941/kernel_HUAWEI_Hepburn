

#ifndef __PREPARE_FRAME_STA_H__
#define __PREPARE_FRAME_STA_H__

/* 1 其他头文件包含 */
#include "oal_ext_if.h"
#include "oal_types.h"
#include "hmac_vap.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 2 宏定义 */
/* 3 枚举定义 */
/* 4 全局变量声明 */
/* 5 消息头定义 */
/* 6 消息定义 */
/* 7 STRUCT定义 */
/* 8 UNION定义 */
/* 9 OTHERS定义 */
/* 10 函数声明 */
extern oal_uint32 hmac_mgmt_encap_asoc_req_sta(hmac_vap_stru *pst_hmac_sta,
                                               oal_uint8 *puc_req_frame,
                                               oal_uint8 *puc_curr_bssid, uint8_t *dest_addr);
extern oal_uint16 hmac_mgmt_encap_auth_req(hmac_vap_stru *pst_sta, oal_uint8 *puc_mgmt_frame, uint8_t *dest_addr);
extern oal_uint16 hmac_mgmt_encap_auth_req_seq3(hmac_vap_stru *pst_sta,
                                                oal_uint8 *puc_mgmt_frame,
                                                oal_uint8 *puc_mac_hrd);
void hmac_set_supported_rates_ie_asoc_req(hmac_vap_stru *pst_hmac_sta, uint8_t *puc_buffer, uint8_t *puc_ie_len);
void hmac_set_exsup_rates_ie_asoc_req(hmac_vap_stru *pst_hmac_sta, uint8_t *puc_buffer, uint8_t *puc_ie_len);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_encap_frame_sta.h */
