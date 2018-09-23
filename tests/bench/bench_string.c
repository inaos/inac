/*
 * Copyright INAOS GmbH, Thalwil, 2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>


INA_BENCH_DATA(string) {
    ina_str_t *strings;
    ina_mempool_t *mp;
    int c;
};


INA_BENCH_SETUP(string) {
    ina_bench_set_scale_label("nr_of_elements");
    ina_bench_set_precision(0);
}
INA_BENCH_TEARDOWN(string) {}
INA_BENCH_BEGIN(string, series_1) {
    data->strings = NULL;
}
INA_BENCH_SCALE(string) {
    data->c = 1000 * ina_bench_get_iteration();
    ina_bench_set_scale(data->c);
}

INA_BENCH(string, series_1, 10) {
    int i;
    INA_BENCH_MSG("iteration: %d - allocate %d strings ",
           ina_bench_get_iteration(),
           data->c);

    data->strings = ina_mem_alloc(sizeof(ina_str_t) * data->c);

    ina_bench_stopwatch_start();
    for (i = 0; i < data->c; i++) {
        data->strings[i] = ina_str_new_fromcstr("this is just a test string");
    }
    ina_bench_set_int64(ina_bench_stopwatch_stop());
    ina_mem_free(data->strings);
}

INA_BENCH_END(string, series_1) {}
INA_BENCH_BEGIN(string, series_2) {
    data->mp = NULL;
}
INA_BENCH(string, series_2, 10) {
    int i;
    INA_BENCH_MSG("iteration: %d allocate %d strings:",
           ina_bench_get_iteration(),
           data->c);

    INA_MUST_SUCCEED(ina_mempool_new(data->c * 50, NULL, INA_MEM_FIXED, &data->mp));
    ina_bench_stopwatch_start();
    for (i = 0; i < data->c; i++) {
        ina_str_new_fromcstr_using_pool("this is just a test string", data->mp);
    }
    ina_bench_set_int64(ina_bench_stopwatch_stop());

    ina_mempool_free(&data->mp);
}


INA_BENCH_END(string, series_2) {}
INA_BENCH_BEGIN(string, series_3) {
    INA_MUST_SUCCEED(ina_mempool_new(data->c * 50, NULL, INA_MEM_FIXED, &data->mp));
}

INA_BENCH(string, series_3, 10) {
    int i;
    INA_BENCH_MSG("iteration: %d allocate %d strings:",
           ina_bench_get_iteration(),
           data->c);

    ina_mempool_reset(data->mp);
    ina_bench_stopwatch_start();
    for (i = 0; i < data->c; i++)  {
        ina_str_new_fromcstr_using_pool("this is just a test string", data->mp);
    }
    ina_bench_set_int64(ina_bench_stopwatch_stop());
}

INA_BENCH_END(string, series_3) {
    ina_mempool_free(&data->mp);
}

