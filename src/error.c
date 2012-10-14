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

#define __INA_ERR_STATE_SIZE (32)

typedef struct ina_error_state_s {
    size_t c;
    size_t ic;
    ina_error_t *errors[__INA_ERR_STATE_SIZE];
} ina_error_state_t;

static ina_rc_t __ina_destroy_error(ina_error_t *error);
static ina_rc_t __ina_pop_error();

/* global error state */
static ina_error_state_t __state;

INA_API(ina_rc_t) ina_err_push(int mod, int fn, int reason, ina_str_t file, 
                               int line, ina_str_t msg)
{
    ina_error_t *error;
    
    INA_ASSERT(mod <= 64);
    INA_ASSERT(fn <= 32);
    INA_ASSERT(reason <= 512);
    INA_ASSERT_NOTNULL(file);
    INA_ASSERT(line > 0);
    INA_ASSERT_NOTNULL(msg);
    
    error = (ina_error_t*)ina_mem_alloc(sizeof(ina_error_t));
    
    if (error == NULL) {
        /* FIXME */
        return INA_FAILURE;
    }
    
    if (__state.c == __INA_ERR_STATE_SIZE) {
        if (__ina_pop_error() == INA_FAILURE) {
            /* FIXME */
            return INA_FAILURE;
        };
    }
    
    error->rc = INA_RC_PACK(mod, fn, reason, ++__state.ic);
    error->ts = time(NULL); /* FIXME: use own time value */
    error->file = ina_str_dup(file, NULL);
    error->line = line;
    error->msg = ina_str_dup(msg, NULL);
    
    __state.errors[__state.c] = error;
    __state.c++;
    return error->rc;
}

INA_API(ina_rc_t) ina_err_peek() 
{
    if (__state.c > 0) {
        return __state.errors[__state.c-1]->rc;
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_err_peek_next(ina_rc_t rc)
{
    size_t i;
    size_t k;
    int m;
    
    INA_ASSERT(INA_RC_ID(rc) <= __state.ic);
    
    if (rc == INA_ERR_PEEK_FIRST) {
        return ina_err_peek();
    }
    
    i = INA_RC_ID(rc);
    if (i <= __state.ic) {
        m = i % __INA_ERR_STATE_SIZE;
        if (m > 0) {
            k = (m * __INA_ERR_STATE_SIZE) - i;
        }
        INA_ASSERT(k > __state.c);
        if (k <= __state.c) {
            return __state.errors[k]->rc;
        }
    }
    return INA_SUCCESS;
    
}

INA_API(ina_rc_t) ina_err_peek_last()
{
    if (__state.c > 0) {
        return __state.errors[0]->rc;
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_err_clear(ina_rc_t rc)
{
    if (rc == INA_ERR_STATE_CLEAR) {
        while (INA_SUCCESS == __ina_pop_error());
        __state.ic = 0;
        __state.c = 0; /* Should be already 0 */
        INA_TRACE("error state clear");
    } else {
        /* TODO: Mark as handled */
    }
    return INA_SUCCESS;
}

static ina_rc_t
__ina_destroy_error(ina_error_t *error)
{
    INA_ASSERT_NOTNULL(error);
    INA_ASSERT_NOTNULL(error->msg);
    INA_ASSERT_NOTNULL(error->file);
    
    ina_mem_free(error->msg);
    ina_mem_free(error->file);
    ina_mem_free(error);
    return INA_SUCCESS;
}

static ina_rc_t 
__ina_pop_error() 
{
    size_t i;
    
    if (__state.c > 0) {
        __ina_destroy_error(__state.errors[0]);
        for (i = 1; i < __state.c; ++i) {
            __state.errors[i-1] = __state.errors[i];
        }
        --__state.c;
        return INA_SUCCESS;
    }
    return INA_FAILURE;
}