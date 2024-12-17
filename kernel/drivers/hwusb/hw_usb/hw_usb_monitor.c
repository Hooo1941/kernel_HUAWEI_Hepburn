

#include <chipset_common/hwusb/hw_usb.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/usb/hw_pd_dev.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/usb.h>
#include <linux/usb/hcd.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <securec.h>

#define HWLOG_TAG hw_usb_monitor
HWLOG_REGIST();

#define HW_USB_MONITOR_EVENT_SIZE   64
#define HW_USB_CMD_MAX_LEN          64

#define USB_MONITOR_EVT_FUNCTEST    0
#define USB_MONITOR_EVT_ENUMSPD     1
#define USB_MONITOR_EVT_EXIT        2
#define USB_MONITOR_EVT_ENTER       3
#define USB_MONITOR_EVT_OLOGON      4
#define USB_MONITOR_EVT_OLOGOFF     5
#define USB_MONITOR_EVT_QREGSET     6
#define USB_MONITOR_EVT_QREGCLR     7
#define USB_MONITOR_EVT_RECHG       8
#define USB_MONITOR_EVT_SETFLG      9
#define USB_MONITOR_EVT_END         10

#define high_word(n)                (((n) >> 16) & 0xFFFF)
#define low_word(n)                 ((n) & 0xFFFF)

enum hw_usb_ctl_cmd {
	HW_USB_CTL_CMD_UNKNOWN,
	HW_USB_CTL_CMD_PR,
	HW_USB_CTL_CMD_DR,
	HW_USB_CTL_CMD_ROLE,
	HW_USB_CTL_CMD_UDC,
	HW_USB_CTL_CMD_STATE,
};

struct um_cmd_pair {
	unsigned int cmd;
	char head[HW_USB_CMD_MAX_LEN];
};

static struct um_cmd_pair g_um_cmd_set[] = {
	{ USB_MONITOR_EVT_FUNCTEST, "um.test:" },
	{ USB_MONITOR_EVT_ENUMSPD, "um.enum.spd" },
	{ USB_MONITOR_EVT_EXIT, "um.exit" },
	{ USB_MONITOR_EVT_ENTER, "um.enter" },
	{ USB_MONITOR_EVT_OLOGON, "um.ologon" },
	{ USB_MONITOR_EVT_OLOGOFF, "um.ologoff" },
	{ USB_MONITOR_EVT_QREGSET, "um.qreg.set:" },
	{ USB_MONITOR_EVT_QREGCLR, "um.qreg.clear" },
	{ USB_MONITOR_EVT_RECHG, "um.rechg" },
	{ USB_MONITOR_EVT_SETFLG, "um.flag:" }
};

static unsigned int g_event_mask;
static struct work_struct g_event_work;
static char g_event_cmd[HW_USB_CMD_MAX_LEN];
static struct hw_usb_device *g_hw_um_di;
static int g_hw_um_flag_temp;

static bool hw_usb_str_jump_hdr(const char **pstr, const char *head, int *pval)
{
	const char *ptr = *pstr;
	long val;

	if (strncmp(ptr, head, strlen(head)))
		return false;

	ptr += strlen(head);
	if (!pval) {
		*pstr = ptr;
		return true;
	}

	if ((*ptr == 0) || (*ptr < '0') || (*ptr > '9'))
		return false;

	if (kstrtol(ptr, 10, &val) < 0)
		return false;

	*pval = val;
	return (*pval >= 0);
}

static int hw_usb_uevment_add_env(char *str)
{
	int ret;
	char *envp[] = { NULL, NULL };

	if ((g_hw_um_di == NULL) || !g_hw_um_di->child_usb_dev || (str == NULL)) {
		hwlog_err("g_hw_usb_di or child_usb_dev is null\n");
		return -ENODEV;
	}

	envp[0] = str;
	ret = kobject_uevent_env(&g_hw_um_di->child_usb_dev->kobj, KOBJ_CHANGE, envp);
	if (ret < 0)
		hwlog_err("uevent sending failed ret=%d\n", ret);
	else
		hwlog_info("sent uevent %s\n", envp[0]);

	return ret;
}

