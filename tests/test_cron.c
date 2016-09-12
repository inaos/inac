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


INA_TEST(cron, add_tasks_non_persistent_and_utils)
{
    ina_cron_ctx_t *ctx;
    ina_str_t cmd, wd;
    ina_cron_task_itr_t *itr;
    ina_cron_task_t *task;
    int found = 0;

    INA_TEST_ASSERT_SUCCEED(ina_cron_init(&ctx, NULL, NULL));

    cmd = ina_str_new_fromcstr("dir.exe .");
    wd = ina_str_new_fromcstr("c:\\windows");
    INA_TEST_ASSERT_SUCCEED(ina_cron_task_add(ctx, "dir", "0 * * * *", 0, "dir.exe .", "c:\\windows"));
    ina_str_free(cmd);

    cmd = ina_str_new_fromcstr("pwd.exe .");
    INA_TEST_ASSERT_SUCCEED(ina_cron_task_add(ctx, "pwd", "0 23 * * *", 0, cmd, wd));
    ina_str_free(cmd);

    cmd = ina_str_new_fromcstr("mkdir.exe .");
    INA_TEST_ASSERT_SUCCEED(ina_cron_task_add(ctx, "mkdir", "0 23 * * *", 0, cmd, wd));
    ina_str_free(cmd);
    ina_str_free(wd);

    INA_TEST_ASSERT_SUCCEED(ina_cron_task_by_id(ctx, "pwd", &task));
    INA_TEST_ASSERT_NOT_NULL(task);

    INA_TEST_ASSERT_SUCCEED(ina_cron_task_remove(ctx, &task));

    INA_TEST_ASSERT_SUCCEED(ina_cron_task_new_iter(ctx, &itr));
    while (task != NULL) {
        int running = 0;
        ina_str_t patt;
        INA_TEST_ASSERT_NOTSUCCEED(ina_cron_task_is_running(task));
        INA_TEST_ASSERT_EQUAL_INTEGER(0, running);
        INA_TEST_ASSERT_SUCCEED(ina_cron_task_get_pattern(task, &patt));
        INA_TEST_ASSERT_NOT_NULL(patt);
        found++;
        INA_TEST_ASSERT_SUCCEED(ina_cron_task_next(itr, &task));
    }
    INA_TEST_ASSERT_EQUAL_INTEGER(2, found);
    INA_TEST_ASSERT_SUCCEED(ina_cron_task_free_iter(&itr));

    INA_TEST_ASSERT_SUCCEED(ina_cron_destroy(&ctx));
}

INA_TEST(cron, add_task_and_exec)
{
    ina_cron_ctx_t *ctx;
    ina_str_t cmd, wd;
    int suggested_sleep_time;
    time_t now;

    INA_TEST_ASSERT_SUCCEED(ina_cron_init(&ctx, NULL, NULL));

    cmd = ina_str_new_fromcstr("pwd.exe");
    wd = ina_str_new_fromcstr("c:\\windows");
    INA_TEST_ASSERT_SUCCEED(ina_cron_task_add(ctx, "pwd", "* * * * *", 0, cmd, wd));
    ina_str_free(cmd);
    ina_str_free(wd);

    now = time(NULL);
    INA_TEST_ASSERT_SUCCEED(ina_cron_process(ctx, now, &suggested_sleep_time));
    now += suggested_sleep_time;
    INA_TEST_ASSERT_SUCCEED(ina_cron_process(ctx, now, &suggested_sleep_time));

    INA_TEST_ASSERT_SUCCEED(ina_cron_destroy(&ctx));
}