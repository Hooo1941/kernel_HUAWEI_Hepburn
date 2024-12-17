

#ifndef __OAL_PLAT_OP_H__
#define __OAL_PLAT_OP_H__

#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include "securec.h"

#ifdef BIULD_FIRMWARE_TO_ARRAY
typedef int oal_loff_t;
typedef struct {
	int file;
	oal_loff_t f_pos;
	int len;
} oal_file_t;

#include "ca01_fw_file_array.h"

#else
typedef struct file oal_file_t;
typedef loff_t oal_loff_t;
#endif

#define PLAT_LOG_NONE 0
#define PLAT_LOG_ALERT 1
#define PLAT_LOG_ERR 2
#define PLAT_LOG_WARNING 3
#define PLAT_LOG_INFO 4
#define PLAT_LOG_DEBUG 5

#ifndef isupper
#define isupper(c) ((c) >= 'A' && (c) <= 'Z')
#define islower(c) ((c) >= 'a' && (c) <= 'z')


static inline char tolower(char c)
{
	if (isupper(c))
		c -= 'A' - 'a';
	return c;
}

static inline char toupper(char c)
{
	if (islower(c))
		c -= 'a' - 'A';
	return c;
}
#endif

static inline void str_to_hex(char *dest, char *src, int len)
{
	char h1, h2;
	char s1, s2;
	int i;

	for (i = 0; i < len; i++) {
		h1 = src[0x2 * i];
		h2 = src[0x2 * i + 1];

		s1 = toupper(h1) - 0x30;
		if (s1 > 0x9)
			s1 -= 0x7;

		s2 = toupper(h2) - 0x30;
		if (s2 > 0x9)
			s2 -= 0x7;

		dest[i] = s1 * 0x10 + s2;
	}
}

#ifndef PLAT_NFC_LOGLEVEL
#define PLAT_NFC_LOGLEVEL PLAT_LOG_INFO
#endif

#if (PLAT_NFC_LOGLEVEL >= PLAT_LOG_DEBUG)
#define PS_PRINT_FUNCTION_NAME  printk(KERN_INFO KBUILD_MODNAME ":D]%s]", __func__)
#else
#define PS_PRINT_FUNCTION_NAME
#endif

#if (PLAT_NFC_LOGLEVEL >= PLAT_LOG_DEBUG)
#define PS_PRINT_DBG(s, args...)  printk(KERN_INFO KBUILD_MODNAME ":D]%s]" s, __func__, ##args)
#else
#define PS_PRINT_DBG(s, args...)
#endif

#if (PLAT_NFC_LOGLEVEL >= PLAT_LOG_INFO)
#define PS_PRINT_INFO(s, args...)  printk(KERN_INFO KBUILD_MODNAME ":I]%s]" s, __func__, ##args)
#else
#define PS_PRINT_INFO(s, args...)
#endif

#if (PLAT_NFC_LOGLEVEL >= PLAT_LOG_INFO)
#define PS_PRINT_SUC(s, args...)  printk(KERN_INFO KBUILD_MODNAME ":S]%s]" s, __func__, ##args)
#else
#define PS_PRINT_SUC(s, args...)
#endif

#if (PLAT_NFC_LOGLEVEL >= PLAT_LOG_WARNING)
#define PS_PRINT_WARNING(s, args...)  printk(KERN_WARNING KBUILD_MODNAME ":W]%s]" s, __func__, ##args)
#else
#define PS_PRINT_WARNING(s, args...)
#endif

#if (PLAT_NFC_LOGLEVEL >= PLAT_LOG_ERR)
#define PS_PRINT_ERR(s, args...)  printk(KERN_ERR KBUILD_MODNAME ":E]%s]" s, __func__, ##args)
#else
#define PS_PRINT_ERR(s, args...)
#endif

#if (PLAT_NFC_LOGLEVEL >= PLAT_LOG_ALERT)
#define PS_PRINT_ALERT(s, args...)  printk(KERN_ALERT KBUILD_MODNAME ":ALERT]%s]" s, __func__, ##args)
#else
#define PS_PRINT_ALERT(s, args...)
#endif

/*
 * 函 数 名  : oal_file_open_rw
 * 功能描述  : 打开文件，方式rw，没有则创建
 * 输入参数  : path: 文件路径
 */
static inline oal_file_t *oal_file_open_rw(const char *path)
{
#ifdef BIULD_FIRMWARE_TO_ARRAY
	unused(path);
	return NULL;
#else
	mm_segment_t fs;
	oal_file_t *file = NULL;

	fs = get_fs();
	set_fs(KERNEL_DS);
	file = filp_open(path, (O_CREAT | O_RDWR), 0);
	set_fs(fs);
	if (IS_ERR(file))
		return NULL;

	return file;
#endif
}

/*
 * 函 数 名  : oal_file_open_readonly
 * 功能描述  : 打开文件，方式只读
 * 输入参数  : path: 文件路径
 */
