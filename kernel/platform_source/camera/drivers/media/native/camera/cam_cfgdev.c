/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2020. All rights reserved.
 * Description: config devive source file.
 * Create: 2016-04-01
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/atomic.h>
#include <linux/fs.h>
#include <linux/debugfs.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/freezer.h>
#include <linux/pid.h>
#include <linux/slab.h>
#include <linux/videodev2.h>
#include <media/media-device.h>
#include <media/v4l2-dev.h>
#include <media/v4l2-device.h>
#include <media/v4l2-event.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-ioctl.h>
#include <securec.h>

#include "cam_intf.h"
#include "cam_log.h"
#include <dsm/dsm_pub.h>
#include <linux/of.h>

struct dsm_client_ops g_ops2 = {
	.poll_state = NULL,
	.dump_func = NULL,
};

struct dsm_dev g_dev_camera_user = {
	.name = "dsm_camera_user",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = &g_ops2,
	.buff_size = 4096, /* 4096, buf size */
};

struct dsm_client *client_camera_user;

typedef struct _tag_cam_cfgdev_vo {
	struct v4l2_device    v4l2;
	struct video_device   *vdev;
	struct media_device   *mdev;

	struct dentry         *debug_root;
	struct v4l2_fh        rq;
	__u8                  sbuf[64]; /* 64, buf len */
	struct mutex          lock;
} cam_cfgdev_vo_t;

typedef struct _tag_motion_data {
	__u8		      motiondata[4];
} cam_cfgdev_motion_dat_t;

static DEFINE_MUTEX(s_cfgdev_lock);
static cam_cfgdev_vo_t s_cfgdev;
static cam_cfgdev_motion_dat_t s_motion_data;

static DEFINE_MUTEX(s_cfgsvr_lock);
static struct pid *s_pid_cfgsvr;

static int cam_cfgdev_send_req(struct v4l2_event *ev, struct v4l2_fh *target);
static ssize_t guard_thermal_show(struct device_driver *drv, char *buf);
static ssize_t guard_thermal_store(struct device_driver *drv,
		const char *buf, size_t count);
static DRIVER_ATTR_RW(guard_thermal);

static ssize_t dump_meminfo_show(struct device_driver *drv, char *buf);
static ssize_t dump_meminfo_store(struct device_driver *drv,
		const char *buf, size_t count);
static DRIVER_ATTR_RW(dump_meminfo);

static ssize_t get_motion_data_show(struct device_driver *drv, char *buf);
static ssize_t get_motion_data_store(struct device_driver *drv,
		const char *buf, size_t count);
static DRIVER_ATTR_RW(get_motion_data);

/* set camera log level by device */
uint32_t cam_debug_mask = CAM_DEBUG_INFO | CAM_DEBUG_EMERG |
CAM_DEBUG_ALERT | CAM_DEBUG_CRIT | CAM_DEBUG_ERR | CAM_DEBUG_WARING;

#ifdef DEBUG_KERNEL_CAMERA
static ssize_t cam_log_show(struct device_driver *drv, char *buf);
static ssize_t cam_log_store(struct device_driver *drv,
		const char *buf, size_t count);
static DRIVER_ATTR_RW(cam_log);

static int s_fbd;
static ssize_t fbd_notify_show(struct device_driver *drv, char *buf);
static ssize_t fbd_notify_store(struct device_driver *drv,
		const char *buf, size_t count);
static DRIVER_ATTR_RW(fbd_notify);
#endif

static int cam_cfgdev_guard_thermal(void)
{
	struct v4l2_event ev = {
		.type = CAM_V4L2_EVENT_TYPE,
		.id = CAM_CFGDEV_REQUEST,
	};
	cam_cfgreq2dev_t *req = (cam_cfgreq2dev_t *)ev.u.data;

	req->kind = CAM_CFGDEV_REQ_GUARD_THERMAL;
	return cam_cfgdev_send_req(&ev, &s_cfgdev.rq);
}

