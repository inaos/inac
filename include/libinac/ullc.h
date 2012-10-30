/*
 * Copyright (c) 2012, INAOS GmbH
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
#ifndef _LIBINAC_ULLC_H_
#define _LIBINAC_ULLC_H_

#include <libinac/lib.h>

#define INA_ULLC_MIN(x,y) max(x,y)



/*
 * GOALs:
 * 
 * 1. Low-Latency lim(0)
 * 2. High-Throughput
 * 3. Scaleability
 *
 * The goal is to provide an in-memory communication framework 
 * and is therefore limited to processes that run on the same 
 * machine. There should not be any serialization, the communication 
 * should happen via shared-memory directly. (structs)
 *
 * DESIGN:
 *
 * In order to achive the goals we should make sure that the communication 
 * is non-blocking and wait-free. Meaning that if required never sends a 
 * thread back to the operating system for re-scheduling. (How it happens 
 * when you use high-level locking)
 *
 * The LMAX engineering department has open-sourced an interesting 
 * datastructure called the "disruptor". 
 * (http://martinfowler.com/articles/lmax.html)
 * This means our design is inspired by the design of the disruptor, however
 * does not attempt to copy it in any way.
 *
 * To understand the disruptor the following links might be helpful:
 * - http://mechanitis.blogspot.ch/2011/06/dissecting-disruptor-whats-so-special.html
 * - http://mechanitis.blogspot.ch/2011/06/dissecting-disruptor-how-do-i-read-from.html
 * - http://mechanitis.blogspot.ch/2011/07/dissecting-disruptor-writing-to-ring.html
 * - http://mechanitis.blogspot.ch/2011/07/dissecting-disruptor-why-its-so-fast.html
 * - http://mechanitis.blogspot.ch/2011/07/dissecting-disruptor-why-its-so-fast_22.html
 *
 * Low-level explanations can be found at this blog:
 * - http://mechanical-sympathy.blogspot.ch/2011/07/memory-barriersfences.html
 * - http://mechanical-sympathy.blogspot.ch/2011/07/false-sharing.html
 * - http://mechanical-sympathy.blogspot.ch/2011/07/processor-affinity-part-1.html
 *
 * At the heart of our communication framework is a ring-buffer.. not in the
 * traditional sense, because it does not have a pointer to the end. Instead
 * it has two incrementing pointers, one to indicate the current slot (cursor) 
 * by doing a modulo operation and one pointer that points to the next
 * writable slot. 
 * We can have 1 to N consumers that all have their own pointer of where they 
 * currently are reading from the ring. The ring and the consumer pointers are 
 * allocated in  process shared-memory. The pointers are defined as volatile 
 * and incremented atomically by HW ops.
 *
 * Producers: 
 * Currently we have only one producer per ring. The Producer operates in a 
 * transactional, manner in order to be ready for multiple-producers. First 
 * the producer 'claims' as slot in the ring (next-pointer) - currently the 
 * implementation is not safe - in the future the next-pointer has to be 
 * updated with CAS op as part of the claim process. Then the producers fills
 * the data-item and when finished commits, which will update the 
 * ring-cursor. (If the consumers wait in a non-spinning mode, the consumers 
 * need to be notified) In case a Consumer can't keep-up with the producer 
 * the ring would wrap around and therefore make the consumer inconsistent.
 * To prevent this the 'claim'  process will block the producer until the last
 * consumer has caught up.
 *
 * Consumers:
 * The consumers start by opening the shared memory, initialized by the 
 * producer.. if no producer is there the consumer can't start. The API then 
 * provides two options to wait for the producer to put item into the ring or 
 * the wait-free option which just returns NULL if there is nothing new in 
 * the ring. Each Consumer maintains its own 
 * atomic cursor.
 *
 * LIMITATIONS:
 *
 * - Number of consumers need to be defined when creating the consumer
 * - Numer of consumers are static
 *
 * TODO:
 * 
 * - Document with graphics
 * - Fix claim_item function.. to properly wait on slow-consumers
 * - Linux, OS X adjustments for Atomic ops and Shared-Memory
 * - Error handling
 * - Proper unit-testing
 * - Proper performance-tests
 * - Options, bis-mask: 
 *   - To decide whether to wait for slow-consumers or wrap around
 *   - Consumer wait strategies
 * - Tuning, cache-lines
 * - Batch writing and reading
 * - Non spinning wait-strategies
 * - Multi procuder handling
 *
 */
