/*
 * Copyright INAOS GmbH, Thalwil, 2013-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>

INA_TEST(cron, id)
{
    ina_cron_ctx_t *ctx;
    ina_cron_event_t *e;
    INA_TEST_ASSERT_SUCCEED(ina_cron_ctx_new(NULL, NULL, &ctx));
    INA_TEST_ASSERT_SUCCEED(ina_cron_event_new(ctx, "t1", "0 * * * *", 0, &e));
    INA_TEST_ASSERT_EQUAL_STR("t1", ina_cron_event_id(e));

    ina_cron_ctx_free(&ctx);
}


INA_TEST(cron, pattern)
{
    ina_cron_ctx_t *ctx;
    ina_cron_event_t *e;
    INA_TEST_ASSERT_SUCCEED(ina_cron_ctx_new(NULL, NULL, &ctx));
    INA_TEST_ASSERT_SUCCEED(ina_cron_event_new(ctx, "t1", "0 * * * *", 0, &e));
    INA_TEST_ASSERT_EQUAL_STR("0 * * * *", ina_cron_event_pattern(e));

    ina_cron_ctx_free(&ctx);
}

INA_TEST(cron, parse_pattern)
{
    const char* pattern0 = "* * * * *";
    const char* pattern1 = "0 * * * *";
    const char* pattern2 = "0 0 * * *";
    const char* pattern3 = "0 0 0 * *";
    const char* pattern4 = "0 0 0 0 *";
    const char* pattern5 = "0 0 0 0 0";
    ina_cron_timetable_t tt;
    INA_MEM_SET_ZERO(&tt, ina_cron_timetable_t);

    INA_TEST_ASSERT_SUCCEED(ina_cron_parse_pattern(pattern0, &tt));
    INA_TEST_ASSERT_SUCCEED(ina_cron_parse_pattern(pattern1, &tt));
    INA_TEST_ASSERT_SUCCEED(ina_cron_parse_pattern(pattern2, &tt));
    INA_TEST_ASSERT_SUCCEED(ina_cron_parse_pattern(pattern3, &tt));
    INA_TEST_ASSERT_SUCCEED(ina_cron_parse_pattern(pattern4, &tt));
    INA_TEST_ASSERT_SUCCEED(ina_cron_parse_pattern(pattern5, &tt));

}

INA_TEST_SKIP(cron, add_tasks_non_persistent_and_utils)
{
    ina_cron_ctx_t *ctx;
    ina_str_t cmd, wd;
    ina_cron_event_iter_t *itr;
    ina_cron_event_t *e;
    int found = 0;

    INA_TEST_ASSERT_SUCCEED(ina_cron_ctx_new(NULL, NULL, &ctx));

#ifdef INA_OS_WINDOWS
    cmd = ina_str_new_fromcstr("dir.exe .");
    wd = ina_str_new_fromcstr("c:\\windows");
#else
    cmd = ina_str_new_fromcstr("ls");
    wd = ina_str_new_fromcstr("./");
#endif
    INA_TEST_ASSERT_SUCCEED(ina_cron_event_new(ctx, "t1", "0 * * * *", 0, &e));
    INA_TEST_ASSERT_SUCCEED(ina_cron_event_set_exec_params(e, cmd, wd));
    ina_str_free(cmd);

#ifdef INA_OS_WINDOWS
    cmd = ina_str_new_fromcstr("pwd.exe .");
#else
    cmd = ina_str_new_fromcstr("pwd");
#endif
    INA_TEST_ASSERT_SUCCEED(ina_cron_event_new(ctx, "t2", "0 23 * * *", 0, &e));
    INA_TEST_ASSERT_SUCCEED(ina_cron_event_set_exec_params(e, cmd, wd));
    ina_str_free(cmd);

#ifdef INA_OS_WINDOWS
    cmd = ina_str_new_fromcstr("mkdir.exe .");
#else
    cmd = ina_str_new_fromcstr("mkdir");
#endif
    INA_TEST_ASSERT_SUCCEED(ina_cron_event_new(ctx, "t3", "0 23 * * *", 0, &e));
    INA_TEST_ASSERT_SUCCEED(ina_cron_event_set_exec_params(e, cmd, wd));
    ina_str_free(cmd);
    ina_str_free(wd);

    e = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_cron_event_by_id(ctx, "t2", &e));
    INA_TEST_ASSERT_NOT_NULL(e);

    ina_cron_event_free(&e);

    INA_TEST_ASSERT_SUCCEED(ina_cron_event_iter_new(ctx, &itr));
    while (e != NULL) {
        INA_TEST_ASSERT_FAILED(ina_cron_event_is_running(e));
        INA_TEST_ASSERT_NOT_NULL(ina_cron_event_pattern(e));
        found++;
        INA_TEST_ASSERT_SUCCEED(ina_cron_event_iter_next(itr, &e));
    }
    INA_TEST_ASSERT_EQUAL_INT(2, found);
    ina_cron_event_iter_free(&itr);
    INA_TEST_ASSERT_NULL(itr);

    ina_cron_ctx_free(&ctx);
}

INA_TEST_SKIP(cron, add_task_and_exec)
{
    ina_cron_ctx_t *ctx;
    ina_str_t cmd, wd;
    int suggested_sleep_time;
    time_t now;
    ina_cron_event_t *e;

    INA_TEST_ASSERT_SUCCEED(ina_cron_ctx_new(NULL, NULL, &ctx));

#ifdef INA_OS_WINDOWS
    cmd = ina_str_new_fromcstr("pwd.exe");
    wd = ina_str_new_fromcstr("c:\\windows");
#else
    cmd = ina_str_new_fromcstr("pwd");
    wd = ina_str_new_fromcstr("./");
#endif
    INA_TEST_ASSERT_SUCCEED(ina_cron_event_new(ctx, "pwd", "* * * * *", 0, &e));
    INA_TEST_ASSERT_SUCCEED(ina_cron_event_set_exec_params(e, cmd, wd));

    ina_str_free(cmd);
    ina_str_free(wd);

    now = time(NULL);
    INA_TEST_ASSERT_SUCCEED(ina_cron_process(ctx, now, &suggested_sleep_time));
    now += suggested_sleep_time;
    INA_TEST_ASSERT_SUCCEED(ina_cron_process(ctx, now, &suggested_sleep_time));

    ina_cron_ctx_free(&ctx);
}