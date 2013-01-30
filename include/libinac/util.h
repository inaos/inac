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
#ifndef _LIBINAC_UTIL_H_
#define _LIBINAC_UTIL_H_

#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define INA_HASH_CSTR_TO_CRC32(s) ina_util_hash_crc32(0, s, strlen(s))
#define INA_HASH_CSTR_TO_SDBM(s)  ina_util_hash_sdbm(0, s, strlen(s))
#define INA_HASH_STR_TO_CRC32(s) ina_util_hash_crc32(0, ina_str_cstr(s), ina_str_len(s))
#define INA_HASH_STR_TO_SDBM(s)  ina_util_hash_sdbm(0, ina_str_cstr(s), ina_str_len(s))


/*
 * Calulate 32bit CRC hash
 *
 * Parameters
 *  hash   starting hash
 *  data   data to hash
 *  size   size of buffer to hash
 *
 * Return Value
 *  Hash
 */
INA_API(uint32_t) ina_util_hash_crc32(uint32_t hash, const void *data, 
                                      size_t size);

/*
 * Calculate 32bit SDBM hash
 *
 * Parameters
 *  hash   starting hash
 *  data   data to hash
 *  size   size of buffer to hash
 *
 * Return Value
 *  Hash
 */
INA_API(uint32_t) ina_util_hash_sdbm(uint32_t hash, const void *data, 
                                     size_t size);

     
#ifdef __cplusplus
}
#endif 

#endif