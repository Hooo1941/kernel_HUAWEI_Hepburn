/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:platform_firmware_oneimage_define.h header file
 * Author: @CompanyNameTag
 */

#ifndef PLATFORM_FIRMWARE_ONEIMAGE_DEFINE_H
#define PLATFORM_FIRMWARE_ONEIMAGE_DEFINE_H

/* 1 其他头文件包含 */
/* 2 宏定义 */
#if !defined(WIN32) && !defined(_PRE_WINDOWS_SUPPORT)
#define firmware_oneimage_rename(NAME)  NAME##_mp

#define firmware_cfg_clear   firmware_oneimage_rename(firmware_cfg_clear)
#define firmware_cfg_init    firmware_oneimage_rename(firmware_cfg_init)
#define firmware_download_cmd    firmware_oneimage_rename(firmware_download_cmd)
#define firmware_download_function         firmware_oneimage_rename(firmware_download_function)
#define firmware_download_function_priv    firmware_oneimage_rename(firmware_download_function_priv)
#define firmware_get_cfg      firmware_oneimage_rename(firmware_get_cfg)
#define firmware_parse_cmd    firmware_oneimage_rename(firmware_parse_cmd)
#define firmware_read_cfg     firmware_oneimage_rename(firmware_read_cfg)
#define parse_file_cmd        firmware_oneimage_rename(parse_file_cmd)
#define send_msg    firmware_oneimage_rename(send_msg)
#define read_msg    firmware_oneimage_rename(read_msg)
#define read_device_reg16    firmware_oneimage_rename(read_device_reg16)
#define write_device_reg16    firmware_oneimage_rename(write_device_reg16)
#define recv_expect_result    firmware_oneimage_rename(recv_expect_result)
#define recv_expect_result_timeout    firmware_oneimage_rename(recv_expect_result_timeout)
#define msg_send_and_recv_except    firmware_oneimage_rename(msg_send_and_recv_except)

#endif

#endif
