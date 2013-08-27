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
#include <libinac/lib.h>
#include "config.h"

#include <libinac/utlist.h>

#include <contribs/yajl/api/yajl_parse.h>  
#include <contribs/yajl/api/yajl_gen.h>

/* 
 * If you plan to have very large buffers you might need to increase  
 * the stack size appropriately
 */
#define __INA_JSON_PARSER_DATA_STACK_SIZE 1024

struct ina_json_parser_s {
    int stack_size;
    int stack_pointer;
    ina_json_data_t *stack;
    yajl_handle handle;
    ina_rc_t error_state;
    ina_str_t error_msg;
};

struct ina_json_generator_s {
    yajl_handle handle;
    
};

static ina_rc_t __ina_json_parser_stack_create(ina_json_parser_t *p)
{
    int i;

    p->stack_size = __INA_JSON_PARSER_DATA_STACK_SIZE;
    p->stack_pointer = 0;
    for (i = 0; i < p->stack_size; i++) {
        p->stack[i] = (ina_json_data_t*)ina_mem_alloc(sizeof(ina_json_data_t));
    }
 
    return INA_SUCCESS;
}

static ina_rc_t __ina_json_parser_stack_destroy(ina_json_parser_t *p) 
{
    int i;

    for (i = 0; i < p->stack_size; i++) {
        ina_mem_free(p->stack[i]);
    }
    
    return INA_SUCCESS;
}

int __ina_json_check_and_incr_data_stack(ina_json_parser_t *p)
{
    if (p->stack_pointer++ == __INA_JSON_PARSER_DATA_STACK_SIZE) {
        p->error_state = INA_ELIMIT;
        p->error_msg = ina_str_fromcstr("Data Stack overflow");
        return yajl_status_client_canceled;
    }
    return yajl_status_ok;
}

int __ina_json_yajl_cb_null(void *ctx)
{
    ina_json_parser_t *p = (ina_json_parser_t*)ctx;
    if (__ina_json_check_and_incr_data_stack(p) != yajl_status_ok) {
        return yajl_status_client_canceled;
    }
    p->stack[p->stack_pointer]->event = INA_JSON_PARSE_EVENT_DATA_NULL;   
    return yajl_status_ok;
}

int __ina_json_yajl_cb_boolean(void *ctx, int boolVal)
{
    ina_json_parser_t *p = (ina_json_parser_t*)ctx;
    if (__ina_json_check_and_incr_data_stack(p) != yajl_status_ok) {
        return yajl_status_client_canceled;
    }
    p->stack[p->stack_pointer]->event = INA_JSON_PARSE_EVENT_DATA_BOOL;
    p->stack[p->stack_pointer]->bool_val = boolVal;
    return yajl_status_ok;
}

int __ina_json_yajl_cb_integer(void *ctx, long long integerVal)
{
    ina_json_parser_t *p = (ina_json_parser_t*)ctx;
    if (__ina_json_check_and_incr_data_stack(p) != yajl_status_ok) {
        return yajl_status_client_canceled;
    }
    p->stack[p->stack_pointer]->event = INA_JSON_PARSE_EVENT_DATA_INT;
    p->stack[p->stack_pointer]->int_val = integerVal;
    return yajl_status_ok;
}

int __ina_json_yajl_cb_double(void *ctx, double doubleVal)
{
    ina_json_parser_t *p = (ina_json_parser_t*)ctx;
    if (__ina_json_check_and_incr_data_stack(p) != yajl_status_ok) {
        return yajl_status_client_canceled;
    }
    p->stack[p->stack_pointer]->event = INA_JSON_PARSE_EVENT_DATA_DOUBLE;
    p->stack[p->stack_pointer]->double_val = doubleVal;
    return yajl_status_ok;
}

int __ina_json_yajl_cb_string(void *ctx, const unsigned char *stringVal, size_t stringLen)
{
    ina_json_parser_t *p = (ina_json_parser_t*)ctx;
    if (__ina_json_check_and_incr_data_stack(p) != yajl_status_ok) {
        return yajl_status_client_canceled;
    }
    p->stack[p->stack_pointer]->event = INA_JSON_PARSE_EVENT_DATA_STRING;
    p->stack[p->stack_pointer]->str_val = stringVal;
    p->stack[p->stack_pointer]->str_len = stringLen; 
    return yajl_status_ok;
}

