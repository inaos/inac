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

#define INA_HASHTABLE_BUCKET_SIZE 32


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
    ina_hash_type_t hash_type;
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
    ina_time_tsc_t *time;
    int id;
};

struct ina_hashtable_iter_s {
    ina_hashtable_t *ht;
    ina_hashtable_bucket_t *bucket;
    ina_hashtable_node_t   *node;
    int checksum;
};

struct ina_hashtable_event_consumer_s {
    ina_ullc_ctx_t *p_ctx;
    ina_ullc_ctx_t *c_ctx;
};

static int __hashtable_id[INA_HASHTABLE_MAX_STAT_TABLES] = {0};
static ina_str_t __cfg_filepath = NULL;

INA_INLINE ina_rc_t __ina_set_hashtable_id(ina_hashtable_t *ht)
{
    INA_ASSERT(ht->id == 0);
    while (__hashtable_id[ht->id] != 0 && ht->id < (INA_HASHTABLE_MAX_STAT_TABLES)) {
        ++ht->id;
    }
    ++ht->id;
    if (ht->id > INA_HASHTABLE_MAX_STAT_TABLES) {
        ht->id = 0;
        return INA_OS_ERROR(INA_ERR_EXCEEDED);
    }
    __hashtable_id[ht->id-1] = 1;
    return INA_SUCCESS;
}

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

INA_INLINE void __ina_push_event(const ina_hashtable_t *ht, uint32_t event, uint64_t data1, uint64_t data2)
{
    ina_ullc_ctx_t *ullc_ctx = ht->ullc_ctx;
    if (ullc_ctx) {
        ina_hashtable_event_t *e;
		time_t secs = 0;
		long nanos = 0;
        ina_time_read_tsc_clock(ht->time);
		ina_time_tsc_seconds_nanos(ht->time, &secs, &nanos);
        e =INA_ULLC_CLAIM(ina_hashtable_event_t, ullc_ctx);
        e->ts = (uint64_t)(secs*1000*1000*1000)+nanos;
        e->event_id = event;
        e->hashtable_id = ht->id;
        e->data1 = data1;
        e->data2 = data2;
        INA_ULLC_COMMIT(ullc_ctx);
        INA_ULLC_SIGNAL_RELEASE(ullc_ctx);
    }
}

#define __INA_SET_BEGIN(ht) __ina_push_event(ht, INA_HASHTABLE_EVENT_SET_BEGIN, 0, 0)
#define __INA_SET_END(ht, hash, count) __ina_push_event(ht, INA_HASHTABLE_EVENT_SET_END, (hash), (count))
#define __INA_GET_BEGIN(ht) __ina_push_event(ht, INA_HASHTABLE_EVENT_GET_BEGIN, 0, 0)
#define __INA_GET_END(ht, hash, missed) __ina_push_event(ht, INA_HASHTABLE_EVENT_GET_END, (hash), (missed))
#define __INA_REMOVE_BEGIN(ht) __ina_push_event(ht, INA_HASHTABLE_EVENT_REMOVE_BEGIN, 0, 0)
#define __INA_REMOVE_END(ht, missed, count) __ina_push_event(ht, INA_HASHTABLE_EVENT_REMOVE_END, (missed), (count))
#define __INA_FREE(ht) __ina_push_event(ht, INA_HASHTABLE_EVENT_FREE, 0, 0)
#define __INA_NEW(ht, hash_type, buckets) __ina_push_event(ht, INA_HASHTABLE_EVENT_NEW, (hash_type), (buckets))
#define __INA_EXPAND(ht, count) __ina_push_event(ht, INA_HASHTABLE_EVENT_EXPANSION, 0, (count))
#define __INA_META(ht, type, value) __ina_push_event(ht, INA_HASHTABLE_EVENT_META, (type), (value))


INA_API(ina_rc_t) ina_hashtable_init(const char* cfg_filepath)
{
    INA_INIT_GUARD();
    if (cfg_filepath != NULL) {
        __cfg_filepath = ina_str_new_fromcstr(cfg_filepath);
    }
    return INA_SUCCESS;
}

