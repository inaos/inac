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
#include "config.h"

static ina_malloc_t  __ina_malloc;
static ina_realloc_t __ina_realloc;
static ina_free_t    __ina_free;
static ina_memmove_t __ina_memmove;
static ina_memcpy_t  __ina_memcpy;
static ina_memcmp_t  __ina_memcmp;
static ina_memchr_t  __ina_memchr;
static ina_memset_t  __ina_memset;

static ina_mempool_t *mempool_root;

 
INA_API(ina_rc_t) ina_mem_set_fn(ina_malloc_t malloc_fn, 
                                 ina_free_t free_fn,
                                 ina_realloc_t realloc_fn,
                                 ina_memmove_t memmove_fn,
                                 ina_memcpy_t memcpy_fn,
                                 ina_memcmp_t memcmp_fn,
                                 ina_memchr_t memchr_fn,
                                 ina_memset_t memset_fn)
{
    __ina_malloc = malloc_fn;
    if (!__ina_malloc) {
        __ina_malloc = malloc;
    }
    __ina_free = free_fn;
    if (!__ina_free) {
        __ina_free = free;
    }
    __ina_realloc = realloc_fn;
    if (!__ina_realloc) {
        __ina_realloc = realloc;
    }
    __ina_memmove = memmove_fn;
    if (!__ina_memmove) {
        __ina_memmove = memmove;
    }
    __ina_memcpy = memcpy_fn;
    if (!__ina_memcpy) {
        __ina_memcpy = memcpy;
    }
    __ina_memcmp = memcmp_fn;
    if (!__ina_memcmp) {
        __ina_memcmp = memcmp;
    }
    __ina_memchr = memchr_fn;
    if (!__ina_memchr) {
        __ina_memchr = memchr;
    }    
    __ina_memset = memset_fn;
    if (!__ina_memset) {
        __ina_memset = memset;
    }    
    return INA_SUCCESS;
}

INA_API(void *) ina_mem_alloc(size_t size)
{
    return __ina_malloc(size);
}

INA_API(void) ina_mem_free(void *ptr)
{
    __ina_free(ptr);
}

INA_API(void *) ina_mem_realloc(void *ptr, size_t nb)
{
    return __ina_realloc(ptr, nb);
}

INA_API(void *) ina_mem_move(void *dest, const void *src, size_t nb)
{
    return __ina_memmove(dest, src, nb);
}

INA_API(void *) ina_mem_cpy(void *dest, const void *src, size_t nb)
{
    return __ina_memcpy(dest, src, nb);
}

INA_API(int) ina_mem_cmp(const void *lhs, const void *rhs, size_t nb)
{
    return __ina_memcmp(lhs, rhs, nb);
}

INA_API(void *) ina_mem_set(void *dest, int value, size_t nb)
{
    return __ina_memset(dest, value, nb);
}

INA_API(void *) ina_mem_chr(const void *dest, int value, size_t nb)
{
    return __ina_memchr(dest, value, nb);
}


INA_API(ina_rc_t) ina_mempool_init(void)
{
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_mempool_create(ina_mempool_t **pool)
{
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_mempool_destroy(ina_mempool_t *pool)
{
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_mempool_reset(ina_mempool_t *pool)
{
    return INA_SUCCESS;
}

INA_API(void *) ina_mempool_palloc(ina_mempool_t *pool, size_t size)
{
    return NULL;
}

INA_API(ina_rc_t) ina_mempool_pfree(ina_mempool_t *pool, void *p)
{
    return INA_SUCCESS;
}

