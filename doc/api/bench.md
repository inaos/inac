

---

```C
typedef void (*ina_bench_setup_cb_t)(void*);
```
Setup callback

---

```C
typedef void (*ina_bench_teardown_cb_t)(void*);
```
Teardown callback

---

```C
typedef void (*ina_bench_scale_cb_t)(void*);
```
Scale callback

---

```C
typedef struct ina_bench_benchmark_s {
    const char* bench_name;
```
Benchmark, single series

---

```C
#define INA_BENCH_MAGIC (0xDEADC0DE)
/* Benchmark function name. For internal purpose only. */
#define INA_BENCH_FNAME(bname, sname) __ina_bench_##bname##_##sname##_run
/* Benchmark struct name. For internal purpose only */
#define INA_BENCH_BNAME(bname, sname) __ina_bench_##bname##_##sname

```
Magic. For internal purpose only.

---

```C
#ifdef INA_OS_OSX
#define INA_BENCH_SECTION __attribute__ ((unused,section ("__DATA, .inabench")))
#define INA_BENCH_SECTION_PUSH
#elif INA_OS_WIN32
#pragma section(".inabench", read)
#define INA_BENCH_SECTION
#define INA_BENCH_SECTION_PUSH __declspec(allocate(".inabench"))
#else
#define INA_BENCH_SECTION __attribute__ ((unused,section (".inabench")))
#define INA_BENCH_SECTION_PUSH
#endif

```
Section holding benchmarks

---

```C
#define INA_BENCH_STRUCT(bname, sname, _skip,  __data, __setup,     \
                            __teardown, __series_setup, __series_teardown, __scale,  __iter)                      \
    INA_BENCH_SECTION_PUSH ina_bench_benchmark_t INA_BENCH_BNAME(bname, sname) INA_BENCH_SECTION = {   \
        #bname,                                                              \
        #sname,                                                              \
        INA_BENCH_FNAME(bname, sname),                                       \
        _skip,                                                               \
        __data,                                                              \
        (ina_bench_setup_cb_t)__setup,                                       \
        (ina_bench_teardown_cb_t)__teardown,                                 \
        (ina_bench_setup_cb_t)__series_setup,                                \
        (ina_bench_teardown_cb_t)__series_teardown,                          \
        (ina_bench_scale_cb_t)__scale,                                       \
        __iter,                                                              \
        0,                                                                   \
        INA_BENCH_MAGIC }

```
Benchmark data defines. For internal purpose only

---

```C
#define INA_BENCH_DATA(bname) struct bname##_data
/* Define setup code für a benchmark */
#define INA_BENCH_SETUP(bname)                                               \
    void bname##_setup(struct bname##_data* data)
/* Define teardown code for a benchmark */
#define INA_BENCH_TEARDOWN(bname)                                            \
    void bname##_teardown(struct bname##_data* data)

```
Define data for a benchmark

---

```C
#define INA_BENCH_SCALE(bname)                                             \
    void bname##_scale(struct bname##_data* data)

```
Define scale code for a benchmark

---

```C
#define INA_BENCH_END(bname, sname)                                       \
    void bname##_##sname##_teardown(struct bname##_data* data)

```
Define teardown code for a benchmark

---

