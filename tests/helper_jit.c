/*
 * Copyright INAOS GmbH, Thalwil, 2014-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
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

    if (!INA_SUCCEED(ina_ljit_ctx_new(&ctx))) {
        *retval = INA_RC_ERROR(ina_err_get_rc());
        return;
    }

    ret = luaL_dostring(ctx->lstate, "t = require(\"test_lsocket\")\n");
    if (ret != 0) {
        *retval = INA_RC_ERRNO(ina_err_get_rc());
        return;
    }
    
    if (!INA_SUCCEED(ina_ljit_call(ctx, "t.echo_server", "si<i", addr, port))) {
        *retval = INA_RC_ERROR(ina_err_get_rc());
        return;
    }
   
	ina_ljit_ctx_free(&ctx);
}

/*
 * Lua Debug Server
 */
INA_TEST_HELPER(ljit, lua_debug_server) {

    ina_ljit_ctx_t *ctx = NULL;
    int ret;
    INA_UNUSED(argv);
    INA_UNUSED(argc);

    if (!INA_SUCCEED(ina_ljit_ctx_new(&ctx))) {
        *retval = INA_RC_ERROR(ina_err_get_rc());
        return;
    }

    ret = luaL_dostring(ctx->lstate, "t = require(\"test_ldebug\")\n");
    if (ret != 0) {
        *retval = INA_RC_ERRNO(ina_err_get_rc());
        return;
    }
    
    if (!INA_SUCCEED(ina_ljit_call(ctx, "t.debug_server", "<"))) {
        *retval = INA_RC_ERROR(ina_err_get_rc());
        return;
    }
   
	ina_ljit_ctx_free(&ctx);
}