static ssize_t guard_thermal_store(struct device_driver *drv,
				   const char *buf, size_t count)
{
	errno_t err;
	size_t size_to_copy;
	const size_t sbuf_size = sizeof(s_cfgdev.sbuf);

	(void)drv;
	if (!buf || count == 0) {
		cam_err("%s input buf is null, or count:%zu invalid",
			__func__, count);
		return 0;
	}

	cam_cfg_info("%s enter", __func__);
	err = memset_s(s_cfgdev.sbuf, sbuf_size, 0, sbuf_size);
	if (err != EOK)
		cam_warn("%s memset_s fail", __func__);

	size_to_copy = min_t(size_t, count, sbuf_size - 1);
	if (count > size_to_copy)
		cam_warn("%s count[%zu] is beyond sbuf size[%zu]",
			__func__, count, sizeof(s_cfgdev.sbuf));

	err = memcpy_s(s_cfgdev.sbuf, sbuf_size - 1, buf, size_to_copy);
	if (err != EOK)
		cam_warn("%s memcpy_s fail", __func__);
	s_cfgdev.sbuf[sbuf_size - 1] = '\0';
	(void)cam_cfgdev_guard_thermal();
	return count;
}

static ssize_t guard_thermal_show(struct device_driver *drv, char *buf)
{
	int ret;

	(void)drv;
	if (!buf) {
		cam_err("%s input buf is null", __func__);
		return 0;
	}

	cam_cfg_info("%s enter", __func__);
	ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
			"guard thermal:[%s]\n", s_cfgdev.sbuf);
	if (ret < 0)
		cam_warn("%s snprintf_s fail", __func__);
	return ret < 0 ? 0 : ret;
}

static void cam_cfgdev_dump_meminfo(void)
{
	struct v4l2_event ev = {
		.type = CAM_V4L2_EVENT_TYPE,
		.id = CAM_CFGDEV_REQUEST,
	};
	cam_cfgreq2dev_t *req = (cam_cfgreq2dev_t *)ev.u.data;
	req->kind = CAM_CFGDEV_REQ_DUMP_MEMINFO;
	(void)cam_cfgdev_send_req(&ev, &s_cfgdev.rq);
}

static ssize_t dump_meminfo_store(struct device_driver *drv,
				  const char *buf, size_t count)
{
	(void)drv;
	if (!buf)
		return 0;
	return strnlen(buf, count);
}

static ssize_t dump_meminfo_show(struct device_driver *drv, char *buf)
{
	(void)drv;
	(void)buf;
	cam_cfg_info("%s enter", __func__);
	cam_cfgdev_dump_meminfo();
	return 0;
}

static int cam_cfgdev_get_motion_data(void)
{
	struct v4l2_event ev = {
		.type = CAM_V4L2_EVENT_TYPE,
		.id = CAM_CFGDEV_REQUEST,
	};
	cam_cfgreq2dev_t *req = (cam_cfgreq2dev_t *)ev.u.data;

	req->kind = CAM_CFGDEV_REQ_GET_MOTION_DATA;
	return cam_cfgdev_send_req(&ev, &s_cfgdev.rq);
}

static ssize_t get_motion_data_store(struct device_driver *drv,
				     const char *buf, size_t count)
{
	errno_t err;
	size_t size_to_copy;
	const size_t motiondata_size = sizeof(s_motion_data);

	(void)drv;
	if (!buf || count <= 1) {
		cam_err("%s input buf is null, or count:%zu invalid",
			__func__, count);
		return 0;
	}

	cam_cfg_info("%s enter", __func__);
	err = memset_s(&s_motion_data, motiondata_size, 0, motiondata_size);
	if (err != EOK)
		cam_warn("%s memset_s fail", __func__);

	size_to_copy = min_t(size_t, count, motiondata_size - 1);
	if (count > size_to_copy)
		cam_warn("%s count[%zu] is beyond motiondata size[%zu]",
			__func__, count, sizeof(s_motion_data));

	err = memcpy_s(&s_motion_data, motiondata_size - 1, buf, size_to_copy);
	if (err != EOK)
		cam_warn("%s memcpy_s fail", __func__);
	s_motion_data.motiondata[motiondata_size - 1] = '\0';
	(void)cam_cfgdev_get_motion_data();
	return (ssize_t)count;
}

static ssize_t get_motion_data_show(struct device_driver *drv, char *buf)
{
	(void)drv;
	(void)buf;
	cam_cfg_info("%s enter", __func__);
	return 0;
}

#ifdef DEBUG_KERNEL_CAMERA
static ssize_t cam_log_store(struct device_driver *drv,
		const char *buf, size_t count)
{
	unsigned long val = 0;

	(void)drv;
	if (!buf) {
		cam_err("%s input param buf is null", __func__);
		return 0;
	}

	if (kstrtoul(buf, 0, &val) < 0)
		cam_err("%s: %s is not in hex or decimal form", __func__, buf);
	else
		cam_debug_mask = val;

	return strnlen(buf, count);
}

static ssize_t cam_log_show(struct device_driver *drv, char *buf)
{
	int ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
			"0x%08x\n", cam_debug_mask);
	if (ret < 0)
		cam_warn("%s snprintf_s fail", __func__);
	(void)drv;
	return ret;
}

static void cam_cfgdev_fbd_notify(void)
{
	struct v4l2_event ev = {
		.type = CAM_V4L2_EVENT_TYPE,
		.id = CAM_CFGDEV_REQUEST,
	};
	cam_cfgreq2dev_t *req = (cam_cfgreq2dev_t *)ev.u.data;

	req->kind = CAM_CFGDEV_REQ_FBD_NOTIFY;
	(void) cam_cfgdev_send_req(&ev, &s_cfgdev.rq);
}

static ssize_t fbd_notify_store(struct device_driver *drv,
				const char *buf, size_t count)
{
	(void)drv;
	cam_cfg_info("%s enter", __func__);
	s_fbd = simple_strtol(buf, NULL, 10);
	(void)cam_cfgdev_fbd_notify();
	return count;
}

static ssize_t fbd_notify_show(struct device_driver *drv, char *buf)
{
	int err;
	(void)drv;
	if (!buf) {
		cam_err("%s input buf is null", __func__);
		return 0;
	}
	cam_cfg_info("%s enter", __func__);
	err = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
			"fbd notify:[%d] \n", s_fbd);
	if (err != EOK) {
		cam_warn("%s snprintf_s fail", __func__);
		return 0;
	}
	return err;
}
#endif

static int cam_cfgdev_thermal_guard(struct v4l2_event *ev)
{
	if (ev) {
		errno_t err = memcpy_s(ev->u.data, sizeof(ev->u.data),
				s_cfgdev.sbuf, sizeof(s_cfgdev.sbuf));
		if (err != EOK)
			cam_warn("%s memcpy_s fail", __func__);
	}
	return 0;
}

static int cam_cfgdev_motion_data(struct v4l2_event *ev)
{
	if (ev) {
		errno_t err = memcpy_s(ev->u.data, sizeof(ev->u.data),
				s_motion_data.motiondata, sizeof(s_motion_data.motiondata));
		if (err != EOK)
			cam_warn("%s memcpy_s fail", __func__);
	}
	return 0;
}

static int cam_cfgdev_send_req(struct v4l2_event *ev, struct v4l2_fh *target)
{
	int rc = 0;

	if (!ev)
		return -ENOENT;

	mutex_lock(&s_cfgdev_lock);
	if (target->vdev) {
		v4l2_event_queue_fh(target, ev);
	} else {
		cam_cfg_err("the target is invalid");
		rc = -ENOENT;
	}
	mutex_unlock(&s_cfgdev_lock);

	return rc;
}

char *gen_media_prefix(char *media_ent, cam_device_id_constants_t dev_const,
		       size_t dst_size)
{
	int rc;

	if (media_ent && dst_size >= 1) {
		rc = snprintf_s(media_ent, dst_size, dst_size - 1,
				"%d", dev_const);
		if (rc < 0)
			cam_cfg_err("snprintf_s media_ent failed");
		strlcat(media_ent, "-", dst_size);
	}

	return media_ent;
}

int init_subdev_media_entity(struct v4l2_subdev *subdev,
			     cam_device_id_constants_t dev_const)
{
	int rc;

	(void)dev_const;
	if (!subdev) {
		cam_err("%s input subdev is null", __func__);
		return -ENOENT;
	}
	rc = media_entity_pads_init(&subdev->entity, 0, NULL);
	subdev->entity.obj_type = MEDIA_ENTITY_TYPE_V4L2_SUBDEV;
	return rc;
}

static void cam_cfgdev_subdev_release(struct video_device *vdev)
{
	struct v4l2_subdev *sd = video_get_drvdata(vdev);

	if (!sd)
		return;
	sd->devnode = NULL;
	video_device_release(vdev);

	cam_cfg_info("TODO");
}

