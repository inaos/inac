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
 * CAUSED AND ON ANYs THEORY OF LIABILITY, WHETHER IN CONTRACT, 
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */
#include <libinac/lib.h>


typedef struct data_s {
    double d1;
    double d2;
} data_t;

INA_BENCH_DATA(mem) {
    int dummy;
};
INA_BENCH_SETUP(mem) {}
INA_BENCH_TEARDOWN(mem) {}
INA_BENCH_SCALE(mem) {
    ina_bench_set_scale(1);
}
INA_BENCH_BEGIN(mem, malloc_aligned_r) {}
INA_BENCH_END(mem, malloc_aligned_r) {}
INA_BENCH(mem, malloc_aligned_r, 1) {
    size_t i;
    data_t *pdata;

    pdata = malloc(sizeof(data_t)*100000);
    ina_bench_stopwatch_start();
    for (i = 0; i < 100000; ++i) {
        pdata[i].d1 = 1.0;
        pdata[i].d2 = 2.0;
    }
    ina_bench_set_int64(ina_bench_stopwatch_stop());
    free(pdata);
}
INA_BENCH_BEGIN(mem, malloc_aligned_rw) {}
INA_BENCH_END(mem, malloc_aligned_rw) {}
INA_BENCH(mem, malloc_aligned_rw, 1) {
    size_t i;
    data_t *pdata;
    double r=0;

    pdata = malloc(sizeof(data_t)*100000);
    ina_bench_stopwatch_start();
    for (i = 0; i < 100000; ++i) {
        r += pdata[i].d1;
        r += pdata[i].d2;
    }
    ina_bench_set_int64(ina_bench_stopwatch_stop());
    free(pdata);
}

INA_BENCH_BEGIN(mem, malloc_inac_aligned_r) {}
INA_BENCH_END(mem, malloc_inac_aligned_r) {}
INA_BENCH(mem, malloc_inac_aligned_r, 1) {
    size_t i;
    data_t *pdata;

    pdata = ina_mem_alloc_aligned(16, sizeof(data_t)*100000);
    ina_bench_stopwatch_start();
    for (i = 0; i < 100000; ++i) {
        pdata[i].d1 = 1.0;
        pdata[i].d2 = 2.0;
    }
    ina_bench_set_int64(ina_bench_stopwatch_stop());
    ina_mem_free(pdata);
}

INA_BENCH_BEGIN(mem, malloc_inac_aligned_rw) {}
INA_BENCH_END(mem, malloc_inac_aligned_rw) {}
INA_BENCH(mem, malloc_inac_aligned_rw, 1) {
    size_t i;
    data_t *pdata;
    double r=0;

    pdata = ina_mem_alloc_aligned(16, sizeof(data_t)*100000);
    ina_bench_stopwatch_start();
    for (i = 0; i < 100000; ++i) {
        r += pdata[i].d1;
        r += pdata[i].d2;
    }
    ina_bench_set_int64(ina_bench_stopwatch_stop());
    ina_mem_free(pdata);
}

INA_BENCH_BEGIN(mem, malloc_inac_aligned_type_r) {}
INA_BENCH_END(mem, malloc_inac_aligned_type_r) {}
INA_BENCH(mem, malloc_inac_aligned_type_r, 1) {
    size_t i;
    data_t *pdata;

    pdata = ina_mem_alloc_aligned(sizeof(data_t), sizeof(data_t)*100000);
    ina_bench_stopwatch_start();
    for (i = 0; i < 100000; ++i) {
        pdata[i].d1 = 1.0;
        pdata[i].d2 = 2.0;
    }
    ina_bench_set_int64(ina_bench_stopwatch_stop());
    ina_mem_free(pdata);
}

INA_BENCH_BEGIN(mem, malloc_inac_aligned_type_rw) {}
INA_BENCH_END(mem, malloc_inac_aligned_type_rw) {}
INA_BENCH(mem, malloc_inac_aligned_type_rw, 1) {
    size_t i;
    data_t *pdata;
    double r=0;

    pdata = ina_mem_alloc_aligned(sizeof(data_t), sizeof(data_t)*100000);
    ina_bench_stopwatch_start();
    for (i = 0; i < 100000; ++i) {
        r += pdata[i].d1;
        r += pdata[i].d2;
    }
    ina_bench_set_int64(ina_bench_stopwatch_stop());
    ina_mem_free(pdata);
}

