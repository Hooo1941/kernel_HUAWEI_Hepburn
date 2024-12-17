/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/zlib.h>
#include <linux/syscalls.h>
#include <linux/file.h>
#include <linux/uaccess.h>

#include <securec.h>

#include "mloader_err.h"
#include "mloader_debug.h"

int mloader_get_file_size(const char *file_path)
{
    int ret;
    struct file *fp = NULL;

    fp = filp_open(file_path, O_RDONLY, 0);
    if (!IS_ERR_OR_NULL(fp)) {
        ret = i_size_read(file_inode(fp));
        if (ret <= 0) {
            mloader_print_err("fail to get '%s' size, ret = %d\n", file_path, ret);
        }
        filp_close(fp, NULL);
    } else {
        ret = (int)PTR_ERR(fp);
    }
    return ret;
}

int bsp_mloader_get_file_size(const char *file_path)
{
    return mloader_get_file_size(file_path);
}

int bsp_mloader_read_file(const char *file_path, unsigned int offset, unsigned int length, char *buffer)
{
    int ret;
    ssize_t read_size;
    struct file *fp = NULL;
    loff_t offset_adp = (loff_t)offset;
    fp = filp_open(file_path, O_RDONLY, 0);
    if (IS_ERR_OR_NULL(fp)) {
        ret = (int)PTR_ERR(fp);
        mloader_print_err("fail to open %s, ret = %d\n", file_path, ret);
        return ret;
    }

    read_size = kernel_read(fp, buffer, length, &offset_adp);
    if (read_size != length) {
        mloader_print_err("fail to read %s, expect len = %u, ret = %zd\n", file_path, length, read_size);
    }
    ret = (int)read_size;
    filp_close(fp, NULL);
    return ret;
}

char *mloader_gzip_get_blob_addr_and_size(void *src_buf, u32 src_len, u32 *size)
{
#define GZIP_HEAD_LEN 10 /* rfc 1952, gzip header */
#define GZIP_TAIL_LEN 8  /* rfc 1952, crc & isize */
    u8 *in_buf = (u8 *)src_buf;
    int in_len = src_len;
    u8 gzip_head_flag;
    if (src_len < (GZIP_HEAD_LEN + GZIP_TAIL_LEN)) {
        mloader_print_err("gzip length %d too small\n", src_len);
        return NULL;
    }
    gzip_head_flag = in_buf[3]; /* 3: rfc 1952, gzip flag */
    /*
     * skip over gzip header (1f,8b,08... 10 bytes total + possible asciz filename)
     */
    in_buf = in_buf + GZIP_HEAD_LEN;
    in_len = in_len - GZIP_HEAD_LEN - GZIP_TAIL_LEN;
    if (gzip_head_flag & 0x8) { /* 8: rfc 1952, bit 3, FNAME */
        while ((*in_buf++) && (in_len--)) {
        };
    }
    if (size != NULL) {
        *size = in_len;
    }
    return (char *)in_buf;
}

int bsp_mloader_decompress(void *dest_buf, u32 dest_len, void *src_buf, u32 src_len)
{
    int ret;
    u32 blob_size = 0;
    char *in_buf = NULL;

    in_buf = mloader_gzip_get_blob_addr_and_size(src_buf, src_len, &blob_size);
    if (in_buf == NULL) {
        mloader_print_err("fail to get gzip blob len\n");
        return -1;
    }

    ret = zlib_inflate_blob(dest_buf, dest_len, (void *)in_buf, blob_size);
    if (ret < 0) {
        mloader_print_err("fail to inflate, ret = %d\n", ret);
    } else {
        mloader_print_err("succeed to inflate, ret = %d\n", ret);
    }
    ret = ret >= 0 ? 0 : ret;

    return ret;
}
EXPORT_SYMBOL_GPL(bsp_mloader_decompress);