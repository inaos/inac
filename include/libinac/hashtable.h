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

/*
 * DESIGN:
 * -------
 *
 * Following the creation flags:
 * - INA_HASHTBL_STATIC:          fixed size hashtable, no growing no shrinking 
 * - INA_HASHTBL_GROWABLE:        enable growing
 * - INA_HASHTBL_SHRINKABLE:      enable shrinking
 * - INA_HASHTBL_PROBE_LINEAR:    when growing use linar growth strategy
 * - INA_HASHTBL_PROBE_QUADRATIC: when growing use quadratic probing strategy 
 * - INA_HASHTBL_DOUBLE_HASHING:  use double hashing as growth strategy
 * Note: INA_HASHTBL_STATIC can NOT be combined with GROWABLE or SHRINKABLE
 *
 *
 *
 * ------- 
 * 
 * Libraries to consider with regard to features or design:
 * - tommyds (https://github.com/amadvance/tommyds)
 * - https://probablydance.com/2017/02/26/i-wrote-the-fastest-hashtable/#more-6655
 * - khash (https://github.com/attractivechaos/klib/blob/master/khash.h)
 * - gcc hashtable libiberty (https://gcc.gnu.org/svn/gcc/trunk/libiberty/hashtab.c)
 * - judy ?
 * - google dense hash (https://github.com/sparsehash/sparsehash)
 * - libdynamic (https://github.com/fredrikwidlund/libdynamic)
 * - ulib (https://github.com/stefanocasazza/ULib)
 * - uthash
 *
 * From an API perspective we should only support to have, integer and string keys.
 *
 * Must have feature -  as seen in uthash (keystats):
 * 
 * fcn  ideal%     #items   #buckets  dup%  fl   add_usec  find_usec  del-all usec
 * ---  ------ ---------- ---------- -----  -- ---------- ----------  ------------
 * SFH   91.6%       1219        256    0%  ok         92        131            25
 * FNV   90.3%       1219        512    0%  ok        107         97            31
 * SAX   88.7%       1219        512    0%  ok        111        109            32
 * OAT   87.2%       1219        256    0%  ok         99        138            26
 * JEN   86.7%       1219        256    0%  ok         87        130            27
 * BER   86.2%       1219        256    0%  ok        121        129            27
 *
 * + This feature in uthash is compile-time, it would be great if we could make it a runtime
 *   feature with zero performance impact - e.g. by using ULLC.
 *   Either this means that we have to store the entire hash-map in shared-memory or 
 *   push off all the keys to an ULLC ring? Or we do something similar then uthash with their
 *   hashscan utility where the memory of the process is scanned by a different process, this 
 *   process looks for uthash structures and reads its content to analyze stats? can we do this 
 *   efficiently without impacting the running process?
 *   Another idea would be to use mmap backed memory-pools instead of shared-memory 
 *   this would simplify the operational handling.
 *
 * + Maybe we could also record the collisions or is that the dup%?
 *
 * Thoughs about refactoring:
 * - Currently in some places we do double hashing by first hashing the string with sdbm and then adding it to uthash 
 *   which involves a lookup3 hash
 * - Insert compiler error into uthash
 *
 * TODO:
 * -----
 *
 * 1. Investigate open questions:
 *    - Should we use chaining or open addressing? or both by choice and use-case?
 *      Here a post which contains some input in that regard: http://preshing.com/20110603/hash-table-performance-tests/
 *    - https://en.wikipedia.org/wiki/Hopscotch_hashing?
 *    - What should be a macro and what can be typed c-code?
 *    - We should probably have some fixed size variants and dynamic ones.. if dynamic how to grow:
 *      Quadratic probing, double hashing, linear probing etc. is it a concern at all if we use our mempools wisely?
 *      Or we could support all sorts of different growth strategies via different functors and then analyse with 
 *      which strategy is the best for the given use-case.
 *    - Should we allow shrikning, in terms of memory? maybe as a special case when space is more 
 *      critical then performance
 *    - How to use the inac mempools? one big pool, one pool per bucket
 *    - How to support shrinking? ina_mempool_realloc?
 *    - How to support perfect hashing for lookup-tables and such
 *      -> http://burtleburtle.net/bob/hash/perfect.html
 *      -> https://gist.github.com/alnsn/68f599bc9358fcee122d6175392d779f
 *   	-> https://github.com/alnsn/rgph (looks interesting because its seems to generate the hashtable at runtime)
 *      -> http://www.theiling.de/projects/lookuptable.html
 *      -> https://github.com/rurban/Perfect-Hash
 *      -> https://github.com/inaos/inac/blob/8bd27379b3f9a737de07c0499ac4555680134afc/contribs/luajit/src/host/buildvm_fold.c
 *      -> https://gist.github.com/alnsn/68f599bc9358fcee122d6175392d779f
 *      -> https://github.com/alnsn/rgph
 *      -> http://zola.di.unipi.it/rossano/wp-content/papercite-data/pdf/dcc14.pdf
 *      -> http://cmph.sourceforge.net/bdz.html
 *      -> https://www.snellman.net/blog/archive/2017-03-19-parallel-hashing-with-avx2/
 *    - Do we need to store data in our nodes or do we store it externally? in other words do we need handles in 
 *      hash nodes. What are the pros and cons?
 *    - How to select hash-buckets: https://probablydance.com/2018/06/16/fibonacci-hashing-the-optimization-that-the-world-forgot-or-a-better-alternative-to-integer-modulo/?		
 *
 * 2. How to benchmark
 *    - must be simple because the real benchmark is alwayls the application
 *    - benchmark can be kept outside if it grows too big.. ideally we want to compare our 
 *      implementation against the above contenders.
 *
 *
 */
#define INA_HASHTABLE_MAX_KEY_LEN 16

#define INA_HASHTABLE_CF_GROWABLE         (1UL)
#define INA_HASHTABLE_CF_SHRINKABLE       (2UL)
#define INA_HASHTABLE_CF_PREALLOCATED     (4UL)

typedef ina_rc_t (*ina_hashtable_foreach_fn_t)(const void *data);

typedef enum ina_hashtable_type_e {
    INA_HASHTABLE_TYPE_CHAINED,
    INA_HASHTABL_TYPE_OPENADR
} ina_hashtable_type_t;

typedef enum ina_hashtable_key_type_e {
     INA_HASHTABLE_STR_KEY,
     INA_HASHTABLE_PTR_KEY,
     INA_HASHTABL_UINT32_KEY,
     INA_HASHTABLE_UINT64_KEY,
     INA_HASHTABL_INT32_KEY,
     INA_HASHTABLE_INT64_KEY
} ina_hashtable_key_type_t;

typedef enum ina_hashtable_growth_strategy_e {
    INA_HASHTABLE_GROW_LINEAR,
    INA_HASHTABLE_GROW_QUADRATIC,
    INA_HASHTABLE_GROW_DOUBLE_HASH,
} ina_hashtable_growth_strategy_t;

typedef enum ina_hashtable_hash_type_e {
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


INA_API(ina_rc_t) ina_hashtable_init(ina_hashtable_key_type_t key_type,
                                     ina_hashtable_hash_type_t hash_type,
                                     ina_hashtable_type_t type,
                                     ina_hashtable_growth_strategy_t growth_strategy,
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

INA_API(ina_rc_t) ina_hashtable_foreach(ina_hashtable_t *ht, ina_hashtable_foreach_fn_t foreach_fn);

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
