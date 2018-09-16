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

static ina_rc_t __ina_shm_open(ina_mempool_t *);
static ina_rc_t __ina_shm_close(ina_mempool_t *);

/* Internal memory function */
static ina_malloc_t  __ina_malloc  = INA_MEM_MALLOC;
static ina_realloc_t __ina_realloc = INA_MEM_REALLOC;
static ina_free_t    __ina_free    = INA_MEM_FREE;
static ina_memmove_t __ina_memmove = INA_MEM_MEMMOVE;
static ina_memcpy_t  __ina_memcpy  = INA_MEM_MEMCPY;
static ina_memcmp_t  __ina_memcmp  = INA_MEM_MEMCMP;
static ina_memchr_t  __ina_memchr  = INA_MEM_MEMCHR;
static ina_memset_t  __ina_memset  = INA_MEM_MEMSET;


INA_API(void *) ina_mem_alloc(size_t size)
{
    return ina_mem_alloc_aligned(sizeof(void*), size);
}

INA_API(ina_rc_t) ina_mem_get_aligned_size(size_t query, size_t *aligned)
{
    INA_VERIFY_NOT_NULL(aligned);
    *aligned =  ((query+(INA_MEM_ALIGN_SIZE-1)) & (~(INA_MEM_ALIGN_SIZE-1)));
    return INA_SUCCESS;
}

INA_API(void *) ina_mem_alloc_aligned(size_t alignment, size_t size)
{
    /*
     * Same behavior as in c-runtime
     */
    if (size == 0) {
        ina_err_reset();
        return NULL;
    }

    if (alignment == 0) {
        INA_ERROR(INA_EINVAL);
        return NULL;
    }

    /* Allocate necessary memory area
     * client request - size parameter -
     * plus area to store the address
     * of the memory returned by standard
     * malloc().
     */
    void *p = __ina_malloc(size + alignment - 1 + sizeof(void*));
     
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
    INA_ERROR(INA_ENOMEM);
    return NULL;
}


INA_API(void) ina_mem_free(void *ptr)
{
    /* Get the address of the memory, stored at the
     * start of our total memory area. Alternatively,
     * you can use void *ptr = *((void **)p-1) instead
     * of the one below.
     */
    void *p = *((void**)((size_t)ptr - sizeof(void*)));
    __ina_free(p);
}

INA_API(void *) ina_mem_realloc(void *ptr, size_t nb)
{
    INA_VERIFY_NOT_NULL(ptr);
    return __ina_realloc(ptr, nb);
}

INA_API(void *) ina_mem_move(void *dest, const void *src, size_t nb)
{
    INA_VERIFY_NOT_NULL(dest);
    INA_VERIFY_NOT_NULL(src);
    return __ina_memmove(dest, src, nb);
}

INA_API(void *) ina_mem_cpy(void *dest, const void *src, size_t nb)
{
    INA_VERIFY_NOT_NULL(dest);
    INA_VERIFY_NOT_NULL(src);
    return __ina_memcpy(dest, src, nb);
}

INA_API(int) ina_mem_cmp(const void *lhs, const void *rhs, size_t nb)
{
    INA_VERIFY_NOT_NULL(lhs);
    INA_VERIFY_NOT_NULL(rhs);
    return __ina_memcmp(lhs, rhs, nb);
}

INA_API(void *) ina_mem_set(void *dest, int value, size_t nb)
{
    INA_VERIFY_NOT_NULL(dest);
    return __ina_memset(dest, value, nb);
}

INA_API(void *) ina_mem_chr(const void *dest, int value, size_t nb)
{
    INA_VERIFY_NOT_NULL(dest);
    return __ina_memchr(dest, value, nb);
}