static inline oal_file_t *oal_file_open_readonly(const char *path)
{
#ifdef BIULD_FIRMWARE_TO_ARRAY
	oal_file_t *f = (oal_file_t *)oal_malloc(sizeof(oal_file_t));
	if (f == NULL)
		return NULL;

	const fw_file_array *file = ca01_file_array_open(path);
	if (file == NULL) {
		oal_free(f);
		return NULL;
	}
	f->f_pos = 0;
	f->file = (int)file->data;
	f->len = file->len;
	return f;
#else
	mm_segment_t fs;
	oal_file_t *file = NULL;

	fs = get_fs();
	set_fs(KERNEL_DS);
	file = filp_open(path, (O_RDONLY), 0);
	set_fs(fs);
	if (IS_ERR(file))
		return NULL;

	return file;
#endif
}

/*
 * 函 数 名  : oal_file_write
 * 功能描述  : 写文件
 * 输入参数  : file: 文件句柄
 *           : string: 输入内容地址
 *           : length: 输入内容长度
 */
static inline oal_file_t *oal_file_write(oal_file_t *file, const char *string, uint32_t length)
{
#ifdef BIULD_FIRMWARE_TO_ARRAY
	unused(file);
	unused(string);
	unused(length);
	return NULL;
#else
	int ret;
	mm_segment_t fs;

	fs = get_fs();
	set_fs(KERNEL_DS);
	ret = file->f_op->write(file, string, length, &file->f_pos);
	set_fs(fs);

	return file;
#endif
}

/*
 * 函 数 名  : oal_file_read
 * 功能描述  : 内核读文件，从头开始读
 * 输入参数  : file:指向要读取的文件的指针
 *             buf:从文件读出数据后存放的buf
 *             count:指定要读取的长度
 */
static inline int32_t oal_file_read(oal_file_t *file, char *buf, uint32_t count)
{
#ifdef BIULD_FIRMWARE_TO_ARRAY
	char *data = (char *)(file->file + file->f_pos);
	uint32_t cpy_count = file->len - file->f_pos;

	if (cpy_count > count)
		cpy_count = count;

	if (memcpy_s(buf, count, data, cpy_count) != EOK)
		return 0;

	return cpy_count;
#else
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
	loff_t pos = 0;
	return kernel_read(file, buf, count, &pos);
#else
	return kernel_read(file, 0, buf, count);
#endif
#endif
}

static inline int oal_file_read_ext(oal_file_t *file, oal_loff_t pos, char *buf, uint32_t count)
{
#ifdef BIULD_FIRMWARE_TO_ARRAY
	char *data = (char *)(file->file + file->f_pos);
	uint32_t cpy_count = file->len - file->f_pos;
	unused(pos);

	if (cpy_count > count)
		cpy_count = count;

	if (memcpy_s(buf, count, data, cpy_count) != EOK)
		return 0;

	return cpy_count;
#else
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
	return kernel_read(file, buf, count, &pos);
#else
	return kernel_read(file, pos, buf, count);
#endif
#endif
}

static inline int32_t oal_file_close(oal_file_t *file)
{
#ifdef BIULD_FIRMWARE_TO_ARRAY
	oal_free(file);
	return 0;
#else
	int ret;
	mm_segment_t fs;

	fs = get_fs();
	set_fs(KERNEL_DS);
	ret = filp_close(file, 0);
	set_fs(fs);
	if (ret == 0) {
		return 0;
	} else {
		return -ENOENT;
	}
#endif
}

static inline int oal_file_open_get_len(const char *path, oal_file_t **fp, unsigned int *file_len)
{
#ifdef BIULD_FIRMWARE_TO_ARRAY
	oal_file_t *f = (oal_file_t *)oal_malloc(sizeof(oal_file_t));
	if (f == NULL)
		return -1;

	const fw_file_array *file = ca01_file_array_open(path);
	if (file == NULL) {
		oal_free(f);
		return -1;
	}
	f->f_pos = 0;
	f->file = (int)file->data;
	f->len = file->len;
	*fp = f;
	*file_len = (unsigned int)f->len;

	return 0;
#else
#define RETRY_CONT 3
	mm_segment_t fs;
	int i;

	fs = get_fs();
	set_fs(KERNEL_DS);
	for (i = 0; i < RETRY_CONT; i++) {
		*fp = filp_open(path, O_RDONLY, 0);
		if (*fp == NULL) {
			continue;
		}
	}

	if (IS_ERR_OR_NULL(*fp)) {
		set_fs(fs);
		return -ENOENT;
	}

	/* 获取file文件大小 */
	*file_len = (unsigned int)vfs_llseek(*fp, 0, SEEK_END);
	if (*file_len <= 0) {
		filp_close(*fp, NULL);
		set_fs(fs);
		return -1;
	}

	/* 恢复fp->f_pos到文件开头 */
	vfs_llseek(*fp, 0, SEEK_SET);
	set_fs(fs);

	return 0;
#endif
}

static inline unsigned long oal_strtoul(const char *str, char **endptr, unsigned int base)
{
	return (unsigned long)simple_strtoul(str, endptr, base);
}


#endif

