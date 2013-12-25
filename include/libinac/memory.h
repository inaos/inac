/*
 * Copyright (c) 2012-2013 INAOS GmbH
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

#ifdef __cplusplus
extern "C" {
#endif
    
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


#define INA_MEM_DFT_POOL_SIZE (8*1024*1204)
/* Minimal allowed pool size */
#define INA_MEM_MIN_POOL_SIZE (1024)
/* Single Pool, fixed size */
#define INA_MEM_BASIC           (0)
/* Dynamic chunk allocation */
#define INA_MEM_DYNAMIC         (1)
/* Autosized chunk */
#define INA_MEM_AUTOSIZE        (2)
/* Fill chunks */
#define INA_MEM_BESTFIT         (4)
/* Child pool */
#define INA_MEM_CHILD           (8)
/* Use shared memory */
#define INA_MEM_SHARED          (32)
/* Open or create shared memory */
#define INA_MEM_SHARED_CREATE   (64)

/* Memory pool handle */
typedef struct ina_mempool_s  {
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
} ina_mempool_t;

/* struct to hold pool information */
typedef struct ina_mempool_info_s {
    size_t size;
    size_t used;
    size_t children; /* number of  pool */
} ina_mempool_info_t;


typedef struct ina_mempool_event_s {
    ina_mempool_t *pool;
} ina_mempool_event_t;

typedef ina_rc_t (*ina_mempool_event_handler)(ina_mempool_event_t*);
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
 * Move a memoy block.
 *
 * Copies the values of nb bytes from the location pointed by source to the 
 * memory block pointed by destination. Copying takes place as if an 
 * intermediate buffer were used, allowing the destination and source to  
 * overlap.
 * The underlying type of the objects pointed by both the source and 
 * destination pointers are irrelevant for this function; The result is a 
 * binary copy of the data.
 * The function does not check for any terminating null character in 
 * source - it always copies exactly nb bytes.
 *
 * To avoid overflows, the size of the arrays pointed by both the destination 
 * and source parameters, shall be at least nb bytes.
 *
 * Parameters
 * dest     Pointer to the destination array where the content is to be copied, 
 *          type-casted to a pointer of type void*
 * 
 * src      Pointer to the source of data to be copied, type-casted to a pointer 
 *          of type const void*.
 * nb       Number of bytes to copy. size_t is an unsigned integral type.
 *
 * Return Value
 * dest is returned
 */
INA_API(void *) ina_mem_move(void *dest, const void *src, size_t nb);
/* 
 * Copy block of memory
 *
 * Copies the values of nb bytes from the location pointed by source directly 
 * to the memory block pointed by destination.
 * 
 * The underlying type of the objects pointed by both the source and 
 * destination pointers are irrelevant for this function; The result is a 
 * binary copy of the data.
 * 
 * The function does not check for any terminating null character in 
 * source - it always copies exactly num bytes.
 *
 * To avoid overflows, the size of the arrays pointed by both the destination 
 * and source parameters, shall be at least nb bytes, and should not overlap 
 * (for overlapping memory blocks, memmove is a safer approach).
 *
 * Parameters
 * dest     Pointer to the destination array where the content is to be 
 *          copied, type-casted to a pointer of type void*.
 * source   Pointer to the source of data to be copied, type-casted to a 
 *          pointer of type const void*.
 * nb       Number of bytes to copy. size_t is an unsigned integral type.
 *
 * Return Value
 * dest is returned.
 */ 
INA_API(void *) ina_mem_cpy(void *dest, const void *src, size_t nb);

/*
 * Compare two blocks of memory
 *
 * Compares the first num bytes of the block of memory pointed by lhs to 
 * the first bn bytes pointed by rhs, returning zero if they all match or a 
 * value different from zero representing which is greater if they do not.
 * 
 * Notice that, unlike strcmp, the function does not stop comparing after 
 * finding a null character.
 *
 * Parameters
 * lhs      Pointer to block of memory.
 * rhs      Pointer to block of memory.
 * nb       Number of bytes to compare.
 *
 * Return Value
 * Returns an integral value indicating the relationship between the content 
 * of the memory blocks:
 * A zero value indicates that the contents of both memory blocks are equal.
 * A value greater than zero indicates that the first byte that does not 
 * match in both memory blocks has a greater value in lhs than in rhs as if 
 * evaluated as unsigned char values; And a value less than zero indicates 
 * the opposite.
 */
