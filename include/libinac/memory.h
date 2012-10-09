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

/* Memmory pool handle */
typedef struct ina_mempool_s ina_mempool_t;

/* Function pointer with malloc()‘s signature */
typedef void *(*ina_malloc_t)(size_t);

/* Function pointer with free()‘s signature */
typedef void (*ina_free_t)(void *);

/**
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
* size   Size of the memory block, in bytes.size_t is an unsigned integral 
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

/**
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

/**
* Set custom memory allocation function.  Overrride stabdard malloc() and
* free().  If NULL ist given standard memmory handler will be used.
*
* This function should be call once and as soon as possible after 
* ina_libinit() or ini_appinit().
*
* Parameters:
* malloc_fn     Pointer to the custom malloc()-function
* free_fn       Pointer to the custom free()-fucntion
*
* Return Value
* INA_SUCCESS if no error occured.
*/
INA_API(ina_rc_t) ina_mem_set_alloc(ina_malloc_t malloc_fn, ina_free_t free_fn);

/* initalize internal structures . */
INA_API(ina_rc_t) ina_mempool_init(ina_malloc_t malloc_fn, ina_free_t free_fn);    

/* create a memory pool. */
INA_API(ina_rc_t) ina_mempool_create(ina_mempool_t **pool);
/* destroy a memory pool and release allocated memory */
INA_API(ina_rc_t) ina_mempool_destroy(ina_mempool_t *pool);
/* reset a memory pool, memory still allocated */
INA_API(ina_rc_t) ina_mempool_reset(ina_mempool_t *pool);
/* allocate memory from a pool */
INA_API(void *)   ina_mempool_palloc(ina_mempool_t *pool, size_t size);
/* release prevously allocated memory. */ 
INA_API(ina_rc_t) ina_mempool_pfree(ina_mempool_t *pool, void *p);

#endif