/*
 * Copyright (c) 2014, INAOS GmbH
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

/*
 * Lua Echo Server
 */
INA_TEST_HELPER(ljit, lua_echo_server) {

    ina_ljit_ctx_t *ctx = NULL;
    const char *addr;
    int port;
    int ret;

    INA_TEST_HELPER_CHECK_ARGC(2);
    addr = INA_TEST_HELPER_CARG(0);
    port = INA_TEST_HELPER_IARG(1);

    if (!INA_SUCCEED(ina_ljit_init(&ctx))) {
        *retval = ina_err_peek();
        return;
    }

    ret = luaL_dostring(ctx->lstate, "t = require(\"test_lsocket\")\n");
    if (ret != 0) {
        *retval = ina_err_peek();
        return;
    }
    
    if (!INA_SUCCEED(ina_ljit_call(ctx, "t.echo_server", "si<i", addr, port))) {
        *retval = ina_err_peek();
        return;
    }
   
    if (!INA_SUCCEED(ina_ljit_destroy(&ctx))) {
        *retval = ina_err_peek();
        return;
    }
}

/*
 * Lua Debug Server
 */
INA_TEST_HELPER(ljit, lua_debug_server) {

    ina_ljit_ctx_t *ctx = NULL;
    int ret;

    if (!INA_SUCCEED(ina_ljit_init(&ctx))) {
        *retval = ina_err_peek();
        return;
    }

    ret = luaL_dostring(ctx->lstate, "t = require(\"test_ldebug\")\n");
    if (ret != 0) {
        *retval = ina_err_peek();
        return;
    }
    
    if (!INA_SUCCEED(ina_ljit_call(ctx, "t.debug_server", "<"))) {
        *retval = ina_err_peek();
        return;
    }
   
    if (!INA_SUCCEED(ina_ljit_destroy(&ctx))) {
        *retval = ina_err_peek();
        return;
    }
}
