/*
 * Copyright INAOS GmbH, Thalwil, 2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>
#include <setjmp.h>
#include "config.h"

#ifdef INA_OS_OSX
#include <dlfcn.h>
#endif

#define __INA_MAX_SERIES 64
#define __INA_MAX_HEADER_LENGTH 4094

typedef int (*ina_bench_filter_fn_t)(ina_bench_benchmark_t*);

static ina_str_t __bench_name = NULL;
static ina_bench_benchmark_t *__current = NULL;
static ina_str_t __scale_label = NULL;
static ina_time_tsc_t *__time1 = NULL;
static ina_time_tsc_t *__time2 = NULL;
static int64_t *__scales = NULL;
static double *__results = NULL;
static double *__current_result = NULL;
static int64_t *__current_scale = NULL;
static int __current_iteration = 0;
static int __current_repetition = 0;
static int __current_series = 0;
static char __header[__INA_MAX_HEADER_LENGTH];
static int __precision = 5;


INA_BENCH_DATA(bench) {
    int dummy;
};

INA_BENCH_SETUP(bench) { INA_UNUSED(data); }
INA_BENCH_TEARDOWN(bench) { INA_UNUSED(data); }
INA_BENCH_SCALE(bench) { INA_UNUSED(data); }
INA_BENCH_BEGIN(bench, series) { INA_UNUSED(data); }
INA_BENCH_END(bench , series) { INA_UNUSED(data); }
INA_BENCH(bench, series, 0, 0) { INA_UNUSED(data); }

static int __ina_bench_all(ina_bench_benchmark_t* b) {
    INA_UNUSED(b);
    return 1;
}

static int __ina_bench_filter(ina_bench_benchmark_t* b) {
    return (strncmp(__bench_name, b->bench_name, ina_str_len(__bench_name)) == 0);
}


#ifdef INA_OS_OSX
static void *__ina_find_symbol(ina_bench_benchmark_t *bench, const char *fname)
{
    size_t len = strlen(bench->bench_name) + 1 + strlen(fname);
    char *symbol_name = (char *) malloc(len + 1);
    memset(symbol_name, 0, len + 1);
    snprintf(symbol_name, len + 1, "%s_%s", bench->bench_name, fname);
    void *symbol = dlsym(RTLD_DEFAULT, symbol_name);
    if (!symbol) {
        INA_OS_ERROR(INA_ES_FUNCTION|INA_ERR_NOT_FOUND);
    }
    free(symbol_name);
    return symbol;
}
static void *__ina_find_symbol2(ina_bench_benchmark_t *bench, const char *fname)
{
    size_t len = strlen(bench->bench_name) + 1 + strlen(bench->series_name) + 1 + strlen(fname);
    char *symbol_name = (char *) malloc(len + 1);
    memset(symbol_name, 0, len + 1);
    snprintf(symbol_name, len + 1, "%s_%s_%s", bench->bench_name, bench->series_name, fname);
    void *symbol = dlsym(RTLD_DEFAULT, symbol_name);
    if (!symbol) {
        INA_OS_ERROR(INA_ES_FUNCTION|INA_ERR_NOT_FOUND);
    }
    free(symbol_name);
    return symbol;
}
static ina_rc_t __ina_find_symbols(ina_bench_benchmark_t *bench)
{
    if (!bench->setup) {
        bench->setup = __ina_find_symbol(bench, "setup");
        INA_RETURN_IF_NULL(bench->setup);
    }
    if (!bench->teardown) {
        bench->teardown = __ina_find_symbol(bench, "teardown");
        INA_RETURN_IF_NULL(bench->teardown);
    }
    if (!bench->scale) {
        bench->scale = __ina_find_symbol(bench, "scale");
        INA_RETURN_IF_NULL(bench->scale);
    }
    if (!bench->series_setup) {
        bench->series_setup = __ina_find_symbol2(bench, "setup");
        INA_RETURN_IF_NULL(bench->series_setup);
    }
    if (!bench->series_teardown) {
        bench->series_teardown = __ina_find_symbol2(bench, "teardown");
        INA_RETURN_IF_NULL(bench->series_setup);
    }
    return INA_SUCCESS;
}
#endif

static ina_rc_t __ina_write_report(int xrepeat, int xiter, int num_series, const char* report_path)
{
    FILE* f;
    ina_str_t file_path;
    double *result;
    int64_t *scale;
    int i;
    int j;
    int k;
    char fmt[20];
    snprintf(fmt, 19, ",%%.%df", __precision);

    if (report_path != NULL) {
        file_path = ina_str_sprintf("%s%cbench_%s.csv", report_path, INA_PATH_SEPARATOR,  __current->bench_name);
    } else {
        file_path = ina_str_sprintf("bench_%s.csv", __current->bench_name);
    }
    INA_ASSERT_NOT_NULL(file_path);

    f = fopen(ina_str_cstr(file_path), "w");
    if (f == NULL) {
        ina_str_free(file_path);
        return INA_OS_ERROR(INA_ES_FILE|INA_ERR_NOT_OPEN);
    }
    ina_str_free(file_path);

    if (ina_bench_get_scale_label() != NULL) {
        fprintf(f, "r,%s,%s\n", ina_bench_get_scale_label(), __header);
    } else {
        fprintf(f, "r,scale,%s\n", __header);
    }
    result = __results;
    scale = __scales;
    for (k = 0; k < xrepeat; ++k) {
        for (j = 0; j < xiter; ++j) {
            fprintf(f, "%d,%"INA_INT64_T_FMT, (k+1),scale[k]);
            for (i = 0; i < num_series; ++i) {
                fprintf(f, fmt, result[k* i * xiter + j]);
            }
            fprintf(f, "\n");
        }
    }
    fclose(f);
    return INA_SUCCESS;
}

static void __ina_clear_cache(size_t size)
{
    size_t n;
    unsigned char *p = ina_mem_alloc(size);
    for (n = 0; n < size; ++n) {
        /*p[n] = (unsigned char)(rand()%255);*/
        p[n] = (unsigned char)((n%254)+1);
    }
    ina_mem_free(p);
}

