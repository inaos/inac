
```C
#ifndef _LIBINAC_STOPWATCH_H_
```

Copyright INAOS GmbH, Thalwil, 2012-2018. All rights reserved

This software is the confidential and proprietary information of INAOS GmbH
("Confidential Information"). You shall not disclose such Confidential
Information and shall use it only in accordance with the terms of the
license agreement you entered into with INAOS GmbH.

```C
typedef struct ina_stopwatch_ts_s {
```
Stopwatch timestamp
```C
typedef struct ina_stopwatch_s ina_stopwatch_t;
```
Stopwatch
```C
INA_API(ina_rc_t) ina_stopwatch_new(int id, int max_stamps, ina_stopwatch_t **stopwatch);
```

Creates a new stopwatch.


**Parameters**
 - `stopwatch`: Where to store the created stopwatch
 - `id`: 
 - `max_stamps`: Defines max number of stamps



**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t)  ina_stopwatch_open(int id, ina_stopwatch_t **stopwatch);
```

Open an existing stopwatch.


**Parameters**
 - `stopwatch`: Where to store the stopwatch
 - `id`: 



**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_stopwatch_read_stamp(ina_stopwatch_t *stopwatch,
```

Read a timestamp from a stopwatch


**Parameters**
 - `stopwatch`: Stopwatch
 - `index`:  Stamp index to read



**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_stopwatch_free(ina_stopwatch_t **stopwatch);
```

Destroy a stopwatch.


**Parameters**
 - `stopwatch`: Stopwatch to free



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_stopwatch_started(const ina_stopwatch_t *stopwatch);
```

Check if stopwatch started.


**Parameters**
 - `stopwatch`: Stopwatch to verify



**Return**

INA_SUCCESS  stopwatch is started
INA_FAILURE  stopwatch is stoppen


```C
INA_API(ina_rc_t) ina_stopwatch_valid(const ina_stopwatch_t *stopwatch);
```

Check if stopwatch has valid values.


**Parameters**
 - `stopwatch`: Stopwatch to verify



**Return**

INA_SUCCESS  valid
INA_FAILURE  invalid


```C
INA_API(ina_rc_t) ina_stopwatch_start(ina_stopwatch_t* stopwatch,
```

Start a stopwatch.


**Parameters**
 - `stopwatch`: Stopwatch to start
 - `start`:  Start time, NULL for current time.



**Return**

INA_SUCCESS if all went well.


```C
INA_API(ina_rc_t) ina_stopwatch_stamp(ina_stopwatch_t* stopwatch,
```

Make a stamp.


**Parameters**
 - `stopwatch`: Stopwatch to stamp
 - `user_data1`: User data to link
 - `user_data2`: User data to link



**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_stopwatch_stop(ina_stopwatch_t* stopwatch);
```

Stop a stopwatch.


**Parameters**
 - `stopwatch`: Stopwatch to stop.



**Return**

INA_SUCCESS if all went well

