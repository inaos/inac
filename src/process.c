/*
 * Copyright (c) 2013-2014, INAOS GmbH
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
    ina_mempool_t *mempool;
};

struct ina_process_s {
    unsigned long key;
    int init;
    ina_process_ctx_t *ctx;
    ina_process_descriptor_t *descriptor;
    ina_fsm_status_t state;
    int exit_code;
#ifdef INA_OS_WIN32
    PROCESS_INFORMATION pi;
#else
     pid_t pid;
#endif
    UT_hash_handle hh;
};

static void __ina_process_is_running(ina_process_t *, int*);
static void __ina_process_start(ina_process_t*);
static void __ina_process_stop(ina_process_t*);
static void __ina_process_reset(ina_process_t*);

static void __ina_process_fsm_event_start(ina_process_t*);
static void __ina_process_fsm_event_stop(ina_process_t*);
static void __ina_process_fsm_event_reset(ina_process_t*);
static void __ina_process_fsm_event_error(ina_process_t*);

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


static void __ina_process_fsm_event_start(ina_process_t *process)
{
    INA_ASSERT_NOTNULL(process);
    INA_ASSERT_NOTNULL(process->descriptor);
    __ina_process_start(process);
}

static void __ina_process_fsm_event_stop(ina_process_t *process)
{
    INA_ASSERT_NOTNULL(process);
    INA_ASSERT_NOTNULL(process->descriptor);
    __ina_process_stop(process);
}

static void __ina_process_fsm_event_reset(ina_process_t *process)
{
    INA_ASSERT_NOTNULL(process);
    INA_ASSERT_NOTNULL(process->descriptor);
    __ina_process_reset(process);
}

static void __ina_process_fsm_event_error(ina_process_t *process)
{
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
    *ctx = (ina_process_ctx_t*)ina_mem_alloc(sizeof(ina_process_ctx_t));
    (*ctx)->processes = NULL;
    if (!INA_SUCCEED(ina_time_sys_new(&(*ctx)->systime))) {
        return INA_ERR_PUSH_LAST;
    }
    if (!INA_SUCCEED(ina_cron_init(&(*ctx)->cron_ctx, NULL, NULL))) {
        return INA_ERR_PUSH_LAST;
    }
    if (!INA_SUCCEED(ina_mempool_create(&(*ctx)->mempool, 
                                        4096, 
                                        INA_MEM_DYNAMIC, 
                                        NULL))) {
        return INA_ERR_PUSH_LAST;
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_process_destroy(ina_process_ctx_t **ctx)
{
    ina_process_t *p, *pt;
    HASH_ITER(hh, (*ctx)->processes, p, pt) {
        HASH_DELETE(hh, (*ctx)->processes, p);
        ina_process_free(&p);
    }
    ina_cron_destroy(&(*ctx)->cron_ctx);
    ina_time_sys_free(&(*ctx)->systime);
    ina_mempool_release((*ctx)->mempool, INA_YES);
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
        if (p->init && p->descriptor->lifecycle == 
            INA_PROCESS_LIFECYCLE_TYPE_MANAGED) {
            
            time_t last_start = 0;
            time_t last_stop = 0;
            if (p->descriptor->managed_type == 
                INA_PROCESS_MANAGED_TYPE_SCHEDULED_START_STOP) {
                ina_cron_last_exec_systime(ctx->cron_ctx, 
                                p->descriptor->scheduled_start_pattern, 
                                curr_time_sec, 
                                &last_start);
                ina_cron_last_exec_systime(ctx->cron_ctx, 
                                p->descriptor->scheduled_stop_pattern, 
                                curr_time_sec, 
                                &last_stop);
                if (last_start > 0 && 
                    (curr_time_sec > last_start && last_stop < last_start)) {
                    /* we should be running therefore start */
                    ina_process_start(p);
                }
            }
            p->init = 0;
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
    ina_cron_process(ctx->cron_ctx, curr_time_sec, &suggested_next_time);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_process_descriptor_new(
                              ina_process_ctx_t *ctx,
                              ina_process_descriptor_t **descriptor,
                              const char *full_path,
                              const char *working_dir,
                              ina_process_lifecycle_type_t lifecycle,
                              ina_process_managed_type_t managed_type,
                              const char *scheduled_start_pattern, 
                              const char *scheduled_stop_pattern,
                              time_t stop_wait_time_ms,
                              uint32_t start_flags,
                              ...)
{
    va_list ap;
    const char *arg;

    INA_ASSERT_NOTNULL(ctx);
    INA_ASSERT_NOTNULL(full_path);
    INA_ASSERT_TRUE(strlen(full_path));

    *descriptor = (ina_process_descriptor_t*) ina_mem_alloc( 
                                        sizeof(ina_process_descriptor_t));
    if (*descriptor == NULL) {
        return INA_ERR_PUSH_LAST;
    }
    (*descriptor)->full_path = ina_str_new_fromcstr(full_path);

    if (working_dir) {
        (*descriptor)->working_dir = ina_str_new_fromcstr(working_dir);
    }
    (*descriptor)->lifecycle = lifecycle;
    (*descriptor)->managed_type = managed_type;
    if (scheduled_start_pattern) {
        (*descriptor)->scheduled_start_pattern = ina_str_new_fromcstr(
                                                    scheduled_start_pattern);
    }
    if (scheduled_stop_pattern) {
        (*descriptor)->scheduled_stop_pattern = ina_str_new_fromcstr(
                                                    scheduled_stop_pattern);
    }
    (*descriptor)->stop_wait_time_ms = stop_wait_time_ms;
    (*descriptor)->start_flags = start_flags;

    va_start(ap, start_flags);
    while ((arg = va_arg(ap, const char *))) {
        if ((*descriptor)->startup_args == NULL) {
            (*descriptor)->startup_args = ina_str_new(128);
        }
        (*descriptor)->startup_args = ina_str_catcstr(
                                            (*descriptor)->startup_args,
                                            arg);
        (*descriptor)->startup_args = ina_str_catcstr(
                                            (*descriptor)->startup_args,
                                            " ");
    }
    va_end(ap);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_process_descriptor_free(ina_process_descriptor_t **descriptor)
{
    if (*descriptor != NULL) {

        if ((*descriptor)->c_ref > 0) {
            /* FIXME: speciific error */
            return INA_FAILURE;
        }

        if ((*descriptor)->full_path != NULL) {
            ina_str_free((*descriptor)->full_path);
        }
        if ((*descriptor)->working_dir != NULL) {
            ina_str_free((*descriptor)->working_dir);
        }
        if ((*descriptor)->scheduled_stop_pattern != NULL) {
            ina_str_free((*descriptor)->scheduled_stop_pattern);
        }
        if ((*descriptor)->scheduled_start_pattern != NULL) {
            ina_str_free((*descriptor)->scheduled_start_pattern);
        }
        if ((*descriptor)->startup_args != NULL) {
            ina_str_free((*descriptor)->startup_args);
        }
        ina_mem_free(*descriptor);
        *descriptor = NULL;
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_process_exec(ina_process_ctx_t *ctx, 
                                   const char *full_path,
                                   const char *startup_args,
                                   ina_process_t **process)
{
    INA_ASSERT_NOTNULL(ctx);
    INA_ASSERT_NOTNULL(full_path);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_process_exec_and_wait(ina_process_ctx_t *ctx, 
                                   const char *full_path,
                                   const char *startup_args,
                                   ina_process_t **process)
{
    INA_ASSERT_NOTNULL(ctx);
    INA_ASSERT_NOTNULL(full_path);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_process_new(ina_process_ctx_t *ctx, 
                                  ina_process_descriptor_t *descriptor, 
                                  ina_process_t **process)
{
    ina_mempool_t *mempool = NULL;

    INA_ASSERT_NOTNULL(descriptor);
    INA_ASSERT_NOTNULL(ctx);

    /* Check descriptor is not referenced */
    if (descriptor->c_ref > 0) {
        /* FIXME: specific error code */
        return INA_FAILURE;
    }
    
    *process = ina_mempool_dalloc(ctx->mempool, sizeof(ina_process_t));

    /* copy descriptor if not allocated from context pool */
    /*if (INA_SUCCEED(ina_mempool_getbypointer(descriptor, &mempool)) && 
        mempool == ctx->mempool) {
        (*process)->descriptor = descriptor;
    } else { */
        (*process)->descriptor = (ina_process_descriptor_t*)ina_mempool_dalloc(
                                        ctx->mempool, 
                                        sizeof(ina_process_descriptor_t));
        if ((*process)->descriptor == NULL) {
            *process = NULL;
            return INA_ERR_PUSH_LAST;
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
    //} 
    (*process)->descriptor->c_ref = 1;
    (*process)->exit_code = -1;
    (*process)->key = INA_HASH_STR_TO_SDBM(descriptor->full_path);
    (*process)->init = 1;
    (*process)->state = 0;
    (*process)->ctx = ctx;

    INA_FSM_SET_STATE(process_fsm, (*process)->state, INA_PROCESS_STARTABLE);
    INA_FSM_SET_EVENT(process_fsm, (*process)->state, INA_PROCESS_START);
    
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
                    return INA_ERR_PUSH_LAST;
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
                return INA_ERR_PUSH_LAST;
            }
            ina_str_free(id);
        }
    }

    HASH_ADD_ULONG(ctx->processes, key, (*process));
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_process_free(ina_process_t **process)
{
    INA_ASSERT_NOTNULL(process);

    if (*process != NULL) {
        (*process)->descriptor->c_ref -= 1;
        /* FIXME: reset memory pool here */
        *process = NULL;
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_process_start(ina_process_t *process)
{
    INA_ASSERT_NOTNULL(process);
    INA_FSM_FIRE_EVENT(process_fsm, process->state, INA_PROCESS_START, process);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_process_stop(ina_process_t *process)
{
    INA_ASSERT_NOTNULL(process);
    INA_FSM_FIRE_EVENT(process_fsm, process->state, INA_PROCESS_STOP, process);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_process_query_state(ina_process_t *process, 
                                          ina_fsm_state_t *state)
{
    INA_ASSERT_NOTNULL(process);
    INA_ASSERT_NOTNULL(state);
    *state = INA_FSM_GET_STATE(process_fsm, process->state);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_process_reset(ina_process_t *process)
{
    INA_ASSERT_NOTNULL(process);
    INA_FSM_FIRE_EVENT(process_fsm, process->state, INA_PROCESS_RESET, process);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_process_get_exit_code(ina_process_t *process, 
                                            int *exit_code)
{

    int still_running;
    INA_ASSERT_NOTNULL(process);
    INA_ASSERT_NOTNULL(exit_code);
    __ina_process_is_running(process, &still_running);
    *exit_code = process->exit_code;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_process_should_be_running(ina_process_t *process, 
                                                int *should_be_running)
{
    time_t curr_time_sec;
    long curr_time_micros;

    INA_ASSERT_NOTNULL(process);
    INA_ASSERT_NOTNULL(process->ctx);
    INA_ASSERT_NOTNULL(should_be_running);
    
    if (!INA_SUCCEED(ina_time_read_sys_clock(process->ctx->systime))) {
        return INA_ERR_PUSH_LAST;
    }
    if (!INA_SUCCEED(ina_time_sys_seconds_micros(process->ctx->systime, 
        &curr_time_sec, &curr_time_micros))) {
        return INA_ERR_PUSH_LAST;
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
    return INA_FAILURE;
}


#ifdef INA_OS_WIN32
static void __ina_process_is_running(ina_process_t *process, 
                                     int *still_running)
{
    DWORD ec;
    BOOL ret;
    /* FIME: Error handling */

    *still_running = INA_NO;

    ret = GetExitCodeProcess(process->pi.hProcess, &ec);
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
}

#else
static void __ina_process_is_running(ina_process_t *process, 
                                      int *still_running)
{
    int status;

    *still_running = INA_NO;

    /* FIME: Error handling */
    if (waitpid(process->pid, &status, WNOHANG) > 0) {
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
        /* FIXME: FSM state change */
        perror("fork");
        return;
    }
     
    if (pid == 0) {
        ina_str_t *tokens;
        char* args[16]; /* FIXME */
        size_t n = 0;
        size_t c = 0;
        
        if (process->descriptor->working_dir != NULL) {
            if (chdir(process->descriptor->working_dir) != 0) {
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
        perror("execv()");
        _exit(127);
    } else {
    
        /* Store pid */
        process->pid = pid;

        int status;
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
        kill(process->pid, SIGTERM);
    }
    __ina_process_is_running(process, &still_running);
}

static void __ina_process_reset(ina_process_t *process)
{
    process->pid = 0;
}

#endif