static unsigned int cam_subdev_poll(struct file *file, poll_table *wait)
{
	struct video_device *vdev = NULL;
	struct v4l2_subdev *sd = NULL;
	struct v4l2_fh *fh = NULL;

	if (!file)
		return POLLERR;

	vdev = video_devdata(file);
	if (!vdev)
		return POLLERR;

	sd = vdev_to_v4l2_subdev(vdev);
	if (!sd)
		return POLLERR;

	fh = file->private_data;
	if (!fh || (sd->flags & V4L2_SUBDEV_FL_HAS_EVENTS) == 0)
		return POLLERR;

	poll_wait(file, &fh->wait, wait);

	if (v4l2_event_pending(fh) != 0)
		return POLLIN;

	return 0;
}

static struct v4l2_file_operations g_cam_v4l2_subdev_fops = {};

static void gen_vname_for_mprefix(struct video_device *vdev,
				  const char *media_prefix,
				  const char *cfgdev_name)
{
	int rc;

	rc = snprintf_s(vdev->name, sizeof(vdev->name),
			sizeof(vdev->name) - 1, "%s", media_prefix);
	if (rc < 0)
		cam_cfg_err("snprintf_s video device name failed");
	(void)strlcpy(vdev->name + strlen(vdev->name), cfgdev_name,
			sizeof(vdev->name) - strlen(vdev->name));
}

static void gen_vdentity_name_for_mprefix(struct video_device *vdev,
					  const char *media_prefix)
{
	int rc;

	rc = snprintf_s(vdev->name + strlen(vdev->name),
			sizeof(vdev->name) - strlen(vdev->name),
			sizeof(vdev->name) - strlen(vdev->name) - 1,
			"%s", video_device_node_name(vdev));
	if (rc < 0) {
		cam_cfg_err("Truncation Occurred");
		rc = snprintf_s(vdev->name, sizeof(vdev->name),
				sizeof(vdev->name) - 1, "%s", media_prefix);
		if (rc < 0)
			cam_cfg_err("cat media prefix failed");

		rc = snprintf_s(vdev->name + strlen(vdev->name),
				sizeof(vdev->name) - strlen(vdev->name),
				sizeof(vdev->name) - strlen(vdev->name) - 1,
				"%s", video_device_node_name(vdev));
		if (rc < 0)
			cam_cfg_err("cat dev node name failed");
	}
}

int cam_cfgdev_register_subdev(struct v4l2_subdev *sd,
			       cam_device_id_constants_t dev_const)
{
	int rc;
	char media_prefix[10]; /* 10,array len */
	struct video_device *vdev = NULL;
	struct v4l2_device *v4l2 = &s_cfgdev.v4l2;

	cam_cfg_info("enter %s[%d]", __func__, __LINE__);
	if (!sd || (sd->name[0] == 0)) {
		cam_cfg_info("enter %s[%d] !sd or !sd->name", __func__, __LINE__);
		rc = -EINVAL;
		goto out;
	}

	if (v4l2->mdev == NULL) {
		cam_cfg_info("enter %s[%d] v4l2->mdev == NULL", __func__, __LINE__);
		rc = -EINVAL;
		goto out;
	}

	cam_cfg_info("%s[%d] before call v4l2_device_register_subdev",
		__func__, __LINE__);
	rc = v4l2_device_register_subdev(v4l2, sd);
	if (rc < 0)
		goto out;

	if ((sd->flags & V4L2_SUBDEV_FL_HAS_DEVNODE) == 0)
		goto out;

	vdev = video_device_alloc();
	if (!vdev) {
		rc = -ENOMEM;
		goto video_alloc_fail;
	}

	video_set_drvdata(vdev, sd);
	gen_media_prefix(media_prefix, dev_const, sizeof(media_prefix));
	gen_vname_for_mprefix(vdev, media_prefix, sd->name);
	vdev->v4l2_dev = v4l2;
	vdev->fops = &g_cam_v4l2_subdev_fops;
	vdev->release = cam_cfgdev_subdev_release;
	rc = __video_register_device(vdev, VFL_TYPE_SUBDEV, -1, 1, sd->owner);
	if (rc < 0)
		goto video_register_fail;
	cam_debug("register video devices %s sucessful", sd->name);
	cam_debug("video dev name %s %s", vdev->dev.kobj.name, vdev->name);
	sd->entity.info.dev.major = VIDEO_MAJOR;
	sd->entity.info.dev.minor = (uint32_t)vdev->minor;

	gen_vdentity_name_for_mprefix(vdev, media_prefix);
	rc = 0;
	sd->entity.name = vdev->name;
	sd->devnode = vdev;
	goto out;

video_register_fail:
	video_device_release(vdev);

video_alloc_fail:
	if (sd->devnode)
		v4l2_device_unregister_subdev(sd);

out:
	return rc;
}

