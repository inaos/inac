

---

```C
typedef struct ina_process_ctx_s ina_process_ctx_t;
```
opaque process context

---

```C
typedef struct ina_process_s ina_process_t;
```
opaque process

---

```C
typedef struct ina_process_stat_s ina_process_stat_t;
```
opaque process stat

---

```C
typedef struct ina_process_descriptor_s {
    ina_str_t full_path;
```
Process descriptor

---

```C
INA_FSM_STATES(process_fsm,
    INA_FSM_STATE(INA_PROCESS_STARTABLE),
    INA_FSM_STATE(INA_PROCESS_RUNNING),
    INA_FSM_STATE(INA_PROCESS_STOPPED)
);
```
FSM states

---

```C
INA_FSM_EVENTS(process_fsm,
    INA_FSM_EVENT(INA_PROCESS_START),
    INA_FSM_EVENT(INA_PROCESS_STOP),
    INA_FSM_EVENT(INA_PROCESS_RESET)
);
```
FSM events

---

```C
INA_API(ina_rc_t) ina_process_ctx_new(ina_process_ctx_t **ctx);
```

Creates and initializes an new process context


**Parameters**
 - `ctx`: Where to store the newly created context



**Return**

INA_SUCCESS if all went well



---

```C
INA_API(void) ina_process_ctx_free(ina_process_ctx_t **ctx);
```

Destroy a process context


**Parameters**
 - `ctx`: Process context to free



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_process_descriptor_new(ina_process_ctx_t *ctx, const char *full_path,
                                                        const char *working_dir,
                                                      const char *startup_args,
                                                      time_t stop_wait_time_ms, uint32_t cf,
                                                      ina_process_descriptor_t **descriptor);
```

Creates an new process descriptor.


**Parameters**
 - `ctx`: 
 - `descriptor`:  Where to store the newly created descriptor
 - `full_path`: 
 - `working_dir`: Working directory
 - `startup_args`: Command arguments
stop_wait_time_ms
cf



**Return**

INA_SUCCESS



---

```C
INA_API(void) ina_process_descriptor_free(
                                    ina_process_descriptor_t **descriptor);
```

Destroy a process descriptor.


**Parameters**
 - `descriptor`: Process descriptor to free



**Return**

INA_SUCCESS if all went well



---

```C
INA_API(ina_rc_t) ina_process_exec(ina_process_ctx_t *ctx,
                                   const char *full_path,
                                   const char *startup_args,
                                   ina_process_t **process);
```

Execute and return.


**Parameters**
 - `ctx`: Process context
 - `full_path`: Full path of executable
 - `startup_args`: Startup arguments
 - `process`: Where to store process instance



**Return**

INA_SUCCESS if all went well



---

```C
INA_API(ina_rc_t) ina_process_exec_and_wait(ina_process_ctx_t *ctx,
                                   const char *full_path,
                                   const char *startup_args,
                                   ina_process_t **process);
```

Execute and wait until process ends.


**Parameters**
 - `ctx`: Process context
 - `full_path`: Full path of executable
 - `startup_args`: Startup arguments
 - `process`: Where to store process instance



**Return**

INA_SUCCESS if all went well



---

```C
INA_API(ina_rc_t) ina_process_new(ina_process_ctx_t *ctx,
                                  ina_process_descriptor_t *descriptor, 
                                  ina_process_t **process);
```

Creates a new process.


**Parameters**
 - `ctx`: Process context
descriptor Process descriptor
 - `process`: WHere to store the newly created process



**Return**

INA_SUCCESS if all went well



---

```C
INA_API(void) ina_process_free(ina_process_t **process);
```

Destroy a process


**Parameters**
 - `process`: Process to free



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_process_start(ina_process_t *process);
```

Start a process


**Parameters**
 - `process`: Process to start



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_process_stop(ina_process_t *process);
```

Stop a process


**Parameters**
 - `process`: Process to stop



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_process_query_state(ina_process_t *process,
                                          ina_fsm_state_t *state);
```

Query current process state


**Parameters**
 - `process`: Process to query
 - `state`: Where to store the process state



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_process_should_be_running(ina_process_t *process,
                                                int *should_be_running);
```

Checks if a process should be running.


**Parameters**
 - `process`: Process to query
 - `should_be_running`: Where to store the state



**Return**

INA_SUCCESS if all went well



---

```C
INA_API(ina_rc_t) ina_process_get_exit_code(ina_process_t *process,
                                            int *exit_code);
```

Get the last exit code of a process


**Parameters**
 - `process`: Process to query
 - `exit_code`: Where to store the exit code



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_process_stat_new(ina_process_stat_t **stat,
                                       const char *binary);
```

Creates a binary status


**Parameters**
 - `stat`: Where to store the newly created status structure
 - `binary`: Full path to the executable



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_process_stat_query(ina_process_stat_t *stat);
```

Query process status. Query alive state, command line , memory usage and
number of threads.


**Parameters**
 - `process`: Process to query



**Return**

INA_SUCCESS if all went well



---

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



---

```C
INA_API(ina_rc_t) ina_process_stat_get_cmd(ina_process_stat_t *stat,
                                           const char **cmd);
```

Retrieve cmd line from a process status.


**Parameters**
 - `stat`: Process status
 - `cmd`: Where to store the command line.



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_process_stat_get_memory(ina_process_stat_t *stat,
                                              uint64_t *memory);
```

Retrieve memory usage from a process status.


**Parameters**
 - `stat`: Process stat
 - `memory`: Where to store the memory usage



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_process_stat_get_num_threads(ina_process_stat_t *stat,
                                                   int *num_threads);
```

Retrieve number of threads from a process status.


**Parameters**
 - `stat`: Process status
 - `num_threads`: Where to store number of threads.



**Return**

INA_SUCCESS



---

```C
INA_API(void) ina_process_stat_free(ina_process_stat_t **stat);
```

Destroy a process.


**Parameters**
 - `stat`: Process status to free



**Return**

INA_SUCCESS