INA_API(ina_rc_t) ina_mem_get_pagesize(size_t *size)
{
#ifndef INA_OS_WIN32
    INA_VERIFY_NOT_NULL(size);
    *size = (size_t)sysconf(_SC_PAGESIZE);
#else
    SYSTEM_INFO si;
    INA_VERIFY_NOT_NULL(size);
    GetSystemInfo(&si);
    *size = (size_t)si.dwPageSize;
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_mempool_init(void)
{
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_mempool_new(ina_mempool_t **pool, size_t size, uint32_t cf, const char *label)
{
    INA_VERIFY_NOT_NULL(pool);
    INA_VERIFY(size > 0);

    if (size < INA_MEM_MIN_POOL_SIZE) {
        size = INA_MEM_MIN_POOL_SIZE;
    }
    size = INA_MEM_ALIGN(size);

    *pool = (ina_mempool_t*)ina_mem_alloc(sizeof(ina_mempool_t));
    INA_RETURN_IF_NULL(*pool);
    ina_mem_set(*pool, 0, sizeof(ina_mempool_t));
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
    };
    if (cf&INA_MEM_SHARED) {
        INA_ASSERT_NOTNULL((*pool)->label);

        if (INA_FAILED((__ina_shm_open(*pool)))) {
            ina_str_free((*pool)->label);
            ina_mem_free(*pool);
            *pool = NULL;
            return ina_err_get_last_rc();
        }
    } else {
        (*pool)->shm_handle = 0;
        (*pool)->m = (unsigned char*)ina_mem_alloc(size);
         if (0 == (cf&INA_MEM_NOZEROFILL)) {
            ina_mem_set((*pool)->m, 0, (*pool)->size);
         }
    }

    if ((*pool)->m == NULL) {
        ina_mem_free(*pool);
        if ((*pool)->label != NULL) {
            ina_str_free((*pool)->label);
        }
        *pool = NULL;
        return INA_ERROR(INA_ENOMEM);
    }
    INA_TRACE3("New memory pool: %p->%p size = %ld", *pool, (*pool)->m, (*pool)->size);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_mempool_free(ina_mempool_t **pool)
{
    ina_mempool_t *pm;
    ina_mempool_t *pn;

    INA_VERIFY_NOT_NULL(pool);
    INA_VERIFY_NOT_NULL(*pool);

    /* Unlink parent */
    if ((*pool)->parent != NULL) {
        (*pool)->parent->child = NULL;
    }

    (*pool)->current = *pool;

    pn = *pool;
    while (pn != NULL) {
        pm = pn;
        if (pn != pn->child) {
            pn = pn->child;
        }
        if (pm->cf&INA_MEM_SHARED) {
            __ina_shm_close(pm);
        } else {
            ina_mem_free(pm->m);
        }
        ina_mem_free(pm);
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_mempool_shrink(ina_mempool_t *pool, size_t chunks, 
                                     ina_mempool_info_t *info)
{
    ina_mempool_t *pm;
    ina_mempool_t *pn;
    size_t c;
 
    INA_VERIFY_NOT_NULL(pool);
    INA_VERIFY_NOT_NULL(info);

    INA_RETURN_IF_FAILED(ina_mempool_getinfo(pool, info));

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
                ina_mem_free(pm->m);
            }
            ina_mem_free(pm);
        } else {
            pm->pos = 0;
        }
        --c;
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_mempool_clear(ina_mempool_t *pool)
{
    ina_mempool_t *pm;
    ina_mempool_t *pn;

    INA_VERIFY_NOT_NULL(pool);

    pn = pool;
    pm = NULL;
    while (pn != NULL) {
        pm = pn;
        if (pn != pn->child) {
            pn = pn->child;
        }
        pm->pos = 0;
        ina_mem_set(pm->m, 0, pm->size);
    }
    pool->current = pool;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_mempool_reset(ina_mempool_t *pool)
{
    ina_mempool_t *pm;
    ina_mempool_t *pn;
  
    INA_VERIFY_NOT_NULL(pool);

    pn = pool;
    pm = NULL;
    while (pn != NULL) {
        pm = pn;
        if (pn != pn->child) {
            pn = pn->child;
        }
        pm->pos = 0;
    }
    pool->current = pool;
    return INA_SUCCESS;
}


INA_API(ina_rc_t) ina_mempool_getinfo(ina_mempool_t *pool, ina_mempool_info_t *info)
{
    ina_mempool_t *pm;

    INA_VERIFY_NOT_NULL(pool);
    INA_VERIFY_NOT_NULL(info);

    pm = pool;

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

    INA_VERIFY_NOT_NULL(pool);
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
            if (INA_FAILED(ina_mempool_new(&pool->current->child, nsize,
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


INA_API(void *) ina_mempool_nalloc(ina_mempool_t *pool, size_t size)
{
    void *ret;

    INA_VERIFY_NOT_NULL(pool);
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

            /* FIXME: shm can not handled in chunks ! */
            ina_mempool_new(&pool->current->child, nsize,
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

    INA_VERIFY_NOT_NULL(pool);
    INA_VERIFY_NOT_NULL(old);
    INA_VERIFY(old_size > 0);
    INA_VERIFY(new_size > 0);
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
            /* FIXME: shm can not handled in chunks ! */
            ina_mempool_new(&pool->current->child, nsize,
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
        /* FIXME: shm can not handled in chunks ! */
        ina_mempool_new(&pool->current->child, nsize,
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
    return INA_SUCCESS;
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
        return INA_OS_ERROR(INA_NN_HANDLE|INA_ERR_INVALID);
    }

    if (pool->cf&INA_MEM_SHARED_EXCL) {
        if (ftruncate(pool->shm_handle, pool->size) == -1) {
            INA_OS_ERROR(INA_NN_OPERATION|INA_ERR_FAILED);
            close(pool->shm_handle);
            pool->shm_handle = 0;
            shm_unlink(ina_str_cstr(pool->label));
            return ina_err_get_last_rc();
       }
    }

    pool->m = (void *)mmap(NULL, pool->size, PROT_READ|PROT_WRITE, 
                        MAP_SHARED, 
                        pool->shm_handle, 0);

    if (pool->m == MAP_FAILED) {
        INA_OS_ERROR(INA_NN_OPERATION|INA_ERR_FAILED);
        close(pool->shm_handle);
        pool->shm_handle = 0;
        if (pool->cf&INA_MEM_SHARED_EXCL) {
            shm_unlink(ina_str_cstr(pool->label));
        }
        return ina_err_get_last_rc();
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
        INA_HIGH32(pool->size),
        INA_LOW32(pool->size),
        ina_str_cstr(pool->label));

    if (pool->shm_handle == NULL) {
        return INA_OS_ERROR(INA_NN_OPERATION|INA_ERR_FAILED);
    }
    pool->m = (void*)MapViewOfFile(pool->shm_handle,
        FILE_MAP_ALL_ACCESS, 
        0,
        0,
        pool->size);

    if (pool->m == NULL) {
        CloseHandle(pool->shm_handle);
        pool->shm_handle = NULL;
        return INA_OS_ERROR(INA_NN_OPERATION|INA_ERR_FAILED);
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
