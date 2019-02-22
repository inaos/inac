

---

```C
typedef struct ina_stopwatch_ts_s {
    ina_time_tsc_t stamp;
```
Stopwatch timestamp

---

```C
typedef struct ina_stopwatch_s ina_stopwatch_t;
```
Stopwatch

---

```C
INA_API(ina_rc_t) ina_stopwatch_new(int id, int max_stamps, ina_stopwatch_t **stopwatch);
```

Creates a new stopwatch.


**Parameters**
 - `stopwatch`: Where to store the created stopwatch
 - `id`: Unique identifier for the stopwatch
 - `max_stamps`: Defines max number of stamps



**Return**

INA_SUCCESS if all went well



---

```C
INA_API(ina_rc_t)  ina_stopwatch_open(int id, ina_stopwatch_t **stopwatch);
```

Open an existing stopwatch.


**Parameters**
 - `stopwatch`: Where to store the stopwatch
 - `id`: Identifier of the stopwatch to open



**Return**

INA_SUCCESS if all went well



---

```C
INA_API(ina_rc_t) ina_stopwatch_read_stamp(ina_stopwatch_t *stopwatch,
                                                int64_t *index,
                                                ina_stopwatch_ts_t **ts);
```

Read a timestamp from a stopwatch


**Parameters**
 - `stopwatch`: Stopwatch
 - `index`: Stamp index to read



**Return**

INA_SUCCESS if all went well



---

```C
INA_API(ina_rc_t) ina_stopwatch_free(ina_stopwatch_t **stopwatch);
```

Destroy a stopwatch.


**Parameters**
 - `stopwatch`: Stopwatch to free



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_stopwatch_started(const ina_stopwatch_t *stopwatch);
```

Check if stopwatch started.


**Parameters**
 - `stopwatch`: Stopwatch to verify



**Return**

INA_SUCCESS  stopwatch is started
INA_FAILURE  stopwatch is stoppen



---

```C
INA_API(ina_rc_t) ina_stopwatch_valid(const ina_stopwatch_t *stopwatch);
```

Check if stopwatch has valid values.


**Parameters**
 - `stopwatch`: Stopwatch to verify



**Return**

INA_SUCCESS  valid
INA_FAILURE  invalid



---

```C
INA_API(ina_rc_t) ina_stopwatch_start(ina_stopwatch_t* stopwatch,
                                           ina_time_tsc_t *start);
```

Start a stopwatch.


**Parameters**
 - `stopwatch`: Stopwatch to start
 - `start`: Start time, NULL for current time.



**Return**

INA_SUCCESS if all went well.



---

```C
INA_API(ina_rc_t) ina_stopwatch_stamp(ina_stopwatch_t* stopwatch,
                                           const char* user_data1,
                                           const char* user_data2);
```

Make a stamp.


**Parameters**
 - `stopwatch`: Stopwatch to stamp
 - `user_data1`: User data to link
 - `user_data2`: User data to link



**Return**

INA_SUCCESS if all went well



---

```C
INA_API(ina_rc_t) ina_stopwatch_stop(ina_stopwatch_t* stopwatch);
```

Stop a stopwatch.


**Parameters**
 - `stopwatch`: Stopwatch to stop.



**Return**

INA_SUCCESS if all went well