int __ina_json_yajl_cb_start_map(void *ctx)
{
    ina_json_parser_t *p = (ina_json_parser_t*)ctx;
    if (__ina_json_check_and_incr_data_stack(p) != yajl_status_ok) {
        return yajl_status_client_canceled;
    }
    p->stack[p->stack_pointer]->event = INA_JSON_PARSE_EVENT_START_OBJECT;
    return yajl_status_ok;
}

int __ina_json_yajl_cb_map_key(void *ctx, const unsigned char *key, size_t stringLen)
{
    ina_json_parser_t *p = (ina_json_parser_t*)ctx;
    if (__ina_json_check_and_incr_data_stack(p) != yajl_status_ok) {
        return yajl_status_client_canceled;
    }
    p->stack[p->stack_pointer]->event = INA_JSON_PARSE_EVENT_OBJECT_KEY;
    return yajl_status_ok;
}

int __ina_json_yajl_cb_end_map(void *ctx)
{
    ina_json_parser_t *p = (ina_json_parser_t*)ctx;
    if (__ina_json_check_and_incr_data_stack(p) != yajl_status_ok) {
        return yajl_status_client_canceled;
    }
    p->stack[p->stack_pointer]->event = INA_JSON_PARSE_EVENT_START_END;
    return yajl_status_ok;
}

int __ina_json_yajl_cb_start_array(void *ctx)
{
    ina_json_parser_t *p = (ina_json_parser_t*)ctx;
    if (__ina_json_check_and_incr_data_stack(p) != yajl_status_ok) {
        return yajl_status_client_canceled;
    }
    p->stack[p->stack_pointer]->event = INA_JSON_PARSE_EVENT_START_ARRAY;
    return yajl_status_ok;
}

int __ina_json_yajl_cb_end_array(void *ctx)
{
    ina_json_parser_t *p = (ina_json_parser_t*)ctx;
    if (__ina_json_check_and_incr_data_stack(p) != yajl_status_ok) {
        return yajl_status_client_canceled;
    }
    p->stack[p->stack_pointer]->event = INA_JSON_PARSE_EVENT_END_ARRAY;
    return yajl_status_ok;
}

void *__ina_json_yajl_alloc(void *ctx, size_t sz)
{
    return ina_mem_alloc(sz);
}

void __ina_json_yajl_free(void *ctx, void *ptr)
{
    ina_mem_free(ptr);
}

void *__ina_json_yajl_realloc(void *ctx, void *ptr, size_t sz)
{
    return ina_mem_realloc(ptr, sz);
}

static yajl_callbacks __ina_json_yajl_callbacks = {  
    __ina_json_yajl_cb_null,  
    __ina_json_yajl_cb_boolean,  
    __ina_json_yajl_cb_integer,  
    __ina_json_yajl_cb_double,  
    NULL,
    __ina_json_yajl_cb_string,  
    __ina_json_yajl_cb_start_map,  
    __ina_json_yajl_cb_map_key,  
    __ina_json_yajl_cb_end_map,  
    __ina_json_yajl_cb_start_array,  
    __ina_json_yajl_cb_end_array  
};

static yajl_alloc_funcs __ina_json_yajl_alloc_funcs = {
    __ina_json_yajl_alloc,
    __ina_json_yajl_realloc,
    __ina_json_yajl_free,
    NULL /* FIXME: here we should pass a mem-pool */
};

