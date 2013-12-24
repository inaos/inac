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
#include "config.h"

struct ina_process_ctx_s {
    ina_cron_ctx_t *cron_ctx;
    ina_time_t *systime;
    ina_process_t *processes;
};

struct ina_process_s {
    unsigned long key;
    int init;
    ina_process_descriptor_t *descriptor;
    ina_fsm_status_t state;
    int exit_code;
#ifdef INA_OS_WIN32
    PROCESS_INFORMATION pi;
#endif
    UT_hash_handle hh;
};

static void __ina_process_fsm_event_start(ina_process_t*);
static void __ina_process_fsm_event_stop(ina_process_t*);
static void __ina_process_fsm_event_reset(ina_process_t*);
static void __ina_process_fsm_event_error(ina_process_t*);

INA_FSM_TRANSITIONS(process_fsm,
    INA_FSM_TRANSITION_EVENT(INA_PROCESS_START,
        INA_FSM_TRANSITION(INA_PROCESS_STARTABLE, __ina_process_fsm_event_start, INA_PROCESS_RUNNING),
        INA_FSM_TRANSITION(INA_PROCESS_RUNNING, __ina_process_fsm_event_error, INA_PROCESS_STARTABLE),
        INA_FSM_TRANSITION(INA_PROCESS_STOPPED, __ina_process_fsm_event_error, INA_PROCESS_STARTABLE)
    ),
    INA_FSM_TRANSITION_EVENT(INA_PROCESS_STOP,
        INA_FSM_TRANSITION(INA_PROCESS_STARTABLE, __ina_process_fsm_event_error, INA_PROCESS_RUNNING),
        INA_FSM_TRANSITION(INA_PROCESS_RUNNING, __ina_process_fsm_event_stop, INA_PROCESS_STOPPED),
        INA_FSM_TRANSITION(INA_PROCESS_STOPPED, __ina_process_fsm_event_error, INA_PROCESS_RUNNING)
    ),
    INA_FSM_TRANSITION_EVENT(INA_PROCESS_RESET,
        INA_FSM_TRANSITION(INA_PROCESS_STARTABLE, __ina_process_fsm_event_error, INA_PROCESS_STOPPED),
        INA_FSM_TRANSITION(INA_PROCESS_RUNNING, __ina_process_fsm_event_error, INA_PROCESS_STOPPED),
        INA_FSM_TRANSITION(INA_PROCESS_STOPPED, __ina_process_fsm_event_reset, INA_PROCESS_STARTABLE)
    )
);

#ifdef INA_OS_WIN32
static ina_rc_t __ina_process_win_is_running(ina_process_t *process, int *still_running)
{
    DWORD ec;
    BOOL ret;
    ret = GetExitCodeProcess(process->pi.hProcess, &ec);
    if (ec == STILL_ACTIVE) {
        process->exit_code = -1;
        *still_running = 1;
    }
    else {
        process->exit_code = ec;
        *still_running = 0;
    }
    return INA_SUCCESS;
}
static void __ina_process_win_start(ina_process_t *process)
{
    STARTUPINFO si;
    char *cmd_line;
    size_t len;
    BOOL ret;
    DWORD creation_flags = 0;

    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(si);
    ZeroMemory(&process->pi, sizeof(PROCESS_INFORMATION));
    len = ina_str_len(process->descriptor->full_path);
    len += 1+ina_str_len(process->descriptor->startup_args);
    cmd_line = (char*)ina_mem_alloc(sizeof(char)*(len+1));
    cmd_line = strcpy(cmd_line, ina_str_cstr(process->descriptor->full_path));
    cmd_line = strcat(cmd_line, " ");
    cmd_line = strcat(cmd_line, ina_str_cstr(process->descriptor->startup_args));
    cmd_line[len] = '\0';

    if ((process->descriptor->start_flags & INA_PROCESS_FLAGS_CHILD_PROCESS) != INA_PROCESS_FLAGS_CHILD_PROCESS) {
        if ((process->descriptor->start_flags & INA_PROCESS_FLAGS_CONSOLE) == INA_PROCESS_FLAGS_CONSOLE) {
            creation_flags |= CREATE_NEW_CONSOLE;
        }
        else {
            creation_flags |= DETACHED_PROCESS;
        }
    }
    creation_flags |= NORMAL_PRIORITY_CLASS;

    ret = CreateProcess(NULL,
        cmd_line, NULL, NULL, FALSE,
        creation_flags, NULL,
        ina_str_cstr(process->descriptor->working_dir),
        &si, &process->pi
    );

    ina_mem_free(cmd_line);
}
static void __ina_process_win_stop(ina_process_t *process)
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
    if (WAIT_TIMEOUT == WaitForSingleObject(process->pi.hProcess, (DWORD)process->descriptor->stop_wait_time_ms)) {
        ret = TerminateProcess(process->pi.hProcess, EXIT_FAILURE);
        WaitForSingleObject(process->pi.hProcess, INFINITE);
    }
    /* update exit-code */
    __ina_process_win_is_running(process, &still_running);
}
static void __ina_process_win_reset(ina_process_t *process)
{
    CloseHandle(process->pi.hProcess);
    CloseHandle(process->pi.hThread);
}
#endif

