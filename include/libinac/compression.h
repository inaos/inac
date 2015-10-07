/*
* Copyright (c) 2014, INAOS GmbH
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
* * Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
* * Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
* * Neither the name of the INAOS GmbH nor the names of its contributors
* may be used to endorse or promote products derived from this software
* without specific prior written permission.
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
#ifndef _LIBINAC_COMPRESSION_H_
#define _LIBINAC_COMPRESSION_H_

#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * FIXME: this should go into the manual instead of the header file
 *
 * How to use the module:
 *
 * 1. Create a new compression state, type (e.g. DEFLATE or LZ4) is mandatory
 *    the size of the source buffer is configurable - default is 4MB
 * 2. Once the state has been created you call ina_compression_get_destination_len()
 *    to obtain the length of the destination buffer that you have to manage/allocate
 * 3. Then call ina_compression_compress_chunk() until your stream is EOF
 * 4. Same goes for decompression but instead you call ina_compression_decompress_chunk()
 *
 */

typedef enum ina_compression_type_e {
    INA_COMPRESSION_TYPE_DEFLATE,
    INA_COMPRESSION_TYPE_DEFLATE_RAW,
    INA_COMPRESSION_TYPE_LZ4,
    INA_COMPRESSION_TYPE_LZ4HC,
} ina_compression_type_t;

typedef enum ina_compression_mode_e {
    INA_COMPRESSION_MODE_TRUSTED_FAST,
    INA_COMPRESSION_MODE_TRUSTED_SAFE,
} ina_compression_mode_t;

/* should be opaque */
typedef struct ina_compression_state_s ina_compression_state_t;

/*
 *
 */
INA_API(ina_rc_t) ina_compression_new(ina_compression_state_t **state, ina_compression_type_t type,
                                      ina_compression_mode_t mode);
/*
 *
 */
INA_API(ina_rc_t) ina_compression_new_using_pool(ina_compression_state_t **state, ina_compression_type_t type,
                                                 ina_compression_mode_t mode, ina_mempool_t *pool);
/*
 *
 */
INA_API(ina_rc_t) ina_compression_reset(ina_compression_state_t *state);

/*
 *
 */
INA_API(ina_rc_t) ina_compression_free(ina_compression_state_t **state);
/*
 *
 */
INA_API(ina_rc_t) ina_compression_get_destination_len(ina_compression_state_t *state, size_t src_len, size_t *len);
/*
 *
 */
INA_API(ina_rc_t) ina_compression_compress_chunk(ina_compression_state_t *state, const unsigned char *src,
                                                 size_t src_len, unsigned char *dst, size_t dst_len, size_t *wrote_len, size_t *read_len, int more);
/*
 *
 */										   
INA_API(ina_rc_t) ina_compression_decompress_chunk(ina_compression_state_t *state, const unsigned char *src,
                                                   size_t src_len, unsigned char *dst, size_t dst_len, size_t *wrote_len, size_t *read_len, int more);

#ifdef __cplusplus
}
#endif

#endif
