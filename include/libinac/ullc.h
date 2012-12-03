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
 * - Error handling
 * - Proper unit-testing
 * - Proper performance-tests
 * - Options, bis-mask: 
 *   - To decide whether to wait for slow-consumers or wrap around
 *   - Consumer wait strategies
 * - Tuning, cache-lines
 * - Batch writing and reading
 * - Multi producer handling
 */
typedef enum ina_ullc_wait_strategy_e {
    INA_ULLC_BUSY_WAIT = 1,
    INA_ULLC_SIGNAL_WAIT,
    INA_ULLC_TIMER_WAIT,
 } ina_ullc_wait_strategy;

/* ring buffer (shared mem) */
typedef struct ina_ullc_rb_s {
    char magic;
    int version;
    int num_consumers;
    size_t size;
    size_t slots;
    volatile int64_t cursor;
    volatile int64_t next_ptr;
    volatile int64_t swait_count;
    ina_semkey_t semkey; /*FIXME: multiple producer */
} ina_ullc_rb_t;

/* consumer */
typedef struct ina_ullc_consumer_s {
    volatile int alive;
    volatile int64_t cursor;
 } ina_ullc_consumer_t;

/* ullc context */
typedef struct ina_ullc_ctx_s {
    int id;                         /* id of consumer or producer */
    ina_handle_t sem_handle;        /* semaphore handle */
    ina_ullc_wait_strategy ws;      /* wait strategy */
    ina_ullc_rb_t *ring;            /* ring buffer */
    ina_ullc_consumer_t *c_offset;  /* consumer(s) */
    unsigned char *data;            /* slot data */
} ina_ullc_ctx_t;

/* Helper macro to create an ullc ring */
#define INA_ULLC_RING_CREATE(rb, version, type, slots, consumers, name) \
ina_ullc_ring_create(rb, version,sizeof(type),slots,consumers,ina_str_fromcstr(name,NULL),  INA_MEM_SHARED_CREATE)
/* Helper macro to open an ullc ring */
#define INA_ULLC_RING_OPEN(rb, version, type, slots, consumers, name) \
ina_ullc_ring_create(rb, version,sizeof(type),slots,consumers, ina_str_fromcstr(name,NULL), 0)
/* Clain an item */
#define INA_ULLC_CLAIM(type, ctx) (type*)ina_ullc_producer_claim(ctx)
/* Commit an item */
#define INA_ULLC_COMMIT(ctx) ina_ullc_producer_commit(ctx)
/* Get an item  waiting a signal*/
#define INA_ULLC_GET_SWAIT(type, ctx) (type*)ina_ullc_consumer_get_swait(ctx)
/* Get an item  waiting a signal with timeout*/
#define INA_ULLC_GET_TWAIT(type, ctx) (type*)ina_ullc_consumer_get_twait(ctx)
/* Get an item  waiting a signal*/
#define INA_ULLC_GET_BWAIT(type, ctx) (type*)ina_ullc_consumer_get_bwait(ctx)
/* Get an item w/o waiting */
#define INA_ULLC_GET(type, ctx) (type*)ina_ullc_consumer_get(ctx)
/* Signal waiting cnsumers */
#define INA_ULLC_SIGNAL(ctx) ina_ullc_producer_signal(ctx)


/*
 *  Create a ULLC ring
 */
INA_API(ina_rc_t) ina_ullc_ring_create(ina_ullc_rb_t **rb, int version, 
                            size_t size, size_t slots, int num_consumers, 
                            const ina_str_t name, int flags);
/*
 *  Destroy a ULLC ring
 */
INA_API(ina_rc_t) in_ullc_ring_destroy(ina_ullc_rb_t **ring);

/*
 *  Create a producer
 */
INA_API(ina_rc_t) ina_ullc_producer_create(int version,
                            ina_ullc_wait_strategy ws,
                            ina_ullc_rb_t *ring, ina_ullc_ctx_t **ctx);
/*
 *  Destroy a producer
 */
INA_API(ina_rc_t) ina_ullc_producer_destroy(ina_ullc_ctx_t **ctx);

/*
 *
 */
INA_API(int64_t) ina_ullc_producer_pos(ina_ullc_ctx_t *ctx);

/*
 * Claim item for a producer
 */
INA_API(void *)  ina_ullc_producer_claim(ina_ullc_ctx_t *ctx);

/*
 * Commmit item for a producer
 */
INA_API(ina_rc_t) ina_ullc_producer_commit(ina_ullc_ctx_t *ctx);
/*
 * Signal observers
 */
INA_API(ina_rc_t) ina_ullc_producer_signal(ina_ullc_ctx_t *ctx);

/*
 * Create a consumer
 */
INA_API(ina_rc_t) ina_ullc_consumer_create(int id, int version,
                            ina_ullc_wait_strategy ws,
                            ina_ullc_rb_t *ring, ina_ullc_ctx_t **ctx);
/*
 * Destroy consumer
 */
INA_API(ina_rc_t) ina_ullc_consumer_destroy(ina_ullc_ctx_t **ctx);
/*
 * Read from consumer, no wait
 */
INA_API(void *)  ina_ullc_consumer_get(ina_ullc_ctx_t *ctx);
/*
 * Read from consumer, timer wait
 */
INA_API(void *)  ina_ullc_consumer_get_twait(ina_ullc_ctx_t *ctx);
/*
 * Read from consumer, signal wait
 */
INA_API(void *)  ina_ullc_consumer_get_swait(ina_ullc_ctx_t *ctx);
/*
 * Read from consumer, busy wait
 */
INA_API(void *)  ina_ullc_consumer_get_bwait(ina_ullc_ctx_t *ctx);
#endif