INA_API(int) ina_mem_cmp(const void *lhs, const void *rhs, size_t nb);
/*
 * Locate character in block of memory
 *
 * Searches within the first num bytes of the block of memory pointed by dest 
 * for the first occurrence of value (interpreted as an unsigned char), and 
 * returns a pointer to it.
 * 
 * Both value and each of the bytes checked on the the dest array are 
 * interpreted as unsigned char for the comparison.
 *
 * Parameters
 * dest     Pointer to the block of memory where the search is performed.
 * value    Value to be located. The value is passed as an int, but the 
 *          function performs a byte per byte search using the unsigned char
 *          conversion of this value.
 * nb       Number of bytes to be analyzed.
 *
 * Return Value
 * A pointer to the first occurrence of value in the block of memory pointed 
 * by dest.
 * If the value is not found, the function returns a null pointer.
 */
INA_API(void *) ina_mem_chr(const void *dest, int value, size_t nb);
/*
 * Fill block of memory
 *
 * Sets the first num bytes of the block of memory pointed by ptr to the 
 * specified value (interpreted as an unsigned char).
 * 
 * Parameters
 * dest     Pointer to the block of memory to fill.
 * value    Value to be set. The value is passed as an int, but the function 
 *          fills the block of memory using the unsigned char conversion of 
 *          this value.
 * nb       Number of bytes to be set to the value.
 *
 * Return Value
 * dest is returned.
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
 * The function returns the number of bytes in a memory page, where "page" is 
 * a fixed-length block, the unit for memory allocation and file mapping.
 *
 * Parameters:
 * size     Size in bytes
 *
 * Return Value
 * INA_SUCCESS if no error occurred.
 */                                 
INA_API(ina_rc_t) ina_mem_get_pagesize(size_t *size);

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

/* 
 * Initalize internal structures an allocate the internal memory pool. This
 * system pool will automatically increase his size if needed.
 * Parameters
 * size     Initial size in bytes
 *
 * Return Value
 * INA_SUCCESS when the system memory pool was succefully allocated.
 * INA_FAILURE if an error occured
 */
INA_API(ina_rc_t) ina_mempool_init(size_t size);

/*
 * Destory all memory pools.
 *
 * Release and destroy all memory pools and internal structures. Once called, 
 * ina_mempool_init() must be called to reuse memory pools.
 *
 * Return Value
 * INA_SUCCESS
 */
INA_API(ina_rc_t) ina_mempool_destroy(void);

/* 
 * Get runtime imformations about a memory pool,.
 *
 * pool     Pointer to a memory pool, pass NULL to query system memory pool.
 * info     Pointer to pool information structure.
 *
 * Return Value
 * INA_SUCCESS if no error occured.
 */
INA_API(ina_rc_t) ina_mempool_getinfo(ina_mempool_t *pool, ina_mempool_info_t *info);

/* 
 * Get a memory pool by label.
 *
 * Parameters
 * label    Pool label.
 * pool     Pointer to a memory pool pointer. Hold the memory pool.
 *
 * Return Value
 * INA_SUCCESS if pool was found otherwise INA_FAILURE
 */
INA_API(ina_rc_t) ina_mempool_getbylabel(const char* label, ina_mempool_t **pool);

/* 
 * Get a memory pool by pointer.
 *
 * Parameters
 * ptr      Pointer to find.
 * pool     Pointer to a memory pool pointer. Hold the memory pool.
 *
 * Return Value
 * INA_SUCCESS if pool was found otherwise INA_FAILURE
 */
INA_API(ina_rc_t) ina_mempool_getbypointer(const char *ptr, ina_mempool_t **pool);

/* 
 * Create a memory pool.
 *
 * Parameters
 * pool     Pointer to a memory pool pointer
 * size     Size of memory pool in bytes.
 * cf       Creation flags
 * label    Pool label. Optional for non shared memory pools.
 *
 * Return Value
 * INA_SUCCESS if pool was craeted successfully.
 */
INA_API(ina_rc_t) ina_mempool_create(ina_mempool_t **pool, size_t size, uint32_t cf, ina_str_t label);

/* 
 * Release pool memory.
 */
INA_API(ina_rc_t) ina_mempool_release(ina_mempool_t *pool, int destroy);

/*
 * Allocate reallocable memory from a pool 
 */
INA_API(void *)  ina_mempool_dalloc(ina_mempool_t *pool, size_t size);
/* 
 * Allocate not reallocable memory from a pool
 */
INA_API(void *)  ina_mempool_nalloc(ina_mempool_t *pool, size_t size);
/* 
 * Reallocate memory from a pool 
 */
INA_API(void *) ina_mempool_ralloc(ina_mempool_t *pool, void *old, size_t old_size, size_t new_size);

#ifdef __cplusplus
}
#endif 

#endif