INA_API(void) ina_hashtable_destroy(void)
{
    INA_DESTROY_GUARD();
    INA_STR_FREE_SAFE(__cfg_filepath);
}

INA_API(ina_rc_t) ina_hashtable_new_from_cfg(ina_hashtable_key_type_t key_type, const char *name, ina_hashtable_t **ht)
{
    ina_hashtable_t htc;
    ina_conffile_t *cf = NULL;
    ina_str_t value = NULL;
    double dbl_value = 0.0;

    htc.capacity = INA_HASHTABLE_DEFAULT_CAPACITY;
    htc.hash_type = INA_HASH_DEFAULT+1;
    htc.cf = INA_HASHTABLE_CF_DEFAULT;
    INA_CONFFILE(cf, __cfg_filepath, NULL,
            INA_CONFFILE_NAMED_SECTION("hashtable", INA_YES, NULL,
                    INA_CONFFILE_STRING_KEY("key_type", INA_NO),
                    INA_CONFFILE_STRING_KEY("type", INA_NO),
                    INA_CONFFILE_STRING_KEY("hash_func", INA_NO),
                    INA_CONFFILE_STRING_KEY("growth_strategy", INA_NO),
                    INA_CONFFILE_STRING_KEY("shrink_strategy", INA_NO),
                    INA_CONFFILE_NUMBER_KEY("capacity", INA_NO),
                    INA_CONFFILE_NUMBER_KEY("preallocated", INA_NO),
                    INA_CONFFILE_NUMBER_KEY("stats_enabled", INA_NO)));

    if (INA_SUCCEED(ina_conffile_get_string(cf, "hashtable", name, "hash_func", &value))) {
        ina_hash_type(value, &htc.hash_type);
    }
    if (INA_SUCCEED(ina_conffile_get_number(cf, "hashtable", name, "capacity", &dbl_value))) {
        htc.capacity = (int)dbl_value;
    }
    if (INA_SUCCEED(ina_conffile_get_number(cf, "hashtable", name, "preallocated", &dbl_value))) {
        int f = (int)dbl_value;
        if (f) {
            htc.cf |= INA_HASHTABLE_CF_PREALLOCATED;
        }
    }
    if (INA_SUCCEED(ina_conffile_get_number(cf, "hashtable", name, "stats_enabled", &dbl_value))) {
        int f = (int)dbl_value;
        if (f) {
            htc.cf |= INA_HASHTABLE_CF_STAT;
        }
    }
    ina_conffile_free(&cf);

    return ina_hashtable_new(key_type,
            htc.hash_type,
            INA_HASHTABLE_TYPE_DEFAULT,
            INA_HASHTABLE_GROW_DEFAULT,
            INA_HASHTABLE_SHRINK_DEFAULT,
            htc.capacity,
            htc.cf,
            ht);
}

