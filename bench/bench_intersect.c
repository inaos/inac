/*
 * Copyright INAOS GmbH, Thalwil, 2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
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
    ina_bench_set_scale(1);
}
INA_BENCH_BEGIN(intersect, zipper) { INA_UNUSED(data);}
INA_BENCH_END(intersect, zipper) { INA_UNUSED(data); }

INA_BENCH(intersect, zipper, 1)
{
    size_t i_a = 0, i_b = 0;
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
    ina_bench_set_int64(ina_bench_stopwatch_stop());
}

INA_BENCH_BEGIN(intersect, logical) {}
INA_BENCH_END(intersect, logical) {}
INA_BENCH(intersect, logical, 1)
{
    size_t i;
    ina_bench_stopwatch_start();
    for (i = 0; i < data->elements; i++) {
        data->C[i] = data->A[i] & data->B[i];
    }
    ina_bench_set_int64(ina_bench_stopwatch_stop());
}

