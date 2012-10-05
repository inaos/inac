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
<<<<<<< HEAD
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
=======
 *     * Neither the name of the INAOS GmbH nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL INAOS GmbH BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
>>>>>>> 244ecc23bf8bc8d3f4c0def4823befca126b443a
 */
#ifndef _LIBINAC_STRING_H_
#define _LIBINAC_STRING_H_

#include <libinac/lib.h>

/* string type (hiding a pointer!)  */
typedef char * ina_str_t;
<<<<<<< HEAD

=======
 
>>>>>>> 244ecc23bf8bc8d3f4c0def4823befca126b443a
/*
 * Memory
 */

/* allocation */
INA_API(ina_str_t) ina_str_new(ina_mempool_t *pool);
INA_API(ina_str_t) ina_str_newlen(const void *anystr, size_t size, ina_mempool_t *pool);

/* destroy */
<<<<<<< HEAD
INA_API(ina_rc_t) ina_str_free(ina_str_t s);

 /* copy */
INA_API(ina_str_t) ina_str_dup(const ina_str_t s, ina_mempool_t *pool);

=======
INA_API(ina_rc_t) ina_str_free(ina_str_t s, ina_mempool_t *pool);

 /* copy */
INA_API(ina_str_t) ina_str_dup(const ina_str_t s, ina_mempool_t *pool);
>>>>>>> 244ecc23bf8bc8d3f4c0def4823befca126b443a
/* conversion to C string */
INA_API(const char *) ina_str_cstr(const ina_str_t s);

/*
<<<<<<< HEAD
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
 * count -   maximum number of characters to copy
 *
 * Returns value
 * dest
 */
INA_API(ina_str_t) ina_str_ncpy(ina_str_t dest, const ina_str_t, size_t count);
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
/*
 * Appends a byte string pointed to by src to a byte string pointed to by dest.
 * At most count characters are copied. The resulting byte string is
 * null-terminated. If the strings overlap, the behavior is undefined.
 *
 * Parameters:
 * dest  - pointer to the null-terminated byte string to append to
 * src   - pointer to the null-terminated byte string to copy from
 * count - maximum number of characters to copy
 *
 * Return value
 * dest
 */
INA_API(ina_str_t) ina_str_ncat(ina_str_t dest, const ina_str_t, size_t);

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
INA_API(size_t) ina_str_len(ina_str_t s);

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
INA_API(ina_rc_t) ina_str_cmp(const ina_str_t lhs, const ina_str_t rhs);
/*
 * Compares at most count characters of two null-terminated byte strings.
 * The comparison is done lexicographically.
 *
 * Parameters
 * lhs, rhs  -  pointers to the null-terminated byte strings to compare
 * count     -  maximum number of characters to compare
 *
 * Return value
 * Negative value if lhs is less than rhs.
 * INA_RC_OK  if lhs is equal to rhs.
 * Positive value if lhs is greater than rhs.
 */
INA_API(ina_rc_t) ina_str_ncmp(const ina_str_t lhs, const ina_str_t rhs, size_t count);

/*
* Locate substring. Returns a pointer to the first occurrence of s2 in s1,
* or a null pointer if s2 is not part of s1.
* The matching process does not include the terminating null-characters.
* Parameters
* s1  - string to be scanned.
* s2  - string containing the sequence of characters to match.
*
* Return Value
* A pointer to the first occurrence in s1 of any of the entire sequence 
* of characters specified in s2, or a null pointer if the sequence is not 
* present in s1.
*/
INA_API(ina_str_t) ina_str_str(const ina_str_t s1, const ina_str_t s2);

/*
 * Locate last occurrence of character in string. Returns a pointer to the
 * last occurrence of character in the C string str. The terminating 
 * null-character is considered part of the string. Therefore, it can also be
 * located to retrieve a pointer to the end of a string.
 * 
 * Parameters
 * str - string.
 * c    - character to be located.
 *
 * Return value:
 * A pointer to the last occurrence of character in str.
 * If the value is not found, the function returns a null pointer.
 */
INA_API(ina_str_t) ina_str_rchr(const ina_str_t s, const char c);

/*
 * Returns text version of the error code errnum. errnum is usually acquired
 * from the errno variable, however the function accepts any value of type int
 * The message is locale-specific.
 * The returned byte string must not be modified by the program, but may be
 * overwritten by a subsequent call to the strerror function.
 *
 * Parameters
 * errnum  - integral value referring to a error code
 *
 * Return value:
 * Pointer to a null-terminated byte string corresponding to the error code 
 * errnum.
 */
INA_API(ina_str_t) ina_str_errmsg(int errnum);

#endif
=======
 * string.h like functions
 */

/* concatenation  */
INA_API(ina_str_t) ina_str_cat(const ina_str_t s1, const ina_str_t s2, ina_mempool_t *pool);
/* comparison  */
INA_API(ina_rc_t) ina_str_cmp(const ina_str_t s1, const ina_str_t s2);
/* searching  */
INA_API(const ina_str_t) ina_str_strstr(const ina_str_t s1, const ina_str_t s2);
INA_API(const ina_str_t) ina_str_strrch(const ina_str_t s1, const ina_str_t s2);


#endif
>>>>>>> 244ecc23bf8bc8d3f4c0def4823befca126b443a
