
```C
#ifndef _LIBINAC_PROCESS_H_
```

Copyright INAOS GmbH, Thalwil, 2013-2018. All rights reserved

This software is the confidential and proprietary information of INAOS GmbH
("Confidential Information"). You shall not disclose such Confidential
Information and shall use it only in accordance with the terms of the
license agreement you entered into with INAOS GmbH.

```C
typedef struct ina_process_ctx_s ina_process_ctx_t;
```
opaque process context
```C
typedef struct ina_process_s ina_process_t;
```
opaque process
```C
typedef struct ina_process_stat_s ina_process_stat_t;
```
opaque process stat
```C
typedef struct ina_process_descriptor_s {
```
Process descriptor
```C
INA_FSM_STATES(process_fsm,
```
FSM states
```C
INA_FSM_EVENTS(process_fsm,
```
FSM events
```C
INA_API(ina_rc_t) ina_process_ctx_new(ina_process_ctx_t **ctx);
```

Creates and initializes an new process context


**Parameters**
 - `ctx`: Where to store the newly created context



**Return**

INA_SUCCESS if all went well


```C
INA_API(void) ina_process_ctx_free(ina_process_ctx_t **ctx);
```

Destroy a process context


**Parameters**
 - `ctx`: Process context to free



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_process_descriptor_new(ina_process_ctx_t *ctx, const char *full_path,
```

Creates an new process descriptor.


**Parameters**
 - `ctx`: 
 - `descriptor`: 
 - `full_path`: 
 - `working_dir`: 
 - `startup_args`: 
stop_wait_time_ms
cf



**Return**

INA_SUCCESS


```C
INA_API(void) ina_process_descriptor_free(
```

Destroy a process descriptor.


**Parameters**
 - `descriptor`: Process descriptor to free



**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_process_exec(ina_process_ctx_t *ctx,
```

Execute and return.


**Parameters**
 - `ctx`: 
 - `full_path`: Full path of executable
 - `startup_args`: Startup arguments
 - `process`: 



**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_process_exec_and_wait(ina_process_ctx_t *ctx,
```

Execute and wait until process ends.


**Parameters**
 - `ctx`: 
 - `full_path`: Full path of executable
 - `startup_args`: Startup arguments
 - `process`: 



**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_process_new(ina_process_ctx_t *ctx,
```

Creates a new process.


**Parameters**
 - `ctx`: 
descriptor Process descriptor
 - `process`: WHere to store the newly created process



**Return**

INA_SUCCESS if all went well


```C
INA_API(void) ina_process_free(ina_process_t **process);
```

Destroy a process


**Parameters**
 - `process`: Process to free



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_process_start(ina_process_t *process);
```

Start a process


**Parameters**
 - `process`: Process to start



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_process_stop(ina_process_t *process);
```

Stop a process


**Parameters**
 - `process`: Process to stop



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_process_query_state(ina_process_t *process,
```

Query current process state


**Parameters**
 - `process`: Process to query
 - `state`: Where to store the process state



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_process_should_be_running(ina_process_t *process,
```

Checks if a process should be running.


**Parameters**
 - `process`: 
 - `should_be_running`: Where to store the state



**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_process_get_exit_code(ina_process_t *process,
```

Get the last exit code of a process


**Parameters**
 - `process`: Process to query
 - `exit_code`: Where to store the exit code



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_process_stat_new(ina_process_stat_t **stat,
```

Creates a binary status


**Parameters**
 - `stat`: Where to store the newly created status structure
 - `binary`: Full path to the executable



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_process_stat_query(ina_process_stat_t *stat);
```

Query process status. Query alive state, command line , memory usage and
number of threads.


**Parameters**
 - `process`: Process to query



**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_process_stat_alive(ina_process_stat_t *stat, int *alive);
```

Retrieve alive state from a process status.


**Parameters**
 - `stat`: Process status
 - `alive`: Where to store the alive state. State is INA_YES if the process
is alive otherwise INA_NO



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_process_stat_get_cmd(ina_process_stat_t *stat,
```

Retrieve cmd line from a process status.


**Parameters**
 - `stat`: Process status
 - `cmd`: Where to store the command line.



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_process_stat_get_memory(ina_process_stat_t *stat,
```

Retrieve memory usage from a process status.


**Parameters**
 - `stat`: Process stat
 - `memory`: Where to store the memory usage



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_process_stat_get_num_threads(ina_process_stat_t *stat,
```

Retrieve number of threads from a process status.


**Parameters**
 - `stat`: 
 - `num_threads`: Where to store number of threads.



**Return**

INA_SUCCESS


```C
INA_API(void) ina_process_stat_free(ina_process_stat_t **stat);
```

Destroy a process.


**Parameters**
 - `stat`: Process status to free



**Return**

INA_SUCCESS

