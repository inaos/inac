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
#ifndef _LIBINAC_MEMORY_H_
#define _LIBINAC_MEMORY_H_

/* Single Pool, fixed size */
#define INA_MEM_BASIC       0
/* Dynamic chunk allocation */
#define INA_MEM_DYNAMIC     1
/* Autosized chunk */
#define INA_MEM_AUTOSIZE    2
/* Fill chunks */
#define INA_MEM_BESTFIT     4
/* Zero fill on allocation */
#define INA_MEM_FILLZERO   16
/* Use shared memory */
#define INA_MEM_SHARED        32
#define INA_MEM_SHARED_CREATE 64

/* Memory pool handle */
typedef struct ina_mempool_s  {
    ina_shm_handle_t shm_handle;
    uint32_t cf;
    size_t size;
    size_t pos;
    size_t end;
    void *m;
    ina_str_t label;
    struct ina_mempool_s *current;
    struct ina_mempool_s *parent;
    struct ina_mempool_s *child;
} ina_mempool_t;

/* struct to hold pool information */
typedef struct ina_mempool_info_s {
    size_t size;
    size_t used;
    int    children; /* number of  pool */
} ina_mempool_info_t;

/* Function pointer with malloc()‘s signature */
typedef void *(*ina_malloc_t)(size_t);
/* Function pointer with realloc()‘s signature */
typedef void *(*ina_realloc_t)(void *, size_t);
/* Function pointer with memmove()‘s signature */
typedef void *(*ina_memmove_t)(void *, const void *, size_t);
/* Function pointer with memcpy()‘s signature */
typedef void *(*ina_memcpy_t)(void *, const void *, size_t);
/* Function pointer with memcmp()‘s signature */
typedef int (*ina_memcmp_t)(const void *, const void *, size_t);
/* Function pointer with memchr()‘s signature */
typedef void *(*ina_memchr_t) (const void *, int , size_t);
/* Function pointer with memset()‘s signature */
typedef void *(*ina_memset_t)(void *, int , size_t);
/* Function pointer with free()‘s signature */
typedef void (*ina_free_t)(void *);

/*
 * Allocate memory block. Allocates a block of size bytes of memory, returning
 * a pointer to the beginning of the block.
 *
 * The content of the newly allocated block of memory is not initialized, 
 * remaining with indeterminate values.
 *
 * If size is zero, it returns a null pointer. But the returned 
 * pointer shall not be used to dereference an object in any case.
 *
 * Parameters
 * size   Size of the memory block, in bytes. size_t is an unsigned integral 
 *        type.
 * 
 * Return Value
 * On success, a pointer to the memory block allocated by the function.
 * The type of this pointer is always void*, which can be cast to the desired
 * type of data pointer in order to be dereferenceable.
 * If the function failed to allocate the requested block of memory, 
 * a null pointer is returned.
 */
INA_API(void *) ina_mem_alloc(size_t size);
/*
 * TODO: documentation
 */
INA_API(void *) ina_mem_realloc(void *ptr, size_t nb);
/*
 * TODO: documentation
 */
INA_API(void *) ina_mem_move(void *dest, const void *src, size_t nb);
/*
 * TODO: documentation
 */
INA_API(void *) ina_mem_cpy(void *dest, const void *src, size_t nb);
/*
 * TODO: documentation
 */
INA_API(int) ina_mem_cmp(const void *lhs, const void *rhs, size_t nb);
/*
 * TODO: documentation
 */
INA_API(void *) ina_mem_chr(const void *dest, int value, size_t nb);
/*
 * TODO: documentation
 */
INA_API(void *) ina_mem_set(void *dest, int value, size_t nb);

/* 
 * Deallocate space in memory. A block of memory previously allocated using a 
 * call to malloc, calloc or realloc is deallocated, making it available again
 * for further allocations.
 *
 * If ptr does not point to a block of memory allocated with the above 
 * functions, the behavior is undefined.
 *
 * If ptr is a null pointer, the function does nothing.
 *
 * Notice that this function does not change the value of ptr itself, hence it
 * still points to the same (now invalid) location.
 *
 * Parameters
 * ptr   pointer to a memory block prevously allocated with ina_mem_alloc()
 *
 * Return Value
 * none
 */
INA_API(void) ina_mem_free(void *ptr);

/*
 * Set custom memory function.
 * If NULL is given standard memmory handler will be used.
 *
 * This function should be called once and as soon as possible after 
 * ina_libinit() or ina_appinit().
 *
 * Parameters:
 * malloc_fn     Pointer to the custom malloc() function
 * free_fn       Pointer to the custom free() function
 * realloc_fn    Pointer to the custom realloc() function
 * memmove_fn    Pointer to the custom memmove() function
 * memcpy_fn     Pointer to the custom memcpy() function
 * memcmp_fn     Pointer to the custom memcmp() function
 * memchr_fn     Pointer to the custom memchr() function
 * memset_fn     Pointer to the custom memset() function
 *
 * Return Value
 * INA_SUCCESS if no error occured.
 */
INA_API(ina_rc_t) ina_mem_set_fn(ina_malloc_t malloc_fn, 
                                 ina_free_t free_fn,
                                 ina_realloc_t realloc_fn,
                                 ina_memmove_t memmove_fn,
                                 ina_memcpy_t memcpy_fn,
                                 ina_memcmp_t memcmp_fn,
                                 ina_memchr_t memchr_fn,
                                 ina_memset_t memset_fn);
/*
 * Set custom allocator function to use with memory pools.
 * If NULL is given standard memmory handler will be used.
 *
 * This function should be called once and as soon as possible after 
 * ina_libinit() or ina_appinit().
 *
 * Parameters:
 * malloc_fn     Pointer to the custom malloc() function
 * free_fn       Pointer to the custom free() function
 * realloc_fn    Pointer to the custom realloc() function
 *
 * Return Value
 * INA_SUCCESS if no error occured.
 */
INA_API(ina_rc_t) ina_mempool_set_fn(ina_malloc_t malloc_fn,
                                 ina_free_t free_fn,
                                 ina_realloc_t realloc_fn);

/* initalize internal structures . */
INA_API(ina_rc_t) ina_mempool_init(size_t size);
/* cleanup */
INA_API(ina_rc_t) ina_mempool_destroy(void);
/* informationen abrufen */
INA_API(ina_rc_t) ina_mempool_getinfo(ina_mempool_t *pool, ina_mempool_info_t *info);
/* create a memory pool. */
INA_API(ina_rc_t) ina_mempool_create(ina_mempool_t **pool, size_t size, uint32_t cf, ina_str_t label);
/* destroy a memory pool and release allocated memory */
INA_API(ina_rc_t) ina_mempool_release(ina_mempool_t *pool, int destroy);
/* reset a memory pool, memory still allocated */
INA_API(ina_rc_t) ina_mempool_reset(ina_mempool_t *pool, size_t size);
/* allocate reallocable memory from a pool */
INA_API(void *)  ina_mempool_dalloc(ina_mempool_t *pool, size_t size);
/* allocate not reallocable memory from a pool */
INA_API(void *)  ina_mempool_nalloc(ina_mempool_t *pool, size_t size);
/* reallocate memory from a pool */
INA_API(void *) ina_mempool_ralloc(ina_mempool_t *pool, void *old, size_t pnb, size_t nnb);

#endif