int cam_cfgdev_unregister_subdev(struct v4l2_subdev *sd)
{
	if (!sd)
		return -EINVAL;
	v4l2_device_unregister_subdev(sd);
	return 0;
}

static unsigned int cam_cfgdev_vo_poll(struct file *filep,
				       struct poll_table_struct *ptbl)
{
	unsigned int rc = 0;

	if (!filep) {
		cam_cfg_err("%s: filep is NULL", __func__);
		return POLLERR;
	}
	if (!ptbl) {
		cam_cfg_err("%s: ptbl is NULL", __func__);
		return POLLERR;
	}

	mutex_lock(&s_cfgdev.lock);
	poll_wait(filep, &s_cfgdev.rq.wait, ptbl);
	if (v4l2_event_pending(&s_cfgdev.rq) != 0)
		rc = POLLIN | POLLRDNORM;
	mutex_unlock(&s_cfgdev.lock);
	return rc;
}

static long cam_cfgdev_vo_dqevent(struct v4l2_event *ev, int nb)
{
	if (!ev)
		return -ENOENT;
	return v4l2_event_dequeue(&s_cfgdev.rq, ev, nb);
}

static long cam_cfgdev_vo_do_ioctl(struct file *filep, unsigned int cmd, void *arg)
{
	long rc = -EINVAL;

	if (!filep || !arg)
		return rc;

	switch (cmd) {
	case VIDIOC_DQEVENT:
		rc = cam_cfgdev_vo_dqevent(arg, filep->f_flags & O_NONBLOCK);
		break;

	case VIDIOC_SUBSCRIBE_EVENT:
		rc = v4l2_event_subscribe(&s_cfgdev.rq,
				(struct v4l2_event_subscription *)arg,
				8, /* 8, arg len */
				NULL);
		break;

	case VIDIOC_UNSUBSCRIBE_EVENT:
		rc = v4l2_event_unsubscribe(&s_cfgdev.rq,
				(struct v4l2_event_subscription *)arg);
		break;

	case CAM_V4L2_IOCTL_THERMAL_GUARD:
		rc = cam_cfgdev_thermal_guard((struct v4l2_event *)arg);
		break;

	case CAM_V4L2_IOCTL_MOTION_DATA:
		rc = cam_cfgdev_motion_data((struct v4l2_event *)arg);
		break;

	default:
		cam_cfg_err("invalid IOCTL CMD %d", cmd);
		break;
	}
	return rc;
}

static long cam_cfgdev_vo_ioctl(struct file *filep,
				unsigned int cmd,
				unsigned long arg)
{
	return video_usercopy(filep, cmd, arg, cam_cfgdev_vo_do_ioctl);
}

static int cam_cfgdev_vo_close(struct file *filep)
{
	void *fpd = NULL;

	mutex_lock(&s_cfgdev.lock);
	if (filep)
		swap(filep->private_data, fpd);
	mutex_unlock(&s_cfgdev.lock);
	if (fpd) {
		struct pid *pid = NULL;

		mutex_lock(&s_cfgsvr_lock);
		swap(s_pid_cfgsvr, pid);

		mutex_lock(&s_cfgdev_lock);
		v4l2_fh_del(&s_cfgdev.rq);
		v4l2_fh_exit(&s_cfgdev.rq);
		mutex_unlock(&s_cfgdev_lock);

		if (pid)
			put_pid(pid);

		mutex_unlock(&s_cfgsvr_lock);

		cam_cfg_info("the server %d detached", current->pid);
	}
	return 0;
}

