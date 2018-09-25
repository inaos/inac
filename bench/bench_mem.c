/*
 * Copyright INAOS GmbH, Thalwil, 2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
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

