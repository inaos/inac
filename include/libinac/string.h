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
#ifndef _LIBINAC_STRING_H_
#define _LIBINAC_STRING_H_

#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif


/* allocation */
INA_API(ina_str_t) ina_str_create(size_t len);
INA_API(ina_str_t) ina_str_create_using_pool(size_t len, ina_mempool_t *pool);

/* 
 * Create a ina_str_t which contains the content of the block blk of length 
 * len.
 *
 * Parameters:
 * blk     Memory block from wich extract the string.
 * len     Length of block to extract.
 * 
 * Return
 * New created string or NULL if an error occurred. 
 */
INA_API(ina_str_t) ina_str_fromblk(const void* blk, size_t len);

/* 
 *  Create a ina_str_t wich contains the content of the block blk of length 
 *  len. Memory will allocated from a memory pool.
 * Parameters:
 * blk     Memory block from wich extract the string.
 * len     Length of block to extract.
 * 
 * Return
 * New created string or NULL if an error occurred.
 */
INA_API(ina_str_t) ina_str_fromblk_using_pool(const void* blk, 
                                              size_t len, 
                                              ina_mempool_t* pool);

INA_API(ina_str_t) ina_str_fromcstr(const char *cstr);
INA_API(ina_str_t) ina_str_fromcstr_using_pool(const char *cstr,  
                                               ina_mempool_t *pool);

/* destroy */
INA_API(ina_rc_t) ina_str_destroy(ina_str_t str);

 /* copy */
INA_API(ina_str_t) ina_str_dup(const ina_str_t str);
INA_API(ina_str_t) ina_str_dup_using_pool(const ina_str_t str, 
                                          ina_mempool_t *pool);

/* conversion to C string */
INA_API(const char *) ina_str_cstr(const ina_str_t str);

/*
 * String manipulation
 */

/*
 * Copies the byte string pointed to by src to byte string, pointed to by dest.
 * If the strings overlap, the behavior is undefined.
 *
 * Parameter:
 *  dest  -  pointer to the byte string to copy to
 *  src   -  pointer to the null-terminated byte string to copy from
 *
 * Return:    dest
 */
INA_API(ina_str_t) ina_str_cpy(ina_str_t dest, const ina_str_t src);
/*
 * Copies at most count characters of the byte string pointed to by src
 * (including the terminating null character) to character array pointed to by
 * dest.
 * If count is reached before the entire string src was copied, the resulting
 * character array is not null-terminated.
 * If, after copying the terminating null character from src, count is not
 * reached, additional null characters are written to dest until the total of
 * count characters have been written.
 * If the strings overlap, the behavior is undefined.
 *
 * Parameters:
 * dest  -   pointer to the character array to copy to
 * src   -   pointer to the byte string to copy from
 * n     -   maximum number of characters to copy
 *
 * Returns value
 * dest
 */
INA_API(ina_str_t) ina_str_ncpy(ina_str_t dest, const ina_str_t src, size_t n);
/*
 * Appends a byte string pointed to by src to a byte string pointed to by dest.
 * The resulting byte string is null-terminated. If the strings overlap, the
 * behavior is undefined.
 *
 * Parameter:
 * dest  -  pointer to the null-terminated byte string to append to
 * src   -  pointer to the null-terminated byte string to copy from
 *
 * Return value
 * dest
 */
INA_API(ina_str_t) ina_str_cat(ina_str_t dest, const ina_str_t src);
INA_API(ina_str_t) ina_str_catcstr(ina_str_t dest, const char* src);

/*
 * Appends a byte string pointed to by src to a byte string pointed to by dest.
 * At most count characters are copied. The resulting byte string is
 * null-terminated. If the strings overlap, the behavior is undefined.
 *
 * Parameters:
 * dest  - pointer to the null-terminated byte string to append to
 * src   - pointer to the null-terminated byte string to copy from
 * n     - maximum number of characters to copy
 *
 * Return value
 * dest
 */
INA_API(ina_str_t) ina_str_ncat(ina_str_t dest, const ina_str_t str, size_t n);
INA_API(ina_str_t) ina_str_ncatcstr(ina_str_t dest, const char *str, size_t n);


/*
 * String examinations
 */

/*
 * Returns the length of the given byte string.
 *
 * Parameters:
 * s  - pointer to the null-terminated byte string to be examined
 *
 * Return value
 * The length of the null-terminated string s.
 */