typedef enum ina_ullc_producer_wait_strategy_e {
    INA_ULLC_PRODUCER_BUSY_WAIT = 1,
} ina_ullc_producer_wait_strategy;

typedef enum ina_ullc_consumer_wait_strategy_e {
    INA_ULLC_CONSUMER_BUSY_WAIT = 1,
 } ina_ullc_consumer_wait_strategy;

#define INA_ULLC_ITEM_FIELDS                             \
    int slot;

typedef struct ina_ullc_rb_t {
    int size;
    volatile int64_t cursor;
    volatile int64_t next_ptr;
} ina_ullc_rb_t;

typedef struct ina_ullc_consumer_t {
    volatile int alive;
    volatile int64_t cursor;
 } ina_ullc_consumer_t;

#define INA_ULLC_PCTX_ROOT(name, type)                      \
typedef struct name##_ullc_pctx_t {                         \
    ina_mempool_t *mempool;                                 \
    int num_consumers;                                      \
    ina_ullc_rb_t *ring;                                    \
    ina_ullc_consumer_t *consumers;                         \
    struct type *data;                                      \
} name##_ullc_pctx_t;

#define INA_ULLC_CCTX_ROOT(name,type)                       \
typedef struct name##_ullc_cctx_t {                         \
    ina_mempool_t *mempool;                                 \
    int id;                                                 \
    ina_ullc_rb_t *ring;                                    \
    ina_ullc_consumer_t *consumer;                          \
    struct type *data;                                      \
} name##_ullc_cctx_t;

#define INA_ULLC_PRODUCER_DEFINES(name,type)                \
    INA_ULLC_PCTX_ROOT(name,type)

#define INA_ULLC_CONSUMER_DEFINES(name,type)                \
    INA_ULLC_CCTX_ROOT(name,type)

#define INA_ULLC_PRODUCER_PROTOTYPES(name,type)                             \
ina_rc_t                                                                    \
name##_create_producer(struct name##_ullc_pctx_t **ctx,                     \
    const char *name, int size, int num_consumers);                         \
                                                                            \
ina_rc_t                                                                    \
name##_destroy_producer(struct name##_ullc_pctx_t **ctx);                   \
                                                                            \
type *                                                                      \
name##_producer_claim_item(struct name##_ullc_pctx_t *ctx);                 \
                                                                            \
ina_rc_t                                                                    \
name##_producer_commit_item(struct name##_ullc_pctx_t *ctx, type* item);

#define INA_ULLC_CONSUMER_PROTOTYPES(name,type)                             \
ina_rc_t                                                                    \
name##_create_consumer(struct name##_ullc_cctx_t **cctx,                    \
    const char *name, int size, int num_consumers, int id);                 \
                                                                            \
ina_rc_t                                                                    \
name##_destroy_consumer(struct name##_ullc_cctx_t **cctx);                  \
                                                                            \
type *                                                                      \
name##_consumer_get_item(struct name##_ullc_cctx_t *ctx);                   \
                                                                            \
type *                                                                      \
name##_consumer_get_item_no_wait(struct name##_ullc_cctx_t *ctx);

