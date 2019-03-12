/*
 * Copyright INAOS GmbH, Thalwil, 2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#ifndef _LIBINAC_BENCH_H_
#define _LIBINAC_BENCH_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * # Benchmark
 *
 *
 */
#include <libinac/lib.h>

/* Internal: Setup callback */
typedef void (*ina_bench_setup_cb_t)(void*);

/* Internal: Teardown callback */

typedef void (*ina_bench_teardown_cb_t)(void*);
/* Internal: Scale callback */
typedef void (*ina_bench_scale_cb_t)(void*);

/* Internal: Benchmark, single series */
typedef struct ina_bench_benchmark_s {
    const char* bench_name;
    const char* series_name;
    void (*run)();
    int skip;
    void *data;
    ina_bench_setup_cb_t setup;
    ina_bench_teardown_cb_t teardown;
    ina_bench_setup_cb_t series_setup;
    ina_bench_teardown_cb_t series_teardown;
    ina_bench_scale_cb_t scale;
    int32_t pad1;
    int32_t pad2;
    unsigned int magic;

} ina_bench_benchmark_t;

/* Internal: Magic. */
#define INA_BENCH_MAGIC (0xDEADC0DE)

/* Internal: Benchmark function name. */
#define INA_BENCH_FNAME(bname, sname) __ina_bench_##bname##_##sname##_run

/* Internal: Benchmark struct name. */
#define INA_BENCH_BNAME(bname, sname) __ina_bench_##bname##_##sname

/* Internal: Section holding benchmarks */
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

/* Internal: Benchmark data defines. For internal purpose only */
#define INA_BENCH_STRUCT(bname, sname, _skip,  __data, __setup,     \
                            __teardown, __series_setup, __series_teardown, __scale)           \
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
        0,0,                                                                 \
        INA_BENCH_MAGIC }

/*
 * Declare data for a benchmark
 *
 * Parameters
 *  bname  Benchmark name
 */
#define INA_BENCH_DATA(bname) struct bname##_data

/*
 * Declare setup for a benchmark
 *
 * Parameters
 *  bname  Benchmark name
 */
#define INA_BENCH_SETUP(bname)                                               \
    void bname##_setup(struct bname##_data* data)

/*
 * Declare teardown for a benchmark
 *
 * Parameters
 *  bname  Benchmark name
 */
#define INA_BENCH_TEARDOWN(bname)                                            \
    void bname##_teardown(struct bname##_data* data)

/*
 * Declare scale for a benchmark
 *
 * Parameters
 *  bname  Benchmark name
 */
#define INA_BENCH_SCALE(bname)                                               \
    void bname##_scale(struct bname##_data* data)

/* Declare benchmark series setup */
#define INA_BENCH_BEGIN(bname, sname)                                        \
    void bname##_##sname##_setup(struct bname##_data* data)

/*
 * Declare benchmark series teardown
 *
 * Parameters
 *  bname  Benchmark name
 *  sname  Series name
 */
#define INA_BENCH_END(bname, sname)                                          \
    void bname##_##sname##_teardown(struct bname##_data* data)


/* Internal: Declare benchmark.  */
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
#define INA_BENCH_DECL(bname, sname, _skip)                                    \
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
        INA_BSCALE_FNAME(bname));   \
    void INA_BENCH_FNAME(bname, sname)(struct bname##_data* data)

/*
 * Declare a benchmark series
 *
 * Parameters
 *  bname  Benchmark name
 *  sname  Series name
 */
#define INA_BENCH(bname, sname) INA_BENCH_DECL(bname, sname, 0)

/*
 * Declare a benchmark series
 *
 * Parameters
 *  bname  Benchmark name
 *  sname  Series name
 */
#define INA_BENCH_SKIP(bname, sname) INA_BENCH_DECL(bname, sname, 1)

/*
 * Print a diagnostic message to th standard output
 *
 * Parameters
 *  fmt  Message format
 *  ...  Message arguments
 */
#define INA_BENCH_MSG(fmt, ...)      \
    fprintf(stdout,                  \
        "%s:%s : " fmt "\n",         \
        ina_bench_get_name(),        \
        ina_bench_get_series_name(), \
        ##__VA_ARGS__           \
        )

#define INA_BENCH_INIT(scale_label, precision, iterations, repetitions)       \
do {                                                                          \
        ina_bench_set_scale_label((scale_label));                             \
        ina_bench_set_precision((precision));                                 \
        ina_bench_set_iterations((iterations));                               \
        ina_bench_set_repetitions((repetitions));                             \
    } while(0)

/*
 * Run benchmarks.
 *
 * Return
 *  Return 0 if all went well, otherwise the error code is returned.
 */
int ina_bench_run(void);

/*
 * Returns the name of the current running benchmark.
 */
INA_API(const char*) ina_bench_get_name(void);

/*
 * Returns the name of the current running series.
 */
INA_API(const char*) ina_bench_get_series_name(void);

/*
 * Set the label for the scale.
 *
 * Parameters
 *  label  Label for scale
 *
 * Return
 *  - INA_SUCCESS if all went well
 *  - INA_ES_ARGUMENT|INA_ERR_INVALID  if label was NULL
 */
INA_API(ina_rc_t) ina_bench_set_scale_label(const char* label);


/*
 * Returns the current scale label.
 */
INA_API(const char*) ina_bench_get_scale_label(void);

/*
 * Set precision for results.
 *
 * Parameters
 *  precision  Precision
 *
 * Return
 *  - INA_SUCCESS if all went well
 *  - INA_ES_ARGUMENT|INA_ERR_INVALID  if precision was < 0
 */
INA_API(ina_rc_t) ina_bench_set_precision(int precision);

/*
 * Return current precision for results.
 */
INA_API(int) ina_bench_get_precision(void);

/*
 * Set the value for the current series and iteration.
 *
 * Parameters
 *  value  Value
 *
 * Return
 * Returns always INA_SUCCESS
 */
INA_API(ina_rc_t) ina_bench_set_value(double value);

/*
 * Returns the current value of current series and iteration.
 */
INA_API(double) ina_bench_get_value(void);


/*
 * Set the scale value for the current series and repetition.
 *
 * Parameters
 *  scale   Scale value
 *
 * Return
 *  Return always INA_SUCCESS
 */
INA_API(ina_rc_t) ina_bench_set_scale(double scale);

/*
 * Returns the scale value of the current series and repetition.
 */
INA_API(double) ina_bench_get_scale(void);

/*
 * Returns the total number of repetition of the current running series.
 */
INA_API(int) ina_bench_get_repetitions(void);

INA_API(void) ina_bench_set_repetitions(int repetitions);


/*
 * Returns the current repetition of the running series.
 */
INA_API(int) ina_bench_get_repetition(void);

/*
 * Returns the total number of iterations of the current running series.
 */
INA_API(int) ina_bench_get_iterations(void);
INA_API(void) ina_bench_set_iterations(int iteration);


/*
 * Returns the current iteration of the running series.
 */
INA_API(int) ina_bench_get_iteration(void);

/*
 * Checks whenever the current iteration a warm-up iteration.
 */
INA_API(ina_rc_t) ina_bench_is_warmup(void);

/*
 * Starts the stopwatch.
 *
 * Return
 *  - INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_bench_stopwatch_start(void);

/*
 * Stop the the stopwatch.
 *
 * Return
 *  Number of microseconds elapsed since the last start.
 */
INA_API(int64_t) ina_bench_stopwatch_stop(void);

#ifdef __cplusplus
}
#endif
#endif
