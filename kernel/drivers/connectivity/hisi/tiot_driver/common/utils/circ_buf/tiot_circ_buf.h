/*
 * Copyright (c) CompanyNameMagicTag. 2019-2024. All rights reserved.
 * Description: audio circle buffer function
 * Author: audio
 * Create: 2019-05-30
 */

#ifndef TIOT_CIRC_BUF_H
#define TIOT_CIRC_BUF_H

#include "securec.h"
#include "tiot_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

typedef struct {
    volatile uint32_t *write; /* circle buffer write pointer */
    volatile uint32_t *read;  /* circle buffer read pointer */
    uint8_t *data;            /* circle buffer data virtual address */
    uint32_t size;            /* circle buffer size */
} tiot_circ_buf;

static inline uint32_t circ_buf_min(uint32_t x, uint32_t y)
{
    return (x <= y) ? x : y;
}

static inline uint32_t saturate_add(uint32_t x, uint32_t y, uint32_t saturation)
{
    /* add x and y, saturate to saturation */
    uint32_t x_temp = x + y;

    return (x_temp >= saturation) ? (x_temp - saturation) : x_temp;
}

static inline void circ_buf_init(tiot_circ_buf *cb, uint32_t *write, uint32_t *read, void *data, uint32_t len)
{
    cb->write = write;
    cb->read = read;
    cb->data = data;
    cb->size = len;

    *cb->write = 0;
    *cb->read = 0;
}

static inline void circ_buf_flush(tiot_circ_buf *cb)
{
    *cb->read = 0;
    *cb->write = 0;
}

static inline uint32_t circ_buf_data_size(uint32_t write, uint32_t read, uint32_t size)
{
    return (write >= read) ? (write - read) : (size - read + write);
}

static inline uint32_t circ_buf_free_size(uint32_t write, uint32_t read, uint32_t size)
{
    return size - circ_buf_data_size(write, read, size);
}

static inline uint32_t __circ_buf_read_data(const tiot_circ_buf *cb, uint8_t *to, uint32_t len, uint32_t off)
{
    if ((len == 0) || (memcpy_s(to, len * sizeof(uint8_t), cb->data + off, len) != EOK)) {
        return 0;
    }

    return len;
}

/* read data from circle buffer */
static inline uint32_t circ_buf_read_data(const tiot_circ_buf *cb, uint8_t *to, uint32_t len, uint32_t off)
{
    uint32_t l = circ_buf_min(len, cb->size - off);

    if (to == NULL) {
        /* dump data in circle buffer */
        return len;
    }

    return __circ_buf_read_data(cb, to, l, off) + __circ_buf_read_data(cb, to + l, len - l, 0);
}

static inline uint32_t __circ_buf_write_zero(tiot_circ_buf *cb, const uint8_t *from, uint32_t len, uint32_t off)
{
    if ((len == 0) || (memset_s(cb->data + off, len, 0, len) != EOK)) {
        return 0;
    }
    (void)from;
    return len;
}

static inline uint32_t __circ_buf_write_data(tiot_circ_buf *cb, const uint8_t *from, uint32_t len, uint32_t off)
{
    if ((len == 0) || (memcpy_s(cb->data + off, cb->size - off, from, len) != EOK)) {
        return 0;
    }

    return len;
}

/*
 * circ_buf_write_data - puts some data from kernel space into the circle buffer
 * @cb: address of the circle buffer to be used
 * @from: pointer to the data to be added
 * @len: the length of the data to be added
 * @off: data write offset in circle buffer
 *
 * Note: Check free space in circle buffer before calling this function
 */
static inline uint32_t circ_buf_write_data(tiot_circ_buf *cb, const uint8_t *from, uint32_t len, uint32_t off)
{
    uint32_t l = circ_buf_min(len, cb->size - off);

    /* generate mute data if input buffer is NULL */
    uint32_t (*action)(tiot_circ_buf *, const uint8_t *, uint32_t, uint32_t) =
        (from == NULL) ? __circ_buf_write_zero : __circ_buf_write_data;

    return action(cb, from, l, off) + action(cb, from + l, len - l, 0);
}

static inline uint32_t circ_buf_query_busy(const tiot_circ_buf *cb)
{
    return circ_buf_data_size(*cb->write, *cb->read, cb->size);
}

static inline uint32_t circ_buf_query_free(const tiot_circ_buf *cb)
{
    return circ_buf_free_size(*cb->write, *cb->read, cb->size);
}

/* read data from circle buffer */
static inline uint32_t circ_buf_read(tiot_circ_buf *cb, uint8_t *to, uint32_t len)
{
    uint32_t read = *cb->read;

    if ((len == 0) || (circ_buf_data_size(*cb->write, read, cb->size) < len) ||
        (circ_buf_read_data(cb, to, len, read) != len)) {
        return 0;
    }

    *cb->read = saturate_add(read, len, cb->size);
    return len;
}

/* write data into circle buffer */
static inline uint32_t circ_buf_write(tiot_circ_buf *cb, const uint8_t *from, uint32_t len)
{
    uint32_t write = *cb->write;

    if ((len == 0) || (circ_buf_write_data(cb, from, len, write) != len)) {
        return 0;
    }

    *cb->write = saturate_add(write, len, cb->size);
    return len;
}

static inline uint32_t circ_buf_update_read_pos(tiot_circ_buf *cb, uint32_t len)
{
    *cb->read = saturate_add(*cb->read, len, cb->size);
    return len;
}

static inline uint32_t circ_buf_update_write_pos(tiot_circ_buf *cb, uint32_t len)
{
    *cb->write = saturate_add(*cb->write, len, cb->size);
    return len;
}

static inline uint32_t circ_buf_cast_read(const tiot_circ_buf *cb, uint32_t *data_offset, uint32_t len)
{
    uint32_t read = *cb->read;

    if (circ_buf_data_size(*cb->write, read, cb->size) < len) {
        *data_offset = 0;
        return 0;
    }

    *data_offset = read;
    return len;
}

static inline uint32_t circ_buf_cast_relese(tiot_circ_buf *cb, uint32_t len)
{
    uint32_t read = *cb->read;
    uint32_t data_size = circ_buf_data_size(*cb->write, read, cb->size);
    uint32_t len_temp;

    len_temp = circ_buf_min(data_size, len);
    *cb->read = saturate_add(read, len_temp, cb->size);

    return len_temp;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif
