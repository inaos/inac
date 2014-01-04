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

#ifdef INA_CSTRING_ENABLED

INA_API(ina_str_t) ina_str_new(size_t size)
{
    ina_str_t str;
    str = (ina_str_t)INA_MEM_MALLOC(size+1);
    if (str == NULL) {
        INA_STR_EALLOC;
    }
    str[0] = '\0';
    return str; 
}

INA_API(ina_str_t) ina_str_new_using_pool(size_t size, ina_mempool_t *pool)
{
    ina_str_t str;
    if (pool == NULL) {
        str = (ina_str_t)ina_mem_alloc(size+1);
        if (str == NULL) {
            INA_STR_EALLOC;
            return NULL;
        }
    } else {
        str = (ina_str_t)ina_mempool_dalloc(pool, size+1);
        if (str == NULL) {
            INA_STR_EALLOC;
            return NULL;
        }
    }
    str[0] = '\0';
    return str; 
}

INA_API(ina_str_t) ina_str_new_fromblk(const void* blk, size_t len) 
{
    ina_str_t str;

    if (blk == NULL) {
        INA_STR_EALLOC;
        return NULL;
    }

    str = (ina_str_t)INA_MEM_MALLOC(len+1);
    if (str == NULL)  {
        INA_STR_EALLOC;
        return NULL;
    }
    if (len > 0) {
        ina_mem_cpy(str, blk, len);
    }
    str[len] = '\0';
    return str;
}

INA_API(ina_str_t) ina_str_new_fromblk_using_pool(const void* blk, 
                                                  size_t len, 
                                                  ina_mempool_t *pool)
{
    ina_str_t str;

    INA_ASSERT_NOTNULL(pool);
    
    if (blk == NULL) {
        INA_STR_EALLOC;
        return NULL;
    }

    str = (ina_str_t)ina_mempool_dalloc(pool, len+1);
    if (str == NULL)  {
        INA_STR_EALLOC;
        return NULL;
    }
    if (len > 0) {
        ina_mem_cpy(str, blk, len);
    }
    str[len] = '\0';
    return str;
}

INA_API(ina_str_t) ina_str_new_fromcstr(const char* cstr)
{
    ina_str_t str;
    size_t len;

    str = NULL;
    
    if (cstr != NULL) {
        len = strlen(cstr);
        str = (ina_str_t)INA_MEM_MALLOC(len+1);
        if (str == NULL) {
            INA_STR_EALLOC;
            return NULL;
        }
        ina_mem_cpy(str, cstr, len);
        str[len] = '\0';
    }
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
        str = (ina_str_t)ina_mempool_dalloc(pool, len+1);
        if (str == NULL) {
            INA_STR_EALLOC;
            return NULL;
        }
        ina_mem_cpy(str, cstr, len);
        str[len] = '\0';
    }
    return str;
}

INA_API(ina_rc_t) ina_str_free(ina_str_t str)
{
    if (str != NULL) {
    	INA_MEM_FREE(str);
    }
    return INA_SUCCESS;
}

INA_API(ina_str_t) ina_str_dup(const ina_str_t str)
{
    if (str == NULL) {
        return NULL;
    }
    return ina_str_new_fromcstr(str);
}

INA_API(ina_str_t) ina_str_dup_using_pool(const ina_str_t str, 
                                          ina_mempool_t *pool)
{
    if (str == NULL) {
        return NULL;
    }
    return ina_str_new_fromcstr_using_pool(str, pool);
}

INA_API(const char*) ina_str_cstr(const ina_str_t str)
{
    return str;
}

INA_API(ina_str_t) ina_str_cpy(ina_str_t dest, const ina_str_t src)
{
    return strcpy(dest, src);
}

INA_API(ina_str_t) ina_str_ncpy(ina_str_t dest, const ina_str_t src, size_t n)
{
    return strncpy(dest, src, n);
}

INA_API(ina_str_t) ina_str_cat(ina_str_t dest, const ina_str_t src)
{
    return strcat(dest, src);
}

INA_API(ina_str_t) ina_str_catcstr(ina_str_t dest, const char *src)
{
    return strcat(dest, src);
}

INA_API(ina_str_t) ina_str_ncat(ina_str_t dest, const ina_str_t src, size_t n)
{
    return strncat(dest, src, n);
}

INA_API(int) ina_str_cmp(const ina_str_t lhs, const ina_str_t rhs)
{
    return strcmp(lhs, rhs);
}

INA_API(int) ina_str_casecmp(const ina_str_t lhs, const ina_str_t rhs)
{
#ifdef INA_OS_WIN32
    return _stricmp(lhs, rhs);
#else
    return strcasecmp(lhs, rhs);
#endif
}

INA_API(int) ina_str_ncmp(const ina_str_t lhs, const ina_str_t rhs, size_t n)
{
    return strncmp(lhs, rhs, n);
}

INA_API(const char*) ina_str_rchr(const ina_str_t str, const char chr)
{
    if (chr == 0) {
        return NULL;
    }   
    return strrchr(str, chr);
}

INA_API(const char*) ina_str_str(const ina_str_t str1, const ina_str_t str2)
{
    if (str2 == NULL) {
        return NULL;
    }
    if (strlen(str2) == 0) {
        return NULL;
    }
    return strstr(str1, str2);
}

INA_API(const char*) ina_str_strcstr(const ina_str_t str1, const char *str2)
{
    if (str2 == NULL) {
        return NULL;
    }
    if (strlen(str2) == 0) {
        return NULL;
    }
    return strstr(str1, str2);
}

INA_API(size_t) ina_str_len(const ina_str_t str)
{
    if (str == NULL) {
        return 0;
    }
    return strlen(str);
}

INA_API(size_t) ina_str_size(const ina_str_t str)
{
    if (str == NULL) {
        return 0;
    }
    return strlen(str)+1;
}

INA_API(ina_str_t) ina_str_toupper(ina_str_t str)
{
    if (str) {
        size_t len = strlen(str);
        size_t j;
        for (j = 0; j < len; j++) {
            str[j] = toupper(str[j]);
        }
    }
    return str;
}

INA_API(ina_str_t) ina_str_tolower(ina_str_t str)
{
    if (str) {
        size_t len = strlen(str);
        size_t j;
        for (j = 0; j < len; j++) {
            str[j] = tolower(str[j]);
        }
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
    str = ina_str_new(size);
    if (str == NULL) {
        INA_STR_EALLOC;
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

    va_copy(args_copy, args);
    if ((l = vsnprintf(*str, len, fmt, args)) >= len) {
        ina_str_t extra_str;
        if ((extra_str = ina_str_new(l))) {
            l = vsnprintf(extra_str, l+1, fmt, args_copy);
            ina_str_free(*str);
            *str = extra_str;
        } else {
            INA_ERR_PUSH_LAST;
            l = 1;
        }
    }
    va_end(args_copy);
    return l;    
}

#endif
