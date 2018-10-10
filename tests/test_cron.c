/*
 * Copyright INAOS GmbH, Thalwil, 2013-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>


INA_TEST_SKIP(cron, add_tasks_non_persistent_and_utils)
{
    ina_cron_ctx_t *ctx;
    ina_str_t cmd, wd;
    ina_cron_task_itr_t *itr;
    ina_cron_task_t *task;
    int found = 0;

    INA_TEST_ASSERT_SUCCEED(ina_cron_ctx_new(&ctx, NULL, NULL, NULL));

#ifdef INA_OS_WIN32
    cmd = ina_str_new_fromcstr("dir.exe .");
    wd = ina_str_new_fromcstr("c:\\windows");
#else
    cmd = ina_str_new_fromcstr("uname");
    wd = ina_str_new_fromcstr("./");
#endif
    INA_TEST_ASSERT_SUCCEED(ina_cron_task_new(ctx, "t1", "0 * * * *", 0, cmd, wd));
    ina_str_free(cmd);

#ifdef INA_OS_WIN32
    cmd = ina_str_new_fromcstr("pwd.exe .");
#else
    cmd = ina_str_new_fromcstr("pwd");
#endif
    INA_TEST_ASSERT_SUCCEED(ina_cron_task_new(ctx, "t2", "0 23 * * *", 0, cmd, wd));
    ina_str_free(cmd);

#ifdef INA_OS_WIN32
    cmd = ina_str_new_fromcstr("mkdir.exe .");
#else
    cmd = ina_str_new_fromcstr("mkdir");
#endif
    INA_TEST_ASSERT_SUCCEED(ina_cron_task_new(ctx, "t3", "0 23 * * *", 0, cmd, wd));
    ina_str_free(cmd);
    ina_str_free(wd);

    task = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_cron_task_by_id(ctx, "t2", &task));
    INA_TEST_ASSERT_NOT_NULL(task);

    ina_cron_task_free(ctx, &task);

    INA_TEST_ASSERT_SUCCEED(ina_cron_task_iter_new(ctx, &itr));
    while (task != NULL) {
        ina_str_t patt;
        INA_TEST_ASSERT_FAILED(ina_cron_task_is_running(task));
        INA_TEST_ASSERT_SUCCEED(ina_cron_task_get_pattern(task, &patt));
        INA_TEST_ASSERT_NOT_NULL(patt);
        found++;
        INA_TEST_ASSERT_SUCCEED(ina_cron_task_next(itr, &task));
    }
    INA_TEST_ASSERT_EQUAL_INT(2, found);
    INA_TEST_ASSERT_SUCCEED(ina_cron_task_iter_free(&itr));

    ina_cron_ctx_free(&ctx);
}

INA_TEST_SKIP(cron, add_task_and_exec)
{
    ina_cron_ctx_t *ctx;
    ina_str_t cmd, wd;
    int suggested_sleep_time;
    time_t now;

    INA_TEST_ASSERT_SUCCEED(ina_cron_ctx_new(&ctx, NULL, NULL, NULL));

#ifdef INA_OS_WIN32
    cmd = ina_str_new_fromcstr("pwd.exe");
    wd = ina_str_new_fromcstr("c:\\windows");
#else
    cmd = ina_str_new_fromcstr("pwd");
    wd = ina_str_new_fromcstr("./");
#endif
    INA_TEST_ASSERT_SUCCEED(ina_cron_task_new(ctx, "pwd", "* * * * *", 0, cmd, wd));
    ina_str_free(cmd);
    ina_str_free(wd);

    now = time(NULL);
    INA_TEST_ASSERT_SUCCEED(ina_cron_process(ctx, now, &suggested_sleep_time));
    now += suggested_sleep_time;
    INA_TEST_ASSERT_SUCCEED(ina_cron_process(ctx, now, &suggested_sleep_time));

    ina_cron_ctx_free(&ctx);
}