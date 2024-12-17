#ifndef _LINUX_HWCXEXTLOG_H
#define _LINUX_HWCXEXTLOG_H

#include <linux/printk.h>
#include <linux/types.h>

enum {
	HWLOG_ERR	= 1U << 0,
	HWLOG_WARNING	= 1U << 1,
	HWLOG_INFO	= 1U << 2,
	HWLOG_DEBUG	= 1U << 3,
};

int audio_log_printk(const char *fmt, ...);
int audio_log_init(void);
int audio_log_exit(void);

#ifndef pr_fmt
#define pr_fmt(fmt) fmt
#endif

#define prr_err(fmt, ...)  audio_log_printk(pr_fmt(fmt), ##__VA_ARGS__)
#define prr_warn(fmt, ...) audio_log_printk(pr_fmt(fmt), ##__VA_ARGS__)
#define prr_info(fmt, ...) audio_log_printk(pr_fmt(fmt), ##__VA_ARGS__)

#define HWLOG_TAG_DEFALUT_LEVEL (HWLOG_ERR | HWLOG_WARNING | HWLOG_INFO | HWLOG_DEBUG)

struct huawei_log_tag {
	const char *name;
	int level;
};

#define TAG_STRUCT_NAME(name) _hwtag_##name

#define hw_fmt_tag(TAG, LEVEL) "(" #LEVEL "/" #TAG ") "

#define HWLOG_REGIST()							\
	HWLOG_REGIST_TAG_LEVEL(HWLOG_TAG, HWLOG_TAG_DEFALUT_LEVEL)

#define HWLOG_REGIST_LEVEL(level)					\
	HWLOG_REGIST_TAG_LEVEL(HWLOG_TAG, level)

#define HWLOG_REGIST_TAG_LEVEL(name, level)				\
	_HWLOG_REGIST_TAG_LEVEL(name, level)

#define _HWLOG_REGIST_TAG_LEVEL(name, level)				\
	static struct huawei_log_tag TAG_STRUCT_NAME(name)		\
	__used								\
	__attribute__((unused, __section__("__hwlog_tag")))		\
	= { #name, level }

#define __hwlog_err(TAG, fmt, ...)					\
	do {								\
		if (TAG_STRUCT_NAME(TAG).level & HWLOG_ERR)		\
			prr_err(hw_fmt_tag(TAG, E) fmt, ##__VA_ARGS__);	\
			printk(KERN_ERR hw_fmt_tag(TAG, E) fmt, ##__VA_ARGS__);	\
	} while (0)

#define __hwlog_warn(TAG, fmt, ...)					\
	do {								\
		if (TAG_STRUCT_NAME(TAG).level & HWLOG_WARNING)		\
			prr_warn(hw_fmt_tag(TAG, W) fmt, ##__VA_ARGS__);	\
			printk(KERN_ERR hw_fmt_tag(TAG, W) fmt, ##__VA_ARGS__);	\
	} while (0)

#define __hwlog_info(TAG, fmt, ...)					\
	do {								\
		if (TAG_STRUCT_NAME(TAG).level & HWLOG_INFO)		\
			prr_info(hw_fmt_tag(TAG, I) fmt, ##__VA_ARGS__);	\
			printk(KERN_ERR hw_fmt_tag(TAG, I) fmt, ##__VA_ARGS__);	\
	} while (0)

#define __hwlog_debug(TAG, fmt, ...)					\
	do {								\
		if (TAG_STRUCT_NAME(TAG).level & HWLOG_DEBUG)		\
			prr_err(hw_fmt_tag(TAG, D) fmt, ##__VA_ARGS__);	\
			printk(KERN_ERR hw_fmt_tag(TAG, D) fmt, ##__VA_ARGS__);	\
	} while (0)

#define _hwlog_err(TAG, x...)    __hwlog_err(TAG, ##x)
#define _hwlog_warn(TAG, x...)   __hwlog_warn(TAG, ##x)
#define _hwlog_info(TAG, x...)   __hwlog_info(TAG, ##x)
#define _hwlog_debug(TAG, x...)  __hwlog_debug(TAG, ##x)

#define hwlog_err(x...)          _hwlog_err(HWLOG_TAG, ##x)
#define hwlog_warn(x...)         _hwlog_warn(HWLOG_TAG, ##x)
#define hwlog_info(x...)         _hwlog_info(HWLOG_TAG, ##x)
#define hwlog_debug(x...)        _hwlog_debug(HWLOG_TAG, ##x)

#endif
