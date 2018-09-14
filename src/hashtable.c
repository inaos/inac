/*
 * Copyright (c) 2018, INAOS GmbH
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

#define INA_HASHTABLE_BUCKET_SIZE 32

struct ina_hashtable_ctx_s {
    size_t             key_len;
    int                key_type;
    ina_hash_func_32_t hash32_fn;
    ina_hash_func_64_t hash64_fn;
    uint32_t           cf;
    ina_ullc_ctx_t     *ullc_ctx;
};

typedef struct ina_hashtable_node_s {
    union {
        uint32_t  u32;
        int32_t   i32;
        uint64_t  u64;
        int64_t   i64;
        uintptr_t ptr;
        char      str[INA_HASHTABLE_MAX_KEY_LEN];
    } key;
    void *data;
} ina_hashtable_node_t;

typedef struct in_hashtable_bucket_s {
    ina_hashtable_node_t *nodes;
    int32_t               count;
    int32_t               max;
    int32_t               free;
    uint64_t              hash;
} ina_hashtable_bucket_t;

struct ina_hashtable_s {
    ina_hashtable_ctx_t *ctx;
    ina_mempool_t *mp;
    int capacity;
    size_t key_len;
    ina_hashtable_key_type_t key_type;
    ina_hashtable_bucket_t *buckets;
    ina_hash_func_32_t hash32_fn;
    ina_hash_func_64_t hash64_fn;
    uint32_t cf;
    int32_t count;
    ina_ullc_ctx_t *ullc_ctx;
};

struct ina_hashtable_iter_s {
    ina_hashtable_t *ht;
    ina_hashtable_bucket_t *bucket;
    ina_hashtable_node_t   *node;
    int checksum;
};

INA_INLINE ina_hashtable_bucket_t* __ina_bucket(const ina_hashtable_t *ht, const void* key, size_t key_len)
{
    ina_hashtable_bucket_t *bucket;
    if (ht->hash32_fn) {
        uint32_t h = ht->hash32_fn(0, key, key_len) % ht->capacity;
        bucket = ht->buckets + h;
        bucket->hash = h;
    } else {
        uint64_t h = ht->hash64_fn(0, key, key_len) % ht->capacity;
        bucket = ht->buckets + h;
        bucket->hash = h;
    }
    return bucket;
}

INA_INLINE void __ina_push_event(const ina_hashtable_t *ht, uint32_t event, uint32_t subject, uint64_t data)
{
    ina_ullc_ctx_t *ullc_ctx = ht->ullc_ctx;
    if (ullc_ctx) {
        ina_hashtable_event_t *e = INA_ULLC_CLAIM(ina_hashtable_event_t, ullc_ctx);
        e->event_id = event;
        e->hashtable_id = subject;
        e->data = data;
        INA_ULLC_COMMIT(ullc_ctx);
        INA_ULLC_SIGNAL_RELEASE(ullc_ctx);
    }
}

#define __INA_SET_BEGIN(ht) __ina_push_event(ht, INA_HASHTABLE_EVENT_SET_BEGIN, 1, 0)
#define __INA_SET_END(ht) __ina_push_event(ht, INA_HASHTABLE_EVENT_SET_END, 1, 0)
#define __INA_GET_BEGIN(ht) __ina_push_event(ht, INA_HASHTABLE_EVENT_GET_BEGIN, 1, 0)
#define __INA_GET_END(ht) __ina_push_event(ht, INA_HASHTABLE_EVENT_GET_END, 1, 0)
#define __INA_REMOVE_BEGIN(ht) __ina_push_event(ht, INA_HASHTABLE_EVENT_SET_BEGIN, 1, 0)
#define __INA_REMOVE_END(ht) __ina_push_event(ht, INA_HASHTABLE_EVENT_SET_END, 1, 0)

INA_API(ina_rc_t) ina_hashtable_init(ina_hashtable_key_type_t key_type,
                                     ina_hashtable_hash_type_t hash_type,
                                     ina_hashtable_type_t type,
                                     ina_hashtable_growth_strategy_t growth_strategy,
                                     uint32_t  cf,
                                     ina_hashtable_ctx_t **ctx)
{
    INA_VERIFY_NOT_NULL(ctx);
    *ctx = ina_mem_alloc(sizeof(ina_hashtable_ctx_t));
    INA_RETURN_IF_NULL(*ctx);
    ina_mem_set(*ctx, 0, sizeof(ina_hashtable_ctx_t));
    (*ctx)->cf = cf;
    (*ctx)->key_type = key_type;

    switch (key_type) {
        case INA_HASHTABL_UINT32_KEY: {
            (*ctx)->key_len = sizeof(uint32_t);
            break;
        }
        case INA_HASHTABL_INT32_KEY:  {
            (*ctx)->key_len = sizeof(int32_t);
            break;
        }
        case INA_HASHTABLE_INT64_KEY: {
            (*ctx)->key_len = sizeof(int64_t);
            break;
        }
        case INA_HASHTABLE_UINT64_KEY: {
            (*ctx)->key_len = sizeof(uint64_t);
            break;
        }
        case INA_HASHTABLE_STR_KEY: {
            (*ctx)->key_len = INA_HASHTABLE_MAX_KEY_LEN;
            break;
        }
        case INA_HASHTABLE_PTR_KEY: {
            (*ctx)->key_len = sizeof(void*);
            break;
        }
    }
    switch (hash_type) {
        case INA_HASHTABLE_HASH32_CRC:
            (*ctx)->hash32_fn = ina_hash_crc32;
            break;
        case INA_HASHTABLE_HASH32_LOOKUP3:
            (*ctx)->hash32_fn = ina_hash_32_lookup3;
            break;
        case INA_HASHTABLE_HASH32_DJB:
            (*ctx)->hash32_fn = ina_hash_32_djb;
            break;
        case INA_HASHTABLE_HASH32_JENKINS_OOAT:
            (*ctx)->hash32_fn = ina_hash_32_jenkins_ooat;
            break;
        case INA_HASHTABLE_HASH32_FNV:
            (*ctx)->hash32_fn = ina_hash_32_fnv;
            break;
        case INA_HASHTABLE_HASH32_SUPERFAST:
            (*ctx)->hash32_fn = ina_hash_32_superfast;
            break;
        case INA_HASHTABLE_HASH32_SDBM:
            (*ctx)->hash32_fn = ina_hash_sdbm;
            break;
        case INA_HASHTABLE_HASH32_FNV_YOSHIMITSU:
            (*ctx)->hash32_fn = ina_hash_32_fnv_yoshimitsu;
            break;
        case INA_HASHTABLE_HASH32_MURMUR3:
            (*ctx)->hash32_fn = ina_hash_32_memhash;
            break;
        case INA_HASHTABLE_HASH32_SPOOKY:
            (*ctx)->hash32_fn = ina_hash_32_spooky;
            break;
        case INA_HASHTABLE_HASH32_XXHASH:
            (*ctx)->hash32_fn = ina_hash_32_xxhash;
            break;
        case INA_HASHTABLE_HASH32_CRC_HW:
            (*ctx)->hash32_fn = ina_hash_32_crc_hw;
            break;
        case INA_HASHTABLE_HASH32_MEMMASH:
            (*ctx)->hash32_fn = ina_hash_32_memhash;
            break;
        case INA_HASHTABLE_HASH32_FALKHASH:
            (*ctx)->hash32_fn = ina_hash_32_falkhash;
            break;
        case INA_HASHTABLE_HASH32_T1HA0:
            (*ctx)->hash32_fn = ina_hash_32_t1ha0;
            break;
        case INA_HASHTABLE_HASH32_T1HA1:
            (*ctx)->hash32_fn = ina_hash_32_t1ha1;
            break;
#ifdef INA_CPU_X86_64
        case INA_HASHTABLE_HASH64_LOCKUP3:
            (*ctx)->hash64_fn = ina_hash_64_lookup3;
            break;
        case INA_HASHTABLE_HASH64_FNV:
            (*ctx)->hash64_fn = ina_hash_64_fnv;
            break;
        case INA_HASHTABLE_HASH64_SPOOKY:
            (*ctx)->hash64_fn = ina_hash_64_spooky;
            break;
        case INA_HASHTABLE_HASH64_XXHASH:
            (*ctx)->hash64_fn = ina_hash_64_xxhash;
            break;
        case INA_HASHTABLE_HASH64_CRC_HW:
            (*ctx)->hash64_fn = ina_hash_64_crc_hw;
            break;
        case INA_HASHTABLE_HASH64_MEMMASH:
            (*ctx)->hash64_fn = ina_hash_64_memhash;
            break;
        case INA_HASHTABLE_HASH64_FALKHASH:
            (*ctx)->hash64_fn = ina_hash_64_falkhash;
            break;
        case INA_HASHTABLE_HASH64_T1HA0:
            (*ctx)->hash64_fn = ina_hash_64_t1ha0;
            break;
        case INA_HASHTABLE_HASH64_T1HA1:
            (*ctx)->hash64_fn = ina_hash_64_t1ha1;
            break;
#endif
    }
    if ((*ctx)->cf&INA_HASHTABLE_CF_STAT) {
        INA_RETURN_IF_FAILED(INA_ULLC_PRODUCER_CREATE(ina_hashtable_event_t,
                1, 4096, 32, 32, "/ina_htmon",
                INA_ULLC_WS_SIGNAL_WAIT,
                &(*ctx)->ullc_ctx));
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_hashtable_destroy(ina_hashtable_ctx_t **ctx)
{
    INA_VERIFY_NOT_NULL(ctx);
    INA_VERIFY_NOT_NULL(*ctx);
    if ((*ctx)->ullc_ctx != NULL) {
        ina_ullc_producer_destroy(&(*ctx)->ullc_ctx);
    }
    ina_mem_free(*ctx);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_hashtable_new(ina_hashtable_ctx_t *ctx,
                                    int capacity,
                                    uint32_t  cf,
                                    ina_hashtable_t **ht)
{
    int i;
    ina_hashtable_bucket_t *b;
    size_t size;

    INA_VERIFY_NOT_NULL(ctx);
    INA_VERIFY_NOT_NULL(ht);

    *ht = ina_mem_alloc(sizeof(ina_hashtable_t));
    INA_RETURN_IF_NULL(*ht);
    ina_mem_set((*ht), 0, sizeof(ina_hashtable_t));
    (*ht)->ctx = ctx;
    (*ht)->capacity = capacity;
    (*ht)->key_len = ctx->key_len;
    (*ht)->key_type = ctx->key_type;
    (*ht)->hash32_fn = ctx->hash32_fn;
    (*ht)->hash64_fn = ctx->hash64_fn;
    (*ht)->cf = ctx->cf|cf;

    if ((*ht)->cf&INA_HASHTABLE_CF_PREALLOCATED) {
        size = (sizeof(ina_hashtable_bucket_t) * (*ht)->capacity) +
                (sizeof(ina_hashtable_node_t) * (*ht)->capacity * INA_HASHTABLE_BUCKET_SIZE * 2);
    } else {
        size = sizeof(ina_hashtable_bucket_t) * (*ht)->capacity;
    }

    if (INA_FAILED(ina_mempool_new(&(*ht)->mp, size, INA_MEM_DYNAMIC, NULL))) {
        ina_hashtable_free(ht);
        return ina_err_get_last_rc();
    }

    (*ht)->buckets = ina_mempool_dalloc((*ht)->mp, sizeof(ina_hashtable_bucket_t) * (*ht)->capacity);
    INA_RETURN_IF_NULL((*ht)->buckets);

    if ((*ht)->cf&INA_HASHTABLE_CF_PREALLOCATED) {
        for (i = 0; i<(*ht)->capacity; ++i) {
            b = (*ht)->buckets + i;
            b->nodes = ina_mempool_dalloc((*ht)->mp, sizeof(ina_hashtable_node_t) * INA_HASHTABLE_BUCKET_SIZE);
            b->max = INA_HASHTABLE_BUCKET_SIZE;
        }
    }

    if ((*ht)->cf&INA_HASHTABLE_CF_STAT) {
        (*ht)->ullc_ctx = ctx->ullc_ctx;
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_hashtable_free(ina_hashtable_t **ht)
{
    INA_VERIFY_NOT_NULL(ht);
    INA_VERIFY_NOT_NULL(*ht);
    ina_mempool_free(&(*ht)->mp);
    ina_mem_free((*ht));
    *ht = NULL;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_hashtable_clear(ina_hashtable_t *ht)
{
    ina_hashtable_bucket_t *bucket;

    INA_VERIFY_NOT_NULL(ht);
    bucket = ht->buckets;
    while (bucket-ht->buckets < ht->capacity) {
        bucket->count = 0;
        bucket->free = 0;
        bucket++;
    }
    ht->count = 0;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_hashtable_count(ina_hashtable_t *ht, int *count)
{
    INA_VERIFY_NOT_NULL(count);
    *count = ht->count;
    return INA_SUCCESS;
}


INA_API(ina_rc_t) ina_hashtable_usage(ina_hashtable_t *ht, size_t *usage)
{
    ina_mempool_info_t info;
    INA_VERIFY_NOT_NULL(usage);
    INA_RETURN_IF_FAILED(ina_mempool_getinfo(ht->mp, &info));
    *usage = info.size;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_hashtable_set(ina_hashtable_t *ht, const void *key, size_t key_len, const void *data)
{
    ina_hashtable_bucket_t *bucket;
    ina_hashtable_node_t *next;
    ina_hashtable_node_t *free;

    __INA_SET_BEGIN(ht);
    INA_VERIFY_NOT_NULL(ht);
    INA_VERIFY_NOT_NULL(key);
    INA_VERIFY_NOT_NULL(data);

    bucket = __ina_bucket(ht, key, key_len);

    if ((bucket->max-bucket->count) == 0) {
        void *nodes = bucket->nodes;
        bucket->nodes = ina_mempool_dalloc(ht->mp, (bucket->max + INA_HASHTABLE_BUCKET_SIZE) * sizeof(ina_hashtable_node_t));
        if (nodes != NULL) {
            ina_mem_cpy(bucket->nodes, nodes, bucket->max * sizeof(ina_hashtable_node_t));
        }
        bucket->max += INA_HASHTABLE_BUCKET_SIZE;
    }

    next = bucket->nodes;
    free = NULL;

    while (next->key.u32 && (next-bucket->nodes) <  bucket->count) {
        if ((0 == ina_mem_cmp(&next->key, key, key_len))) {
            next->data =(void*)data;
            __INA_SET_END(ht);
            return INA_SUCCESS;
        }
        if (next->data == NULL) {
            free = next;
        }
        next++;
    }
    if (free == NULL) {
        free = bucket->nodes+bucket->count;
        bucket->count++;
    }
    ina_mem_cpy(&free->key, key, key_len);
    free->data = (void*)data;
    ++ht->count;
    __INA_SET_END(ht);

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_hashtable_get(const ina_hashtable_t *ht, const void *key, size_t key_len, void **data)
{
    ina_hashtable_bucket_t *bucket;
    ina_hashtable_node_t *next;

    __INA_GET_BEGIN(ht);
    INA_VERIFY_NOT_NULL(ht);
    INA_VERIFY_NOT_NULL(key);
    INA_VERIFY_NOT_NULL(data);

    bucket = __ina_bucket(ht, key, key_len);
    next = bucket->nodes;
    while (next && next->key.u32 && (next-bucket->nodes) <  bucket->count) {
        if (next->data != NULL && (0 == ina_mem_cmp(&next->key, key, key_len))) {
            *data = next->data;
            __INA_GET_END(ht);
            return INA_SUCCESS;
        }
        next++;
    }
    __INA_GET_END(ht);
    return INA_ERROR(INA_ERR_NOT_FOUND);
}

INA_API(ina_rc_t) ina_hashtable_remove(ina_hashtable_t *ht,  const void *key, size_t key_len, void **data)
{
    ina_hashtable_bucket_t *bucket;
    ina_hashtable_node_t *next;

    __INA_REMOVE_BEGIN(ht);
    INA_VERIFY_NOT_NULL(ht);
    INA_VERIFY_NOT_NULL(key);
    INA_VERIFY_NOT_NULL(data);

    bucket = __ina_bucket(ht, key, key_len);
    next = bucket->nodes;
    while (next && next->key.u32 && (next-bucket->nodes) <  bucket->count) {
        if (next->data != NULL && (0 == ina_mem_cmp(&next->key, key, key_len))) {
            *data = next->data;
            next->data = NULL;
            ++bucket->free;
            --ht->count;
            __INA_REMOVE_END(ht);
            return INA_SUCCESS;
        }
        next++;
    }
    __INA_REMOVE_END(ht);
    return INA_ERROR(INA_ERR_NOT_FOUND);
}

INA_API(ina_rc_t) ina_hashtable_foreach(ina_hashtable_t* ht, ina_foreach_fn_t foreach_fn)
{
    ina_hashtable_bucket_t *bucket;
    ina_hashtable_node_t *next;

    INA_VERIFY_NOT_NULL(ht);
    INA_VERIFY_NOT_NULL(foreach_fn);

    bucket = ht->buckets;
    while (bucket-ht->buckets < ht->capacity) {
        if (bucket->count > 0) {
            next = bucket->nodes;
            while (next && next-bucket->nodes < bucket->count) {
                if (next->data) {
                    if (INA_FAILED(foreach_fn(next->data))) {
                        return ina_err_get_last_rc();
                    }
                }
                next++;
            }
        }
        bucket++;
    }
    return INA_SUCCESS;
}


INA_API(ina_rc_t) ina_hashtable_iter_new(ina_hashtable_t *ht, ina_hashtable_iter_t **iter)
{
    INA_VERIFY_NOT_NULL(iter);
    *iter = ina_mem_alloc(sizeof(ina_hashtable_iter_t));
    INA_RETURN_IF_NULL(*iter);
    ina_mem_set(*iter, 0, sizeof(ina_hashtable_iter_t));
    (*iter)->ht = ht;
    return ina_hashtable_iter_reset(*iter);

}

INA_API(ina_rc_t) ina_hashtable_iter_free(ina_hashtable_iter_t **iter)
{
    INA_VERIFY_NOT_NULL(iter);
    INA_VERIFY_NOT_NULL(*iter);
    ina_mem_free(*iter);
    *iter = NULL;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_hashtable_iter_next(ina_hashtable_iter_t *iter, void **data)
{
    INA_VERIFY_NOT_NULL(iter);
    INA_VERIFY_NOT_NULL(data);

    if (iter->checksum != iter->ht->count) {
        return INA_ERROR(INA_NN_STATE|INA_ERR_INVALID);
    }

    while (iter->bucket-iter->ht->buckets < iter->ht->capacity) {
        while (iter->node && iter->node-iter->bucket->nodes < iter->bucket->count) {
            if (iter->node->data != NULL) {
                *data = iter->node->data;
                iter->node++;
                return INA_SUCCESS;
            }
            iter->node++;
        }
        iter->bucket++;
        iter->node = iter->bucket->nodes;
    }
    return INA_ERROR(INA_ERR_END_OF);
}

INA_API(ina_rc_t) ina_hashtable_iter_reset(ina_hashtable_iter_t *iter)
{
    INA_VERIFY_NOT_NULL(iter);
    iter->bucket = iter->ht->buckets;
    iter->node = iter->bucket->nodes;
    iter->checksum = iter->ht->count;
    iter->node = NULL;
    return INA_SUCCESS;
}