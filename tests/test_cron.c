/*
 * Copyright INAOS GmbH, Thalwil, 2013-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>

INA_TEST_DATA(cron) {};
INA_TEST(cron, id)
{
    ina_cron_ctx_t *ctx;
    ina_cron_event_t *e;
    INA_UNUSED(data);

    INA_TEST_ASSERT_SUCCEED(ina_cron_ctx_new(NULL, NULL, &ctx));
    INA_TEST_ASSERT_SUCCEED(ina_cron_event_new(ctx, "t1", "0 * * * *", 0, &e));
    INA_TEST_ASSERT_EQUAL_STR("t1", ina_cron_event_id(e));

    ina_cron_ctx_free(&ctx);
}


INA_TEST(cron, pattern)
{
    ina_cron_ctx_t *ctx;
    ina_cron_event_t *e;
    INA_UNUSED(data);

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
    INA_UNUSED(data);

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
    INA_UNUSED(data);

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
    INA_UNUSED(data);

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

INA_TEST(cron, invalid_arguments)
{
    int fake = 0;
    ina_cron_ctx_t *ctx = NULL;
    INA_DISABLE_WARNING(int-to-pointer-cast, int-to-pointer-cast,4312)
    ina_cron_event_t *event = (ina_cron_event_t*)fake;
    INA_ENABLE_WARNING(int-to-pointer-cast, int-to-pointer-cast,4312)
    ina_str_t cmd = NULL;
    ina_str_t working_dir = NULL;
    ina_cron_push_cb_t push_cb = NULL;
    uint32_t  key;
    ina_cron_event_iter_t *iter = NULL;
    ina_cron_timetable_t tt;
    time_t last_exec_time;
    INA_UNUSED(data);

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cron_ctx_new(NULL, NULL, NULL));
    INA_TEST_ASSERT_SUCCEED(ina_cron_ctx_new(NULL, NULL, &ctx));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cron_event_new(NULL, "id", "pattern", 0, &event));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cron_event_new(ctx, NULL, "pattern", 0, &event));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cron_event_new(ctx, "", "pattern", 0, &event));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cron_event_new(ctx, "id", "pattern", 0, NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cron_event_set_exec_params(NULL, "cmd", "working_dir"));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cron_event_set_exec_params(event, NULL, "working_dir"));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cron_event_get_exec_params(NULL, &cmd, &working_dir));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cron_event_get_exec_params(event, NULL, &working_dir));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cron_event_get_exec_params(event, &cmd, NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cron_event_set_push_params(NULL, NULL, NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cron_event_get_push_params(NULL, &push_cb, NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cron_event_get_push_params(event, NULL, NULL));


    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cron_event_get_pull_params(NULL, &key, NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cron_event_get_pull_params(event, NULL, NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cron_event_set_pull_params(NULL, key, NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cron_event_set_pull_params(event, key, NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cron_process(NULL, 0, &fake));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cron_process(ctx, 0, NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cron_event_iter_new(NULL, &iter));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cron_event_iter_new(ctx, NULL));

    INA_TEST_ASSERT_SUCCEED(ina_cron_event_iter_new(ctx, &iter));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cron_event_iter_next(NULL, &event));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cron_event_iter_next(iter, NULL));
    ina_cron_event_iter_free(&iter);

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cron_event_by_id(NULL, "id", &event));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cron_event_by_id(ctx, NULL, &event));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cron_event_by_id(ctx, "id", NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cron_event_is_running(NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cron_last_exec_systime(NULL, &tt, 0, &last_exec_time));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cron_last_exec_systime(ctx, NULL, 0, &last_exec_time));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cron_last_exec_systime(ctx, &tt, 0, NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cron_try_pull(NULL, &key, NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cron_try_pull(ctx, NULL, NULL));


}