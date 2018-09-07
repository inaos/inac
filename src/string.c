/*
 * Copyright (c) 2013-2018 INAOS GmbH
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

#ifdef INA_OS_WIN32
INA_INLINE int __ina_vsnprintf(char *str, size_t size, const char *format, va_list args)
{
    int c;
    va_list args_copy;

    va_copy(args_copy, args);
    if ((c = _vscprintf(format, args)) >= (int)size) {
        return c;
    }
    return vsnprintf(str, size, format, args_copy);
}
#else
#define __ina_vsnprintf vsnprintf
#endif

#define __INA_HDR_OFFSET(s) (ina_str_hdr_t*)((s)-(sizeof(ina_str_hdr_t)))
#define __INA_STR_OFFSET(h) (char*)((h)+(sizeof(ina_str_hdr_t)))

INA_VS_BEGIN_PACK
typedef struct ina_str_hdr_s {
    size_t  size;
    size_t  len;
    uint8_t pooled;
    char data[];
} INA_PACKED ina_str_hdr_t;
INA_VS_END_PACK

static ina_str_hdr_t* __ina_ensure_size(ina_str_hdr_t*, size_t);
static ina_str_hdr_t* __ina_ensure_size_pool(ina_mempool_t *pool, ina_str_hdr_t*, size_t);

INA_API(ina_str_t) ina_str_new(size_t len)
{
    ina_str_hdr_t *hdr;
    hdr = (ina_str_hdr_t*)INA_MEM_MALLOC(len + 1 + sizeof(ina_str_hdr_t));
    if (hdr == NULL) {
        return NULL;
    }
    hdr->size = len+1;
    hdr->len = 0;
    hdr->pooled = INA_NO;
    hdr->data[0] = '\0';
    return (ina_str_t)hdr->data; 
}

INA_API(ina_str_t) ina_str_new_using_pool(size_t len, ina_mempool_t *pool)
{
    ina_str_hdr_t *hdr;

    hdr = (ina_str_hdr_t*)ina_mempool_dalloc(pool,len+1+sizeof(ina_str_hdr_t));
    if (hdr == NULL) {
        return NULL;
    }
    hdr->size = len+1;
    hdr->pooled = INA_YES;      
    return (ina_str_t)hdr->data; 
}

INA_API(ina_str_t) ina_str_new_fromblk(const void* blk, size_t len) 
{
    ina_str_hdr_t *hdr;
    ina_str_t str;

    if (blk == NULL) {
        INA_ERROR(INA_NN_ARGUMENT|INA_ERR_INVALID);
        return NULL;
    }

    str = ina_str_new(len);
    if (str == NULL)  {
        return NULL;
    }
    hdr = __INA_HDR_OFFSET(str);
    if (len > 0) {
        INA_MEM_MEMCPY(str, blk, len);
    }
    hdr->len = len;
    str[len]='\0';
    return str;
}

INA_API(ina_str_t) ina_str_new_fromblk_using_pool(const void* blk, 
                                                  size_t len, 
                                                  ina_mempool_t *pool)
{
    ina_str_t str;

    INA_ASSERT_NOTNULL(pool);
    
    if (blk == NULL) {
        INA_ERROR(INA_NN_ARGUMENT|INA_ERR_INVALID);
        return NULL;
    }

    str = ina_str_new_using_pool(len, pool);
    if (str == NULL)  {
        return NULL;
    }
    if (len > 0) {
        INA_MEM_MEMCPY(str, blk, len);
    }
    (__INA_HDR_OFFSET(str))->len = len;
    return str;
}

INA_API(ina_str_t) ina_str_new_fromcstr(const char* cstr)
{
    ina_str_t str;
    size_t len;

    if (cstr != NULL) {
        len = strlen(cstr);
    } else {
        len = 0;
    }
    str = ina_str_new(len);
    if (str == NULL) {
        return NULL;
    }
    INA_MEM_MEMCPY(str, cstr, len);
    (__INA_HDR_OFFSET(str))->len = len;
    str[len]='\0';
    return str;
}

INA_API(ina_str_t) ina_str_new_fromcstr_using_pool(const char* cstr, 
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
    str = ina_str_new_using_pool(len, pool);
    if (str == NULL) {
        return NULL;
    }
    INA_MEM_MEMCPY(str, cstr, len);
    (__INA_HDR_OFFSET(str))->len = len;
    return str;
}

INA_API(ina_rc_t) ina_str_free(ina_str_t str)
{
    if (str != NULL) {
        ina_str_hdr_t *hdr = __INA_HDR_OFFSET(str);
        if (!hdr->pooled) {
            INA_MEM_FREE(hdr);
        }
    }
    return INA_SUCCESS;
}

INA_API(ina_str_t) ina_str_dup(ina_cstr_t str)
{
    if (str == NULL) {
        return NULL;
    }
    return ina_str_new_fromcstr(str);
}

INA_API(ina_str_t) ina_str_dup_using_pool(ina_cstr_t str,
                                          ina_mempool_t *pool)
{
    if (str == NULL) {
        return NULL;
    }
    return ina_str_new_fromcstr_using_pool(str, pool);
}

INA_API(const char*) ina_str_cstr(ina_cstr_t str)
{
    return str;
}

INA_API(ina_str_t) ina_str_cpy(ina_str_t dest, ina_cstr_t src)
{
    return ina_str_ncpy(dest, src, ina_str_len(src));
}

INA_API(ina_str_t) ina_str_ncpy(ina_str_t dest,  ina_cstr_t src, size_t n)
{
    ina_str_hdr_t *d;

    INA_ASSERT_NOTNULL(dest);

    if (src == NULL) {
        return dest;
    }

    d = __INA_HDR_OFFSET(dest);
    d = __ina_ensure_size(d, n);
    INA_MEM_MEMCPY(d->data, src, n);
    d->data[n] = 0;
    d->len = n;
    return d->data;
}

INA_API(ina_str_t) ina_str_cat(ina_str_t dest, ina_cstr_t src)
{
    return ina_str_ncat(dest, src, ina_str_len(src));
}

INA_API(ina_str_t) ina_str_cat_using_pool(ina_str_t dest,  ina_cstr_t src, ina_mempool_t *pool)
{
    return ina_str_ncat_using_pool(dest, src, ina_str_len(src), pool);
}

INA_API(ina_str_t) ina_str_catcstr(ina_str_t dest, const char *src)
{
    return ina_str_ncatcstr(dest, src, strlen(src));
}

INA_API(ina_str_t) ina_str_catcstr_using_pool(ina_str_t dest, const char *src, ina_mempool_t *pool)
{
    return ina_str_ncatcstr_using_pool(dest, src, strlen(src), pool);
}

INA_API(ina_str_t) ina_str_ncat(ina_str_t dest, ina_cstr_t src, size_t n)
{
    ina_str_hdr_t *d;

    INA_ASSERT_NOTNULL(dest);

    if (src == NULL) {
        return dest;
    }

    d = __INA_HDR_OFFSET(dest);
    d = __ina_ensure_size(d, d->len+n);
    INA_MEM_MEMCPY(&d->data[d->len], src, n);
    d->len += n;
    d->data[d->len] = '\0';
    return (ina_str_t)d->data;
}

INA_API(ina_str_t) ina_str_ncat_using_pool(ina_str_t dest, ina_cstr_t src, size_t n, ina_mempool_t *pool)
{
    ina_str_hdr_t *d;

    INA_ASSERT_NOTNULL(dest);

    if (src == NULL) {
        return dest;
    }

    d = __INA_HDR_OFFSET(dest);
    d = __ina_ensure_size_pool(pool, d, d->len+n);
    INA_MEM_MEMCPY(&d->data[d->len], src, n);
    d->len += n;
    d->data[d->len] = '\0';
    return (ina_str_t)d->data;
}

INA_API(ina_str_t) ina_str_ncatcstr(ina_str_t dest, const char *src, size_t n)
{
    ina_str_hdr_t *d;

    INA_ASSERT_NOTNULL(dest);
    INA_ASSERT_TRUE(strlen(src) <= n);

    if (src == NULL) {
        return dest;
    }

    d = __INA_HDR_OFFSET(dest);
    d = __ina_ensure_size(d, d->len+n);
    INA_MEM_MEMCPY(&d->data[d->len], src, n);
    d->len += n;
    d->data[d->len] = '\0';
    return (ina_str_t)d->data;
}

INA_API(ina_str_t) ina_str_ncatcstr_using_pool(ina_str_t dest, const char *src, size_t n, ina_mempool_t *pool)
{
    ina_str_hdr_t *d;

    INA_ASSERT_NOTNULL(dest);
    INA_ASSERT_TRUE(strlen(src) <= n);

    if (src == NULL) {
        return dest;
    }

    d = __INA_HDR_OFFSET(dest);
    d = __ina_ensure_size_pool(pool, d, d->len+n);
    INA_MEM_MEMCPY(&d->data[d->len], src, n);
    d->len += n;
    d->data[d->len] = '\0';
    return (ina_str_t)d->data;
}

INA_API(int) ina_str_cmp(ina_cstr_t lhs, ina_cstr_t rhs)
{
    size_t l1, l2, minlen;
    int cmp;

    l1 = ina_str_len(lhs);
    l2 = ina_str_len(rhs);
    minlen = INA_MIN(l1,l2);

    cmp = INA_MEM_MEMCMP(lhs, rhs, minlen);
    if (cmp == 0) {
		unsigned char c1 = (unsigned char)lhs[minlen];
		unsigned char c2 = (unsigned char)rhs[minlen];
		return c1 - c2;
    }
    return cmp;
}

INA_API(int) ina_str_casecmp(ina_cstr_t lhs, ina_cstr_t rhs)
{
    return INA_CSTR_CASECMP(lhs, rhs);
}

INA_API(int) ina_str_ncmp(ina_cstr_t lhs, ina_cstr_t rhs, size_t n)
{
    return strncmp(lhs, rhs, n);
}

INA_API(const char*) ina_str_rchr(ina_cstr_t str, const char chr)
{
    if (chr == 0) {
        return NULL;
    }
    return strrchr(str, chr);
}

INA_API(const char*) ina_str_str(ina_cstr_t str1,  ina_cstr_t str2)
{
    if (str2 == NULL || str1 == NULL) {
        return NULL;
    }
    if (ina_str_len(str2) == 0) {
        return NULL;
    }
    return strstr(str1, str2);
}

INA_API(const char*) ina_str_strcstr(ina_cstr_t str1, const char *str2)
{
    if (str2 == NULL || str1 == NULL) {
        return NULL;
    }
    if (strlen(str2) == 0) {
        return NULL;
    }
    return strstr(str1, str2);
}

INA_API(size_t) ina_str_len(ina_cstr_t str)
{
    if (str == NULL) {
        return 0;
    }
    return (__INA_HDR_OFFSET(str))->len;
}

INA_API(size_t) ina_str_size(ina_cstr_t str)
{
    if (str == NULL) {
        return 0;
    }
    return (__INA_HDR_OFFSET(str))->size;
}

INA_API(size_t) ina_str_available(ina_cstr_t str)
{
    if (str == NULL) {
        return 0;
    }
    return (__INA_HDR_OFFSET(str))->size -(__INA_HDR_OFFSET(str))->len-1;
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

INA_API(ina_str_t) ina_str_truncate(ina_str_t str, size_t pos)
{
    if (str != NULL) {
        INA_ASSERT_TRUE(pos <= (__INA_HDR_OFFSET(str))->len);
        (__INA_HDR_OFFSET(str))->len = pos;
        (__INA_HDR_OFFSET(str))->data[pos] = '\0';
    }
    return str;
}

INA_API(ina_str_t) ina_str_trim(ina_str_t str, const char* chars)
{
    INA_ASSERT_NOTNULL(str);
    if (chars != NULL) {
        ina_str_hdr_t *hdr = __INA_HDR_OFFSET(str);
        char *start, *end, *sp, *ep;
        size_t len;

        sp = start = str;
        ep = end = str+(hdr->len)-1;
        while(sp <= end && strchr(chars, *sp)) {
            sp++;
        }
        while(ep > start && strchr(chars, *ep)) {
            ep--;
        }
        if (sp > ep) {
            len = 0; 
        } else {
            len = ((ep-sp)+1);
        }
        if (hdr->data != sp) {
            INA_MEM_MEMMOVE(hdr->data, sp, len);
        }
        hdr->data[len] = '\0';
        hdr->len = len;
    }
    return str;   
}

static size_t __ina_str_substr_internal(ina_cstr_t str, size_t start, size_t end, size_t len)
{
    size_t newlen = 0;

    if (len == 0) {
        return 0;
    }

    if (start > end) {
        newlen = 0;
    } else {
        newlen = (end-start)+1;
    }
    if (newlen != 0) {
        if (start >= len) {
            newlen = 0;
        } else if (end >= len) {
            end = len-1;
            if (start > end) {
                newlen = 0;
            } else { 
                newlen = (end-start)+1;
            }
        }
    }
    return newlen;
}

INA_API(ina_str_t) ina_str_substr_using_pool(ina_cstr_t str, size_t start, size_t end, ina_mempool_t *pool)
{
    size_t newlen;
    INA_ASSERT_NOTNULL(str);

    newlen = __ina_str_substr_internal(str, start, end, ina_str_len(str));
    return ina_str_new_fromblk_using_pool((__INA_HDR_OFFSET(str))->data+start, newlen, pool);
}

INA_API(ina_str_t) ina_str_substr(ina_cstr_t str, size_t start, size_t end)
{
    size_t newlen;

    INA_ASSERT_NOTNULL(str);

    newlen = __ina_str_substr_internal(str, start, end, ina_str_len(str));
    return ina_str_new_fromblk((__INA_HDR_OFFSET(str))->data+start, newlen);
}

INA_API(ina_str_t*) ina_str_split(const char *str, const char *sep, size_t *count)
{
    size_t elements = 0, slots = 5, j, start = 0, seplen, len;
    ina_str_t *tokens;
    
    if (str == NULL) {
        if (count != NULL) {
            *count = 0;
        }
        return NULL;
    }

    if (sep == NULL) {
        if (count != NULL) {
            *count = 0;
        }
        return NULL;
    }

    len = strlen(str);
    seplen = strlen(sep);    
    if (len == 0 || seplen == 0) {
        if (count != NULL) {
            *count = 0;
        }
        return NULL;        
    }     

    tokens = INA_MEM_MALLOC(sizeof(ina_str_t)*slots);
    if (tokens == NULL) {
        return NULL;
    }

    for (j = 0; j < (len-1); j++) {
        /* make sure there is room for the next element and the final one
         * and the terminator */
        if (slots < elements+3) {
            ina_str_t *newtokens;

            slots *= 2;
            newtokens = INA_MEM_REALLOC(tokens, sizeof(ina_str_t)*slots);
            if (newtokens == NULL) goto cleanup;
            tokens = newtokens;
        }
        /* search the separator */
        /* FIXME: Optimize */
        if ((seplen == 1 && *(str+j) == sep[0]) || 
            (INA_MEM_MEMCMP(str+j,sep,seplen) == 0)) {
            tokens[elements] = ina_str_new_fromblk(str+start,j-start);
            if (tokens[elements] == NULL) {
                goto cleanup;
            }
            elements++;
            start = j+seplen;
            j = j+seplen-1; /* skip the separator */
        }
    }
    /* Add the final element. We are sure there is room in the tokens array. */
    tokens[elements] = ina_str_new_fromblk(str+start, len-start);
    if (tokens[elements] == NULL) {
        goto cleanup;
    }
    elements++;
    if (count != NULL) {
        *count = elements;
    }
    /* Add the terminator element. We are sure there is room in the array. */
    tokens[elements] = NULL; 
    return tokens;

