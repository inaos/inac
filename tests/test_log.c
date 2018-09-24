/*
 * Copyright INAOS GmbH, Thalwil, 2012-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>

INA_TEST(log, open_close_console)
{
    ina_log_t *log = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_log_init("test_log.conf"));
    INA_TEST_ASSERT_SUCCEED(ina_log_new("test", &log));
    INA_TEST_ASSERT_NOT_NULL(log);
    INA_TEST_ASSERT_SUCCEED(ina_log(log, INA_LOG_LEVEL_DEBUG, "Test DEBUG log entry, var=%d", 2));
    INA_TEST_ASSERT_SUCCEED(ina_log(log, INA_LOG_LEVEL_INFO, "Test INFO log entry, var=%d", 2));
    INA_TEST_ASSERT_SUCCEED(ina_log(log, INA_LOG_LEVEL_WARNING, "Test WARNING entry, var=%d", 2));
    INA_TEST_ASSERT_SUCCEED(ina_log(log, INA_LOG_LEVEL_ERROR, "Test ERROR entry, var=%d", 2));
    ina_log_free(&log);
    INA_TEST_ASSERT_NULL(log);
}
