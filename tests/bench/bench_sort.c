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
#include <stdlib.h>
#include <libinac/lib.h>


size_t j = 0, p = 0, temp = 0;

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
INA_BENCH_SETUP(sort){}
INA_BENCH_TEARDOWN(sort) {}
INA_BENCH_SCALE(sort) {
    ina_bench_set_scale(1);
}
INA_BENCH_BEGIN(sort, quicksort_simple){}
INA_BENCH_END(sort, quicksort_simple) {}
INA_BENCH(sort, quicksort_simple, 1) {
    ina_bench_stopwatch_start();
    quicksort(data->input, &data->position, data->k, data->m);
    ina_bench_set_int64(ina_bench_stopwatch_stop());
}


