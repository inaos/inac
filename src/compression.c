/*
 * Copyright (c) 2014, INAOS GmbH
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
#include <libinac/lib.h>
#include "config.h"

#include <contribs/lz4/lz4.h>
#include <contribs/lz4/lz4hc.h>

#define MINIZ_NO_MALLOC

#include <contribs/miniz/miniz.c>

struct ina_compression_state_s;

typedef ina_rc_t (*ina_compression_compress_fn)(struct ina_compression_state_s *state, const unsigned char *src,
                                                unsigned char *dst, size_t dst_len, size_t *wrote_len);

typedef ina_rc_t (*ina_compression_decompress_fn)(struct ina_compression_state_s *state, const unsigned char *src,
                                                  size_t src_len, unsigned char *dst, size_t dst_len, size_t *wrote_len);

typedef ina_rc_t (*ina_compression_dest_len_fn)(struct ina_compression_state_s *state, size_t *dst_len);

struct ina_compression_state_s {
    ina_compression_type_t type;
    ina_compression_direction_t direction;
    ina_compression_compress_fn compress_fn;
    ina_compression_decompress_fn decompress_fn;
    ina_compression_dest_len_fn dest_len_fn;
    ina_mempool_t *mempool;
    size_t chunk_src_len;
    size_t chunk_proposed_dst_len;
    void *statedata;
};

static ina_rc_t ina_compression_compress_lz4(ina_compression_state_t *state, const unsigned char *src, 
                                             unsigned char *dst, size_t dst_len, size_t *wrote_len)
{
	*wrote_len = LZ4_compress_withState(state->statedata, 
            (const char*)src, (char*)dst, state->chunk_src_len);
	if (wrote_len == 0) {
		return INA_FAILURE;
	}
	return INA_SUCCESS;
}

static ina_rc_t ina_compression_compress_lz4hc(ina_compression_state_t *state, const unsigned char *src, 
                                             unsigned char *dst, size_t dst_len, size_t *wrote_len)
{
	*wrote_len = LZ4_compressHC_withStateHC(state->statedata, 
            (const char*)src, (char*)dst, state->chunk_src_len);
	if (wrote_len == 0) {
		return INA_FAILURE;
	}
	return INA_SUCCESS;
}

static ina_rc_t ina_compression_decompress_lz4_fast(ina_compression_state_t *state, const unsigned char *src, 
                                                    size_t src_len, unsigned char *dst, size_t dst_len, size_t *wrote_len)
{
	*wrote_len = LZ4_decompress_fast((const char*)src, (char*)dst, state->chunk_src_len);
	if (wrote_len == 0) {
		return INA_FAILURE;
	}
	return INA_SUCCESS;
}

static ina_rc_t ina_compression_decompress_lz4_safe(ina_compression_state_t *state, const unsigned char *src, 
                                                    size_t src_len, unsigned char *dst, size_t dst_len, size_t *wrote_len)
{
	*wrote_len = LZ4_decompress_safe((const char*)src, (char*)dst, state->chunk_src_len, dst_len);
	if (wrote_len == 0) {
		return INA_FAILURE;
	}
	return INA_SUCCESS;
}

static ina_rc_t ina_compression_bounds_lz4(struct ina_compression_state_s *state, size_t *dst_len)
{
    *dst_len = LZ4_compressBound(state->chunk_src_len);
    return INA_SUCCESS;
}

static void *ina_mz_wop_alloc_func(void *opaque, size_t items, size_t size)
{
    return ina_mem_alloc(items * size);
}
static void ina_mz_wop_free_func(void *opaque, void *address)
{
    ina_mem_free(address);
}
static void *ina_mz_wp_alloc_func(void *opaque, size_t items, size_t size)
{
    ina_mempool_t *pool = (ina_mempool_t*)opaque;
    return ina_mempool_nalloc(pool, items * size);
}
static void ina_mz_wp_free_func(void *opaque, void *address)
{
    /* we do not free using a pool */
}

static ina_rc_t ina_compression_compress_miniz(ina_compression_state_t *state, const unsigned char *src, 
                                               unsigned char *dst, size_t dst_len, size_t *wrote_len)
{
    int status;
    mz_stream *stream = (mz_stream*)state->statedata;
    
    ina_mem_set(stream, 0, sizeof(stream));
    if (state->mempool) {
        stream->opaque = state->mempool;
        stream->zalloc = ina_mz_wp_alloc_func;
        stream->zfree = ina_mz_wp_free_func;
    }
    else {
        stream->zalloc = ina_mz_wop_alloc_func;
        stream->zfree = ina_mz_wop_free_func;
    }
    
    stream->next_in = src;
    stream->avail_in = (mz_uint32)state->chunk_src_len;
    stream->next_out = dst;
    stream->avail_out = (mz_uint32)dst_len;
    
    status = mz_deflateInit(stream, MZ_DEFAULT_COMPRESSION);
    if (status != MZ_OK) {
        return INA_FAILURE;
    }
    
    status = mz_deflate(stream, MZ_FINISH);
    if (status != MZ_STREAM_END) {
        mz_deflateEnd(stream);
        if (status == MZ_OK) {
            /* BUF_ERROR */
            return INA_FAILURE;
        }
        else {
            return INA_FAILURE;
        }
    }
    
    *wrote_len = stream->total_out;
    
    if (mz_deflateEnd(stream) == MZ_OK) {
        return INA_SUCCESS;
    }
    else {
        return INA_FAILURE;
    }
}

