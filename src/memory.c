/*
 * Copyright (c) 2012-2018, INAOS GmbH
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
#include <sys/stat.h>
#include <libinac/lib.h>
#include "config.h"

typedef struct __ina_mplist_s {
    ina_mempool_t *pool;
    int     active;
    struct __ina_mplist_s *next;
} __ina_mplist_t;
 
struct ina_mempool_s  {
    ina_handle_t shm_handle;
    uint32_t cf;
    size_t size;
    size_t pos;
    size_t end;
    unsigned char *m;
    ina_str_t label;
    ina_malloc_t memalloc;
    ina_free_t  memfree;
    struct ina_mempool_s *current;
    struct ina_mempool_s *parent;
    struct ina_mempool_s *child;
    ina_mempool_event_handler_t event_handler;
};

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

/* Internal memory function */
static ina_malloc_t  __ina_malloc  = __ina_sys_malloc;
static ina_realloc_t __ina_realloc = __ina_sys_realloc;
static ina_free_t    __ina_free    = __ina_sys_free;
static ina_memmove_t __ina_memmove = INA_MEM_MEMMOVE;
static ina_memcpy_t  __ina_memcpy  = INA_MEM_MEMCPY;
static ina_memcmp_t  __ina_memcmp  = INA_MEM_MEMCMP;
static ina_memchr_t  __ina_memchr  = INA_MEM_MEMCHR;
static ina_memset_t  __ina_memset  = INA_MEM_MEMSET;

INA_API(ina_rc_t) ina_mempool_set_fn(ina_malloc_t malloc_fn, 
                                 ina_free_t free_fn,
                                 ina_realloc_t realloc_fn)
{
    __ina_mp_malloc = malloc_fn;
    if (!__ina_mp_malloc) {
        __ina_mp_malloc = INA_MEM_MALLOC;
    }
    __ina_mp_free = free_fn;
    if (!__ina_mp_free) {
        __ina_mp_free = INA_MEM_FREE;
    }
    __ina_mp_realloc = realloc_fn;
    if (!__ina_mp_realloc) {
        __ina_mp_realloc = INA_MEM_REALLOC;
    }
    return INA_SUCCESS;
}

INA_API(void *) ina_mem_alloc(size_t size)
{
    void *p = NULL;
    p = __ina_malloc(size);
    if (p == NULL) {
        INA_OS_ERROR(INA_ERR_OUT_OF|INA_NN_MEMORY);
        return NULL;
    }
    ina_mem_set(p, 0, size);
    return p;
}

INA_API(ina_rc_t) ina_mem_get_aligned_size(size_t query, size_t *aligned)
{
    INA_ASSERT_NOTNULL(aligned);
    *aligned =  ((query+(INA_MEM_ALIGN_SIZE-1)) & (~(INA_MEM_ALIGN_SIZE-1)));
    return INA_SUCCESS;
}

INA_API(void *) ina_mem_alloc_aligned(size_t alignment, size_t size)
{     
    /* Allocate necessary memory area
     * client request - size parameter -
     * plus area to store the address
     * of the memory returned by standard
     * malloc().
     */
    void *p = ina_mem_alloc(size + alignment - 1 + sizeof(void*));
     
    if (p != NULL) {
        void *ptr;
        /* Address of the aligned memory according to the align parameter*/
        ptr = (void*) (((size_t)p + sizeof(void*) + alignment -1) & ~(alignment-1));
        /* store the address of the malloc() above
         * at the beginning of our total memory area.
         * You can also use *((void **)ptr-1) = p
         * instead of the one below.
         */
        *((void**)((size_t)ptr - sizeof(void*))) = p;
        /* Return the address of aligned memory */
        return ptr;
    }
    INA_ERROR(INA_ERR_OUT_OF|INA_NN_MEMORY);
    return NULL;
}

INA_API(void) ina_mem_free(void *ptr)
{
    INA_ASSERT_NOTNULL(ptr);
    __ina_free(ptr);
}

