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
#include "config.h"

#ifdef INA_OS_WIN32
#include <tlhelp32.h>
#include <Psapi.h>
#endif

struct ina_process_ctx_s {
    ina_cron_ctx_t *cron_ctx;
    ina_time_t *systime;
    ina_process_t *processes;
    ina_mempool_t *mempool;
};

struct ina_process_s {
    unsigned long key;
    int init;
    ina_process_ctx_t *ctx;
    ina_process_descriptor_t *descriptor;
    ina_fsm_status_t state;
    int exit_code;
    ina_rc_t last_rc;
#ifdef INA_OS_WIN32
    PROCESS_INFORMATION pi;
#else
     pid_t pid;
#endif
    UT_hash_handle hh;
};

struct ina_process_stat_s {
    ina_str_t binary;
    int available;
    ina_str_t cmd;
    uint64_t mem_bytes;
    int num_threads;
};

static void __ina_process_is_running(ina_process_t *, int*);
static void __ina_process_start(ina_process_t*);
static void __ina_process_stop(ina_process_t*);
static void __ina_process_reset(ina_process_t*);
static ina_rc_t __ina_process_query(const char *binary,
                                    int *available,
                                    ina_str_t *cmd,
                                    uint64_t *mem,
                                    int *num_threads);

static void __ina_process_fsm_event_start(void*);
static void __ina_process_fsm_event_stop(void*);
static void __ina_process_fsm_event_reset(void*);
static void __ina_process_fsm_event_error(void*);

INA_FSM_TRANSITIONS(process_fsm,
    INA_FSM_TRANSITION_EVENT(INA_PROCESS_START,
        INA_FSM_TRANSITION(INA_PROCESS_STARTABLE,
                            __ina_process_fsm_event_start,
                            INA_PROCESS_RUNNING),
        INA_FSM_TRANSITION(INA_PROCESS_RUNNING,
                            __ina_process_fsm_event_error,
                            INA_PROCESS_STARTABLE),
        INA_FSM_TRANSITION(INA_PROCESS_STOPPED,
                            __ina_process_fsm_event_error,
                            INA_PROCESS_STARTABLE)
    ),
    INA_FSM_TRANSITION_EVENT(INA_PROCESS_STOP,
        INA_FSM_TRANSITION(INA_PROCESS_STARTABLE,
                           __ina_process_fsm_event_error,
                           INA_PROCESS_RUNNING),
        INA_FSM_TRANSITION(INA_PROCESS_RUNNING,
                            __ina_process_fsm_event_stop,
                            INA_PROCESS_STOPPED),
        INA_FSM_TRANSITION(INA_PROCESS_STOPPED,
                            __ina_process_fsm_event_error,
                            INA_PROCESS_RUNNING)
    ),
    INA_FSM_TRANSITION_EVENT(INA_PROCESS_RESET,
        INA_FSM_TRANSITION(INA_PROCESS_STARTABLE,
                            __ina_process_fsm_event_error,
                            INA_PROCESS_STOPPED),
        INA_FSM_TRANSITION(INA_PROCESS_RUNNING,
                            __ina_process_fsm_event_error,
                            INA_PROCESS_STOPPED),
        INA_FSM_TRANSITION(INA_PROCESS_STOPPED,
                            __ina_process_fsm_event_reset,
                            INA_PROCESS_STARTABLE)
    )
);


static void __ina_process_fsm_event_start(void *user_data)
{
    ina_process_t *process = (ina_process_t*)user_data;
    INA_ASSERT_NOTNULL(process);
    INA_ASSERT_NOTNULL(process->descriptor);
    __ina_process_start(process);
}

static void __ina_process_fsm_event_stop(void *user_data)
{
    ina_process_t *process = (ina_process_t*)user_data;
    INA_ASSERT_NOTNULL(process);
    INA_ASSERT_NOTNULL(process->descriptor);
    __ina_process_stop(process);
}

static void __ina_process_fsm_event_reset(void *user_data)
{
    ina_process_t *process = (ina_process_t*)user_data;
    INA_ASSERT_NOTNULL(process);
    INA_ASSERT_NOTNULL(process->descriptor);
    __ina_process_reset(process);
}

static void __ina_process_fsm_event_error(void *user_data)
{
    ina_process_t *process = (ina_process_t*)user_data;
    INA_ASSERT_NOTNULL(process);
    /* FIXME error handling */
}