INA_API(int) ina_bench_run(void)
{
    static int total = 0;
    static ina_bench_filter_fn_t filter = __ina_bench_all;
    static ina_bench_benchmark_t* bench;
    ina_bench_benchmark_t* begin;
    ina_bench_benchmark_t* end;
    ina_str_t report_path = NULL;
    int xrepeat = 0;
    int xiter = 0;
    int core = 0;
    size_t cache_size;
    size_t tot_cache_size;
    if (INA_FAILED(ina_cpu_get_l1_cache_size(&cache_size))) {
        INA_BENCH_MSG("%s", "WARNING: failed to get LL cache size");
    }
    tot_cache_size = cache_size;
    if (INA_FAILED(ina_cpu_get_l2_cache_size(&cache_size))) {
        INA_BENCH_MSG("%s", "WARNING: failed to get L2 cache size");
    }
    tot_cache_size += cache_size;
    if (INA_FAILED(ina_cpu_get_l3_cache_size(&cache_size))) {
        INA_BENCH_MSG("%s", "WARNING: failed to get L3 cache size");
    }
    tot_cache_size += cache_size;
    if (tot_cache_size == 0) {
        int size;
        ina_opt_get_int("cache-size", &size);
        if (size <= 0) {
            size = 10;
        }
        tot_cache_size = (size_t)size * 1024 * 1024;
    }


    INA_MUST_SUCCEED(ina_init());

    INA_MUST_SUCCEED(ina_time_tsc_new(&__time1));
    INA_MUST_SUCCEED(ina_time_tsc_new(&__time2));

    ina_opt_get_string("r", &report_path);
    ina_opt_get_string("n", &__bench_name);
    if (ina_str_len(__bench_name)) {
        filter = __ina_bench_filter;
    }
    ina_opt_get_int("x-repeat", &xrepeat);
    ina_opt_get_int("x-iter", &xiter);
    ina_opt_get_int("c", &core);

    if (core >= 0) {
        if (INA_FAILED(ina_cpu_pin_to_core(core))) {
            printf("couldn't pin on core %d", core);
            return 1;
        }
    }

    begin = &INA_BENCH_BNAME(bench, series);
    end = &INA_BENCH_BNAME(bench, series);
    while (begin) {
        ina_bench_benchmark_t* t = begin-1;
        if (t->magic != INA_BENCH_MAGIC) {
            break;
        }
        begin--;
    }
    while (end) {
        ina_bench_benchmark_t* t = end+1;
        if (t->magic != INA_BENCH_MAGIC) {
            break;
        }
        end++;
    }
    end++;

    for (bench = begin; bench != end; bench++) {
        if (bench == &INA_BENCH_BNAME(bench, series)) {
            continue;
        }
        if (filter(bench)) {
            total++;
        }
    }
    if (begin && end) {
        for (bench = begin; bench != end; bench++) {
            if (bench == &__ina_bench_bench_series) {
                continue;
            }
            if (filter(bench) && !bench->skip) {
                int ic;
                int rc;
#ifdef INA_OS_OSX
                INA_MUST_SUCCEED(__ina_find_symbols(bench));
#endif
				if (xrepeat == 0) {
					xrepeat = bench->repetitions;
				}
				if (xiter == 0) {
					xiter = bench->iterations;
				}
                if (__current == NULL ||
                    strcmp(__current->bench_name, bench->bench_name) != 0) {
                    if (__current != NULL) {
                        INA_MUST_SUCCEED(
                                __ina_write_report(xrepeat, xiter, __current_series,
                                                   ina_str_cstr(
                                                           report_path)));
                        ina_mem_free(__results);
                        ina_mem_free(__scales);
                    }
                    __scales = ina_mem_alloc(sizeof(int64_t) * xrepeat);
                    __results = ina_mem_alloc(
                            sizeof(int64_t) * xiter * xrepeat *
                            __INA_MAX_SERIES);
                    __current_result = __results;
                    __header[0] = '\0';
                    __current_series = 0;
                    __current_iteration = 0;
                    __precision = 5;
                }
                __current_scale = __scales;
                __current = bench;
                if (strlen(__header)) {
                    strncat(__header, ",",
                            sizeof(__header) - strlen(__header) + 1);
                }
                strncat(__header, bench->series_name,
                        sizeof(__header) - strlen(__header) + 1);

                printf("%s:%s : setup\n", ina_bench_get_name(),
                       ina_bench_get_series_name());
                bench->setup(bench->data);
                printf("%s:%s : begin\n", ina_bench_get_name(),
                       ina_bench_get_series_name());

                for (rc = 0; rc < xrepeat; ++rc) {
                    __current_repetition = rc;
                    bench->scale(bench->data);
                    bench->series_setup(bench->data);
                    for (ic = 0; ic < xiter; ++ic) {
                        __current_iteration = ic;
                        __ina_clear_cache(tot_cache_size*2);
                        bench->run(bench->data);
                        __current_result += 1;
                    }
                    __current_scale += 1;
                }
                printf("%s:%s : end\n", ina_bench_get_name(),
                       ina_bench_get_series_name());
                bench->series_teardown(bench->data);
                printf("%s:%s : teardown\n", ina_bench_get_name(),
                       ina_bench_get_series_name());
                bench->teardown(bench->data);

                __current_series += 1;
            }
        }
    }
    if (__current != NULL) {
        __ina_write_report(xrepeat, xiter, __current_series, report_path);
        ina_mem_free(__results);
    }
    ina_time_tsc_free(&__time1);
    ina_time_tsc_free(&__time2);
    ina_str_free(report_path);
    return total;
}

