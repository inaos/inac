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
#ifndef _LIBINAC_STRING_H_
#define _LIBINAC_STRING_H_

#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * String type
 */
typedef char * ina_str_t;
typedef const char * ina_cstr_t;

/*
 * Creates a empty ina_str_t with as preallocated length len.
 *
 * Parameters
 *  len  Length
 *
 * Return
 *  New created string or NULL if an error occurred.
 */
INA_API(ina_str_t) ina_str_new(size_t len);

/*
 * Creates a empty ina_str_t from a memory pool with as preallocated length len.
 *
 * Parameters
 *  len  Length
 *
 * Return
 *  New created string or NULL if an error occurred.
 */
INA_API(ina_str_t) ina_str_new_using_pool(size_t len, ina_mempool_t *pool);

/* 
 * Creates a ina_str_t which contains the content of the block blk of length
 * len.
 *
 * Parameters
 *  blk  Memory block from wich extract the string.
 *  len  Length of block to extract.
 * 
 * Return
 *  New created string or NULL if an error occurred.
 */
INA_API(ina_str_t) ina_str_new_fromblk(const void* blk, size_t len);

/* 
 * Creates a ina_str_t wich contains the content of blk of length
 * len. Memory will allocated from a memory pool.
 *
 * Parameters
 *  blk  Memory block from which extract the string.
 *  len  Length of block to extract.
 * 
 * Return
 *  New created string or NULL if an error occurred.
 */
INA_API(ina_str_t) ina_str_new_fromblk_using_pool(const void* blk, 
                                                  size_t len, 
                                                  ina_mempool_t* pool);

/*
 * Creates a ina_str_t which contains the content of cstr.
 *
 * Parameters
 *  cstr  C string
 *
 * Return
 *  New created string or NULL if an error occurred.
 */
INA_API(ina_str_t) ina_str_new_fromcstr(const char *cstr);

/*
 * Creates a ina_str_t from a memory pool which contains the content of cstr.
 *
 * Parameters
 *  cstr  C string
 *  pool  Memory pool
 *
 * Return
 *  New created string or NULL if an error occurred.
 */
INA_API(ina_str_t) ina_str_new_fromcstr_using_pool(const char *cstr,
                                               ina_mempool_t *pool);

/*
 * Destroy a string.
 *
 * Parameter
 *  str  String to free
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_str_free(ina_str_t str);


/*
 * Duplicate a string.
 *
 * Parameter
 *  str   String to duplicate
 *
 * Return
 *   Duplicated string or NULL if an error occurred.
 */
INA_API(ina_str_t) ina_str_dup(ina_cstr_t str);

/*
 * Duplicate a string usind a memory pool
 *
 * Parameter
 *  str  String to duplicate
 *  pool  Memory pool
 *
 * Return
 *   Duplicated string or NULL if an error occurred.
 */
INA_API(ina_str_t) ina_str_dup_using_pool(ina_cstr_t str,
                                          ina_mempool_t *pool);

/*
 * Cast a INAC string to a C string
 *
 * Parameter
 *  str  String to cast
 *
 * Return
 *   Casted string
 */
INA_API(const char *) ina_str_cstr(ina_cstr_t str);

/*
 * String manipulation
 */

/*
 * Copies the byte string pointed to by src to byte string, pointed to by dest.
 * If the strings overlap, the behavior is undefined.
 *
 * Parameters
 *  dest  Pointer to the byte string to copy to
 *  src   Pointer to the null-terminated byte string to copy from
 *
 * Return
 *   dest
 */
INA_API(ina_str_t) ina_str_cpy(ina_str_t dest, ina_cstr_t src);

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
 * Parameters
 *  dest  Pointer to the character array to copy to
 *  src   Pointer to the byte string to copy from
 *  n     Maximum number of characters to copy
 *
 * Return
 *  dest
 */
INA_API(ina_str_t) ina_str_ncpy(ina_str_t dest, ina_cstr_t src, size_t n);
/*
 * Appends a byte string pointed to by src to a byte string pointed to by dest.
 * The resulting byte string is null-terminated. If the strings overlap, the
 * behavior is undefined.
 *
 * Parameters
 *  dest  Pointer to the null-terminated byte string to append to
 *  src   Pointer to the null-terminated byte string to copy from
 *
 * Return
 *  dest
 */
INA_API(ina_str_t) ina_str_cat(ina_str_t dest, ina_cstr_t src);

