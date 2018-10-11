/*
 * Copyright INAOS GmbH, Thalwil, 2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>
#include "config.h"

#include <contribs/lz4/lz4.h>
#include <contribs/lz4/lz4hc.h>

#define MINIZ_NO_MALLOC

#include <contribs/miniz/miniz.h>

struct ina_compression_state_s;

typedef ina_rc_t (*ina_compression_compress_fn)(struct ina_compression_state_s *state, const unsigned char *src,
                                                unsigned char *dst, int dst_len, int *wrote_len, int *read_len, int more);

typedef ina_rc_t (*ina_compression_decompress_fn)(struct ina_compression_state_s *state, const unsigned char *src,
                                                  int src_len, unsigned char *dst, int dst_len, int *wrote_len, int *read_len, int more);

typedef ina_rc_t (*ina_compression_dest_len_fn)(struct ina_compression_state_s *state, int *dst_len);

struct ina_compression_state_s {
    ina_compression_type_t type;
    ina_compression_compress_fn compress_fn;
    ina_compression_decompress_fn decompress_fn;
    ina_compression_dest_len_fn dest_len_fn;
    ina_mempool_t *mempool;
    int chunk_src_len;
    int chunk_proposed_dst_len;
    void *statedata;
    int flags;
    int more;
    int initialized;
    int finalized;
};

static ina_rc_t ina_compression_compress_lz4(ina_compression_state_t *state, const unsigned char *src, 
                                             unsigned char *dst, int dst_len, int *wrote_len, int *read_len, int more)
{
    INA_ASSERT_NOT_NULL(state);
    INA_ASSERT_NOT_NULL(wrote_len);
    INA_ASSERT_NOT_NULL(read_len);
    INA_UNUSED(more);
    *wrote_len = 0;
    *read_len = 0;

    *wrote_len = LZ4_compress_fast_extState(state->statedata,
            (const char*)src, (char*)dst, state->chunk_src_len, dst_len, 1);
    if (*wrote_len == 0) {
        return INA_ERROR(INA_ES_COMPRESSION|INA_ERR_FAILED);
    }
    return INA_SUCCESS;
}

static ina_rc_t ina_compression_compress_lz4hc(ina_compression_state_t *state, const unsigned char *src, 
                                             unsigned char *dst, int dst_len, int *wrote_len, int *read_len, int more)
{
    INA_UNUSED(more);
    INA_ASSERT_NOT_NULL(state);
    INA_ASSERT_NOT_NULL(wrote_len);
    INA_ASSERT_NOT_NULL(read_len);
    *wrote_len = 0;
    *read_len = 0;

    *wrote_len = LZ4_compress_HC_extStateHC(state->statedata,
            (const char*)src, (char*)dst, state->chunk_src_len, dst_len, 1);
    if (*wrote_len == 0) {
        return INA_ERROR(INA_ES_COMPRESSION|INA_ERR_FAILED);
    }
    return INA_SUCCESS;
}

static ina_rc_t ina_compression_decompress_lz4_fast(ina_compression_state_t *state, const unsigned char *src, 
                                                    int src_len, unsigned char *dst, int dst_len, int *wrote_len, int *read_len, int more)
{
    int read = 0;
    INA_UNUSED(src_len);
    INA_UNUSED(dst_len);
    INA_UNUSED(src);
    INA_UNUSED(more);
    INA_ASSERT_NOT_NULL(state);
    INA_ASSERT_NOT_NULL(wrote_len);
    INA_ASSERT_NOT_NULL(read_len);
    *wrote_len = 0;

    INA_ASSERT_TRUE(state->chunk_src_len > 0);
    *read_len = LZ4_decompress_fast((const char*)src, (char*)dst, state->chunk_src_len);
    if (read < 0) {
        return INA_ERROR(INA_ES_DECOMPRESSION|INA_ERR_FAILED);
    }
    *wrote_len = state->chunk_src_len;
    return INA_SUCCESS;
}

static ina_rc_t ina_compression_decompress_lz4_safe(ina_compression_state_t *state, const unsigned char *src, 
                                                    int src_len, unsigned char *dst, int dst_len, int *wrote_len, int *read_len, int more)
{
    INA_UNUSED(more);
    INA_ASSERT_NOT_NULL(state);
    INA_ASSERT_NOT_NULL(wrote_len);
    INA_ASSERT_NOT_NULL(read_len);
    *wrote_len = 0;
    *read_len = 0;

    *wrote_len = LZ4_decompress_safe((const char*)src, (char*)dst, src_len, dst_len);
    if (*wrote_len <= 0) {
        return INA_ERROR(INA_ES_DECOMPRESSION|INA_ERR_FAILED);
    }
    return INA_SUCCESS;
}

static ina_rc_t ina_compression_bounds_lz4(struct ina_compression_state_s *state, int *dst_len)
{
    INA_ASSERT_NOT_NULL(state);
    INA_ASSERT_NOT_NULL(dst_len);

    *dst_len = LZ4_compressBound(state->chunk_src_len);
    return INA_SUCCESS;
}

static void *ina_mz_wop_alloc_func(void *opaque, size_t items, size_t size)
{
    INA_UNUSED(opaque);
    return ina_mem_alloc(items * size);
}
static void ina_mz_wop_free_func(void *opaque, void *address)
{
    INA_UNUSED(opaque);
    ina_mem_free(address);
}
static void *ina_mz_wp_alloc_func(void *opaque, size_t items, size_t size)
{
    ina_mempool_t *pool = (ina_mempool_t*)opaque;
    return ina_mempool_nalloc(pool, items * size);
}
static void ina_mz_wp_free_func(void *opaque, void *address)
{
    INA_UNUSED(opaque);
    INA_UNUSED(address);
    /* we do not free using a pool */
}

