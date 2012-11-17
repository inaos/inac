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

/* FIXME: define as macro or inline */
static int64_t __ina_inc(volatile int64_t *);
static int64_t __ina_dec(volatile int64_t *);
static int64_t __ina_comp_swap(volatile int64_t *, int64_t, int64_t);

INA_API(ina_ullc_rb_t*) ina_ullc_ring_create(int version, size_t size, 
                            size_t slots, int num_consumers, ina_str_t name, int init)
{
    ina_ullc_rb_t *ring;
    ina_mempool_t* pool;
    size_t mem_size;

    INA_ASSERT(version > 0);
    INA_ASSERT(slots > 0);
    INA_ASSERT(size > 0);
    INA_ASSERT_NOTNULL(name);

    if (size % 2 != 0) {
        /*INA_ULLC_EBADALIGN;*/
        return NULL;
    }

    
    mem_size = (sizeof(ina_ullc_rb_t)+size*slots)+
                (sizeof(ina_ullc_consumer_t)*num_consumers);

    ring = NULL;
    pool = NULL;

    if (INA_SUCCEED(ina_mempool_create(&pool, mem_size, INA_MEM_SHARED|init, name))) {
        ring = (ina_ullc_rb_t*)ina_mempool_dalloc(pool, mem_size);
    }
    if (ring == NULL) {
        return NULL;
    }
    if (ring->magic != __INA_MAGIC_HDR || init == INA_MEM_SHARED_CREATE) { /* FIXME: Make it better */
        ina_mem_set(ring, 0, mem_size);
        ring->magic = __INA_MAGIC_HDR;
        ring->version = version;
        ring->size = size;
        ring->slots = slots;
        ring->num_consumers = num_consumers;
        ring->cursor = -1;
        ring->next_ptr = 0;
    }
    return ring;
}

INA_API(ina_rc_t) ina_ullc_ring_destroy(ina_ullc_rb_t **ring)
{
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ullc_producer_create(int id, int version, 
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
    pctx->id = id;
    pctx->ring = ring;
    pctx->data = ((void*)ring) + sizeof(ina_ullc_rb_t);
    pctx->c_offset = (ina_ullc_consumer_t*)&pctx->data[(ring->slots-1)*ring->size]+sizeof(ina_ullc_consumer_t);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ullc_producer_destroy(ina_ullc_ctx_t **ctx)
{
    return INA_SUCCESS;
}

INA_API(void *)ina_ullc_producer_claim_item(ina_ullc_ctx_t *ctx)
{
    int i;
    int like_to_write = ctx->ring->next_ptr % ctx->ring->slots;
    int slow_consumer = -1;
    int num = ctx->ring->num_consumers;
    void *item;
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
    __ina_inc(&ctx->ring->next_ptr);
    return item;
}

INA_API(ina_rc_t) ina_ullc_producer_commit_item(ina_ullc_ctx_t *ctx, void *item)
{
    __ina_inc(&ctx->ring->cursor);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ullc_consumer_create(int id, int version, ina_ullc_rb_t* ring, ina_ullc_ctx_t **ctx)
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
    ccxt->ring = ring;
    ccxt->data = ((void*)ring) + sizeof(ina_ullc_rb_t);
    cons = (ina_ullc_consumer_t*)&ccxt->data[(ring->slots-1)*ring->size]+sizeof(ina_ullc_consumer_t);
    ccxt->c_offset = &cons[id];
    ccxt->c_offset->alive = 1;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ullc_consumer_destroy(ina_ullc_ctx_t **ctx)
{
    INA_ASSERT_NOTNULL(*ctx);
    (*ctx)->c_offset->alive = 0;
    return INA_SUCCESS;
}

INA_API(void *) ina_ullc_consumer_get_item(ina_ullc_ctx_t *ctx)
{
    void *item;
    int64_t wait_for = ctx->c_offset->cursor;
    int idx;
    while (ctx->ring->cursor < wait_for) {
    }
    idx = ctx->c_offset->cursor % ctx->ring->slots;
    item = &ctx->data[idx*ctx->ring->size];
    __ina_inc(&ctx->c_offset->cursor);
    return item;
}

INA_API(void *) ina_ullc_consumer_get_item_no_wait(ina_ullc_ctx_t *ctx)
{
    void *item;
    int64_t wait_for = ctx->c_offset->cursor;
    int idx;
    if (ctx->ring->cursor < wait_for) {
        return NULL;
    }
    idx = ctx->c_offset->cursor % ctx->ring->slots;
    item = &ctx->data[idx*ctx->ring->size];
    __ina_inc(&ctx->c_offset->cursor);
    return item;
}

#ifdef INA_OS_WIN32
static int64_t 
__ina_inc(volatile int64_t *value)
{
    return(InterlockedIncrement64(value));
}
static int64_t 
__ina_dec(volatile int64_t *value)
{
    return(InterlockedDecrement64(value));
}
static int64_t 
__ina_comp_swap(volatile int64_t *value, int64_t with, int64_t cmp)
{
    return(InterlockedCompareExchange64(value, with, cmp));
}
#elif defined(__GNUC__) && ( __GNUC__ * 100 + __GNUC_MINOR__ >= 401 )
static int64_t 
__ina_inc(volatile int64_t *value)
{
    return(__sync_fetch_and_add(value, 1));
}
static int64_t 
__ina_dec(volatile int64_t *value)
{
    return(__sync_fetch_and_sub(value, 1));
}
static int64_t 
__ina_comp_swap(volatile int64_t *value, int64_t with, int64_t cmp)
{
    return(__sync_val_compare_and_swap(value, cmp, with));
}
#else
#error Compiler not supported yet!
#endif