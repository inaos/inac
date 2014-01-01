/*
 * Copyright (c) 2012-2013, INAOS GmbH
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the INAOS GmbH nor the names of its contributors
 *       may be used to endorse or promote products derived from this software 
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL INAOS GmbH BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */
#include <libinac/lib.h>
#include "config.h"

#ifdef INA_ISTRING_ENABLED

#define __INA_HDR_OFFSET(s) (ina_str_hdr_t*)(s-(sizeof(ina_str_hdr_t)))
#define __INA_STR_OFFSET(h) (char*)(h+(sizeof(ina_str_hdr_t)))

typedef struct ina_str_hdr_s {
    size_t  size;
    size_t  len;
    uint8_t pooled;
    char data[];
} __attribute__ ((__packed__)) ina_str_hdr_t;

static ina_str_hdr_t* __ina_ensure_size(ina_str_hdr_t*, size_t);

INA_API(ina_str_t) ina_str_create(size_t len)
{
    ina_str_hdr_t *hdr;
    hdr = (ina_str_hdr_t*)INA_MEM_MALLOC(len + 1 + sizeof(ina_str_hdr_t));
    if (hdr == NULL) {
        INA_STR_EALLOC;
    }
    hdr->size = len+1;
    hdr->len = 0;
    hdr->pooled = INA_NO;
    hdr->data[0] = '\0';
    return (char*)hdr->data; 
}

INA_API(ina_str_t) ina_str_create_using_pool(size_t len, ina_mempool_t *pool)
{
    ina_str_hdr_t *hdr;

    if (pool == NULL) {
        hdr = (ina_str_hdr_t*)ina_mem_alloc(len+1+sizeof(ina_str_hdr_t));
        if (hdr == NULL) {
            INA_STR_EALLOC;
            return NULL;
        }
    } else {
        hdr = (ina_str_hdr_t*)ina_mempool_dalloc(pool,len+1+sizeof(ina_str_hdr_t));
        if (hdr == NULL) {
            INA_STR_EALLOC;
            return NULL;
        }
    }
    hdr->size = len+1;
    hdr->pooled = INA_YES;
    return (char*)hdr->data; 
}

INA_API(ina_str_t) ina_str_fromblk(const void* blk, size_t len) 
{
    ina_str_hdr_t *hdr;
    ina_str_t str;

    if (blk == NULL) {
        INA_STR_EALLOC;
        return NULL;
    }

    str = ina_str_create(len);
    if (str == NULL)  {
        INA_ERR_PUSH_LAST;
        return NULL;
    }
    hdr = __INA_HDR_OFFSET(str);
    if (len > 0) {
        ina_mem_cpy(str, blk, len);
    }
    hdr->len = len;
    str[len]='\0';
    return str;
}

INA_API(ina_str_t) ina_str_fromblk_using_pool(const void* blk, 
                                              size_t len, 
                                              ina_mempool_t *pool)
{
    ina_str_t str;

    INA_ASSERT_NOTNULL(pool);
    
    if (blk == NULL) {
        INA_STR_EALLOC;
        return NULL;
    }

    str = ina_str_create_using_pool(len, pool);
    if (str == NULL)  {
        INA_ERR_PUSH_LAST;
        return NULL;
    }
    if (len > 0) {
        ina_mem_cpy(str, blk, len);
    }
    (__INA_HDR_OFFSET(str))->len = len;
    return str;
}

INA_API(ina_str_t) ina_str_fromcstr(const char* cstr)
{
    ina_str_t str;
    size_t len;

    if (cstr != NULL) {
        len = strlen(cstr);
    } else {
        len = 0;
    }
    str = ina_str_create(len);
    if (str == NULL) {
        INA_ERR_PUSH_LAST;
        return NULL;
    }
    ina_mem_cpy(str, cstr, len);
    (__INA_HDR_OFFSET(str))->len = len;
    str[len]='\0';
    return str;
}

