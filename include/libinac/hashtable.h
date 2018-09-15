/*
 * Copyright (c) 2016-2018, INAOS GmbH
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
 
#ifndef _LIBINAC_HASHTABLE_H_
#define _LIBINAC_HASHTABLE_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <libinac/lib.h>
#include "lib.h"


#define INA_HASHTABLE_DEFAULT_CAPACITY     (0)
#define INA_HASHTABLE_MAX_KEY_LEN          16
#define INA_HASHTABLE_MAX_STAT_TABLES      16
#define INA_HASHTABLE_CF_PREALLOCATED    (4UL)
#define INA_HASHTABLE_CF_STAT           (16UL)
#define INA_HASHTABLE_CF_DEFAULT         (0UL)



typedef enum ina_hashtable_type_e {
    INA_HASHTABLE_TYPE_DEFAULT = -1,
    INA_HASHTABLE_TYPE_CHAINED,
} ina_hashtable_type_t;

typedef enum ina_hashtable_key_type_e {
    INA_HASHTABLE_INT32_KEY,
    INA_HASHTABLE_UINT32_KEY,
    INA_HASHTABLE_INT64_KEY,
    INA_HASHTABLE_UINT64_KEY,
    INA_HASHTABLE_STR_KEY,
    INA_HASHTABLE_PTR_KEY,
} ina_hashtable_key_type_t;

typedef enum ina_hashtable_growth_strategy_e {
    INA_HASHTABLE_GROW_DEFAULT = -1,
    INA_HASHTABLE_GROW_NEVER,
} ina_hashtable_growth_strategy_t;


typedef enum ina_hashtable_shrink_strategy_e {
    INA_HASHTABLE_SHRINK_DEFAULT = -1,
    INA_HASHTABLE_SHRINK_NEVER,
} ina_hashtable_shrink_strategy_t;

typedef enum ina_hashtable_hash_type_e {
     INA_HASHTABLE_HASH_DEFAULT = -1,
     INA_HASHTABLE_HASH32_CRC,
     INA_HASHTABLE_HASH32_LOOKUP3,
     INA_HASHTABLE_HASH32_DJB,
     INA_HASHTABLE_HASH32_JENKINS_OOAT,
     INA_HASHTABLE_HASH32_FNV,
     INA_HASHTABLE_HASH32_SUPERFAST,
     INA_HASHTABLE_HASH32_SDBM,
     INA_HASHTABLE_HASH32_FNV_YOSHIMITSU,
     INA_HASHTABLE_HASH32_MURMUR3,
     INA_HASHTABLE_HASH32_SPOOKY,
     INA_HASHTABLE_HASH32_XXHASH,
     INA_HASHTABLE_HASH32_CRC_HW,
     INA_HASHTABLE_HASH32_MEMMASH,
     INA_HASHTABLE_HASH32_FALKHASH,
     INA_HASHTABLE_HASH32_T1HA0,
     INA_HASHTABLE_HASH32_T1HA1,
#ifdef INA_CPU_X86_64
     INA_HASHTABLE_HASH64_LOCKUP3,
     INA_HASHTABLE_HASH64_FNV,
     INA_HASHTABLE_HASH64_SPOOKY,
     INA_HASHTABLE_HASH64_XXHASH,
     INA_HASHTABLE_HASH64_CRC_HW,
     INA_HASHTABLE_HASH64_MEMMASH,
     INA_HASHTABLE_HASH64_FALKHASH,
     INA_HASHTABLE_HASH64_T1HA0,
     INA_HASHTABLE_HASH64_T1HA1
#endif
} ina_hashtable_hash_type_t;

/* opaque hashtable types */
typedef struct ina_hashtable_ctx_s   ina_hashtable_ctx_t;
typedef struct ina_hashtable_s       ina_hashtable_t;
typedef struct ina_hashtable_iter_s  ina_hashtable_iter_t;

typedef enum ina_hashtable_event_id_e {
    INA_HASHTABLE_EVENT_NEW,
    INA_HASHTABLE_EVENT_SET_BEGIN,
    INA_HASHTABLE_EVENT_SET_END,
    INA_HASHTABLE_EVENT_GET_BEGIN,
    INA_HASHTABLE_EVENT_GET_END,
    INA_HASHTABLE_EVENT_REMOVE_BEGIN,
    INA_HASHTABLE_EVENT_REMOVE_END,
    INA_HASHTABLE_EVENT_EXPANSION,
    INA_HASHTABLE_EVENT_FREE,
} ina_hashtable_event_id_t;

/* stat event */
typedef struct ina_hashtable_event_s {
    uint64_t ts;
    uint32_t event_id;
    int32_t  hashtable_id;
    uint64_t data1;
    uint64_t data2;
} ina_hashtable_event_t;

INA_API(ina_rc_t) ina_hashtable_init(ina_hashtable_key_type_t key_type,
                                     ina_hashtable_hash_type_t hash_type,
                                     ina_hashtable_type_t type,
                                     ina_hashtable_growth_strategy_t growth_strategy,
                                     ina_hashtable_shrink_strategy_t shrink_strategy,
                                     int capacity,
                                     uint32_t  cf,
                                     ina_hashtable_ctx_t **ctx);

INA_API(ina_rc_t) ina_hashtable_destroy(ina_hashtable_ctx_t **ctx);


INA_API(ina_rc_t) ina_hashtable_new(ina_hashtable_ctx_t *ctx,
                                    int capacity,
                                    uint32_t  cf,
                                    ina_hashtable_t **ht);


INA_API(ina_rc_t) ina_hashtable_free(ina_hashtable_t **ht);

INA_API(ina_rc_t) ina_hashtable_clear(ina_hashtable_t *ht);

