

---

```C
#ifndef _LIBINAC_CRON_H_
#define _LIBINAC_CRON_H_

```

Copyright INAOS GmbH, Thalwil, 2013-2018. All rights reserved

This software is the confidential and proprietary information of INAOS GmbH
("Confidential Information"). You shall not disclose such Confidential
Information and shall use it only in accordance with the terms of the
license agreement you entered into with INAOS GmbH.


---

```C
typedef struct ina_cron_timetable_s {
    char mins[60];     /* 0-59 */
    char hours[24];    /* 0-23 */
    char days[32];     /* 1-31 */
    char mons[12];     /* 0-11 */
    char dow[7];       /* 0-6, beginning sunday */
} ina_cron_timetable_t;
```

Crontab time table


---

```C
typedef struct ina_cron_ctx_s ina_cron_ctx_t;
```
opaque cron context

---

```C
typedef struct ina_cron_event_s ina_cron_event_t;
```
opaque cron task

---

```C
typedef struct ina_cron_event_iter_s ina_cron_event_iter_t;
```
opaque task list iterator

---

```C
typedef ina_rc_t (*ina_cron_load_cb)(ina_cron_ctx_t *ctx);
```
cron load callback

---

```C
typedef ina_rc_t (*ina_cron_save_cb)(const ina_cron_ctx_t *ctx,
                                     const ina_cron_event_t *event);
```
cron save callback

---

```C
typedef ina_rc_t (*ina_cron_push_cb_t)(ina_cron_ctx_t *ctx,
                                       void *user_data);
```
cron execution callback

---

```C
INA_API(ina_rc_t) ina_cron_parse_pattern(const char* pattern,
                                     ina_cron_timetable_t *tt);
```

Parse a cron pattern by filling a given time table


**Parameters**
 - `pattern`: Cron pattern to parse
 - `table`: Time table to fill



**Return**

INA_SUCCESS if all went well



---

```C
INA_API(ina_rc_t) ina_cron_make_pattern(const ina_cron_timetable_t *tt,
                                        ina_str_t *pattern);
```

Create a cron pattern from a given time table.


**Parameters**
 - `table`: Time table
 - `pattern`: Where to store the pattern



**Return**

INA_SUCCESS when all went well



---

```C
INA_API(ina_rc_t) ina_cron_ctx_new(ina_cron_load_cb load_cb,
                                   ina_cron_save_cb save_cb,
                                   ina_cron_ctx_t **ctx);
```

Create and initialize a new cron context


**Parameters**
 - `ctx`: Where to store the newly created context
 - `load_cb`: If not NULL, tasks will be loaded using this callback
 - `save_cb`: If not NULL, tasks can be saved on ina_cron_task_add()



**Return**

INA_SUCCESS if all went well



---

```C
INA_API(void) ina_cron_ctx_free(ina_cron_ctx_t **ctx);
```

Free a cron context. Destroy all registered cron events


**Parameters**
 - `ctx`: cron context to free



**Return**

INA_SUCCESS



---

```C
INA_API(const char*) ina_cron_event_pattern(const ina_cron_event_t *event);
```

Get the current crontab pattern for an event.


**Parameters**
 - `task`: Task
 - `pattern`: Where to store events's pattern



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_cron_event_by_id(ina_cron_ctx_t *ctx,
                                       const char *id,
                                       ina_cron_event_t **event);
```

Retrieve a task by his ID.


**Parameters**
 - `ctx`: Context
 - `id`: Unique task ID
 - `task`: Where to store the task, contains NULL if task was not found



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_cron_event_is_running(const ina_cron_event_t *event);
```

Query if a task is running.


**Parameters**
 - `task`: Task to check



**Return**

INA_SUCCESS  Running
INA_FAILURE  Not running



---

```C
INA_API(ina_rc_t) ina_cron_event_iter_new(ina_cron_ctx_t *ctx,
                                         ina_cron_event_iter_t **iter);
```

Create a new cron task iterator.


**Parameters**
 - `ctx`: cron context
 - `iter`: where fo store the iterator



**Return**

INA_SUCCESS



---

```C
INA_API(void) ina_cron_event_iter_free(ina_cron_event_iter_t **iter);
```

Free a task iterator


**Parameters**
 - `iter`: Iterator to free



---

```C
INA_API(ina_rc_t) ina_cron_event_iter_next(ina_cron_event_iter_t *iter,
                                           ina_cron_event_t **event);
```

Get next task


**Parameters**
 - `iter`: Task iterator
 - `task`: Where to store the task



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_cron_process(ina_cron_ctx_t *ctx,
                                   time_t now,
                                   int *suggested_next_time);
```

Execute task and call registered cron function at their specified times and
suggest the next time this function should be called.


**Parameters**
 - `ctx`: 
 - `now`: 
 - `suggested_next_time`: Where to store the suggested next time



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_cron_try_pull(ina_cron_ctx_t *ctx,
                                    uint32_t *key,
                                    void **user_data);
```

Try for next pullable event.


**Parameters**
 - `ctx`: Cron context
 - `key`: Where to store the pullable event key



**Parameters**
INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_cron_last_exec_systime(ina_cron_ctx_t *ctx,
                                             const ina_cron_timetable_t *tt,
                                             time_t now,
                                             time_t *last_exec_time);
```

Calculate last execution time from a crontab pattern


**Parameters**
 - `ctx`: Cron context
 - `pattern`: Crontab pattern
 - `now`: Current time
 - `last_exec_time`: Where to store the last execution time



**Return**

INA_SUCCESS if all went well


