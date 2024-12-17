

#ifndef APP_PROXY_H
#define APP_PROXY_H

#include "netlink_handle.h"

#define NOTIFY_BUF_LEN 512

struct app_proxy_info {
	u16 type;
	u16 len;
	u8 is_proxy_enable;
};

void send_porxy_result(u32, u32);
msg_process *hw_app_proxy_init(notify_event *);

#endif