INA_API(ina_str_t) ina_str_fromcstr_using_pool(const char* cstr, 
                                               ina_mempool_t *pool)
{
    ina_str_t str;
    size_t len;

    str = NULL;
    
    if (cstr != NULL) {
        len = strlen(cstr);
    } else {
        len = 0;
    }
    str = ina_str_create_using_pool(len, pool);
    if (str == NULL) {
        INA_ERR_PUSH_LAST;
        return NULL;
    }
    ina_mem_cpy(str, cstr, len);
    (__INA_HDR_OFFSET(str))->len = len;
    return str;
}

INA_API(ina_rc_t) ina_str_destroy(ina_str_t str)
{
    if (str != NULL) {
        ina_str_hdr_t *hdr = __INA_HDR_OFFSET(str);
        if (hdr->pooled) {
            INA_MEM_FREE(hdr);
        }
    }
    return INA_SUCCESS;
}

INA_API(ina_str_t) ina_str_dup(const ina_str_t str)
{
    if (str == NULL) {
        return NULL;
    }
    return ina_str_fromcstr(str);
}

INA_API(ina_str_t) ina_str_dup_using_pool(const ina_str_t str, 
                                          ina_mempool_t *pool)
{
    if (str == NULL) {
        return NULL;
    }
    return ina_str_fromcstr_using_pool(str, pool);
}

INA_API(const char*) ina_str_cstr(const ina_str_t str)
{
    return str;
}

INA_API(ina_str_t) ina_str_cpy(ina_str_t dest, const ina_str_t src)
{
    return ina_str_ncpy(dest, src, ina_str_len(src));
}

INA_API(ina_str_t) ina_str_ncpy(ina_str_t dest, const ina_str_t src, size_t n)
{
    ina_str_hdr_t *d;
    ina_str_hdr_t *s;

    INA_ASSERT_NOTNULL(dest);

    if (src == NULL) {
        return dest;
    }

    d = __INA_HDR_OFFSET(dest);
    s = __INA_HDR_OFFSET(src);
    d = __ina_ensure_size(d, s->len);
    memcpy(d->data, src, n);
    return d->data;
}

INA_API(ina_str_t) ina_str_cat(ina_str_t dest, const ina_str_t src)
{
    return ina_str_ncat(dest, src, ina_str_len(src));
}

INA_API(ina_str_t) ina_str_catcstr(ina_str_t dest, const char *src)
{
    return ina_str_ncatcstr(dest, src, strlen(src));
}

INA_API(ina_str_t) ina_str_ncat(ina_str_t dest, const ina_str_t src, size_t n)
{
    ina_str_hdr_t *d;

    INA_ASSERT_NOTNULL(dest);

    if (src == NULL) {
        return dest;
    }

    d = __INA_HDR_OFFSET(dest);
    d = __ina_ensure_size(d, d->len+n);
    memcpy(&d->data[d->len], src, n);
    d->len += n;
    d->data[d->len] = '\0';
    return d->data;
}

INA_API(ina_str_t) ina_str_ncatcstr(ina_str_t dest, const char *src, size_t n)
{
    ina_str_hdr_t *d;

    INA_ASSERT_NOTNULL(dest);
    INA_ASSERT_FALSE(strlen(src) < n);

    if (src == NULL) {
        return dest;
    }

    d = __INA_HDR_OFFSET(dest);

    d = __ina_ensure_size(d, d->len+n);
    memcpy(&d->data[d->len], src, n);
    d->len += n;
    d->data[d->len] = '\0';
    return d->data;
}


INA_API(ina_rc_t) ina_str_cmp(const ina_str_t lhs, const ina_str_t rhs)
{
    return strcmp(lhs, rhs);
}

INA_API(ina_rc_t) ina_str_casecmp(const ina_str_t lhs, const ina_str_t rhs)
{
    return strcasecmp(lhs, rhs);
}

INA_API(ina_rc_t) ina_str_ncmp(const ina_str_t lhs, const ina_str_t rhs, size_t n)
{
    return strncmp(lhs, rhs, n);
}

INA_API(ina_str_t) ina_str_rchr(const ina_str_t str, const char chr)
{
    const char* s;
    if (chr == 0) {
        return NULL;
    }
    s = strrchr(str, chr);
    if (s) {
        return ina_str_fromcstr(s);
    }
    return NULL; 
}