/*
 * Appends a byte string pointed to by src to a byte string pointed to by dest.
 * The resulting byte string is null-terminated. If the strings overlap, the
 * behavior is undefined.
 *
 * Parameters
 *  dest  Pointer to the null-terminated byte string to append to
 *  src   Pointer to the null-terminated byte string to copy from
 *  pool  Memory pool
 *
 * Return
 *  dest
 */
INA_API(ina_str_t) ina_str_cat_using_pool(ina_str_t dest,
                                          ina_cstr_t src,
                                          ina_mempool_t *pool);

/*
 * Appends a byte string pointed to by src to a byte string pointed to by dest.
 * The resulting byte string is null-terminated. If the strings overlap, the
 * behavior is undefined.
 *
 * Parameters
 *  dest  Pointer to the null-terminated byte string to append to
 *  src   Pointer to the null-terminated byte c string to copy from
 *
 * Return
 *  dest
 */
INA_API(ina_str_t) ina_str_catcstr(ina_str_t dest, const char* src);

/*
 * Appends a byte string pointed to by src to a byte string pointed to by dest.
 * The resulting byte string is null-terminated. If the strings overlap, the
 * behavior is undefined.
 *
 * Parameters
 *  dest  Pointer to the null-terminated byte string to append to
 *  src   Pointer to the null-terminated byte c string to copy from
 *  pool  Memory pool
 *
 * Return
 *  dest
 */
INA_API(ina_str_t) ina_str_catcstr_using_pool(ina_str_t dest,
                                              const char* src,
                                              ina_mempool_t *pool);

/*
 * Appends a byte string pointed to by src to a byte string pointed to by dest.
 * At most count characters are copied. The resulting byte string is
 * null-terminated. If the strings overlap, the behavior is undefined.
 *
 * Parameters
 *  dest  Pointer to the null-terminated byte string to append to
 *  src   Pointer to the null-terminated byte string to copy from
 *  n     Maximum number of characters to copy
 *
 * Return
 *  dest
 */
INA_API(ina_str_t) ina_str_ncat(ina_str_t dest, ina_cstr_t src, size_t n);
/*
 * Appends a byte string pointed to by src to a byte string pointed to by dest.
 * At most count characters are copied. The resulting byte string is
 * null-terminated. If the strings overlap, the behavior is undefined.
 *
 * Parameters
 *  dest  Pointer to the null-terminated byte string to append to
 *  src   Pointer to the null-terminated byte string to copy from
 *  n     Maximum number of characters to copy
 *  pool  Memory pool
 *
 * Return
 *  dest
 */
INA_API(ina_str_t) ina_str_ncat_using_pool(ina_str_t dest,
                                           ina_cstr_t src,
                                           size_t n,
                                           ina_mempool_t *pool);
/*
 * Appends a byte string pointed to by src to a byte string pointed to by dest.
 * At most count characters are copied. The resulting byte string is
 * null-terminated. If the strings overlap, the behavior is undefined.
 *
 * Parameters
 *  dest  Pointer to the null-terminated byte string to append to
 *  src   Pointer to the null-terminated byte string to copy from
 *  n     Maximum number of characters to copy
 *
 * Return
 *  dest
 */
INA_API(ina_str_t) ina_str_ncatcstr(ina_str_t dest, const char *src, size_t n);

/*
 * Appends a byte string pointed to by src to a byte string pointed to by dest.
 * At most count characters are copied. The resulting byte string is
 * null-terminated. If the strings overlap, the behavior is undefined.
 *
 * Parameters
 *  dest  Pointer to the null-terminated byte string to append to
 *  src   Pointer to the null-terminated byte c string to copy from
 *  n     Maximum number of characters to copy
 *  pool  Memory pool
 *
 * Return
 *  dest
 */
INA_API(ina_str_t) ina_str_ncatcstr_using_pool(ina_str_t dest,
                                               const char *src,
                                               size_t n,
                                               ina_mempool_t *pool);


/*
 * String examinations
 */

/*
 * Returns the length of the given string byte string.
 *
 * Parameters
 *  s  Pointer to the null-terminated byte string to be examined
 *
 * Return
 *  The length of the null-terminated string s.
 */
INA_API(size_t) ina_str_len(ina_cstr_t str);

/*
 * Returns the memory size of the given string byte string.
 *
 * Parameters
 *  s  Pointer to the null-terminated byte string to be examined
 *
 * Return
 *  The memory size in bytes of string s.
 */
