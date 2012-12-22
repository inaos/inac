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
#include <libinac/lib.h>
#include "config.h"

#define __INA_MAGIC_HDR 'Z'

#ifdef INA_OS_WIN32
#define __INA_SEMKEY "ULLC_SEM_"
#else
#define __INA_SEMKEY 0x300
#endif

#ifdef INA_OS_WIN32
#define __INA_ULLC_INC(vv_ptr) InterlockedIncrement64(vv_ptr)
#define __INA_ULLC_DEC(vv_ptr) InterlockedDecrement64(vv_ptr)
#elif defined(__GNUC__) && ( __GNUC__ * 100 + __GNUC_MINOR__ >= 401 )
#define __INA_ULLC_INC(vv_ptr) __sync_fetch_and_add(vv_ptr, 1)
#define __INA_ULLC_DEV(vv_ptr) __sync_fetch_and_sub(vv_ptr, 1)
#else
#error Compiler not supported yet for ULLC!
#endif

/* make unique sem key */
static ina_rc_t __ina_sem_makekey(ina_ullc_rb_t*, const ina_str_t);
/* create semaphore */
static ina_rc_t __ina_sem_create(ina_ullc_ctx_t*);
/* open semaphore */
static ina_rc_t __ina_sem_open(ina_ullc_ctx_t*);
/* close semphore */
static ina_rc_t __ina_sem_close(ina_ullc_ctx_t*);
/* release semphore */
static ina_rc_t __ina_sem_operation(ina_ullc_ctx_t*, ina_ullc_signal_type st);


