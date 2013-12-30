/*
 * Copyright (c) 2013, INAOS GmbH
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
#ifndef _LIBINAC_MAP_H_
#define _LIBINAC_MAP_H_

#include <libinac/lib.h>

#include <libinac/maps/uthash.h>
#include <libinac/maps/tree.h>
#include <libinac/maps/khash.h>
#include <libinac/maps/ktree.h>
#include <libinac/maps/hattrie64.c>
#include <libinac/maps/tommyhash.h>
/* FIXME: complete all imports */

/*
 * Design principles
 * -----------------
 * - basic idea is to provide a generic macro interface with as little overhead possible,
 *   for different map implementations. 
 * - all implementations should be for single threaded processsing, we do NOT want any locking 
 *   in the code, since this is one of our core principles in INAC.
 * - we thought about using lock-free (wait-free et al) implementations - however we do not 
 *   need lock-free for our basic data-structures.
 * - to leverage multi-core we have a IPC message passing system where we use lock-free 
 *   data-structures on shared-memory.
 * - another important aspect is to provide a pre-proc option to collect data about the keys 
 *   when running an application - this data should then be statistically analyzed by testing 
 *   all the implementations against it, in order to provide the best fit impl. for each 
 *   application
 */

/*
 * Open items
 * ----------
 *
 * - we need to adjust all the implementations to handle our memory-pool
 * - add more tree's -> AVL, SPLAY, another rb (http://www.canonware.com/rb/), a treap (http://www.canonware.com/trp/)
 * - we need to define a cursor API for trees and an iterator for hashes 
 * - foreach macro would be the best idea
 */

/* 
 * Issues with kbtree
 * ------------------
 *
 * - we can't store a value (addr) in the tree, or how would that work?
 *  
 */

/* 
 * Issues with hattrie64.c  
 * 
 * - Impl does not feel complete
 * - is only 64bit supported?
 * - how to include it, create header and compile include .c file directly
 * - no remove functionality
 *
 */

/*
 * Discussion points:
 * ------------------
 *
 * - should we have seperate interfaces for tree's?
 * - should we hash string keys to 32 or 64 bit, when we have to option, or configurable?
 * - how to do tuning and configuration of all these implementations .. descriptor struct?
 * - how to do the analysis of the best hash/tree implementation, I like how uthash does it 
 *   we can take this as a starting-point... and improve from there 
 * - how intrusive can our changes be in order to arrive at a simple API, we need to change 
 *   all the implementations anyway since we want to use our memory-pool
 */

