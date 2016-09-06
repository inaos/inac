/*
 * Copyright (c) 2013-2016, INAOS GmbH
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
#ifndef _LIBINAC_CRON_H_
#define _LIBINAC_CRON_H_

#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

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
                                     ina_cron_task_t *task);
/* cron execution callback */
typedef ina_rc_t (*ina_cron_func_cb)(struct ina_cron_ctx_s *ctx,
                                     void *user_data);

/* cron context */
typedef struct ina_cron_ctx_s {
    ina_cron_load_cb load_cb;     /* load callback */
    ina_cron_save_cb save_cb;     /* save callback */
    void *data;                   /* user data attached per context */
    ina_cron_task_t *task_head;   /* first task */
    ina_cron_func_t *func_head;   /* first cron function */
    time_t t1;                    /* ? */
    time_t t2;                    /* ? */
    short stime;                  /* ? */
} ina_cron_ctx_t;


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
INA_API(ina_rc_t) ina_cron_init(ina_cron_ctx_t **ctx,
                                ina_cron_load_cb load_cb,
                                ina_cron_save_cb save_cb);

/*
 * Free a cron context. Destroy all registred cron task and cron function.
 *
 * Parameters
 *  ctx  cron context to free
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_cron_destroy(ina_cron_ctx_t **ctx);

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
INA_API(ina_rc_t) ina_cron_task_new_iter(ina_cron_ctx_t *ctx,
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
INA_API(ina_rc_t) ina_cron_task_free_iter(ina_cron_task_itr_t **iter);

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
 *  cmd          Shell command
 *  working_dir  Where to run the shell command
 *
 * Return
 *  INA_SUCCESS if all went well
 *
 * FIXME: Use const char* for cmd and working_dir
 */
INA_API(ina_rc_t) ina_cron_task_add(ina_cron_ctx_t *ctx,
                                    const char *id,
                                    const char *pattern,
                                    int persistent,
                                    ina_str_t cmd,
                                    ina_str_t working_dir);

/*
 * Retrieve a task by his ID.
 *
 * Parameters
 *  ctx   Context
 *  id    Unique task ID
 *  task  WHere to store the task, contains NULL if task was not found
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
 * Query if a task is running.
 *
 * Parameters
 *  task     Task to check
 *  running  Where to store the running state
 *
 * Return
 *  INA_SUCCESS
 *
 * FIXME: Should return INA_FAILURE if a tast is not runnung. Eliminate the
 *        running function parameter.
 */
INA_API(ina_rc_t) ina_cron_task_is_running(ina_cron_task_t *task, int *running);

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
 * FIXME: Use cons char* instead of ina_str_t for argument pattern.
 */
INA_API(ina_rc_t) ina_cron_last_exec_systime(ina_cron_ctx_t *ctx,
                                             ina_str_t pattern,
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