INA_API(ina_rc_t) ina_ullc_ring_create(ina_ullc_rb_t **rb, int version,
                            size_t size, size_t slots, int num_consumers,
                            const ina_str_t name, int flags)
{
    ina_mempool_t *pool;
    size_t mem_size;

	INA_ASSERT(version > 0);
    INA_ASSERT(slots > 0);
    INA_ASSERT(size > 0);
    INA_ASSERT_NOTNULL(name);

    if (size % 2 != 0) {
        return INA_ULLC_EBADALIGN;
    }

    mem_size = (sizeof(ina_ullc_rb_t)+size*slots)+
                 (sizeof(ina_ullc_consumer_t)*num_consumers);

    pool = NULL;

    if (!INA_SUCCEED(ina_mempool_create(&pool, mem_size, INA_MEM_SHARED|flags, name))) {
        return ina_err_peek();
    }

    *rb = (ina_ullc_rb_t*)ina_mempool_dalloc(pool, mem_size);
    if (*rb == NULL) {
        return ina_err_peek();
    }

    if ((*rb)->magic != __INA_MAGIC_HDR || flags&INA_MEM_SHARED_CREATE) {
        ina_mem_set(*rb, 0, mem_size);
        (*rb)->magic = __INA_MAGIC_HDR;
        (*rb)->version = version;
        (*rb)->size = size;
        (*rb)->slots = slots;
        (*rb)->num_consumers = num_consumers;
        (*rb)->cursor = -1;
        (*rb)->next_ptr = 0;
        if (!INA_SUCCEED(__ina_sem_makekey(*rb, name))) {
            return ina_err_peek();
        }
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ullc_ring_destroy(ina_ullc_rb_t **ring)
{
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ullc_producer_create(int version,
                    ina_ullc_wait_strategy ws,
                    ina_ullc_rb_t *ring, ina_ullc_ctx_t **ctx)
{
    ina_ullc_ctx_t* pctx;

    INA_ASSERT_NOTNULL(ring);

    if (ring->version != version) {
        return INA_ULLC_EVERSION;
    }

    *ctx = (ina_ullc_ctx_t*)ina_mem_alloc(sizeof(ina_ullc_ctx_t));
    if (*ctx == NULL) {
        return ina_err_peek();
    }

    pctx = *ctx;
    pctx->id = 0;
    pctx->type = INA_ULLC_CTX_PRODUCER;
    pctx->ws = ws;
    pctx->ring = ring;
    pctx->data = ((unsigned char*)ring) + sizeof(ina_ullc_rb_t);
    pctx->c_offset = (ina_ullc_consumer_t*)&pctx->data[(ring->slots-1)*ring->size]+sizeof(ina_ullc_consumer_t);
    return __ina_sem_create(pctx);
}

INA_API(ina_rc_t) ina_ullc_producer_destroy(ina_ullc_ctx_t **ctx)
{
    if (*ctx == NULL) {
        return INA_SUCCESS;
    }
    INA_ASSERT_EQUAL(INA_ULLC_CTX_PRODUCER, (*ctx)->type);

    return __ina_sem_close(*ctx);
}

INA_API(void *)ina_ullc_producer_claim(ina_ullc_ctx_t *ctx)
{
    int i;
    int like_to_write;
    int slow_consumer;
    int num;
    void *item;

    INA_ASSERT_NOTNULL(ctx);
    INA_ASSERT_EQUAL(INA_ULLC_CTX_PRODUCER, ctx->type);

    slow_consumer = -1;
    like_to_write = ctx->ring->next_ptr % ctx->ring->slots;
    num = ctx->ring->num_consumers;

    while (slow_consumer > like_to_write) {
        for (i = 0; i < num; i++) {
            int read_cur;
            if (ctx->c_offset[i].alive) {
                read_cur = ctx->c_offset[i].cursor % ctx->ring->slots;
                slow_consumer = INA_ULLC_MIN(slow_consumer, read_cur);
            }
        }
    }
    item = &ctx->data[like_to_write*ctx->ring->size];
    __INA_ULLC_INC(&ctx->ring->next_ptr);
    return item;
}

INA_API(ina_rc_t) ina_ullc_producer_commit(ina_ullc_ctx_t *ctx)
{
    INA_ASSERT_NOTNULL(ctx);
    INA_ASSERT_EQUAL(INA_ULLC_CTX_PRODUCER, ctx->type);

    __INA_ULLC_INC(&ctx->ring->cursor);
    return INA_SUCCESS;
}

INA_API(int64_t) ina_ullc_producer_pos(ina_ullc_ctx_t *ctx)
{
    INA_ASSERT_NOTNULL(ctx);
    return ctx->ring->cursor;
}

INA_API(ina_rc_t) ina_ullc_producer_signal(ina_ullc_ctx_t *ctx, ina_ullc_signal_type st)
{
    INA_ASSERT_NOTNULL(ctx);
    INA_ASSERT_EQUAL(INA_ULLC_CTX_PRODUCER, ctx->type);

    /* FIMXE: maybe declare API as inline */
    return __ina_sem_operation(ctx, st);
}

INA_API(ina_rc_t) ina_ullc_consumer_create(int id, int version,
                        ina_ullc_rb_t* ring, ina_ullc_ctx_t **ctx)
{
    ina_ullc_consumer_t *cons;
    ina_ullc_ctx_t* ccxt;

    if (ring->version != version) {
        return INA_ULLC_EVERSION;
    }

    *ctx = (ina_ullc_ctx_t*)ina_mem_alloc(sizeof(ina_ullc_ctx_t));

    if (*ctx == NULL) {
        return ina_err_peek();
    }

    ccxt = *ctx;
    ccxt->id = id;
    ccxt->type = INA_ULLC_CTX_CONSUMER;
    ccxt->ws = INA_ULLC_WS_NONE;
    ccxt->sem_handle = 0;
    ccxt->ring = ring;
    ccxt->data = ((unsigned char*)ring) + sizeof(ina_ullc_rb_t);
    cons = (ina_ullc_consumer_t*)&ccxt->data[(ring->slots-1)*ring->size]+sizeof(ina_ullc_consumer_t);
    ccxt->c_offset = &cons[id];
    ccxt->c_offset->alive = 1;

	return __ina_sem_open(ccxt);
}

INA_API(ina_rc_t) ina_ullc_consumer_destroy(ina_ullc_ctx_t **ctx)
{
    if (*ctx == NULL) {
        return INA_SUCCESS;
    }

    INA_ASSERT_EQUAL(INA_ULLC_CTX_CONSUMER, (*ctx)->type);

    (*ctx)->c_offset->alive = 0;
    return INA_SUCCESS;
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
	__INA_ULLC_INC(&ctx->ring->swait_count);
#endif
	return(INA_SUCCESS);
}

INA_API(ina_rc_t) ina_ullc_consumer_swait_end(ina_ullc_ctx_t *ctx)
{
    INA_ASSERT_NOTNULL(ctx);
    INA_ASSERT_EQUAL(INA_ULLC_CTX_CONSUMER, ctx->type);

#ifdef INA_OS_WIN32
	__INA_ULLC_DEC(&ctx->ring->swait_count);
#endif
	return(INA_SUCCESS);
}


INA_API(void *) ina_ullc_consumer_get(ina_ullc_ctx_t *ctx)
{
    INA_ASSERT_NOTNULL(ctx);
    INA_ASSERT_EQUAL(INA_ULLC_CTX_CONSUMER, ctx->type);

    void *item;
    int64_t wait_for = ctx->c_offset->cursor;
    int idx;
    if (ctx->ring->cursor < wait_for) {
        return NULL;
    }
    idx = ctx->c_offset->cursor % ctx->ring->slots;
    item = &ctx->data[idx*ctx->ring->size];
    __INA_ULLC_INC(&ctx->c_offset->cursor);
    return item;
}


/*
 * Unix implementations
 */
#ifndef INA_OS_WIN32

static ina_rc_t
__ina_sem_makekey(ina_ullc_rb_t *rb, const ina_str_t name)
{
    INA_ASSERT_NOTNULL(rb);
    INA_ASSERT_NOTNULL(name);

    rb->semkey = __INA_SEMKEY;
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
    ctx->sem_handle = semget(ctx->ring->semkey, 0, 0);
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
        return INA_FAILURE;
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
__ina_sem_makekey(ina_ullc_rb_t *rb, const ina_str_t name)
{
    ina_str_t semkey;

    INA_ASSERT_NOTNULL(rb);
    INA_ASSERT_NOTNULL(name);

    semkey = ina_str_newlen(strlen(__INA_SEMKEY) + ina_str_len(name));
    semkey = ina_str_cat(semkey, name);
	semkey = ina_str_cat(semkey, "_sem");
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
        __INA_ULLC_INC(&ctx->ring->swait_count);
        WaitForSingleObject(ctx->sem_handle, INFINITE);
        __INA_ULLC_DEC(&ctx->ring->swait_count);
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