static int hw_usb_send_ctlcmd_event(enum hw_usb_ctl_cmd cmd, const char *arg)
{
	char event_buf[HW_USB_MONITOR_EVENT_SIZE] = {0};
	int ret = -EINVAL;

	if ((cmd == HW_USB_CTL_CMD_UNKNOWN) || !arg || (strlen(arg) == 0)) {
		hwlog_err("invalid cmd %d\n", cmd);
		return -EINVAL;
	}

	if ((g_hw_um_di == NULL) || (g_hw_um_di->usb_monitor_flag == 0)) {
		hwlog_err("mask not set\n");
		return -EINVAL;
	}

	hwlog_info("send vendor cmd %d, arg %s\n", cmd, arg);

	switch (cmd) {
	case HW_USB_CTL_CMD_PR:
		ret = sprintf_s(event_buf, sizeof(event_buf), "umctlcmd:power_role=%s", arg);
		break;
	case HW_USB_CTL_CMD_DR:
		ret = sprintf_s(event_buf, sizeof(event_buf), "umctlcmd:data_role=%s", arg);
		break;
	case HW_USB_CTL_CMD_ROLE:
		ret = sprintf_s(event_buf, sizeof(event_buf), "umctlcmd:role=%s", arg);
		break;
	case HW_USB_CTL_CMD_UDC :
		ret = sprintf_s(event_buf, sizeof(event_buf), "umctlcmd:udc=%s", arg);
		break;
	case HW_USB_CTL_CMD_STATE :
		ret = sprintf_s(event_buf, sizeof(event_buf), "umctlcmd:state=%s", arg);
		break;
	default:
		return -EINVAL;
	}

	if (ret < 0) {
		hwlog_err("Format arg failed %d\n", ret);
		return ret;
	}

	return hw_usb_uevment_add_env(event_buf);
}

/* copy from '/sys/kernel/debug/usb/devcies', proc in 'usb_device_dump' */
static void hw_usb_max_usbspeed(struct usb_device *dev, int depth,
	enum usb_device_speed *spd_dev, enum usb_device_speed *spd_hub)
{
	struct usb_device *childdev = NULL;
	int chix;

	/* Check 5 layers at the most */
	if (depth >= 5) {
		hwlog_err("get usb speed too deep.\n");
		return;
	}

	hwlog_info("depth=%d, speed=%d, class=0x%x, child=%d\n", depth,
		(int)dev->speed, dev->descriptor.bDeviceClass, dev->maxchild);

	if (dev->descriptor.bDeviceClass != USB_CLASS_HUB)
		*spd_dev = (*spd_dev > dev->speed) ? *spd_dev : dev->speed;
	else
		*spd_hub = (*spd_hub > dev->speed) ? *spd_hub : dev->speed;

	usb_hub_for_each_child(dev, chix, childdev) {
		usb_lock_device(childdev);
		hw_usb_max_usbspeed(childdev, (depth + 1), spd_dev, spd_hub);
		usb_unlock_device(childdev);
	}
}