INA_API(size_t) ina_str_size(ina_cstr_t str);

/*
 * Returns the free memory size of the given string byte string.
 *
 * Parameters
 *  s  Pointer to the null-terminated byte string to be examined
 *
 * Return
 *  The free memory size in bytes for string s.
 */
INA_API(size_t) ina_str_available(ina_cstr_t str);

/*
 * Compares two null-terminated byte strings. The comparison is done
 * lexicographically.
 *
 * Parameters
 *  lhs, rhs  Pointers to the null-terminated byte strings to compare
 *
 * Return
 *  Negative value if lhs is less than rhs.
 *  INA_SUCCESS if lhs is equal to rhs.
 *  Positive value if lhs is greater than rhs.
 */
INA_API(int) ina_str_cmp(ina_cstr_t lhs, ina_cstr_t rhs);

/*
 * Same as ina_str_cmp but ignores case.
 *
 * Parameters
 * lhs, rhs  Pointers to the null-terminated byte strings to compare
 *
 * Return
 *  Negative value if lhs is less than rhs.
 *  INA_SUCCESS if lhs is equal to rhs.
 *  Positive value if lhs is greater than rhs.
 */
INA_API(int) ina_str_casecmp(ina_cstr_t lhs, ina_cstr_t rhs);

/*
 * Compares at most count characters of two null-terminated byte strings.
 * The comparison is done lexicographically.
 *
 * Parameters
 * lhs, rhs  Pointers to the null-terminated byte strings to compare
 * n         Maximum number of characters to compare
 *
 * Return
 *  Negative value if lhs is less than rhs.
 *  INA_SUCCESS  if lhs is equal to rhs.
 *  Positive value if lhs is greater than rhs.
 */
INA_API(int) ina_str_ncmp(ina_cstr_t lhs, ina_cstr_t rhs, size_t n);

/*
 * Locate substring. Returns a pointer to the first occurrence of s2 in s1,
 * or a null pointer if s2 is not part of s1.
 * The matching process does not include the terminating null-characters.
 *
 * Parameters
 *  str1  String to be scanned.
 *  str2  String containing the sequence of characters to match.
 *
 * Return
 *  A pointer to the first occurrence in s1 of any of the entire sequence
 *  of characters specified in s2, or a null pointer if the sequence is not
 *  present in s1.
 */
INA_API(const char*) ina_str_str(ina_cstr_t str1, ina_cstr_t str2);

/*
 * Locate substring. Returns a pointer to the first occurrence of s2 in s1,
 * or a null pointer if s2 is not part of s1.
 * The matching process does not include the terminating null-characters.
 *
 * Parameters
 *  str1  String to be scanned.
 *  str2  C string containing the sequence of characters to match.
 *
 * Return
 *  A pointer to the first occurrence in s1 of any of the entire sequence
 *  of characters specified in s2, or a null pointer if the sequence is not
 *  present in s1.
 */
INA_API(const char*) ina_str_strcstr(ina_cstr_t str1, const char *str2);

/*
 * Locate last occurrence of character in string. Returns a pointer to the
 * last occurrence of character in the C string str. The terminating 
 * null-character is considered part of the string. Therefore, it can also be
 * located to retrieve a pointer to the end of a string.
 * 
 * Parameters
 *  str  String
 *  chr  Character to be located.
 *
 * Return
 *  A pointer to the last occurrence of character in str. If the value is not
 *  found, the function returns a null pointer.
 */
INA_API(const char*) ina_str_rchr(ina_cstr_t str, const char chr);

/*
 * Perform a zero copy tokenizing of a string. Bea aware, the returning string
 * muss be freed by the caller.
 *
 * Parameters
 *  str   Source string to tokenize
 *  sep   Separator char
 *  next  Where to store the next token
 *
 * Return
 *  token or NULL if no more token founds
 */
INA_API(const char*) ina_str_tok(char* str, const char *sep, char **next);

/*
 * Convert a string to uppercase.
 *
 * Parameters
 *  str  String to convert
 *
 * Return
 *  str
 */
INA_API(ina_str_t) ina_str_toupper(ina_str_t str);

/*
 * Convert a string to lowercase
 *
 * Parameters
 *  str  String to convert
 *
 * Return
 *  str
 */
INA_API(ina_str_t) ina_str_tolower(ina_str_t str);

/*
 * Truncate a string at a given position. The internal memory size will not
 * be truncated.
 *
 * Parameters
 *  str   String to truncate
 *  pos   Position at which the string should be truncated
 *
 * Return
 *  str
 */