ina_rc_t __ina_process_cron_start_cb(ina_cron_ctx_t *ctx, void *user_data)
{
    ina_process_t *p = (ina_process_t*)user_data;
    if (INA_FSM_GET_STATE(process_fsm, p->state) == INA_PROCESS_STARTABLE) {
        INA_FSM_FIRE_EVENT(process_fsm, p->state, INA_PROCESS_START, p);
    }
    return INA_SUCCESS;
}

ina_rc_t __ina_process_cron_stop_cb(ina_cron_ctx_t *ctx, void *user_data)
{
    ina_process_t *p = (ina_process_t*)user_data;
    if (INA_FSM_GET_STATE(process_fsm, p->state) == INA_PROCESS_RUNNING) {
        INA_FSM_FIRE_EVENT(process_fsm, p->state, INA_PROCESS_STOP, p);
        INA_FSM_FIRE_EVENT(process_fsm, p->state, INA_PROCESS_RESET, p);
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_process_init(ina_process_ctx_t **ctx)
{
    INA_VERIFY_NOT_NULL(ctx);
    *ctx = (ina_process_ctx_t*)ina_mem_alloc(sizeof(ina_process_ctx_t));
    INA_RETURN_IF(*ctx == NULL);
    INA_RETURN_IF_FAILED(ina_time_sys_new(&(*ctx)->systime));
    INA_RETURN_IF_FAILED(ina_mempool_create(&(*ctx)->mempool,
                                           4096,
                                           INA_MEM_DYNAMIC,
                                           NULL));
    INA_RETURN_IF_FAILED(ina_cron_init(&(*ctx)->cron_ctx, NULL, NULL, *ctx));

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_process_destroy(ina_process_ctx_t **ctx)
{
    INA_VERIFY_NOT_NULL(ctx);
    INA_VERIFY_NOT_NULL(*ctx);

    if (HASH_COUNT((*ctx)->processes) > 0) {
        ina_process_t *p, *pt;
        HASH_ITER(hh, (*ctx)->processes, p, pt) {
            HASH_DELETE(hh, (*ctx)->processes, p);
            INA_MUST_SUCCEED(ina_process_free(&p));
        }
    }
    if ((*ctx)->cron_ctx != NULL) {
        INA_MUST_SUCCEED(ina_cron_destroy(&(*ctx)->cron_ctx));
    }
    if ((*ctx)->systime != NULL) {
        INA_MUST_SUCCEED(ina_time_sys_free(&(*ctx)->systime));
    }
    if ((*ctx)->mempool != NULL) {
        INA_MUST_SUCCEED(ina_mempool_release((*ctx)->mempool, INA_YES));
    }
    ina_mem_free(*ctx);
    *ctx = NULL;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_process_manage(ina_process_ctx_t *ctx)
{
    ina_process_t *p, *pt;
    time_t curr_time_sec;
    long curr_time_micros;
    int suggested_next_time;

    INA_VERIFY_NOT_NULL(ctx);

    if (INA_FAILED(ina_time_read_sys_clock(ctx->systime))) {
        return ina_err_get_last_rc();
    }

    ina_time_sys_seconds_micros(ctx->systime,
                                &curr_time_sec,
                                &curr_time_micros);

    HASH_ITER(hh, ctx->processes, p, pt) {
        /* if we need to perform init checks */
        if (p->init && p->descriptor->lifecycle ==
            INA_PROCESS_LIFECYCLE_TYPE_MANAGED) {

            time_t last_start = 0;
            time_t last_stop = 0;
            if (p->descriptor->managed_type ==
                INA_PROCESS_MANAGED_TYPE_SCHEDULED_START_STOP) {
                if (INA_FAILED(ina_cron_last_exec_systime(ctx->cron_ctx,
                                p->descriptor->scheduled_start_pattern,
                                curr_time_sec,
                                &last_start))) {
                    p->last_rc = ina_err_get_last_rc();
                    continue;
                }
                if (INA_FAILED(ina_cron_last_exec_systime(ctx->cron_ctx,
                                p->descriptor->scheduled_stop_pattern,
                                curr_time_sec,
                                &last_stop))) {
                    p->last_rc = ina_err_get_last_rc();
                    continue;
                }
                if (last_start > 0 &&
                    (curr_time_sec > last_start && last_stop < last_start)) {
                    /* we should be running therefore start */
                    p->last_rc = ina_process_start(p);
                }
            }
            p->init = INA_NO;
        }
        /* if process is supposed to be running, check if still running */
        if (INA_FSM_GET_STATE(process_fsm, p->state) == INA_PROCESS_RUNNING) {
            int running = 0;
            __ina_process_is_running(p, &running);
            if (!running) {
                INA_FSM_FIRE_EVENT(process_fsm, p->state, INA_PROCESS_STOP, p);
            }
        }
    }
    /* let cron decide whether its time to do something on a managed process */
    return ina_cron_process(ctx->cron_ctx, curr_time_sec, &suggested_next_time);
}

INA_API(ina_rc_t) ina_process_descriptor_new(
                              ina_process_ctx_t *ctx,
                              ina_process_descriptor_t **descriptor,
                              const char *full_path,
                              const char *working_dir,
                              const char *startup_args,
                              ina_process_lifecycle_type_t lifecycle,
                              ina_process_managed_type_t managed_type,
                              const char *scheduled_start_pattern,
                              const char *scheduled_stop_pattern,
                              time_t stop_wait_time_ms,
                              uint32_t start_flags)
{
    INA_VERIFY_NOT_NULL(ctx);
    INA_VERIFY_NOT_NULL(full_path);
    INA_VERIFY(strlen(full_path));

    *descriptor = (ina_process_descriptor_t*) ina_mem_alloc(
                                        sizeof(ina_process_descriptor_t));
    INA_RETURN_IF(*descriptor == NULL);

    (*descriptor)->full_path = ina_str_new_fromcstr(full_path);

    if (working_dir != NULL) {
        (*descriptor)->working_dir = ina_str_new_fromcstr(working_dir);
    }
    if (startup_args != NULL) {
        (*descriptor)->startup_args = ina_str_new_fromcstr(startup_args);
    }

    (*descriptor)->lifecycle = lifecycle;
    (*descriptor)->managed_type = managed_type;
    if (scheduled_start_pattern != NULL) {
        (*descriptor)->scheduled_start_pattern = ina_str_new_fromcstr(
                                                    scheduled_start_pattern);
    }
    if (scheduled_stop_pattern != NULL) {
        (*descriptor)->scheduled_stop_pattern = ina_str_new_fromcstr(
                                                    scheduled_stop_pattern);
    }
    (*descriptor)->stop_wait_time_ms = stop_wait_time_ms;
    (*descriptor)->start_flags = start_flags;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_process_descriptor_free(ina_process_descriptor_t **descriptor)
{
    INA_VERIFY_NOT_NULL(descriptor);
    INA_VERIFY_NOT_NULL(*descriptor);

    if ((*descriptor)->c_ref > 0) {
        return INA_ERROR(INA_NN_DESCRIPTOR|INA_ERR_IN_USE);
    }

    if ((*descriptor)->full_path != NULL) {
        ina_str_free((*descriptor)->full_path);
    }
    if ((*descriptor)->working_dir != NULL) {
        ina_str_free((*descriptor)->working_dir);
    }
    if ((*descriptor)->startup_args != NULL) {
        ina_str_free((*descriptor)->startup_args);
    }
    if ((*descriptor)->scheduled_stop_pattern != NULL) {
        ina_str_free((*descriptor)->scheduled_stop_pattern);
    }
    if ((*descriptor)->scheduled_start_pattern != NULL) {
        ina_str_free((*descriptor)->scheduled_start_pattern);
    }
    ina_mem_free(*descriptor);
    *descriptor = NULL;

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_process_exec(ina_process_ctx_t *ctx,
                                   const char *full_path,
                                   const char *startup_args,
                                   ina_process_t **process)
{
    ina_process_descriptor_t *ds = NULL;
    INA_VERIFY_NOT_NULL(ctx);
    INA_VERIFY_NOT_NULL(full_path);
    INA_VERIFY_NOT_NULL(process);

    *process = NULL;

    if (INA_FAILED(ina_process_descriptor_new(ctx, &ds, full_path, NULL, startup_args,
        INA_PROCESS_LIFECYCLE_TYPE_FIRE_AND_FORGET,
        INA_PROCESS_MANAGED_TYPE_PARENT_LIFETIME,
        NULL,
        NULL,
        20,
        0))) {
        return ina_err_get_last_rc();
    }
    if (INA_SUCCEED(ina_process_new(ctx, ds, process))) {
        return ina_process_start(*process);
    }
    return ina_err_get_last_rc();
}

INA_API(ina_rc_t) ina_process_exec_and_wait(ina_process_ctx_t *ctx,
                                   const char *full_path,
                                   const char *startup_args,
                                   ina_process_t **process)
{
    int exit_code;
    ina_process_descriptor_t *ds = NULL;

    INA_VERIFY_NOT_NULL(ctx);
    INA_VERIFY_NOT_NULL(full_path);
    INA_VERIFY_NOT_NULL(process);

    *process = NULL;

    if (INA_FAILED(ina_process_descriptor_new(ctx, &ds, full_path, NULL, startup_args,
        INA_PROCESS_LIFECYCLE_TYPE_WAIT,
        INA_PROCESS_MANAGED_TYPE_PARENT_LIFETIME,
        NULL,
        NULL,
        20,
        0))) {
        return ina_err_get_last_rc();
    }

    if (INA_SUCCEED(ina_process_new(ctx, ds, process))) {
        if (INA_FAILED(ina_process_start(*process))) {
            return ina_err_get_last_rc();
        }
        if (INA_FAILED(ina_process_get_exit_code(*process, &exit_code))) {
            return ina_err_get_last_rc();
        }
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_process_new(ina_process_ctx_t *ctx,
                                  ina_process_descriptor_t *descriptor,
                                  ina_process_t **process)
{
    ina_mempool_t *mp;
    INA_VERIFY_NOT_NULL(ctx);
    INA_VERIFY_NOT_NULL(descriptor);
    INA_VERIFY_NOT_NULL(process);

    *process = NULL;

    /* Check descriptor is not referenced */
    if (descriptor->c_ref > 0) {
        return INA_ERROR(INA_NN_DESCRIPTOR|INA_ERR_IN_USE);
    }

    /* Search for a recyclable process */
    if (HASH_COUNT(ctx->processes) > 0) {
        ina_process_t *p, *pt;
        HASH_ITER(hh, ctx->processes, p, pt) {
            if (p->descriptor == NULL) {
                *process = p;
                ina_mem_set(*process, 0, sizeof(ina_process_t));
            }
        }
    }

    /* If not found, allocate a new one from the memory pool */
    if (*process == NULL) {
        *process = ina_mempool_dalloc(ctx->mempool, sizeof(ina_process_t));
    }
    INA_RETURN_IF(*process == NULL);

    /* copy descriptor if not allocated from context pool */
    if (INA_SUCCEED(ina_mempool_getbypointer(descriptor, &mp)) &&
        mp == ctx->mempool) {
        (*process)->descriptor = descriptor;
    } else {
        (*process)->descriptor = (ina_process_descriptor_t*)ina_mempool_dalloc(
                                        ctx->mempool,
                                        sizeof(ina_process_descriptor_t));
        if ((*process)->descriptor == NULL) {
            *process = NULL;
            return ina_err_get_last_rc();
        }

        (*process)->descriptor->full_path = ina_str_dup_using_pool(
                                                    descriptor->full_path,
                                                    ctx->mempool);
        (*process)->descriptor->working_dir = ina_str_dup_using_pool(
                                                    descriptor->working_dir,
                                                    ctx->mempool);
        (*process)->descriptor->startup_args = ina_str_dup_using_pool(
                                                    descriptor->startup_args,
                                                    ctx->mempool);
        (*process)->descriptor->scheduled_start_pattern = ina_str_dup_using_pool(
                                        descriptor->scheduled_start_pattern,
                                        ctx->mempool);

        (*process)->descriptor->scheduled_stop_pattern = ina_str_dup_using_pool(
                                        descriptor->scheduled_stop_pattern,
                                        ctx->mempool);
        (*process)->descriptor->stop_wait_time_ms = descriptor->stop_wait_time_ms;
        (*process)->descriptor->start_flags = descriptor->start_flags;
        (*process)->descriptor->lifecycle = descriptor->lifecycle;
        (*process)->descriptor->managed_type = descriptor->managed_type;
    }
    (*process)->descriptor->c_ref = 1;
    (*process)->exit_code = -1;
    (*process)->key = INA_HASH_STR_TO_SDBM(descriptor->full_path);
    (*process)->init = INA_YES;
    (*process)->ctx = ctx;

    INA_FSM_SET_STATE(process_fsm, (*process)->state, INA_PROCESS_STARTABLE);

    if (descriptor->lifecycle == INA_PROCESS_LIFECYCLE_TYPE_MANAGED) {
        if (descriptor->managed_type == INA_PROCESS_MANAGED_TYPE_SCHEDULED_START
            || descriptor->managed_type ==
               INA_PROCESS_MANAGED_TYPE_SCHEDULED_START_STOP) {

                ina_str_t id = ina_str_sprintf("START_%lld", (int64_t)process);

                INA_ASSERT_NOTNULL(descriptor->scheduled_start_pattern);

                if (!INA_SUCCEED(ina_cron_register_function(ctx->cron_ctx,
                                    ina_str_cstr(id),
                                    descriptor->scheduled_start_pattern,
                                    *process,
                                    __ina_process_cron_start_cb))) {
                    ina_str_free(id);
                    *process = NULL;
                    return ina_err_get_last_rc();
                }
                ina_str_free(id);
        }
        if (descriptor->managed_type ==
            INA_PROCESS_MANAGED_TYPE_SCHEDULED_START_STOP) {

            ina_str_t id = ina_str_sprintf("STOP_%lld", (int64_t)process);

            INA_ASSERT_NOTNULL(descriptor->scheduled_stop_pattern);

            if (!INA_SUCCEED(ina_cron_register_function(ctx->cron_ctx,
                                ina_str_cstr(id),
                                descriptor->scheduled_stop_pattern,
                                *process,
                                __ina_process_cron_stop_cb))) {
                ina_str_free(id);
                *process = NULL;
                return ina_err_get_last_rc();
            }
            ina_str_free(id);
        }
    }

    HASH_ADD_ULONG(ctx->processes, key, (*process));
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_process_free(ina_process_t **process)
{
    INA_VERIFY_NOT_NULL(process);
    INA_VERIFY_NOT_NULL(*process);

    /* Release descriptor if any */
    if ((*process)->descriptor != NULL) {
        (*process)->descriptor->c_ref -= 1;
    }
    (*process)->init = INA_YES;
    (*process)->descriptor = NULL;
    *process = NULL;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_process_start(ina_process_t *process)
{
    INA_VERIFY_NOT_NULL(process);
    INA_FSM_FIRE_EVENT(process_fsm, process->state, INA_PROCESS_START, process);
    return process->last_rc;
}

INA_API(ina_rc_t) ina_process_stop(ina_process_t *process)
{
    INA_VERIFY_NOT_NULL(process);
    INA_FSM_FIRE_EVENT(process_fsm, process->state, INA_PROCESS_STOP, process);
    return process->last_rc;
}

INA_API(ina_rc_t) ina_process_query_state(ina_process_t *process,
                                          ina_fsm_state_t *state)
{
    INA_VERIFY_NOT_NULL(process);
    INA_VERIFY_NOT_NULL(state);
    *state = INA_FSM_GET_STATE(process_fsm, process->state);
    return process->last_rc;
}

INA_API(ina_rc_t) ina_process_reset(ina_process_t *process)
{
    INA_VERIFY_NOT_NULL(process);
    INA_FSM_FIRE_EVENT(process_fsm, process->state, INA_PROCESS_RESET, process);
    return process->last_rc;
}

INA_API(ina_rc_t) ina_process_get_exit_code(ina_process_t *process,
                                            int *exit_code)
{
    int still_running;
    INA_VERIFY_NOT_NULL(process);
    INA_VERIFY_NOT_NULL(exit_code);
    __ina_process_is_running(process, &still_running);
    if (still_running == INA_YES) {
        return INA_ERROR(INA_NN_PROCESS|INA_ERR_RUNNING);
    }
    *exit_code = process->exit_code;
    return process->last_rc;
}

INA_API(ina_rc_t) ina_process_should_be_running(ina_process_t *process,
                                                int *should_be_running)
{
    time_t curr_time_sec;
    long curr_time_micros;

    INA_VERIFY_NOT_NULL(process);
    INA_VERIFY_NOT_NULL(process->ctx);
    INA_VERIFY_NOT_NULL(should_be_running);

    if (INA_FAILED(ina_time_read_sys_clock(process->ctx->systime))) {
        return ina_err_get_last_rc();
    }
    if (INA_FAILED(ina_time_sys_seconds_micros(process->ctx->systime,
        &curr_time_sec, &curr_time_micros))) {
        return ina_err_get_last_rc();
    }

    if (process->descriptor->lifecycle == INA_PROCESS_LIFECYCLE_TYPE_MANAGED) {
        time_t last_start = 0;
        time_t last_stop = 0;
        if (process->descriptor->managed_type ==
            INA_PROCESS_MANAGED_TYPE_SCHEDULED_START_STOP) {

            ina_cron_last_exec_systime(process->ctx->cron_ctx,
                                process->descriptor->scheduled_start_pattern,
                                curr_time_sec,
                                &last_start);
            ina_cron_last_exec_systime(process->ctx->cron_ctx,
                                process->descriptor->scheduled_stop_pattern,
                                curr_time_sec,
                                &last_stop);
            if (last_start > 0 &&
                (curr_time_sec > last_start && last_stop < last_start)) {

                *should_be_running = 1;
                return INA_SUCCESS;
            } else {
                *should_be_running = 0;
                return INA_SUCCESS;
            }
        }
    }
    *should_be_running = 0;
    return INA_ERROR(INA_NN_PROCESS|INA_ERR_NOT_ALLOWED);
}

INA_API(ina_rc_t) ina_process_stat_new(ina_process_stat_t **stat, const char *binary)
{
    INA_VERIFY_NOT_NULL(stat);
    INA_VERIFY_NOT_NULL(binary);
    INA_VERIFY(strlen(binary));

    *stat = (ina_process_stat_t*)ina_mem_alloc(sizeof(ina_process_stat_t));
    INA_RETURN_IF(*stat == NULL);
    (*stat)->binary = ina_str_new_fromcstr(binary);
    (*stat)->available = 0;
    (*stat)->cmd = NULL;
    (*stat)->mem_bytes = 0;
    (*stat)->num_threads = 0;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_process_stat_query(ina_process_stat_t *stat)
{
    INA_VERIFY_NOT_NULL(stat);
    if (stat->cmd != NULL) {
        ina_str_free(stat->cmd);
    }
    stat->available = 0;
    stat->cmd = NULL;
    stat->mem_bytes = 0;
    stat->num_threads = 0;

    return __ina_process_query(ina_str_cstr(stat->binary),
        &stat->available, &stat->cmd, &stat->mem_bytes, &stat->num_threads);
}

INA_API(ina_rc_t) ina_process_stat_alive(ina_process_stat_t *stat, int *alive)
{

    INA_VERIFY_NOT_NULL(stat);
    INA_VERIFY_NOT_NULL(alive);
    *alive = stat->available;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_process_stat_get_cmd(ina_process_stat_t *stat, const char **cmd)
{
    INA_VERIFY_NOT_NULL(stat);
    INA_VERIFY_NOT_NULL(cmd);
    *cmd = ina_str_cstr(stat->cmd);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_process_stat_get_memory(ina_process_stat_t *stat, uint64_t *memory)
{
    INA_VERIFY_NOT_NULL(stat);
    INA_VERIFY_NOT_NULL(memory);
    *memory = stat->mem_bytes;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_process_stat_get_num_threads(ina_process_stat_t *stat, int *num_threads)
{
    INA_VERIFY_NOT_NULL(stat);
    INA_VERIFY_NOT_NULL(num_threads);
    *num_threads = stat->num_threads;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_process_stat_free(ina_process_stat_t **stat)
{
    INA_VERIFY_NOT_NULL(stat);
    INA_VERIFY_NOT_NULL(*stat);

    if ((*stat)->cmd != NULL) {
        ina_str_free((*stat)->cmd);
    }
    if ((*stat)->binary != NULL) {
        ina_str_free((*stat)->binary);
    }
    ina_mem_free(*stat);
    *stat = NULL;
    return INA_SUCCESS;
}

#ifdef INA_OS_WIN32
static void __ina_process_is_running(ina_process_t *process,
                                     int *still_running)
{
    DWORD ec;
    /* FIME: Error handling */

    *still_running = INA_NO;

    if (GetExitCodeProcess(process->pi.hProcess, &ec) == 0) {
        process->last_rc = INA_OS_ERROR(INA_NN_OPERATION|INA_ERR_FAILED);
        return;
    }
    if (ec == STILL_ACTIVE) {
        *still_running = INA_YES;
    } else if (process->exit_code < 0) {
        process->exit_code = ec;
    }
}

static void __ina_process_start(ina_process_t *process)
{
    STARTUPINFO si;
    ina_str_t cmd_line;
    BOOL ret;
    DWORD creation_flags = 0;

    ina_mem_set(&si, 0, sizeof(STARTUPINFO));
    si.cb = sizeof(si);
    ina_mem_set(&process->pi, 0, sizeof(PROCESS_INFORMATION));
    cmd_line = ina_str_new(1024);
    cmd_line = ina_str_cat(cmd_line, process->descriptor->full_path);
    cmd_line = ina_str_catcstr(cmd_line, " ");
    cmd_line = ina_str_cat(cmd_line, process->descriptor->startup_args);

    if ((process->descriptor->start_flags & INA_PROCESS_FLAGS_CHILD_PROCESS)
        != INA_PROCESS_FLAGS_CHILD_PROCESS) {
        if ((process->descriptor->start_flags & INA_PROCESS_FLAGS_CONSOLE)
            == INA_PROCESS_FLAGS_CONSOLE) {
            creation_flags |= CREATE_NEW_CONSOLE;
        } else {
            creation_flags |= DETACHED_PROCESS;
        }
    }
    creation_flags |= NORMAL_PRIORITY_CLASS;

    ret = CreateProcess(NULL,
        (LPSTR)ina_str_cstr(cmd_line), NULL, NULL, FALSE,
        creation_flags, NULL,
        ina_str_cstr(process->descriptor->working_dir),
        &si, &process->pi
    );

    if (process->descriptor->lifecycle == INA_PROCESS_LIFECYCLE_TYPE_WAIT) {
        int still_running;
        WaitForSingleObject(process->pi.hProcess, INFINITE);
        __ina_process_is_running(process, &still_running);
    }

    ina_str_free(cmd_line);
}
static void __ina_process_stop(ina_process_t *process)
{
    BOOL ret;
    HANDLE rh;
    LPTHREAD_START_ROUTINE lsp = NULL;
    DWORD rhexit = 0;
    int still_running = 0;

    lsp = (LPTHREAD_START_ROUTINE)GetProcAddress(
        GetModuleHandle(TEXT("kernel32.dll")), "CtrlRoutine");
    rh = CreateRemoteThread(process->pi.hProcess, NULL, 0, lsp, (void*)CTRL_C_EVENT, 0, NULL);
    WaitForSingleObject(rh, INFINITE);
    GetExitCodeThread(rh, &rhexit);
    CloseHandle(rh);
    if (WAIT_TIMEOUT == WaitForSingleObject(
                            process->pi.hProcess,
                            (DWORD)process->descriptor->stop_wait_time_ms)) {
        ret = TerminateProcess(process->pi.hProcess, EXIT_FAILURE);
        WaitForSingleObject(process->pi.hProcess, INFINITE);
    }
    /* update exit-code */
    __ina_process_is_running(process, &still_running);
}
static void __ina_process_reset(ina_process_t *process)
{
    CloseHandle(process->pi.hProcess);
    CloseHandle(process->pi.hThread);
    process->last_rc = ina_err_clear_rc(process->last_rc);
}
static ina_rc_t __ina_process_query(const char *binary,
                                    int *available,
                                    ina_str_t *cmd,
                                    uint64_t *mem,
                                    int *num_threads)
{
    PROCESSENTRY32 p_entry;
    DWORD th32procid = 0;
    HANDLE snapshot;
    p_entry.dwSize = sizeof(PROCESSENTRY32);

    *available = 0;

    snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, th32procid);
    if (Process32First(snapshot, &p_entry) == TRUE) {
        while (Process32Next(snapshot, &p_entry) == TRUE) {
            if (_stricmp(p_entry.szExeFile, binary) == 0) {
                HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, p_entry.th32ProcessID);
                PROCESS_MEMORY_COUNTERS mem_info;
                char fullpath[4096];
                DWORD path_size = 4096;
                memset(&mem_info, 0, sizeof(PROCESS_MEMORY_COUNTERS));
                GetProcessMemoryInfo(hProcess, &mem_info, sizeof(PROCESS_MEMORY_COUNTERS));
                QueryFullProcessImageName(hProcess, 0, fullpath, &path_size);
                *num_threads = p_entry.cntThreads;
                *cmd = ina_str_new_fromcstr(fullpath);
                *mem = mem_info.WorkingSetSize;
                *available = 1;
                CloseHandle(hProcess);
                break;
            }
        }
    }
    CloseHandle(snapshot);

    if (!*available) {
        *cmd = NULL;
        *mem = 0;
        *num_threads = 0;
    }

    return INA_SUCCESS;
}

#else
static void __ina_process_is_running(ina_process_t *process,
                                      int *still_running)
{
    int status = 0;
    pid_t w;

    *still_running = INA_NO;

    w = waitpid(process->pid, &status, WNOHANG);
    if (w == -1) {
        if (errno != ECHILD) {
            process->last_rc = INA_OS_ERROR(INA_NN_OPERATION|INA_ERR_FAILED);
        }
    } else if (w > 0) {
        *still_running = INA_YES;
    } else if (process->exit_code < 0) {
        if (WIFEXITED(status)) {
            process->exit_code = WEXITSTATUS(status);
        }
    }
}

static void __ina_process_start(ina_process_t *process)
{
    pid_t pid = fork();

    if (pid < 0) {
        process->last_rc = INA_OS_ERROR(INA_NN_PROCESS|INA_ERR_NOT_CREATED);
        return;
    }

    if (pid == 0) {
        ina_str_t *tokens;
        char* args[16]; /* FIXME */
        size_t n = 0;
        size_t c = 0;

        if (process->descriptor->working_dir != NULL) {
            if (chdir(process->descriptor->working_dir) != 0) {
                INA_OS_ERROR(INA_NN_DIRECTORY|INA_ERR_NOT_CHANGED);
                return;
            }
        }

        c = 0;
        args[n++] = (char*)ina_str_cstr(process->descriptor->full_path);
        tokens = ina_str_split(process->descriptor->startup_args, " ", &c);
        while (c--) {
            args[n] = (char*)tokens[n-1];
            n++;
        }
        args[n++] = NULL;
        execv(args[0], args);
        INA_OS_ERROR(INA_NN_PROCESS|INA_ERR_NOT_CREATED);
        INA_TRACE("%s", "FAILED");
        exit(127);
    } else {
        int status = 0;

        /* Store pid */
        process->pid = pid;

        if (process->descriptor->lifecycle == INA_PROCESS_LIFECYCLE_TYPE_WAIT) {
            waitpid(process->pid, &status, 0);
            if (WIFEXITED(status)) {
                process->exit_code = WEXITSTATUS(status);
            }
        }
    }
}

static void __ina_process_stop(ina_process_t *process)
{
    int still_running = INA_NO;

    if (process->pid > 0) {
        INA_TRACE("Kill %d", process->pid);
        if (kill(process->pid, SIGTERM) == -1) {
            INA_TRACE("%s", "FAILED to kill");
            process->last_rc = INA_OS_ERROR(INA_NN_PROCESS|INA_ERR_NOT_STOPPED);
            return;
        }
    }
    __ina_process_is_running(process, &still_running);
}

static void __ina_process_reset(ina_process_t *process)
{
    process->pid = 0;
    process->exit_code  = -1;
    process->last_rc = ina_err_clear_rc(process->last_rc);
}

static ina_rc_t __ina_process_query(const char *binary,
                                    int *available,
                                    ina_str_t *cmd,
                                    uint64_t *mem,
                                    int *num_threads)
{
    ina_ljit_ctx_t *ctx;

    INA_RETURN_IF_FAILED(ina_ljit_init(&ctx));

    if (INA_FAILED(ina_ljit_dostring(ctx, "local pq = require(\"lprocqry\");pqf=pq.query"))) {
        return ina_err_get_last_rc();
    }

    lua_getglobal(ctx->lstate, "pqf");
    lua_pushstring(ctx->lstate, binary);
    lua_pcall(ctx->lstate, 1, 1, 0);
    if (lua_istable(ctx->lstate, -1)) {
        *available = 1;
        /* get_cmd */
        lua_pushstring(ctx->lstate, "_cmd");
        lua_gettable(ctx->lstate, -2);
        *cmd = ina_str_new_fromcstr(lua_tostring(ctx->lstate, -1));
        lua_pop(ctx->lstate, 1);
        /* get_used_mem */
        lua_pushstring(ctx->lstate, "_rss");
        lua_gettable(ctx->lstate, -2);
        *mem = (uint64_t)lua_tonumber(ctx->lstate, -1);
        lua_pop(ctx->lstate, 1);
        /* get_num_threads */
        lua_pushstring(ctx->lstate, "_threads");
        lua_gettable(ctx->lstate, -2);
        *num_threads = (int)lua_tonumber(ctx->lstate, -1);
        lua_pop(ctx->lstate, 1);
        /* pop the table */
        lua_pop(ctx->lstate, 1);
    } else {
        *available = 0;
        *cmd = NULL;
        *mem = 0;
        *num_threads = 0;
    }

    ina_ljit_destroy(&ctx);

    return INA_SUCCESS;
}

#endif
