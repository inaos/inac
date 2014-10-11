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
#include <libinac/lib.h>
#include "config.h"

#define __INA_MAGIC_HDR 'Z'

#ifdef INA_OS_WIN32
#define __INA_SEMKEY "ULLC_SEM_"
#else
#define __INA_SEMKEY 0x300
#endif

/* make unique sem key */
static ina_rc_t __ina_sem_makekey(ina_ullc_rb_t*, const char*);
/* create semaphore */
static ina_rc_t __ina_sem_create(ina_ullc_ctx_t*);
/* open semaphore */
static ina_rc_t __ina_sem_open(ina_ullc_ctx_t*);
/* close semphore */
static ina_rc_t __ina_sem_close(ina_ullc_ctx_t*);
/* release semphore */
static ina_rc_t __ina_sem_operation(ina_ullc_ctx_t*, ina_ullc_signal_type st);
/* create/open ring buffer */
static ina_rc_t __ina_ullc_ring_create(ina_ullc_rb_t**, ina_ullc_ctx_t*, int,
                             size_t, size_t, int, int, const char*, int);


INA_API(ina_rc_t) ina_ullc_get_ring_info(const char *name, ina_ullc_rb_info_t *info)
{
    ina_mempool_t *m = NULL;
    ina_ullc_rb_t *rb = NULL;
    /* size_t c = 0; */

    INA_ASSERT_NOTNULL(info);

    if (!INA_SUCCEED(ina_mempool_create(&m, sizeof(ina_ullc_rb_t), INA_MEM_SHARED, name))) {
        return INA_ERR_PUSH_LAST;
    }
    rb = (ina_ullc_rb_t*)ina_mempool_dalloc(m, sizeof(ina_ullc_rb_t));
    if (rb == NULL) {
        ina_mempool_release(m, INA_YES);
        return INA_ERR_PUSH_LAST;
    }

    info->ring_version = rb->version;
    info->num_write_op = (size_t)rb->next_ptr;
    info->last_writer = 0;
    info->num_read_op = (size_t)rb->cursor;
    info->last_reader = 0;
    info->num_producers = rb->num_producers;
    info->num_producers_alive = 0;
    info->num_consumers = rb->num_consumers;
    info->num_consumers_alive = 0;
    info->mem_size = m->size;
    info->slot_size = rb->size;
    info->num_slots = rb->slots;
    info->current_slot = rb->cursor;
    /*for (c = 0; c < info->num_producers; ++c) {
        if (p->alive) {
            info->num_producers_alive++;
        }
    }
    for (c = 0; c < info->num_consumers; ++c) {
        if (p->alive) {
            info->num_consumers_alive++;
        }
    }*/

    ina_mempool_release(m, INA_YES);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ullc_producer_create(int version, size_t size, 
                            size_t slots, int  num_producers, int num_consumers,
                            const char* name, ina_ullc_wait_strategy ws,
                            ina_ullc_ctx_t **ctx)
{
    ina_ullc_ctx_t *pctx;
    
    if (version <= 0) {
        return INA_ULLC_EINVERSION;
    }
    if (size == 0) {
        return INA_ULLC_EINSIZE;
    }
    if (slots == 0) {
        return INA_ULLC_EINSLOTS;
    }
    if (num_consumers <= 0) {
        return INA_ULLC_EINCONSUMERS;
    }

    *ctx = (ina_ullc_ctx_t*)ina_mem_alloc(sizeof(ina_ullc_ctx_t));
    if (*ctx == NULL) {
        return INA_ERR_PUSH_LAST;
    }
    pctx = *ctx;

    if (!INA_SUCCEED(__ina_ullc_ring_create(&pctx->ring, pctx, version, size, 
                                            slots, 
                                            num_producers, 
                                            num_consumers, 
                                            name, 
                                            INA_MEM_SHARED_CREATE|INA_MEM_SHARED_EXCL))) {
        ina_err_clear(ina_err_peek());
        if (!INA_SUCCEED(__ina_ullc_ring_create(&pctx->ring, pctx, version, size, 
                                            slots, 
                                            num_producers, 
                                            num_consumers, 
                                            name, 
                                            INA_MEM_SHARED_CREATE))) {
            return INA_ERR_PUSH_LAST;
        }
    }

    INA_ASSERT_NOTNULL(pctx->ring);

    if (pctx->ring->version != version) {
        return INA_ULLC_EVERSION;
    }
    
    pctx->id = 0;
    pctx->type = INA_ULLC_CTX_PRODUCER;
    pctx->ws = ws;
    pctx->ring = pctx->ring;
    pctx->data = ((unsigned char*)pctx->ring) + sizeof(ina_ullc_rb_t);
    pctx->c_offset = (ina_ullc_cursor_t*)&pctx->data[(pctx->ring->slots)*pctx->ring->size];
    pctx->p_offset = &pctx->c_offset[num_consumers];
    while (pctx->id < num_producers) {
        pctx->p_offset = &pctx->p_offset[pctx->id];
        if (INA_ATOMIC_SWAP(&pctx->p_offset->alive, 0, 1) == 0) {
            break;
        }
        ++pctx->id;
    }
    if (pctx->id == num_producers) {
        return INA_ULLC_EPLIMIT;
    }
    return __ina_sem_create(pctx);
}

INA_API(ina_rc_t) ina_ullc_reset_ring(const char *name)
{
    ina_mempool_t *m = NULL;
    ina_ullc_rb_t *rb = NULL;

    INA_ASSERT_NOTNULL(name);
    INA_ASSERT_TRUE(strlen(name));

    if (!INA_SUCCEED(ina_mempool_create(&m, sizeof(ina_ullc_rb_t), INA_MEM_SHARED, name))) {
        return INA_ERR_PUSH_LAST;
    }
    rb = (ina_ullc_rb_t*)ina_mempool_dalloc(m, sizeof(ina_ullc_rb_t));
    if (rb == NULL) {
        ina_mempool_release(m, INA_YES);
        return INA_ERR_PUSH_LAST;
    }
    rb->magic = 0;
    ina_mempool_release(m, INA_YES);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ullc_overrun_enable(ina_ullc_ctx_t *ctx) 
{
    INA_ASSERT_NOTNULL(ctx);
    if (INA_ATOMIC_SWAP(&ctx->ring->overrun_enabled, INT64_MAX, -1) == INT64_MAX) {
        return INA_SUCCESS;
    }
    return INA_FAILURE;
}

INA_API(ina_rc_t) ina_ullc_overrun_disable(ina_ullc_ctx_t *ctx)
{
    INA_ASSERT_NOTNULL(ctx);
    if (INA_ATOMIC_SWAP(&ctx->ring->overrun_enabled, -1, INT64_MAX) == -1) {
        return INA_SUCCESS;
    }
    return INA_FAILURE;
}

INA_API(ina_rc_t) ina_ullc_producer_reset(ina_ullc_ctx_t *ctx)
{
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ullc_producer_destroy(ina_ullc_ctx_t **ctx)
{
    if (*ctx == NULL) {
        return INA_SUCCESS;
    }
    INA_ASSERT_EQUAL(INA_ULLC_CTX_PRODUCER, (*ctx)->type);
    
    INA_ATOMIC_SWAP(&(*ctx)->p_offset->alive,1,0);
    INA_ASSERT_EQUAL(0, (*ctx)->p_offset->alive);

    /* Force re-initialization on last producers */
    if (INA_ATOMIC_DEC(&(*ctx)->ring->alive_producers) == 1) {
        (*ctx)->ring->magic = 0;
    }

    if (!INA_SUCCEED(__ina_sem_close(*ctx))) {
        return INA_ERR_PUSH_LAST;
    }

    if (!INA_SUCCEED(ina_mempool_release((*ctx)->pool, 1))) {
        return INA_ERR_PUSH_LAST;
    }

    *ctx = NULL;
    return INA_SUCCESS;
}

INA_API(void *)ina_ullc_producer_claim(ina_ullc_ctx_t *ctx)
{
    int64_t i;
    int64_t like_to_write;
    int64_t slow_consumer;
    int64_t num;
    void *item;

    INA_ASSERT_NOTNULL(ctx);
    INA_ASSERT_EQUAL(INA_ULLC_CTX_PRODUCER, ctx->type);

    slow_consumer = ctx->ring->overrun_enabled;
    like_to_write = ctx->ring->next_ptr % ctx->ring->slots;
    num = ctx->ring->num_consumers;

    while (slow_consumer > like_to_write) {
        for (i = 0; i < num; ++i) {
            int read_cur;
            slow_consumer = -1;
            if (ctx->c_offset[i].alive) {
                read_cur = ctx->c_offset[i].cursor % ctx->ring->slots;
                INA_TRACE3("wait consumer(%ld) %ld at position %d for %ld", i, slow_consumer, read_cur, like_to_write);
                slow_consumer = INA_MAX(slow_consumer, read_cur);
            }
        }
    }
    item = &ctx->data[like_to_write*ctx->ring->size];
    
    return item;
}

INA_API(ina_rc_t) ina_ullc_producer_commit(ina_ullc_ctx_t *ctx)
{
    INA_ASSERT_NOTNULL(ctx);
    INA_ASSERT_EQUAL(INA_ULLC_CTX_PRODUCER, ctx->type);
    INA_ATOMIC_INC(&ctx->ring->next_ptr);
    INA_ATOMIC_INC(&ctx->ring->cursor);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ullc_producer_get_pos(ina_ullc_ctx_t *ctx, int64_t *pos)
{
    INA_ASSERT_NOTNULL(ctx);
    INA_ASSERT_NOTNULL(pos);
    INA_ASSERT_EQUAL(INA_ULLC_CTX_PRODUCER, ctx->type);

    *pos = ctx->ring->cursor;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ullc_producer_signal(ina_ullc_ctx_t *ctx, ina_ullc_signal_type st)
{
    INA_ASSERT_NOTNULL(ctx);
    /*INA_ASSERT_EQUAL(INA_ULLC_CTX_PRODUCER, ctx->type);*/

    /* FIMXE: maybe declare API as inline */
    return __ina_sem_operation(ctx, st);
}

INA_API(ina_rc_t) ina_ullc_consumer_create(int version, size_t size, 
                        size_t slots, int num_producers, int num_consumers, 
                        const char *name, ina_ullc_ctx_t **ctx)
{
    ina_ullc_cursor_t *cons;
    ina_ullc_ctx_t* ccxt;

    *ctx = (ina_ullc_ctx_t*)ina_mem_alloc(sizeof(ina_ullc_ctx_t));
    if (*ctx == NULL) {
        return INA_ERR_PUSH_LAST;
    }
    ccxt = *ctx;

    if (!INA_SUCCEED(__ina_ullc_ring_create(&ccxt->ring, ccxt, version, size, 
            slots, num_producers, num_consumers, name, 0))) {
        return INA_ERR_PUSH_LAST;
    }

    if (ccxt->ring->version != version) {
        return INA_ULLC_EVERSION;
    }

    ccxt->id = 0;
    ccxt->type = INA_ULLC_CTX_CONSUMER;
    ccxt->ws = INA_ULLC_WS_NONE;
    ccxt->sem_handle = 0;
    ccxt->ring = ccxt->ring;
    ccxt->data = ((unsigned char*)ccxt->ring) + sizeof(ina_ullc_rb_t);
    cons = (ina_ullc_cursor_t*)&ccxt->data[(ccxt->ring->slots)*ccxt->ring->size];
    while (ccxt->id < num_consumers) {
        ccxt->c_offset = &cons[ccxt->id];
        if (INA_ATOMIC_SWAP(&ccxt->c_offset->alive, 0, 1) == 0) {
            break;
        }
        ++ccxt->id;
    }
    if (ccxt->id == num_consumers) {
        return INA_ULLC_ECLIMIT;
    }
    INA_ATOMIC_SWAP(&ccxt->c_offset->cursor, 0, ccxt->ring->cursor);
    if (ccxt->c_offset->cursor < 0) {
        ccxt->c_offset->cursor = 0;
    }
    return __ina_sem_open(ccxt);
}

INA_API(ina_rc_t) ina_ullc_consumer_destroy(ina_ullc_ctx_t **ctx)
{
    if (*ctx == NULL) {
        return INA_SUCCESS;
    }

    INA_ASSERT_EQUAL(INA_ULLC_CTX_CONSUMER, (*ctx)->type);
    INA_ATOMIC_SWAP(&(*ctx)->c_offset->alive,1,0);
    INA_ASSERT_EQUAL(0, (*ctx)->c_offset->alive);
    (*ctx)->c_offset->cursor = 0;

    if (!INA_SUCCEED(__ina_sem_close(*ctx))) {
        return INA_ERR_PUSH_LAST;
    }

    if (!INA_SUCCEED(ina_mempool_release((*ctx)->pool, 1))) {
        return INA_ERR_PUSH_LAST;
    }

    *ctx = NULL;
    return INA_SUCCESS;
}


INA_API(ina_rc_t) ina_ullc_consumer_get_pos(ina_ullc_ctx_t *ctx, int64_t *pos)
{
    INA_ASSERT_NOTNULL(ctx);
    INA_ASSERT_NOTNULL(pos);
    INA_ASSERT_EQUAL(INA_ULLC_CTX_CONSUMER, ctx->type);

    *pos = ctx->c_offset->cursor;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ullc_consumer_set_pos(ina_ullc_ctx_t *ctx, int64_t pos)
{
    int64_t cursor;

    INA_ASSERT_NOTNULL(ctx);
    INA_ASSERT_EQUAL(INA_ULLC_CTX_CONSUMER, ctx->type);

    cursor = ctx->c_offset->cursor;

    if (pos == -1 || pos > ctx->ring->next_ptr) {
        pos = ctx->ring->next_ptr;
    }    
    if (INA_ATOMIC_SWAP(&ctx->c_offset->cursor, cursor, pos) == cursor) {
        return INA_SUCCESS;
    }
    return INA_FAILURE;
}

INA_API(ina_rc_t) ina_ullc_consumer_swait(ina_ullc_ctx_t *ctx)
{
    INA_ASSERT_NOTNULL(ctx);
    INA_ASSERT_EQUAL(INA_ULLC_CTX_CONSUMER, ctx->type);

    return __ina_sem_operation(ctx, INA_ULLC_SIG_WAIT);
}

INA_API(ina_rc_t) ina_ullc_consumer_swait_begin(ina_ullc_ctx_t *ctx)
{
    INA_ASSERT_NOTNULL(ctx);
    INA_ASSERT_EQUAL(INA_ULLC_CTX_CONSUMER, ctx->type);

#ifdef INA_OS_WIN32
	INA_ATOMIC_INC(&ctx->ring->swait_count);
#endif
    return ina_ullc_consumer_swait(ctx);
}

INA_API(ina_rc_t) ina_ullc_consumer_swait_end(ina_ullc_ctx_t *ctx)
{
    INA_ASSERT_NOTNULL(ctx);
    INA_ASSERT_EQUAL(INA_ULLC_CTX_CONSUMER, ctx->type);

#ifdef INA_OS_WIN32
	INA_ATOMIC_DEC(&ctx->ring->swait_count);
#endif
	return(INA_SUCCESS);
}


INA_API(void *) ina_ullc_consumer_get(ina_ullc_ctx_t *ctx)
{
    int idx;
    void *item;
    int64_t wait_for;

    INA_ASSERT_NOTNULL(ctx);
    INA_ASSERT_EQUAL(INA_ULLC_CTX_CONSUMER, ctx->type);
    
    wait_for = ctx->c_offset->cursor;
    if (ctx->ring->cursor < wait_for) {
        return NULL;
    }
    idx = ctx->c_offset->cursor % ctx->ring->slots;
    item = &ctx->data[idx*ctx->ring->size];
    INA_ATOMIC_INC(&ctx->c_offset->cursor);
    return item;
}

static ina_rc_t 
__ina_ullc_ring_create(ina_ullc_rb_t **rb, ina_ullc_ctx_t *ctx, int version,
                            size_t size, size_t slots, int num_producers,
                            int num_consumers, const char *name, int flags)
{
    size_t mem_size;

    INA_ASSERT(version > 0);
    INA_ASSERT(slots > 0);
    INA_ASSERT(size > 0);
    INA_ASSERT_NOTNULL(name);

    if (size % 2 != 0) {
        return INA_ULLC_EBADALIGN;
    }

    mem_size = (sizeof(ina_ullc_rb_t)+size*slots)+
                 (sizeof(ina_ullc_cursor_t)*num_consumers) +
                 (sizeof(ina_ullc_cursor_t)*num_producers);

	ctx->pool = NULL;

    if (!INA_SUCCEED(ina_mempool_create(&ctx->pool, mem_size, INA_MEM_SHARED|flags, name))) {
        return INA_ERR_PUSH_LAST;
    }

    *rb = (ina_ullc_rb_t*)ina_mempool_dalloc(ctx->pool, mem_size);
    if (*rb == NULL) {
        return INA_ERR_PUSH_LAST;
    }

    if ((*rb)->magic != __INA_MAGIC_HDR || (flags&INA_MEM_SHARED_EXCL)) {
        (*rb)->magic = __INA_MAGIC_HDR;
        (*rb)->slots = slots;
        (*rb)->num_producers = num_producers;
        (*rb)->num_consumers = num_consumers;
        (*rb)->overrun_enabled = -1;
        (*rb)->cursor = -1;
        (*rb)->next_ptr = 0;
        (*rb)->alive_producers = 0;
        (*rb)->version = version;
        (*rb)->size = size;
        if (!INA_SUCCEED(__ina_sem_makekey(*rb, name))) {
            return INA_ERR_PUSH_LAST;
        }
    }
    
    if ((*rb)->version != version) {
        return INA_ULLC_EINVERSION;
    }
    if ((*rb)->size != size) {
        return INA_ULLC_EINSIZE;
    }
    if ((*rb)->slots != slots) {
        return INA_ULLC_EINSLOTS;
    }
    if ((*rb)->num_consumers != num_consumers) {
        return INA_ULLC_EINCONSUMERS;
    }
    if ((*rb)->num_producers != num_producers) {
        return INA_ULLC_EINPRODUCERS;
    }
    return INA_SUCCESS;
}


/*
 * Unix implementations
 */
#ifndef INA_OS_WIN32

static ina_rc_t
__ina_sem_makekey(ina_ullc_rb_t *rb, const char *name)
{
    INA_ASSERT_NOTNULL(rb);
    INA_ASSERT_NOTNULL(name);

    rb->semkey = INA_HASH_CSTR_TO_CRC32(name);
    return INA_SUCCESS;
}

static ina_rc_t
__ina_sem_create(ina_ullc_ctx_t *ctx)
{
    INA_ASSERT_NOTNULL(ctx);
    INA_ASSERT_EQUAL(INA_ULLC_CTX_PRODUCER, ctx->type);

    ctx->sem_handle = semget(ctx->ring->semkey, ctx->ring->num_consumers, 0666 | IPC_CREAT);
    if (ctx->sem_handle < 0) {
        return INA_ULLC_ESEMINIT;
    }
    if (semctl(ctx->sem_handle, 0, SETVAL, (int)1) == -1) {
        return INA_ULLC_ESEMINIT;
    }
     return INA_SUCCESS;
}

static ina_rc_t
__ina_sem_open(ina_ullc_ctx_t *ctx)
{
    INA_ASSERT_NOTNULL(ctx);
    ctx->sem_handle = semget(ctx->ring->semkey, 1, 0);
    return INA_SUCCESS;
}

static ina_rc_t
__ina_sem_operation(ina_ullc_ctx_t *ctx, ina_ullc_signal_type st)
{
    struct sembuf op;

    INA_ASSERT_NOTNULL(ctx);

    op.sem_op = (int)st;
    op.sem_num = 0;
    op.sem_flg = SEM_UNDO;

    if (semop(ctx->sem_handle, &op, 1) == -1) {
        return INA_ULLC_ESEMOP;
    }
    return INA_SUCCESS;
}
static ina_rc_t
__ina_sem_close(ina_ullc_ctx_t *ctx)
{
    INA_ASSERT_NOTNULL(ctx);

    if (ctx->ring->semkey != 0) {
        semctl(ctx->sem_handle, 0, IPC_RMID , 0);
    }
    return INA_SUCCESS;
}
/*
 * Windows implementations
 */
#else
static ina_rc_t
__ina_sem_makekey(ina_ullc_rb_t *rb, const char *name)
{
    ina_str_t semkey;

    INA_ASSERT_NOTNULL(rb);
    INA_ASSERT_NOTNULL(name);

    semkey = ina_str_new_fromcstr(name);
    semkey = ina_str_catcstr(semkey, "_sem");
    strcpy(rb->semkey, ina_str_cstr(semkey));
    return INA_SUCCESS;
}

static ina_rc_t
__ina_sem_create(ina_ullc_ctx_t *ctx)
{
    INA_ASSERT_NOTNULL(ctx);
    INA_ASSERT_EQUAL(INA_ULLC_CTX_PRODUCER, ctx->type);

    ctx->sem_handle = CreateSemaphore(NULL,
                        0,
                        ctx->ring->num_consumers,
                        ctx->ring->semkey);

    if (ctx->sem_handle == NULL) {
        return INA_ULLC_ESEMINIT;
    }
    return INA_SUCCESS;
}
static ina_rc_t
__ina_sem_open(ina_ullc_ctx_t *ctx)
{
    INA_ASSERT_NOTNULL(ctx);

    ctx->sem_handle = OpenSemaphore(SEMAPHORE_ALL_ACCESS,
                            FALSE,
                            ctx->ring->semkey);
    return INA_SUCCESS;
}
static ina_rc_t
__ina_sem_operation(ina_ullc_ctx_t *ctx, ina_ullc_signal_type st)
{
    INA_ASSERT_NOTNULL(ctx);

    if (st == INA_ULLC_SIG_RELEASE) {
        if (ctx->ring->swait_count > 0) {
            ReleaseSemaphore(ctx->sem_handle, (LONG)ctx->ring->swait_count, NULL);
        // FIXME: error handling
        }
    } else {
        INA_ATOMIC_INC(&ctx->ring->swait_count);
        WaitForSingleObject(ctx->sem_handle, INFINITE);
        INA_ATOMIC_DEC(&ctx->ring->swait_count);
    }
    return INA_SUCCESS;
}
static ina_rc_t
__ina_sem_close(ina_ullc_ctx_t *ctx)
{
    INA_ASSERT_NOTNULL(ctx);

    CloseHandle(ctx->sem_handle);
    return INA_SUCCESS;
}
#endif
