/*
 * Copyright (c) 2012-2014, INAOS GmbH
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

#ifdef __cplusplus
extern "C" {
#endif

#define INA_ULLC_MIN(x,y) INA_MAX(x,y)


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
 * - Number of consumers/producers need to be defined when creating the 
 * - consumer/producers.
 * - Number of consumers/producers are static
 *
 * TODO:
 * - Document with graphics
 * - Fix claim_item function.. to properly wait on slow-consumers
 * - Error handling
 * - Proper performance-tests
 * - Options, bis-mask: 
 *   - To decide whether to wait for slow-consumers or wrap around
 *   - Consumer wait strategies
 * - Tuning, cache-lines
 * - Batch writing and reading
 */

#define INA_ULLC_MAX_PRODUCERS (64)
#define INA_ULLC_MAX_CONSUMERS (INA_ULLC_MAX_PRODUCERS)
#define INA_ULLC_MAX_FLAGS     (INA_ULLC_MAX_PRODUCERS)

/* Context types */
typedef enum ina_ullc_ctx_type_e {
    INA_ULLC_CTX_PRODUCER = 0,
    INA_ULLC_CTX_CONSUMER,
} ina_ullc_ctx_type_t;
    
/* Signal types for INA_ULLC_SIGNAL_WAIT */
typedef enum ina_ullc_signal_type_e {
     INA_ULLC_SIG_WAIT = -1,
     INA_ULLC_SIG_RELEASE = 1,
} ina_ullc_signal_type;

/* ULLC wait strategies */
typedef enum ina_ullc_wait_strategy_e {
	INA_ULLC_WS_NONE = 0,
    INA_ULLC_WS_BUSY_WAIT,
    INA_ULLC_WS_SIGNAL_WAIT,
    INA_ULLC_WS_TIMER_WAIT,
 } ina_ullc_wait_strategy;

/* ring buffer (shared mem) */
typedef struct ina_ullc_rb_s {
    char magic;
    int16_t version;
    int32_t num_consumers;
    int32_t num_producers;
    size_t size;
    size_t slots;
    volatile int64_t cursor;
    volatile int64_t next_ptr;
    volatile int64_t swait_count;
    volatile int64_t alive_producers;
    ina_semkey_t semkey; /*FIXME: multiple producer */
} ina_ullc_rb_t;

/* ULLC ring cursor */
typedef struct ina_ullc_cursor_s {
    volatile int64_t alive;
    volatile int64_t cursor;
 } ina_ullc_cursor_t;

/* ullc context */
typedef struct ina_ullc_ctx_s {
    int id;                         /* id of consumer or producer */
    ina_mempool_t *pool;            /* memory-pool */
    ina_ullc_ctx_type_t type;       /* type of context */
    ina_handle_t sem_handle;        /* semaphore handle */
    ina_ullc_wait_strategy ws;      /* wait strategy */
    ina_ullc_rb_t *ring;            /* ring buffer */
    ina_ullc_cursor_t *c_offset;    /* consumer(s) */
    ina_ullc_cursor_t *p_offset;    /* prodducers */
    unsigned char *data;            /* slot data */
} ina_ullc_ctx_t;

/* ULLC Ring buffer info */
typedef struct ina_ullc_rb_info_s {
    int    ring_version;            /* Ring version */
    size_t num_write_op;            /* Number of write operation */
    int64_t last_writer;            /* Last writing producer */
    size_t num_read_op;             /* Nr. od read operations */
    int64_t last_reader;            /* Last reading consumer */
    size_t num_producers;           /* Nr of producers */
    size_t num_producers_alive;     /* Nr of active producers */
    size_t num_consumers;           /* Max nr. of consumers */
    size_t num_consumers_alive;     /* Nr of active consumers */
    size_t mem_size;                /* Allocated size in bytes */
    size_t slot_size;               /* Size in bytes for each slot */
    size_t num_slots;               /* Nr of slots */
    int64_t current_slot;           /* Last commited slot */
    ina_ullc_cursor_t c_cursors[INA_ULLC_MAX_PRODUCERS];    /* Consumer cursor states */
    ina_ullc_cursor_t p_cursors[INA_ULLC_MAX_CONSUMERS];    /* Producers cursor states */
} ina_ullc_rb_info_t;


/* ULLC flag set */
typedef struct ina_ullc_flags_s {
    ina_mempool_t *pool;            /* shared memory-pool */
    int64_t *f;                     /* flags  */
} ina_ullc_flags_t;

#define INA_ULLC_PRODUCER_CREATE(type, version, slots, producers, consumers, name, ws, ctx) \
    ina_ullc_producer_create(version, sizeof(type), slots, producers, consumers, name, ws, ctx)
#define INA_ULLC_CONSUMER_CREATE(type, version, slots, producers, consumers, name, ctx) \
    ina_ullc_consumer_create(version, sizeof(type), slots, producers, consumers, name, ctx)
    