static ina_rc_t ina_compression_compress_miniz(ina_compression_state_t *state, const unsigned char *src, 
                                               unsigned char *dst, int dst_len, int *wrote_len, int *read_len, int more)
{
    int status;
    mz_stream *stream;
    
    INA_ASSERT_NOT_NULL(state);
    INA_ASSERT_NOT_NULL(wrote_len);

    *wrote_len = 0;
    stream = (mz_stream*)state->statedata;
    INA_ASSERT_NOT_NULL(stream);

    if (!state->initialized) {
        ina_mem_set(stream, 0, sizeof(stream));
        if (state->mempool) {
            stream->opaque = state->mempool;
            stream->zalloc = ina_mz_wp_alloc_func;
            stream->zfree = ina_mz_wp_free_func;
        } else {
            stream->zalloc = ina_mz_wop_alloc_func;
            stream->zfree = ina_mz_wop_free_func;
        }
        status = mz_deflateInit(stream, state->flags);
        if (status != MZ_OK) {
            return INA_ERROR(INA_ES_COMPRESSION|INA_ERR_NOT_INITIALIZED);
        }
        state->initialized = INA_YES;
    }

    stream->next_in = src;
    stream->avail_in = (mz_uint32)state->chunk_src_len;
    stream->next_out = dst;
    stream->avail_out = (mz_uint32)dst_len;
        
    status = mz_deflate(stream, (more?MZ_SYNC_FLUSH:MZ_FINISH));
    
    *wrote_len = stream->total_out;
    *read_len = stream->total_in;

    if (status != MZ_STREAM_END && status != MZ_OK) {
        return INA_ERROR(INA_ES_DECOMPRESSION|INA_ERR_FAILED);
    }
    
    if (!more) {
        state->initialized = INA_NO;
        state->finalized = INA_YES;
        if (mz_deflateEnd(stream) != MZ_OK) {
            return INA_ERROR(INA_ES_COMPRESSION|INA_ERR_FAILED);
        }
    }
    return INA_SUCCESS;
}

static ina_rc_t ina_compression_decompress_miniz(ina_compression_state_t *state, const unsigned char *src, 
                                                 int src_len, unsigned char *dst, int dst_len, int *wrote_len, int *read_len, int more)
{
    int status;
    mz_stream *stream;

    INA_ASSERT_NOT_NULL(state);
    INA_ASSERT_NOT_NULL(wrote_len);

    *wrote_len = 0;
    *read_len = 0;
    stream = (mz_stream*)state->statedata;
    INA_ASSERT_NOT_NULL(stream);

    if (!state->initialized) {
        ina_mem_set(stream, 0, sizeof(stream));
        if (state->mempool) {
            stream->opaque = state->mempool;
            stream->zalloc = ina_mz_wp_alloc_func;
            stream->zfree = ina_mz_wp_free_func;
        } else {
            stream->zalloc = ina_mz_wop_alloc_func;
            stream->zfree = ina_mz_wop_free_func;
        }
        status = mz_inflateInit2(stream, state->flags);
        if (status != MZ_OK) {
            return INA_ERROR(INA_ES_DECOMPRESSION|INA_ERR_NOT_INITIALIZED);
        }
        state->initialized = INA_YES;
    }

    stream->next_in = src;
    stream->avail_in = (mz_uint32)src_len;
    stream->next_out = dst;
    stream->avail_out = (mz_uint32)dst_len;

    status = mz_inflate(stream, (more?MZ_SYNC_FLUSH:MZ_FINISH));
    *wrote_len = dst_len - stream->avail_out;
    *read_len = src_len - stream->avail_in;
    
    if (status != MZ_OK && status != MZ_STREAM_END) {
        return INA_ERROR(INA_ES_DECOMPRESSION|INA_ERR_FAILED);
    }
 
    if (!more) {
        state->initialized = INA_NO;
        state->finalized = INA_YES;
        if (mz_inflateEnd(stream) != MZ_OK) {
            return INA_ERROR(INA_ES_DECOMPRESSION|INA_ERR_FAILED);
        }
    }
    return INA_SUCCESS;
}

