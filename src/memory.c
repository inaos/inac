/*
 * Copyright INAOS GmbH, Thalwil, 2012-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <sys/stat.h>
#include <libinac/lib.h>
#include "config.h"

typedef  struct ina_mempool_chunk_s {
    size_t size;
    size_t pos;
    size_t end;
    ina_list_node_t node;
    unsigned char* m;
} ina_mempool_chunk_t;

struct ina_mempool_s  {
    ina_handle_t shm_handle;
    uint32_t cf;
    ina_str_t label;
    ina_mempool_chunk_t* current;
    ina_list_t *chunks;
    ina_list_node_t node;
};


static ina_list_t *__pools = NULL;

static ina_rc_t __ina_shm_open(ina_mempool_t *);
static ina_rc_t __ina_shm_close(ina_mempool_t *);

static ina_rc_t __ina_free_pool(void *data)
{
    ina_mempool_t *pool = (ina_mempool_t*)data;
    ina_mempool_free(&pool);
    return INA_SUCCESS;
}

static ina_rc_t __ina_free_chunk(void *data, void *arg)
{
    ina_mempool_chunk_t* chunk = (ina_mempool_chunk_t*)data;
    ina_mempool_t *pool = (ina_mempool_t*)arg;
    if (pool->shm_handle) {
        __ina_shm_close(pool);
        pool->shm_handle = 0;
    } else {
        ina_mem_free(chunk->m);
    }
    ina_mem_free(chunk);
    return INA_SUCCESS;
}

static ina_rc_t __ina_chunk_info(void *data, void *arg)
{
    ina_mempool_chunk_t *chunk = (ina_mempool_chunk_t*)data;
    ina_mempool_info_t *info = (ina_mempool_info_t*)arg;
    info->size += chunk->size;
    info->used +=chunk->pos + (chunk->size-chunk->end);
    ++info->children;
    return INA_SUCCESS;
}

static ina_rc_t __ina_chunk_clear(void *data)
{
    ina_mempool_chunk_t *chunk = (ina_mempool_chunk_t*)data;
    chunk->pos = 0;
    ina_mem_set(chunk->m, 0, chunk->size);
    return INA_SUCCESS;
}

static ina_rc_t __ina_chunk_reset(void *data)
{
    ina_mempool_chunk_t *chunk = (ina_mempool_chunk_t*)data;
    chunk->pos = 0;
    return INA_SUCCESS;
}

INA_API(void *) ina_mem_alloc_aligned(size_t alignment, size_t size)
{
    /*
     * Same behavior as in c-runtime
     */
    if (INA_UNLIKELY(size == 0)) {
        ina_err_reset();
        return NULL;
    }

    if (INA_UNLIKELY(alignment == 0)) {
        INA_ERROR(INA_ERR_INVALID_ARGUMENT);
        return NULL;
    }

    /* Allocate necessary memory area
     * client request - size parameter -
     * plus area to store the address
     * of the memory returned by standard
     * malloc().
     */
    void *p = INA_MEM_MALLOC(size + alignment - 1 + sizeof(void*));
     
    if (INA_UNLIKELY(p != NULL)) {
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
    INA_ERROR(INA_ERR_OUT_OF_MEMORY);
    return NULL;
}

INA_API(void*) ina_mem_realloc(void *ptr, size_t nb)
{
    void *p;
    INA_ASSERT_NOT_NULL(ptr);
    p = *((void**)((size_t)ptr - sizeof(void*)));

    if (nb == 0) {
        ina_mem_free(ptr);
        return NULL;
    }

    p = INA_MEM_REALLOC(p, nb+ sizeof(void*) - 1 + sizeof(void*));
    ptr = (void*) (((size_t)p + sizeof(void*) + sizeof(void*) -1) & ~(sizeof(void*)-1));
    *((void**)((size_t)ptr - sizeof(void*))) = p;
    return ptr;
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
    INA_INIT_GUARD();
    INA_RETURN_IF_FAILED(ina_list_new(INA_LIST_CF_NOMALLOC, &__pools));
    return INA_SUCCESS;
}

