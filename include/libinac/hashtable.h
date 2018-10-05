/*
 * Copyright INAOS GmbH, Thalwil, 2016-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#ifndef _LIBINAC_HASHTABLE_H_
#define _LIBINAC_HASHTABLE_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <libinac/lib.h>


#define INA_HASHTABLE_DEFAULT_CAPACITY    (32)
#define INA_HASHTABLE_MAX_KEY_LEN       (16UL)
#define INA_HASHTABLE_CF_PREALLOCATED    (4UL)
#define INA_HASHTABLE_CF_STAT           (16UL)
#define INA_HASHTABLE_CF_DEFAULT         (0UL)



typedef enum ina_hashtable_type_e {
    INA_HASHTABLE_TYPE_DEFAULT = -1,
    INA_HASHTABLE_TYPE_CHAINED,
} ina_hashtable_type_t;

typedef enum ina_hashtable_key_type_e {
    INA_HASHTABLE_UNDEFINED_KEY = -1,
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


/* opaque hash table types */
typedef struct ina_hashtable_s                ina_hashtable_t;
typedef struct ina_hashtable_iter_s           ina_hashtable_iter_t;

/*
 * Initialize hash table module. This function is called by
 * ina_init().
 *
 * Parameters
 *  cfg_filepath  Path to the configuration file containing the hash table configuration
 *
 * Return
 *  INA_SUCCESS if all went well
 *
 *  Possible error codes
 *    INA_ERR_INVALID_ARGUMENT  if cfg_filepath is NULL or empty
 *    INA_ERR_OUT_OF_MEMORY     out of memory
 */
INA_API(ina_rc_t) ina_hashtable_init(const char *cfg_filepath);

/*
 * Destroy and free resources. This function is called by ina_exit
 */
INA_API(void) ina_hashtable_destroy(void);


INA_API(ina_rc_t) ina_hashtable_new(ina_hashtable_key_type_t key_type,
                                    ina_hash_type_t hash_type,
                                    ina_hashtable_type_t type,
                                    ina_hashtable_growth_strategy_t growth_strategy,
                                    ina_hashtable_shrink_strategy_t shrink_strategy,
                                    size_t capacity,
                                    uint32_t  cf,
                                    ina_hashtable_t **ht);

INA_API(ina_rc_t) ina_hashtable_new_from_cfg(ina_hashtable_key_type_t key_type, const char *name, ina_hashtable_t **ht);

INA_API(void) ina_hashtable_free(ina_hashtable_t **ht);

INA_API(ina_rc_t) ina_hashtable_clear(ina_hashtable_t *ht);

INA_API(ina_rc_t) ina_hashtable_count(ina_hashtable_t *ht, size_t *count);

INA_API(ina_rc_t) ina_hashtable_usage(ina_hashtable_t *ht, size_t *usage);

INA_API(ina_rc_t) ina_hashtable_set(ina_hashtable_t *ht, const void *key, size_t key_len,  const void *data);

INA_API(ina_rc_t) ina_hashtable_get(const ina_hashtable_t *ht, const void *key, size_t key_len,  void **data);

INA_API(ina_rc_t) ina_hashtable_remove(ina_hashtable_t *ht,  const void *key, size_t key_len, void **data);

INA_API(ina_rc_t) ina_hashtable_foreach(ina_hashtable_t *ht, ina_foreach_fn_t foreach_fn);

INA_API(ina_rc_t) ina_hashtable_foreach_arg(ina_hashtable_t *ht, ina_foreach_arg_fn_t foreach_fn, void* arg);

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
    return ina_hashtable_get(ht, key, key_len, data);
}

INA_INLINE ina_rc_t ina_hashtable_remove_str(ina_hashtable_t *ht, const char* key, void **data)
{
    size_t key_len = INA_MIN(INA_HASHTABLE_MAX_KEY_LEN, strlen(key));
    return ina_hashtable_remove(ht, key, key_len, data);
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

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                             *
 * PRIVATE API                                                                 *
 *                                                                             *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
#define INA_HASHTABLE_MAX_STAT_TABLES  (16)

typedef struct ina_hashtable_event_consumer_s ina_hashtable_event_consumer_t;

typedef enum ina_hashtable_event_id_e {
    INA_HASHTABLE_EVENT_IDLE,
    INA_HASHTABLE_EVENT_META,
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


INA_API(ina_rc_t) ina_hashtable_event_consumer_new(ina_hashtable_event_consumer_t **event_consumer, uint32_t flag);

INA_API(void) ina_hashtable_event_consumer_free(ina_hashtable_event_consumer_t **event_consumer);

INA_API(ina_rc_t) ina_hashtable_event_consumer_next(ina_hashtable_event_consumer_t *event_consumer, ina_hashtable_event_t **event);


#ifdef __cplusplus
}
#endif

#endif
