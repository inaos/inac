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

typedef struct __ina_mplist_s {
    ina_mempool_t *pool;
    int     active;
    struct __ina_mplist_s *next;
} __ina_mplist_t;

/* Align to 2x word size (as GNU libc does). */
#define __INA_ALIGN_SIZE (2 * sizeof(void*))

/* Round up 'n' to a multiple of ALIGN_SIZE. */
#define __INA_MEM_ALIGN(n) ((n+(__INA_ALIGN_SIZE-1)) & (~(__INA_ALIGN_SIZE-1)))
 
/* Internal memory function */
static ina_malloc_t  __ina_malloc  = NULL;
static ina_realloc_t __ina_realloc = NULL;
static ina_free_t    __ina_free    = NULL;
static ina_memmove_t __ina_memmove = NULL;
static ina_memcpy_t  __ina_memcpy  = NULL;
static ina_memcmp_t  __ina_memcmp  = NULL;
static ina_memchr_t  __ina_memchr  = NULL;
static ina_memset_t  __ina_memset  = NULL;

/* Allocators for memory pools */
static ina_malloc_t  __ina_mp_malloc   = NULL;
static ina_realloc_t __ina_mp_realloc  = NULL;
static ina_free_t    __ina_mp_free     = NULL;

static ina_mempool_t *__pool   = NULL;
static __ina_mplist_t *__pools = NULL;

static void *__ina_sys_malloc(size_t);
static void *__ina_sys_realloc(void *, size_t);
static void __ina_sys_free(void *);
static ina_rc_t __ina_shm_open(ina_mempool_t *);
static ina_rc_t __ina_shm_close(ina_mempool_t *);

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
    void *p;
    p = __ina_malloc(size);
    ina_mem_set(p, 0, size);
    return p;
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