/* Clain an item */
#define INA_ULLC_CLAIM(type, ctx) (type*)ina_ullc_producer_claim(ctx)
/* Commit an item */
#define INA_ULLC_COMMIT(ctx) ina_ullc_producer_commit(ctx)
/* Waiting for a signal*/
#define INA_ULLC_SWAIT(ctx) ina_ullc_consumer_swait(ctx)
/* When waiting externally use to indicate start and end of wait */
#define INA_ULLC_SWAIT_BEGIN(ctx) ina_ullc_consumer_swait_begin(ctx)
#define INA_ULLC_SWAIT_END(ctx) ina_ullc_consumer_swait_end(ctx)
/* Waiting for a signal with timeout*/
#define INA_ULLC_TWAIT(ctx) ina_ullc_consumer_twait(ctx)
/* Get an item w/o waiting */
#define INA_ULLC_GET(type, ctx) (type*)ina_ullc_consumer_get(ctx)
/* "Signal" consumers  to wait */
#define INA_ULLC_SIGNAL_WAIT(ctx) ina_ullc_producer_signal(ctx, INA_ULLC_SIG_WAIT)
/* "Singal" consumers to read */
#define INA_ULLC_SIGNAL_RELEASE(ctx) ina_ullc_producer_signal(ctx, INA_ULLC_SIG_RELEASE)

/*
 * Get current ring status informations
 */
INA_API(ina_rc_t) ina_ullc_get_ring_info(const char *name, ina_ullc_rb_info_t *info);

/*
 * Reset ULLC ring
 */
INA_API(ina_rc_t) ina_ullc_reset_ring(const char *name);

/*
 *  Create a producer
 */
INA_API(ina_rc_t) ina_ullc_producer_create(int version, size_t size, 
                    size_t slots, int producers, int num_consumers,
                    const char *name, ina_ullc_wait_strategy ws, 
                    ina_ullc_ctx_t **ctx);
/*
 *  Destroy a producer
 */
INA_API(ina_rc_t) ina_ullc_producer_destroy(ina_ullc_ctx_t **ctx);

/*
 *
 */
INA_API(ina_rc_t) ina_ullc_producer_get_pos(ina_ullc_ctx_t *ctx, int64_t *pos);
/*
 *
 */
INA_API(ina_rc_t) ina_ullc_consumer_get_pos(ina_ullc_ctx_t *ctx, int64_t *pos);

/*
 *
 */
INA_API(ina_rc_t) ina_ullc_consumer_set_pos(ina_ullc_ctx_t *ctx, int64_t pos);
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
INA_API(ina_rc_t) ina_ullc_producer_signal(ina_ullc_ctx_t *ctx, ina_ullc_signal_type st);

/*
 * Create a consumer
 */
INA_API(ina_rc_t) ina_ullc_consumer_create(int version, size_t size, 
                                    size_t slots, 
                                    int producers,
                                    int num_consumers, 
                                    const char *name, 
                                    ina_ullc_ctx_t **ctx);
/*
 * Destroy consumer
 */
INA_API(ina_rc_t) ina_ullc_consumer_destroy(ina_ullc_ctx_t **ctx);
/*
 * Read from consumer, no wait
 */
INA_API(void *)  ina_ullc_consumer_get(ina_ullc_ctx_t *ctx);
/*
 * timer wait
 */
INA_API(ina_rc_t) ina_ullc_consumer_twait(ina_ullc_ctx_t *ctx);
/*
 * signal wait
 */
INA_API(ina_rc_t) ina_ullc_consumer_swait(ina_ullc_ctx_t *ctx);
INA_API(ina_rc_t) ina_ullc_consumer_swait_begin(ina_ullc_ctx_t *ctx);
INA_API(ina_rc_t) ina_ullc_consumer_swait_end(ina_ullc_ctx_t *ctx);


/*
 * Create an ULLC flag set
 */
INA_API(ina_rc_t) ina_ullc_flags_create(const char* name, int num_flags, 
                                        ina_ullc_flags_t **flags);
/*
 * Open an ULLC flag set
 */
INA_API(ina_rc_t) ina_ullc_flags_open(const char* name, int num_flags, 
                                        ina_ullc_flags_t **flags);
/*
 * Destory an ULLC flag set
 */
INA_API(ina_rc_t) ina_ullc_flags_destroy(ina_ullc_flags_t **flags);

/*
 * Get a single ULLC flag
 */
INA_API(ina_rc_t) ina_ullc_flags_get(ina_ullc_flags_t *flags, int id, 
                                        int64_t **v);
/*
 * Set a single ULLC flag
 */
INA_API(ina_rc_t) ina_ullc_flags_set(ina_ullc_flags_t *flags, int id, 
                                        int64_t **new_v);
/*
 * Wait for one or more flags changed
 */
INA_API(ina_rc_t) ina_ullc_flags_wait_for_change(ina_ullc_flags_t **flags, 
                        int64_t mask, int wait_msec);

#ifdef __cplusplus
}
#endif 

#endif
