/*
 * Copyright INAOS GmbH, Thalwil, 2013-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>

#define __INA_TCP_ADDR "127.0.0.1"
#define __INA_TCP_PORT  8033

INA_TEST_DATA(ljit_ex) {
    ina_test_hid_t echo_hid;
    ina_test_hid_t debug_hid;
};

INA_TEST_SETUP(ljit_ex) {
    INA_TEST_HELPER_INVOKE(&data->echo_hid, ljit, lua_echo_server, 
        __INA_TCP_ADDR, 
         INA_NUM2STR(__INA_TCP_PORT),
         NULL);
    INA_TEST_HELPER_INVOKE(&data->debug_hid, ljit, lua_debug_server, NULL);
}

INA_TEST_TEARDOWN(ljit_ex) {
    INA_TEST_HELPER_TERMINATE(&data->echo_hid);
    INA_TEST_HELPER_TERMINATE(&data->debug_hid);
}

INA_TEST_FIXTURE_SKIP(ljit_ex, lsocket_echo_client)
{
    ina_ljit_ctx_t *ctx = NULL;
    int r = 0;
    INA_UNUSED(data);
    INA_TEST_ASSERT_SUCCEED(ina_ljit_ctx_new(&ctx));
    INA_TEST_ASSERT_NOT_NULL(ctx);
    INA_TEST_ASSERT_NOT_NULL(ctx->lstate);

    INA_TEST_ASSERT_EQUAL_INT(0, luaL_dostring(ctx->lstate,
                                    "t = require(\"test_lsocket\")\n"));

    INA_TEST_ASSERT_SUCCEED(ina_ljit_call(ctx, "t.echo_client", "si<i", 
                                            "127.0.0.1", 
                                            8033, 
                                            &r));

    ina_ljit_ctx_free(&ctx);
    INA_TEST_ASSERT_NULL(ctx);
}

INA_TEST_FIXTURE_SKIP(ljit_ex, debug)
{
    ina_ljit_ctx_t *ctx = NULL;
    INA_UNUSED(data);

    INA_TEST_ASSERT_SUCCEED(ina_ljit_ctx_new(&ctx));
    INA_TEST_ASSERT_NOT_NULL(ctx);
    INA_TEST_ASSERT_NOT_NULL(ctx->lstate);

    INA_TEST_ASSERT_EQUAL_INT(0, luaL_dostring(ctx->lstate,
                                    "t = require(\"test_ldebug\")\n"));

    INA_TEST_ASSERT_SUCCEED(ina_ljit_call(ctx, "t.debug_client", "<"));

    ina_ljit_ctx_free(&ctx);
    INA_TEST_ASSERT_NULL(ctx);
}

INA_TEST(ljit, call)
{
    ina_ljit_ctx_t *ctx = NULL;
    double r = 0;
    char *rs = NULL;

    ina_err_reset();

    INA_TEST_ASSERT_SUCCEED(ina_ljit_ctx_new(&ctx));
    INA_TEST_ASSERT_NOT_NULL(ctx);
    INA_TEST_ASSERT_NOT_NULL(ctx->lstate);

    INA_TEST_ASSERT_EQUAL_INT(0, luaL_dostring(ctx->lstate,
                                    "x = require(\"test_ljit\")\n"));

    INA_TEST_ASSERT_SUCCEED(ina_ljit_call(ctx, "x.test_params", "dd<d", 
                                            (double)10, 
                                            (double)5, 
                                            &r));
    INA_TEST_ASSERT_EQUAL_FLOATING(50, r);
    
    INA_TEST_ASSERT_SUCCEED(ina_ljit_call(ctx, "x.test_app_get_name", "<s", &rs));
    INA_TEST_ASSERT_EQUAL_STR(ina_app_get_name(), rs);
    INA_TEST_ASSERT_SAME(ina_app_get_name(), rs);

    ina_ljit_ctx_free(&ctx);
    ina_ljit_ctx_free(&ctx);
    INA_TEST_ASSERT_NULL(ctx);
    INA_TEST_ASSERT_NULL(ctx);
}

INA_TEST(ljit, luaL_dostring)
{
    ina_ljit_ctx_t *ctx = NULL;

    INA_TEST_ASSERT_SUCCEED(ina_ljit_ctx_new(&ctx));
    INA_TEST_ASSERT_NOT_NULL(ctx);
    INA_TEST_ASSERT_NOT_NULL(ctx->lstate);

    INA_TEST_ASSERT_EQUAL_INT(0, luaL_dostring(ctx->lstate, "return 100\n"));
    INA_TEST_ASSERT_TRUE(lua_isnumber(ctx->lstate, -1));
    INA_TEST_ASSERT_EQUAL_INT(100, (int)lua_tonumber(ctx->lstate, -1));
    lua_pop(ctx->lstate, 1);

    INA_TEST_ASSERT_EQUAL_INT64(0, ina_ljit_dostring(ctx, "local t = require(\"test_ljit\")\n return t.test()\n"));
    INA_TEST_ASSERT_TRUE(lua_isnumber(ctx->lstate, -1));
    INA_TEST_ASSERT_EQUAL_INT(99, (int)lua_tonumber(ctx->lstate, -1));
    lua_pop(ctx->lstate, 1);

    INA_TEST_ASSERT_EQUAL_INT(0, luaL_dostring(ctx->lstate, "local t = require(\"test_ljit\")\n return t.test_app_get_name()\n"));
    /*INA_TEST_ASSERT_TRUE(lua_isstring(ctx->lstate, -1));*/
    INA_TEST_ASSERT_EQUAL_STR(ina_app_get_name(), *(const char **)lua_topointer(ctx->lstate, -1));
    lua_pop(ctx->lstate, 1);
    
    lua_pushnumber(ctx->lstate, 5);
    lua_setglobal(ctx->lstate, "d1");
    lua_pushnumber(ctx->lstate, 10);
    lua_setglobal(ctx->lstate, "d2");
    INA_TEST_ASSERT_EQUAL_INT(0, luaL_dostring(ctx->lstate, "local t = require(\"test_ljit\")\n return t.test_params(d1, d2)\n"));
    INA_TEST_ASSERT_TRUE(lua_isnumber(ctx->lstate, -1));
    INA_TEST_ASSERT_EQUAL_INT(50, (int)lua_tonumber(ctx->lstate, -1));
    lua_pop(ctx->lstate, 1);

    ina_ljit_ctx_free(&ctx);
    INA_TEST_ASSERT_NULL(ctx);
}

INA_TEST(ljit, init_destroy)
{
    ina_ljit_ctx_t *ctx = NULL;
    
    INA_TEST_ASSERT_SUCCEED(ina_ljit_ctx_new(&ctx));
    INA_TEST_ASSERT_NOT_NULL(ctx);
    INA_TEST_ASSERT_NOT_NULL(ctx->lstate);
    ina_ljit_ctx_free(&ctx);
    INA_TEST_ASSERT_NULL(ctx);
}

INA_TEST(ljit, open_close_state_native)
{
    lua_State *lstate = luaL_newstate();

    INA_TEST_ASSERT_NOT_NULL(lstate);
    luaL_openlibs(lstate);
    lua_close(lstate);
}

INA_TEST(ljit, invalid_arguments)
{
    ina_ljit_ctx_t *ctx = NULL;

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ljit_ctx_new(NULL));

    INA_TEST_ASSERT_SUCCEED(ina_ljit_ctx_new(&ctx));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ljit_call(NULL, "test", ">"));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ljit_call(ctx, NULL, ">"));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ljit_call(ctx, "test", NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ljit_dostring(NULL, "print()"));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ljit_dostring(ctx, NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ljit_dump_stack(NULL));

    ina_ljit_ctx_free(&ctx);
}

