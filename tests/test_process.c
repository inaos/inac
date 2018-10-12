/*
 * Copyright INAOS GmbH, Thalwil, 2013-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>

#ifndef INA_OS_WIN32
#define __INA_TEST_EXE "tests"
#else
#define __INA_TEST_EXE "tests.exe"
#endif

INA_TEST(process, init_destroy)
{   
    ina_process_ctx_t *ctx;    
    INA_TEST_ASSERT_SUCCEED(ina_process_ctx_new(&ctx));
    INA_TEST_ASSERT_NOT_NULL(ctx);
    ina_process_ctx_free(&ctx);
    INA_TEST_ASSERT_NULL(ctx);
}


INA_TEST(process, descriptor_new_free)
{
    ina_process_ctx_t *ctx;
    ina_process_descriptor_t *pd;
   
    INA_TEST_ASSERT_SUCCEED(ina_process_ctx_new(&ctx));
    INA_TEST_ASSERT_NOT_NULL(ctx);

    INA_TEST_ASSERT_SUCCEED(ina_process_descriptor_new(ctx,
                                                       "full_path",
                                                       "working_dir",
                                                       "1 2 3 4",
                                                       100,
                                                       0, &pd));
    INA_TEST_ASSERT_NOT_NULL(pd);
    INA_TEST_ASSERT_EQUAL_STR("full_path", pd->full_path);
    INA_TEST_ASSERT_EQUAL_STR("working_dir", pd->working_dir);
    INA_TEST_ASSERT_EQUAL_INT64(100, pd->stop_wait_time_ms);
    INA_TEST_ASSERT_EQUAL_INT(0, pd->cf);
    INA_TEST_ASSERT_EQUAL_STR("1 2 3 4", ina_str_cstr(pd->startup_args));
    ina_process_descriptor_free(&pd);
    INA_TEST_ASSERT_NULL(pd);
    ina_process_ctx_free(&ctx);
}

INA_TEST(process, new_free)
{
    ina_process_ctx_t *ctx;
    ina_process_descriptor_t pd;
    ina_process_t *process;
   
    INA_TEST_ASSERT_SUCCEED(ina_process_ctx_new(&ctx));
    INA_TEST_ASSERT_NOT_NULL(ctx);
    
    ina_mem_set(&pd, 0, sizeof(ina_process_descriptor_t));
    pd.full_path = ina_str_new_fromcstr(__INA_TEST_EXE);
    pd.working_dir = ina_str_new_fromcstr("./");
    pd.startup_args = ina_str_new_fromcstr("-h process");
    pd.stop_wait_time_ms = 100;
    pd.cf = 0;

    INA_TEST_ASSERT_SUCCEED(ina_process_new(ctx, &pd, &process));
    INA_TEST_ASSERT_NOT_NULL(process);
    ina_process_free(&process);
    INA_TEST_ASSERT_NULL(process);
    ina_process_ctx_free(&ctx);
    INA_TEST_ASSERT_NULL(ctx);
}

INA_TEST(process, start_and_wait)
{
    ina_process_ctx_t *ctx;
    ina_process_t *process;
    ina_process_descriptor_t pd;
    int  exit_code;
   
    INA_TEST_ASSERT_SUCCEED(ina_process_ctx_new(&ctx));
    INA_TEST_ASSERT_NOT_NULL(ctx);

    ina_mem_set(&pd, 0, sizeof(ina_process_descriptor_t));
    pd.full_path = ina_str_new_fromcstr(__INA_TEST_EXE);
    pd.startup_args = ina_str_new_fromcstr("-h process spawn_and_wait 0");
    pd.cf = INA_PROCESS_CF_WAIT;
    INA_TEST_ASSERT_SUCCEED(ina_process_new(ctx, &pd, &process));
    INA_TEST_ASSERT_NOT_NULL(process);
    INA_TEST_ASSERT_SUCCEED(ina_process_start(process));
    INA_TEST_ASSERT_SUCCEED(ina_process_get_exit_code(process, &exit_code));
    INA_TEST_ASSERT_EQUAL_INT(0, exit_code);
    ina_process_free(&process);
    ina_process_ctx_free(&ctx);
}

INA_TEST(process, stop)
{
    ina_process_ctx_t *ctx;
    ina_process_t *process;
    ina_process_descriptor_t pd;
    ina_fsm_state_t state;

    INA_TEST_ASSERT_SUCCEED(ina_process_ctx_new(&ctx));
    INA_TEST_ASSERT_NOT_NULL(ctx);

    ina_mem_set(&pd, 0, sizeof(ina_process_descriptor_t));
    pd.full_path = ina_str_new_fromcstr(__INA_TEST_EXE);
    pd.working_dir = NULL;
    pd.startup_args = ina_str_new_fromcstr("-h process spawn_and_wait 0");
    pd.stop_wait_time_ms = 100;
    pd.cf = 0;

    INA_TEST_ASSERT_SUCCEED(ina_process_new(ctx, &pd, &process));
    INA_TEST_ASSERT_NOT_NULL(process);
    INA_TEST_ASSERT_SUCCEED(ina_process_start(process));
    INA_TEST_ASSERT_SUCCEED(ina_process_query_state(process, &state));
    INA_TEST_ASSERT_EQUAL_INT(INA_PROCESS_RUNNING, state);
    INA_TEST_ASSERT_SUCCEED(ina_process_stop(process));
    INA_TEST_ASSERT_SUCCEED(ina_process_query_state(process, &state));
    INA_TEST_ASSERT_EQUAL_INT(INA_PROCESS_STOPPED, state);
    ina_process_free(&process);
    INA_TEST_ASSERT_NULL(process);
    ina_process_ctx_free(&ctx);
}

INA_TEST(process, state)
{
    ina_process_ctx_t *ctx;
    ina_process_t *process;
    ina_fsm_state_t state;
    ina_process_descriptor_t pd;
    
    INA_TEST_ASSERT_SUCCEED(ina_process_ctx_new(&ctx));
    INA_TEST_ASSERT_NOT_NULL(ctx);

    ina_mem_set(&pd, 0, sizeof(ina_process_descriptor_t));
    pd.full_path = ina_str_new_fromcstr(__INA_TEST_EXE);
    pd.working_dir = NULL;
    pd.startup_args = ina_str_new_fromcstr("-h process spawn_and_wait 0");
    pd.stop_wait_time_ms = 100;
    pd.cf = 0;

    INA_TEST_ASSERT_SUCCEED(ina_process_new(ctx, &pd, &process));
    INA_TEST_ASSERT_NOT_NULL(process);
    INA_TEST_ASSERT_SUCCEED(ina_process_query_state(process, &state));
    INA_TEST_ASSERT_EQUAL_INT(INA_PROCESS_STARTABLE, state);
    INA_TEST_ASSERT_SUCCEED(ina_process_start(process));
    INA_TEST_ASSERT_SUCCEED(ina_process_query_state(process, &state));
    INA_TEST_ASSERT_EQUAL_INT(INA_PROCESS_RUNNING, state);
    ina_process_free(&process);
    INA_TEST_ASSERT_NULL(process);
}

INA_TEST(process, should_be_running)
{
    ina_process_ctx_t *ctx;
    ina_process_t *process;
    ina_process_descriptor_t pd;
    
    INA_TEST_ASSERT_SUCCEED(ina_process_ctx_new(&ctx));
    INA_TEST_ASSERT_NOT_NULL(ctx);

    ina_mem_set(&pd, 0, sizeof(ina_process_descriptor_t));
    pd.full_path = ina_str_new_fromcstr(__INA_TEST_EXE);

    pd.working_dir = NULL;
    pd.startup_args = ina_str_new_fromcstr("-h process spawn_and_wait 0");
    pd.stop_wait_time_ms = 100;
    pd.cf = 0;
   
    INA_TEST_ASSERT_SUCCEED(ina_process_new(ctx, &pd, &process));
    INA_TEST_ASSERT_NOT_NULL(process);
    INA_TEST_ASSERT_SUCCEED(ina_process_start(process));
    ina_process_free(&process);
}

INA_TEST(process, get_exit_code)
{
    ina_process_ctx_t *ctx;
    ina_process_t *process;
    ina_process_descriptor_t pd;
    int  exit_code;
   
    INA_TEST_ASSERT_SUCCEED(ina_process_ctx_new(&ctx));
    INA_TEST_ASSERT_NOT_NULL(ctx);

    ina_mem_set(&pd, 0, sizeof(ina_process_descriptor_t));
    pd.full_path = ina_str_new_fromcstr(__INA_TEST_EXE);
    pd.startup_args = ina_str_new_fromcstr("-h process spawn_and_wait 123");
    pd.cf = INA_PROCESS_CF_WAIT;
   
    INA_TEST_ASSERT_SUCCEED(ina_process_new(ctx, &pd, &process));
    INA_TEST_ASSERT_NOT_NULL(process);
    INA_TEST_ASSERT_SUCCEED(ina_process_start(process));
    INA_TEST_ASSERT_SUCCEED(ina_process_get_exit_code(process, &exit_code));
    INA_TEST_ASSERT_EQUAL_INT(123, exit_code);
    ina_process_free(&process);
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
    ina_process_stat_free(&ps);
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
    ina_process_stat_free(&ps);
    INA_TEST_ASSERT_NULL(ps);
}
