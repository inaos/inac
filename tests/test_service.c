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


INA_TEST(service, init_destroy)
{
    ina_service_ctx_t *ctx = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_service_init(&ctx));
    INA_TEST_ASSERT_NOT_NULL(ctx);
    INA_TEST_ASSERT_SUCCEED(ina_service_destroy(&ctx));
    INA_TEST_ASSERT_NULL(ctx);
}


INA_TEST(service, get_descriptor)
{
    int user_data = 0;
    ina_service_ctx_t *ctx = NULL;
    ina_service_descriptor_t *ds = NULL;
  
    INA_TEST_ASSERT_SUCCEED(ina_service_init(&ctx));
    INA_TEST_ASSERT_NOT_NULL(ctx);
    INA_TEST_ASSERT_SUCCEED(ina_service_get_descriptor(ctx, &ds));
    INA_TEST_ASSERT_NOT_NULL(ds);

    INA_TEST_ASSERT_EQUAL_STR("test", ds->name);
    INA_TEST_ASSERT_EQUAL_STR("Test Daemon", ds->display_name);
    INA_TEST_ASSERT_EQUAL_STR("Simple test deamon", ds->description);
    INA_TEST_ASSERT_EQUAL_STR("root", ds->username); 
    INA_TEST_ASSERT_EQUAL_STR("password", ds->password);
    INA_TEST_ASSERT_EQUAL_STR("-h service simple_deamon 127.0.0.1 9998",ds->startup_args); 
    INA_TEST_ASSERT_EQUAL_STR("/opt/test", ds->working_directory);
    INA_TEST_ASSERT_EQUAL_STR("1234 99 10", ds->chkconfig);
    INA_TEST_ASSERT_NOT_NULL(ds->service_fn);
    INA_TEST_ASSERT_EQUAL_INTEGER(INA_SERVICE_STARTUP_TYPE_AUTO, ds->startup);
    INA_TEST_ASSERT_EQUAL_INTEGER(INA_YES, ds->exclusive_flag);

    INA_TEST_ASSERT_SUCCEED(ds->service_fn(ctx, INA_SERVICE_STATUS_INIT, &user_data));
 
    INA_TEST_ASSERT_SUCCEED(ds->service_fn(ctx, INA_SERVICE_STATUS_START, &user_data));
    INA_TEST_ASSERT_EQUAL_INTEGER(1, user_data);

    INA_TEST_ASSERT_SUCCEED(ds->service_fn(ctx, INA_SERVICE_STATUS_RUN, &user_data));
    INA_TEST_ASSERT_EQUAL_INTEGER(2, user_data);

    INA_TEST_ASSERT_SUCCEED(ds->service_fn(ctx, INA_SERVICE_STATUS_SHUTDOWN, &user_data));
    INA_TEST_ASSERT_EQUAL_INTEGER(3, user_data);

    INA_TEST_ASSERT_SUCCEED(ds->service_fn(ctx, INA_SERVICE_STATUS_STOP, &user_data));

    INA_TEST_ASSERT_SUCCEED(ina_service_destroy(&ctx));
    INA_TEST_ASSERT_NULL(ctx);
}


#ifndef INA_OS_WIN32
INA_TEST(service, install_uninstall)
{
    ina_service_ctx_t *ctx = NULL;
 
    INA_TEST_ASSERT_SUCCEED(ina_service_init(&ctx));
    INA_TEST_ASSERT_SUCCEED(ina_service_install(ctx));
    INA_TEST_ASSERT_SUCCEED(ina_service_uninstall(ctx));
    INA_TEST_ASSERT_SUCCEED(ina_service_destroy(&ctx));

}
#endif

INA_TEST(service, run)
{
    /*ina_service_ctx_t *ctx;
    ina_service_descriptor_t sd;*/
}