INA_API(void) ina_mem_free_aligned(void *ptr)
{
    /* Get the address of the memory, stored at the
     * start of our total memory area. Alternatively,
     * you can use void *ptr = *((void **)p-1) instead
     * of the one below.
     */
    void *p = *((void**)((size_t)ptr - sizeof(void*)));
    ina_mem_free(p);
}

INA_API(void *) ina_mem_realloc(void *ptr, size_t nb)
{
    INA_ASSERT_NOTNULL(ptr);
    return __ina_realloc(ptr, nb);
}

INA_API(void *) ina_mem_move(void *dest, const void *src, size_t nb)
{
    INA_ASSERT_NOTNULL(dest);
    INA_ASSERT_NOTNULL(src);
    return __ina_memmove(dest, src, nb);
}

INA_API(void *) ina_mem_cpy(void *dest, const void *src, size_t nb)
{
    INA_ASSERT_NOTNULL(dest);
    INA_ASSERT_NOTNULL(src);
    return __ina_memcpy(dest, src, nb);
}

INA_API(int) ina_mem_cmp(const void *lhs, const void *rhs, size_t nb)
{
    INA_ASSERT_NOTNULL(lhs);
    INA_ASSERT_NOTNULL(rhs);
    return __ina_memcmp(lhs, rhs, nb);
}

INA_API(void *) ina_mem_set(void *dest, int value, size_t nb)
{
    INA_ASSERT_NOTNULL(dest);
    return __ina_memset(dest, value, nb);
}

INA_API(void *) ina_mem_chr(const void *dest, int value, size_t nb)
{
    INA_ASSERT_NOTNULL(dest);
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
        size = INA_MEM_ALIGN(INA_MEMPOOL_SIZE);
    }
    __pools = (__ina_mplist_t*)__ina_mp_malloc(sizeof(__ina_mplist_t));
    if (__pools == NULL) {
        return INA_ERROR(INA_NN_POOL|INA_ERR_FATAL);
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
    return INA_ERROR(INA_NN_POOL|INA_ERR_FATAL);
}

