/*
 * Copyright (c) 2018, INAOS GmbH
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the INAOS GmbH nor the names of its contributors
 *       may be used to endorse or promote products derived from this software 
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL INAOS GmbH BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */
#include <libinac/lib.h>
#include <setjmp.h>
#include "config.h"

#ifdef INA_OS_OSX
#include <dlfcn.h>
#endif

#ifdef INA_OS_WIN32
#define snprintf sprintf_s
#endif

typedef int (*ina_bench_filter_fn_t)(ina_bench_benchmark_t*);

static const char* __bench_name = NULL;
static const char* __binpath = NULL;
static ina_bench_benchmark_t *__current = NULL;
static ina_str_t __scale_label = NULL;
static ina_str_t __value_label = NULL;
static int64_t __scale = 0;
static int64_t __value = 0;
static ina_time_tsc_t *__time1;
static ina_time_tsc_t *__time2;

INA_BENCH_DATA(bench) {
    int dummy;
};

INA_BENCH(bench, series, 0) { }

static int __ina_bench_all(ina_bench_benchmark_t* b) {
    return 1;
}

static int __ina_bench_filter(ina_bench_benchmark_t* b) {
    return (strncmp(__bench_name, b->bench_name, strlen(__bench_name)) == 0);
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
        //fprintf(stderr, ">>>> ERROR: %s\n", dlerror());
    }
    // returns NULL on error
    free(symbol_name);
    return symbol;
}
static void *__ina_find_symbol2(ina_bench_benchmark_t *bench, const char *fname)
{
    size_t len = strlen(bench->bench_name) + 1 + strlen(bench->serie_name) + 1 + strlen(fname);
    char *symbol_name = (char *) malloc(len + 1);
    memset(symbol_name, 0, len + 1);
    snprintf(symbol_name, len + 1, "%s_%s_%s", bench->bench_name, bench->serie_name, fname);
    void *symbol = dlsym(RTLD_DEFAULT, symbol_name);
    if (!symbol) {
        //fprintf(stderr, ">>>> ERROR: %s\n", dlerror());
    }
    // returns NULL on error
    free(symbol_name);
    return symbol;
}
#endif


INA_API(int) ina_bench_run(int argc, char *argv[])
{
    static int total = 0;
    static int index = 1;
    static ina_bench_filter_fn_t filter = __ina_bench_all;
    static ina_bench_benchmark_t* bench;
    ina_bench_benchmark_t* begin;
    ina_bench_benchmark_t* end;

    __binpath = argv[0];

    INA_MUST_SUCCEED(ina_time_tsc_new(&__time1));
    INA_MUST_SUCCEED(ina_time_tsc_new(&__time2));

    if (argc > 2) {
        __bench_name = argv[2];
        filter = __ina_bench_filter;
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

    for (bench = begin; bench != end; bench++) {
        if (bench == &__ina_bench_bench_series) {
            continue;
        }
        if (filter(bench)) {
            if (!bench->skip) {
#ifdef INA_OS_OSX
                if (!bench->setup) {
                    bench->setup = __ina_find_symbol(bench, "setup");
                }
                if (!bench->teardown) {
                    bench->teardown = __ina_find_symbol(bench, "teardown");
                }
                if (!bench->scale) {
                    bench->scale = __ina_find_symbol(bench, "scale");
                }
                if (!bench->series_setup) {
                    bench->series_setup = __ina_find_symbol2(bench, "setup");
                }
                if (!bench->series_teardown) {
                    bench->series_teardown = __ina_find_symbol2(bench, "teardown");
                }
#endif
                __current = bench;

                bench->setup(bench->data);
                bench->series_setup(bench->data);
                for (int ic = 0; ic < bench->iterations; ++ic) {
                    bench->scale(bench->data, ic+1);
                    bench->run(bench->data, ic+1);
                }
                bench->series_teardown(bench->data);
                bench->teardown(bench->data);

                __current = NULL;
            }
            index++;
        }
    }
    ina_time_tsc_free(&__time1);
    ina_time_tsc_free(&__time2);
    return total;
}

INA_API(const char*) ina_bench_get_name(void)
{
    if (__current != NULL) {
        return __current->bench_name;
    }
    return NULL;
}

INA_API(const char*) ina_bench_get_series_name(void)
{
    if (__current != NULL) {
        return __current->serie_name;
    }
    return NULL;
}

INA_API(ina_rc_t) ina_bench_set_value_label(const char* label)
{
    INA_VERIFY_NOT_NULL(label);
    if (__value_label != NULL) {
        ina_str_free(__value_label);
    }
    __value_label = ina_str_new_fromcstr(label);
    return INA_SUCCESS;
}

INA_API(const char*) ina_bench_get_value_label(void)
{
    return __value_label;
}

INA_API(ina_rc_t) ina_bench_set_scale_label(const char* label)
{
    INA_VERIFY_NOT_NULL(label);
    if (__scale_label != NULL) {
        ina_str_free(__scale_label);
    }
    __scale_label = ina_str_new_fromcstr(label);
    return INA_SUCCESS;
}

INA_API(const char*) ina_bench_get_scale_label(void)
{
    return __scale_label;
}

INA_API(ina_rc_t) ina_bench_set_value(int64_t value)
{
    __value = value;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_bench_set_scale(int64_t scale)
{
    __scale = scale;
    return INA_SUCCESS;
}

INA_API(int64_t) ina_bench_get_value(void)
{
    return __value;
}

INA_API(int64_t) ina_bench_get_scale(void)
{
    return __scale;
}

INA_API(int) ina_bench_get_iterations(void)
{
    if (__current != NULL) {
        return __current->iterations;
    }
    return 0;
}

INA_API(ina_rc_t) ina_bench_stopwatch_start(void)
{
    return  ina_time_read_tsc_clock(__time1);
}

INA_API(int64_t) ina_bench_stopwatch_stop(void)
{
    time_t secs;
    long nanos;
    int64_t micros;

    INA_MUST_SUCCEED(ina_time_read_tsc_clock(__time2));
    ina_time_tsc_seconds_nanos(__time2, &secs, &nanos);
    micros = secs * 1000*1000*1000 + nanos;
    ina_time_tsc_seconds_nanos(__time1, &secs, &nanos);
    micros -= (secs * 1000 * 1000 *1000 + nanos);
    return micros;

}