INA_API(ina_str_t) ina_str_str(const ina_str_t str1, const ina_str_t str2)
{
    const char* str;
    
    if (str2 == NULL) {
        return NULL;
    }
    if (ina_str_len(str2) == 0) {
        return NULL;
    }
    str = strstr(str1, str2);
    if (str) {
        return ina_str_fromcstr(str);
    }
    return NULL;
}

INA_API(ina_str_t) ina_str_strcstr(const ina_str_t str1, const char *str2)
{
    const char* str;

    if (str2 == NULL) {
        return NULL;
    }
    if (strlen(str2) == 0) {
        return NULL;
    }
    str = strstr(str1, str2);
    if (str) {
        return ina_str_fromcstr(str);
    }
    return NULL;
}

INA_API(size_t) ina_str_len(const ina_str_t str)
{
    if (str == NULL) {
        return 0;
    }
    return (__INA_HDR_OFFSET(str))->len;
}

INA_API(size_t) ina_str_size(const ina_str_t str)
{
    if (str == NULL) {
        return 0;
    }
    return (__INA_HDR_OFFSET(str))->size;
}

INA_API(ina_str_t) ina_str_toupper(ina_str_t str)
{
    if (str) {
        ina_str_t s = str;
        do {
            if (96 == (224 & *s)) {
                *s ^= 32;
            }
        }  while (*s++);
    }
    return str;
}

INA_API(ina_str_t) ina_str_tolower(ina_str_t str)
{
    if (str) {
        ina_str_t s = str;
        do {
            if (64 == (224 & *s)) {
                *s ^= 32;
            }
        }  while (*s++);       
    }
    return str;
}

INA_API(ina_str_t) ina_str_sprintf(const char *fmt, ...)
{
    va_list args;
    ina_str_t str;
    size_t size;
    int n;

    INA_ASSERT_NOTNULL(fmt);

    size = 128;
    str = ina_str_create(size);
    if (str == NULL) {
        INA_ERR_PUSH_LAST;
        return NULL;
    }

    va_start(args, fmt);
    n = ina_str_vsnprintf(&str, size, fmt, args);
    va_end(args);
    if (n <= 0) {
        ina_str_destroy(str);
        return NULL;
    }
    return str;
}

INA_API(int) ina_str_snprintf(ina_str_t *str, size_t len, const char* fmt, ...)
{
    va_list args;
    int retval = 0;

    INA_ASSERT_NOTNULL(fmt);
    INA_ASSERT_NOTNULL(str);
    INA_ASSERT_TRUE(len > 0);

    va_start(args, fmt);
    retval = ina_str_vsnprintf(str, len, fmt, args);
    va_end(args);
    return retval;
}

INA_API(int) ina_str_vsnprintf(ina_str_t *str, size_t len, const char* fmt,  
                               va_list args)
{
    int l;
    va_list args_copy;

    INA_ASSERT_NOTNULL(fmt);
    INA_ASSERT_NOTNULL(str);
    INA_ASSERT_TRUE(len > 0);
    INA_ASSERT_FALSE((__INA_HDR_OFFSET(str))->size < len);
 
    va_copy(args_copy, args);
    if ((l = vsnprintf(*str, len, fmt, args)) >= len) {
        ina_str_t extra_str;
        if ((extra_str = ina_str_create(l))) {
            l = vsnprintf(extra_str, l+1, fmt, args_copy);
            ina_str_destroy(*str);
            *str = extra_str;
        } else {
            INA_ERR_PUSH_LAST;
            l = 1;
        }
    }
    va_end(args_copy);
    return l;    
}

static ina_str_hdr_t* 
__ina_ensure_size(ina_str_hdr_t *hdr, size_t len)
{
    INA_ASSERT_NOTNULL(hdr);
    INA_ASSERT_TRUE(len > 0);
    if ((hdr->size-hdr->len-1) > len) {
        return hdr;
    }
    hdr->size = (hdr->size-hdr->len)+len;
    hdr = INA_MEM_REALLOC(hdr, hdr->size);
    hdr->pooled = INA_NO;
    return hdr;
}

#endif
