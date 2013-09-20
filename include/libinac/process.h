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
#ifndef _LIBINAC_PROCESS_H_
#define _LIBINAC_PROCESS_H_

#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define INA_PROCESS_FLAGS_CHILD_PROCESS  1
#define INA_PROCESS_FLAGS_CONSOLE        2

/* opaque process context */
typedef struct ina_process_ctx_s ina_process_ctx_t;

/* opaque process */
typedef struct ina_process_s ina_process_t;

typedef enum ina_process_managed_type_e {
    INA_PROCESS_MANAGED_TYPE_SCHEDULED_START,
    INA_PROCESS_MANAGED_TYPE_SCHEDULED_START_STOP,
    INA_PROCESS_MANAGED_TYPE_PARENT_LIFETIME
} ina_process_managed_type_t;

typedef enum ina_process_lifecycle_type_e {
    INA_PROCESS_LIFECYCLE_TYPE_FIRE_AND_FORGET,
    INA_PROCESS_LIFECYCLE_TYPE_MANAGED,
} ina_process_lifecycle_type_t;

typedef struct ina_process_descriptor_s {
    ina_str_t full_path;
    ina_str_t working_dir;
    ina_str_t startup_args;
    ina_process_lifecycle_type_t lifecycle;
    ina_process_managed_type_t managed_type;
    ina_str_t scheduled_start_pattern;
    ina_str_t scheduled_stop_pattern;
    time_t stop_wait_time_ms;
    int start_flags;
} ina_process_descriptor_t;

INA_FSM_STATES(process_fsm, 
    INA_FSM_STATE(INA_PROCESS_STARTABLE),
    INA_FSM_STATE(INA_PROCESS_RUNNING),
    INA_FSM_STATE(INA_PROCESS_STOPPED)
);

INA_FSM_EVENTS(process_fsm, 
    INA_FSM_EVENT(INA_PROCESS_START),
    INA_FSM_EVENT(INA_PROCESS_STOP),
    INA_FSM_EVENT(INA_PROCESS_RESET)
);

/*
 * 
 */
INA_API(ina_rc_t) ina_process_init(ina_process_ctx_t **ctx);
/*
 * 
 */
INA_API(ina_rc_t) ina_process_destroy(ina_process_ctx_t **ctx);
/*
 * 
 */
INA_API(ina_rc_t) ina_process_manage(ina_process_ctx_t *ctx);
/*
 * 
 */
INA_API(ina_rc_t) ina_process_new(ina_process_ctx_t *ctx, ina_process_descriptor_t *descriptor, ina_process_t **process);
/*
 * 
 */
INA_API(ina_rc_t) ina_process_free(ina_process_ctx_t *ctx, ina_process_t **process);
/*
 * 
 */
INA_API(ina_rc_t) ina_process_start(ina_process_ctx_t *ctx, ina_process_t *process);
/*
 * 
 */
INA_API(ina_rc_t) ina_process_stop(ina_process_ctx_t *ctx, ina_process_t *process);
/*
 * 
 */
INA_API(ina_rc_t) ina_process_query_state(ina_process_ctx_t *ctx, ina_process_t *process, ina_fsm_state_t *state);
/*
 * 
 */
INA_API(ina_rc_t) ina_process_should_be_running(ina_process_ctx_t *ctx, ina_process_t *process, int *should_be_running);
/*
 * 
 */
INA_API(ina_rc_t) ina_process_next_state(ina_process_ctx_t *ctx, ina_process_t *process);
/*
 * 
 */
INA_API(ina_rc_t) ina_process_get_exit_code(ina_process_ctx_t *ctx, ina_process_t *process, int *exit_code);


#ifdef __cplusplus
}
#endif

#endif
