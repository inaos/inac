/*
 * Copyright INAOS GmbH, Thalwil, 2012-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#ifndef _LIBINAC_MEMPOOL_H_
#define _LIBINAC_MEMPOOL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <libinac/lib.h>

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
/* Use shared memory */
#define INA_MEM_SHARED          (32)
/* Open or create shared memory */
#define INA_MEM_SHARED_CREATE   (64)
/* Shared memory owner */
#define INA_MEM_SHARED_OWNER    (128)
/* Open shared memory exclusive */
#define INA_MEM_SHARED_EXCL    (256)
/* Do not fill zero on creation */
#define INA_MEM_NOZEROFILL     (512)

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

/*
 * Initialized memory pool module
 *
 * Return
 *  INA_SUCCES if all went well
 */
INA_API(ina_rc_t) ina_mempool_init(void);

/*
 * Destroy memory pool module.
 */
INA_API(void) ina_mempool_destroy(void);

/*
 * Get runtime information about a memory pool.
 *
 * Parameters
 *  pool  Pointer to a memory pool, pass NULL to query system memory pool.
 *  info   Pointer to pool information structure.
 *
 * Return
 *  INA_SUCCESS if no error occurred.
 */
INA_API(ina_rc_t) ina_mempool_info(ina_mempool_t *pool,
                                   ina_mempool_info_t *info);

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
 *  INA_SUCCESS if pool was created successfully.
 */
INA_API(ina_rc_t) ina_mempool_new(size_t size, const char *label, uint32_t cf, ina_mempool_t **pool);

/* 
 * Free a memory pool.
 *
 * Parameters
 *  pool     Memory pool to free
 */
INA_API(void) ina_mempool_free(ina_mempool_t **pool);

/*
 * Merge a memory pools.
 *
 * Parameters
 *  dest  Destination
 *  src   Source, pool that will be merged into dest. After this call the src
 *        content is undefined and you should not use it anymore.
 *
 * Return
 *  INA_SUCCESS if all went well
 *  INA_EOP     if trying to merge shared memory pool
 */
INA_API(ina_rc_t) ina_mempool_merge(ina_mempool_t *dest, ina_mempool_t *src);

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
 * Clear a memory pool, fill all chunks with 0.
 *
 * Parameters
 *  pool  Memory pool to clear.
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_mempool_clear(ina_mempool_t *pool);

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