static void __ina_process_fsm_event_start(ina_process_t *process)
{
#ifdef INA_OS_WIN32
    __ina_process_win_start(process);
#else
#endif
}

static void __ina_process_fsm_event_stop(ina_process_t *process)
{
#ifdef INA_OS_WIN32
    __ina_process_win_stop(process);
#else
#endif
}

static void __ina_process_fsm_event_reset(ina_process_t *process)
{
#ifdef INA_OS_WIN32
    __ina_process_win_reset(process);
#else
#endif
}

static void __ina_process_fsm_event_error(ina_process_t *process)
{
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
    *ctx = (ina_process_ctx_t*)ina_mem_alloc(sizeof(ina_process_ctx_t));
    (*ctx)->processes = NULL;
    if (!INA_SUCCEED(ina_time_sys_new(&(*ctx)->systime))) {
        return INA_ERR_PUSH_LAST;
    }
    if (!INA_SUCCEED(ina_cron_init(&(*ctx)->cron_ctx, NULL, NULL))) {
        return INA_ERR_PUSH_LAST;
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_process_destroy(ina_process_ctx_t **ctx)
{
    ina_process_t *p, *pt;
    HASH_ITER(hh, (*ctx)->processes, p, pt) {
        HASH_DELETE(hh, (*ctx)->processes, p);
        ina_process_free((*ctx), &p);
    }
    ina_cron_destroy(&(*ctx)->cron_ctx);
    ina_time_sys_free(&(*ctx)->systime);
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
    
    if (!INA_SUCCEED(ina_time_read_sys_clock(ctx->systime))) {
        return INA_ERR_PUSH_LAST;
    }
    if (!INA_SUCCEED(ina_time_sys_seconds_micros(ctx->systime, 
        &curr_time_sec, &curr_time_micros))) {
        return INA_ERR_PUSH_LAST;
    }

    HASH_ITER(hh, ctx->processes, p, pt) {
        /* if we need to perfom init checks */
        if (p->init && p->descriptor->lifecycle == INA_PROCESS_LIFECYCLE_TYPE_MANAGED) {
            time_t last_start = 0;
            time_t last_stop = 0;
            if (p->descriptor->managed_type == INA_PROCESS_MANAGED_TYPE_SCHEDULED_START_STOP) {
                ina_cron_last_exec_systime(ctx->cron_ctx, p->descriptor->scheduled_start_pattern, curr_time_sec, &last_start);
                ina_cron_last_exec_systime(ctx->cron_ctx, p->descriptor->scheduled_stop_pattern, curr_time_sec, &last_stop);
                if (last_start > 0 && ( curr_time_sec > last_start && last_stop < last_start) ) {
                    /* we should be running therefore start */
                    ina_process_start(ctx, p);
                }
            }
            p->init = 0;
        }
        /* if process is supposed to be running, check if still running */
        if (INA_FSM_GET_STATE(process_fsm, p->state) == INA_PROCESS_RUNNING) {
            int running = 0;
#ifdef INA_OS_WIN32
            __ina_process_win_is_running(p, &running);
#else
#endif
            if (!running) {
                INA_FSM_FIRE_EVENT(process_fsm, p->state, INA_PROCESS_STOP, p);
            }
        }
    }
    /* let cron decide whether its time to do something on a managed process */
    ina_cron_process(ctx->cron_ctx, curr_time_sec, &suggested_next_time);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_process_new(ina_process_ctx_t *ctx, ina_process_descriptor_t *descriptor, ina_process_t **process)
{
    *process = (ina_process_t*)ina_mem_alloc(sizeof(ina_process_t));
    (*process)->descriptor = descriptor;
    (*process)->exit_code = 0;
    (*process)->key = INA_HASH_CSTR_TO_SDBM(ina_str_cstr(descriptor->full_path));
    (*process)->init = 1;
    ina_mem_set(&(*process)->state, 0, sizeof(ina_fsm_status_t));

    INA_FSM_SET_STATE(process_fsm, (*process)->state, INA_PROCESS_STARTABLE);
    INA_FSM_SET_EVENT(process_fsm, (*process)->state, INA_PROCESS_START);
    
    if (descriptor->lifecycle == INA_PROCESS_LIFECYCLE_TYPE_MANAGED) {
        if (descriptor->managed_type == INA_PROCESS_MANAGED_TYPE_SCHEDULED_START
            || descriptor->managed_type == INA_PROCESS_MANAGED_TYPE_SCHEDULED_START_STOP) {
                ina_str_t id = ina_str_sprintf("START_%s", ina_str_cstr(descriptor->full_path));
                INA_ASSERT_NOTNULL(descriptor->scheduled_start_pattern);
                if (!INA_SUCCEED(ina_cron_register_function(ctx->cron_ctx, ina_str_cstr(id), 
                    descriptor->scheduled_start_pattern, *process, __ina_process_cron_start_cb))) {
                        return INA_ERR_PUSH_LAST;
                }
        }
        if (descriptor->managed_type == INA_PROCESS_MANAGED_TYPE_SCHEDULED_START_STOP) {
            ina_str_t id = ina_str_sprintf("STOP_%s", ina_str_cstr(descriptor->full_path));
            INA_ASSERT_NOTNULL(descriptor->scheduled_stop_pattern);
            if (!INA_SUCCEED(ina_cron_register_function(ctx->cron_ctx, ina_str_cstr(id), 
                descriptor->scheduled_stop_pattern, *process, __ina_process_cron_stop_cb))) {
                    return INA_ERR_PUSH_LAST;
            }
        }
    }

    HASH_ADD_ULONG(ctx->processes, key, (*process));

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_process_free(ina_process_ctx_t *ctx, ina_process_t **process)
{
    ina_mem_free(*process);
    *process = NULL;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_process_start(ina_process_ctx_t *ctx, ina_process_t *process)
{
    INA_FSM_FIRE_EVENT(process_fsm, process->state, INA_PROCESS_START, process);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_process_stop(ina_process_ctx_t *ctx, ina_process_t *process)
{
    INA_FSM_FIRE_EVENT(process_fsm, process->state, INA_PROCESS_STOP, process);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_process_query_state(ina_process_ctx_t *ctx, ina_process_t *process, ina_fsm_state_t *state)
{
    *state = INA_FSM_GET_STATE(process_fsm, process->state);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_process_reset(ina_process_ctx_t *ctx, ina_process_t *process)
{
    INA_FSM_FIRE_EVENT(process_fsm, process->state, INA_PROCESS_RESET, process);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_process_get_exit_code(ina_process_ctx_t *ctx, ina_process_t *process, int *exit_code)
{
    *exit_code = process->exit_code;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_process_should_be_running(ina_process_ctx_t *ctx, ina_process_t *p, int *should_be_running)
{
    time_t curr_time_sec;
    long curr_time_micros;
    
    if (!INA_SUCCEED(ina_time_read_sys_clock(ctx->systime))) {
        return INA_ERR_PUSH_LAST;
    }
    if (!INA_SUCCEED(ina_time_sys_seconds_micros(ctx->systime, 
        &curr_time_sec, &curr_time_micros))) {
        return INA_ERR_PUSH_LAST;
    }

    if (p->descriptor->lifecycle == INA_PROCESS_LIFECYCLE_TYPE_MANAGED) {
        time_t last_start = 0;
        time_t last_stop = 0;
        if (p->descriptor->managed_type == INA_PROCESS_MANAGED_TYPE_SCHEDULED_START_STOP) {
            ina_cron_last_exec_systime(ctx->cron_ctx, p->descriptor->scheduled_start_pattern, curr_time_sec, &last_start);
            ina_cron_last_exec_systime(ctx->cron_ctx, p->descriptor->scheduled_stop_pattern, curr_time_sec, &last_stop);
            if (last_start > 0 && ( curr_time_sec > last_start && last_stop < last_start) ) {
                *should_be_running = 1;
                return INA_SUCCESS;
            }
            else {
                *should_be_running = 0;
                return INA_SUCCESS;
            }
        }
    }

    *should_be_running = 0;
    return INA_FAILURE;
}