INA_API(size_t) ina_str_len(const ina_str_t str);
INA_API(size_t) ina_str_size(const ina_str_t str);

/*
 * Compares two null-terminated byte strings. The comparison is done
 * lexicographically.
 * Parameters
 * lhs, rhs -  pointers to the null-terminated byte strings to compare
 *
 * Return value:
 * Negative value if lhs is less than rhs.
 * INA_RC_OK if lhs is equal to rhs.
 * Positive value if lhs is greater than rhs.
 */
INA_API(int) ina_str_cmp(const ina_str_t lhs, const ina_str_t rhs);
/*
 * Same as ina_str_cmp but ignores case.
 * Parameters
 * lhs, rhs -  pointers to the null-terminated byte strings to compare
 *
 * Return value:
 * Negative value if lhs is less than rhs.
 * INA_RC_OK if lhs is equal to rhs.
 * Positive value if lhs is greater than rhs.
 */
INA_API(int) ina_str_casecmp(const ina_str_t lhs, const ina_str_t rhs);
/*
 * Compares at most count characters of two null-terminated byte strings.
 * The comparison is done lexicographically.
 *
 * Parameters
 * lhs, rhs  -  pointers to the null-terminated byte strings to compare
 * n         -  maximum number of characters to compare
 *
 * Return value
 * Negative value if lhs is less than rhs.
 * INA_RC_OK  if lhs is equal to rhs.
 * Positive value if lhs is greater than rhs.
 */
INA_API(int) ina_str_ncmp(const ina_str_t lhs, const ina_str_t rhs, size_t n);

/*
* Locate substring. Returns a pointer to the first occurrence of s2 in s1,
* or a null pointer if s2 is not part of s1.
* The matching process does not include the terminating null-characters.
* Parameters
* str1  - string to be scanned.
* str2 - string containing the sequence of characters to match.
*
* Return Value
* A pointer to the first occurrence in s1 of any of the entire sequence 
* of characters specified in s2, or a null pointer if the sequence is not 
* present in s1.
*/
INA_API(ina_str_t) ina_str_str(const ina_str_t str1, const ina_str_t str2);
INA_API(ina_str_t) ina_str_strcstr(const ina_str_t str1, const char *str2);

/*
 * Locate last occurrence of character in string. Returns a pointer to the
 * last occurrence of character in the C string str. The terminating 
 * null-character is considered part of the string. Therefore, it can also be
 * located to retrieve a pointer to the end of a string.
 * 
 * Parameters
 * str - string.
 * chr    - character to be located.
 *
 * Return value:
 * A pointer to the last occurrence of character in str.
 * If the value is not found, the function returns a null pointer.
 */
INA_API(ina_str_t) ina_str_rchr(const ina_str_t str, const char chr);

INA_API(ina_str_t) ina_str_sprintf(const char *fmt, ...);

INA_API(ina_str_t) ina_str_toupper(ina_str_t str);
INA_API(ina_str_t) ina_str_tolower(ina_str_t str);
/*
 * Writes output to the string str, under control of the format string format, 
 * that specifies how subsequent arguments are converted for output. It is 
 * similar to sprintf, except that size specifies the maximum number of 
 * characters to produce. The trailing nul character is counted towards this 
 * limit, so you must allocate at least size characters for str. 
 * If size is zero, nothing is written and str may be null. Otherwise, output 
 * characters beyond the n-1st are discarded rather than being written to str, 
 * and a nul character is written at the end of the characters actually written 
 * to str. If copying takes place between objects that overlap, the behaviour
 * is undefined.
 * On success, returns the number of characters that would have been written 
 * had size been sufficiently large, not counting the terminating nul 
 * character. Thus, the nul-terminated output has been completely written if 
 * and only if the return value is nonnegative and less than size. On error, 
 * returns -1 (i.e. encoding error).
 */
INA_API(int) ina_str_snprintf(ina_str_t *str, size_t len, const char* fmt, ...);

/*
 * Equivalent to ina_str_snprintf(3) with the variable argument list specified 
 * directly as for vsprintf.
 */
INA_API(int) ina_str_vsnprintf(ina_str_t *str, size_t len, const char* fmt,  
                               va_list args);


#ifdef __cplusplus
}
#endif 

#endif
