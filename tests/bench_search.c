/*
 * Copyright (c) 2017, INAOS GmbH
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

/*
 * Mike Pall's input on this topic 2017
 * 
 * Second, any searching or sorting function that has an unbiased branch at it's heart will exhibit potentially unstable performance. Key to performance (and not just with LuaJIT) is to use a branch-free comparisons and a branch-free step or swap.
 *
 * The usual trick is to replace the comparison with
 *   sar(a - b, 31)
 * which returns either 0 (all zeros) or -1 (all ones). [Assuming you've aliased bit.arshift to sar.]
 *
 * Then use the result as a mask (or inverted mask) for the step amount or swap index. There are more elegant formulations of binary search that easily accomodate such a masked step amount.
 * If log2(N) is low enough, you can even unroll the steps.
 *
 * [
 *
 * Actually, branch-free binary search can be optimized further. And there are many more algorithmic choices. Some references:
 *
 * https://schani.wordpress.com/2010/04/30/linear-vs-binary-search/
 *
 * https://www.pvk.ca/Blog/2012/07/03/binary-search-star-eliminates-star-branch-mispredictions/
 *
 * https://www.researchgate.net/profile/Florian_Gross/publication/275971053_Index_Search_Algorithms_for_Databases_and_Modern_CPUs/links/554cffca0cf29f836c9cd539.pdf
 *
 * http://www.researchgate.net/profile/Jatin_Chhugani/publication/221213860_FAST_fast_architecture_sensitive_tree_search_on_modern_CPUs_and_GPUs/links/0c96051f5d2990770d000000.pdf
 *
 * http://erikdemaine.org/papers/BRICS2002/paper.pdf
 *
 * ]
 *
 */

static ina_stopwatch_t   *stopwatch = NULL;
static ina_str_t          benchmark = NULL;

static void its_cleanup_handler(int sig, int *error)
{
    if (stopwatch != NULL && INA_SUCCEED(ina_time_stopwatch_started(stopwatch))) {
        
        INA_TIME_STOPWATCH_STOP(stopwatch);

        printf("%s: Duration %f seconds\n", 
            ina_str_cstr(benchmark),
            stopwatch->tv->sec_duration);
        
        ina_time_stopwatch_destroy(&stopwatch);
    }
    if (benchmark) {
        ina_str_free(benchmark);
    }
}

int main(int argc, char **argv)
{
    size_t len;
    size_t k;

    INA_OPTS(opt,
        INA_OPT_INT("i", "size", 1e6, "Number of elements in the input array"),
        INA_OPT_FLAG("l", "linear-search", "Linear Search"),
        INA_OPT_FLAG("b", "binary-search", "Binary Search")
    );

    if (!INA_SUCCEED(ina_app_init(argc, argv, opt))) {
        return EXIT_FAILURE;
    }
    ina_set_cleanup_handler(its_cleanup_handler);

    if (!INA_SUCCEED(INA_TIME_STOPWATCH_CREATE(&stopwatch, 1, -1))) {
        return EXIT_FAILURE;
    }

    ina_opt_get_int("i", (int*)&len);
    ina_opt_get_int("k", (int*)&k);
    
    if (INA_SUCCEED(ina_opt_isset("l"))) {
        benchmark = ina_str_new_fromcstr("linear search");
        INA_TIME_STOPWATCH_START(stopwatch);
    }
    else if (INA_SUCCEED(ina_opt_isset("b"))) {
        benchmark = ina_str_new_fromcstr("binary search");
        INA_TIME_STOPWATCH_START(stopwatch);
    }
    else {
        printf("Invalid benchmark!\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