INA_API(ina_rc_t) ina_hashtable_new(ina_hashtable_key_type_t key_type,
                                    ina_hash_type_t hash_type,
                                    ina_hashtable_type_t type,
                                    ina_hashtable_growth_strategy_t growth_strategy,
                                    ina_hashtable_shrink_strategy_t shrink_strategy,
                                    int capacity,
                                    uint32_t  cf,
                                    ina_hashtable_t **ht)
{
    int i;
    ina_hashtable_bucket_t *b;
    size_t size;

    INA_VERIFY_NOT_NULL(ht);

    INA_UNUSED(type);
    INA_UNUSED(growth_strategy);
    INA_UNUSED(shrink_strategy);

    *ht = ina_mem_alloc(sizeof(ina_hashtable_t));
    INA_RETURN_IF_NULL(*ht);
    ina_mem_set((*ht), 0, sizeof(ina_hashtable_t));

    if (capacity > 0) {
        (*ht)->capacity = capacity;
    } else {
        (*ht)->capacity = 256;
    }
    (*ht)->key_type = key_type;
    switch ((*ht)->key_type) {
        case INA_HASHTABLE_UINT32_KEY: {
            (*ht)->key_len = sizeof(uint32_t);
            break;
        }
        case INA_HASHTABLE_INT32_KEY:  {
            (*ht)->key_len = sizeof(int32_t);
            break;
        }
        case INA_HASHTABLE_INT64_KEY: {
            (*ht)->key_len = sizeof(int64_t);
            break;
        }
        case INA_HASHTABLE_UINT64_KEY: {
            (*ht)->key_len = sizeof(uint64_t);
            break;
        }
        case INA_HASHTABLE_STR_KEY: {
            (*ht)->key_len = INA_HASHTABLE_MAX_KEY_LEN;
            break;
        }
        case INA_HASHTABLE_PTR_KEY: {
            (*ht)->key_len = sizeof(void*);
            break;
        }
        default:
            return INA_ERROR(INA_ERR_INVALID);
    }

    (*ht)->hash_type = hash_type;
    switch ((*ht)->hash_type) {
        case INA_HASH_DEFAULT:
            (*ht)->hash_type++;
        case INA_HASH32_CRC:
            (*ht)->hash32_fn = ina_hash_crc32;
            break;
        case INA_HASH32_LOOKUP3:
            (*ht)->hash32_fn = ina_hash_32_lookup3;
            break;
        case INA_HASH32_DJB:
            (*ht)->hash32_fn = ina_hash_32_djb;
            break;
        case INA_HASH32_JENKINS_OOAT:
            (*ht)->hash32_fn = ina_hash_32_jenkins_ooat;
            break;
        case INA_HASH32_FNV:
            (*ht)->hash32_fn = ina_hash_32_fnv;
            break;
        case INA_HASH32_SUPERFAST:
            (*ht)->hash32_fn = ina_hash_32_superfast;
            break;
        case INA_HASH32_SDBM:
            (*ht)->hash32_fn = ina_hash_sdbm;
            break;
        case INA_HASH32_FNV_YOSHIMITSU:
            (*ht)->hash32_fn = ina_hash_32_fnv_yoshimitsu;
            break;
        case INA_HASH32_MURMUR3:
            (*ht)->hash32_fn = ina_hash_32_memhash;
            break;
        case INA_HASH32_SPOOKY:
            (*ht)->hash32_fn = ina_hash_32_spooky;
            break;
        case INA_HASH32_XXHASH:
            (*ht)->hash32_fn = ina_hash_32_xxhash;
            break;
        case INA_HASH32_CRC_HW:
            (*ht)->hash32_fn = ina_hash_32_crc_hw;
            break;
        case INA_HASH32_MEMMASH:
            (*ht)->hash32_fn = ina_hash_32_memhash;
            break;
        case INA_HASH32_FALKHASH:
            (*ht)->hash32_fn = ina_hash_32_falkhash;
            break;
        case INA_HASH32_T1HA0:
            (*ht)->hash32_fn = ina_hash_32_t1ha0;
            break;
        case INA_HASH32_T1HA1:
            (*ht)->hash32_fn = ina_hash_32_t1ha1;
            break;
#ifdef INA_CPU_X86_64
        case INA_HASH64_LOCKUP3:
            (*ht)->hash64_fn = ina_hash_64_lookup3;
            break;
        case INA_HASH64_FNV:
            (*ht)->hash64_fn = ina_hash_64_fnv;
            break;
        case INA_HASH64_SPOOKY:
            (*ht)->hash64_fn = ina_hash_64_spooky;
            break;
        case INA_HASH64_XXHASH:
            (*ht)->hash64_fn = ina_hash_64_xxhash;
            break;
        case INA_HASH64_CRC_HW:
            (*ht)->hash64_fn = ina_hash_64_crc_hw;
            break;
        case INA_HASH64_MEMMASH:
            (*ht)->hash64_fn = ina_hash_64_memhash;
            break;
        case INA_HASH64_FALKHASH:
            (*ht)->hash64_fn = ina_hash_64_falkhash;
            break;
        case INA_HASH64_T1HA0:
            (*ht)->hash64_fn = ina_hash_64_t1ha0;
            break;
        case INA_HASH64_T1HA1:
            (*ht)->hash64_fn = ina_hash_64_t1ha1;
            break;
#endif
    }
    (*ht)->cf = cf;


    if ((*ht)->cf&INA_HASHTABLE_CF_PREALLOCATED) {
        size = (sizeof(ina_hashtable_bucket_t) * ((*ht)->capacity+1)) +
                (sizeof(ina_hashtable_node_t) * (*ht)->capacity * (INA_HASHTABLE_BUCKET_SIZE+1) * 2);
    } else {
        size = sizeof(ina_hashtable_bucket_t) * ((*ht)->capacity+1);
    }

    if (INA_FAILED(ina_mempool_new(size, NULL, INA_MEM_DYNAMIC, &(*ht)->mp))) {
        ina_hashtable_free(ht);
        return ina_err_get_last_rc();
    }

    (*ht)->buckets = ina_mempool_dalloc((*ht)->mp, sizeof(ina_hashtable_bucket_t) * ((*ht)->capacity+1));
    if ((*ht)->buckets == NULL) {
        ina_hashtable_free(ht);
    }

    if ((*ht)->cf&INA_HASHTABLE_CF_PREALLOCATED) {
        for (i = 0; i<(*ht)->capacity; ++i) {
            b = (*ht)->buckets + i;
            b->nodes = ina_mempool_dalloc((*ht)->mp, sizeof(ina_hashtable_node_t) * (INA_HASHTABLE_BUCKET_SIZE+1));
            b->max = INA_HASHTABLE_BUCKET_SIZE;
        }
    }


    if ((*ht)->cf&INA_HASHTABLE_CF_STAT) {
        if (INA_FAILED(__ina_set_hashtable_id((*ht)))) {
            ina_hashtable_free(ht);
            return ina_err_get_last_rc();
        }
        if (INA_FAILED(INA_ULLC_PRODUCER_NEW(ina_hashtable_event_t,
                                                          1, 4096, INA_HASHTABLE_MAX_STAT_TABLES,
                                                          INA_HASHTABLE_MAX_STAT_TABLES, "/ina_htmon",
                                                          INA_ULLC_WS_SIGNAL_WAIT,
                                                          &(*ht)->ullc_ctx))) {
            ina_hashtable_free(ht);
            return ina_err_get_last_rc();
        }

        if (INA_FAILED(ina_time_tsc_new(&(*ht)->time))) {
            ina_mem_free(*ht);
            return ina_err_get_last_rc();
        }

        __INA_NEW(*ht, (*ht)->key_type, (uint64_t)(*ht)->capacity);
        __INA_META(*ht, 1, (*ht)->hash_type);
        __INA_META(*ht, 2, (*ht)->key_len);
        __INA_META(*ht, 3, INA_HASHTABLE_TYPE_CHAINED);
        __INA_META(*ht, 4, (*ht)->cf);
        __INA_META(*ht, 5, sizeof(ina_hashtable_bucket_t));
        __INA_META(*ht, 6, sizeof(ina_hashtable_node_t));

    }
    return INA_SUCCESS;
}