#define INA_ULLC_PRODUCER_GENERATE(name,type)                               \
ina_rc_t                                                                    \
name##_create_producer(struct name##_ullc_pctx_t **pctx,                    \
    const char *name, int size, int num_consumers)                          \
{                                                                           \
    size_t mem_size;                                                        \
    int i;                                                                  \
    name##_ullc_pctx_t *ctx;                                                \
    *pctx = (name##_ullc_pctx_t*)ina_mem_alloc(sizeof(name##_ullc_pctx_t)); \
    ctx = *pctx;                                                            \
    ctx->mempool = NULL;                                                    \
    ctx->num_consumers = num_consumers;                                     \
    if (size % 2 != 0) {                                                    \
        return INA_MEM_EALLOC;                                              \
    }                                                                       \
    mem_size = sizeof(ina_ullc_rb_t)+(sizeof(type)*size)                    \
        +(sizeof(ina_ullc_consumer_t)*num_consumers);                       \
    if (!INA_SUCCEED(ina_mempool_create(&ctx->mempool,                      \
                        mem_size,                                           \
                        INA_MEM_SHARED|INA_MEM_SHARED_CREATE,               \
                        ina_str_fromcstr(name, NULL))))                     \
    {                                                                       \
        return ina_err_peek();                                              \
    }                                                                       \
    ctx->ring = (ina_ullc_rb_t*)ina_mempool_dalloc(ctx->mempool, mem_size); \
    if (!INA_SUCCEED(ina_err_peek())) {                                     \
        return ina_err_peek();                                              \
    }                                                                       \
    ina_mem_set(ctx->ring, 0, mem_size);                                    \
                                                                            \
    ctx->ring->size = size;                                                 \
    ctx->ring->cursor = -1;                                                 \
    ctx->ring->next_ptr = 0;                                                \
    ctx->data = (type*)ctx->ring + sizeof(ina_ullc_rb_t);                   \
    for (i=0; i < size; i++) {                                              \
        ctx->data[i].slot = i;                                              \
    }                                                                       \
    return INA_SUCCESS;                                                     \
}                                                                           \
ina_rc_t                                                                    \
name##_destroy_producer(struct name##_ullc_pctx_t **pctx)                   \
{                                                                           \
    name##_ullc_pctx_t* ctx;                                                \
    ctx = *pctx;                                                            \
    ina_mempool_release(ctx->mempool, 1);                                   \
    ina_mem_free(ctx);                                                      \
    return INA_SUCCESS;                                                     \
 }                                                                          \
 type *                                                                     \
 name##_producer_claim_item(struct name##_ullc_pctx_t *ctx)                 \
 {                                                                          \
     int i;                                                                 \
     int like_to_write = ctx->ring->next_ptr % ctx->ring->size;             \
     int slow_consumer = -1;                                                \
     int num = ctx->num_consumers;                                          \
     type *item;                                                            \
    while (slow_consumer > like_to_write) {                                 \
        for (i = 0; i < num; i++) {                                         \
            int read_cur;                                                   \
            if (ctx->consumers[i].alive) {                                  \
                read_cur = ctx->consumers[i].cursor % ctx->ring->size;      \
                slow_consumer = INA_ULLC_MIN(slow_consumer, read_cur);      \
            }                                                               \
        }                                                                   \
    }                                                                       \
    item = &(ctx->data[like_to_write]);                                     \
    ina_increment(&ctx->ring->next_ptr);                                    \
    return item;                                                            \
}                                                                           \
ina_rc_t                                                                    \
name##_producer_commit_item(struct name##_ullc_pctx_t *ctx, type* item)     \
{                                                                           \
    ina_increment(&ctx->ring->cursor);                                      \
    return INA_SUCCESS;                                                     \
}

/* Public Producer API */
#define INA_ULLC_PRODUCER_CREATE(name, context, mem_name, size, num_consumers) \
    name##_create_producer(context, mem_name, size, num_consumers)

#define INA_ULLC_PRODUCER_DESTROY(name, context)                            \
    name##_destroy_producer(context)

#define INA_ULLC_PRODUCER_CLAIM_ITEM(name, type, context)                   \
    name##_producer_claim_item(context)

#define INA_ULLC_PRODUCER_COMMIT_ITEM(name, type, context, item)            \
    name##_producer_commit_item(context, item)

#define INA_ULLC_CONSUMER_GENERATE(name, type)                              \
ina_rc_t                                                                    \
name##_create_consumer(struct name##_ullc_cctx_t **cctx,                    \
    const char *name, int size, int num_consumers, int id)                  \
{                                                                           \
    size_t mem_size;                                                        \
    name##_ullc_cctx_t *ctx;                                                \
    ina_ullc_consumer_t *cons;                                              \
    *cctx = (name##_ullc_cctx_t*)ina_mem_alloc(sizeof(name##_ullc_cctx_t)); \
    ctx = *cctx;                                                            \
    ctx->mempool = NULL;                                                    \
    ctx->id = id;                                                           \
    mem_size = sizeof(ina_ullc_rb_t)+(sizeof(type)*size)                    \
        +(sizeof(ina_ullc_consumer_t)*num_consumers);                       \
    if (!INA_SUCCEED(ina_mempool_create(&ctx->mempool,                      \
            mem_size,                                                       \
            INA_MEM_SHARED,                                                 \
            ina_str_fromcstr(name, NULL))))                                 \
    {                                                                       \
        return ina_err_peek();                                              \
    }                                                                       \
    ctx->ring = (ina_ullc_rb_t*)ina_mempool_dalloc(ctx->mempool, mem_size); \
    if (INA_SUCCEED(ina_err_peek())) {                                      \
        return ina_err_peek();                                              \
    }                                                                       \
    ctx->data = (type*)ctx->ring + sizeof(ina_ullc_rb_t);                   \
    cons = (ina_ullc_consumer_t*)(&(ctx->data[size-1]))                     \
       + sizeof(type);                                                      \
    ctx->consumer = &cons[id];                                              \
    ctx->consumer->alive = 1;                                               \
    return INA_SUCCESS;                                                     \
}                                                                           \
ina_rc_t                                                                    \
name##_destroy_consumer(struct name##_ullc_cctx_t **cctx)                   \
{                                                                           \
    name##_ullc_cctx_t* ctx;                                                \
    ctx = *cctx;                                                            \
    ctx->consumer->alive = 0;                                               \
    ina_mempool_release(ctx->mempool, 1);                                   \
    ina_mem_free(ctx);                                                      \
    return 0;                                                               \
}                                                                           \
                                                                            \
type *                                                                      \
name##_consumer_get_item(struct name##_ullc_cctx_t *ctx)                    \
{                                                                           \
    type *item;                                                             \
    int64_t wait_for = ctx->consumer->cursor;                               \
    int idx;                                                                \
    while (ctx->ring->cursor < wait_for) {                                  \
    }                                                                       \
    idx = ctx->consumer->cursor % ctx->ring->size;                          \
    item = &(ctx->data[idx]);                                               \
    ina_increment(&ctx->consumer->cursor);                                  \
    return item;                                                            \
 }                                                                          \
type *                                                                      \
name##_consumer_get_item_no_wait(struct name##_ullc_cctx_t *ctx)            \
{                                                                           \
    type *item;                                                             \
    int64_t wait_for = ctx->consumer->cursor;                               \
    int idx;                                                                \
    if (ctx->ring->cursor < wait_for) {                                     \
        return NULL;                                                        \
    }                                                                       \
    idx = ctx->consumer->cursor % ctx->ring->size;                          \
    item = &(ctx->data[idx]);                                               \
    ina_increment(&ctx->consumer->cursor);                                  \
    return item;                                                            \
}

/* Public Consumer API */
#define INA_ULLC_CONSUMER_CREATE(name, context, mem_name, size, num, id)    \
    name##_create_consumer(context, mem_name, size, num, id)

#define INA_ULLC_CONSUMER_DESTROY(name, context)                            \
    name##_destroy_consumer(context)

#define INA_ULLC_CONSUMER_GET_ITEM(name, context)                           \
    name##_consumer_get_item(context)

#define INA_ULLC_CONSUMER_GET_ITEM_NO_WAIT(name, context)                   \
    name##_consumer_get_item_no_wait(context)

#endif