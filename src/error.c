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

#define __INA_ERR_STATE_SIZE 32

typedef struct ina_error_state_s {
    size_t c;
    size_t ic;
    ina_error_t *errors[__INA_ERR_STATE_SIZE];
} ina_error_state_t;


static ina_error_state_t __state;

/*
 * Pack return code
 */
static ina_rc_t __ina_pack_rc(int mod, int fn, int reason, int index)
{
    return ((((uint32_t)index)&0xffL)*0x100000000)|
        ((((uint32_t)mod)&0xffL)*0x1000000)|
        ((((uint32_t)fn)&0xffL)*0x1000)|
        ((((uint32_t)reason)&0xfffL));
}

static ina_rc_t __ina_destroy_error(ina_error_t *error)
{
    INA_ASSERT_NOTNULL(error);
    INA_ASSERT_NOTNULL(error->msg);
    INA_ASSERT_NOTNULL(error->file);
    
    ina_mem_free(error->msg);
    ina_mem_free(error->file);
    ina_mem_free(error);
    return INA_SUCCESS;
}

static ina_rc_t __ina_pop_error() 
{
    int i;
    
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

INA_API(ina_rc_t) ina_err_push(int mod, int fn, int reason, ina_str_t file, int line)
{
    ina_error_t *error;

    error = (ina_error_t*)ina_mem_alloc(sizeof(ina_error_t));
    
    if (error == NULL) {
        /* FIXME */
        return INA_FAILURE;
    }
    
    if (++__state.c >= __INA_ERR_STATE_SIZE) {
        __ina_pop_error();
    }
    
    error->flags = 0;
    error->rc = __ina_pack_rc(mod, fn, reason, ++__state.ic);
    error->ts = time(NULL);
    error->file = ina_str_dup(file, NULL);
    error->line = line;
    __state.errors[__state.c] = error;
    return error->rc;
}

INA_API(ina_rc_t) ina_err_peek() 
{
    if (__state.c > 0) {
        return __state.errors[__state.c]->rc;
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_err_peek_next(ina_rc_t rc)
{
    uint32_t i;
    
    return INA_SUCCESS;
    
}
INA_API(ina_rc_t) ina_err_getinfo(ina_rc_t rc, ina_error_t *error)
{
    return INA_FAILURE;
}


INA_API(ina_rc_t) ina_err_clear(ina_rc_t rc)
{
    if (rc == INA_ERR_CLEAR_ALL) {
        while (INA_SUCCESS == __ina_pop_error());
        __state.ic = 0;
    }
    return INA_SUCCESS;
}