INA_API(void) ina_hashtable_free(ina_hashtable_t **ht)
{
	INA_FREE_CHECK(ht);
	__INA_FREE(*ht);
	ina_ullc_producer_free(&(*ht)->ullc_ctx);
	ina_time_tsc_free(&(*ht)->time);
	__hashtable_id[(*ht)->id] = 0;
	ina_mempool_free(&(*ht)->mp);
	INA_MEM_FREE_SAFE(*ht);
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
    INA_RETURN_IF_FAILED(ina_mempool_info(ht->mp, &info));
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
        bucket->nodes = ina_mempool_dalloc(ht->mp, (bucket->max + INA_HASHTABLE_BUCKET_SIZE+1) * sizeof(ina_hashtable_node_t));
        if (nodes != NULL) {
            ina_mem_cpy(bucket->nodes, nodes, bucket->max * sizeof(ina_hashtable_node_t));
        }
        bucket->max += INA_HASHTABLE_BUCKET_SIZE;
        __INA_EXPAND(ht, (uint64_t)bucket->max);
    }

    next = bucket->nodes;
    free = NULL;

    while (next->key.u32 && (next-bucket->nodes) <  bucket->count) {
        if ((0 == ina_mem_cmp(&next->key, key, key_len))) {
            next->data =(void*)data;
            __INA_SET_END(ht, bucket->hash, 0);
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
    __INA_SET_END(ht, bucket->hash, (uint64_t)ht->count);

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
            __INA_GET_END(ht, bucket->hash, 0);
            return INA_SUCCESS;
        }
        next++;
    }
    __INA_GET_END(ht, bucket->hash, 1);
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
            __INA_REMOVE_END(ht, 0, (uint64_t)ht->count);
            return INA_SUCCESS;
        }
        next++;
    }
    __INA_REMOVE_END(ht, 1, (uint64_t)ht->count);
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