static ina_rc_t ina_compression_bounds_miniz(struct ina_compression_state_s *state, int *dst_len)
{
    INA_ASSERT_NOT_NULL(state);

    *dst_len = mz_compressBound(state->chunk_src_len);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_compression_new(ina_compression_state_t **state, ina_compression_type_t type,
                                      ina_compression_mode_t mode)
{
    return ina_compression_new_using_pool(state, type, mode, NULL);
}

INA_API(ina_rc_t) ina_compression_new_using_pool(ina_compression_state_t **state, ina_compression_type_t type,
                                                 ina_compression_mode_t mode, ina_mempool_t *pool)
{
    size_t sstate = 0;
    INA_VERIFY_NOT_NULL(state);

    *state = NULL;

    if (pool != NULL) {
        *state = (ina_compression_state_t*)ina_mempool_dalloc(pool, sizeof(struct ina_compression_state_s));
    }
    else {
        *state = (ina_compression_state_t*)ina_mem_alloc(sizeof(struct ina_compression_state_s));
    }
    INA_RETURN_IF(*state == NULL);

    (*state)->type = type;
    (*state)->chunk_src_len = 0;
	(*state)->chunk_proposed_dst_len = 0;
    (*state)->flags = 0;
	(*state)->initialized = 0;
	(*state)->finalized = 0;
	(*state)->more = 0;
    switch (type) {
        case INA_COMPRESSION_TYPE_DEFLATE:
        case INA_COMPRESSION_TYPE_DEFLATE_RAW:
            (*state)->compress_fn = ina_compression_compress_miniz;
            (*state)->decompress_fn = ina_compression_decompress_miniz;
            (*state)->dest_len_fn = ina_compression_bounds_miniz;
            sstate = sizeof(mz_stream);
            if (type == INA_COMPRESSION_TYPE_DEFLATE) {
                (*state)->flags = MZ_DEFAULT_WINDOW_BITS;
            } else {
                (*state)->flags = -MZ_DEFAULT_WINDOW_BITS;
            }
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
        default:
            INA_ASSERT_TRUE(0);
            break;
    }

    (*state)->mempool = pool;
    if (pool != NULL) {
        (*state)->statedata = ina_mempool_dalloc(pool, sstate);
    }
    else {
        (*state)->statedata = ina_mem_alloc(sstate);
    }

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_compression_free(ina_compression_state_t **state)
{
    INA_VERIFY_NOT_NULL(state);
    INA_VERIFY_NOT_NULL(*state);

    if ((*state)->mempool == NULL) {
        ina_mem_free((*state)->statedata);
    }

    ina_mem_free(*state);
    *state = NULL;

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_compression_compress_chunk(ina_compression_state_t *state, const unsigned char *src,
                                                 int src_len, unsigned char *dst, int dst_len, int *wrote_len, int *read_len, int more)
{
    INA_VERIFY_NOT_NULL(state);
    INA_VERIFY_NOT_NULL(src);
    INA_VERIFY_NOT_NULL(dst);
    INA_VERIFY(dst_len>0);
    INA_VERIFY_NOT_NULL(wrote_len);
    INA_VERIFY_NOT_NULL(read_len);

    state->chunk_src_len = src_len;
    return state->compress_fn(state, src, dst, dst_len, wrote_len, read_len, more);
}

INA_API(ina_rc_t) ina_compression_decompress_chunk(ina_compression_state_t *state, const unsigned char *src,
                                                   int src_len, unsigned char *dst, int dst_len, int *wrote_len, int *read_len, int more)
{
    INA_VERIFY_NOT_NULL(state);
    INA_VERIFY_NOT_NULL(src);
    INA_VERIFY_NOT_NULL(dst);
    INA_VERIFY(dst_len>0);
    INA_VERIFY_NOT_NULL(wrote_len);
    INA_VERIFY_NOT_NULL(read_len);

    return state->decompress_fn(state, src, src_len, dst, dst_len, wrote_len, read_len, more);
}

INA_API(ina_rc_t) ina_compression_get_destination_len(ina_compression_state_t *state, int src_len, int *len)
{
    INA_VERIFY_NOT_NULL(state);
    INA_VERIFY_NOT_NULL(len);

    state->chunk_src_len = src_len;
    state->dest_len_fn(state, &state->chunk_proposed_dst_len);
    *len = state->chunk_proposed_dst_len;

    return INA_SUCCESS;
}