INA_API(const char*) ina_bench_get_name(void)
{
    if (__current != NULL) {
        return ina_str_cstr(__current->bench_name);
    }
    return NULL;
}

INA_API(const char*) ina_bench_get_series_name(void)
{
    if (__current != NULL) {
        return ina_str_cstr(__current->series_name);
    }
    return NULL;
}

INA_API(ina_rc_t) ina_bench_set_scale_label(const char* label)
{
    INA_VERIFY_NOT_NULL(label);
    if (__scale_label != NULL) {
        ina_str_free(__scale_label);
    }
    __scale_label = ina_str_new_fromcstr(label);
    printf("%s:%s : set scale label '%s'\n",
           ina_bench_get_name(),
           ina_bench_get_series_name(),
           ina_str_cstr(__scale_label));

    return INA_SUCCESS;
}

INA_API(const char*) ina_bench_get_scale_label(void)
{
    if (__scale_label != NULL) {
        return ina_str_cstr(__scale_label);
    }
    return NULL;
}

INA_API(ina_rc_t) ina_bench_set_double(double value)
{
    *__current_result = value;
    printf("%s:%s : set result %f for iteration '%d'\n",
           ina_bench_get_name(),
           ina_bench_get_series_name(),
           value,
           ina_bench_get_iteration());

    return INA_SUCCESS;
}