INA_API(ina_rc_t) ina_json_init(ina_json_ctx_t **ctx, int parser_pool_size, 
                                int generator_pool_size)
{
    int i;

    *ctx = (ina_json_ctx_t*)ina_mem_alloc(sizeof(ina_json_ctx_t));
    (*ctx)->parser_pool_size = parser_pool_size;
    (*ctx)->generator_pool_size = generator_pool_size;
    
    for (i = 0; i < parser_pool_size; i++) {
        ina_json_parser_t *p = (ina_json_parser_t*)ina_mem_alloc(sizeof(struct ina_json_parser_s));
        if (!INA_SUCCEED(__ina_json_parser_stack_create(p))) {
            return INA_ERROR_PUSH_LAST;
        }
        p->error_state = INA_SUCCESS;
        p->error_msg = NULL;
        p->handle = yajl_alloc(__ina_json_yajl_callbacks, __ina_json_yajl_alloc_funcs, p);
        DL_APPEND((*ctx)->parsers, p);
    }

    for (i = 0; i < generator_pool_size; i++) {
        ina_json_generator_t *g = (ina_json_generator_t*)ina_mem_alloc(sizeof(struct ina_json_generator_s));
        g->handle = yajl_gen_alloc(__ina_json_yajl_alloc_funcs);
        DL_APPEND((*ctx)->generators, g);
    }
   
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_json_destroy(ina_json_ctx_t **ctx)
{
    ina_json_ctx_t *c = *ctx;
    ina_json_parser_t *p, *ptmp;
    ina_json_generator_t *g, *gtmp;
    int cnt = 0;

    INA_ASSERT_NOT_NULL(*ctx);

    DL_FOREACH_SAFE(c->parsers, p, ptmp) {
        __ina_json_parser_stack_destroy(p);
        DL_DELETE(c->parsers, p);
        ina_mem_free(p);
        cnt++;
    }

    if (cnt != c->parser_pool_size) {
        return INA_JSON_EPOOLF;
    }
    
    cnt = 0;

    DL_FOREACH_SAFE(c->generators, g, gtmp) {
        DL_DELETE(c->generators, g);
        ina_mem_free(g);
        cnt++;
    }

    if (cnt != c->generator_pool_size) {
         return INA_JSON_EPOOLF;
    }

    ina_mem_free(*ctx);

    *ctx = NULL;

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_json_parser_borrow(ina_json_ctx_t *ctx, ina_json_parser_t **p)
{
    INA_ASSERT_NOTNULL(ctx);

    /* we ran out of parsers */
    if (ctx->parsers == NULL) {
        *p = NULL;
        return INA_JSON_EPOOLE;
    }

    /* return the head and delete from the list */
    *p = ctx->parsers;
    DL_DELETE(ctx->parsers, *p);

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_json_parser_release(ina_json_ctx_t *ctx, ina_json_parser_t **p)
{
    ina_json_parser_t *parser = *p;

    INA_ASSERT_NOTNULL(ctx);
    INA_ASSERT_NOTNULL(parser);

    /* return parser */
    DL_APPEND(ctx->parsers, parser);
    *p = NULL;

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_json_generator_borrow(ina_json_ctx_t *ctx, ina_json_generator_t **g)
{
    INA_ASSERT_NOTNULL(ctx);

    /* we ran out of parsers */
    if (ctx->generators == NULL) {
        *p = NULL;
        return INA_JSON_EPOOLE;
    }

    /* return the head and delete from the list */
    *p = ctx->generators;
    DL_DELETE(ctx->generators, *g);

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_json_generator_release(ina_json_ctx_t *ctx, ina_json_generator_t **g)
{
    ina_json_generator_t *generator = *g;

    INA_ASSERT_NOTNULL(ctx);
    INA_ASSERT_NOTNULL(generator);

    /* return parser */
    DL_APPEND(ctx->generators, generator);
    *g = NULL;

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_json_parser_execute(ina_json_parser_t *p, unsigned char *buffer, size_t buf_len)
{
    yajl_status s;

    if (p->stack_pointer != 0) {
        return INA_JSON_ERROR(INA_EOVRFL, "Stack not reset, consume all pending data items first");        
    }

    s  = yajl_parse(p->handle, buffer, buf_len);
    if (s != yajl_status_ok) {
        /* FIXME: handle error */
        if (s == yajl_status_client_canceled) {
        }
        else {
        }
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_json_parser_try_data(ina_json_parser_t *p, ina_json_data_t **data)
{
    INA_ASSERT_NOTNULL(p);

    if (p->stack_pointer > 0) {
        ina_json_data_t *d = p->stack[p->stack_pointer];
        if (!INA_SUCCEED(p->error_state)) {
            char em[128];
            strcpy(em, ina_str_cstr(d->error_msg));
            ina_str_destroy(d->error_msg);
            return INA_JSON_ERROR(p->error_state, em);
        }
        *data = d;
        p->stack_pointer--;
    }
    else {
        *data = NULL;
    }
 
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_json_generator_start_object(ina_json_generator_t *g)
{
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_json_generator_end_object(ina_json_generator_t *g)
{
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_json_generator_start_array(ina_json_generator_t *g)
{
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_json_generator_end_array(ina_json_generator_t *g)
{
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_json_generator_add_null(ina_json_generator_t *g)
{
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_json_generator_add_boolean(ina_json_generator_t *g, int bool_val)
{
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_json_generator_add_integer(ina_json_generator_t *g, int64_t int_val)
{
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_json_generator_add_double(ina_json_generator_t *g, double dbl_val)
{
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_json_generator_add_string(ina_json_generator_t *g, const char *str, size_t len)
{
    return INA_SUCCESS;
}