static int cam_cfgdev_vo_open(struct file *filep)
{
	int rc = 0;

	mutex_lock(&s_cfgsvr_lock);
	if (s_pid_cfgsvr) {
		mutex_unlock(&s_cfgsvr_lock);
		cam_cfg_info("only one server can attach to cfgdev");
		rc = -EBUSY;
		goto exit_open;
	}
	s_pid_cfgsvr = get_pid(task_pid(current));

	mutex_lock(&s_cfgdev_lock);
	v4l2_fh_init(&s_cfgdev.rq, s_cfgdev.vdev);
	v4l2_fh_add(&s_cfgdev.rq);
	mutex_unlock(&s_cfgdev_lock);
	mutex_unlock(&s_cfgsvr_lock);

	if (filep)
		filep->private_data = &s_cfgdev;

	cam_cfg_info("the server %d attached", current->pid);

exit_open:
	return rc;
}

static struct v4l2_file_operations s_fops_cfgdev = {
	.owner = THIS_MODULE,
	.open = cam_cfgdev_vo_open,
	.poll = cam_cfgdev_vo_poll,
	.unlocked_ioctl = cam_cfgdev_vo_ioctl,
	.release = cam_cfgdev_vo_close,
};

static void cam_cfgdev_vo_subdev_notify(
	struct v4l2_subdev *sd,
	unsigned int notification,
	void *arg)
{
	cam_cfg_info("TODO");
	(void)sd;
	(void)notification;
	(void)arg;
}

static int create_media_device(struct platform_device *pdev,
			       struct media_device **pmdev)
{
	int rc;
	struct media_device *mdev = NULL;

	mdev = kzalloc(sizeof(struct media_device), GFP_KERNEL);
	if (!mdev)
		return -ENOMEM;

	media_device_init(mdev);
	(void)strlcpy(mdev->model, CAM_MODEL_CFG, sizeof(mdev->model));
	if (pdev)
		mdev->dev = &(pdev->dev);

	rc = media_device_register(mdev);
	if (rc < 0) {
		cam_cfg_err("%s %d media_device_register failed", __func__, __LINE__);
		kfree_sensitive(mdev);
		return rc;
	}
	cam_cfg_info("%s %d media_device_register succ = %lx", __func__, __LINE__, mdev);
	*pmdev = mdev;

	return 0;
}

static void destroy_media_device(struct media_device *mdev)
{
	media_device_unregister(mdev);
	kfree_sensitive(mdev);
}

static int video_device_init(struct video_device *vdev,
			     struct v4l2_device *v4l2)
{
	int rc;
	char media_prefix[10]; /* 10, array len */

	vdev->v4l2_dev = v4l2;
	(void)gen_media_prefix(media_prefix, CAM_VNODE_GROUP_ID,
			sizeof(media_prefix));

	gen_vname_for_mprefix(vdev, media_prefix, "cam-cfgdev");
	vdev->entity.obj_type = MEDIA_ENTITY_TYPE_VIDEO_DEVICE;
	vdev->release = video_device_release_empty;
	vdev->fops = &s_fops_cfgdev;
	vdev->minor = -1;
	vdev->device_caps = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_VIDEO_CAPTURE_MPLANE;
	vdev->vfl_type = VFL_TYPE_VIDEO;
	rc = video_register_device(vdev, VFL_TYPE_VIDEO, -1);
	if (rc < 0)
		return rc;
	cam_cfg_info("video dev name %s %s", vdev->dev.kobj.name, vdev->name);

	gen_vdentity_name_for_mprefix(vdev, media_prefix);
	vdev->entity.name = vdev->name;
	vdev->lock = &s_cfgdev_lock;

	return 0;
}

