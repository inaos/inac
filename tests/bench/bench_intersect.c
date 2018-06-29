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
INA_BENCH_BEGIN(intersect, zipper) {}
INA_BENCH_END(intersect, zipper) {}

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

