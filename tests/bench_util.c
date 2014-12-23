/*
 * Copyright (c) 2014, INAOS GmbH
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

uint32_t __ina_util_bin32search(uint32_t *haystack, uint32_t count, uint32_t needle)
{
    uint32_t min = 0, max = count;
    while (min < max) {
        uint32_t middle = (min + max) >> 1;
        if (needle > haystack[middle]) {
            min = middle + 1;
        }
        else {
            max = middle;
        }
    }
    return haystack[min];
}

uint32_t __ina_util_lin32search(uint32_t *haystack, uint32_t count, uint32_t needle)
{
    uint32_t i;
    
    for (i = 0; i < count; i++) {
        uint32_t p = haystack[i];
        if (p >= needle) {
            return p;
        }
    }

    return 0;
}

#define TEST_SIZE 86408
#define FILL_SIZE 86400

int __cmp_4_qsort(const void *a, const void *b)
{
    uint32_t lhs = *(uint32_t*)a;
    uint32_t rhs = *(uint32_t*)b;

    if (lhs < rhs) {
        return -1;
    }
    else if (lhs > rhs) {
        return 1;
    }
    else {
        return 0;
    }
}

int main(int argc, char **argv)
{
    int i;
    int mode = 0;
    int iterations = 0;
    uint32_t *test_vals;
    uint32_t test_keys[1000];
    int z = 0;
    ina_stopwatch_t *s1 = NULL;

    INA_OPTS(opt,
        INA_OPT_INT("i", "iterations", 1, "Number of benchmark iterations"),
        INA_OPT_INT("m", "mode", 1, "Which test should be executed") 
    );

    if (!INA_SUCCEED(ina_app_init(argc, argv, 0, opt))) {
        return EXIT_FAILURE;
    }

    ina_opt_get_int("i", &iterations);
    ina_opt_get_int("m", &mode);

    INA_TIME_STOPWATCH_CREATE(&s1, 1, -1);

    srand(time(NULL));
    test_vals  = (uint32_t*)ina_mem_alloc(sizeof(uint32_t)*TEST_SIZE);
    ina_mem_set(test_vals, 0, sizeof(uint32_t)*TEST_SIZE);

    for (i = 0; i < FILL_SIZE; i++) {
        uint32_t tk = rand() % 86400 + 1;
        test_vals[i] = tk;
        if (i % FILL_SIZE/1000 == 0) {
            test_keys[z++] = tk; 
        }
    }
    qsort(test_vals, FILL_SIZE, sizeof(uint32_t), __cmp_4_qsort);

    for (i = 0; i < iterations; i++) {
        switch (mode) {
            case 1:
                printf("Mode 1: Simple linar search\n");
                break;
            case 2:
                printf("Mode 2: Simple binary search\n");
                break;
        }
        INA_TIME_STOPWATCH_START(s1);
        for (z = 0; z < 1000; z++) {
            uint32_t tk = test_keys[z];
            uint32_t fk = 0;
            switch (mode) {
                case 1:
                    fk = __ina_util_lin32search(test_vals, FILL_SIZE-1, tk);
                    break;
                case 2:
                    fk = __ina_util_bin32search(test_vals, FILL_SIZE-1, tk);
                    break;
            }
            INA_ASSERT_TRUE(tk == fk);
        }
        INA_TIME_STOPWATCH_STOP(s1);
        printf("micro-time: %f\n", s1->tv->usec_duration);
    }

    INA_TIME_STOPWATCH_DESTROY(&s1);
    ina_mem_free(test_vals);
    
    return EXIT_SUCCESS;
}