cleanup:
    {
        size_t i;
        for (i = 0; i < elements; i++) ina_str_free(tokens[i]);
        INA_MEM_FREE(tokens);
        if (count != NULL) {
            *count = 0;
        }
        return NULL;
    }
}

INA_API(ina_rc_t)  ina_str_split_free_tokens(ina_str_t *tokens)
{
    if (tokens) {
        size_t i = 0;
        while(tokens[i]) {
            ina_str_free(tokens[i++]);
        }
        INA_MEM_FREE(tokens);
    }
    return INA_SUCCESS;
}

INA_API(const char*) ina_str_tok(char *str, const char *sep, char **next)
{
    char *ret = NULL;
    if (sep && strlen(sep)) {
        if (str) {
            *next = str;
        }
        if (*next != NULL) {
            unsigned long tokmap[1 << (CHAR_BIT - 5)] = {0};
            /* ^ a map of token dividers, containing 256 bits. */
            char * p;
            unsigned char tmp;
            while ((tmp = ((unsigned char) *sep++))) {
                tokmap[ (tmp & ~31) >> 5 ] |= 1u << (tmp & 31);
            }
            p = *next;
            /* We find the first character that is not a token
             * divider. */
            while (tokmap[(*p & ~31) >> 5 ] & (1u << (*p & 31))) {
                ++p;
            }
            /* It may be that there are no more tokens. */
            if (!*p) {
                *next = NULL;
            } else {
                /* But in this path, there are. */
                
                ret = p;
                /* Now we loop until we get a nontoken
                 * character. We want NULL to be a non-token
                 * character, so we first modify our map to
                 * take that in account.
                 */
                tokmap[0] |= 1;
                do {
                    ++p;
                } while (!(tokmap[(*p & ~31) >> 5] &
                        (1u << (*p & 31))));
                /* Now p points at a non-token character. */
                *p = 0;
                *next = p + 1;
            }
        }
    }
    return ret;
}

