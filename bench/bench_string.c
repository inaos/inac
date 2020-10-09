/*
 * Copyright 2018-2020 INAOS GmbH, Thalwil
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <libinac/lib.h>


INA_BENCH_DATA(string) {
    ina_str_t *strings;
    ina_mempool_t *mp;
    int c;
};


INA_BENCH_SETUP(string) {
    INA_UNUSED(data);
    ina_bench_set_scale_label("nr_of_elements");
    ina_bench_set_precision(0);
}
INA_BENCH_TEARDOWN(string) { INA_UNUSED(data);}
INA_BENCH_BEGIN(string, series_1) {
    data->strings = NULL;
}
INA_BENCH_SCALE(string) {
    data->c = 1000 * ina_bench_get_repetition();
    ina_bench_set_scale(data->c);
}

INA_BENCH(string, series_1, 10, 1) {
    int i;
    INA_BENCH_MSG("iteration: %d - allocate %d strings ",
           ina_bench_get_iteration(),
           data->c);
    if (INA_SUCCEED(ina_bench_is_warmup())) {
        INA_BENCH_MSG("warm-up");
    }

    data->strings = ina_mem_alloc(sizeof(ina_str_t) * data->c);

    ina_bench_stopwatch_start();
    for (i = 0; i < data->c; i++) {
        data->strings[i] = ina_str_new_fromcstr("this is just a test string");
    }
    ina_bench_set_value((double)ina_bench_stopwatch_stop());
    ina_mem_free(data->strings);
}

INA_BENCH_END(string, series_1) { INA_UNUSED(data);}
INA_BENCH_BEGIN(string, series_2) {
    data->mp = NULL;
}
INA_BENCH(string, series_2, 10, 1) {
    int i;
    INA_BENCH_MSG("iteration: %d allocate %d strings:",
           ina_bench_get_iteration(),
           data->c);

    INA_MUST_SUCCEED(ina_mempool_new(data->c * 50, NULL, INA_MEM_FIXED, &data->mp));
    ina_bench_stopwatch_start();
    for (i = 0; i < data->c; i++) {
        ina_str_new_fromcstr_using_pool("this is just a test string", data->mp);
    }
    ina_bench_set_value((double)ina_bench_stopwatch_stop());

    ina_mempool_free(&data->mp);
}


INA_BENCH_END(string, series_2) { INA_UNUSED(data);}
INA_BENCH_BEGIN(string, series_3) {
    INA_MUST_SUCCEED(ina_mempool_new(data->c * 50, NULL, INA_MEM_FIXED, &data->mp));
}

INA_BENCH(string, series_3, 10, 1) {
    int i;
    INA_BENCH_MSG("iteration: %d allocate %d strings:",
           ina_bench_get_iteration(),
           data->c);

    ina_mempool_reset(data->mp);
    ina_bench_stopwatch_start();
    for (i = 0; i < data->c; i++)  {
        ina_str_new_fromcstr_using_pool("this is just a test string", data->mp);
    }
    ina_bench_set_value((double)ina_bench_stopwatch_stop());
}

INA_BENCH_END(string, series_3) {
    ina_mempool_free(&data->mp);
}

