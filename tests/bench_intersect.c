/*
 * Copyright (c) 2015, INAOS GmbH
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

static ina_stopwatch_t   *stopwatch = NULL;
static ina_str_t          benchmark = NULL;

size_t intersect_zipper(int32_t *A, int32_t *B, size_t s_a, size_t s_b, int32_t *C) 
{
    size_t i_a = 0, i_b = 0;
    size_t counter = 0;
 
    while(i_a < s_a && i_b < s_b) {
        if(A[i_a] < B[i_b]) {
            i_a++;
        } else if(B[i_b] < A[i_a]) {
            i_b++;
        } else {
            C[counter++] = A[i_a];
            i_a++; i_b++;
        }
    }
    return counter;
}

static void its_cleanup_handler(int sig, int *error)
{
    if (stopwatch != NULL) {
        
        INA_TIME_STOPWATCH_STOP(stopwatch);

        printf("%s: Duration %f seconds\n", 
            benchmark,
            stopwatch->tv->sec_duration);
        ina_time_stopwatch_destroy(&stopwatch);
    }
    if (benchmark) {
        ina_str_free(benchmark);
    }
}

int main(int argc, char **argv)
{
    int32_t *A, *B, *C;
    size_t len;

    INA_OPTS(opt,
        INA_OPT_INT("i", "size", 1024, "Number of elements in the intersection arrays"),
        INA_OPT_FLAG("z", "zipper", "Execute a simple zipper intersection")
    );

    if (!INA_SUCCEED(ina_app_init(argc, argv, 0, opt))) {
        return EXIT_FAILURE;
    }
    ina_set_cleanup_handler(its_cleanup_handler);

    if (!INA_SUCCEED(INA_TIME_STOPWATCH_CREATE(&stopwatch, 1, -1))) {
        return EXIT_FAILURE;
    }

    ina_opt_get_int("i", (int*)&len);
    A = (int32_t*)ina_mem_alloc(sizeof(int32_t)*len);
    B = (int32_t*)ina_mem_alloc(sizeof(int32_t)*len);
    C = (int32_t*)ina_mem_alloc(sizeof(int32_t)*len);

    INA_TIME_STOPWATCH_START(stopwatch);
    
    if (INA_SUCCEED(ina_opt_isset("z"))) {
        benchmark = ina_str_new_fromcstr("zipper");
        intersect_zipper(A, B, len, len, C);
    } else {
        printf("Invalid benchmark!\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