INA_API(ina_rc_t) ina_mem_get_pagesize(size_t *size)
{
#ifndef INA_OS_WIN32
    *size = (size_t)sysconf(_SC_PAGESIZE);
#else
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    *size = (size_t)si.dwPageSize;
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_mempool_init(size_t size)
{
    if (__pools) {
        return INA_SUCCESS;
    }

    if (size == 0) {
        size = __INA_MEM_ALIGN(INA_MEMPOOL_SIZE);
    }
    __pools = (__ina_mplist_t*)__ina_mp_malloc(sizeof(__ina_mplist_t));
    if (__pools == NULL) {
        return INA_FAILURE;
    }

    __pools->pool = NULL;
    __pools->next = NULL;
    __pools->active = 0;

    if (INA_SUCCEED(ina_mempool_create(&__pool, size, INA_MEM_DYNAMIC, NULL))) {
        __pools->pool = __pool;
        __pools->next = NULL;
        __pools->active = 1;
        return INA_SUCCESS;
    }
    __ina_mp_free(__pools);
    __ina_mp_free(__pool);
    __pools = NULL;
    __pool = NULL;
    return INA_FAILURE;
}

INA_API(ina_rc_t) ina_mempool_create(ina_mempool_t **pool, size_t size, uint32_t cf, ina_str_t label)
{
    __ina_mplist_t *last;
    __ina_mplist_t *next;

    last = NULL;
    next = NULL;

    INA_ASSERT(size > 0);

    if (size < INA_MEM_MIN_POOL_SIZE) {
        size = INA_MEM_MIN_POOL_SIZE;
    }
    size = __INA_MEM_ALIGN(size);
    

    *pool = (ina_mempool_t*)__ina_mp_malloc(sizeof(ina_mempool_t));
    if (*pool == NULL) {
        if (__pool != NULL) {
            return INA_MEM_EALLOC;
        }
        return INA_MEM_EALLOC;
    }
    (*pool)->cf = cf;
    (*pool)->pos = 0;
    (*pool)->size = size;
    (*pool)->end = (*pool)->size;
    (*pool)->m = NULL;
    (*pool)->parent = NULL;
    (*pool)->child = NULL;
    (*pool)->current = *pool;
    if (label != NULL) {
        (*pool)->label = ina_str_dup(label);
    } else {
        (*pool)->label = NULL;
    }
    if (cf&INA_MEM_SHARED) {
        INA_ASSERT_NOTNULL((*pool)->label);

        if (!INA_SUCCEED((__ina_shm_open(*pool)))) {
            ina_mem_free((*pool)->label);
            __ina_mp_free(*pool);
            *pool = NULL;
            return INA_MEM_ESHMALLOC;
        }
    } else {
        (*pool)->shm_handle = 0;
        (*pool)->m = __ina_mp_malloc(size);
    }

    if ((*pool)->m == NULL) {
        __ina_mp_free(*pool);
        *pool = NULL;
        return INA_MEM_EALLOC;
    }

    if (!(cf&INA_MEM_SHARED)) {
        __ina_memset((*pool)->m, 0, size);
    }

    if (!(cf&INA_MEM_CHILD)) {
    	last = __pools;
    	while (last != NULL && last->next != NULL) {
            last = last->next;
    	}

    	next = (__ina_mplist_t*)__ina_mp_malloc(sizeof(__ina_mplist_t));
    	if (next == NULL) {
            ina_mem_free((*pool)->label);
            __ina_mp_free((*pool)->m);
            __ina_mp_free(*pool);
            *pool = NULL;
           return ina_err_peek();
        }
    	if (last != NULL) {
            last->next = next;
            next->next = NULL;
            next->pool = *pool;
            next->active = 1;
        }
    }
    INA_TRACE2("New memory pool: size = %ld", (*pool)->size);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_mempool_release(ina_mempool_t *pool, int destroy)
{
    ina_mempool_t *pm;
    ina_mempool_t *pn;
    __ina_mplist_t *ref;

    if (pool == NULL) {
        return INA_SUCCESS;
    }

    /* Unlink parent */
    if (destroy == 1) {
        if (pool->parent != NULL) {
            pool->parent->child = NULL;
        } else {
            ref = __pools;
            while (ref != NULL && ref->pool != pool) {
                ref = ref->next;
            }
            ref->active = 0;
            ref->pool = NULL;
        }
    }

    pn = pool;
    pm = NULL;
    while (pn != NULL) {
        pm = pn;
        if (pn != pn->child) {
            pn = pn->child;
        }
        if (destroy == 1) {
            if (pm->cf&INA_MEM_SHARED) {
                __ina_shm_close(pm);
            } else {
                __ina_mp_free(pm->m);
            }
            __ina_mp_free(pm);
        } else {
            pm->pos = 0;
            pm->end = pm->size;
            ina_mem_set(pm->m, 0, pm->size);
        }
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_mempool_getbylabel(const char* label, ina_mempool_t **pool)
{
    __ina_mplist_t *next;

    INA_ASSERT_NOTNULL(label);
    
     if (__pools == NULL) {
         return INA_FAILURE;
     }

     next = __pools->next;
     while (next != NULL) {
         if (next->active == 1) {
             if (next->pool->label != NULL && strcmp(next->pool->label, label) == 0) {
                 *pool = next->pool;
                 return INA_SUCCESS;
             }
         }
         next = next->next;
     }
     return INA_FAILURE;
}

INA_API(ina_rc_t) ina_mempool_getinfo(ina_mempool_t *pool, ina_mempool_info_t *info)
{
    ina_mempool_t *pm;

    INA_ASSERT_NOTNULL(info);
    if (pool == NULL) {
        pm = __pool;
    }
    else {
        pm = pool;
    }
    if (pm == NULL) {
        return INA_FAILURE;
    }

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

INA_API(void *) ina_mempool_dalloc(ina_mempool_t *pool, size_t size)
{
    void *ret;

    INA_ASSERT_NOTNULL(pool->current);

    ret = NULL;
    size = __INA_MEM_ALIGN(size);

    if ((pool->current->pos + size > pool->current->end) || 
        (pool->current->pos + size < pool->current->pos)) {
        if (pool->cf&INA_MEM_DYNAMIC) {
            size_t nsize = 0;
            if (pool->cf&INA_MEM_BESTFIT) {
                 /* TODO: Best Fit strategy */
            }
            if (pool->size < size && pool->cf&INA_MEM_AUTOSIZE) {
                nsize = __INA_MEM_ALIGN(pool->size * 2);
            } else {
                nsize = pool->size;
            }
            /* FIXME: Push an error , if fails */
            /* FXIME: shm can not handled in chunks ! */
            ina_mempool_create(&pool->current->child, nsize, 
                    pool->cf|INA_MEM_CHILD, 
                    pool->label);
            pool->current->child->parent = pool->current;
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

INA_API(void *) ina_mempool_nalloc(ina_mempool_t *pool, size_t size)
{
    void *ret;

    INA_ASSERT_NOTNULL(pool->current);
    ret = NULL;

    size = __INA_MEM_ALIGN(size);

    if ((pool->current->pos + size > pool->current->end) || 
        (pool->current->pos + size < pool->current->pos)) {
        INA_MEM_EALLOC;
        return NULL;
    }
    ret = &pool->current->m[pool->current->end - size];
    pool->current->end -= size;
    return ret;
}

INA_API(void *) ina_mempool_ralloc(ina_mempool_t *pool, void *old, 
                                    size_t old_size, size_t new_size)
{
    void *ret;

    ret = NULL;
    new_size = __INA_MEM_ALIGN(new_size);

     /* unsatisfiable or bogus request */
    if ((pool->end < old_size) || (pool->end < new_size)) {
        return NULL;
    }
    /* was the previous allocation - optimize! */
    if ((pool->pos >= old_size) && (&pool->m[pool->pos - old_size] == old)) {
        /* fits */
         if (pool->pos + new_size - old_size <= pool->end) {
            /* shrinking - zero again! */
            pool->pos += new_size - old_size;
            if (new_size < old_size) {
                ina_mem_set(&pool->m[pool->pos], 0, old_size - new_size);
                return old;
            }
            /* does not fit */
            INA_MEM_ERALLOC;
            return NULL;
        }
    }
    /* cannot shrink, we need to move */
    if (new_size <= old_size) {
        return old;
    }
    /* fits */
    if ((pool->pos + new_size >= pool->pos) &&
        (pool->pos + new_size <= pool->end))
    {
        ret = &pool->m[pool->pos];
        ina_mem_cpy(ret, old, old_size);
        pool->pos += new_size;
        return ret;
    }
    /* does not fit */
    INA_MEM_ERALLOC;
    return NULL;
}

INA_API(ina_rc_t) ina_mempool_destroy(void)
{
    __ina_mplist_t *ref;
    __ina_mplist_t *next;

    if (__pools == NULL) {
        return INA_SUCCESS;
    }

    next = __pools->next;
    ref = NULL;
    while (next != NULL) {
        if (next->active == 1) {
            /* FXIME: error handling */
            ina_mempool_release(next->pool, 1);
	    next->active = 0;
        }
        next = next->next;
    }
    if (INA_SUCCEED(ina_mempool_release(__pool,1))) {
        ref = __pools;
        next = NULL;
        while (ref != NULL) {
            next = ref->next;
            __ina_mp_free(ref);
            ref = next;
        }
        __pool = NULL;
        __pools = NULL;
        return INA_SUCCESS;
    }
    return INA_FAILURE;
}

static void *
__ina_sys_malloc(size_t nb)
{
    INA_ASSERT_NOTNULL(__pool);
    INA_ASSERT(nb > 0);
    return ina_mempool_dalloc(__pool, nb);
}

static void *
__ina_sys_realloc(void *src, size_t nb) 
{
    INA_ASSERT_NOTNULL(__pool);
    INA_ASSERT_NOTNULL(src);
    INA_ASSERT(nb > 0);
    /* FIXME */
    return ina_mempool_ralloc(__pool, src, nb, nb);
}

static void 
__ina_sys_free(void * ptr)
{
    INA_ASSERT_NOTNULL(__pool);
    INA_ASSERT_NOTNULL(ptr);
}

#ifndef INA_OS_WIN32

static ina_rc_t 
__ina_shm_open(ina_mempool_t *pool)
{
    int flags;

    INA_ASSERT_NOTNULL(pool);
    INA_ASSERT_NOTNULL(pool->label);
    INA_ASSERT(pool->size > 0);
    INA_ASSERT(pool->cf&INA_MEM_SHARED);
    INA_ASSERT_NULL(pool->m);

    pool->size = __INA_MEM_ALIGN(pool->size+sizeof(int64_t));
    pool->end = pool->size;

    flags = O_RDWR;
    if (pool->cf&INA_MEM_SHARED_CREATE) {
        flags =  O_CREAT|O_EXCL|O_RDWR;
        shm_unlink(ina_str_cstr(pool->label));
    }

    pool->shm_handle = shm_open(ina_str_cstr(pool->label), flags, 0x0770);
    if (pool->shm_handle == -1) {
        return INA_MEM_ESHMALLOC;
    }

    if (pool->cf&INA_MEM_SHARED_CREATE) {
        if (ftruncate(pool->shm_handle, pool->size) == -1) {
            close(pool->shm_handle);
            pool->shm_handle = 0;
            shm_unlink(ina_str_cstr(pool->label));
            return INA_MEM_ESHMALLOC;
       }
    }

    pool->m = (void *)mmap(NULL, pool->size, PROT_READ|PROT_WRITE, 
                        MAP_SHARED, 
                        pool->shm_handle, 0);

    if (pool->m == MAP_FAILED) {
        close(pool->shm_handle);
        pool->shm_handle = 0;
        if (pool->cf&INA_MEM_SHARED_CREATE) {
            shm_unlink(ina_str_cstr(pool->label));
        }
        return INA_MEM_ESHMALLOC;
    }
    /* Inc ref count */
    __sync_fetch_and_add((int64_t*)pool->m, 1);
    /* Inc start pos */
    pool->pos += sizeof(int64_t);
    INA_TRACE2("shared mem %s ref count =  %lld", pool->label, *(int64_t*)pool->m);
    return INA_SUCCESS;
}

static ina_rc_t 
__ina_shm_close(ina_mempool_t *pool)
{
    INA_ASSERT_NOTNULL(pool);
    INA_ASSERT(pool->size > 0);
    INA_ASSERT_NOTNULL(pool->label); 
    int64_t cn;

    if (pool->m == NULL) {
         return INA_SUCCESS;
    }

    /* Dec an get ref count before unmap memory */
    cn = __sync_sub_and_fetch((int64_t*)pool->m, 1);
    
    /* Unmap memory */
    munmap(pool->m, pool->size);
    pool->m = NULL;
    pool->size = 0;
    pool->pos = 0;
    pool->end = 0;

    /* Close shared mem */
    close(pool->shm_handle);

    /* Dec ref count, unlink on last relase */
    if (cn == 0) {
        INA_TRACE2("unlinking shared mem %s", pool->label);
        shm_unlink(ina_str_cstr(pool->label));
    }
    INA_TRACE2("shared mem %s ref count =  %lld", pool->label, cn);
    ina_str_destroy(pool->label);

    return INA_SUCCESS;
}
#else
static ina_rc_t 
__ina_shm_open(ina_mempool_t *pool)
{
    INA_ASSERT_NOTNULL(pool);
    INA_ASSERT_NOTNULL(pool->label);
    INA_ASSERT(pool->size > 0);
    INA_ASSERT(pool->cf&INA_MEM_SHARED);
    INA_ASSERT_NULL(pool->m);

    pool->shm_handle = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        pool->size,
        ina_str_cstr(pool->label));

    if (pool->shm_handle == NULL) {
        return INA_MEM_ESHMALLOC;
    }
    pool->m = (void*)MapViewOfFile(pool->shm_handle,
        FILE_MAP_ALL_ACCESS, 
        0,
        0,
        pool->size);

    if (pool->m == NULL) {
        CloseHandle(pool->shm_handle);
        pool->shm_handle = NULL;
        return INA_MEM_ESHMALLOC;
    }
    return INA_SUCCESS;
}

static ina_rc_t 
__ina_shm_close(ina_mempool_t *pool)
{
    INA_ASSERT_NOTNULL(pool);
    INA_ASSERT(pool->size > 0);
    INA_ASSERT_NOTNULL(pool->label); 
    INA_ASSERT_NOTNULL(pool->shm_handle);

    if (pool->m == NULL) {
         return INA_SUCCESS;
    }

    /* TODO: Error handling */
    UnmapViewOfFile(pool->shm_handle);
    CloseHandle(pool->shm_handle);

    pool->m = NULL;
    pool->shm_handle = NULL;
    pool->size = 0;
    pool->pos = 0;
    pool->end = 0;
    
    return INA_SUCCESS;
}
#endif
