

#include "app_proxy.h"

#include "netlink_handle.h"
#include "hw_booster_common.h"

#include "hwnet/network_dcp/network_dcp_handle.h"

static notify_event *notifier = NULL;

void app_proxy_process(struct req_msg_head *msg, u32 len)
{
	if (!msg)
		return;

	if (msg->len != len) {
		pr_err("app_proxy_process msg len error!");
		return;
	}

	if (msg->type == APP_PROXY_CMD) {
		struct app_proxy_info *appproxy = (struct app_proxy_info *)msg;
		if (appproxy == NULL)
			return;
		if (appproxy->is_proxy_enable) {
			dcp_close_proxy(true);
		} else {
			dcp_close_proxy(false);
		}
	}
}

msg_process *__init hw_app_proxy_init(notify_event *fn)
{
	if (!fn) {
		pr_info("app_proxy_init null parameter");
		return NULL;
	}
	notifier = fn;
	pr_info("hw_app_proxy_init success");
	return app_proxy_process;
}

void send_porxy_result(u32 uid, u32 result)
{
	char event[NOTIFY_BUF_LEN];
	char *p = event;
	if (!notifier) {
		pr_info("network_dcp notifier is null");
		return;
	}

	// type
	assign_short(p, APP_PROXY_RESULT_RPT);
	skip_byte(p, sizeof(s16));

	// len 12 = 2B type + 2B len + 4B uid + 4B result
	assign_short(p, 12);
	skip_byte(p, sizeof(s16));

	// uid
	assign_uint(p, uid);
	skip_byte(p, sizeof(int));

	// result
	assign_uint(p, result);
	skip_byte(p, sizeof(int));
	notifier((struct res_msg_head *)event);
}
