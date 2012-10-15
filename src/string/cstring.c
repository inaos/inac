/*
 * Copyright (c) 2012, INAOS GmbH
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
#include "../config.h"

#ifdef CSTRING_ENABLED

INA_API(ina_str_t) ina_str_newlen(size_t len, ina_mempool_t *pool)
{
    ina_str_t str;
    if (pool == NULL) {
        str = (ina_str_t)ina_mem_alloc(len+1);
    } else {
        str = (ina_str_t)ina_mempool_alloc(pool, len+1);
    }
    str[0] = '\0';
    return str; 
}

INA_API(ina_str_t) ina_str_fromcstr(const char* cstr, ina_mempool_t *pool)
{
    ina_str_t str;
    size_t len;

    str = NULL;
    
    if (cstr != NULL) {
        len = strlen(cstr);
        if (pool == NULL) {
            str = (ina_str_t)ina_mem_alloc(len+1);
        } else {
            str = (ina_str_t)ina_mempool_alloc(pool, len+1);
        }
        ina_mem_cpy(str, cstr, len);
        str[len] = '\0';
    }
    return str;
}

INA_API(ina_rc_t) ina_str_destroy(ina_str_t str)
{
    ina_mem_free(str);
    return INA_SUCCESS;
}

INA_API(ina_str_t) ina_str_dup(const ina_str_t str, ina_mempool_t *pool)
{
    if (str == NULL) {
        return NULL;
    }
    return ina_str_fromcstr(str, pool);
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

INA_API(ina_str_t) ina_str_ncat(ina_str_t dest, const ina_str_t src, size_t n)
{
    return strncat(dest, src, n);
}

INA_API(ina_rc_t) ina_str_cmp(const ina_str_t lhs, const ina_str_t rhs)
{
    return strcmp(lhs, rhs);
}

INA_API(ina_rc_t) ina_str_ncmp(const ina_str_t lhs, const ina_str_t rhs, size_t n)
{
    return strncmp(lhs, rhs, n);
}

INA_API(size_t) ina_str_len(const ina_str_t str)
{
    return strlen(str);
}


INA_API(ina_str_t) ina_str_vsprintf(const char *fmt, ...)
{
    va_list arglist;
    ina_str_t str;

    INA_NOT_IMPL;
    return str;
}
#endif