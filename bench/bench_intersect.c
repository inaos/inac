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

INA_BENCH_DATA(intersect){
    int32_t* A;
    int32_t* B;
    int32_t* C;
    size_t elements;
};

INA_BENCH_SETUP(intersect)
{
    ina_bench_set_precision(0);
    data->elements = 1024;
    data->A = (int32_t*)ina_mem_alloc(sizeof(int32_t)*data->elements);
    data->B = (int32_t*)ina_mem_alloc(sizeof(int32_t)*data->elements);
    data->C = (int32_t*)ina_mem_alloc(sizeof(int32_t)*data->elements);
}
INA_BENCH_TEARDOWN(intersect)
{
    ina_mem_free(data->A);
    ina_mem_free(data->C);
    ina_mem_free(data->B);
}

INA_BENCH_SCALE(intersect)
{
    INA_UNUSED(data);
    ina_bench_set_scale(1);
}
INA_BENCH_BEGIN(intersect, zipper) { INA_UNUSED(data);}
INA_BENCH_END(intersect, zipper) { INA_UNUSED(data); }

INA_BENCH(intersect, zipper)
{
    size_t i_a = 0;
    size_t i_b = 0;
    size_t counter = 0;

    ina_bench_stopwatch_start();
    while(i_a < data->elements && i_b < data->elements) {
        if(data->A[i_a] < data->B[i_b]) {
            i_a++;
        } else if(data->B[i_b] < data->A[i_a]) {
            i_b++;
        } else {
            data->C[counter++] = data->A[i_a];
            i_a++; i_b++;
        }
    }
    ina_bench_set_value((double)ina_bench_stopwatch_stop());
}

INA_BENCH_BEGIN(intersect, logical) { INA_UNUSED(data);}
INA_BENCH_END(intersect, logical) { INA_UNUSED(data);}
INA_BENCH(intersect, logical)
{
    size_t i;
    ina_bench_stopwatch_start();
    for (i = 0; i < data->elements; i++) {
        data->C[i] = data->A[i] & data->B[i];
    }
    ina_bench_set_value((double)ina_bench_stopwatch_stop());
}

