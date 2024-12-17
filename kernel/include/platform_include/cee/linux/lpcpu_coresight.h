/*
 * lpcpu_coresight.h
 *
 * lpcpu coresight head file
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef _LPCPU_CORESIGHT_H
#define _LPCPU_CORESIGHT_H

#include <linux/device.h>
#include <linux/io.h>
#include <linux/perf_event.h>
#include <linux/sched.h>
#include <linux/version.h>

#define SNID_MASK               (BIT(7) | BIT(6)) /* Secure non-invasive debug */
#define SNID_DEBUG_ENABLE       (BIT(7) | BIT(6))
#define SNID_DEBUG_DISABLE      BIT(7)

#define SID_MASK                (BIT(5) | BIT(4)) /* Secure invasive debug */
#define SID_DEBUG_ENABLE        (BIT(5) | BIT(4))
#define SID_DEBUG_DISABLE       BIT(5)

#define NSNID_MASK              (BIT(3) | BIT(2)) /* Non-secure non-invasive debug */
#define NSNID_DEBUG_ENABLE      (BIT(3) | BIT(2))
#define NSNID_DEBUG_DISABLE     BIT(3)

#define NSID_MASK (BIT(1) | BIT(0)) /* Non-secure invasive debug */
#define NSID_DEBUG_ENABLE (BIT(1) | BIT(0))
#define NSID_DEBUG_DISABLE BIT(1)

#define err_print(fmt, ...)  	\
	printk(KERN_ERR "%s: "fmt, __func__, ##__VA_ARGS__)
#define info_print(fmt, ...)  	\
	printk(KERN_INFO "%s: "fmt, __func__, ##__VA_ARGS__)

#if IS_ENABLED(CONFIG_CORESIGHT)
extern unsigned int coresight_access_enabled(void);
extern void coresight_refresh_path(struct coresight_device *csdev, int enable);
extern int check_cpu_online(struct coresight_device *csdev);
extern void etm4_disable_all(void);
extern void noc_trace_disable_all(void);
extern void qic_trace_disable_all(void);
extern void stm_trace_disable_all(void);
extern void coresight_disable_all(void);
extern void *get_etb_drvdata_bydevnode(struct device_node *np);
extern int etbetf_restore(void *drv);
extern void *get_funnel_drvdata_bydevnode(struct device_node *np);
extern int funnel_restore(void *drv);
int get_etm_cpu(struct coresight_device *csdev);
#else
static inline unsigned int coresight_access_enabled(void)
{
	return 0;
}
static inline void etm4_disable_all(void)
{
	return;
}
static inline void noc_trace_disable_all(void)
{
	return;
}
static inline void qic_trace_disable_all(void)
{
	return;
}
static inline void stm_trace_disable_all(void)
{
	return;
}
static inline void coresight_disable_all(void)
{
	return;
}
static inline void *get_etb_drvdata_bydevnode(struct device_node *np)
{
	return NULL;
}

static inline int etbetf_restore(void *drv)
{
	return -1;
}

static inline void *get_funnel_drvdata_bydevnode(struct device_node *np)
{
	return NULL;
}

static inline int funnel_restore(void *drv)
{
	return -1;
}

static inline int get_etm_cpu(struct coresight_device *csdev)
{
	return 0;
}

#endif /* IS_ENABLED(CONFIG_CORESIGHT) */

#ifdef CONFIG_OF
extern struct coresight_platform_data * of_get_coresight_platform_data(struct device *dev,
						struct device_node *node);
extern struct device_node *of_get_coresight_etb_data(
				struct device *dev, struct device_node *node);
extern struct device_node *of_get_coresight_funnel_data(
				struct device *dev, struct device_node *node);
#else
static inline struct coresight_platform_data *of_get_coresight_platform_data(
	struct device *dev, struct device_node *node)
{
	return NULL;
}

static inline struct device_node *of_get_coresight_etb_data(
	struct device *dev, struct device_node *node)
{
	return NULL;
}
static inline struct device_node *of_get_coresight_funnel_data(
	struct device *dev, struct device_node *node)
{
	return NULL;
}
#endif /* CONFIG_OF */
#endif /* _LPCPU_CORESIGHT_H */
