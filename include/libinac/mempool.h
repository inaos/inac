/*
 * Copyright (c) 2012-2018 INAOS GmbH
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
#ifndef _LIBINAC_MEMPOOL_H_
#define _LIBINAC_MEMPOOL_H_

#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif
    
/* TODO: rename all constants to INA_MEMPOOL_.... */
#define INA_MEM_DFT_POOL_SIZE (8*1024*1204)
/* Minimal allowed pool size */
#define INA_MEM_MIN_POOL_SIZE (1024)
/* Single Pool, fixed size , no chunks */
#define INA_MEM_FIXED           (0)
/* Dynamic chunk allocation */
#define INA_MEM_DYNAMIC         (1)
/* Autosized chunk */
#define INA_MEM_AUTOSIZE        (2)
/* Fill chunks */
#define INA_MEM_BESTFIT         (4)
/* Child pool (internal used) */
#define INA_MEM_CHILD           (8)
/* Use shared memory */
#define INA_MEM_SHARED          (32)
/* Open or create shared memory */
#define INA_MEM_SHARED_CREATE   (64)
/* Shared memory owner */
#define INA_MEM_SHARED_OWNER    (128)
/* Open shared memory exclusive */
#define INA_MEM_SHARED_EXCL    (256)

/* Opaque emory pool handle */
typedef struct ina_mempool_s ina_mempool_t;

/* struct to hold pool information */
typedef struct ina_mempool_info_s {
    uint32_t cf;       /* creation flags */
    size_t size;       /* current size of all chunks */
    size_t used;       /* current used size incl. all chunks */
    size_t children;   /* number of chunks */
    size_t chunk_size; /* default chunks size */
} ina_mempool_info_t;

/* Memory pool events */
typedef enum ina_mempool_event_e {
    INA_MEMPOOL_EVENT_GROW,
    INA_MEMPOOL_EVENT_RELEASE,
    INA_MEMPOOL_EVENT_RELEASE_AND_DESTROY,
} ina_mempool_event_t;

/* struct to hold pool event info */
typedef struct ina_mempool_event_info_s {
    ina_mempool_event_t event;
    ina_mempool_t *pool;
    ina_mempool_info_t info;
} ina_mempool_event_info_t;

typedef ina_rc_t (*ina_mempool_event_handler_t)
        (const ina_mempool_event_info_t*, size_t*);

/*
 * Set custom allocator function to use with memory pools.
 * If NULL is given standard memmory handler will be used.
 *
 * This function should be called once and as soon as possible after 
 * ina_libinit() or ina_appinit().
 *
 * Parameters:
 *  malloc_fn    Pointer to the custom malloc() function
 *  free_fn      Pointer to the custom free() function
 *  realloc_fn   Pointer to the custom realloc() function
 *
 * Return
 *  INA_SUCCESS if no error occurred.
 */
INA_API(ina_rc_t) ina_mempool_set_fn(ina_malloc_t malloc_fn,
                                 ina_free_t free_fn,
                                 ina_realloc_t realloc_fn);

/* 
 * Initialize internal structures.
 *
 * Return
 *  INA_SUCCESS if all went well
 *  INA_FAILURE if an error  occurred
 */
INA_API(ina_rc_t) ina_mempool_init(void);

/*
 * Destroy all memory pools.
 *
 * Release and destroy all memory pools and internal structures. Once called, 
 * ina_mempool_init() must be called to reuse memory pools.
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_mempool_destroy(void);

/* 
 * Get runtime imformations about a memory pool.
 *
 * Parameters
 *  pool  Pointer to a memory pool, pass NULL to query system memory pool.
 *  info   Pointer to pool information structure.
 *
 * Return
 *  INA_SUCCESS if no error occurred.
 */
INA_API(ina_rc_t) ina_mempool_getinfo(ina_mempool_t *pool,
                                      ina_mempool_info_t *info);

/* 
 * Get a memory pool by label.
 *
 * Parameters
 *  label    Pool label.
 *  pool     Pointer to a memory pool pointer. Hold the memory pool.
 *
 * Return
 *  INA_SUCCESS if pool was found otherwise INA_FAILURE
 */
INA_API(ina_rc_t) ina_mempool_getbylabel(const char* label,
                                         ina_mempool_t **pool);

/* 
 * Get a memory pool by pointer.
 *
 * Parameters
 *  ptr   Pointer to find.
 *  pool  Pointer to a memory pool pointer. Hold the memory pool.
 *
 * Return
 *  INA_SUCCESS if pool was found otherwise INA_FAILURE
 */
INA_API(ina_rc_t) ina_mempool_getbypointer(const void *ptr,
                                           ina_mempool_t **pool);

/* 
 * Creates a memory pool.
 *
 * Parameters
 *  pool     Pointer to a memory pool pointer
 *  size     Size of memory pool in bytes.
 *  cf       Creation flags
 *  label    Pool label. Optional for non shared memory pools.
 *
 * Return
 *  INA_SUCCESS if pool was craeted successfully.
 */
INA_API(ina_rc_t) ina_mempool_new(ina_mempool_t **pool,
                                     size_t size,
                                     uint32_t cf,
                                     const char* label);

/* 
 * Free a memory pool.
 *
 * Parameters
 *  pool     Memory pool to free
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_mempool_free(ina_mempool_t *pool);

/*
 * Shrink a memory pool.
 *
 * Parameters
 *  pool    Memory too to shrink
 *  chunks  Number of chunk
 *  info    Optional. Where to store pool information after shrink
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_mempool_shrink(ina_mempool_t *pool,
                                     size_t chunks,
                                     ina_mempool_info_t *info);

/*
 * Reset a memory pool.
 *
 * Parameters
 *  pool  Memory pool to reset.
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_mempool_reset(ina_mempool_t *pool);

/*
 * Allocate reallocable memory from a pool.
 *
 * Parameters
 *  pool  Memory pool
 *  size  Number of bytes to allocate. Effective size may vary because memory
 *        will be allocated aligned.
 *
 * Return
 *   Pointer to the allocated memory that is suitably aligned for any
 *   kind of variable
 */
INA_API(void *)  ina_mempool_dalloc(ina_mempool_t *pool, size_t size);

/*
 * Allocate not reallocable memory from a pool.
 *
 * Parameters
 *  pool  Memory pool
 *  size  Number of bytes to allocate. Effective size may vary because memory
 *        will be allocated aligned.
 *
 * Return
 *   Pointer to the allocated memory that is suitably aligned for any
 *   kind of variable
 */
INA_API(void *)  ina_mempool_nalloc(ina_mempool_t *pool, size_t size);

/*
 * Reallocate memory from a pool
 *
 * Parameters
 *  pool      Memory pool
 *  old       Old pointer
 *  old_size  Old size in bytes
 *  new_size  New size in bytes
 *
 * Return
 *   Pointer to the allocated memory that is suitably aligned for any
 *   kind of variable
 */
INA_API(void *) ina_mempool_ralloc(ina_mempool_t *pool, void *old, size_t old_size, size_t new_size);

#ifdef __cplusplus
}
#endif 

#endif