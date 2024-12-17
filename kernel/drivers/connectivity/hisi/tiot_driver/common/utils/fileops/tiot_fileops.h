/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description: Tiot file types. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-04-12, Create file. \n
 */
#ifndef TIOT_FILEOPS_H
#define TIOT_FILEOPS_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @defgroup tiot_common_utils_file_ops File Ops
 * @ingroup  tiot_common_utils
 * @{
 */

#ifdef CONFIG_FILE_BY_ARRAY
#define TIOT_FILE_NAME_MAX  16

/**
 * @brief  TIoT file struct for file array, read only.
 */
typedef struct {
    const char name[TIOT_FILE_NAME_MAX];
    int len;
    const char *data;
} tiot_file;

/**
 * @brief  TIoT file search path, set for search file array, read only.
 */
typedef struct {
    const tiot_file *file_list;   /* A list of file array, read only. */
    const unsigned int file_num;  /* File array num, read only. */
} tiot_file_path;
#else
typedef void *tiot_file;          /*!< Dummy type for filp when board has file system. */
typedef char tiot_file_path;      /*!< Dummy type for path when board has file system. */
#endif

#ifdef CONFIG_FILE_BY_ARRAY
#define tiot_file_close(filp)   /*!< No need to close for file array. */
#else
/**
 * @brief  TIoT file open, redirect to osal_klib_fclose.
 * @param  [in]  filp File handle.
 */
void tiot_file_close(tiot_file *filp);
#endif

/**
 * @brief  TIoT file open, for file system see @ref osal_klib_fopen, path is ignored.
 *                         for file array, search in file name in the tiot_file_path.
 * @param  [in]  path File array path, ignore for file system.
 * @param  [in]  file_name File name.
 * @param  [in]  flags File flags, like: OSAL_O_RDONLY.
 * @param  [in]  mode File rwx mode, like: 0644.
 * @return if OK return file handle, else return NULL.
 */
tiot_file *tiot_file_open(const tiot_file_path *path, const char *file_name, int flags, int mode);

/**
 * @brief  TIoT file read, for file system read to buf->buff.
 *                         for file array, only move the buff pointer to offset.
 * @param  [in]  filp   TIoT file handle.
 * @param  [in]  buf    TIoT file readbuf.
 * @param  [in]  size   Required read size.
 * @param  [in]  offset Read offset of file.
 * @return The length that has been read, could less than required size, or -1 if fail.
 */
int tiot_file_read(tiot_file *filp, char *buf, unsigned long size, int offset);

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif
