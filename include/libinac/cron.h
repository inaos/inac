/*
 * Copyright 2013-2020 INAOS GmbH, Thalwil
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _LIBINAC_CRON_H_
#define _LIBINAC_CRON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <libinac/lib.h>


#define INA_CRON_CF_EXEC    (1)
#define INA_CRON_CF_PUSH    (2)
#define INA_CRON_CF_PULL    (4)
#define INA_CRON_CF_PERSIST (8)

/*
 * Crontab time table
 */
typedef struct ina_cron_timetable_s {
    char mins[60];     /* 0-59 */
    char hours[24];    /* 0-23 */
    char days[32];     /* 1-31 */
    char mons[12];     /* 0-11 */
    char dow[7];       /* 0-6, beginning sunday */
} ina_cron_timetable_t;


/* opaque cron context */
typedef struct ina_cron_ctx_s ina_cron_ctx_t;

/* opaque cron task */
typedef struct ina_cron_event_s ina_cron_event_t;

/* opaque task list iterator */
typedef struct ina_cron_event_iter_s ina_cron_event_iter_t;

/* cron load callback */
typedef ina_rc_t (*ina_cron_load_cb)(ina_cron_ctx_t *ctx);
/* cron save callback */
typedef ina_rc_t (*ina_cron_save_cb)(const ina_cron_ctx_t *ctx,
                                     const ina_cron_event_t *event);

/* cron execution callback */
typedef ina_rc_t (*ina_cron_push_cb_t)(ina_cron_ctx_t *ctx,
                                       void *user_data);

/*
 * Parse a cron pattern by filling a given time table
 *
 * Parameters
 *  pattern   Cron pattern to parse
 *  table     Time table to fill
 *
 *  Return
 *   INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_cron_parse_pattern(const char* pattern,
                                     ina_cron_timetable_t *tt);

/*
 * Create a cron pattern from a given time table.
 *
 * Parameters
 *  table    Time table
 *  pattern  Where to store the pattern
 *
 *  Return
 *   INA_SUCCESS when all went well
 */
INA_API(ina_rc_t) ina_cron_make_pattern(const ina_cron_timetable_t *tt,
                                        ina_str_t *pattern);
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
INA_API(ina_rc_t) ina_cron_ctx_new(ina_cron_load_cb load_cb,
                                   ina_cron_save_cb save_cb,
                                   ina_cron_ctx_t **ctx);

/*
 * Free a cron context. Destroy all registered cron events
 *
 * Parameters
 *  ctx  cron context to free
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(void) ina_cron_ctx_free(ina_cron_ctx_t **ctx);


INA_API(ina_rc_t) ina_cron_event_new(ina_cron_ctx_t *ctx,
                                    const char *id,
                                    const char *pattern,
                                    uint32_t cf,
                                    ina_cron_event_t **event);


INA_API(void) ina_cron_event_free(ina_cron_event_t **event);

INA_API(const char*) ina_cron_event_id(const ina_cron_event_t *event);

/*
 * Get the current crontab pattern for an event.
 *
 * Parameters
 *  task     Task
 *  pattern  Where to store event's pattern
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(const char*) ina_cron_event_pattern(const ina_cron_event_t *event);


INA_API(ina_rc_t) ina_cron_event_set_exec_params(ina_cron_event_t *event,
                                                 const char* cmd,
                                                 const char* working_dir);

INA_API(ina_rc_t) ina_cron_event_get_exec_params(const ina_cron_event_t *event,
                                                 ina_str_t *cmd,
                                                 ina_str_t *working_dir);

INA_API(ina_rc_t) ina_cron_event_set_push_params(ina_cron_event_t *event,
                                                 ina_cron_push_cb_t push_cb,
                                                 void *user_data);

INA_API(ina_rc_t) ina_cron_event_get_push_params(const ina_cron_event_t *event,
                                                 ina_cron_push_cb_t *push_cb,
                                                 void **user_data);

INA_API(ina_rc_t) ina_cron_event_set_pull_params(ina_cron_event_t *event,
                                                 uint32_t  key,
                                                 void *user_data);

INA_API(ina_rc_t) ina_cron_event_get_pull_params(const ina_cron_event_t *event,
                                                 uint32_t  *key,
                                                 void **user_data);

INA_API(ina_rc_t) ina_cron_event_check_capability(const ina_cron_event_t *event,
                                                  uint32_t cf);


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
 */
INA_API(ina_rc_t) ina_cron_event_by_id(ina_cron_ctx_t *ctx,
                                       const char *id,
                                       ina_cron_event_t **event);


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
INA_API(ina_rc_t) ina_cron_event_is_running(const ina_cron_event_t *event);

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
INA_API(ina_rc_t) ina_cron_event_iter_new(ina_cron_ctx_t *ctx,
                                         ina_cron_event_iter_t **iter);

/*
 * Free a task iterator
 *
 * Parameters
 *  iter  Iterator to free
 */
INA_API(void) ina_cron_event_iter_free(ina_cron_event_iter_t **iter);

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
INA_API(ina_rc_t) ina_cron_event_iter_next(ina_cron_event_iter_t *iter,
                                           ina_cron_event_t **event);

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
 * Try for next pullable event.
 *
 * Parameters
 *  ctx   Cron context
 *  key   Where to store the pullable event key
 *
 * Parameters
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_cron_try_pull(ina_cron_ctx_t *ctx,
                                    uint32_t *key,
                                    void **user_data);

/*
 * Calculate last execution time from a crontab pattern
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
                                             const ina_cron_timetable_t *tt,
                                             time_t now,
                                             time_t *last_exec_time);



#ifdef __cplusplus
}
#endif

#endif