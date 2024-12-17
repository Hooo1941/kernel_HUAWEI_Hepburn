#ifndef __MDRV_BMI_IH_BRIDGE_H__
#define __MDRV_BMI_IH_BRIDGE_H__

#include <linux/skbuff.h>
#include "hooks/mdrv_bmi_ih.h"
#include "br_private.h"

typedef void br_forward_func(const struct net_bridge_port *to,
    struct sk_buff *skb, bool local_rcv, bool local_orig);
typedef int (*br_pass_frame_up_func)(struct sk_buff *skb);

DECLARE_IOT_HOOK(iot_hook_net_mesh_forward_proc,
    IH_PROTO(struct sk_buff *skb, bool local_rcv, br_forward_func forward_proc,
        br_pass_frame_up_func frame_up_proc, bool *has_proc, int *ret),
    IH_ARGS(skb, local_rcv, forward_proc, frame_up_proc, has_proc, ret));

DECLARE_IOT_HOOK(iot_hook_net_mesh_allow_learn,
    IH_PROTO(const struct sk_buff *skb, bool *allow),
    IH_ARGS(skb, allow));

#endif