INA_API(ina_rc_t) ina_mempool_create(ina_mempool_t **pool, size_t size, uint32_t cf, const char *label)
{
    __ina_mplist_t *last;
    __ina_mplist_t *next;

    last = NULL;
    next = NULL;

    INA_ASSERT(size > 0);

    if (size < INA_MEM_MIN_POOL_SIZE) {
        size = INA_MEM_MIN_POOL_SIZE;
    }
    size = INA_MEM_ALIGN(size);

    *pool = (ina_mempool_t*)__ina_mp_malloc(sizeof(ina_mempool_t));
    if (*pool == NULL) {
        if (__pool != NULL) {
            return INA_ERROR(INA_ERR_OUT_OF|INA_NN_MEMORY);
        }
        return INA_ERROR(INA_ERR_OUT_OF|INA_NN_MEMORY);
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
        (*pool)->label = ina_str_new_fromcstr(label);
    } else {
        (*pool)->label = NULL;
    }
    if (cf&INA_MEM_SHARED) {
        INA_ASSERT_NOTNULL((*pool)->label);

        if (!INA_SUCCEED((__ina_shm_open(*pool)))) {
            ina_mem_free((*pool)->label);
            __ina_mp_free(*pool);
            *pool = NULL;
            return ina_err_get_last_rc();
        }
    } else {
        (*pool)->shm_handle = 0;
        (*pool)->m = (unsigned char*)__ina_mp_malloc(size);
    }

    if ((*pool)->m == NULL) {
        __ina_mp_free(*pool);
        *pool = NULL;
        return INA_ERROR(INA_ERR_OUT_OF|INA_NN_MEMORY);
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
           return INA_ERROR(INA_ERR_OUT_OF|INA_NN_MEMORY);
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

    pool->current = pool;

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
            /*pm->end = pm->size;*/
            ina_mem_set(pm->m, 0, pm->end);
        }
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_mempool_shrink(ina_mempool_t *pool, size_t chunks, 
                                     ina_mempool_info_t *info)
{
    ina_mempool_t *pm;
    ina_mempool_t *pn;
    size_t c;
 
    INA_ASSERT_NOTNULL(pool);
    INA_ASSERT_NOTNULL(info);

    if (INA_FAILED(ina_mempool_getinfo(pool, info))) {
        return ina_err_get_last_rc();
    }

    if (info->children <= chunks) {
        return INA_SUCCESS;
    }

    c = info->children;
    pn = pool;
    pm = NULL;
    while (pn != NULL) {
        pm = pn;
        if (pn != pn->child) {
            pn = pn->child;
        }
        if (c > chunks && pool->end == pool->size) {
            if (pm->cf&INA_MEM_SHARED) {
                __ina_shm_close(pm);
            } else {
                __ina_mp_free(pm->m);
            }
            __ina_mp_free(pm);
        } else {
            pm->pos = 0;
            ina_mem_set(pm->m, 0, pm->end);
        }
        --c;
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_mempool_reset(ina_mempool_t *pool)
{
    ina_mempool_t *pm;
    ina_mempool_t *pn;
  
    INA_ASSERT_NOTNULL(pool);

    pn = pool;
    pm = NULL;
    while (pn != NULL) {
        pm = pn;
        if (pn != pn->child) {
            pn = pn->child;
        }
        pm->pos = 0;
        ina_mem_set(pm->m, 0, pm->end);
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_mempool_getbylabel(const char* label, ina_mempool_t **pool)
{
    __ina_mplist_t *next;

    INA_ASSERT_NOTNULL(label);

     if (__pools == NULL) {
         return INA_ERROR(INA_NN_POOL|INA_ERR_NOT_INITIALIZED);
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
     return INA_ERROR(INA_ERR_NOT_FOUND);
}

INA_API(ina_rc_t) ina_mempool_getbypointer(const void *ptr, ina_mempool_t **pool)
{
    __ina_mplist_t *next;

    INA_ASSERT_NOTNULL(ptr);
    INA_ASSERT_NOTNULL(pool);

     if (__pools == NULL) {
         return INA_ERROR(INA_NN_POOL|INA_ERR_NOT_INITIALIZED);
     }

     next = __pools->next;
     while (next != NULL) {
         if (next->active == 1) {
             if (next->pool->m >= (unsigned char*)ptr || (next->pool->m + next->pool->end) > (unsigned char*)ptr) {
                 *pool = next->pool;
                 return INA_SUCCESS;
             }
         }
         next = next->next;
     }
     return INA_ERROR(INA_ERR_NOT_FOUND);
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
        return INA_ERROR(INA_NN_POOL|INA_ERR_NOT_INITIALIZED);
    }

    info->size = 0;
    info->used = 0;
    info->children = 0;
    info->cf = pm->cf;
    while (pm != NULL) {
        info->size += pm->size;
        info->used += pm->pos + (pm->size-pm->end);
        ++info->children;
        pm = pm->child;
    }
    --info->children;
    return INA_SUCCESS;
}

INA_API(void *) ina_mempool_dalloc(ina_mempool_t *pool, size_t size)
{
    void *ret;
    size_t nsize;

    INA_ASSERT_NOTNULL(pool);
    INA_ASSERT_NOTNULL(pool->current);

    ret = NULL;
    if (pool->cf^INA_MEM_BESTFIT) {
        size = INA_MEM_ALIGN(size);
    }

retry:

    if ((pool->current->pos + size > pool->current->end) || 
        (pool->current->pos + size < pool->current->pos)) {
        if (pool->cf&INA_MEM_DYNAMIC) {
            if (pool->current->child != NULL) {
                pool->current = pool->current->child;
                goto retry;
            }
            nsize = 0;
            if (pool->cf&INA_MEM_BESTFIT) {
                 /* TODO: Best Fit strategy */
            }

            nsize = 0;
  
            if (pool->cf&INA_MEM_AUTOSIZE || size > pool->size) {
                nsize = size;
            } else {
                nsize = pool->size;
            }

            /* FXIME: shm can not handled in chunks ! */
            if (!INA_SUCCEED(ina_mempool_create(&pool->current->child, nsize, 
                    pool->cf|INA_MEM_CHILD, 
                    pool->label))) {
                return NULL;
            }
            pool->current->child->parent = pool->current;
            pool->current = pool->current->child;
        } else {
            INA_ERROR(INA_NN_POOL|INA_ERR_FULL);
            return NULL;
        }
    }
    ret = &pool->current->m[pool->current->pos];
    pool->current->pos += size;
    return ret;
}

INA_API(ina_rc_t) ina_mempool_free(ina_mempool_t *pool, void *ptr, size_t size)
{

    INA_ASSERT_NOTNULL(pool);
    INA_ASSERT_NOTNULL(ptr);
    INA_ASSERT_TRUE(size > 0);
    
    if (pool->cf^INA_MEM_BESTFIT) {
        size = INA_MEM_ALIGN(size);
    }
    
     /* bogus request */
    if (pool->end < size) {
        return INA_ERROR(INA_NN_SIZE|INA_ERR_INVALID);
    }

    if ((pool->pos >= size) && (&pool->m[pool->pos - size] == ptr)) {
        /* fits */
         /*if (pool->pos + new_size - old_size <= pool->end) { */
            /* shrinking - zero again! */
           /* pool->pos -= size;
            if (new_size < old_size) {
                ina_mem_set(&pool->m[pool->pos], 0, old_size - new_size);
            }
            return INA_SUCCESS;
        }*/
    }
    return INA_SUCCESS;
}

INA_API(void *) ina_mempool_nalloc(ina_mempool_t *pool, size_t size)
{
    void *ret;

    INA_ASSERT_NOTNULL(pool);
    INA_ASSERT_NOTNULL(pool->current);
    ret = NULL;

    if (pool->cf^INA_MEM_BESTFIT) {
        size = INA_MEM_ALIGN(size);
    }

     /* bogus request */
    if (pool->end < size) {
        INA_ERROR(INA_NN_SIZE|INA_ERR_INVALID);
        return NULL;
    }

    if ((pool->current->pos + size > pool->current->end) || 
        (pool->current->pos + size < pool->current->pos)) {
        if (pool->cf&INA_MEM_DYNAMIC) {
            size_t nsize = 0;
  
            if (pool->cf&INA_MEM_AUTOSIZE || size > pool->size) {
                nsize = size;
            } else {
                nsize = pool->size;
            }

            /* FXIME: shm can not handled in chunks ! */
            ina_mempool_create(&pool->current->child, nsize, 
                    pool->cf|INA_MEM_CHILD, 
                    pool->label);
            pool->current->child->parent = pool->current;
            pool->current = pool->current->child;
        } else {
            INA_ERROR(INA_NN_POOL|INA_ERR_FULL);
            return NULL;
        }
    }
    ret = &pool->current->m[pool->current->end - size];
    pool->current->end -= size;
    return ret;
}

INA_API(void *) ina_mempool_ralloc(ina_mempool_t *pool, void *old, 
                                    size_t old_size, size_t new_size)
{
    void *ret;

    INA_ASSERT_NOTNULL(pool);
    INA_ASSERT_NOTNULL(pool->current);

    ret = NULL;
    if (pool->cf^INA_MEM_BESTFIT) {
        new_size = INA_MEM_ALIGN(new_size);
        old_size = INA_MEM_ALIGN(old_size);
    }

     /* bogus request */
    if (pool->end < old_size) {
        INA_ERROR(INA_NN_SIZE|INA_ERR_INVALID);
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
            }
            return old;
        }
        /* does not fit */
        if (pool->cf&INA_MEM_DYNAMIC) {
            size_t nsize = 0;
 
            if (pool->cf&INA_MEM_AUTOSIZE || new_size > pool->size) {
                nsize = new_size;
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
            ret = &pool->m[pool->pos];
            ina_mem_cpy(ret, old, old_size);
            pool->pos += new_size;
            return ret;
        }
        INA_ERROR(INA_NN_POOL|INA_ERR_FULL);
        return NULL;
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
    if (pool->cf&INA_MEM_DYNAMIC) {
        size_t nsize = 0;

        if (pool->cf&INA_MEM_AUTOSIZE || new_size > pool->size) {
            nsize = new_size;
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
        ret = &pool->m[pool->pos];
        ina_mem_cpy(ret, old, old_size);
        pool->pos += new_size;
        return ret;
    }
    INA_ERROR(INA_NN_POOL|INA_ERR_FULL);
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
    if (INA_SUCCEED(ina_mempool_release(__pool, INA_YES))) {
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
    return ina_err_get_last_rc();
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

    if (nb == 0 && src != NULL) {
        ina_mem_free(src);
        return NULL;
    }

    if (src != NULL) {
        void *p = ina_mempool_dalloc(__pool, nb);
        ina_mem_cpy(p, src, nb);
        return p;
    }
    return ina_mempool_dalloc(__pool, nb);
}

static void __ina_sys_free(void *ptr)
{
    INA_ASSERT_NOTNULL(ptr);
    INA_ASSERT_NOTNULL(__pool);
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

    pool->size = INA_MEM_ALIGN(pool->size+sizeof(int64_t));
    pool->end = pool->size;

    flags = O_RDWR;
    if (pool->cf&INA_MEM_SHARED_CREATE) {
        flags |=  O_CREAT;
        if (pool->cf&INA_MEM_SHARED_EXCL) {
            flags |= O_EXCL;
        }
    }

    pool->shm_handle = shm_open(ina_str_cstr(pool->label), flags, S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP | S_IWOTH | S_IROTH);
    if (pool->shm_handle == -1) {
        return INA_ERROR(INA_NN_HANDLE|INA_ERR_INVALID);
    }

    if (pool->cf&INA_MEM_SHARED_EXCL) {
        if (ftruncate(pool->shm_handle, pool->size) == -1) {
            close(pool->shm_handle);
            pool->shm_handle = 0;
            shm_unlink(ina_str_cstr(pool->label));
            return INA_ERROR(INA_NN_OPERATION|INA_ERR_FAILED);
       }
    }

    pool->m = (void *)mmap(NULL, pool->size, PROT_READ|PROT_WRITE, 
                        MAP_SHARED, 
                        pool->shm_handle, 0);

    if (pool->m == MAP_FAILED) {
        close(pool->shm_handle);
        pool->shm_handle = 0;
        if (pool->cf&INA_MEM_SHARED_EXCL) {
            shm_unlink(ina_str_cstr(pool->label));
        }
        return INA_ERROR(INA_NN_OPERATION|INA_ERR_FAILED);
    }
    /* Inc ref count */
    __sync_fetch_and_add((int64_t*)pool->m, 1);
    /* Inc start pos */
    pool->pos += sizeof(int64_t);
    INA_TRACE2("shared mem %s ref count =  %" INA_INT64_T_FMT, pool->label, *(int64_t*)pool->m);
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
    if (cn == 0 || pool->cf&INA_MEM_SHARED_EXCL) {
        INA_TRACE2("unlinking shared mem %s", pool->label);
        shm_unlink(ina_str_cstr(pool->label));
    }
    INA_TRACE2("shared mem %s ref count =  %" INA_INT64_T_FMT, pool->label, cn);
    ina_str_free(pool->label);

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
        return INA_ERR(INA_EALLOC);
    }
    pool->m = (void*)MapViewOfFile(pool->shm_handle,
        FILE_MAP_ALL_ACCESS, 
        0,
        0,
        pool->size);

    if (pool->m == NULL) {
        CloseHandle(pool->shm_handle);
        pool->shm_handle = NULL;
        return INA_ERR(INA_EALLOC);
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
