/*
 * Copyright INAOS GmbH, Thalwil, 2013-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#ifndef _LIBINAC_PROCESS_H_
#define _LIBINAC_PROCESS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <libinac/lib.h>

#define INA_PROCESS_CF_DEFAULT       (0UL)
#define INA_PROCESS_CF_CHILD_PROCESS (1UL)
#define INA_PROCESS_CF_CONSOLE       (2UL)
#define INA_PROCESS_CF_WAIT          (4UL)


/* opaque process context */
typedef struct ina_process_ctx_s ina_process_ctx_t;

/* opaque process */
typedef struct ina_process_s ina_process_t;

/* opaque process stat */
typedef struct ina_process_stat_s ina_process_stat_t;

/* Process descriptor */
typedef struct ina_process_descriptor_s {
    ina_str_t full_path;
    ina_str_t working_dir;
    ina_str_t startup_args;
    time_t stop_wait_time_ms;
    uint32_t cf;
    uint32_t c_ref;
} ina_process_descriptor_t;

/* FSM states */
INA_FSM_STATES(process_fsm, 
    INA_FSM_STATE(INA_PROCESS_STARTABLE),
    INA_FSM_STATE(INA_PROCESS_RUNNING),
    INA_FSM_STATE(INA_PROCESS_STOPPED)
);

/* FSM events */
INA_FSM_EVENTS(process_fsm, 
    INA_FSM_EVENT(INA_PROCESS_START),
    INA_FSM_EVENT(INA_PROCESS_STOP),
    INA_FSM_EVENT(INA_PROCESS_RESET)
);

/*
 * Creates and initializes an new process context
 *
 * Parameters
 *  ctx  Where to store the newly created context
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_process_ctx_new(ina_process_ctx_t **ctx);

/*
 *  Destroy a process context
 *
 *  Parameters
 *   ctx   Process context to free
 *
 *  Return
 *   INA_SUCCESS
 */
INA_API(void) ina_process_ctx_free(ina_process_ctx_t **ctx);

/*
 * Creates an new process descriptor.
 *
 * Parameters
 *  ctx                      Process context
 *  descriptor               Where to store the newly created descriptor
 *  full_path                Full path of executable
 *  working_dir              Working directory
 *  startup_args             Command arguments
 *  stop_wait_time_ms
 *  cf
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_process_descriptor_new(ina_process_ctx_t *ctx, const char *full_path,
                                                        const char *working_dir,
                                                      const char *startup_args,
                                                      time_t stop_wait_time_ms, uint32_t cf,
                                                      ina_process_descriptor_t **descriptor);

/*
 * Destroy a process descriptor.
 *
 * Parameters
 *  descriptor  Process descriptor to free
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(void) ina_process_descriptor_free(
                                    ina_process_descriptor_t **descriptor);

/*
 * Execute and return.
 *
 * Parameters
 *  ctx           Process context
 *  full_path     Full path of executable
 *  startup_args  Startup arguments
 *  process       Where to store process instance
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_process_exec(ina_process_ctx_t *ctx, 
                                   const char *full_path,
                                   const char *startup_args,
                                   ina_process_t **process);
/*
 * Execute and wait until process ends.
 *
 * Parameters
 *  ctx           Process context
 *  full_path     Full path of executable
 *  startup_args  Startup arguments
 *  process       Where to store process instance
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_process_exec_and_wait(ina_process_ctx_t *ctx, 
                                   const char *full_path,
                                   const char *startup_args,
                                   ina_process_t **process);

/*
 * Creates a new process.
 *
 * Parameters
 *  ctx        Process context
 *  descriptor Process descriptor
 *  process    WHere to store the newly created process
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_process_new(ina_process_ctx_t *ctx, 
                                  ina_process_descriptor_t *descriptor, 
                                  ina_process_t **process);

/*
 * Destroy a process
 *
 * Parameters
 *  process   Process to free
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(void) ina_process_free(ina_process_t **process);

/*
 * Start a process
 *
 * Parameters
 *  process  Process to start
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_process_start(ina_process_t *process);

/*
 * Stop a process
 *
 * Parameters
 *  process  Process to stop
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_process_stop(ina_process_t *process);

/*
 * Query current process state
 *
 * Parameters
 *  process  Process to query
 *  state    Where to store the process state
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_process_query_state(ina_process_t *process, 
                                          ina_fsm_state_t *state);
/*
 * Checks if a process should be running.
 *
 * Parameters
 *  process            Process to query
 *  should_be_running  Where to store the state
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_process_should_be_running(ina_process_t *process, 
                                                int *should_be_running);
/*
 * Get the last exit code of a process
 *
 * Parameters
 *  process    Process to query
 *  exit_code  Where to store the exit code
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_process_get_exit_code(ina_process_t *process, 
                                            int *exit_code);

/*
 * Creates a binary status
 *
 * Parameters
 *  stat    Where to store the newly created status structure
 *  binary  Full path to the executable
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_process_stat_new(const char *binary,
                                       ina_process_stat_t **stat);

/*
 * Query process status. Query alive state, command line , memory usage and
 * number of threads.
 *
 * Parameters
 *  process  Process to query
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_process_stat_query(ina_process_stat_t *stat);

/*
 * Retrieve alive state from a process status.
 *
 * Parameters
 *  stat   Process status
 *  alive  Where to store the alive state. State is INA_YES if the process
 *         is alive otherwise INA_NO
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_process_stat_alive(ina_process_stat_t *stat, int *alive);

/*
 * Retrieve cmd line from a process status.
 *
 * Parameters
 *  stat  Process status
 *  cmd   Where to store the command line.
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_process_stat_get_cmd(ina_process_stat_t *stat,
                                           const char **cmd);

/*
 * Retrieve memory usage from a process status.
 *
 * Parameters
 *  stat    Process stat
 *  memory  Where to store the memory usage
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_process_stat_get_memory(ina_process_stat_t *stat,
                                              uint64_t *memory);

/*
 * Retrieve number of threads from a process status.
 *
 * Parameters
 *  stat         Process status
 *  num_threads  Where to store number of threads.
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_process_stat_get_num_threads(ina_process_stat_t *stat,
                                                   int *num_threads);

/*
 * Destroy a process.
 *
 * Parameters
 *  stat  Process status to free
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(void) ina_process_stat_free(ina_process_stat_t **stat);

#ifdef __cplusplus
}
#endif

#endif