static int cam_cfgdev_vo_probe(struct platform_device *pdev)
{
	int rc;

	struct video_device *vdev = NULL;
	struct media_device *mdev = NULL;
	struct v4l2_device *v4l2 = &s_cfgdev.v4l2;

	vdev = video_device_alloc();
	if (!vdev) {
		rc = -ENOMEM;
		cam_cfg_info("%s[%d] ", __func__, __LINE__);
		goto probe_end;
	}

	rc = create_media_device(pdev, &mdev);
	if (rc != 0) {
		cam_cfg_info("%s[%d] ", __func__, __LINE__);
		goto media_alloc_fail;
	}

	rc = media_entity_pads_init(&vdev->entity, 0, NULL);
	if (rc < 0) {
		cam_cfg_info("%s[%d] ", __func__, __LINE__);
		goto entity_init_fail;
	}

	v4l2->mdev = mdev;
	v4l2->notify = cam_cfgdev_vo_subdev_notify;
	rc = v4l2_device_register(&(pdev->dev), v4l2);
	if (rc < 0) {
		cam_cfg_info("%s[%d] ", __func__, __LINE__);
		goto v4l2_register_fail;
	}

	rc = video_device_init(vdev, v4l2);
	if (rc < 0) {
		cam_cfg_info("%s[%d] ", __func__, __LINE__);
		goto video_register_fail;
	}

	video_set_drvdata(vdev, &s_cfgdev);

	s_cfgdev.vdev = vdev;
	s_cfgdev.mdev = mdev;

	s_cfgdev.debug_root = debugfs_create_dir("hwcam", NULL);
	mutex_init(&s_cfgdev.lock);

	if (!client_camera_user)
		client_camera_user = dsm_register_client(&g_dev_camera_user);

	goto probe_end;

video_register_fail:
	cam_cfg_info("%s[%d] ", __func__, __LINE__);
	v4l2_device_unregister(v4l2);

v4l2_register_fail:
	cam_cfg_info("%s[%d] ", __func__, __LINE__);
	media_entity_cleanup(&vdev->entity);

entity_init_fail:
	cam_cfg_info("%s[%d] ", __func__, __LINE__);
	destroy_media_device(mdev);

media_alloc_fail:
	cam_cfg_info("%s[%d] ", __func__, __LINE__);
	video_device_release(vdev);
	v4l2->mdev = NULL;
	s_cfgdev.mdev = NULL;

probe_end:
	cam_cfg_info("exit");
	return rc;
}

static int cam_cfgdev_vo_remove(struct platform_device *pdev)
{
	(void)pdev;
	video_unregister_device(s_cfgdev.vdev);
	v4l2_device_unregister(&s_cfgdev.v4l2);
	media_entity_cleanup(&(s_cfgdev.vdev)->entity);
	media_device_unregister(s_cfgdev.mdev);
	kfree_sensitive(s_cfgdev.mdev);
	video_device_release(s_cfgdev.vdev);
	s_cfgdev.vdev = NULL;
	s_cfgdev.mdev = NULL;
	mutex_destroy(&s_cfgdev.lock);
	return 0;
}

static const struct of_device_id s_cfgdev_devtbl_match[] = {
	{ .compatible = "huawei,camcfgdev" },
	{},
}
MODULE_DEVICE_TABLE(of, s_cfgdev_devtbl_match);

static struct platform_driver s_cfgdev_driver = {
	.probe = cam_cfgdev_vo_probe,
	.remove = cam_cfgdev_vo_remove,
	.driver = {
		.name = "huawei,camcfgdev",
		.owner = THIS_MODULE,
		.of_match_table = s_cfgdev_devtbl_match,
	},
};

static int __init cam_cfgdev_vo_init(void)
{
	int ret;

	g_cam_v4l2_subdev_fops = v4l2_subdev_fops;
	g_cam_v4l2_subdev_fops.poll = cam_subdev_poll;
	ret = platform_driver_register(&s_cfgdev_driver);
	if (ret == 0) {
		if (driver_create_file(&s_cfgdev_driver.driver,
					&driver_attr_guard_thermal) != 0)
			cam_warn("%s create driver attr failed", __func__);

		if (driver_create_file(&s_cfgdev_driver.driver,
					&driver_attr_dump_meminfo) != 0)
			cam_warn("%s create driver attr dump_meminfo failed",
					__func__);

		if (driver_create_file(&s_cfgdev_driver.driver,
					&driver_attr_get_motion_data) != 0)
			cam_warn("%s create driver attr get_motion_data failed",
					__func__);
#ifdef DEBUG_KERNEL_CAMERA
		if (driver_create_file(&s_cfgdev_driver.driver,
					&driver_attr_cam_log) != 0)
			cam_warn("%s create driver attr cam_log failed",
					__func__);

		if (driver_create_file(&s_cfgdev_driver.driver,
					&driver_attr_fbd_notify) != 0)
			cam_warn("%s create driver attr fbd_notify failed",
					__func__);
#endif
	}
	return ret;
}

static void __exit cam_cfgdev_vo_exit(void)
{
	platform_driver_unregister(&s_cfgdev_driver);
}

module_init(cam_cfgdev_vo_init);
module_exit(cam_cfgdev_vo_exit);
MODULE_DESCRIPTION("native V4L2 Camera");
MODULE_LICENSE("GPL v2");
