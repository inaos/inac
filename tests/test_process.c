/*
 * Copyright (c) 2013-2018 INAOS GmbH
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

#ifndef INA_OS_WIN32
#define __INA_TEST_EXE "./test"
#else
#define __INA_TEST_EXE "test.exe"
#endif

INA_TEST(process, init_destroy)
{   
    ina_process_ctx_t *ctx;    
    INA_TEST_ASSERT_SUCCEED(ina_process_init(&ctx));
    INA_TEST_ASSERT_NOT_NULL(ctx);
    INA_TEST_ASSERT_SUCCEED(ina_process_destroy(&ctx));
    INA_TEST_ASSERT_NULL(ctx);
}

INA_TEST(process, manage)
{
    ina_process_ctx_t *ctx = NULL;
    ina_process_descriptor_t *pd = NULL;
    ina_process_t *p = NULL;

    INA_TEST_ASSERT_SUCCEED(ina_process_init(&ctx));
    INA_TEST_ASSERT_NOT_NULL(ctx);
    INA_TEST_ASSERT_SUCCEED(ina_process_descriptor_new(ctx, &pd,
            __INA_TEST_EXE,
            NULL,
            "-h process spawn_and_wait 0",
            INA_PROCESS_LIFECYCLE_TYPE_MANAGED,
            INA_PROCESS_MANAGED_TYPE_PARENT_LIFETIME,
            NULL,
            NULL,
            100,
            0));
    INA_TEST_ASSERT_SUCCEED(ina_process_new(ctx, pd, &p));
    INA_TEST_ASSERT_NOT_NULL(p);
    INA_TEST_ASSERT_SUCCEED(ina_process_manage(ctx));
    INA_TEST_ASSERT_SUCCEED(ina_process_destroy(&ctx));
    INA_TEST_ASSERT_NULL(ctx);
}

INA_TEST(process, descriptor_new_free)
{
    ina_process_ctx_t *ctx;
    ina_process_descriptor_t *pd;
   
    INA_TEST_ASSERT_SUCCEED(ina_process_init(&ctx));
    INA_TEST_ASSERT_NOT_NULL(ctx);

    INA_TEST_ASSERT_SUCCEED(ina_process_descriptor_new(ctx, &pd,
        "full_path",
        "working_dir",
        "1 2 3 4",
        INA_PROCESS_LIFECYCLE_TYPE_FIRE_AND_FORGET,
        INA_PROCESS_MANAGED_TYPE_SCHEDULED_START,
        "scheduled_start_pattern",
        "scheduled_stop_pattern",
        100,
        0));
    INA_TEST_ASSERT_NOT_NULL(pd);
    INA_TEST_ASSERT_EQUAL_STR("full_path", pd->full_path);
    INA_TEST_ASSERT_EQUAL_STR("working_dir", pd->working_dir);
    INA_TEST_ASSERT_EQUAL_STR("scheduled_stop_pattern", pd->scheduled_stop_pattern);
    INA_TEST_ASSERT_EQUAL_STR("scheduled_start_pattern", pd->scheduled_start_pattern);
    INA_TEST_ASSERT_EQUAL_INTEGER(INA_PROCESS_LIFECYCLE_TYPE_FIRE_AND_FORGET, pd->lifecycle);
    INA_TEST_ASSERT_EQUAL_INTEGER(INA_PROCESS_MANAGED_TYPE_SCHEDULED_START, pd->managed_type);
    INA_TEST_ASSERT_EQUAL_INTEGER(100, pd->stop_wait_time_ms);
    INA_TEST_ASSERT_EQUAL_INTEGER(0, pd->start_flags);
    INA_TEST_ASSERT_EQUAL_STR("1 2 3 4", ina_str_cstr(pd->startup_args));
    INA_TEST_ASSERT_SUCCEED(ina_process_descriptor_free(&pd));
    INA_TEST_ASSERT_NULL(pd);
    INA_TEST_ASSERT_SUCCEED(ina_process_destroy(&ctx));
}

INA_TEST(process, new_free)
{
    ina_process_ctx_t *ctx;
    ina_process_descriptor_t pd;
    ina_process_t *process;
   
    INA_TEST_ASSERT_SUCCEED(ina_process_init(&ctx));
    INA_TEST_ASSERT_NOT_NULL(ctx);
    
    ina_mem_set(&pd, 0, sizeof(ina_process_descriptor_t));
    pd.full_path = ina_str_new_fromcstr(__INA_TEST_EXE);
    pd.working_dir = ina_str_new_fromcstr("./");
    pd.startup_args = ina_str_new_fromcstr("-h process");
    pd.lifecycle = INA_PROCESS_LIFECYCLE_TYPE_FIRE_AND_FORGET;
    pd.managed_type = 0;
    pd.scheduled_start_pattern = ina_str_new_fromcstr("");
    pd.scheduled_stop_pattern = ina_str_new_fromcstr("");
    pd.stop_wait_time_ms = 100;
    pd.start_flags = 0;

    INA_TEST_ASSERT_SUCCEED(ina_process_new(ctx, &pd, &process));
    INA_TEST_ASSERT_NOT_NULL(process);
    INA_TEST_ASSERT_SUCCEED(ina_process_free(&process));
    INA_TEST_ASSERT_NULL(process);
    
    INA_TEST_ASSERT_SUCCEED(ina_process_destroy(&ctx));
    INA_TEST_ASSERT_NULL(ctx);
}

INA_TEST(process, start_and_wait)
{
    ina_process_ctx_t *ctx;
    ina_process_t *process;
    ina_process_descriptor_t pd;
    int  exit_code;
   
    INA_TEST_ASSERT_SUCCEED(ina_process_init(&ctx));
    INA_TEST_ASSERT_NOT_NULL(ctx);

    ina_mem_set(&pd, 0, sizeof(ina_process_descriptor_t));
    pd.full_path = ina_str_new_fromcstr(__INA_TEST_EXE);
    pd.startup_args = ina_str_new_fromcstr("-h process spawn_and_wait 0");
    pd.lifecycle = INA_PROCESS_LIFECYCLE_TYPE_WAIT;
   
    INA_TEST_ASSERT_SUCCEED(ina_process_new(ctx, &pd, &process));
    INA_TEST_ASSERT_NOT_NULL(process);
    INA_TEST_ASSERT_SUCCEED(ina_process_start(process));
    INA_TEST_ASSERT_SUCCEED(ina_process_get_exit_code(process, &exit_code));
    INA_TEST_ASSERT_EQUAL_INTEGER(0, exit_code);
    INA_TEST_ASSERT_SUCCEED(ina_process_free(&process));
    INA_TEST_ASSERT_SUCCEED(ina_process_destroy(&ctx));
}

INA_TEST(process, stop)
{
    ina_process_ctx_t *ctx;
    ina_process_t *process;
    ina_process_descriptor_t pd;
    ina_fsm_state_t state;

    INA_TEST_ASSERT_SUCCEED(ina_process_init(&ctx));
    INA_TEST_ASSERT_NOT_NULL(ctx);

    ina_mem_set(&pd, 0, sizeof(ina_process_descriptor_t));
    pd.full_path = ina_str_new_fromcstr(__INA_TEST_EXE);
    pd.working_dir = NULL;
    pd.startup_args = ina_str_new_fromcstr("-h process spawn_and_wait 0");
    pd.lifecycle = INA_PROCESS_LIFECYCLE_TYPE_FIRE_AND_FORGET;
    pd.managed_type = 0;
    pd.scheduled_start_pattern = ina_str_new_fromcstr("");
    pd.scheduled_stop_pattern = ina_str_new_fromcstr("");
    pd.stop_wait_time_ms = 100;
    pd.start_flags = 0;

    INA_TEST_ASSERT_SUCCEED(ina_process_new(ctx, &pd, &process));
    INA_TEST_ASSERT_NOT_NULL(process);
    INA_TEST_ASSERT_SUCCEED(ina_process_start(process));
    INA_TEST_ASSERT_SUCCEED(ina_process_query_state(process, &state));
    INA_TEST_ASSERT_EQUAL_INTEGER(INA_PROCESS_RUNNING, state);
    INA_TEST_ASSERT_SUCCEED(ina_process_stop(process));
    INA_TEST_ASSERT_SUCCEED(ina_process_query_state(process, &state));
    INA_TEST_ASSERT_EQUAL_INTEGER(INA_PROCESS_STOPPED, state);
    INA_TEST_ASSERT_SUCCEED(ina_process_free(&process));
    INA_TEST_ASSERT_NULL(process);
    INA_TEST_ASSERT_SUCCEED(ina_process_destroy(&ctx));
}

INA_TEST(process, state)
{
    ina_process_ctx_t *ctx;
    ina_process_t *process;
    ina_fsm_state_t state;
    ina_process_descriptor_t pd;
    
    INA_TEST_ASSERT_SUCCEED(ina_process_init(&ctx));
    INA_TEST_ASSERT_NOT_NULL(ctx);

    ina_mem_set(&pd, 0, sizeof(ina_process_descriptor_t));
    pd.full_path = ina_str_new_fromcstr(__INA_TEST_EXE);
    pd.working_dir = NULL;
    pd.startup_args = ina_str_new_fromcstr("-h process spawn_and_wait 0");
    pd.lifecycle = INA_PROCESS_LIFECYCLE_TYPE_FIRE_AND_FORGET;
    pd.managed_type = 0;
    pd.scheduled_start_pattern = ina_str_new_fromcstr("");
    pd.scheduled_stop_pattern = ina_str_new_fromcstr("");
    pd.stop_wait_time_ms = 100;
    pd.start_flags = 0;

    INA_TEST_ASSERT_SUCCEED(ina_process_new(ctx, &pd, &process));
    INA_TEST_ASSERT_NOT_NULL(process);
    INA_TEST_ASSERT_SUCCEED(ina_process_query_state(process, &state));
    INA_TEST_ASSERT_EQUAL_INTEGER(INA_PROCESS_STARTABLE, state);
    INA_TEST_ASSERT_SUCCEED(ina_process_start(process));
    INA_TEST_ASSERT_SUCCEED(ina_process_query_state(process, &state));
    INA_TEST_ASSERT_EQUAL_INTEGER(INA_PROCESS_RUNNING, state);
    INA_TEST_ASSERT_SUCCEED(ina_process_free(&process));
    INA_TEST_ASSERT_NULL(process);
}

INA_TEST(process, should_be_running)
{
    ina_process_ctx_t *ctx;
    ina_process_t *process;
    ina_process_descriptor_t pd;
    
    INA_TEST_ASSERT_SUCCEED(ina_process_init(&ctx));
    INA_TEST_ASSERT_NOT_NULL(ctx);

    ina_mem_set(&pd, 0, sizeof(ina_process_descriptor_t));
    pd.full_path = ina_str_new_fromcstr(__INA_TEST_EXE);

    pd.working_dir = NULL;
    pd.startup_args = ina_str_new_fromcstr("-h process spawn_and_wait 0");
    pd.lifecycle = INA_PROCESS_LIFECYCLE_TYPE_WAIT;
    pd.managed_type = 0;
    pd.scheduled_start_pattern = NULL;
    pd.scheduled_stop_pattern = NULL;
    pd.stop_wait_time_ms = 100;
    pd.start_flags = 0;
   
    INA_TEST_ASSERT_SUCCEED(ina_process_new(ctx, &pd, &process));
    INA_TEST_ASSERT_NOT_NULL(process);
    INA_TEST_ASSERT_SUCCEED(ina_process_start(process));
    INA_TEST_ASSERT_SUCCEED(ina_process_free(&process));
}

INA_TEST(process, get_exit_code)
{
    ina_process_ctx_t *ctx;
    ina_process_t *process;
    ina_process_descriptor_t pd;
    int  exit_code;
   
    INA_TEST_ASSERT_SUCCEED(ina_process_init(&ctx));
    INA_TEST_ASSERT_NOT_NULL(ctx);

    ina_mem_set(&pd, 0, sizeof(ina_process_descriptor_t));
    pd.full_path = ina_str_new_fromcstr(__INA_TEST_EXE);
    pd.startup_args = ina_str_new_fromcstr("-h process spawn_and_wait 123");
    pd.lifecycle = INA_PROCESS_LIFECYCLE_TYPE_WAIT;
   
    INA_TEST_ASSERT_SUCCEED(ina_process_new(ctx, &pd, &process));
    INA_TEST_ASSERT_NOT_NULL(process);
    INA_TEST_ASSERT_SUCCEED(ina_process_start(process));
    INA_TEST_ASSERT_SUCCEED(ina_process_get_exit_code(process, &exit_code));
    INA_TEST_ASSERT_EQUAL_INTEGER(123, exit_code);
    INA_TEST_ASSERT_SUCCEED(ina_process_free(&process));
}

INA_TEST(process, stat)
{
    ina_process_stat_t *ps = NULL;
    int alive = 0;
    uint64_t mem = 0;
    int num_threads = 0;
    const char *cmd = NULL;

    INA_TEST_ASSERT_SUCCEED(ina_process_stat_new(&ps, __INA_TEST_EXE));
    INA_TEST_ASSERT_NOT_NULL(ps);
    INA_TEST_ASSERT_SUCCEED(ina_process_stat_query(ps));
    INA_TEST_ASSERT_SUCCEED(ina_process_stat_alive(ps, &alive));
    INA_TEST_ASSERT_SUCCEED(ina_process_stat_get_memory(ps, &mem));
    INA_TEST_ASSERT_SUCCEED(ina_process_stat_get_cmd(ps, &cmd));
    INA_TEST_ASSERT_SUCCEED(ina_process_stat_get_num_threads(ps, &num_threads));
    INA_TEST_ASSERT_TRUE(alive);
    INA_TEST_ASSERT_TRUE(mem > 0);
    INA_TEST_ASSERT_TRUE(num_threads > 0);
    INA_TEST_ASSERT_SUCCEED(ina_process_stat_free(&ps));
    INA_TEST_ASSERT_NULL(ps);

    alive = 0;
    mem = 0;
    num_threads = 0;
    cmd = NULL;

#ifdef INA_OS_WIN32
    INA_TEST_ASSERT_SUCCEED(ina_process_stat_new(&ps, "foo.exe"));
#else
    INA_TEST_ASSERT_SUCCEED(ina_process_stat_new(&ps, "foo"));
#endif
    INA_TEST_ASSERT_NOT_NULL(ps);
    INA_TEST_ASSERT_SUCCEED(ina_process_stat_query(ps));
    INA_TEST_ASSERT_SUCCEED(ina_process_stat_alive(ps, &alive));
    INA_TEST_ASSERT_SUCCEED(ina_process_stat_get_memory(ps, &mem));
    INA_TEST_ASSERT_SUCCEED(ina_process_stat_get_cmd(ps, &cmd));
    INA_TEST_ASSERT_SUCCEED(ina_process_stat_get_num_threads(ps, &num_threads));
    INA_TEST_ASSERT_FALSE(alive);
    INA_TEST_ASSERT_TRUE(mem == 0);
    INA_TEST_ASSERT_TRUE(num_threads == 0);
    INA_TEST_ASSERT_SUCCEED(ina_process_stat_free(&ps));
    INA_TEST_ASSERT_NULL(ps);
}
