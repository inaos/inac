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
 */
#ifndef _LIBINAC_STRING_H_
#define _LIBINAC_STRING_H_

#include <libinac/lib.h>

/* string type (hiding a pointer!)  */
typedef char * ina_str_t;
 
/*
 * Memory
 */

/* allocation */
INA_API(ina_str_t) ina_str_new(ina_mempool_t *pool);
INA_API(ina_str_t) ina_str_newlen(const void *anystr, size_t size, ina_mempool_t *pool);

/* destroy */
INA_API(ina_rc_t) ina_str_free(ina_str_t s, ina_mempool_t *pool);

 /* copy */
INA_API(ina_str_t) ina_str_dup(const ina_str_t s, ina_mempool_t *pool);
/* conversion to C string */
INA_API(const char *) ina_str_cstr(const ina_str_t s);

/*
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