static ina_rc_t ina_compression_decompress_miniz(ina_compression_state_t *state, const unsigned char *src, 
                                                 size_t src_len, unsigned char *dst, size_t dst_len, size_t *wrote_len)
{
    int status;
    mz_stream *stream = (mz_stream*)state->statedata;
    
    ina_mem_set(stream, 0, sizeof(stream));
    if (state->mempool) {
        stream->opaque = state->mempool;
        stream->zalloc = ina_mz_wp_alloc_func;
        stream->zfree = ina_mz_wp_free_func;
    }
    else {
        stream->zalloc = ina_mz_wop_alloc_func;
        stream->zfree = ina_mz_wop_free_func;
    }
    
    stream->next_in = src;
    stream->avail_in = (mz_uint32)src_len;
    stream->next_out = dst;
    stream->avail_out = (mz_uint32)dst_len;
    
    status = mz_inflateInit(stream);
    if (status != MZ_OK) {
        return INA_FAILURE;
    }
    
    status = mz_inflate(stream, MZ_FINISH);
    if (status != MZ_STREAM_END) {
        mz_inflateEnd(stream);
        if ((status == MZ_BUF_ERROR) && (!stream->avail_in)) {
            /* BUF_ERR */
            return INA_FAILURE;
        }
        else {
            return INA_FAILURE;
        }
    }
    *wrote_len = stream->total_out;

    if (mz_inflateEnd(stream) == MZ_OK) {
        return INA_SUCCESS;
    }
    else {
        return INA_FAILURE;
    }
}

static ina_rc_t ina_compression_bounds_miniz(struct ina_compression_state_s *state, size_t *dst_len)
{
    *dst_len = mz_compressBound(state->chunk_src_len);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_compression_new(ina_compression_state_t **state, ina_compression_type_t type,
                                               ina_compression_direction_t direction,
                                               ina_compression_mode_t mode)
{
    return ina_compression_new_using_pool(state, type, direction, mode, NULL);
}

INA_API(ina_rc_t) ina_compression_new_using_pool(ina_compression_state_t **state, ina_compression_type_t type,
                                               ina_compression_direction_t direction, ina_compression_mode_t mode,
                                               ina_mempool_t *pool)
{
    size_t sstate;

    if (pool != NULL) {
        *state = (ina_compression_state_t*)ina_mempool_dalloc(pool, sizeof(struct ina_compression_state_s));
    }
    else {
        *state = (ina_compression_state_t*)ina_mem_alloc(sizeof(struct ina_compression_state_s));
    }

    (*state)->type = type;
    (*state)->direction = direction;
    switch (type) {
        case INA_COMPRESSION_TYPE_DEFLATE:
            (*state)->compress_fn = ina_compression_compress_miniz;
            (*state)->decompress_fn = ina_compression_decompress_miniz;
            (*state)->dest_len_fn = ina_compression_bounds_miniz;
            sstate = sizeof(mz_stream);
            break;
        case INA_COMPRESSION_TYPE_LZ4:
            (*state)->compress_fn = ina_compression_compress_lz4;
            (*state)->dest_len_fn = ina_compression_bounds_lz4;
            if (mode == INA_COMPRESSION_MODE_TRUSTED_FAST) {
                (*state)->decompress_fn = ina_compression_decompress_lz4_fast;
            }
            else {
                (*state)->decompress_fn = ina_compression_decompress_lz4_safe;
            }
            sstate = LZ4_sizeofState();
            break;
        case INA_COMPRESSION_TYPE_LZ4HC:
            (*state)->compress_fn = ina_compression_compress_lz4hc;
            (*state)->dest_len_fn = ina_compression_bounds_lz4;
            if (mode == INA_COMPRESSION_MODE_TRUSTED_FAST) {
                (*state)->decompress_fn = ina_compression_decompress_lz4_fast;
            }
            else {
                (*state)->decompress_fn = ina_compression_decompress_lz4_safe;
            }
            sstate = LZ4_sizeofStateHC();
            break;
    }
    (*state)->mempool = pool;
    if (direction == INA_COMPRESSION_DIRECTION_COMPRESS || type == INA_COMPRESSION_TYPE_DEFLATE) {
        if (pool != NULL) {
            (*state)->statedata = ina_mempool_dalloc(pool, sstate);
        }
        else {
            (*state)->statedata = ina_mem_alloc(sstate);
        }
    }

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_compression_free(ina_compression_state_t **state)
{
    if ((*state)->mempool == NULL) {
        ina_mem_free((*state)->statedata);
        ina_mem_free(*state);
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_compression_compress_chunk(ina_compression_state_t *state, const unsigned char *src,
                                                 size_t src_len, unsigned char *dst, size_t dst_len, size_t *wrote_len)
{
    state->chunk_src_len = src_len;
    return state->compress_fn(state, src, dst, dst_len, wrote_len);
}

INA_API(ina_rc_t) ina_compression_decompress_chunk(ina_compression_state_t *state, const unsigned char *src,
                                                   size_t src_len, unsigned char *dst, size_t dst_len, size_t *wrote_len)
{
    state->chunk_src_len = src_len;
    return state->decompress_fn(state, src, src_len, dst, dst_len, wrote_len);
}

INA_API(ina_rc_t) ina_compression_get_destination_len(ina_compression_state_t *state, size_t src_len, size_t *len)
{
    state->chunk_src_len = src_len;
    state->dest_len_fn(state, &state->chunk_proposed_dst_len);
    *len = state->chunk_proposed_dst_len;
    return INA_SUCCESS;
}
