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
    ina_log_t *cfg;
    
    cfg = NULL;

    INA_TEST_ASSERT_SUCCEED(ina_log_new("", "", &cfg));
    INA_TEST_ASSERT_NOT_NULL(cfg);
    INA_TEST_ASSERT_SUCCEED(ina_log(cfg, INA_LOG_LEVEL_DEBUG, "Test log entry, var=%d", 2));
    ina_log_free(&cfg);
    INA_TEST_ASSERT_NULL(cfg);
}

#ifndef INA_OS_WIN32
INA_TEST(log, syslog)
{
  	ina_log_t *cfg = NULL;
  
    INA_TEST_ASSERT_SUCCEED(ina_log_new("test", "", &cfg));
    INA_TEST_ASSERT_NOT_NULL(cfg);
    INA_TEST_ASSERT_SUCCEED(ina_log(cfg, INA_LOG_LEVEL_DEBUG, "Test DEBUG log entry, var=%d", 2));
    INA_TEST_ASSERT_SUCCEED(ina_log(cfg, INA_LOG_LEVEL_INFO, "Test INFO log entry, var=%d", 2));
    INA_TEST_ASSERT_SUCCEED(ina_log(cfg, INA_LOG_LEVEL_WARNING, "Test WARNING log entry, var=%d", 2));
    INA_TEST_ASSERT_SUCCEED(ina_log(cfg, INA_LOG_LEVEL_ERROR, "Test ERROR log entry, var=%d", 2));
    ina_log_free(&cfg);
    INA_TEST_ASSERT_NULL(cfg);	
}
#endif