INA_API(ina_rc_t) ina_hashtable_foreach_arg(ina_hashtable_t* ht, ina_foreach_arg_fn_t foreach_fn, void *arg)
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
                    if (INA_FAILED(foreach_fn(next->data, arg))) {
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
    iter->checksum = iter->ht->count;
    iter->node = iter->bucket->nodes;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_hashtable_event_consumer_new(ina_hashtable_event_consumer_t **event_consumer, uint32_t flags)
{
    INA_UNUSED(flags);
    INA_VERIFY_NOT_NULL(event_consumer);

    *event_consumer = ina_mem_alloc(sizeof(ina_hashtable_event_consumer_t));
    INA_RETURN_IF_NULL(*event_consumer);

    if (INA_SUCCEED(INA_ULLC_PRODUCER_NEW(ina_hashtable_event_t,
                                               1, 4096,
                                               INA_HASHTABLE_MAX_STAT_TABLES,
                                               INA_HASHTABLE_MAX_STAT_TABLES,
                                               "/ina_htmon", INA_ULLC_WS_SIGNAL_WAIT,
                                               &(*event_consumer)->p_ctx)) &&
        INA_SUCCEED(INA_ULLC_CONSUMER_NEW(ina_hashtable_event_t,
                                               1, 4096,
                                               INA_HASHTABLE_MAX_STAT_TABLES,
                                               INA_HASHTABLE_MAX_STAT_TABLES,
                                               "/ina_htmon", &(*event_consumer)->c_ctx))) {
        return INA_SUCCESS;
    }

    ina_ullc_consumer_free(&(*event_consumer)->c_ctx);
    ina_ullc_producer_free(&(*event_consumer)->p_ctx);
    INA_MEM_FREE_SAFE(*event_consumer);
    return ina_err_get_last_rc();
}

INA_API(void) ina_hashtable_event_consumer_free(ina_hashtable_event_consumer_t **event_consumer)
{
    INA_FREE_CHECK(event_consumer);
    ina_ullc_producer_free(&(*event_consumer)->p_ctx);
    ina_ullc_consumer_free(&(*event_consumer)->c_ctx);
}

INA_API(ina_rc_t) ina_hashtable_event_consumer_next(ina_hashtable_event_consumer_t *event_consumer, ina_hashtable_event_t **event)
{
    INA_VERIFY_NOT_NULL(event);
    *event = INA_ULLC_GET(ina_hashtable_event_t, event_consumer->c_ctx);
    if (*event == NULL) {
        return INA_ERROR(INA_ERR_TRY_AGAIN);
    }
    return INA_SUCCESS;
}
