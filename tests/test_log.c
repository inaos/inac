/*
 * Copyright INAOS GmbH, Thalwil, 2012-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>

INA_TEST(log, default_context)
{
    INA_UNUSED(data);

    INA_TEST_ASSERT_SUCCEED(ina_log_init_from_file("test_log.conf"));
    INA_TEST_ASSERT_SUCCEED(ina_log_write(NULL, INA_LOG_LEVEL_DEBUG, INA_AT, "Test DEBUG log entry, var=%d", 2));
    INA_TEST_ASSERT_SUCCEED(ina_log_write(NULL, INA_LOG_LEVEL_INFO, INA_AT, "Test INFO log entry, var=%d", 2));
    INA_TEST_ASSERT_SUCCEED(ina_log_write(NULL, INA_LOG_LEVEL_WARNING, INA_AT, "Test WARNING entry, var=%d", 2));
    INA_TEST_ASSERT_SUCCEED(ina_log_write(NULL, INA_LOG_LEVEL_ERROR, INA_AT,"Test ERROR entry, var=%d", 2));
}

INA_TEST(log, open_close_console)
{
    ina_log_ctx_t *ctx = NULL;
    INA_UNUSED(data);

    INA_TEST_ASSERT_SUCCEED(ina_log_ctx_new("test", &ctx));
    INA_TEST_ASSERT_NOT_NULL(ctx);
    INA_TEST_ASSERT_SUCCEED(ina_log_write(ctx, INA_LOG_LEVEL_DEBUG, INA_AT, "Test DEBUG log entry, var=%d", 2));
    INA_TEST_ASSERT_SUCCEED(ina_log_write(ctx, INA_LOG_LEVEL_INFO, INA_AT, "Test INFO log entry, var=%d", 2));
    INA_TEST_ASSERT_SUCCEED(ina_log_write(ctx, INA_LOG_LEVEL_WARNING, INA_AT, "Test WARNING entry, var=%d", 2));
    INA_TEST_ASSERT_SUCCEED(ina_log_write(ctx, INA_LOG_LEVEL_ERROR, INA_AT,"Test ERROR entry, var=%d", 2));
    ina_log_ctx_free(&ctx);
    INA_TEST_ASSERT_NULL(ctx);
}

INA_TEST(log, inavalid_arguments)
{
    ina_log_ctx_t *ctx = NULL;
    INA_UNUSED(data);

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_log_ctx_new(NULL, &ctx));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_log_ctx_new("cat", NULL));

    INA_TEST_ASSERT_SUCCEED(ina_log_init_from_file("test_log.conf"));
    INA_TEST_ASSERT_SUCCEED(ina_log_ctx_new("test.debug", &ctx));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_log_write(ctx, INA_LOG_LEVEL_DEBUG, NULL, NULL, "s"));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_log_write(ctx, INA_LOG_LEVEL_DEBUG, NULL, "", "s"));

    ina_log_ctx_free(&ctx);
}