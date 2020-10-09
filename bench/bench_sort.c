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
#include <stdlib.h>
#include <libinac/lib.h>

size_t j = 0;
size_t p = 0;
size_t temp = 0;

static void quicksort(double *input, size_t *position, size_t k, size_t m)
{
    for (j = p = 0; j < m; j++) {
        if (input[position[j]] < input[position[m]]) {
            continue;
        }
        temp = position[p];
        position[p] = position[j];
        position[j] = temp;
        p++;
    }

    temp = position[m];
    position[m] = position[p];
    position[p] = temp;

    if (p > k) {
        quicksort(input, position, k, p-1);
    }
    else if (p < k) {
        quicksort(input, position+p+1, k-p-1, m-p-1);
    }
}

INA_BENCH_DATA(sort) {
    double *input;
    size_t position;
    size_t k;
    size_t m;
};
INA_BENCH_SETUP(sort) { INA_UNUSED(data);}
INA_BENCH_TEARDOWN(sort) { INA_UNUSED(data);}
INA_BENCH_SCALE(sort) {
    INA_UNUSED(data);
    ina_bench_set_scale(1);
}
INA_BENCH_BEGIN(sort, quicksort_simple){ INA_UNUSED(data);}
INA_BENCH_END(sort, quicksort_simple) { INA_UNUSED(data);}
INA_BENCH(sort, quicksort_simple, 1, 1) {
    ina_bench_stopwatch_start();
    quicksort(data->input, &data->position, data->k, data->m);
    ina_bench_set_value((double)ina_bench_stopwatch_stop());
}


