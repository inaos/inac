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


