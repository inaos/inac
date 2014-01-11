/*
 * Copyright (c) 2013-2014, INAOS GmbH
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

#include <contribs/yajl/yajl_parse.h>  
#include <contribs/yajl/yajl_gen.h>

/* 
 * If you plan to have very large buffers you might need to increase  
 * the stack size appropriately
 */
#define __INA_JSON_PARSER_DATA_STACK_SIZE 1024

struct ina_json_parser_s {
    yajl_alloc_funcs json_alloc_funcs;
    uint32_t stack_size;
    uint32_t stack_pointer;
    ina_json_data_t *stack;
    yajl_handle handle;
    ina_rc_t error_state;
    ina_str_t error_msg;
    struct ina_json_parser_s *next;
    struct ina_json_parser_s *prev;
};

struct ina_json_gen_s {
    yajl_alloc_funcs json_alloc_funcs;
    yajl_gen handle;
    struct ina_json_gen_s *next;
    struct ina_json_gen_s *prev;  
};

static ina_rc_t __ina_parser_stack_create(ina_json_parser_t *p)
{
    p->stack_size = 0;
    p->stack_pointer = 0;
    p->stack = (ina_json_data_t*)ina_mem_alloc(
                    sizeof(ina_json_data_t)*__INA_JSON_PARSER_DATA_STACK_SIZE);
    return INA_SUCCESS;
}

static ina_rc_t __ina_parser_stack_destroy(ina_json_parser_t *p) 
{
    ina_mem_free(p->stack);
    return INA_SUCCESS;
}

INA_INLINE int __ina_check_and_incr_data_stack(ina_json_parser_t *p)
{
    if (p->stack_pointer++ == __INA_JSON_PARSER_DATA_STACK_SIZE) {
        p->error_state = INA_ELIMIT;
        p->error_msg = ina_str_new_fromcstr("Data Stack overflow");
        return INA_NO;
    }
    p->stack_size++;
    return INA_YES;
}

int __ina_yajl_cb_null(void *ctx)
{
    ina_json_parser_t *p = (ina_json_parser_t*)ctx;
    p->stack[p->stack_pointer].event = INA_JSON_PARSE_EVENT_DATA_NULL;   
    return __ina_check_and_incr_data_stack(p);
}

int __ina_yajl_cb_boolean(void *ctx, int32_t value)
{
    ina_json_parser_t *p = (ina_json_parser_t*)ctx;
    p->stack[p->stack_pointer].event = INA_JSON_PARSE_EVENT_DATA_BOOL;
    p->stack[p->stack_pointer].size = sizeof(int32_t);
    p->stack[p->stack_pointer].value.b = value;
    return __ina_check_and_incr_data_stack(p);
}

int __ina_yajl_cb_integer(void *ctx, long long value)
{
    ina_json_parser_t *p = (ina_json_parser_t*)ctx;
    p->stack[p->stack_pointer].event = INA_JSON_PARSE_EVENT_DATA_INT;
    /* FIXME: data loss */
    p->stack[p->stack_pointer].size = sizeof(long long);
    p->stack[p->stack_pointer].value.i = value;
    return __ina_check_and_incr_data_stack(p);
}

int __ina_yajl_cb_double(void *ctx, double value)
{
    ina_json_parser_t *p = (ina_json_parser_t*)ctx;
    p->stack[p->stack_pointer].event = INA_JSON_PARSE_EVENT_DATA_DOUBLE;
    p->stack[p->stack_pointer].size = sizeof(double);
    p->stack[p->stack_pointer].value.d = value;
    return __ina_check_and_incr_data_stack(p);
}

int __ina_yajl_cb_string(void *ctx, const unsigned char *value, size_t len)
{
    ina_json_parser_t *p = (ina_json_parser_t*)ctx;
    p->stack[p->stack_pointer].event = INA_JSON_PARSE_EVENT_DATA_STRING;
    p->stack[p->stack_pointer].value.s = value;
    p->stack[p->stack_pointer].size = len; 
    return __ina_check_and_incr_data_stack(p);
}

int __ina_yajl_cb_start_map(void *ctx)
{
    ina_json_parser_t *p = (ina_json_parser_t*)ctx;
    p->stack[p->stack_pointer].event = INA_JSON_PARSE_EVENT_START_OBJECT;
    p->stack[p->stack_pointer].size = 0;     
    return __ina_check_and_incr_data_stack(p);
}

int __ina_yajl_cb_map_key(void *ctx, const unsigned char *key, size_t len)
{
    ina_json_parser_t *p = (ina_json_parser_t*)ctx;
    p->stack[p->stack_pointer].event = INA_JSON_PARSE_EVENT_OBJECT_KEY;
    p->stack[p->stack_pointer].value.s = key;
    p->stack[p->stack_pointer].size = len; 
    return __ina_check_and_incr_data_stack(p);
}

int __ina_yajl_cb_end_map(void *ctx)
{
    ina_json_parser_t *p = (ina_json_parser_t*)ctx;
    p->stack[p->stack_pointer].event = INA_JSON_PARSE_EVENT_END_OBJECT;
    p->stack[p->stack_pointer].size = 0; 
    return __ina_check_and_incr_data_stack(p);
}

int __ina_yajl_cb_start_array(void *ctx)
{
    ina_json_parser_t *p = (ina_json_parser_t*)ctx;
    p->stack[p->stack_pointer].event = INA_JSON_PARSE_EVENT_START_ARRAY;
    p->stack[p->stack_pointer].size = 0; 
    return __ina_check_and_incr_data_stack(p);
}

int __ina_yajl_cb_end_array(void *ctx)
{
    ina_json_parser_t *p = (ina_json_parser_t*)ctx;
    p->stack[p->stack_pointer].event = INA_JSON_PARSE_EVENT_END_ARRAY;    
    p->stack[p->stack_pointer].size = 0; 
    return __ina_check_and_incr_data_stack(p);
}

void *__ina_yajl_alloc(void *ctx, size_t sz)
{
    ina_mempool_t *pool = (ina_mempool_t*)ctx;
    INA_ASSERT_NOTNULL(ctx);
    return ina_mempool_dalloc(pool, sz);
}

void __ina_yajl_free(void *ctx, void *ptr)
{
}

void *__ina_yajl_realloc(void *ctx, void *ptr, size_t sz)
{
    ina_mempool_t *pool = (ina_mempool_t*)ctx;
    INA_ASSERT_NOTNULL(ctx);
    return ina_mempool_dalloc(pool, sz);
}

static yajl_callbacks __yajl_callbacks = {  
    __ina_yajl_cb_null, 
    __ina_yajl_cb_boolean, 
    __ina_yajl_cb_integer, 
    __ina_yajl_cb_double, 
    NULL,
    __ina_yajl_cb_string, 
    __ina_yajl_cb_start_map,
    __ina_yajl_cb_map_key,
    __ina_yajl_cb_end_map,
    __ina_yajl_cb_start_array,
    __ina_yajl_cb_end_array
};


INA_API(ina_rc_t) ina_json_init(ina_json_ctx_t **ctx, 
                                uint32_t parser_pool_size, 
                                uint32_t generator_pool_size)
{
    size_t i;

    *ctx = (ina_json_ctx_t*)ina_mem_alloc(sizeof(ina_json_ctx_t));
    if (*ctx == NULL) {
        return INA_ERR_PUSH_LAST;
    }

    (*ctx)->parser_pool_size = parser_pool_size;
    (*ctx)->generator_pool_size = generator_pool_size;

    if (!INA_SUCCEED(ina_mempool_create(&(*ctx)->mempool, 
                                        1024*1024*5, 
                                        INA_MEM_DYNAMIC, 
                                        NULL))) {
        ina_mem_free(*ctx);
        return INA_ERR_PUSH_LAST;
    }

    for (i = 0; i < parser_pool_size; i++) {
        ina_json_parser_t *p = (ina_json_parser_t*)ina_mem_alloc(
                                            sizeof(struct ina_json_parser_s));
        if (p == NULL) {
            return INA_ERR_PUSH_LAST;
        }
        if (!INA_SUCCEED(__ina_parser_stack_create(p))) {
            return INA_ERR_PUSH_LAST;
        }
        p->json_alloc_funcs.ctx = (void*)(*ctx)->mempool;
        p->json_alloc_funcs.malloc = __ina_yajl_alloc;
        p->json_alloc_funcs.realloc = __ina_yajl_realloc;
        p->json_alloc_funcs.free = __ina_yajl_free;
        p->error_state = INA_SUCCESS;
        p->error_msg = NULL;
        p->handle = yajl_alloc(&__yajl_callbacks, &p->json_alloc_funcs, p);
        DL_APPEND((*ctx)->parsers, p);
    }

    for (i = 0; i < generator_pool_size; i++) {
        ina_json_gen_t *g = (ina_json_gen_t*)ina_mem_alloc(
                                    sizeof(struct ina_json_gen_s));
        g->json_alloc_funcs.ctx = (void*)(*ctx)->mempool;
        g->json_alloc_funcs.malloc = __ina_yajl_alloc;
        g->json_alloc_funcs.realloc = __ina_yajl_realloc;
        g->json_alloc_funcs.free = __ina_yajl_free;
        g->handle = yajl_gen_alloc(&g->json_alloc_funcs);
        DL_APPEND((*ctx)->generators, g);
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_json_destroy(ina_json_ctx_t **ctx)
{
    ina_json_ctx_t    *c = *ctx;
    ina_json_parser_t *p, *ptmp;
    ina_json_gen_t    *g, *gtmp;
    int cnt = 0;

    INA_ASSERT_NOTNULL(*ctx);

    DL_FOREACH_SAFE(c->parsers, p, ptmp) {
        __ina_parser_stack_destroy(p);
        DL_DELETE(c->parsers, p);
        yajl_free(p->handle);
        ina_mem_free(p);
        cnt++;
    }

    if (cnt != c->parser_pool_size) {
        return INA_JSON_EPOOLF;
    }
    
    cnt = 0;

    DL_FOREACH_SAFE(c->generators, g, gtmp) {
        DL_DELETE(c->generators, g);
        yajl_gen_free(g->handle);
        ina_mem_free(g);
        cnt++;
    }

    if (cnt != c->generator_pool_size) {
         return INA_JSON_EPOOLF;
    }

    ina_mempool_release((*ctx)->mempool, INA_YES);

    ina_mem_free(*ctx);

    *ctx = NULL;

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_json_parser_borrow(ina_json_ctx_t *ctx, 
                                         ina_json_parser_t **parser)
{
    INA_ASSERT_NOTNULL(ctx);

    /* we ran out of parsers */
    if (ctx->parsers == NULL) {
        *parser = NULL;
        return INA_JSON_EPOOLE;
    }

    /* return the head and delete from the list */
    *parser = ctx->parsers;
    DL_DELETE(ctx->parsers, *parser);

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_json_parser_release(ina_json_ctx_t *ctx, 
                                          ina_json_parser_t **parser)
{
    ina_json_parser_t *p = *parser;

    INA_ASSERT_NOTNULL(ctx);
    INA_ASSERT_NOTNULL(parser);

    /* return parser */
    DL_APPEND(ctx->parsers, p);
    ina_json_parser_reset(p);

    *parser = NULL;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_json_generator_borrow(ina_json_ctx_t *ctx, 
                                            ina_json_gen_t **generator)
{
    INA_ASSERT_NOTNULL(ctx);

    /* we ran out of parsers */
    if (ctx->generators == NULL) {
        *generator = NULL;
        return INA_JSON_EPOOLE;
    }

    /* return the head and delete from the list */
    *generator = ctx->generators;
    DL_DELETE(ctx->generators, *generator);

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_json_generator_release(ina_json_ctx_t *ctx, 
                                             ina_json_gen_t **generator)
{
    ina_json_gen_t *g = *generator;
 
    INA_ASSERT_NOTNULL(ctx);
    INA_ASSERT_NOTNULL(g);

    /* return parser */
    DL_APPEND(ctx->generators, g);
    ina_json_generator_reset(g);

    *generator = NULL;

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_json_parser_execute(ina_json_parser_t *parser, 
                                          const unsigned char *buffer, 
                                          size_t buf_len,
                                          int32_t complete)
{
    yajl_status s;

    if (parser->stack_pointer > 0) {
        return INA_JSON_ERROR(INA_EOVRFL, 
            "Stack not reset, consume all pending data items" 
                "first or reset parser");        
    }

    s  = yajl_parse(parser->handle, buffer, buf_len);
    if (complete == INA_YES) {
        s = yajl_complete_parse(parser->handle);
    }
    if (s == yajl_status_ok) {
        return INA_SUCCESS;
    }
       
    /* FIXME: handle error */
    if (s != yajl_status_client_canceled) {
        unsigned char *errmsg = yajl_get_error(parser->handle, 
                                               1,
                                               buffer, 
                                               buf_len);
        INA_JSON_ERROR(INA_EINVAL, (const char*)errmsg);
        yajl_free_error(parser->handle, errmsg);
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_json_parser_try_data(ina_json_parser_t *parser, 
                                           const ina_json_data_t **data)
{
    INA_ASSERT_NOTNULL(parser);

    if (parser->stack_pointer > 0) {
        size_t i = parser->stack_size-parser->stack_pointer;
        ina_json_data_t *d = &parser->stack[i];
        if (!INA_SUCCEED(parser->error_state)) {
            return INA_JSON_ERROR(parser->error_state, 
                                  ina_str_cstr(parser->error_msg));
        }
        *data = d;
        parser->stack_pointer--;
        return INA_SUCCESS;
    }

    *data = NULL;
    return INA_FAILURE;
}

/*
 *
 */
INA_API(ina_rc_t) ina_json_parser_reset(ina_json_parser_t *parser)
{
    INA_ASSERT_NOTNULL(parser);
    yajl_free(parser->handle);
    parser->stack_pointer = 0;
    parser->stack_size = 0;
    parser->handle = yajl_alloc(&__yajl_callbacks, &parser->json_alloc_funcs, parser);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_json_generator_start_object(ina_json_gen_t *generator)
{
    INA_ASSERT_NOTNULL(generator);
    if (yajl_gen_status_ok != yajl_gen_map_open(generator->handle)) {
        return INA_JSON_ERROR(INA_ELOGIC, "Can't start object");
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_json_generator_end_object(ina_json_gen_t *generator)
{
    INA_ASSERT_NOTNULL(generator);
    if (yajl_gen_status_ok != yajl_gen_map_close(generator->handle)) {
        return INA_JSON_ERROR(INA_ELOGIC, "Can't end object");
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_json_generator_start_array(ina_json_gen_t *generator)
{
    INA_ASSERT_NOTNULL(generator);
    if (yajl_gen_status_ok != yajl_gen_array_open(generator->handle)) {
        return INA_JSON_ERROR(INA_ELOGIC, "Can't start array");
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_json_generator_end_array(ina_json_gen_t *generator)
{
    INA_ASSERT_NOTNULL(generator);
    if (yajl_gen_status_ok != yajl_gen_array_close(generator->handle)) {
        return INA_JSON_ERROR(INA_ELOGIC, "Can't end array");
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_json_generator_add_null(ina_json_gen_t *generator)
{
    INA_ASSERT_NOTNULL(generator);
    if (yajl_gen_status_ok != yajl_gen_null(generator->handle)) {
        return INA_JSON_ERROR(INA_ELOGIC, "Can't add NULL");
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_json_generator_add_boolean(ina_json_gen_t *generator, 
                                                 int32_t value)
{
    INA_ASSERT_NOTNULL(generator);
    if (yajl_gen_status_ok != yajl_gen_bool(generator->handle, value)) {
        return INA_JSON_ERROR(INA_ELOGIC, "Can't add boolean");
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_json_generator_add_integer(ina_json_gen_t *generator, 
                                                 int64_t value)
{
    INA_ASSERT_NOTNULL(generator);
    if (yajl_gen_status_ok != yajl_gen_integer(generator->handle, value)) {
        return INA_JSON_ERROR(INA_ELOGIC, "Can't add integer");
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_json_generator_add_double(ina_json_gen_t *generator, 
                                                double value)
{
    INA_ASSERT_NOTNULL(generator);
    if (yajl_gen_status_ok != yajl_gen_double(generator->handle, value)) {
        return INA_JSON_ERROR(INA_ELOGIC, "Can't add double");
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_json_generator_add_string(ina_json_gen_t *generator, 
                                                const char *str, 
                                                size_t len)
{
    INA_ASSERT_NOTNULL(generator);
    if (yajl_gen_status_ok != yajl_gen_string(generator->handle, 
                                              (const unsigned char*)str, 
                                              len)) {
        return INA_JSON_ERROR(INA_ELOGIC, "Can't add string");
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_json_generator_get_buffer(ina_json_gen_t *generator, 
                                                const unsigned char **buffer,
                                                size_t *len)
{
    INA_ASSERT_NOTNULL(generator);
    if (yajl_gen_status_ok == yajl_gen_get_buf(generator->handle, buffer, len)) {
        return INA_SUCCESS;
    }
    return INA_JSON_ENOBUF;
}

INA_API(ina_rc_t) ina_json_generator_reset(ina_json_gen_t *generator)
{
    INA_ASSERT_NOTNULL(generator);
    yajl_gen_free(generator->handle);
    generator->handle = yajl_gen_alloc(&generator->json_alloc_funcs);
    return INA_SUCCESS;
}