INA_API(ina_rc_t) ina_hashtable_count(ina_hashtable_t *ht, int *count);

INA_API(ina_rc_t) ina_hashtable_usage(ina_hashtable_t *ht, size_t *usage);

INA_API(ina_rc_t) ina_hashtable_set(ina_hashtable_t *ht, const void *key, size_t key_len,  const void *data);

INA_API(ina_rc_t) ina_hashtable_get(const ina_hashtable_t *ht, const void *key, size_t key_len,  void **data);

INA_API(ina_rc_t) ina_hashtable_remove(ina_hashtable_t *ht,  const void *key, size_t key_len, void **data);

INA_API(ina_rc_t) ina_hashtable_foreach(ina_hashtable_t *ht, ina_foreach_fn_t foreach_fn);

INA_API(ina_rc_t) ina_hashtable_iter_new(ina_hashtable_t *ht, ina_hashtable_iter_t **iter);

INA_API(ina_rc_t) ina_hashtable_iter_free(ina_hashtable_iter_t **iter);

INA_API(ina_rc_t) ina_hashtable_iter_next(ina_hashtable_iter_t *iter, void **data);

INA_API(ina_rc_t) ina_hashtable_iter_reset(ina_hashtable_iter_t *iter);


INA_INLINE ina_rc_t ina_hashtable_set_i32(ina_hashtable_t *ht, int32_t key, const void *data)
{
    return ina_hashtable_set(ht, &key, sizeof(int32_t), data);
}

INA_INLINE ina_rc_t ina_hashtable_get_i32(const ina_hashtable_t *ht, int32_t key, void **data)
{
    return ina_hashtable_get(ht, &key, sizeof(int32_t), data);
}

INA_INLINE ina_rc_t ina_hashtable_remove_i32(ina_hashtable_t *ht, int32_t key, void **data)
{
    return ina_hashtable_remove(ht, &key, sizeof(int32_t), data);
}

INA_INLINE ina_rc_t ina_hashtable_set_u32(ina_hashtable_t *ht, uint32_t key, const void *data)
{
    return ina_hashtable_set(ht, &key, sizeof(uint32_t), data);
}

INA_INLINE ina_rc_t ina_hashtable_get_u32(const ina_hashtable_t *ht, uint32_t key, void **data)
{
    return ina_hashtable_get(ht, &key, sizeof(uint32_t), data);
}

INA_INLINE ina_rc_t ina_hashtable_remove_u32(ina_hashtable_t *ht, uint32_t key, void **data)
{
    return ina_hashtable_remove(ht, &key, sizeof(uint32_t), data);
}

INA_INLINE ina_rc_t ina_hashtable_set_i64(ina_hashtable_t *ht, int64_t key, const void *data)
{
    return ina_hashtable_set(ht, &key,  sizeof(int64_t), data);
}

INA_INLINE ina_rc_t ina_hashtable_get_i64(const ina_hashtable_t *ht, int64_t key, void **data)
{
    return ina_hashtable_get(ht, &key, sizeof(int64_t), data);
}

INA_INLINE ina_rc_t ina_hashtable_remove_i64(ina_hashtable_t *ht, int64_t key, void **data)
{
    return ina_hashtable_remove(ht, &key, sizeof(int64_t), data);
}

INA_INLINE ina_rc_t ina_hashtable_set_u64(ina_hashtable_t *t, uint64_t key, const void *data)
{
    return ina_hashtable_set(t, &key, sizeof(uint64_t), data);
}

INA_INLINE ina_rc_t ina_hashtable_get_u64(const ina_hashtable_t *ht, uint64_t key, void **data)
{
    return ina_hashtable_get(ht, &key, sizeof(uint64_t), data);
}

INA_INLINE ina_rc_t ina_hashtable_remove_u64(ina_hashtable_t *ht, uint64_t key, void **data)
{
    return ina_hashtable_remove(ht, &key, sizeof(uint64_t), data);
}

INA_INLINE ina_rc_t ina_hashtable_set_str(ina_hashtable_t *ht, const char* key, const void *data)
{
    size_t key_len = INA_MIN(INA_HASHTABLE_MAX_KEY_LEN, strlen(key));
    return ina_hashtable_set(ht, key, key_len, data);
}

INA_INLINE ina_rc_t ina_hashtable_get_str(const ina_hashtable_t *ht, const char* key, void **data)
{
    size_t key_len = INA_MIN(INA_HASHTABLE_MAX_KEY_LEN, strlen(key));
    return ina_hashtable_get(ht, key, strlen(key), data);
}

INA_INLINE ina_rc_t ina_hashtable_remove_str(ina_hashtable_t *ht, const char* key, void **data)
{
    size_t key_len = INA_MIN(INA_HASHTABLE_MAX_KEY_LEN, strlen(key));
    return ina_hashtable_remove(ht, key, strlen(key), data);
}

INA_INLINE ina_rc_t ina_hashtable_set_ptr(ina_hashtable_t *ht, const void* key, const void *data)
{
    uintptr_t p = (uintptr_t )key;
    return ina_hashtable_set(ht, &p, sizeof(uintptr_t), data);
}

INA_INLINE ina_rc_t ina_hashtable_get_ptr(const ina_hashtable_t *ht, const void* key, void **data)
{
    uintptr_t p = (uintptr_t )key;
    return ina_hashtable_get(ht, &p, sizeof(uintptr_t), data);
}

INA_INLINE ina_rc_t ina_hashtable_remove_ptr(ina_hashtable_t *ht, const void * key, void **data)
{
    uintptr_t p = (uintptr_t )key;
    return ina_hashtable_remove(ht, &p, sizeof(uintptr_t), data);
}

#ifdef __cplusplus
}
#endif

#endif
