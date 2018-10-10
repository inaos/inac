/*
 * Copyright INAOS GmbH, Thalwil, 2012-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>
#include "config.h"

#define __INA_MAGIC_HDR 'Z'

#ifdef INA_OS_WIN32
#define __INA_SEMKEY "ULLC_SEM_"
#else
#define __INA_SEMKEY 0x300
#endif

/* ullc context */
struct ina_ullc_ctx_s {
    int id;                         /* id of consumer or producer */
    ina_mempool_t *pool;            /* memory-pool */
    ina_ullc_ctx_type_t type;       /* type of context */
    ina_handle_t sem_handle;        /* semaphore handle */
    ina_ullc_wait_strategy ws;      /* wait strategy */
    ina_ullc_rb_t *ring;            /* ring buffer */
    ina_ullc_cursor_t *c_offset;    /* consumer(s) */
    ina_ullc_cursor_t *p_offset;    /* prodducers */
    unsigned char *data;            /* slot data */
};

/* ring buffer (shared mem) */
struct ina_ullc_rb_s {
    char magic;
    int32_t version;
    int32_t num_consumers;
    int32_t num_producers;
    int64_t size;
    int64_t slots;
    INA_VOLATILE int64_t cursor;
    INA_VOLATILE int64_t next_ptr;
    INA_VOLATILE int64_t swait_count;
    INA_VOLATILE int64_t alive_producers;
    INA_VOLATILE int64_t overrun_enabled;
    ina_semkey_t semkey; /*FIXME: multiple producer */
};

