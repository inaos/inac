/*
 * Copyright (c) 2013, INAOS GmbH
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

static void test_malloc_aligned(ina_stopwatch_t *sw)
{
    size_t i;
    data_t *pdata;
 
    pdata = malloc(sizeof(data_t)*100000);
    INA_TIME_STOPWATCH_START(sw);
    for (i = 0; i < 100000; ++i) {
        pdata[i].d1 = 1.0;
        pdata[i].d2 = 2.0;
    }
    INA_TIME_STOPWATCH_STOP(sw);
    INA_TRACE("Linar R/W = %f msec", sw->tv->msec_duration);
    free(pdata);
}

static void test_malloc_inac_aligned(ina_stopwatch_t *sw)
{
    size_t i;
    data_t *pdata;
 
    pdata = ina_mem_alloc_aligned(16, sizeof(data_t)*100000);
    INA_TIME_STOPWATCH_START(sw);
    for (i = 0; i < 100000; ++i) {
        pdata[i].d1 = 1.0;
        pdata[i].d2 = 2.0;
    }
    INA_TIME_STOPWATCH_STOP(sw);
    INA_TRACE("Linar R/W aligned = %f msec", sw->tv->msec_duration);
    ina_mem_free_aligned(pdata);
}

int main(int argc,  char** argv) 
{ 
    ina_stopwatch_t *sw;

    if (!INA_SUCCEED(ina_app_init(argc, argv, 0, NULL))) {
        return EXIT_FAILURE;
    }

    INA_TIME_STOPWATCH_CREATE(&sw, 1, 1024);

    test_malloc_aligned(sw);
    test_malloc_inac_aligned(sw);
    
    return EXIT_SUCCESS;
}
