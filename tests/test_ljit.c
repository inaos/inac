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

void test_ljit_call()
{
    ina_ljit_ctx_t *ctx = NULL;
    double r = 0;
    char *rs = NULL;

    INA_TRACE_MSG("test_ljit_call");

    INA_ASSERT_SUCCEED(ina_ljit_init(&ctx));
    INA_ASSERT_NOTNULL(ctx);
    INA_ASSERT_NOTNULL(ctx->lstate);

    INA_ASSERT_EQUAL(0, luaL_dostring(ctx->lstate, "local t = require(\"test_ljit\")\n"));
    lua_getglobal(ctx->lstate, "t");
    
    INA_ASSERT_SUCCEED(ina_ljit_call(ctx, "test_params", "dd<d", 10, 5, &r));
    INA_ASSERT_EQUAL(50, r);
    
    INA_ASSERT_SUCCEED(ina_ljit_call(ctx, "test_params", "<s", &rs));
    INA_ASSERT_EQUAL(0, strcmp(ina_appname(), rs));

    INA_ASSERT_SUCCEED(ina_ljit_destroy(&ctx));
    INA_ASSERT_NULL(ctx);
}

void test_ljit_luaL_dostring()
{
    ina_ljit_ctx_t *ctx = NULL;

    INA_TRACE_MSG("test_ljit_luaL_dostring");

    INA_ASSERT_SUCCEED(ina_ljit_init(&ctx));
    INA_ASSERT_NOTNULL(ctx);
    INA_ASSERT_NOTNULL(ctx->lstate);

    INA_ASSERT_EQUAL(0, luaL_dostring(ctx->lstate, "return 100\n"));
    INA_ASSERT_TRUE(lua_isnumber(ctx->lstate, -1));
    INA_ASSERT_EQUAL(100, (int)lua_tonumber(ctx->lstate, -1));
    lua_pop(ctx->lstate, 1);

    INA_ASSERT_EQUAL(0, luaL_dostring(ctx->lstate, "local t = require(\"test_ljit\")\n return t.test_appname()\n"));
    INA_ASSERT_TRUE(lua_isstring(ctx->lstate, -1));
    INA_ASSERT_EQUAL(0, strcmp(ina_appname(), (const char *)lua_tostring(ctx->lstate, -1)));
    lua_pop(ctx->lstate, 1);
    
    lua_pushnumber(ctx->lstate, 5);
    lua_setglobal(ctx->lstate, "d1");
    lua_pushnumber(ctx->lstate, 10);
    lua_setglobal(ctx->lstate, "d2");
    INA_ASSERT_EQUAL(0, luaL_dostring(ctx->lstate, "local t = require(\"test_ljit\")\n return t.test_params(d1, d2)\n"));
    INA_ASSERT_TRUE(lua_isnumber(ctx->lstate, -1));
    INA_ASSERT_EQUAL(50, (int)lua_tonumber(ctx->lstate, -1));
    lua_pop(ctx->lstate, 1);

    INA_ASSERT_SUCCEED(ina_ljit_destroy(&ctx));
    INA_ASSERT_NULL(ctx);
}

void test_ljit_init_destroy()
{
    ina_ljit_ctx_t *ctx = NULL;
    
    INA_TRACE_MSG("test_ljit_init_destroy");

    INA_ASSERT_SUCCEED(ina_ljit_init(&ctx));
    INA_ASSERT_NOTNULL(ctx);
    INA_ASSERT_NOTNULL(ctx->lstate);
    INA_ASSERT_SUCCEED(ina_ljit_destroy(&ctx));
    INA_ASSERT_NULL(ctx);
    INA_ASSERT_SUCCEED(ina_ljit_destroy(&ctx));
}

void test_ljit_open_close_state_native() 
{
 
    lua_State *lstate = luaL_newstate();
    INA_TRACE_MSG("test_ljit_open_close_state_native");

    INA_ASSERT_NOTNULL(lstate);
    luaL_openlibs(lstate);
    lua_close(lstate);
}