/* ULLC ring cursor */
struct ina_ullc_cursor_s {
    INA_VOLATILE int64_t alive;
    INA_VOLATILE int64_t cursor;
};

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

    INA_VERIFY_NOT_NULL(info);

    if (INA_FAILED(ina_mempool_new(sizeof(ina_ullc_rb_t), name, INA_MEM_SHARED, &m))) {
        return ina_err_get_rc();
    }
    rb = (ina_ullc_rb_t*)ina_mempool_dalloc(m, sizeof(ina_ullc_rb_t));
    if (rb == NULL) {
        ina_mempool_free(&m);
        return ina_err_get_rc();
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
    /*info->mem_size = m->size;*/
    info->slot_size = (size_t)rb->size;
    info->num_slots = (size_t)rb->slots;
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

    ina_mempool_free(&m);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ullc_producer_new(int version, size_t size,
                                        size_t slots, int num_producers, int num_consumers,
                                        const char *name, ina_ullc_wait_strategy ws,
                                        ina_ullc_ctx_t **ctx)
{
    ina_ullc_ctx_t *pctx;
    
    INA_VERIFY_NOT_NULL(name);
    INA_VERIFY(strlen(name));
    INA_VERIFY_NOT_NULL(ctx);

    *ctx = NULL;

    if (version <= 0) {
        return INA_ERROR(INA_ES_ARGUMENT | INA_ERR_INVALID);
    }
    if (size == 0) {
        return INA_ERROR(INA_ES_ARGUMENT | INA_ERR_INVALID);
    }
    if (slots == 0) {
        return INA_ERROR(INA_ES_ARGUMENT | INA_ERR_INVALID);
    }
    if (num_consumers <= 0) {
        return INA_ERROR(INA_ES_ARGUMENT | INA_ERR_INVALID);
    }

    *ctx = (ina_ullc_ctx_t*)ina_mem_alloc(sizeof(ina_ullc_ctx_t));
    INA_RETURN_IF_NULL(*ctx);
    ina_mem_set(*ctx, 0, sizeof(ina_ullc_ctx_t));
    pctx = *ctx;

    if (INA_FAILED(__ina_ullc_ring_create(&pctx->ring, pctx, version, size,
                                            slots, 
                                            num_producers, 
                                            num_consumers, 
                                            name, 
                                            INA_MEM_SHARED_CREATE|INA_MEM_SHARED_EXCL))) {
        ina_err_reset();
        if (INA_FAILED(__ina_ullc_ring_create(&pctx->ring, pctx, version, size,
                                            slots, 
                                            num_producers, 
                                            num_consumers, 
                                            name, 
                                            INA_MEM_SHARED_CREATE))) {
            ina_mempool_free(&(pctx)->pool);
            INA_MEM_FREE_SAFE(*ctx);
            return ina_err_get_rc();
        }
    }

    INA_ASSERT_NOTNULL(pctx->ring);

    if (pctx->ring->version != version) {
        ina_mempool_free(&(*ctx)->pool);
        INA_MEM_FREE_SAFE(*ctx);
        return INA_ERROR(INA_ES_VERSION | INA_ERR_INVALID);
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
        ina_mempool_free(&(*ctx)->pool);
        INA_MEM_FREE_SAFE(*ctx);
        return INA_ERROR(INA_ES_LIMIT | INA_ERR_EXCEEDED);
    }
    if (!INA_SUCCEED( __ina_sem_create(pctx))) {
        ina_mempool_free(&(*ctx)->pool);
        INA_MEM_FREE_SAFE(*ctx);
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ullc_reset_ring(const char *name)
{
    ina_mempool_t *m = NULL;
    ina_ullc_rb_t *rb = NULL;

    INA_VERIFY_NOT_NULL(name);
    INA_VERIFY(strlen(name));

    if (INA_FAILED(ina_mempool_new(sizeof(ina_ullc_rb_t), name, INA_MEM_SHARED, &m))) {
        return ina_err_get_rc();
    }
    rb = (ina_ullc_rb_t*)ina_mempool_dalloc(m, sizeof(ina_ullc_rb_t));
    if (rb == NULL) {
        ina_mempool_free(&m);
        return ina_err_get_rc();
    }
    rb->magic = 0;
    ina_mempool_free(&m);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ullc_overrun_enable(ina_ullc_ctx_t *ctx) 
{
    INA_VERIFY_NOT_NULL(ctx);
    if (INA_ATOMIC_SWAP(&ctx->ring->overrun_enabled, INT64_MAX, -1) == INT64_MAX) {
        return INA_SUCCESS;
    }
    return INA_ERROR(INA_ES_OPERATION | INA_ERR_FAILED);
}

INA_API(ina_rc_t) ina_ullc_overrun_disable(ina_ullc_ctx_t *ctx)
{
    INA_VERIFY_NOT_NULL(ctx);
    if (INA_ATOMIC_SWAP(&ctx->ring->overrun_enabled, -1, INT64_MAX) == -1) {
        return INA_SUCCESS;
    }
    return INA_ERROR(INA_ES_OPERATION | INA_ERR_FAILED);
}

INA_API(ina_rc_t) ina_ullc_producer_reset(ina_ullc_ctx_t *ctx)
{
    INA_UNUSED(ctx);
    return INA_SUCCESS;
}

INA_API(void) ina_ullc_producer_free(ina_ullc_ctx_t **ctx)
{
    INA_FREE_CHECK(ctx);
    INA_ASSERT_EQUAL(INA_ULLC_CTX_PRODUCER, (*ctx)->type);
    
    INA_ATOMIC_SWAP(&(*ctx)->p_offset->alive,1,0);
    INA_ASSERT_EQUAL(0, (*ctx)->p_offset->alive);

    /* Force re-initialization on last producers */
    if (INA_ATOMIC_DEC(&(*ctx)->ring->alive_producers) == 1) {
        (*ctx)->ring->magic = 0;
    }

    INA_MUST_SUCCEED(__ina_sem_close(*ctx));
    ina_mempool_free(&(*ctx)->pool);
    INA_MEM_FREE_SAFE(*ctx);
}

INA_API(void *)ina_ullc_producer_claim(ina_ullc_ctx_t *ctx)
{
    int64_t i;
    int64_t like_to_write;
    int64_t slow_consumer;
    int64_t num;
    void *item;

    INA_ASSERT_NOTNULL(ctx);
    INA_ASSERT(INA_ULLC_CTX_PRODUCER == ctx->type);

    slow_consumer = ctx->ring->overrun_enabled;
    like_to_write = ctx->ring->next_ptr % ctx->ring->slots;
    num = ctx->ring->num_consumers;

    while (slow_consumer > like_to_write) {
        slow_consumer = -1;
        for (i = 0; i < num; ++i) {
            int64_t read_cur;
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
    INA_VERIFY_NOT_NULL(ctx);
    INA_VERIFY(INA_ULLC_CTX_PRODUCER == ctx->type);
    INA_ATOMIC_INC(&ctx->ring->next_ptr);
    INA_ATOMIC_INC(&ctx->ring->cursor);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ullc_producer_get_pos(ina_ullc_ctx_t *ctx, int64_t *pos)
{
    INA_VERIFY_NOT_NULL(ctx);
    INA_VERIFY_NOT_NULL(pos);
    INA_VERIFY(INA_ULLC_CTX_PRODUCER == ctx->type);

    *pos = ctx->ring->next_ptr;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ullc_producer_signal(ina_ullc_ctx_t *ctx, ina_ullc_signal_type st)
{
    INA_VERIFY_NOT_NULL(ctx);
    /*INA_ASSERT_EQUAL(INA_ULLC_CTX_PRODUCER, ctx->type);*/

    /* FIMXE: maybe declare API as inline */
    return __ina_sem_operation(ctx, st);
}

INA_API(ina_rc_t) ina_ullc_consumer_new(int version, size_t size,
                                        size_t slots, int num_producers, int num_consumers,
                                        const char *name, ina_ullc_ctx_t **ctx)
{
    ina_ullc_cursor_t *cons;
    ina_ullc_ctx_t* ccxt;

    INA_VERIFY_NOT_NULL(name);
    INA_VERIFY(strlen(name));
    INA_VERIFY_NOT_NULL(ctx);

    *ctx = (ina_ullc_ctx_t*)ina_mem_alloc(sizeof(ina_ullc_ctx_t));
    INA_RETURN_IF_NULL(*ctx);
    ina_mem_set(*ctx, 0, sizeof(ina_ullc_ctx_t));

    ccxt = *ctx;
    ccxt->type = INA_ULLC_CTX_CONSUMER;
    if (INA_FAILED(__ina_ullc_ring_create(&ccxt->ring, ccxt, version, size,
            slots, num_producers, num_consumers, name, 0))) {
        ina_mempool_free(&(*ctx)->pool);
        INA_MEM_FREE_SAFE(*ctx);
        return ina_err_get_rc();
    }

    if (ccxt->ring->version != version) {
        ina_mempool_free(&(*ctx)->pool);
        INA_MEM_FREE_SAFE(*ctx);
        return INA_ERROR(INA_ES_VERSION | INA_ERR_INVALID);
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
        ina_mempool_free(&(*ctx)->pool);
        INA_MEM_FREE_SAFE(*ctx);
        return INA_ERROR(INA_ES_LIMIT | INA_ERR_EXCEEDED);
    }
    INA_ATOMIC_SWAP(&ccxt->c_offset->cursor, 0, ccxt->ring->cursor);
    if (ccxt->c_offset->cursor < 0) {
        ccxt->c_offset->cursor = 0;
    }
    if (INA_FAILED(__ina_sem_open(ccxt))) {
        ina_mempool_free(&(*ctx)->pool);
        INA_MEM_FREE_SAFE(*ctx);
    }
    return INA_SUCCESS;
}

INA_API(void) ina_ullc_consumer_free(ina_ullc_ctx_t **ctx)
{
    INA_FREE_CHECK(ctx);
    INA_ASSERT(INA_ULLC_CTX_CONSUMER == (*ctx)->type);

    INA_ATOMIC_SWAP(&(*ctx)->c_offset->alive,1,0);
    INA_ASSERT_EQUAL(0, (*ctx)->c_offset->alive);
    (*ctx)->c_offset->cursor = 0;

    INA_MUST_SUCCEED(__ina_sem_close(*ctx));
    ina_mempool_free(&(*ctx)->pool);
    INA_MEM_FREE_SAFE(*ctx);
}


INA_API(ina_rc_t) ina_ullc_consumer_get_pos(ina_ullc_ctx_t *ctx, int64_t *pos)
{
    INA_VERIFY_NOT_NULL(ctx);
    INA_VERIFY_NOT_NULL(pos);
    INA_VERIFY(INA_ULLC_CTX_CONSUMER == ctx->type);

    *pos = ctx->c_offset->cursor;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ullc_consumer_set_pos(ina_ullc_ctx_t *ctx, int64_t pos)
{
    int64_t cursor;

    INA_VERIFY_NOT_NULL(ctx);
    INA_VERIFY(INA_ULLC_CTX_CONSUMER == ctx->type);

    cursor = ctx->c_offset->cursor;

    if (pos == -1 || pos > ctx->ring->next_ptr) {
        pos = ctx->ring->next_ptr;
    }    
    if (INA_ATOMIC_SWAP(&ctx->c_offset->cursor, cursor, pos) == cursor) {
        return INA_SUCCESS;
    }
    return INA_ERROR(INA_ES_OPERATION | INA_ERR_FAILED);
}

INA_API(ina_rc_t) ina_ullc_consumer_swait(ina_ullc_ctx_t *ctx)
{
    INA_VERIFY_NOT_NULL(ctx);
    INA_VERIFY(INA_ULLC_CTX_CONSUMER == ctx->type);

    return __ina_sem_operation(ctx, INA_ULLC_SIG_WAIT);
}

INA_API(ina_rc_t) ina_ullc_consumer_swait_begin(ina_ullc_ctx_t *ctx)
{
    INA_VERIFY_NOT_NULL(ctx);
    INA_VERIFY(INA_ULLC_CTX_CONSUMER == ctx->type);

#ifdef INA_OS_WIN32
	INA_ATOMIC_INC(&ctx->ring->swait_count);
#endif
    return ina_ullc_consumer_swait(ctx);
}

INA_API(ina_rc_t) ina_ullc_consumer_swait_end(ina_ullc_ctx_t *ctx)
{
    INA_VERIFY_NOT_NULL(ctx);
    INA_VERIFY(INA_ULLC_CTX_CONSUMER == ctx->type);

#ifdef INA_OS_WIN32
	INA_ATOMIC_DEC(&ctx->ring->swait_count);
#endif
	return INA_SUCCESS;
}


INA_API(void *) ina_ullc_consumer_get(ina_ullc_ctx_t *ctx)
{
    int64_t idx;
    void *item;
    int64_t wait_for;

    INA_ASSERT_NOTNULL(ctx);
    INA_ASSERT(INA_ULLC_CTX_CONSUMER == ctx->type);
    
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
    INA_ASSERT(size < INT64_MAX);
    INA_ASSERT(slots < INT64_MAX);
    INA_ASSERT_NOTNULL(name);

    int64_t rsize = (int64_t)size;
    int64_t rslots = (int64_t)slots;

    if (size % 2 != 0) {
        return INA_ERROR(INA_ES_MEMORY | INA_ERR_NOT_ALIGNED);
    }

    mem_size = (sizeof(ina_ullc_rb_t)+rsize*rslots)+
                 (sizeof(ina_ullc_cursor_t)*num_consumers) +
                 (sizeof(ina_ullc_cursor_t)*num_producers);

	ctx->pool = NULL;

    if (INA_FAILED(ina_mempool_new(mem_size, name, INA_MEM_SHARED | flags, &ctx->pool))) {
        return ina_err_get_rc();
    }

    *rb = (ina_ullc_rb_t*)ina_mempool_dalloc(ctx->pool, mem_size);
    if (*rb == NULL) {
        return ina_err_get_rc();
    }

    if ((*rb)->magic != __INA_MAGIC_HDR || (flags&INA_MEM_SHARED_EXCL)) {
        (*rb)->magic = __INA_MAGIC_HDR;
        (*rb)->slots = rslots;
        (*rb)->num_producers = num_producers;
        (*rb)->num_consumers = num_consumers;
        (*rb)->overrun_enabled = -1;
        (*rb)->cursor = -1;
        (*rb)->next_ptr = 0;
        (*rb)->alive_producers = 0;
        (*rb)->version = version;
        (*rb)->size = rsize;
        if (INA_FAILED(__ina_sem_makekey(*rb, name))) {
            return ina_err_get_rc();
        }
    }
    
    if ((*rb)->version != version) {
        return INA_ERROR(INA_ES_ARGUMENT | INA_ERR_INVALID);
    }
    if ((*rb)->size != rsize) {
        return INA_ERROR(INA_ES_ARGUMENT | INA_ERR_INVALID);
    }
    if ((*rb)->slots != rslots) {
        return INA_ERROR(INA_ES_ARGUMENT | INA_ERR_INVALID);
    }
    if ((*rb)->num_consumers != num_consumers) {
        return INA_ERROR(INA_ES_ARGUMENT | INA_ERR_INVALID);
    }
    if ((*rb)->num_producers != num_producers) {
        return INA_ERROR(INA_ES_ARGUMENT | INA_ERR_INVALID);
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
        return INA_OS_ERROR(INA_ES_OPERATION | INA_ERR_FAILED);
    }
    if (semctl(ctx->sem_handle, 0, SETVAL, (int)1) == -1) {
        return INA_OS_ERROR(INA_ES_OPERATION | INA_ERR_FAILED);
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
        return INA_ERROR(INA_ES_OPERATION | INA_ERR_FAILED);
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
        return INA_ERROR(INA_ES_SEMAPHORE|INA_ERR_NOT_CREATED);
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
