/*
 * Copyright INAOS GmbH, Thalwil, 2013-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>

#ifndef INA_OS_WINDOWS
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

INA_TEST_SKIP(process, stop)
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

INA_TEST_SKIP(process, stat)
{
    ina_process_stat_t *ps = NULL;
    int alive = 0;
    uint64_t mem = 0;
    int num_threads = 0;
    const char *cmd = NULL;

    INA_TEST_ASSERT_SUCCEED(ina_process_stat_new(__INA_TEST_EXE, &ps));
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

#ifdef INA_OS_WINDOWS
    INA_TEST_ASSERT_SUCCEED(ina_process_stat_new("foo.exe", &ps));
#else
    INA_TEST_ASSERT_SUCCEED(ina_process_stat_new("foo", &ps));
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

INA_TEST(process, invalid_arguments)
{
    ina_process_ctx_t *ctx = NULL;
    ina_process_descriptor_t *ds = NULL;
    ina_process_t *process = NULL;
    ina_fsm_state_t state = 0;
    ina_process_stat_t *stat;
    const char* c = NULL;
    uint64_t u64 = 0;
    int fake  = 0;

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_process_ctx_new(NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_process_descriptor_new(NULL, "test.exe", "c:\\temp", "-b", 0, 0, &ds));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_process_descriptor_new(ctx, NULL, "c:\\temp", "-b", 0, 0, &ds));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_process_descriptor_new(ctx, "", "c:\\temp", "-b", 0, 0, NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_process_exec(NULL, "test.exe", "-b", &process));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_process_exec(ctx, NULL, "-b", &process));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_process_exec(ctx, "", "-b", &process));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_process_exec(ctx, "test.exe", "-b", NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_process_exec_and_wait(NULL, "test.exe", "-b", &process));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_process_exec_and_wait(ctx, NULL, "-b", &process));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_process_exec_and_wait(ctx, "", "-b", &process));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_process_exec_and_wait(ctx, "test.exe", "-b", NULL));

    ctx = (ina_process_ctx_t*)&fake;
    ds = (ina_process_descriptor_t*)&fake;
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_process_new(NULL, ds, &process));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_process_new(ctx, NULL, &process));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_process_new(ctx, ds, NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_process_start(NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_process_stop(NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_process_query_state(NULL, &state));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_process_query_state(process, NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_process_get_exit_code(NULL, &fake));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_process_get_exit_code(process, NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_process_should_be_running(NULL, &fake));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_process_should_be_running(process, NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_process_stat_new(NULL, &stat));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_process_stat_new("", &stat));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_process_stat_new("test.exe", NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_process_stat_query(NULL));

    stat = (ina_process_stat_t*)&fake;
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_process_stat_alive(NULL, &fake));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_process_stat_alive(stat, NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_process_stat_get_cmd(NULL, &c));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_process_stat_get_cmd(stat, NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_process_stat_get_memory(NULL, &u64));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_process_stat_get_memory(stat, NULL));


    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_process_stat_get_num_threads(NULL, &fake));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_process_stat_get_num_threads(stat, NULL));



}