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

    INA_MUST_SUCCEED(ina_mempool_new(&data->mp, data->c*50, INA_MEM_FIXED, NULL));
    ina_bench_stopwatch_start();
    for (i = 0; i < data->c; i++) {
        ina_str_new_fromcstr_using_pool("this is just a test string", data->mp);
    }
    ina_bench_set_int64(ina_bench_stopwatch_stop());

    INA_MUST_SUCCEED(ina_mempool_free(data->mp));
}


INA_BENCH_END(string, series_2) {}
INA_BENCH_BEGIN(string, series_3) {
    INA_MUST_SUCCEED(ina_mempool_new(&data->mp, data->c*50, INA_MEM_FIXED, NULL));
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
    INA_MUST_SUCCEED(ina_mempool_free(data->mp));
}

