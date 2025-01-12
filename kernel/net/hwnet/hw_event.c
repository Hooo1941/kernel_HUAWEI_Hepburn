

#include <chipset_common/hwnet/hw_event.h>
#include <linux/etherdevice.h>
#include <linux/if_arp.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/wireless.h>
#include <linux/export.h>
#include <net/iw_handler.h>
#include <net/cfg80211.h>
#include <net/rtnetlink.h>
#include <../net/wireless/nl80211.h>
#include <../net/wireless/reg.h>
#include <../net/wireless/rdev-ops.h>
#ifdef CONFIG_HW_WIFI_TEMP
#include <../net/wireless/core.h>
#endif

#ifdef CONFIG_HW_ABS
/*
 * This function is used to report the event to the grab of the antenna
 * in ABS project.
 */
void cfg80211_drv_ant_grab(struct net_device *dev, gfp_t gfp)
{
	struct wireless_dev *wdev = dev->ieee80211_ptr;
	struct cfg80211_registered_device *rdev = wiphy_to_rdev(wdev->wiphy);
	struct cfg80211_event *ev;
	unsigned long flags;

	ev = kzalloc(sizeof(*ev), gfp);
	if (ev == NULL) {
		printk(KERN_ERR "%s: malloc fail\n", __FUNCTION__);
		return;
	}
	ev->type = EVENT_DRV_ANT;
	spin_lock_irqsave(&wdev->event_lock, flags);
	list_add_tail(&ev->list, &wdev->event_list);
	spin_unlock_irqrestore(&wdev->event_lock, flags);
	queue_work(cfg80211_wq, &rdev->event_work);
}
EXPORT_SYMBOL(cfg80211_drv_ant_grab);
#endif

#ifdef CONFIG_HW_WIFI_MSS
void cfg80211_drv_mss_result(struct net_device *dev, gfp_t gfp, const u8 *buf,
	size_t len)
{
	struct wireless_dev *wdev = dev->ieee80211_ptr;
	struct cfg80211_registered_device *rdev = wiphy_to_rdev(wdev->wiphy);
	struct cfg80211_event *ev;
	unsigned long flags;

	ev = kzalloc(sizeof(*ev) + len, gfp);
	if (ev == NULL)
		return;

	ev->type = EVENT_DRV_MSS;
	ev->dc.ie = ((u8 *)ev) + sizeof(*ev);
	ev->dc.ie_len = len;
	memcpy((void *)ev->dc.ie, buf, len);
	spin_lock_irqsave(&wdev->event_lock, flags);
	list_add_tail(&ev->list, &wdev->event_list);
	spin_unlock_irqrestore(&wdev->event_lock, flags);
	queue_work(cfg80211_wq, &rdev->event_work);
}
EXPORT_SYMBOL(cfg80211_drv_mss_result);
#endif

#ifdef CONFIG_HW_WIFI_RSSI
void cfg80211_drv_tas_result(struct net_device *dev, gfp_t gfp, const u8 *buf,
	size_t len)
{
	struct wireless_dev *wdev = dev->ieee80211_ptr;
	struct cfg80211_registered_device *rdev = wiphy_to_rdev(wdev->wiphy);
	struct cfg80211_event *ev;
	unsigned long flags;

	ev = kzalloc(sizeof(*ev) + len, gfp);
	if (ev == NULL)
		return;

	ev->type = EVENT_DRV_TAS_RSSI;
	ev->dc.ie = ((u8 *)ev) + sizeof(*ev);
	ev->dc.ie_len = len;
	memcpy((void *)ev->dc.ie, buf, len);
	spin_lock_irqsave(&wdev->event_lock, flags);
	list_add_tail(&ev->list, &wdev->event_list);
	spin_unlock_irqrestore(&wdev->event_lock, flags);
	queue_work(cfg80211_wq, &rdev->event_work);
}
EXPORT_SYMBOL(cfg80211_drv_tas_result);
#endif

#ifdef CONFIG_HW_WIFI_TEMP
void cfg80211_drv_temp_result(struct net_device *dev, gfp_t gfp, const u8 *buf,
	size_t len)
{
	struct wireless_dev *wdev = dev->ieee80211_ptr;
	struct cfg80211_registered_device *rdev = wiphy_to_rdev(wdev->wiphy);
	struct cfg80211_event *ev;
	unsigned long flags;

	ev = kzalloc(sizeof(*ev) + len, gfp);
	if (ev == NULL)
		return;

	ev->type = EVENT_OVERTEMP;
	ev->dc.ie = ((u8 *)ev) + sizeof(*ev);
	ev->dc.ie_len = len;
	memcpy((void *)ev->dc.ie, buf, len);
	spin_lock_irqsave(&wdev->event_lock, flags);
	list_add_tail(&ev->list, &wdev->event_list);
	spin_unlock_irqrestore(&wdev->event_lock, flags);
	queue_work(cfg80211_wq, &rdev->event_work);
}
EXPORT_SYMBOL(cfg80211_drv_temp_result);
#endif

#ifdef CONFIG_HW_WIFI_EVENT
void cfg80211_drv_event_result(struct net_device *dev, gfp_t gfp, const u8 *buf,
	size_t len)
{
	struct wireless_dev *wdev = dev->ieee80211_ptr;
	struct cfg80211_registered_device *rdev = wiphy_to_rdev(wdev->wiphy);
	struct cfg80211_event *ev;
	unsigned long flags;

	ev = kzalloc(sizeof(*ev) + len, gfp);
	if (ev == NULL)
		return;

	ev->type = EVENT_DRV_WIFI_EVENT;
	ev->dc.ie = ((u8 *)ev) + sizeof(*ev);
	ev->dc.ie_len = len;
	memcpy((void *)ev->dc.ie, buf, len);
	spin_lock_irqsave(&wdev->event_lock, flags);
	list_add_tail(&ev->list, &wdev->event_list);
	spin_unlock_irqrestore(&wdev->event_lock, flags);
	queue_work(cfg80211_wq, &rdev->event_work);
}
EXPORT_SYMBOL(cfg80211_drv_event_result);
#endif
