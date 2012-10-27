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
#include <unistd.h>
#ifndef INA_OS_WIN32
#include <sys/mman.h>

#include <fcntl.h>
#endif

#include <libinac/lib.h>
#include "config.h"

typedef struct __ina_mempool_list_s {
    ina_mempool_t *pool;
    uint32_t active;
    struct __ina_mempool_list_s *next;
    struct __ina_mempool_list_s *prev;
} __ina_mempool_list_t;

/* Align to 2x word size (as GNU libc does). */
#define __INA_ALIGN_SIZE (2 * sizeof(void*))

/* Round up 'n' to a multiple of ALIGN_SIZE. */
#define __INA_MEM_ALIGN(n) ((n+(__INA_ALIGN_SIZE-1)) & (~(__INA_ALIGN_SIZE-1)))
 
/* Internal memory function */
static ina_malloc_t  __ina_malloc;
static ina_realloc_t __ina_realloc;
static ina_free_t    __ina_free;
static ina_memmove_t __ina_memmove;
static ina_memcpy_t  __ina_memcpy;
static ina_memcmp_t  __ina_memcmp;
static ina_memchr_t  __ina_memchr;
static ina_memset_t  __ina_memset;

/* Allocators for memeory pools */
static ina_malloc_t  __ina_mp_malloc;
static ina_realloc_t __ina_mp_realloc;
static ina_free_t    __ina_mp_free;

static ina_mempool_t *__sysmempool = NULL;
static __ina_mempool_list_t *__mempools = NULL;

