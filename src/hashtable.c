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

#define INA_HASHTABLE_MAX_KEY_LEN 16
#define INA_HASHTABLE_BUCKET_SIZE 32

struct ina_hashtable_ctx_s {
    int key_len;
};

typedef struct ina_hashtable_node_s {
    union {
        uint32_t u32;
        int32_t  i32;
        uint64_t u64;
        int64_t  i64;
        char     str[INA_HASHTABLE_MAX_KEY_LEN];
    } key;
    void *data;
} ina_hashtable_node_t;


struct ina_hashtable_s {
    ina_hashtable_ctx_t *ctx;
    ina_mempool_t *mp;
    int capacity;
    ina_hashtable_node_t **buckets;
};

INA_API(ina_rc_t) ina_hashtable_new(ina_hashtable_ctx_t *ctx,
                                    int capacity,
                                    uint32_t  cf,
                                    ina_hashtable_t **t)
{
    INA_VERIFY_NOT_NULL(ctx);
    *t = ina_mem_alloc(sizeof(ina_hashtable_t));
    INA_RETURN_IF_NULL(*t);
    (*t)->capacity = capacity;
    if (INA_FAILED(ina_mempool_new(&(*t)->mp, (sizeof(ina_hashtable_node_t) * (*t)->capacity),
            0, NULL))) {
        ina_hashtable_free(t);
        return ina_err_get_last_rc();
    }

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_hashtable_free(ina_hashtable_t **t)
{
    INA_VERIFY_NOT_NULL(t);
    INA_VERIFY_NOT_NULL(*t);
    ina_mempool_free((*t)->mp);
    ina_mem_free((*t));
    *t = NULL;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_hashtable_set(ina_hashtable_t *t, const void *key, const void *data)
{
    ina_hashtable_node_t *next;

    INA_VERIFY_NOT_NULL(t);
    INA_VERIFY_NOT_NULL(t);
    INA_VERIFY_NOT_NULL(data);

    uint32_t hash = ina_hash_32_sdbm(0, key , sizeof(uint32_t)) % t->capacity;
    next = t->buckets[hash];
    ina_mem_cpy(&next->key, key, (size_t)t->ctx->key_len);
    next->data = (void*)data;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_hashtable_get(const ina_hashtable_t *t, const void *key, void **data)
{
    ina_hashtable_node_t *next;

    INA_VERIFY_NOT_NULL(t);
    INA_VERIFY_NOT_NULL(key);
    INA_VERIFY_NOT_NULL(data);
    uint32_t hash = ina_hash_32_sdbm(0, key , sizeof(uint32_t)) % t->capacity;
    next = t->buckets[hash];
    if (NULL != next->data) {
        return INA_ERROR(INA_ERR_NOT_FOUND);
    }
    *data = next->data;
    return  INA_SUCCESS;

}

INA_API(ina_rc_t) ina_hashtable_remove(ina_hashtable_t *t,  const void *key, void **data)
{
    ina_hashtable_node_t *next;

    INA_VERIFY_NOT_NULL(t);
    INA_VERIFY_NOT_NULL(key);
    INA_VERIFY_NOT_NULL(data);
    uint32_t hash = ina_hash_32_sdbm(0, key , sizeof(uint32_t)) % t->capacity;
    next = t->buckets[hash];
    if (NULL != next->data) {
        return INA_ERROR(INA_ERR_NOT_FOUND);
    }
    *data = next->data;
    ina_mem_set(next, 0, sizeof(ina_hashtable_node_t));
    return INA_SUCCESS;

}