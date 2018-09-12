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

typedef uint32_t (*__ina_hash_fn)(const void*, size_t key_len);

struct ina_hashtable_ctx_s {
    size_t             key_len;
    int                key_type;
    ina_hash_func_32_t hash32_fn;
    ina_hash_func_64_t hash64_fn;
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
    int32_t               free;
} ina_hashtable_bucket_t;

struct ina_hashtable_s {
    ina_mempool_t *mp;
    int capacity;
    size_t key_len;
    ina_hashtable_key_type_t key_type;
    ina_hashtable_bucket_t *buckets;
    ina_hash_func_32_t hash32_fn;
    ina_hash_func_64_t hash64_fn;
};

INA_INLINE ina_hashtable_bucket_t* __ina_bucket(const ina_hashtable_t *ht, const void* key, size_t key_len)
{
    if (ht->hash32_fn) {
        uint32_t h = ht->hash32_fn(0, key, key_len) % ht->capacity;
        return ht->buckets + h;
    } else {
        uint64_t h = ht->hash64_fn(0, key, key_len) % ht->capacity;
        return ht->buckets + h;
    }
}


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
        case INA_HASHTABLE_HASH_CRC: {
            (*ctx)->hash32_fn = ina_hash_crc32;
            break;
        }
        case INA_HASHTABLE_HASH_SDBM: {
            (*ctx)->hash32_fn = ina_hash_sdbm;
            break;
        }
        case INA_HASHTABLE_HASH_SPOOKY64: {
            (*ctx)->hash64_fn = ina_hash_64_spooky;
            break;
        }
        case INA_HASHTABLE_HASH_SPOOKY32: {
            (*ctx)->hash32_fn = ina_hash_32_spooky;
            break;
        }
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_hashtable_destroy(ina_hashtable_ctx_t **ctx)
{
    INA_VERIFY_NOT_NULL(*ctx);
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
    INA_VERIFY_NOT_NULL(ctx);
    *ht = ina_mem_alloc(sizeof(ina_hashtable_t));
    INA_RETURN_IF_NULL(*ht);

    (*ht)->capacity = capacity;
    (*ht)->key_len = ctx->key_len;
    (*ht)->key_type = ctx->key_type;
    (*ht)->hash32_fn = ctx->hash32_fn;
    (*ht)->hash64_fn = ctx->hash64_fn;

    if (INA_FAILED(ina_mempool_new(&(*ht)->mp, (sizeof(ina_hashtable_node_t) * (*ht)->capacity * INA_HASHTABLE_BUCKET_SIZE *2),
            0, NULL))) {
        ina_hashtable_free(ht);
        return ina_err_get_last_rc();
    }
    (*ht)->buckets = ina_mempool_dalloc((*ht)->mp, sizeof(ina_hashtable_bucket_t) * (*ht)->capacity);
    INA_RETURN_IF_NULL((*ht)->buckets);

    for (i = 0; i<(*ht)->capacity; ++i) {
        b = (*ht)->buckets + i;
        b->nodes = ina_mempool_dalloc((*ht)->mp, sizeof(ina_hashtable_node_t) * INA_HASHTABLE_BUCKET_SIZE);

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

INA_API(ina_rc_t) ina_hashtable_set(ina_hashtable_t *ht, const void *key, size_t key_len, const void *data)
{
    ina_hashtable_bucket_t *bucket;
    ina_hashtable_node_t *next;
    ina_hashtable_node_t *free;

    INA_VERIFY_NOT_NULL(ht);
    INA_VERIFY_NOT_NULL(key);
    INA_VERIFY_NOT_NULL(data);

    bucket = __ina_bucket(ht, key, key_len);
    next = bucket->nodes;
    free = NULL;
    while (next->key.u32 && (next-bucket->nodes) <  bucket->count) {
        if ((0 == ina_mem_cmp(&next->key, key, key_len))) {
            next->data =(void*)data;
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

    if ((bucket->count%INA_HASHTABLE_BUCKET_SIZE) == 0) {
        void *nodes = bucket->nodes;
        bucket->nodes = ina_mempool_dalloc(ht->mp, INA_HASHTABLE_BUCKET_SIZE*bucket->count*2* sizeof(ina_hashtable_node_t));
        ina_mem_cpy(bucket->nodes, nodes, bucket->count* sizeof(ina_hashtable_node_t));
    }

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_hashtable_get(const ina_hashtable_t *ht, const void *key, size_t key_len, void **data)
{
    ina_hashtable_bucket_t *bucket;
    ina_hashtable_node_t *next;

    INA_VERIFY_NOT_NULL(ht);
    INA_VERIFY_NOT_NULL(key);
    INA_VERIFY_NOT_NULL(data);

    bucket = __ina_bucket(ht, key, key_len);
    next = bucket->nodes;
    while (next->key.u32 && (next-bucket->nodes) <  bucket->count) {
        if (next->data != NULL && (0 == ina_mem_cmp(&next->key, key, key_len))) {
            *data = next->data;
            return INA_SUCCESS;
        }
        next++;
    }
    return INA_ERROR(INA_ERR_NOT_FOUND);
}

INA_API(ina_rc_t) ina_hashtable_remove(ina_hashtable_t *t,  const void *key, size_t key_len, void **data)
{
    ina_hashtable_bucket_t *bucket;
    ina_hashtable_node_t *next;

    INA_VERIFY_NOT_NULL(t);
    INA_VERIFY_NOT_NULL(key);
    INA_VERIFY_NOT_NULL(data);

    bucket = __ina_bucket(t, key, key_len);
    next = bucket->nodes;
    while (next->key.u32 && (next-bucket->nodes) <  bucket->count) {
        if (next->data != NULL && (0 == ina_mem_cmp(&next->key, key, key_len))) {
            *data = next->data;
            next->data = NULL;
            bucket->free++;
            return INA_SUCCESS;
        }
        next++;
    }
    return INA_ERROR(INA_ERR_NOT_FOUND);
}

INA_API(ina_rc_t) ina_hashtable_foreach(ina_hashtable_t* ht, ina_hashtable_foreach_fn foreach_fn)
{
    ina_hashtable_bucket_t *bucket;
    ina_hashtable_node_t *next;

    INA_VERIFY_NOT_NULL(ht);
    INA_VERIFY_NOT_NULL(foreach_fn);

    bucket = ht->buckets;
    while (bucket-ht->buckets < ht->capacity) {
        if (bucket->count > 0) {
            next = bucket->nodes;
            while (next-bucket->nodes < bucket->count) {
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
