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
    ina_process_ctx_t *ctx;    
    INA_TEST_ASSERT_SUCCEED(ina_process_init(&ctx));
    INA_TEST_ASSERT_NOT_NULL(ctx);
    INA_TEST_ASSERT_SUCCEED(ina_process_manage(ctx));
    INA_TEST_ASSERT_SUCCEED(ina_process_destroy(&ctx));
    INA_TEST_ASSERT_NULL(ctx);    
}

INA_TEST(process, new_free)
{
    ina_process_ctx_t *ctx;
    ina_process_descriptor_t pd;
    ina_process_t *process;
   
    INA_TEST_ASSERT_SUCCEED(ina_process_init(&ctx));
    INA_TEST_ASSERT_NOT_NULL(ctx);
    
    ina_mem_set(&pd, 0, sizeof(ina_process_descriptor_t));
    pd.full_path = ina_str_create("");
    pd.working_dir = ina_str_create("");
    pd.startup_args = ina_str_create("");
    pd.lifecycle = INA_PROCESS_LIFECYCLE_TYPE_FIRE_AND_FORGET;
    pd.managed_type = 0;
    pd.scheduled_start_pattern = ina_str_create("");
    pd.scheduled_stop_pattern = ina_str_create("");
    pd.stop_wait_time_ms = ina_str_create("");
    pd.start_flags = 0;

    INA_TEST_ASSERT_SUCCEED(ina_process_new(ctx, &pd, &process));
    INA_TEST_ASSERT_NOT_NULL(process);
    INA_TEST_ASSERT_SUCCEED(ina_process_free(ctx, &process));
    INA_TEST_ASSERT_NULL(process);
    
    INA_TEST_ASSERT_SUCCEED(ina_process_destroy(&ctx));
    INA_TEST_ASSERT_NULL(ctx);    
}

INA_TEST(process, start)
{
    ina_process_ctx_t *ctx;
    ina_process_t *process;
    
    INA_TEST_ASSERT_SUCCEED(ina_process_init(&ctx));
    INA_TEST_ASSERT_NOT_NULL(ctx);

    ina_process_start(ctx, process);
}

INA_TEST(process, stop)
{
    ina_process_ctx_t *ctx;
    ina_process_t *process;
    
    ina_process_stop(ctx,  &process);
}

INA_TEST(process, state)
{
    ina_process_ctx_t *ctx;
    ina_process_t *process;
    ina_fsm_state_t *state;
    
    ina_process_query_state(ctx, process, state);
}

INA_TEST(process, hould_be_running)
{
    ina_process_ctx_t *ctx;
    ina_process_t *process;
    int should_be_running;
    ina_process_should_be_running(ctx, process, &should_be_running);
}
    
INA_TEST(process, next_state)
{
    ina_process_ctx_t *ctx;
    ina_process_t *process;

    ina_process_next_state(ctx, process);
}

INA_TEST(process, get_exit_code)
{
    ina_process_ctx_t *ctx;
    ina_process_t *process;
    int exit_code;
    ina_process_get_exit_code(ctx, process, &exit_code);
}