INA_API(ina_rc_t) ina_bench_set_int64(int64_t value)
{
    return ina_bench_set_double((double)value);
}


INA_API(ina_rc_t) ina_bench_set_scale(int64_t scale)
{
    printf("%s:%s : set scale %"INA_INT64_T_FMT" for iteration '%d'\n",
            ina_bench_get_name(),
           ina_bench_get_series_name(),
           scale,
           ina_bench_get_iteration());

    *__current_scale = scale;
    return INA_SUCCESS;
}

INA_API(double) ina_bench_get_double(void)
{
    return *__current_result;
}

INA_API(int64_t) ina_bench_get_int64(void)
{
    return (int64_t )*__current_result;
}

INA_API(int64_t) ina_bench_get_scale(void)
{
    return *__current_scale;
}

INA_API(int) ina_bench_get_repetitions(void)
{
    if (__current != NULL) {
        return __current->repetitions;
    }
    return 0;
}

INA_API(int) ina_bench_get_repetition(void)
{
    if (__current != NULL) {
        return __current_repetition + 1;
    }
    return 0;
}


INA_API(int) ina_bench_get_iterations(void)
{
    if (__current != NULL) {
        return __current->iterations;
    }
    return 0;
}

INA_API(int) ina_bench_get_iteration(void)
{
    if (__current != NULL) {
        return __current_iteration + 1;
    }
    return 0;
}

INA_API(ina_rc_t) ina_bench_stopwatch_start(void)
{
    printf("%s:%s : start time measurement\n", ina_bench_get_name(), ina_bench_get_series_name());
    return  ina_time_read_tsc_clock(__time1);
}

INA_API(int64_t) ina_bench_stopwatch_stop(void)
{
    time_t secs;
    long nanos;
    int64_t micros;

    INA_MUST_SUCCEED(ina_time_read_tsc_clock(__time2));
    printf("%s:%s : stop time measurement\n", ina_bench_get_name(), ina_bench_get_series_name());
    ina_time_tsc_seconds_nanos(__time2, &secs, &nanos);
    micros = secs * 1000*1000*1000 + nanos;
    ina_time_tsc_seconds_nanos(__time1, &secs, &nanos);
    micros -= (secs * 1000 * 1000 *1000 + nanos);
    return micros;
}

INA_API(ina_rc_t) ina_bench_set_precision(int precision)
{
    INA_VERIFY(precision >= 0);
    printf("%s:%s : set precision  '%d'\n", ina_bench_get_name(), ina_bench_get_series_name(), precision);
    __precision = precision;
    return INA_SUCCESS;
}

INA_API(int) ina_bench_get_precision(void)
{
    return __precision;
}