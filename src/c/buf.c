/**
 * Copyright (c) 2015, Chao Wang (hit9 <hit9@icloud.com>)
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "buf.h"

/**
 * New buf.
 */
buf_t *
buf_new(size_t unit)
{
    buf_t *buf = malloc(sizeof(buf_t));

    if (buf != NULL) {
        buf->data = NULL;
        buf->size = 0;
        buf->cap = 0;
        buf->unit = unit;
    }

    return buf;
}

/**
 * Free buf.
 */
void
buf_free(buf_t *buf)
{
    if (buf != NULL) {
        if (buf->data != NULL)
            free(buf->data);
        free(buf);
    }
}


/**
 * Free buf data.
 */
void
buf_clear(buf_t *buf)
{
    assert(buf != NULL);

    if (buf->data != NULL)
        free(buf->data);
    buf->data = NULL;
    buf->size = 0;
    buf->cap = 0;
}

/**
 * Increase buf allocated size to `size`, O(1), O(n)
 */
int
buf_grow(buf_t *buf, size_t size)
{
    assert(buf != NULL && buf->unit != 0);

    if (size > BUF_MAX_SIZE)
        return BUF_ENOMEM;

    if (size <= buf->cap)
        return BUF_OK;

    size_t cap = buf->cap + buf->unit;

    while (cap < size)
        cap += buf->unit;

    uint8_t *data = realloc(buf->data, cap);

    if (data == NULL)
        return BUF_ENOMEM;

    buf->data = data;
    buf->cap = cap;
    return BUF_OK;
}

/**
 * Get data as c string (terminate with '\0'), O(1), O(n)
 */
char *
buf_str(buf_t *buf)
{
    assert(buf && buf->unit);

    if (buf->size < buf->cap && buf->data[buf->size] == '\0')
        return (char *)buf->data;

    if (buf->size + 1 <= buf->cap ||
            buf_grow(buf, buf->size + 1) == BUF_OK) {
        buf->data[buf->size] = '\0';
        return (char *)buf->data;
    }

    return NULL;
}

/**
 * Put a char to buf, O(1), O(n)
 */
int
buf_putc(buf_t *buf, char ch)
{
    int res = buf_grow(buf, buf->size + 1);

    if (res != BUF_OK)
        return res;

    buf->data[buf->size] = ch;
    buf->size += 1;
    return BUF_OK;
}

/**
 * Print buf to stdout
 */
void
buf_print(buf_t *buf)
{
    printf("%.*s", (int)buf->size, buf->data);
}

/**
 * Print buf to stdout (with '\n')
 */
void
buf_println(buf_t *buf)
{
    printf("%.*s\n", (int)buf->size, buf->data);
}

/**
 * Put data to buf, O(n)
 */
int
buf_put(buf_t *buf, uint8_t *data, size_t size)
{
    int result = buf_grow(buf, buf->size + size);

    if (result == BUF_OK) {
        memcpy(buf->data + buf->size, data, size);
        buf->size += size;
    }

    return result;
}

/**
 * Put string to buf, O(n)
 */
int
buf_puts(buf_t *buf, char *str)
{
    return buf_put(buf, (uint8_t *)str, strlen(str));
}


/**
 * Remove left data from buf by number of bytes, O(n)
 */
size_t
buf_lrm(buf_t *buf, size_t size)
{
    assert(buf != NULL && buf->unit != 0);

    if (size > buf->size) {
        size_t size_ = buf->size;
        buf->size = 0;
        return size_;
    }

    buf->size -= size;
    memmove(buf->data, buf->data + size, buf->size);
    return size;
}


/**
 * Remove right data from buf by number of bytes, O(1)
 */
size_t
buf_rrm(buf_t *buf, size_t size)
{
    assert(buf != NULL && buf->unit != 0);

    if (size > buf->size) {
        size_t size_ = buf->size;
        buf->size = 0;
        return size_;
    }
    buf->size -= size;
    return size;
}

/**
 * Formatted printing to a buffer.
 */