```C
#ifdef INA_OS_OSX
#define INA_BSETUP_FNAME(bname) NULL
#define INA_BTEARDOWN_FNAME(bname) NULL
#define INA_BBEGIN_FNAME(bname, sname) NULL
#define INA_BEND_FNAME(bname, sname) NULL
#define INA_BSCALE_FNAME(bname) NULL
#else
#define INA_BSETUP_FNAME(bname) bname##_setup
#define INA_BTEARDOWN_FNAME(bname) bname##_teardown
#define INA_BSCALE_FNAME(bname) bname##_scale
#define INA_BBEGIN_FNAME(bname, sname) bname##_##sname##_setup
#define INA_BEND_FNAME(bname, sname) bname##_##sname##_teardown
#endif
#define INA_BENCH_DECL(bname, sname, iter, _skip)                              \
    static struct bname##_data  __ina_bench_##bname##_data;                    \
    INA_BENCH_SETUP(bname);                                                    \
    INA_BENCH_TEARDOWN(bname);                                                 \
    INA_BENCH_SCALE(bname);                                                    \
    INA_BENCH_BEGIN(bname, sname);                                             \
    INA_BENCH_END(bname, sname);                                               \
    void INA_BENCH_FNAME(bname, sname)(struct bname##_data* data);             \
    INA_BENCH_STRUCT(bname, sname, _skip,  &__ina_bench_##bname##_data,        \
        INA_BSETUP_FNAME(bname), INA_BTEARDOWN_FNAME(bname),                   \
        INA_BBEGIN_FNAME(bname, sname), INA_BEND_FNAME(bname, sname) ,         \
        INA_BSCALE_FNAME(bname),  iter);   \
    void INA_BENCH_FNAME(bname, sname)(struct bname##_data* data)

```
Declare benchmark. For internal purpose only.

---

```C
#define INA_BENCH(bname, sname, iter) INA_BENCH_DECL(bname, sname, iter, 0)
/* Skip a series */
#define INA_BENCH_SKIP(bname, sname, iter) INA_BENCH_DECL(bname, sname, iter, 1)

```
Declare a series

---

```C
int ina_bench_run(void);
```

Run benchmarks.


**Return**

Exit code



---

```C
INA_API(const char*) ina_bench_get_name(void);
```

Returns the name of the current running benchmark


---

```C
INA_API(const char*) ina_bench_get_series_name(void);
```

Returns the name of the current running series


---

```C
INA_API(ina_rc_t) ina_bench_set_scale_label(const char* label);
```

Set the label for the scale


**Parameters**
 - `label`: Label for scale



**Return**

INA_SUCCESS if all went well
INA_ES_ARGUMENT|INA_ERR_INVALID  if label was NULL



---

```C
INA_API(const char*) ina_bench_get_scale_label(void);
```

Returns the current scale label


---

```C
INA_API(ina_rc_t) ina_bench_set_precision(int precision);
```

Set precision for results


**Parameters**
 - `precision`: Precision



**Return**

INA_SUCCESS if all went well
INA_ES_ARGUMENT|INA_ERR_INVALID  if precision was < 0



---

```C
INA_API(int) ina_bench_get_precision(void);
```

Return current precision for results


---

```C
INA_API(ina_rc_t) ina_bench_set_double(double value);
```

Set the value for the current series and iteration.


**Parameters**
 - `value`: Value



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_bench_set_int64(int64_t value);
```

Set the value for the current series and iteration.


**Parameters**
 - `value`: Value



**Return**

INA_SUCCESS



---

```C
INA_API(double) ina_bench_get_double(void);
```

Returns the current value of current series and iteration.


---

```C
INA_API(int64_t) ina_bench_get_int64(void);
```

Returns the current value of current series and iteration.


---

```C
INA_API(ina_rc_t) ina_bench_set_scale(int64_t scale);
```

Set the scale value for the current series and iteration.


**Parameters**
 - `scale`: Scale value



**Return**

INA_SUCCESS



---

```C
INA_API(int64_t) ina_bench_get_scale(void);
```

Returns the scale value of the current series and iteration.


---

```C
INA_API(int) ina_bench_get_iterations(void);
```

Returns the total number of iterations of the current running
series.


---

```C
INA_API(int) ina_bench_get_iteration(void);
```

Returns the current iteration of the running series


---

```C
INA_API(ina_rc_t) ina_bench_stopwatch_start(void);
```

Starts the stopwatch.


**Return**

INA_SUCCESS if all went well



---

```C
INA_API(int64_t) ina_bench_stopwatch_stop(void);
```

Stop the the stopwatch.


**Return**

Number of microseconds elapsed since the last start.