INA_API(ina_str_t) ina_str_truncate(ina_str_t str, size_t pos);

/*
 * Trim (left and right) a string.
 *
 * Parameters
 *  str    String to trim
 *  chars  Char pattern to trim
 *
 * Return
 *  Trimmed string (str)
 */
INA_API(ina_str_t) ina_str_trim(ina_str_t str, const char* chars);

/*
 * Extract the substring starting at start until end. A new string will be
 * returned and must be freed by the caller.
 *
 * Parameters
 *  str    String
 *  start  Substring start position
 *  end    Substring end position (If zero (0) end is string len)
 *
 * Return
 *  Substring or NULL if an error occurred
 */
INA_API(ina_str_t) ina_str_substr(ina_cstr_t str, size_t start, size_t end);

/*
 * Extract the substring starting at start until end. A new string will be
 * returned.
 *
 * Parameters
 *  str    String
 *  start  Substring start position
 *  end    Substring end position (If zero (0) end is string len)
 *  pool   Memory pool
 *
 * Return
 *  Substring or NULL if an error occurred
 */
INA_API(ina_str_t) ina_str_substr_using_pool(ina_cstr_t str,
                                             size_t start,
                                             size_t end,
                                             ina_mempool_t *pool);


/*
 * Splits a string into array by a separator char
 *
 * Parameters
 *  str    The input string
 *  sep    Separator char
 *  count  Where to store the number of elements returned
 *
 * Return
 *  String array or NULL if an error occurred. The array must be freed by
 *  calling ina_str_split_free_tokens()
 */
INA_API(ina_str_t*) ina_str_split(const char *str,
                                  const char *sep,
                                  size_t *count);

/*
 * Free token array created by ina_str_split().
 *
 * Parameters
 *  tokens  String array to free
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t)  ina_str_split_free_tokens(ina_str_t *tokens);

/*
 * Adjust the internal length. Use this function after the string buffer was
 * modified directly.
 *
 * Parameters
 *   str  Sting to adjust
 *
 * Return
 *  str
 */
INA_API(ina_str_t) ina_str_adjust_len(ina_str_t str);


/*
 * Writes output to the string str, under control of the format string format,
 * that specifies how subsequent arguments are converted for output.  The
 * trailing nul character is counted towards this limit, so you must allocate
 * at least size characters for str.
 * If size is zero, nothing is written and str may be null. Otherwise, output
 * characters beyond the n-1st are discarded rather than being written to str,
 * and a nul character is written at the end of the characters actually written
 * to str. If copying takes place between objects that overlap, the behaviour
 * is undefined
 *
 * Parameters
 *  fmt  String format
 *  ...  Arguments for formatting
 *
 * Return
 *  On success, returns the number of characters that would have been written
 *  had size been sufficiently large, not counting the terminating nul
 *  character. Thus, the nul-terminated output has been completely written if
 *  and only if the return value is nonnegative and less than size. On error,
 *  returns -1 (i.e. encoding error).
 */
INA_API(ina_str_t) ina_str_sprintf(const char *fmt, ...);

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
 *
 * Parameters
 *  str  Output string
 *  len  Maximum output length to produce
 *  str  String format
 *  ...  Arguments for formatting
 *
 * Return
 *  On success, returns the number of characters that would have been written
 *   had size been sufficiently large, not counting the terminating nul
 *  character. Thus, the nul-terminated output has been completely written if
 *  and only if the return value is nonnegative and less than size. On error,
 *  returns -1 (i.e. encoding error).
 */
INA_API(int) ina_str_snprintf(ina_str_t *str, size_t len, const char* fmt, ...);

/*
 * Equivalent to ina_str_snprintf(3) with the variable argument list specified
 * directly as for vsprintf.
 */
INA_API(int) ina_str_vsnprintf(ina_str_t *str, size_t len, const char* fmt,  
                               va_list args);

/*
 * Tests the tame string if it matches the given wildcard. Supported wildcard
 * characters:
 *  - ? = matches a single character
 *  - * = matches any number of characters
 * Note: This does not support regular expressions
 *
 * Parameters
 *  tame      Input string
 *  wildcard  Wildcard character
 */
INA_API(ina_rc_t) ina_str_wildcard_match(ina_cstr_t tame,
                                         const char *wildcard);

#ifdef __cplusplus
}
#endif 

#endif