static int hw_usb_check_host_spd(struct hw_usb_device *di)
{
	int id;
	struct usb_bus *bus;
	enum usb_device_speed dev_spd = USB_SPEED_UNKNOWN;
	enum usb_device_speed hub_spd = USB_SPEED_UNKNOWN;

	mutex_lock(&usb_bus_idr_lock);
	/* print devices for all busses */
	idr_for_each_entry(&usb_bus_idr, bus, id) {
		/* recurse through all children of the root hub */
		if (!bus_to_hcd(bus)->rh_registered)
			continue;
		usb_lock_device(bus->root_hub);
		hw_usb_max_usbspeed(bus->root_hub, 0, &dev_spd, &hub_spd);
		usb_unlock_device(bus->root_hub);
	}
	mutex_unlock(&usb_bus_idr_lock);

	if (di->max_dev_spd != dev_spd) {
		hwlog_info("Max device speed %d - %d\n", di->max_dev_spd, dev_spd);
		di->max_dev_spd = dev_spd;
	}

	if (di->max_hub_spd != hub_spd) {
		hwlog_info("Max hub speed %d - %d\n", di->max_hub_spd, hub_spd);
		di->max_hub_spd = hub_spd;
	}

	return 0;
}

static int hw_usb_monitor_ctl_test(const char *func)
{
	int val;
	unsigned int misc;
	int ret = 0;
	const char *buf = func;

	if (!func || !strlen(func))
		return -EINVAL;

	hwlog_info("%s: test function %s\n", __func__, func);

	if (hw_usb_str_jump_hdr(&buf, "pr=", &val)) {
		ret = hw_usb_set_typec_pr((!val) ? TYPEC_SINK : TYPEC_SOURCE);
	} else if (hw_usb_str_jump_hdr(&buf, "dr=", &val)) {
		ret = hw_usb_set_typec_dr((!val) ? TYPEC_DEVICE : TYPEC_HOST);
	} else if (hw_usb_str_jump_hdr(&buf, "role=", &val)) {
		ret = hw_usb_set_usb_role((!val) ? USB_ROLE_NONE :
			((val == 1) ? USB_ROLE_HOST : USB_ROLE_DEVICE));
	} else if (hw_usb_str_jump_hdr(&buf, "udc=", &val)) {
		ret = hw_usb_set_udc_state(val != 0);
	} else if (hw_usb_str_jump_hdr(&buf, "misc=", &val)) {
		misc = (unsigned int)val;
		ret = pd_dpm_send_misc_cmd(low_word(misc), high_word(misc));
	} else {
		hwlog_info("invaid func for test\n");
		ret = -EINVAL;
	}

	return ret;
}

static int hw_usb_parse_flag(struct hw_usb_device *di, const char *flag)
{
	long val = 0;

	if ((kstrtol(flag, 10, &val) < 0) || (val < 0)) {
		hwlog_err("paser mask string failed\n");
		return -EINVAL;
	}

	di->usb_monitor_flag = (unsigned int)val;
	hwlog_info("Set mask as %x\n", di->usb_monitor_flag);

	return 0;
}

static void hw_usb_monitor_work(struct work_struct *work)
{
	struct hw_usb_device *di = g_hw_um_di;
	char buf[HW_USB_CMD_MAX_LEN] = { 0 };
	int ret = 0;

	if (di == NULL) {
		hwlog_err("Um not start\n");
		return;
	}

	hwlog_info("Um work to process %u\n", g_event_mask);

	switch (g_event_mask) {
	case USB_MONITOR_EVT_ENUMSPD :
		ret = hw_usb_check_host_spd(di);
		break;
	case USB_MONITOR_EVT_OLOGON :
	case USB_MONITOR_EVT_OLOGOFF :
	case USB_MONITOR_EVT_QREGCLR:
		hw_usb_send_ctlcmd_event(HW_USB_CTL_CMD_STATE,
			g_um_cmd_set[g_event_mask].head + strlen("um."));
		break;
	case USB_MONITOR_EVT_QREGSET :
		ret = sprintf_s(buf, HW_USB_CMD_MAX_LEN, "qreg.set:%s", g_event_cmd);
		if (ret > 0)
			hw_usb_send_ctlcmd_event(HW_USB_CTL_CMD_STATE, buf);
		break;
	case USB_MONITOR_EVT_RECHG:
		hwlog_info("do rechg\n");
		break;
	case USB_MONITOR_EVT_FUNCTEST :
		ret = hw_usb_monitor_ctl_test(g_event_cmd);
		break;
	case USB_MONITOR_EVT_SETFLG :
		ret = hw_usb_parse_flag(di, g_event_cmd);
		break;
	default :
		break;
	}

	hwlog_info("Um work process ret %u\n", ret);

	g_event_mask = USB_MONITOR_EVT_END;
	g_event_cmd[0] = 0;
}

