
```C
#ifndef _LIBINAC_TIME_H_
```

Copyright INAOS GmbH, Thalwil, 2012-2018. All rights reserved

This software is the confidential and proprietary information of INAOS GmbH
("Confidential Information"). You shall not disclose such Confidential
Information and shall use it only in accordance with the terms of the
license agreement you entered into with INAOS GmbH.

```C
typedef struct ina_time_s ina_time_t;
```
Time value - opaque
```C
typedef union ina_time_tsc_value_u {
```
Time Stamp Counter Value
```C
typedef struct ina_time_tsc_s {
```
Time Stamp Counter
```C
typedef struct ina_time_tsc_info_s {
```
TSC time backend information
```C
typedef struct ina_time_sys_info_s {
```
Time backend information
```C
INA_API(ina_rc_t) ina_time_sys_backend_info(ina_time_sys_info_t *info);
```

Get system time backend information.


**Parameters**
 - `info`: Where to store the backend information.



**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_time_tsc_backend_info(ina_time_tsc_info_t *info);
```

Get TSC Time backend information.


**Parameters**
 - `info`: Where to store the backend information.



**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_time_sleep(time_t msec);
```

Sleep for X milliseconds.


**Parameters**
 - `msec`: Number of milliseconds to sleep



**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_time_tsc_new(ina_time_tsc_t **time);
```

Allocate TSC time structure.


**Parameters**
 - `time`: Where to store tsc time



**Return**

INA_SUCCESS


```C
INA_API(void) ina_time_tsc_free(ina_time_tsc_t **time);
```

Free TSC time


**Parameters**
 - `time`: TSC time to free



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_time_sys_new(ina_time_t **time);
```

Allocate system time.


**Parameters**
 - `time`: Where to store the sys time



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_time_sys_free(ina_time_t **time);
```

Free system time.


**Parameters**
 - `time`: System time to free



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_time_tsc_enable_rdtsc(void);
```

RDTSC is required if you do not want the process to
go back and forth to the kernel all the time, this is
even much more efficient then using VDSO (which are good)
but RDTSC is the most efficient way to measure time

Two conditions must apply if one wants to use RDTSC:
- Invariant TSC is supported first (can be checked through
the CPU flag `constant_tsc`
- Time-stamping thread is bound to a single CPU, which
means one must first pin the process to a particular CPU

Enabling RDTSC is on process scope


**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_time_tsc_disable_rdtsc(void);
```

Disable RDTSC.


**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_time_read_tsc_clock(ina_time_tsc_t* time);
```

Read the Time Stamp Counter.


**Parameters**
 - `time`: Where to store the TSC.



**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_time_read_sys_clock(ina_time_t* time);
```

Read the System-Clock.


**Parameters**
 - `time`: Were to store the system time



**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_time_tsc_seconds_nanos(const ina_time_tsc_t* time,
```

Read the second and nanosecond part of the TSC.


**Parameters**
 - `time`: Input TSC
 - `secs`: Where to store the seconds
 - `nanos`: Where to store the nanoseconds



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_time_sys_seconds_micros(const ina_time_t* time,
```

Convert the ina_time_t to a UNIX timestamp and microseconds.


**Parameters**
 - `time`: Input time
 - `secs`: Where to store the seconds
 - `micros`: Where to store the microseconds



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_time_strftime(ina_str_t buf,
```

Basically strftime.


**Parameters**
 - `buf`:  Output buffer
 - `buflen`: Size of output buffer
 - `written`: Number of bytes written
 - `fmt`:  String format
 - `time`: Time



**Return**

INA_SUCCESS if all wen well


```C
INA_API(ina_rc_t) ina_time_strptime(ina_str_t input,
```

Basically POSIX strptime, with a workaround for windows.


**Parameters**
 - `buf`:  Output buffer
 - `buflen`: Size of output buffer
 - `written`: Number of bytes written
 - `fmt`:  String format
 - `time`: Time



**Return**

INA_SUCCESS if all wen well


```C
INA_API(ina_rc_t) ina_time_tsc_strftime(ina_str_t buf,
```

Basically strftime but using TSC.


**Parameters**
 - `buf`: 
 - `fmt`: 
 - `time`: 
 - `show_nanos`: Defines whenever append nanos to the output


```C
INA_API(ina_rc_t) ina_time_tsc_millis(ina_time_tsc_t *tsc, time_t *now_millis);
```

Convert the ina_time_tsc_t to a millisecond timestamp since epoch.


**Parameters**
 - `tsc`: 
 - `now_millis`: Where to store milliseconds since epoch.



**Return**

INA_SUCCESS

