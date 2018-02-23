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
    int c1;
    int c2;
};

INA_BENCH_SETUP(string) {
    ina_bench_set_x_label("hits");
    ina_bench_set_y_label("ms");
    printf("Setup %s\n", ina_bench_get_name());
}

INA_BENCH_TEARDOWN(string) {
    printf("Teardown %s\n", ina_bench_get_name());
}

INA_BENCH_BEGIN(string, series_1) {
    printf("Setup %s - %s\n", ina_bench_get_name(), ina_bench_get_series_name());
    ina_bench_stopwatch_start();

}
INA_BENCH(string, series_1, 100) {
    printf("%s - iteration: %d\n", ina_bench_get_series_name(), ic);
}

INA_BENCH_END(string, series_1) {
    printf("Teardown %s - %s\n", ina_bench_get_name(), ina_bench_get_series_name());
}


INA_BENCH_BEGIN(string, series_2) {
    printf("Setup %s - %s\n", ina_bench_get_name(), ina_bench_get_series_name());
    ina_bench_stopwatch_start();
}

INA_BENCH(string, series_2, 100) {
    printf("%s - iteration: %d\n", ina_bench_get_series_name(), ic);
}


INA_BENCH_END(string, series_2) {
    printf("Teardown %s - %s\n", ina_bench_get_name(), ina_bench_get_series_name());
}


