/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2020. All rights reserved.
 * Description: Netfilter hook head file.
 * Author: chenzhongxian@huawei.com
 * Create: 2016-05-28
 */

#ifndef _NF_HW_HOOK
#define _NF_HW_HOOK

#define NETLINK_DOWNLOADEVENT_TO_USER 1000
#define NETLINK_REG_TO_KERNEL 0
#define NETLINK_UNREG_TO_KERNEL 1
#define NETLINK_HWCOMM_TO_KERNEL 2
#define NETLINK_SET_AD_RULE_TO_KERNEL 3
#define NETLINK_SET_APPDL_RULE_TO_KERNEL 4
#define NETLINK_CLR_AD_RULE_TO_KERNEL 5
#define NETLINK_CLR_APPDL_RULE_TO_KERNEL 6
#define NETLINK_OUTPUT_AD_TO_KERNEL 7
#define NETLINK_OUTPUT_APPDL_TO_KERNEL 8
#define NETLINK_APPDL_CALLBACK_TO_KERNEL 9
#define NETLINK_SET_INSTALL_RULE_TO_KERNEL 10
#define NETLINK_CLR_INSTALL_RULE_TO_KERNEL 11
#define NETLINK_OUTPUT_INS_DELTA_TO_KERNEL 12
#define NETLINK_REPORT_DLID_TO_USER 999
#define REPORT_LIMIT_TIMR 1000
#define REPORT_MSG_TYPE 100
#define IPV4_LEN 32
#define SKB_LEN_MIN 5
#define SKB_LEN_AVG 1000
#define SKB_LEN_MAX 2000
#define SKB_HEAD_MASK_8 8
#define SKB_HEAD_MASK_16 16
#define SKB_HEAD_MASK_24 24

#endif /* _NF_HW_HOOK */
