/*
 * Copyright INAOS GmbH, Thalwil, 2013-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#ifndef _LIBINAC_CRON_H_
#define _LIBINAC_CRON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <libinac/lib.h>

/* forward decl */
struct ina_cron_ctx_s;

/* opaque cron task */
typedef struct ina_cron_task_s ina_cron_task_t;
/* opaque cron function */
typedef struct ina_cron_func_s ina_cron_func_t;
/* opaque task list iterator */
typedef struct ina_cron_task_itr_s ina_cron_task_itr_t;

/* cron load callback */
typedef ina_rc_t (*ina_cron_load_cb)(struct ina_cron_ctx_s *ctx);
/* cron save callback */
typedef ina_rc_t (*ina_cron_save_cb)(struct ina_cron_ctx_s *ctx,
                                     ina_cron_task_t *task,
                                     int removed);
/* cron execution callback */
typedef ina_rc_t (*ina_cron_func_cb)(struct ina_cron_ctx_s *ctx,
                                     void *user_data);

/* cron context */
typedef struct ina_cron_ctx_s ina_cron_ctx_t;


/*
 * Create and initialize a new cron context
 *
 * Parameters
 *  ctx      Where to store the newly created context
 *  load_cb  If not NULL, tasks will be loaded using this callback
 *  save_cb  If not NULL, tasks can be saved on ina_cron_task_add()
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_cron_ctx_new(ina_cron_ctx_t **ctx,
                                   ina_cron_load_cb load_cb,
                                   ina_cron_save_cb save_cb,
                                   ina_process_ctx_t *process_ctx);

/*
 * Free a cron context. Destroy all registred cron task and cron function.
 *
 * Parameters
 *  ctx  cron context to free
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(void) ina_cron_ctx_free(ina_cron_ctx_t **ctx);

/*
 * Create a new cron task iterator.
 *
 * Parameters
 *  ctx   cron context
 *  iter  where fo store the iterator
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_cron_task_iter_new(ina_cron_ctx_t *ctx,
                                         ina_cron_task_itr_t **iter);

/*
 * Free a task iterator
 *
 * Parameters
 *  iter  Iterator to free
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_cron_task_iter_free(ina_cron_task_itr_t **iter);

/*
 * Get next task
 *
 * Parameters
 *  iter  Task iterator
 *  task  Where to store the task
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_cron_task_next(ina_cron_task_itr_t *iter,
                                     ina_cron_task_t **task);

/*
 * Add a cron task to a context.
 *
 * Parameters
 *  ctx          Context where to add a new task
 *  id           Unique task ID
 *  pattern      Crontab pattern for task execution/reccurance
 *  persistent   Defines whenever save  callback should be called
 *  cmd          Shell command
 *  working_dir  Where to run the shell command
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_cron_task_new(ina_cron_ctx_t *ctx,
                                    const char *id,
                                    const char *pattern,
                                    int persistent,
                                    const char* cmd,
                                    const char* working_dir);

/*
 * Retrieve a task by his ID.
 *
 * Parameters
 *  ctx   Context
 *  id    Unique task ID
 *  task  Where to store the task, contains NULL if task was not found
 *
 * Return
 *  INA_SUCCESS
 *
 * FIXME: Should return INA_EEXISTS if task was not found
 */
INA_API(ina_rc_t) ina_cron_task_by_id(ina_cron_ctx_t *ctx,
                                      const char *id,
                                      ina_cron_task_t **task);

/*
 * Remove a task.
 *
 * Parameters
 *  ctx   Cron context
 *  task  Task to remove
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_cron_task_free(ina_cron_ctx_t *ctx,
                                      ina_cron_task_t **task);

/*
 * Query if a task is running.
 *
 * Parameters
 *  task     Task to check
 *
 * Return
 *  INA_SUCCESS  Running
 *  INA_FAILURE  Not running
 */
INA_API(ina_rc_t) ina_cron_task_is_running(ina_cron_task_t *task);

/*
 * Get the current crontab pattern for a task.
 *
 * Parameters
 *  task     Task
 *  pattern  Where to store task's pattern
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_cron_task_get_pattern(ina_cron_task_t *task,
                                            ina_str_t *pattern);

/*
 * Execute task and call registered cron function at their specified times and
 * suggest the next time this function should be called.
 *
 * Parameters
 *  ctx                  Cron context
 *  now                  Current timestamp
 *  suggested_next_time  Where to store the suggested next time
 *
 * Return
 * INA_SUCCESS
 */
INA_API(ina_rc_t) ina_cron_process(ina_cron_ctx_t *ctx,
                                   time_t now,
                                   int *suggested_next_time);

/*
 * Register a cron callback.
 *
 * Parameters
 *  ctx        Context where to register the callback
 *  id         Unique ID
 *  pattern    Crontab pattern for callback execution/reccurance
 *  user_data  User data used to call the callback
 *  cb         Callback to register
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_cron_register_function(ina_cron_ctx_t *ctx,
                                             const char *id,
                                             const char *pattern,
                                             void *user_data,
                                             ina_cron_func_cb cb);

/*
 * Unregister a previously registered cron callback.
 *
 * Parameters
 *  ctx  Cron context
 *  id   Unique ID
 *
 * Return
 *  INA_SUCCESS
 *
 * FIXME: return INA_EEXISTS if callback was not found
 */
INA_API(ina_rc_t) ina_cron_unregister_function(ina_cron_ctx_t *ctx,
                                               const char *id);

/*
 * Calculate last execution time from a crontap pattern
 *
 * Parameters
 *  ctx             Cron context
 *  pattern         Crontab pattern
 *  now             Current time
 *  last_exec_time  Where to store the last execution time
 *
 * Return
 *  INA_SUCCESS if all went well
 *
 */
INA_API(ina_rc_t) ina_cron_last_exec_systime(ina_cron_ctx_t *ctx,
                                             const char *pattern,
                                             time_t now,
                                             time_t *last_exec_time);

/*
 * Register a pullable 'event'.
 *
 * Parameters
 *  ctx      Context where to store the pullable event
 *  id       Unique ID for the event
 *  pattern  Crontab pattern for event execution/recurrence
 *  key      Where to store event id
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_cron_register_pull(ina_cron_ctx_t *ctx,
                                         const char *id,
                                         const char *pattern,
                                         unsigned long *key);

/*
 * Try for next pullable event.
 *
 * Parameters
 *  ctx   Cron context
 *  key   Where to store the pullable event key
 *
 * Parameters
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_cron_try_pull(ina_cron_ctx_t *ctx, unsigned long *key);

#ifdef __cplusplus
}
#endif

#endif