INA_API(ina_str_t) ina_str_adjust_len(ina_str_t str)
{
    INA_ASSERT_NOTNULL(str);
    (__INA_HDR_OFFSET(str))->len = strlen(str);
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
    str = ina_str_new(size);
    if (str == NULL) {
        return NULL;
    }

    va_start(args, fmt);
    n = ina_str_vsnprintf(&str, size, fmt, args);
    va_end(args);
    if (n <= 0) {
        ina_str_free(str);
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
    INA_ASSERT_FALSE((__INA_HDR_OFFSET(*str))->size < len);
 
    va_copy(args_copy, args);
    if ((l = __ina_vsnprintf(*str, len, fmt, args)) >= (int)len) {
        ina_str_t extra_str;
        if ((extra_str = ina_str_new(l))) {
            l = __ina_vsnprintf(extra_str, l+1, fmt, args_copy);
            ina_str_free(*str);
            *str = extra_str;
        } else {
            l = -1;
        }
    }
    if (l >= 0) {
        (__INA_HDR_OFFSET(*str))->len = (size_t)l;
    }
    va_end(args_copy);
    return l;    
}

INA_API(ina_rc_t) ina_str_wildcard_match(ina_cstr_t tame, const char *wildcard)
{
    /* From Dr.Dobbs -> By Kirk J. Krauss, October 07, 2014 */

    const char *pTameBookmark = (char*)0;
    const char *pWildBookmark = (char*)0;
    const char *pTameText = ina_str_cstr(tame);
    const char *pWildText = wildcard;
 
    /* Walk the text strings one character at a time. */
    while (1) {
        /* How do you match a unique text string? */
        if (*pWildText == '*') {
            // Easy: unique up on it!
            while (*(++pWildText) == '*') {
                /* "xy" matches "x**y" */
            }
 
            if (!*pWildText) {
                /* "x" matches "*" */
                return INA_SUCCESS;
            }
 
            if (*pWildText != '?') {
                /* Fast-forward to next possible match. */
                while (*pTameText != *pWildText) {
                    if (!(*(++pTameText))) {
                        /* "x" doesn't match "*y*" */
                        return INA_ERROR(INA_NN_TEXT|INA_ERR_NOT_MATCH);
                    }
                }
            }
 
            pWildBookmark = pWildText;
            pTameBookmark = pTameText;
        }
        else if (*pTameText != *pWildText && *pWildText != '?') {
            /* Got a non-match.  If we've set our bookmarks, back up to one 
               or both of them and retry.
            */
            if (pWildBookmark) {
                if (pWildText != pWildBookmark) {
                    pWildText = pWildBookmark;
                    if (*pTameText != *pWildText) {
                        /* Don't go this far back again. */
                        pTameText = ++pTameBookmark;
                        /* "xy" matches "*y" */
                        continue;
                    }
                    else {
                        pWildText++;
                    }
                }
 
                if (*pTameText) {
                    pTameText++;
                    /* "mississippi" matches "*sip*" */
                    continue;
                }
            }
            /* "xy" doesn't match "x" */
            return INA_ERROR(INA_NN_TEXT|INA_ERR_NOT_MATCH);
        }
 
        pTameText++;
        pWildText++;
 
        /* How do you match a tame text string? */
        if (!*pTameText) {
            /* The tame way: unique up on it! */
            while (*pWildText == '*') {
                /* "x" matches "x*" */
                pWildText++;
            }
 
            if (!*pWildText) {
                /* "x" matches "x" */
                return INA_SUCCESS;
            }
            /* "x" doesn't match "xy" */
            return INA_ERROR(INA_NN_TEXT|INA_ERR_NOT_MATCH);
        }
    }
}

static ina_str_hdr_t* 
__ina_ensure_size(ina_str_hdr_t *hdr, size_t len)
{
    INA_ASSERT_NOTNULL(hdr);
    if ((hdr->size-hdr->len-1) > len) {
        return hdr;
    }
    hdr->size = (hdr->size-hdr->len)+len;
    hdr = (ina_str_hdr_t*)INA_MEM_REALLOC(hdr, sizeof(ina_str_hdr_t) + hdr->size);
    INA_ASSERT_NOTNULL(hdr);
    hdr->pooled = INA_NO;
    return hdr;
}

static ina_str_hdr_t* 
__ina_ensure_size_pool(ina_mempool_t *pool, ina_str_hdr_t *hdr, size_t len)
{
    size_t old_size = hdr->size;

    INA_ASSERT_NOTNULL(hdr);
    INA_ASSERT_TRUE(hdr->pooled == INA_YES);

    if ((hdr->size-hdr->len-1) > len) {
        return hdr;
    }
    
    hdr->size = (hdr->size-hdr->len)+len;
    hdr = (ina_str_hdr_t*)ina_mempool_ralloc(pool, hdr, 
                            sizeof(ina_str_hdr_t) + old_size,
                            sizeof(ina_str_hdr_t) + hdr->size);
    INA_ASSERT_NOTNULL(hdr);
    return hdr;
}