#ifdef __cplusplus
extern "C" {
#endif

#define INA_MAP_IMPL_UTHASH      ("UTHASH")
#define INA_MAP_IMPL_RBTREE      ("RBTREE")
#define INA_MAP_IMPL_KHASH       ("KHASH")
#define INA_MAP_IMPL_KBTREE      ("KBTREE")
#define INA_MAP_IMPL_HATTRIE     ("HATTRIE")
#define INA_MAP_IMPL_THASHLIN    ("THASHLIN")
#define INA_MAP_IMPL_THASHTAB    ("THASHTAB")
#define INA_MAP_IMPL_TIPTRIE     ("TIPTRIE")
#define INA_MAP_IMPL_TTRIE       ("TTRIE")
#ifdef INA_MAP_HAS_JUDY
    #define INA_MAP_IMPL_JUDY    ("JUDY")
#endif

typedef const ina_str_t ina_map_str_t;

#define INA_MAP_KEY_TYPE_INT8    ("int8_t")
#define INA_MAP_KEY_TYPE_INT16   ("int16_t")
#define INA_MAP_KEY_TYPE_INT32   ("int32_t")
#define INA_MAP_KEY_TYPE_INT64   ("int64_t")
#define INA_MAP_KEY_TYPE_UINT8   ("uint8_t")
#define INA_MAP_KEY_TYPE_UINT16  ("uint16_t")
#define INA_MAP_KEY_TYPE_UINT32  ("uint32_t")
#define INA_MAP_KEY_TYPE_UINT64  ("uint64_t")
#define INA_MAP_KEY_TYPE_STRING  ("ina_map_str_t")
#define INA_MAP_KEY_TYPE_WORD    ("ina_word_t")

typedef struct ina_map_ctx_s ina_map_ctx_t;

typedef int (*ina_map_compare_fn)(const void* arg, const void* obj);

/* BEGIN prototypes  */
#define INA_MAP_PROTOTYPE(impl,id,kt,vt,cmp) __INA_MAP_PROTOTYPE_##impl(id,kt,vt,cmp)

#define __INA_MAP_PROTOTYPE_UTHASH(id,kt,vt,cmp) \
    struct ina_map_ctx_s { \
        ina_str_t implementation; \
        ina_str_t kt; \
        size_t kt_size; \
        ina_mempool_t *pool; \
        vt *head; \
    }; 

#define __INA_MAP_PROTOTYPE_RBTREE(id,kt,vt,cmp) \
    struct ina_map_ctx_s { \
        ina_str_t implementation; \
        ina_str_t kt; \
        size_t kt_size; \
        ina_mempool_t *pool; \
        RB_HEAD(id,vt) head; \
    }; \
    RB_PROTOTYPE(id,vt,"rbf")

#define __INA_MAP_PROTOTYPE_KHASH(id,kt,vt,cmp) \
    KHASH_INIT(id, kt, vt, 1, kh_##kt_hash_func, kh_##kt_hash_equal) \
    struct ina_map_ctx_s { \
        ina_str_t implementation; \
        ina_str_t kt; \
        size_t kt_size; \
        ina_mempool_t *pool; \
        khash_t(id) *head; \
    };

#define __INA_MAP_PROTOTYPE_KBTREE(id,kt,vt,cmp) \
    KBTREE_INIT(id,kt,cmp) \
    struct ina_map_ctx_s { \
        ina_str_t implementation; \
        ina_str_t kt; \
        size_t kt_size; \
        ina_mempool_t *pool; \
        kbtree_t(id) *head; \
    };

#define __INA_MAP_PROTOTYPE_HATTRIE(id,kt,vt,cmp) \
    struct ina_map_ctx_s { \
        ina_str_t implementation; \
        ina_str_t kt; \
        size_t kt_size; \
        ina_mempool_t *pool; \
        void *head; \
    };

#define __INA_MAP_PROTOTYPE_THASHLIN(id,kt,vt,cmp) \
    struct ina_map_ctx_s { \
        ina_str_t implementation; \
        ina_str_t kt; \
        size_t kt_size; \
        ina_mempool_t *pool; \
        tommy_hashslin *head; \
        ina_map_compare_fn cmp_fn;
    };

/* END prototypes */

/* BEGIN generators */

#define INA_MAP_GENERATE(impl,id,vt,cmp) __INA_MAP_GENERATE_##impl(id,vt,cmp)

#define __INA_MAP_GENERATE_UTHASH(id,vt,cmp) #

#define __INA_MAP_GENERATE_RBTREE(id,vt,cmp) RB_GENERATE(id,vt,"rbf",cmp) 

#define __INA_MAP_GENERATE_KHASH(id,vt,cmp) #

#define __INA_MAP_GENERATE_KBTREE(id,vt,cmp) #

#define __INA_MAP_GENERATE_HATTRIE(id,vt,cmp) #

#define __INA_MAP_GENERATE_THASHLIN(id,vt,cmp) #

/* END generators */

/* BEGIN markers */

#define INA_MAP_MARKER(impl,type) __INA_MAP_MARKER_##impl

#define __INA_MAP_MARKER_UTHASH UT_hash_handle hh;

#define __INA_MAP_MARKER_RBTREE RB_ENTRY(type) rbf;

#define __INA_MAP_MARKER_KHASH  #

#define __INA_MAP_MARKER_KBTREE #

#define __INA_MAP_MARKER_HATTRIE #

#define __INA_MAP_MARKER_THASHLIN tommy_node node;

/* END markers */

/* BEGIN init */

#define INA_MAP_INIT(impl,id,kt,vt,ctx,pool,cmp) __INA_MAP_INIT_##impl(id,kt,vt,ctx,pool,cmp) 

#define __INA_MAP_INIT_UTHASH(id,kt,vt,ctx,pool,cmp) \
do { \
    \
    if (strcmp(kt, INA_MAP_KEY_TYPE_INT8) == 0) { \
        (*ctx)->kt_size = sizeof(int8_t); \
    } \
    else if (strcmp(kt, INA_MAP_KEY_TYPE_INT16) == 0) { \
        (*ctx)->kt_size = sizeof(int16_t); \
    } \
    else if (strcmp(kt,INA_MAP_KEY_TYPE_STRING) == 0) { \
        (*ctx)->kt_size = sizeof(ina_map_str_t); \
    } \
    else { \
        /* FIXME: error handling */ \
    } \
    (*ctx)->head = NULL;\
} while (0)

#define __INA_MAP_INIT_RBTREE(id,kv,vt,ctx,pool,cmp) # /* FIXME */

#define __INA_MAP_INIT_KHASH(id,kv,vt,ctx,pool,cmp) \
do { \
    /* FIXME: do kt type checking only uint32, uint64 and str allowed  */
   (*ctx)->head = kh_init(id); \
} while (0)

#define __INA_MAP_INIT_KBTREE(id,kv,vt,ctx,pool,cmp) \
do { \
   (*ctx)->head = kb_init(id, KB_DEFAULT_SIZE); \
} while (0)

#define __INA_MAP_INIT_HATTRIE(id,kv,vt,ctx,pool,cmp) \
do { \
    /* FIXME: How to define the root level 3?*/ \
    /* also do kt type checking only strings allowed */ \
    (*ctx)->head = hat_open(3, sizeof(size_t)); \
} while (0)

#define __INA_MAP_INIT_THASHLIN(id,kv,vt,ctx,pool,cmp) \
do { \
    ctx->thashfun = /* depends on kt .. uint32, uint64 or string 32/64 tommyhash.h */ \
    ctx->head = (tommy_hashslin*)ina_mem_alloc(sizeof(tommy_hashslin)); \
    tommy_hashlin_init(ctx->head); \
    ctx->cmp_fn = cmp; \
} while (0)

/* END init */

/* BEGIN destroy */

#define INA_MAP_DESTROY(impl,id,ctx) __INA_MAP_DESTROY_##impl(id,ctx)

#define __INA_MAP_DESTROY_UTHASH(id,ctx) \
do { \
  \
} while (0)

/* END destroy */

/* BEGIN clear */

#define INA_MAP_CLEAR(impl,id,vt,ctx) __INA_MAP_CLEAR_##impl(id,vt,ctx)

#define __INA_MAP_CLEAR_UTHASH(id,vt,ctx) HASH_CLEAR(hh, ctx->head)

#define __INA_MAP_CLEAR_RBTREE(id,vt,ctx) \
do { \
    vt *var; \
    vt *nxt; \ 
    for (var = RB_MIN(id, ctx->head); var != NULL; var = nxt) { \
        nxt = RB_NEXT(id, ctx->head, var); \
        RB_REMOVE(id, ctx->head, var); \
        free(var); \
    } \
} while(0)

#define __INA_MAP_CLEAR_KHASH(id,vt,ctx) kh_clear(id,ctx->head)

#define __INA_MAP_CLEAR_KBTREE(id,vt,ctx) \
do { \
\
} while (0)

#define __INA_MAP_CLEAR_HATTRIE(id,vt,ctx) \
do { \
/* FIXME: check whether the approach of close and open is sensible  */ \
    hat_close(ctx->head); \
    ctx->head = hat_open(3, sizeof(size_t)); \
} while(0)

#define __INA_MAP_CLEAR_THASHLIN(id,vt,ctx) \
do { \
    /* check how to do a clear */ \
} while(0)

/* END clear */

/* BEGIN get_k */

#define INA_MAP_GET_K(impl,id,ctx,key,out) __INA_MAP_GET_K_##impl(id,ctx,key,out)

#define __INA_MAP_GET_K_UTHASH(id,ctx,key,out) \
    HASH_FIND(hh,ctx->head,&key,ctx->kt_size,out)

#define __INA_MAP_GET_K_RBTREE(id,ctx,key,out) \
    error("NYI: __INA_MAP_GET_RBTREE")

#define __INA_MAP_GET_K_KHASH(id,ctx,key,out) \
do { \
    khiter_t k; \
    k = kh_get(id, ctx->head, key);\
    out = kh_value(ctx->head, k);\
} while (0)

#define __INA_MAP_GET_K_HATTRIE(id,ctx,key,out) \
do { \
    out = hat_find(ctx->head, key, ina_str_len(key) - 1); \
} while (0)

#define __INA_MAP_GET_K_THASHLIN(id,ctx,key,out) \
do { \
    out = tommy_hashlin_search(ctx->head, ctx->cmp_fn, key, ctx->thashfun(key)); \
} while(0)

/* END get_k */

/* BEGIN get_v */

#define INA_MAP_GET_V(impl,id,ctx,vs,out) __INA_MAP_GET_V_##impl(id,ctx,vs,out)

#define __INA_MAP_GET_V_UTHASH(id,ctx,vs,out) \
    error("NYI: __INA_MAP_GET_V_UTHASH)

#define __INA_MAP_GET_V_RBTREE(id,ctx,key,out) \
do { \
    out = RB_FIND(id,ctx->head,vs); \
} while (0)

#define __INA_MAP_GET_V_KHASH(id,ctx,vs,out) \
    error("NYI: __INA_MAP_GET_V_KHASH")

#define __INA_MAP_GET_V_HATTRIE(id,ctx,vs,out) \
    error("NYI: __INA_MAP_GET_V_HATTRIE")

#define __INA_MAP_GET_V_THASHLIN(id,ctx,vs,out) \
    error("NYI: __INA_MAP_GET_V_THASHLIN")

/* END get_v */

/* BEGIN put */

#define INA_MAP_PUT(impl,id,ctx,key,value) __INA_MAP_PUT_##impl(id,ctx,key,value)

#define __INA_MAP_PUT_UTHASH(id,ctx,key,value) \
do { \
    if (ctx->kt == INA_MAP_KEY_TYPE_STRING) { \
        HASH_ADD_KEYPTR(hh,ctx->head,&key,ina_str_len(key)*ctx->kt_size,value); \
    } \
    else { \
        HASH_ADD_KEYPTR(hh,ctx->head,&key,ctx->kt_size,value); \
    } \
} while (0)

#define __INA_MAP_PUT_RBTREE(id,ctx,key,value) \
    RB_INSERT(id,ctx->head,value)

#define __INA_MAP_PUT_KHASH(id,ctx,key,value) \
do { \
    khiter_t k; \
    int ret; \
    k = kh_put(id, ctx->head, key, &ret);\
    kvalue(ctx-head, k) = value; \
} while (0)

#define __INA_MAP_PUT_HATTRIE(id,ctx,key,value) \
do { \
    void *addr = hat_cell(ctx->head, key, ina_str_len(key) -1); \
    addr = value; \
} while(0)

#define __INA_MAP_PUT_THASHLIN(id,ctx,key,value) \
do { \
    tommy_hashlin_insert(ctx->head, &value->node, value, ctx->thashfun(key)); \
} while (0)

/* END put */

/* BEGIN remove_k */

#define INA_MAP_REMOVE_K(impl,id,ctx,key) __INA_MAP_REMOVE_K_##impl(id,ctx,key)

#define __INA_MAP_REMOVE_K_UTHASH(id,ctx,key) \
    error("NYI: __INA_MAP_REMOVE_K_UTHASH")

#define __INA_MAP_REMOVE_K_RBTREE(id,ctx,key) \
    error("NYI: __INA_MAP_REMOVE_K_RBTREE")

#define __INA_MAP_REMOVE_K_KHASH(id,ctx,key) \
do { \
    khiter_t k; \
    k = kh_get(id, ctx->head, key); \
    kh_del(id, ctx->head, key); \
} while (0)

#define __INA_MAP_REMOVE_K_HATTRIE(id,ctx,key) \
    error("NYI: __INA_MAP_REMOVE_K_HATTRIE")

#define __INA_MAP_REMOVE_K_THASHLIN(id,ctx,key) \
do { \
    tommy_hashlin_remove(ctx->head, ctx->cmp_fn, key, ctx->thashfun(key)); \
} while (0)

/* END remove_k */

/* BEGIN remove_v */

#define INA_MAP_REMOVE_V(impl,id,ctx,value) __INA_MAP_REMOVE_V_##impl(id,ctx,value)

#define __INA_MAP_REMOVE_V_UTHASH(id,ctx,value) \
    HASH_DELETE(hh,ctx->head,value)

#define __INA_MAP_REMOVE_V_RBTREE(id,ctx,value) \
    RB_REMOVE(id,ctx->head,value)

#define __INA_MAP_REMOVE_V_KHASH(id,ctx,value) \
    error("NYI: __INA_MAP_REMOVE_V_KHASH")

#define __INA_MAP_REMOVE_V_HATTRIE(id,ctx,value) \
    error("NYI: __INA_MAP_REMOVE_V_HATTRIE")

#define __INA_MAP_REMOVE_V_THASHLIN(id,ctx,value) \
    error("NYI: __INA_MAP_REMOVE_V_THASHLIN")

/* END remove_v */

#ifdef __cplusplus
}
#endif

#endif
