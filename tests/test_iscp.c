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

static int __send_count = 0;
static int __recv_count = 0;
static int __p_count = 0;
 
static ina_rc_t __null_send_cb(ina_iscp_ctx_t *ctx, size_t size, const unsigned char *buf)
{
    INA_ASSERT_NOTNULL(buf);
    ++__send_count;
    return INA_SUCCESS;
}

static ina_rc_t __null_recv_cb(ina_iscp_ctx_t *ctx, size_t *size, unsigned char *buf)
{   
    INA_ASSERT_NOTNULL(buf);
    ++__recv_count;
    return INA_SUCCESS;
}

static ina_rc_t __null_handler(int cmd_id, int count, ina_iscp_param_t *params)
{
    __p_count = 0;
    return INA_SUCCESS;
}

static ina_rc_t __null_handler2(int cmd_id, int count, ina_iscp_param_t *params)
{   
    while (params) {
        ++__p_count;
    }
    return INA_SUCCESS;
}

void test_iscp_send_local()
{
    ina_iscp_ctx_t ctx;
    ctx.data = NULL;

    __send_count = 0;

    INA_TRACE("test_iscp_send_local");
    INA_ASSERT_SUCCEED(ina_iscp_reset());
    INA_ASSERT_FAILURE(ina_iscp_send(&ctx, 1, 
                             INA_ISCP_TYPE_INT64, 300,
                             INA_ISCP_TYPE_DBL, 3.2,
                             INA_ISCP_TYPE_STR, "test"));
    INA_ASSERT_EQUAL(0, __send_count);       
    INA_ASSERT_SUCCEED(ina_iscp_init(__null_send_cb, __null_recv_cb));
    INA_ASSERT_FAILURE(ina_iscp_send(&ctx, 1, 
                            INA_ISCP_TYPE_INT64, 300,
                            INA_ISCP_TYPE_DBL, 3.2,
                            INA_ISCP_TYPE_STR, "test"));
    INA_ASSERT_SUCCEED(ina_iscp_register(1, 3, NULL));
    INA_ASSERT_SUCCEED(ina_iscp_send(&ctx, 1, 
                             INA_ISCP_TYPE_INT64, 20,
                             INA_ISCP_TYPE_DBL, 5.2,
                             INA_ISCP_TYPE_STR, "test-2"));
    INA_ASSERT_EQUAL(1, __send_count);
}

void test_iscp_setup()
{
     INA_TRACE("test_iscp_setup");
     INA_ASSERT_SUCCEED(ina_iscp_init(__null_send_cb, __null_recv_cb));
     INA_ASSERT_FAILURE(ina_iscp_init(NULL, __null_recv_cb));
     INA_ASSERT_FAILURE(ina_iscp_init(NULL, NULL));
     INA_ASSERT_FAILURE(ina_iscp_init(__null_send_cb, NULL));
     INA_ASSERT_SUCCEED(ina_iscp_init(__null_send_cb, __null_recv_cb));
     INA_ASSERT_SUCCEED(ina_iscp_register(1, 3, __null_handler));
     INA_ASSERT_SUCCEED(ina_iscp_register(1, 3, __null_handler));
     INA_ASSERT_SUCCEED(ina_iscp_register(1, 3, __null_handler2));
     INA_ASSERT_FAILURE(ina_iscp_register(1, 2, __null_handler));
     INA_ASSERT_FAILURE(ina_iscp_register(1, 4, __null_handler2));
     INA_ASSERT_SUCCEED(ina_iscp_reset());
     INA_ASSERT_FAILURE(ina_iscp_register(1, 4, __null_handler2));
     INA_ASSERT_SUCCEED(ina_iscp_init(__null_send_cb, __null_recv_cb));
     INA_ASSERT_SUCCEED(ina_iscp_register(1, 4, __null_handler2));
}