int hw_usb_set_typec_pr(enum typec_role role)
{
	const char *arg = (role == TYPEC_SINK) ? "sink" : "source";
	return hw_usb_send_ctlcmd_event(HW_USB_CTL_CMD_PR, arg);
}

int hw_usb_set_typec_dr(enum typec_data_role role)
{
	const char *arg = (role == TYPEC_DEVICE) ? "device" : "host";
	return hw_usb_send_ctlcmd_event(HW_USB_CTL_CMD_DR, arg);
}

int hw_usb_set_usb_role(enum usb_role role)
{
	const char *arg = (role == USB_ROLE_NONE) ? "none" :
		((role == USB_ROLE_HOST) ? "host" : "device");
	return hw_usb_send_ctlcmd_event(HW_USB_CTL_CMD_ROLE, arg);
}

int hw_usb_set_udc_state(bool enable)
{
	const char *arg = enable ? "enable" : "disable";
	return hw_usb_send_ctlcmd_event(HW_USB_CTL_CMD_UDC, arg);
}

void hw_usb_monitor_proc_cmdbuf(const char *buf, int size)
{
	int idx;
	int ret;

	if ((g_hw_um_di == NULL) || (buf == NULL) || (size == 0))
		return;

	hwlog_info("%s: get buf [%s]\n", __func__, buf);
	g_event_mask = USB_MONITOR_EVT_END;
	for (idx = 0; idx < ARRAY_SIZE(g_um_cmd_set); idx++) {
		if (hw_usb_str_jump_hdr(&buf, g_um_cmd_set[idx].head, NULL)) {
			g_event_mask = g_um_cmd_set[idx].cmd;
			ret = strcpy_s(g_event_cmd, HW_USB_CMD_MAX_LEN, buf);
			if (ret) {
				hwlog_err("copy buf param failed\n");
				g_event_mask = USB_MONITOR_EVT_END;
			}
			break;
		}
	}

	if (g_event_mask < USB_MONITOR_EVT_END) {
		hwlog_info("%s: set event mask %x\n", __func__, g_event_mask);
		schedule_work(&g_event_work);
	}
}

void hw_usb_monitor_handle_event(unsigned long event)
{
	char event_buf[HW_USB_MONITOR_EVENT_SIZE] = "OTG_CONNECT=";

	if ((g_hw_um_di == NULL) || (g_hw_um_di->usb_monitor_flag == 0)) {
		hwlog_info("Um not start, ignore event %lu\n", event);
		return;
	}

	switch (event) {
	case HW_USB_EVENT_OTG_ON:
		hw_usb_uevment_add_env(event_buf);
		break;
	default :
		break;
	}
}

void hw_usb_monitor_init(struct hw_usb_device *di)
{
	int ret;

	if (di == NULL)
		return;

	di->max_dev_spd = USB_SPEED_UNKNOWN;
	di->max_hub_spd = USB_SPEED_UNKNOWN;

	INIT_WORK(&g_event_work, hw_usb_monitor_work);
	g_hw_um_di = di;
	g_hw_um_di->usb_monitor_flag = g_hw_um_flag_temp;
	hwlog_info("init um_flag 0x%x\n", g_hw_um_flag_temp);
}

void hw_usb_monitor_set_flag(unsigned int flag)
{
	hwlog_info("set um_flag 0x%x\n", flag);
	g_hw_um_flag_temp = flag;
	if (g_hw_um_di != NULL)
		g_hw_um_di->usb_monitor_flag = g_hw_um_flag_temp;
}