int
buf_sprintf(buf_t *buf, const char *fmt, ...)
{
    assert(buf != NULL && buf->unit != 0);

    if (buf->size >= buf->cap &&
            buf_grow(buf, buf->size + 1) != BUF_OK)
        return BUF_ENOMEM;

    va_list ap;
    int num;

    va_start(ap, fmt);
    num = vsnprintf((char *)buf->data + buf->size,
            buf->cap - buf->size, fmt, ap);
    va_end(ap);

    if (num < 0)
        return BUF_EFAILED;

    size_t size = (size_t)num;

    if (size >= buf->cap - buf->size) {
        if (buf_grow(buf, buf->size + size + 1) != BUF_OK)
            return BUF_ENOMEM;
        va_start(ap, fmt);
        num = vsnprintf((char *)buf->data + buf->size,
                buf->cap - buf->size, fmt, ap);
        va_end(ap);
    }

    if (num < 0)
        return BUF_EFAILED;

    buf->size += num;
    return BUF_OK;
}

/**
 * Compare buf with string. O(n)
 */
int
buf_cmp(buf_t *buf, char *s)
{
    return strcmp(buf_str(buf), s);
}

/**
 * Test if buf eqauals with string. O(n)
 */
bool
buf_equals(buf_t *buf, char *s)
{
    if (buf_cmp(buf, s) == 0)
        return true;
    return false;
}

/**
 * Test if a buf is space. O(n)
 */
bool
buf_isspace(buf_t *buf)
{
    assert(buf != NULL);

    size_t idx;

    for (idx = 0; idx < buf->size; idx++)
        if (!isspace(buf->data[idx]))
            return false;
    if (buf->size > 0)
        return true;
    return false;
}

/**
 * Test if a buf is startswith a prefix. O(k)
 */
bool
buf_startswith(buf_t *buf, char *prefix)
{
    assert(buf != NULL);

    size_t idx = 0;

    while (idx < buf->size && prefix[idx] != '\0') {
        if (buf->data[idx] != (uint8_t)prefix[idx])
            return false;
        idx++;
    }
    return true;
}

/**
 * Test if a buf is startswith a prefix. O(k)
 */
bool
buf_endswith(buf_t *buf, char *suffix)
{
    assert(buf != NULL);

    size_t len = 0;
    size_t idx = 0;

    while (suffix[len] != '\0') {len++;}

    while (idx < buf->size && idx < len) {
        // buf->size >= 1 was already gauranteed
        if (buf->data[buf->size - 1 - idx] != (uint8_t)suffix[len - 1 - idx])
            return false;
        idx++;
    }
    return true;
}

/**
 * Reverse buf in place. O(n/2)
 */
void
buf_reverse(buf_t *buf)
{
    assert(buf != NULL);

    if (buf->size == 0)
        return;

    uint8_t tmp;
    size_t idx = 0;
    // buf->size >= 1 was already gauranteed
    size_t end = buf->size - 1;

    while (idx < end) {
        // swap
        tmp = buf->data[idx];
        buf->data[idx] = buf->data[end];
        buf->data[end] = tmp;
        // move
        idx ++;
        end --;
    }
}

/**
 * Search string in buf by Boyer-Moore algorithm.
 */
size_t
buf_index(buf_t *buf, char *sub, size_t start)
{
    assert(buf != NULL);

    size_t len = strlen(sub);
    size_t last = len - 1;
    size_t idx;

    size_t table[MAX_UINT8] = {0};

    // build bad char table
    for (idx = 0; idx < MAX_UINT8; idx++)
        table[idx] = len;
    for (idx = 0; idx < len; idx++)
        table[(uint8_t)sub[idx]] = last - idx;

    // search
    size_t i, j, k, t, skip;

    for (i = start; i < buf->size; i += skip) {
        skip = 0;
        for (j = 0; j < len; j++) {
            k = last - j;
            if ((uint8_t)sub[k] != buf->data[i + k]) {
                t = table[buf->data[i + k]];
                skip = t > j? t - j : 1;
                break;
            }
        }
        if (skip == 0)
            return i;
    }

    return buf->size;
}