INA_API(void) ina_mempool_destroy(void)
{
    INA_DESTROY_GUARD();
    ina_list_foreach(__pools, __ina_free_pool);
    ina_list_free(&__pools);
}

INA_INLINE ina_mempool_chunk_t*  __ina_add_chunk(ina_mempool_t *pool, size_t size)
{
    ina_mempool_chunk_t *chunk = ina_mem_alloc(size);
    chunk->pos = 0;
    chunk->size = size;
    chunk->end = size;
    chunk->node.data = chunk;

    if (pool->cf^INA_MEM_SHARED) {
        chunk->m = (unsigned char *) ina_mem_alloc(size);
        if (0 == (pool->cf & INA_MEM_NOZEROFILL)) {
            ina_mem_set(chunk->m, 0, chunk->size);
        }
    }
    ina_list_insert_tail(pool->chunks, &chunk->node);
    return chunk;
}


INA_API(ina_rc_t) ina_mempool_new(size_t size, const char *label, uint32_t cf, ina_mempool_t **pool)
{
    INA_VERIFY_NOT_NULL(pool);
    INA_VERIFY(size > 0);

    if (size < INA_MEM_MIN_POOL_SIZE) {
        size = INA_MEM_MIN_POOL_SIZE;
    }
    size = INA_MEM_ALIGN(size);

    *pool = (ina_mempool_t*)ina_mem_alloc(sizeof(ina_mempool_t));
    INA_RETURN_IF_NULL(*pool);
    INA_MEM_SET_ZERO(*pool, ina_mempool_t);
    (*pool)->cf = cf;

    if (label != NULL) {
        (*pool)->label = ina_str_new_fromcstr(label);
    }

    INA_FAIL_IF_ERROR(ina_list_new(INA_LIST_CF_NOMALLOC, &(*pool)->chunks));

    (*pool)->current = __ina_add_chunk(*pool, size);
    INA_FAIL_IF((*pool)->current == NULL);
    if ((*pool)->cf&INA_MEM_SHARED) {
        INA_FAIL_IF_ERROR(__ina_shm_open(*pool));
    }
    (*pool)->node.data = *pool;
    ina_list_insert_tail(__pools, &(*pool)->node);
    return INA_SUCCESS;

fail:
    ina_mempool_free(pool);
    return ina_err_get_rc();
}

INA_API(void) ina_mempool_free(ina_mempool_t **pool)
{
    INA_VERIFY_FREE(pool);
    ina_list_foreach_arg((*pool)->chunks, __ina_free_chunk, *pool);
    INA_MUST_SUCCEED(ina_list_remove(__pools, &(*pool)->node));
    ina_list_free(&(*pool)->chunks);
    ina_str_free((*pool)->label);
    INA_MEM_FREE_SAFE(*pool);
}

INA_API(ina_rc_t) ina_mempool_merge(ina_mempool_t *dest, ina_mempool_t *src)
{
    INA_VERIFY_NOT_NULL(dest);

    if (src == NULL) {
        return INA_SUCCESS;
    }
    if (dest->cf&INA_MEM_SHARED || src->cf&INA_MEM_SHARED) {
        return INA_ERROR(INA_ES_OPERATION | INA_ERR_INVALID);
    }
    ina_list_concat(dest->chunks, src->chunks);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_mempool_shrink(ina_mempool_t *pool, size_t chunks, 
                                     ina_mempool_info_t *info)
{
    ina_list_node_t *prev;
    ina_mempool_chunk_t *chunk;
    size_t c;
 
    INA_VERIFY_NOT_NULL(pool);
    INA_VERIFY(chunks > 0);
    INA_VERIFY(0 == (pool->cf&INA_MEM_SHARED));
    INA_VERIFY_NOT_NULL(info);
    INA_RETURN_IF_FAILED(ina_mempool_info(pool, info));

    c = info->children;
    ina_list_tail(pool->chunks, &prev);

    while (prev && c > chunks) {
        chunk = (ina_mempool_chunk_t*)prev->data;
        prev = prev->prev;
        chunk->pos = 0;
        if (chunk->end == chunk->size) {
            ina_list_remove(pool->chunks, &chunk->node);
            __ina_free_chunk(chunk, pool);
            --c;
        }
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_mempool_clear(ina_mempool_t *pool)
{
    ina_list_node_t* head;

    INA_VERIFY_NOT_NULL(pool);
    ina_list_foreach(pool->chunks, __ina_chunk_clear);
    ina_list_head(pool->chunks, &head);
    pool->current = (ina_mempool_chunk_t*)head->data;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_mempool_reset(ina_mempool_t *pool)
{
    ina_list_node_t* head;

    INA_VERIFY_NOT_NULL(pool);
    ina_list_foreach(pool->chunks, __ina_chunk_reset);
    ina_list_head(pool->chunks, &head);
    pool->current = (ina_mempool_chunk_t*)head->data;
    return INA_SUCCESS;
}


INA_API(ina_rc_t) ina_mempool_info(ina_mempool_t *pool, ina_mempool_info_t *info)
{
    INA_VERIFY_NOT_NULL(pool);
    INA_VERIFY_NOT_NULL(info);

    INA_MEM_SET_ZERO(info, ina_mempool_info_t);
    info->cf = pool->cf;
    ina_list_foreach_arg(pool->chunks, __ina_chunk_info, info);
    --info->children;
    return INA_SUCCESS;
}

INA_API(void *) ina_mempool_dalloc(ina_mempool_t *pool, size_t size)
{
    void *ret;

    INA_ASSERT_NOT_NULL(pool);
    INA_ASSERT_NOT_NULL(pool->current);

    if (pool->cf^INA_MEM_BESTFIT) {
        size = INA_MEM_ALIGN(size);
        /*INA_TRACE("dalloc bytes: %d", size);*/
    }

    if ((pool->current->pos + size > pool->current->end) || 
        (pool->current->pos + size < pool->current->pos)) {
        if (pool->cf&INA_MEM_DYNAMIC) {
            size_t new_size;
            if (pool->cf&INA_MEM_AUTOSIZE || size > pool->current->size) {
                new_size = size;
            } else {
                new_size = pool->current->size;
            }
            pool->current = __ina_add_chunk(pool, new_size);
        } else {
            INA_ERROR(INA_ERR_POOL_FULL);
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

    INA_ASSERT_NOT_NULL(pool);
    INA_ASSERT_NOT_NULL(pool->current);
    ret = NULL;

    if (pool->cf^INA_MEM_BESTFIT) {
        size = INA_MEM_ALIGN(size);
    }

     /* bogus request */
    if (pool->current->end < size) {
        INA_ERROR(INA_ES_SIZE | INA_ERR_INVALID);
        return NULL;
    }

    if ((pool->current->pos + size > pool->current->end) ||
        (pool->current->pos + size < pool->current->pos)) {
        if (pool->cf&INA_MEM_DYNAMIC) {
            size_t new_size;
            if (pool->cf&INA_MEM_AUTOSIZE || size > pool->current->size) {
                new_size = size;
            } else {
                new_size = pool->current->size;
            }
            pool->current = __ina_add_chunk(pool, new_size);
        } else {
            INA_ERROR(INA_ERR_POOL_FULL);
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

    INA_ASSERT_NOT_NULL(pool);
    INA_ASSERT_NOT_NULL(old);
    INA_ASSERT(old_size > 0);
    INA_ASSERT(new_size > 0);
    INA_ASSERT_NOT_NULL(pool->current);


    if (pool->cf^INA_MEM_BESTFIT) {
        new_size = INA_MEM_ALIGN(new_size);
        old_size = INA_MEM_ALIGN(old_size);
    }

     /* bogus request */
    if (pool->current->end < old_size) {
        INA_ERROR(INA_ES_SIZE | INA_ERR_INVALID);
        return NULL;
    }
    /* was the previous allocation - optimize! */
    if ((pool->current->pos >= old_size) && (&pool->current->m[pool->current->pos - old_size] == old)) {
        /* fits */
         if (pool->current->pos + new_size - old_size <= pool->current->end) {
            /* shrinking - zero again! */
            pool->current->pos += new_size - old_size;
            if (new_size < old_size) {
                ina_mem_set(&pool->current->m[pool->current->pos], 0, old_size - new_size);
            }
            return old;
        }
        /* does not fit */
        if (pool->cf&INA_MEM_DYNAMIC) {
            size_t nsize = 0;
 
            if (pool->cf&INA_MEM_AUTOSIZE || new_size > pool->current->size) {
                nsize = new_size;
            } else {
                nsize = pool->current->size;
            }
            pool->current = __ina_add_chunk(pool, nsize);

            ret = &pool->current->m[pool->current->pos];
            ina_mem_cpy(ret, old, old_size);
            pool->current->pos += new_size;
            return ret;
        }
        INA_ERROR(INA_ERR_OUT_OF_MEMORY);
        return NULL;
    }
    /* cannot shrink, we need to move */
    if (new_size <= old_size) {
        return old;
    }
    /* fits */
    if ((pool->current->pos + new_size >= pool->current->pos) &&
        (pool->current->pos + new_size <= pool->current->end))
    {
        ret = &pool->current->m[pool->current->pos];
        ina_mem_cpy(ret, old, old_size);
        pool->current->pos += new_size;
        return ret;
    }
    if (pool->cf&INA_MEM_DYNAMIC) {
        size_t nsize = 0;

        if (pool->cf&INA_MEM_AUTOSIZE || new_size > pool->current->size) {
            nsize = new_size;
        } else {
            nsize = pool->current->size;
        }
        pool->current = __ina_add_chunk(pool, nsize);
        ret = &pool->current->m[pool->current->pos];
        ina_mem_cpy(ret, old, old_size);
        pool->current->pos += new_size;
        return ret;
    }
    INA_ERROR(INA_ERR_POOL_FULL);
    return NULL;
}


#ifndef INA_OS_WIN32

static ina_rc_t 
__ina_shm_open(ina_mempool_t *pool)
{
    int flags;

    INA_ASSERT_NOT_NULL(pool);
    INA_ASSERT_NOT_NULL(pool->label);
    INA_ASSERT(pool->current->size > 0);
    INA_ASSERT(pool->cf&INA_MEM_SHARED);
    INA_ASSERT_NULL(pool->current->m);

    pool->current->size = INA_MEM_ALIGN(pool->current->size+sizeof(int64_t));
    pool->current->end = pool->current->size;

    flags = O_RDWR;
    if (pool->cf&INA_MEM_SHARED_CREATE) {
        flags |=  O_CREAT;
        if (pool->cf&INA_MEM_SHARED_EXCL) {
            flags |= O_EXCL;
        }
    }

    pool->shm_handle = shm_open(ina_str_cstr(pool->label), flags, S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP | S_IWOTH | S_IROTH);
    if (pool->shm_handle == -1) {
        return INA_OS_ERROR(INA_ES_HANDLE | INA_ERR_INVALID);
    }

    if (pool->cf&INA_MEM_SHARED_EXCL) {
        if (ftruncate(pool->shm_handle, pool->current->size) == -1) {
            INA_OS_ERROR(INA_ES_OPERATION | INA_ERR_FAILED);
            close(pool->shm_handle);
            pool->shm_handle = 0;
            shm_unlink(ina_str_cstr(pool->label));
            return ina_err_get_rc();
       }
    }

    pool->current->m = (void *)mmap(NULL, pool->current->size, PROT_READ|PROT_WRITE,
                        MAP_SHARED, 
                        pool->shm_handle, 0);

    if (pool->current->m == MAP_FAILED) {
        INA_OS_ERROR(INA_ES_OPERATION | INA_ERR_FAILED);
        close(pool->shm_handle);
        pool->shm_handle = 0;
        if (pool->cf&INA_MEM_SHARED_EXCL) {
            shm_unlink(ina_str_cstr(pool->label));
        }
        return ina_err_get_rc();
    }
    /* Inc ref count */
    __sync_fetch_and_add((int64_t*)pool->current->m, 1);
    /* Inc start pos */
    pool->current->pos += sizeof(int64_t);
    INA_TRACE2("shared mem %s ref count =  %" INA_INT64_T_FMT, pool->label, *(int64_t*)pool->m);
    return INA_SUCCESS;
}

static ina_rc_t 
__ina_shm_close(ina_mempool_t *pool)
{
    INA_ASSERT_NOT_NULL(pool);
    INA_ASSERT(pool->current->size > 0);
    INA_ASSERT_NOT_NULL(pool->label);
    int64_t cn;

    if (pool->current->m == NULL) {
         return INA_SUCCESS;
    }

    /* Dec an get ref count before unmap memory */
    cn = __sync_sub_and_fetch((int64_t*)pool->current->m, 1);
    
    /* Unmap memory */
    munmap(pool->current->m, pool->current->size);
    pool->current->m = NULL;
    pool->current->size = 0;
    pool->current->pos = 0;
    pool->current->end = 0;

    /* Close shared mem */
    close(pool->shm_handle);

    /* Dec ref count, unlink on last relase */
    if (cn == 0 || pool->cf&INA_MEM_SHARED_EXCL) {
        INA_TRACE2("unlinking shared mem %s", pool->label);
        shm_unlink(ina_str_cstr(pool->label));
    }
    INA_TRACE2("shared mem %s ref count =  %" INA_INT64_T_FMT, pool->label, cn);

    return INA_SUCCESS;
}
#else
static ina_rc_t 
__ina_shm_open(ina_mempool_t *pool)
{
	INA_ASSERT_NOT_NULL(pool);
    INA_ASSERT_NOT_NULL(pool->label);
    INA_ASSERT(pool->current->size > 0);
    INA_ASSERT(pool->cf&INA_MEM_SHARED);
    INA_ASSERT_NULL(pool->current->m);

    pool->shm_handle = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        INA_HIGH32(pool->current->size),
        INA_LOW32(pool->current->size),
        ina_str_cstr(pool->label));

    if (pool->shm_handle == NULL) {
        return INA_OS_ERROR(INA_ES_OPERATION|INA_ERR_FAILED);
    }
    pool->current->m = (void*)MapViewOfFile(pool->shm_handle,
        FILE_MAP_ALL_ACCESS, 
        0,
        0,
        pool->current->size);

    if (pool->current->m == NULL) {
        CloseHandle(pool->shm_handle);
        pool->shm_handle = NULL;
        return INA_OS_ERROR(INA_ES_OPERATION|INA_ERR_FAILED);
    }
    return INA_SUCCESS;
}

static ina_rc_t 
__ina_shm_close(ina_mempool_t *pool)
{
    INA_ASSERT_NOT_NULL(pool);
    INA_ASSERT(pool->current->size > 0);
    INA_ASSERT_NOT_NULL(pool->label);
    INA_ASSERT_NOT_NULL(pool->shm_handle);

    if (pool->current->m == NULL) {
         return INA_SUCCESS;
    }

    /* TODO: Error handling */
    UnmapViewOfFile(pool->shm_handle);
    CloseHandle(pool->shm_handle);

    pool->current->m = NULL;
    pool->shm_handle = NULL;
    pool->current->size = 0;
    pool->current->pos = 0;
    pool->current->end = 0;
    
    return INA_SUCCESS;
}
#endif
