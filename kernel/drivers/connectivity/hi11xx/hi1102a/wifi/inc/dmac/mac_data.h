
#ifndef __MAC_DATA_H__
#define __MAC_DATA_H__

/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "oal_ext_if.h"
#include "wlan_mib.h"
#include "mac_user.h"
#include "oam_ext_if.h"
#include "mac_regdomain.h"
#include "hal_ext_if.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  2 �궨��
*****************************************************************************/
/* ��ʱ���������Ҫ����oal_net.h */
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
#define oal_swap_byteorder_16(_val) ((((_val) & 0x00FF) << 8) + (((_val) & 0xFF00) >> 8))
#define OAL_HOST2NET_SHORT(_x)  oal_swap_byteorder_16(_x)
#define OAL_NET2HOST_SHORT(_x)  oal_swap_byteorder_16(_x)
#define OAL_HOST2NET_LONG(_x)   OAL_SWAP_BYTEORDER_32(_x)
#define OAL_NET2HOST_LONG(_x)   OAL_SWAP_BYTEORDER_32(_x)

#define OAL_IPPROTO_UDP    17 /* User Datagram Protocot */
#define OAL_IPPROTO_ICMPV6 58 /* ICMPv6 */
#endif

#define OAL_EAPOL_INFO_POS 13
#define OAL_EAPOL_TYPE_POS 9
#define OAL_EAPOL_TYPE_KEY 3

/* �ж��Ƿ�Ϊ�ؼ�֡ */
#define OAL_IS_VIP_FRAME(mac_data_type) (((mac_data_type) > MAC_DATA_UNSPEC) && ((mac_data_type) <= MAC_DATA_VIP))
/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/
typedef enum {
    MAC_DATA_UNSPEC = 0,           /* 0 */
    MAC_DATA_DHCP,                 /* 1 */
    MAC_DATA_ARP_REQ,              /* 2 */
    MAC_DATA_ARP_RSP,              /* 3 */
    MAC_DATA_EAPOL,                /* 4 */
    MAC_DATA_VIP = MAC_DATA_EAPOL, /* MAC_DATA_VIP == MAC_DATA_EAPOL, ��߹ؼ�֡�ж�Ч�ʣ�����mips */
    MAC_DATA_ND,                   /* 5 */
    MAC_DATA_DHCPV6,               /* 6 */
    MAC_DATA_WAPI,                 /* 7 */
    MAC_DATA_URGENT_TCP_ACK,       /* 8 */
    MAC_DATA_NORMAL_TCP_ACK,       /* 9 */
    MAC_DATA_TCP_SYN,              /* 10 */
    MAC_DATA_DNS,                  /* 11 */

    MAC_DATA_BUTT, /* 12 */

    MAC_DATA_RTSP, /* 13 */
} mac_data_type_enum;
typedef oal_uint8 mac_data_type_enum_uint8;

typedef enum {
    MAC_NETBUFF_PAYLOAD_ETH = 0,
    MAC_NETBUFF_PAYLOAD_SNAP,

    MAC_NETBUFF_PAYLOAD_BUTT
} mac_netbuff_payload_type;
typedef oal_uint8 mac_netbuff_payload_type_uint8;
extern oal_bool_enum_uint8 mac_is_dhcp_port(mac_ip_header_stru *pst_ip_hdr);
extern oal_bool_enum_uint8 mac_is_nd(oal_ipv6hdr_stru *pst_ipv6hdr);
extern oal_bool_enum_uint8 mac_is_dhcp6(oal_ipv6hdr_stru *pst_ether_hdr);
mac_data_type_enum_uint8 mac_get_arp_type_by_arphdr(oal_eth_arphdr_stru *pst_rx_arp_hdr);
extern oal_uint8 mac_get_data_type(oal_netbuf_stru *pst_netbuff);
extern oal_uint8 mac_get_data_type_from_8023(oal_uint8 *puc_frame_hdr, mac_netbuff_payload_type uc_hdr_type);
oal_bool_enum_uint8 mac_is_eapol_key_ptk(mac_eapol_header_stru *pst_eapol_header);
extern oal_uint8 mac_get_data_type_from_80211(oal_netbuf_stru *pst_netbuff, oal_uint16 us_mac_hdr_len);
extern oal_uint16 mac_get_eapol_keyinfo(oal_netbuf_stru *pst_netbuff);
extern oal_uint8 mac_get_eapol_type(oal_netbuf_stru *pst_netbuff);
extern oal_bool_enum_uint8 mac_is_eapol_key_ptk_4_4(oal_netbuf_stru *pst_netbuff);
extern oal_bool_enum_uint8 mac_is_dns_frame(mac_ip_header_stru *pst_ip_hdr);
extern mac_eapol_type_enum_uint8 mac_get_eapol_key_type(oal_uint8 *pst_payload);

extern oal_uint8 mac_get_dhcp_frame_type(oal_ip_header_stru *pst_rx_ip_hdr);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of mac_vap.h */
