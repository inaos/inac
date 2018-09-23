/*
 * Copyright INAOS GmbH, Thalwil, 2012-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#ifndef _LIBINAC_MEMORY_H_
#define _LIBINAC_MEMORY_H_


#ifdef __cplusplus
extern "C" {
#endif
#include <libinac/lib.h>

#define INA_ENOMEM   INA_NN_MEMORY|INA_ERR_OUT_OF


	/* Align to 2x word size (as GNU libc does). */
#define INA_MEM_ALIGN_SIZE (2 * sizeof(void*))

/* Round up 'n' to a multiple of ALIGN_SIZE. */
#define INA_MEM_ALIGN(n) ((n+(INA_MEM_ALIGN_SIZE-1)) & (~(INA_MEM_ALIGN_SIZE-1)))

#define INA_MEM_IS_ALIGNED(ptr, alignment) \
    (((uintptr_t)(const void *)(ptr)) % (alignment) == 0)

#define INA_MEM_FREE_SAFE(ptr) do {          \
	if (ptr != NULL) { ina_mem_free(ptr); }  \
	ptr = NULL;                              \
} while(0)

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
 * The function returns the number of bytes in a memory page, where "page" is 
 * a fixed-length block, the unit for memory allocation and file mapping.
 *
 * Parameters
 *  size Size in bytes
 *
 * Return
 *  INA_SUCCESS if no error occurred.
 */                                 
INA_API(ina_rc_t) ina_mem_get_pagesize(size_t *size);
/*
 * The function calculates the size of a memory segment after
 * proper alignment.
 *
 * Parameters:
 *  query    Input size in bytes
 *  aligned  The size after alignment
 *
 * Return
 *  INA_SUCCESS if no error occurred.
 */                                 
INA_API(ina_rc_t) ina_mem_get_aligned_size(size_t query, size_t *aligned);

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
 * size  Size of the memory block, in bytes. size_t is an unsigned integral
 *       type.
 * 
 * Return
 *  On success, a pointer to the memory block allocated by the function.
 *  The type of this pointer is always void*, which can be cast to the desired
 *  type of data pointer in order to be dereferenceable.
 *  If the function failed to allocate the requested block of memory,
 *  a null pointer is returned.
 */
INA_API(void *) ina_mem_alloc(size_t size);

/*
 * Allocate aligned memory block. Allocates a block of size bytes of memory,
 * returning a pointer to the beginning of the block.
 *
 * The content of the newly allocated block of memory is not initialized,
 * remaining with indeterminate values.
 *
 * If size is zero, it returns a null pointer. But the returned
 * pointer shall not be used to dereference an object in any case.
 *
 * Parameters
 * alignment  Memory alignment in bytes
 * size       Size of the memory block, in bytes. size_t is an unsigned integral
 *            type.
 *
 * Return
 *  On success, a pointer to the memory block allocated by the function.
 *  The type of this pointer is always void*, which can be cast to the desired
 *  type of data pointer in order to be dereferenceable.
 *  If the function failed to allocate the requested block of memory,
 *  a null pointer is returned.
 */
INA_API(void *) ina_mem_alloc_aligned(size_t alignment, size_t size);

/*
 * Attempts to resize the memory block pointed to by ptr that was previously
 * allocated with a call to ina_mem_alloc.
 *
 * Parameters
 *  ptr  This is the pointer to a memory block previously allocated with
 *       ina_mem_alloc(). If this is NULL, a new block is allocated and a
 *       pointer to it is returned by the function.
 *  nb   This is the new size for the memory block, in bytes. If it is 0 and ptr
 *       points to an existing block of memory, the memory block pointed by0 ptr
 *       is deallocated and a NULL pointer is returned.
 *
 * Return
 *  This function returns a pointer to the newly allocated memory, or NULL if
 *  the request fails.
 */
INA_API(void *) ina_mem_realloc(void *ptr, size_t nb);

/*
 * Move a memory block.
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
 *  dest  Pointer to the destination array where the content is to be copied,
 *        type-casted to a pointer of type void*
 * 
 *  src   Pointer to the source of data to be copied, type-casted to a pointer
 *        of type const void*.
 *  nb    Number of bytes to copy. size_t is an unsigned integral type.
 *
 * Return
 *  dest is returned
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
 *  dest    Pointer to the destination array where the content is to be
 *          copied, type-casted to a pointer of type void*.
 *  source  Pointer to the source of data to be copied, type-casted to a
 *          pointer of type const void*.
 *  nb      Number of bytes to copy. size_t is an unsigned integral type.
 *
 * Return
 *  dest is returned.
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
 *  lhs  Pointer to block of memory.
 *  rhs  Pointer to block of memory.
 *  nb   Number of bytes to compare.
 *
 * Return
 *  Returns an integral value indicating the relationship between the content
 *  of the memory blocks:
 *  A zero value indicates that the contents of both memory blocks are equal.
 *  A value greater than zero indicates that the first byte that does not
 *  match in both memory blocks has a greater value in lhs than in rhs as if
 *  evaluated as unsigned char values; And a value less than zero indicates
 *  the opposite.
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
 *  dest   Pointer to the block of memory where the search is performed.
 *  value  Value to be located. The value is passed as an int, but the
 *         function performs a byte per byte search using the unsigned char
 *         conversion of this value.
 *  nb     Number of bytes to be analyzed.
 *
 * Return
 *  A pointer to the first occurrence of value in the block of memory pointed
 *  by des. If the value is not found, the function returns a null pointer.
 */
INA_API(void *) ina_mem_chr(const void *dest, int value, size_t nb);

/*
 * Fill block of memory
 *
 * Sets the first num bytes of the block of memory pointed by ptr to the 
 * specified value (interpreted as an unsigned char).
 * 
 * Parameters
 *  dest   Pointer to the block of memory to fill.
 *  value  Value to be set. The value is passed as an int, but the function
 *         fills the block of memory using the unsigned char conversion of
 *         this value.
 *  nb     Number of bytes to be set to the value.
 *
 * Return
 *  dest is returned.
 */
INA_API(void *) ina_mem_set(void *dest, int value, size_t nb);

/* 
 * Deallocate space in memory. A block of memory previously allocated using a 
 * call to ine_mem_alloc() making it available again
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
 */

INA_API(void) ina_mem_free(void *ptr);


#ifdef __cplusplus
}
#endif 

#endif