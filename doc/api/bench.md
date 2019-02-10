
```C
#ifndef _LIBINAC_BENCH_H_
```

Copyright INAOS GmbH, Thalwil, 2018. All rights reserved

This software is the confidential and proprietary information of INAOS GmbH
("Confidential Information"). You shall not disclose such Confidential
Information and shall use it only in accordance with the terms of the
license agreement you entered into with INAOS GmbH.

```C
typedef void (*ina_bench_setup_cb_t)(void*);/* Teardown callback */
```
Setup callback
```C
typedef void (*ina_bench_scale_cb_t)(void*);
```
Scale callback
```C
typedef struct ina_bench_benchmark_s {
```
Benchmark, single series
```C
#define INA_BENCH_MAGIC (0xDEADC0DE)
```
Magic. For internal purpose only.
```C
#define INA_BENCH_FNAME(bname, sname) __ina_bench_##bname##_##sname##_run
```
Benchmark function name. For internal purpose only.
```C
#define INA_BENCH_BNAME(bname, sname) __ina_bench_##bname##_##sname
```
Benchmark struct name. For internal purpose only
```C
#ifdef INA_OS_OSX
```
Section holding benchmarks
```C
#define INA_BENCH_STRUCT(bname, sname, _skip,  __data, __setup,     \
```
Benchmark data defines. For internal purpose only
```C
#define INA_BENCH_DATA(bname) struct bname##_data
```
Define data for a benchmark
```C
#define INA_BENCH_SETUP(bname)                                               \
```
Define setup code für a benchmark
```C
#define INA_BENCH_TEARDOWN(bname)                                            \
```
Define teardown code for a benchmark
```C
#define INA_BENCH_SCALE(bname)                                             \
```
Define scale code for a benchmark
```C
#define INA_BENCH_END(bname, sname)                                       \
```
Define teardown code for a benchmark
```C
#ifdef INA_OS_OSX
```
Declare benchmark. For internal purpose only.
```C
#define INA_BENCH(bname, sname, iter) INA_BENCH_DECL(bname, sname, iter, 0)
```
Declare a series
```C
#define INA_BENCH_SKIP(bname, sname, iter) INA_BENCH_DECL(bname, sname, iter, 1)
```
Skip a series
```C
int ina_bench_run(int argc, char *argv[]);
```

Run benchmarks.


**Parameters**
 - `argc`: Argument count
 - `argv`: Array of arguments



**Return**

Exit code


```C
INA_API(const char*) ina_bench_get_name(void);
```

Returns the name of the current running benchmark

```C
INA_API(const char*) ina_bench_get_series_name(void);
```

Returns the name of the current running series

```C
INA_API(ina_rc_t) ina_bench_set_scale_label(const char* label);
```

Set the label for the scale


**Parameters**
 - `label`: Label for scale



**Return**

INA_SUCCESS if all went well
INA_ES_ARGUMENT|INA_ERR_INVALID  if label was NULL


```C
INA_API(const char*) ina_bench_get_scale_label(void);
```

Returns the current scale label

```C
INA_API(ina_rc_t) ina_bench_set_precision(int precision);
```

Set precision for results


**Parameters**
 - `precision`: Precision



**Return**

INA_SUCCESS if all went well
INA_ES_ARGUMENT|INA_ERR_INVALID  if precision was < 0


```C
INA_API(int) ina_bench_get_precision(void);
```

Return current precision for results

```C
INA_API(ina_rc_t) ina_bench_set_double(double value);
```

Set the value for the current series and iteration.


**Parameters**
 - `value`: Value



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_bench_set_int64(int64_t value);
```

Set the value for the current series and iteration.


**Parameters**
 - `value`: Value



**Return**

INA_SUCCESS


```C
INA_API(double) ina_bench_get_double(void);
```

Returns the current value of current series and iteration.

```C
INA_API(int64_t) ina_bench_get_int64(void);
```

Returns the current value of current series and iteration.

```C
INA_API(ina_rc_t) ina_bench_set_scale(int64_t scale);
```

Set the scale value for the current series and iteration.


**Parameters**
 - `scale`: Scale value



**Return**

INA_SUCCESS


```C
INA_API(int64_t) ina_bench_get_scale(void);
```

Returns the scale value of the current series and iteration.

```C
INA_API(int) ina_bench_get_iterations(void);
```

Returns the total number of iterations of the current running
series.

```C
INA_API(int) ina_bench_get_iteration(void);
```

Returns the current iteration of the running series

```C
INA_API(ina_rc_t) ina_bench_stopwatch_start(void);
```

Starts the stopwatch.


**Return**

INA_SUCCESS if all went well


```C
INA_API(int64_t) ina_bench_stopwatch_stop(void);
```

Stop the the stopwatch.


**Return**

Number of microseconds elapsed since the last start.