static void *__ina_sys_malloc(size_t);
static void *__ina_sys_realloc(void *, size_t);
static void __ina_sys_free(void *);
static ina_shm_handle_t __ina_shm_open(ina_str_t, size_t );
static void * __ina_mmap(void *, size_t, ina_shm_handle_t, size_t);
static ina_rc_t __ina_munmap(void *, size_t);
static ina_rc_t __ina_shm_close(ina_str_t label, ina_shm_handle_t handle);

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
        __ina_malloc = __ina_sys_malloc;
    }
    __ina_free = free_fn;
    if (!__ina_free) {
        __ina_free = __ina_sys_free;
    }
    __ina_realloc = realloc_fn;
    if (!__ina_realloc) {
        __ina_realloc = __ina_sys_realloc;
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

INA_API(ina_rc_t) ina_mempool_set_fn(ina_malloc_t malloc_fn, 
                                 ina_free_t free_fn,
                                 ina_realloc_t realloc_fn)
{
    __ina_mp_malloc = malloc_fn;
    if (!__ina_mp_malloc) {
        __ina_mp_malloc = malloc;
    }
    __ina_mp_free = free_fn;
    if (!__ina_mp_free) {
        __ina_mp_free = free;
    }
    __ina_mp_realloc = realloc_fn;
    if (!__ina_mp_realloc) {
        __ina_mp_realloc = realloc;
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


INA_API(ina_rc_t) ina_mempool_init(size_t size)
{
    ina_rc_t rc;
    
    if (__sysmempool) {
        return INA_SUCCESS;
    }
    
    if (size == 0) {
        size = __INA_MEM_ALIGN(MEMPOOL_SIZE);
    }
    __mempools = (__ina_mempool_list_t*)__ina_mp_malloc(sizeof(__ina_mempool_list_t));
    if (__mempools == NULL) {
        return INA_FAILURE;
    }
    
    rc = ina_mempool_create(&__sysmempool, size, INA_MEM_DYNAMIC, NULL);
    if (INA_SUCCEED(rc)) {
        __mempools->pool = __sysmempool;
        __mempools->prev = NULL;
        __mempools->next = NULL;
        __mempools->active = 1;
    } else {
        __ina_mp_free(__mempools);
    }
    return rc;
}

INA_API(ina_rc_t) ina_mempool_create(ina_mempool_t **pool, size_t size, uint32_t cf, ina_str_t label)
{
    __ina_mempool_list_t *last;
    __ina_mempool_list_t *next;
    
    INA_TRACE("create memory pool");
    INA_ASSERT(size > 0);

    size = __INA_MEM_ALIGN(size);
    
    *pool = (ina_mempool_t*)__ina_mp_malloc(sizeof(ina_mempool_t));
    if (*pool == NULL) {
        return INA_FAILURE;
    }

    if (cf&INA_MEM_SHARED) {
        INA_ASSERT_NOTNULL(label);
        (*pool)->shm_handle = __ina_shm_open(label, size);
        if ((*pool)->shm_handle == INA_FAILURE) {
            INA_TRACE("failed open shared memory");
            __ina_mp_free(*pool);
            return ina_err_peek();
        }
        (*pool)->m = __ina_mmap(NULL, size, (*pool)->shm_handle, 0);
        if (!INA_SUCCEED(ina_err_peek())) {
            INA_TRACE("failed map shared memory");
            __ina_shm_close(label, (*pool)->shm_handle);
            __ina_mp_free(*pool);
            return ina_err_peek();
        }
    } else {
        (*pool)->m = __ina_mp_malloc(size);
    }

    if ((*pool)->m == NULL) {
        __ina_mp_free(*pool);
        return INA_FAILURE;
    }

    if (cf&INA_MEM_FILLZERO) {
        __ina_memset((*pool)->m, 0, size);
    }

    (*pool)->pid = getpid();
    (*pool)->tid = 0;
    (*pool)->cf = cf;
    (*pool)->pos = 0;
    (*pool)->end = size;
    (*pool)->size = size;
    (*pool)->parent = NULL;
    (*pool)->current = *pool;
    if (label != NULL) {
        (*pool)->label = ina_str_dup(label, NULL);
    }
    
    last = __mempools;
    while (last->next != NULL) {
        last = last->next;
    }
    INA_ASSERT_NOTNULL(last);

    next = (__ina_mempool_list_t*)ina_mem_alloc(sizeof(__ina_mempool_list_t));
    if (next == NULL) {
        ina_mem_free((*pool)->label);
        __ina_mp_free((*pool)->m);
        __ina_mp_free(*pool);
        return INA_FAILURE;
    }
    last->next = next;
    next->next = NULL;
    next->active = 1;

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_mempool_release(ina_mempool_t *pool, int destroy)
{
    ina_mempool_t *pm;
    ina_mempool_t *pn;
    __ina_mempool_list_t *ref;

    if (pool == NULL) {
        return INA_SUCCESS;
    }

    INA_TRACE("destroy memory pool");
    if (__sysmempool == pool) {
        INA_TRACE("system memory pool"); 
    }
    
    /* Unlink parent */
    if (destroy == 1) {
        if (pool->parent != NULL) {
            pool->parent->child = NULL;
        } else {
            ref = __mempools;
            while (ref->next != NULL && ref->next->pool == pool) {
                ref = ref->next;
            }
            ref->active = 0;
            ref->pool = NULL;
        }
    }
    INA_TRACE("destroy memory pool 2");
    pn = pool;
    while (pn != NULL) {
        pm = pn;
        pn = pn->child;
        if (destroy == 1) {
            if (pm->cf&INA_MEM_SHARED) {
                __ina_munmap(pm->m, pm->size);
                __ina_shm_close(pm->label, pm->shm_handle);
            } else {
                INA_TRACE("destroy memory pool 3");
                /*__ina_mp_free(pm->m);*/
            }
            /*__ina_mp_free(pm)*/;
        } else {
            pm->pos = 0;
            pm->end = pm->size;
        }
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_mempool_getinfo(ina_mempool_t *pool, ina_mempool_info_t *info)
{
    ina_mempool_t *pm;

    INA_ASSERT_NOTNULL(info);
    pm = (pool==NULL?__sysmempool:pool);
    INA_ASSERT_NOTNULL(pm);

    info->size = 0;
    info->used = 0;
    info->children = -1;
    while (pm != NULL) {
        info->size += pm->size;
        info->used += pm->pos + (pm->size-pm->end);
        ++info->children;
        pm = pm->child;
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_mempool_reset(ina_mempool_t *pool, size_t size)
{
    INA_NOT_IMPL;
    return INA_SUCCESS;
}

INA_API(void *) ina_mempool_dalloc(ina_mempool_t *pool, size_t size)
{
    void *ret;

    INA_ASSERT_NOTNULL(pool->current);

    size = __INA_MEM_ALIGN(size);
    
    if ((pool->current->pos + size > pool->current->end) || 
        (pool->current->pos + size < pool->current->pos)) {
        if (pool->cf&INA_MEM_DYNAMIC) {
            if (pool->cf&INA_MEM_BESTFIT) {
                 /* TODO: Best Fit strategy */
            }
            if (pool->size < size && pool->cf&INA_MEM_AUTOSIZE) {
                size = __INA_MEM_ALIGN(pool->size * 2);
            }
            /* FIXME: Push an error , if fails */
            /* FXIME: shm can not handled in chunks ! */
            ina_mempool_create(&pool->current->child, size, pool->cf, pool->label);
            pool->current = pool->current->child;
        } else {
            INA_MEM_EALLOC;
            return NULL;
        }
    }
    ret = &pool->current->m[pool->current->pos];
    pool->current->pos += size;
    return ret;
}

INA_API(void *)  ina_mempool_nalloc(ina_mempool_t *pool, size_t size)
{
    void *ret;

    INA_ASSERT_NOTNULL(pool->current);
    
    size = __INA_MEM_ALIGN(size);
    
    if ((pool->current->pos + size > pool->current->end) || 
        (pool->current->pos + size < pool->current->pos)) {
        return NULL;
    }
    ret = &pool->current->m[pool->current->end - size];
    pool->current->end -= size;
    return ret;
}

INA_API(void *) ina_mempool_ralloc(ina_mempool_t *pool, void *old, size_t pnb, size_t nnb)
{
    INA_NOT_IMPL;
    return NULL;
}

INA_API(ina_rc_t) ina_mempool_destroy(void)
{
    __ina_mempool_list_t *ref;
    __ina_mempool_list_t *next;

    INA_TRACE("release and destroy all memory pools");
    
    ref = __mempools;
    while (ref != NULL) {
        if (ref->pool != __sysmempool) {
            if (ref->active == 1) {
                /* FXIME: error handling */
                ina_mempool_release(ref->pool, 1);
            }
        }
        ref = ref->next;
    }
    if (INA_SUCCEED(ina_mempool_release(__sysmempool,1))) {
        ref = __mempools;
        next = NULL;
        while (ref != NULL) {
            next = ref->next;
            ina_mem_free(ref);
            ref = next;
        }
        __sysmempool = NULL;
        __mempools = NULL;
    }
    return INA_SUCCESS;
}

static void *
__ina_sys_malloc(size_t nb)
{
    INA_ASSERT_NOTNULL(__sysmempool);
    INA_ASSERT(nb > 0);
    return ina_mempool_dalloc(__sysmempool, nb);
}

static void *
__ina_sys_realloc(void *src, size_t nb) 
{
    INA_ASSERT_NOTNULL(__sysmempool);
    INA_ASSERT_NOTNULL(src);
    INA_ASSERT(nb > 0);
    /* FIXME */
    return ina_mempool_ralloc(__sysmempool, src, nb, nb);
}

static void 
__ina_sys_free(void * ptr)
{
    INA_ASSERT_NOTNULL(__sysmempool);
    INA_ASSERT_NOTNULL(ptr);
}

static ina_shm_handle_t 
__ina_shm_open(ina_str_t label, size_t size)
{
    ina_shm_handle_t handle;
    
#ifndef INA_OS_WIN32
    handle = shm_open(ina_str_cstr(label) , O_CREAT|O_RDWR, 0777);
    if (handle == -1) {
        INA_TRACE("failed shm_open()");
        /* FIXME: Specific error */
        INA_MEM_EALLOC;
        return INA_FAILURE;
    }

    if (ftruncate(handle, size) == -1) {
        INA_TRACE("failed ftruncate()");
        close(handle);
        /* shm_unlink(ina_str_cstr(label));*/
       /* FIXME: Specific error */
        INA_MEM_EALLOC;
        return INA_FAILURE;
    }
#else
#   error platform not supported
#endif

    return handle;
}

static ina_rc_t 
__ina_shm_close(ina_str_t label, ina_shm_handle_t handle)
{
#ifndef INA_OS_WIN32
    close(handle);
    /* shm_unlink(ina_str_cstr(label));*/
#else
#   error platform not supported
#endif
    return INA_SUCCESS;
}
static void *
__ina_mmap(void *addr, size_t size, ina_shm_handle_t handle, size_t offset)
{
    void *m;

#ifndef INA_OS_WIN32
    m = mmap(addr, size, PROT_READ|PROT_WRITE, MAP_SHARED, handle, offset);
    if (m == MAP_FAILED) {
        INA_TRACE("failed mmap()");
        INA_MEM_EALLOC;
    }
#else
#   error platform not supported
#endif
    return m;
}

static ina_rc_t
__ina_munmap(void *ptr, size_t size)
{
    if (ptr == NULL) {
        return INA_SUCCESS;
    }
    INA_ASSERT(size > 0);
    munmap(ptr, size);
    return INA_SUCCESS;
}

