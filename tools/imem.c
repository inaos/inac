/*
 * Copyright INAOS GmbH, Thalwil, 2015-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <stdio.h>
#include <libinac/lib.h>

static size_t *__test_array_a = NULL;
static size_t *__test_array_b = NULL;

static ina_rc_t __allocate_test_array(size_t **test_array, size_t test_array_size)
{
    size_t i;
    size_t array_size = 1024*1024/sizeof(size_t)*test_array_size;
    printf("array_size=%zd  ", array_size);
    *test_array = (size_t*)calloc(array_size, sizeof(size_t));

    if (*test_array == NULL) {
        return INA_OS_ERROR(INA_ERR_OUT_OF | INA_ES_MEMORY);
    }

    for(i = 0; i < array_size; i++) {
        (*test_array)[i] = 0xaa;
    }

    return INA_SUCCESS;
}

static ina_rc_t __run_memcpy_test(int iterations, size_t test_array_size)
{
    ina_stopwatch_t *w = NULL;
    size_t array_size = 1024*1024/sizeof(size_t)*test_array_size;
    size_t array_bytes = array_size * sizeof(size_t);
    double elapsed_total = 0;
    double duration = 0;
    size_t mib = test_array_size;
    int i;

    if (!INA_SUCCEED(INA_STOPWATCH_NEW(1, -1, &w))) {
        return ina_err_get_rc();
    }

    for (i = 0; i < iterations; i++) {
        
        if (INA_FAILED(__allocate_test_array(&__test_array_a, test_array_size))) {
            return ina_err_get_rc();
        }
        if (INA_FAILED(__allocate_test_array(&__test_array_b, test_array_size))) {
            return ina_err_get_rc();
        }

        INA_STOPWATCH_START(w);
        memcpy(__test_array_b, __test_array_a, array_bytes);
        INA_STOPWATCH_STOP(w);

        ina_stopwatch_duration(w, &duration);
        printf("Elapsed: %.5f\t", duration);
        printf("MiB: %zu\t", mib);
        printf("Copy: %.3f MiB/s\n", test_array_size/duration);

        elapsed_total += duration;

        free(__test_array_a);
        free(__test_array_b);
    }

    INA_STOPWATCH_FREE(&w);

    printf("AVG:\tElapsed: %.5f\t", elapsed_total/iterations);
    printf("MiB: %zu\t", mib);
    printf("Copy: %.3f MiB/s\n", test_array_size/(elapsed_total/iterations));

    return INA_SUCCESS;
}

static void ina_cleanup_handler(int error, int *exitcode)
{
	INA_UNUSED(error);
	*exitcode = 0;
}

int main(int argc,  char** argv) 
{
    int size;
    size_t test_array_size;
    int test_iter;
    int core;

    INA_OPTS(opt,
        INA_OPT_INT("c", "cpu", -1, "CPU id to pin"),
        INA_OPT_FLAG("b", "bandwidth", "Test memory bandwidth"),
        INA_OPT_INT("s", "size", 1, "Test-Array size in Mbyte"),
        INA_OPT_INT("i", "iterations", 10, "Number of test iterations"),
        INA_OPT_FLAG("r", "rdtsc", "Use RDTSC for stopwatch")
    );

    if (!INA_SUCCEED(ina_app_init(argc, argv, opt))) {
        return EXIT_FAILURE;
    }
    ina_set_cleanup_handler(ina_cleanup_handler);

    if (INA_SUCCEED(ina_opt_isset("r"))) {
        ina_time_tsc_enable_rdtsc();
    }

    ina_opt_get_int("s", &size);
    ina_opt_get_int("i", &test_iter);
    ina_opt_get_int("c", &core);

    test_array_size = (size_t)size;

    if (core >= 0) {
        printf("Pinning process to core: %d\n", core);
        ina_cpu_pin_to_core(core);
    }

    if (INA_SUCCEED(ina_opt_isset("b"))) {
        if (!INA_SUCCEED(__run_memcpy_test(test_iter, (size_t)test_array_size))) {
            return ina_err_get_rc();
        }
    }
    else {
        printf("You have to choose a program option, use --help\n");
    }

    return EXIT_SUCCESS;
}
