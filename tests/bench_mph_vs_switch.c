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

static int templates[] = {
    120, 109, 97, 99, 98, 94, 124, 95, 96, 93, 512
};

static ina_stopwatch_t    *stopwatch = NULL;
static ina_str_t           benchmark = NULL;

static int __counter_120 = 0;
static int __counter_109 = 0;
static int __counter_97 = 0;
static int __counter_99 = 0;
static int __counter_98 = 0;
static int __counter_94 = 0;
static int __counter_124 = 0;
static int __counter_95 = 0;
static int __counter_96 = 0;
static int __counter_93 = 0;
static int __counter_512 = 0;

typedef void (*__inafx_eurex_feed_listener_emdi_cb_t)();

static __inafx_eurex_feed_listener_emdi_cb_t __eurex_emdi_fast_path_switch[16];

/* BEGIN - EUREX 3.0 EMDI TID switch */

static uint32_t __eurex_emdi_tid_hash_tab[] = {
    5,  4,  0,  3,  7, 10,  8,  2,
};

static uint32_t __inafx_fast_eurex_emdi_tid_hash(uint32_t val)
{
    uint32_t a, b, rsl;
    val += 0xc7eac9e2;
    val += (val << 8);
    val ^= (val >> 4);
    b = (val >> 8) & 0x7;
    a = (val + (val << 16)) >> 29;
    rsl = (a^__eurex_emdi_tid_hash_tab[b]);
    return rsl;
}

/* END - EUREX 3.0 EMDI TID switch */

static void __dummy_120()
{
    __counter_120++;
}
static void __dummy_109()
{
    __counter_109++;
}
static void __dummy_97()
{
    __counter_97++;
}
static void __dummy_99()
{
    __counter_99++;
}
static void __dummy_98()
{
    __counter_98++;
}
static void __dummy_94()
{
    __counter_94++;
}
static void __dummy_124()
{
    __counter_124++;
}
static void __dummy_95()
{
    __counter_95++;
}
static void __dummy_96()
{
    __counter_96++;
}
static void __dummy_93()
{
    __counter_93++;
}
static void __dummy_512()
{
    __counter_512++;
}

static void _run_switch(int num_templates, size_t iterations)
{
    size_t i;
    for (i = 0; i < iterations*100000; i++) {
        int tid = templates[rand() % num_templates];
        switch (tid) {
            case 120:
                __dummy_120();
                break;
            case 109:
                __dummy_109();
                break;
            case 97:
                __dummy_97();
                break;
            case 99:
                __dummy_99();
                break;
            case 98:
                __dummy_98();
                break;
            case 94:
                __dummy_94();
                break;
            case 124:
                __dummy_124();
                break;
            case 95:
                __dummy_95();
                break;
            case 96:
                __dummy_96();
                break;
            case 93:
                __dummy_93();
                break;
            case 512:
                __dummy_512();
                break;
            default:
                abort();
        }
    }
}

static void _run_fast(int num_templates, size_t iterations)
{
    size_t i;
    for (i = 0; i < iterations*100000; i++) {
        int tid = templates[rand() % num_templates];
        __eurex_emdi_fast_path_switch[__inafx_fast_eurex_emdi_tid_hash(tid)]();
    }
}

static void _cleanup_handler(int sig, int *error)
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
    size_t iterations;
    int numtpl = sizeof(templates)/sizeof(int);

    INA_OPTS(opt,
        INA_OPT_INT("i", "iterations", 1, "Number of million iterations"),
        INA_OPT_FLAG("n", "naive", "Execute the switch approach"),
        INA_OPT_FLAG("p", "mph", "Execute the approach with minimal perfect hash")
    );

    if (!INA_SUCCEED(ina_app_init(argc, argv, 0, opt))) {
        return EXIT_FAILURE;
    }
    ina_set_cleanup_handler(_cleanup_handler);

    /* initialize random seed: */
    srand((unsigned int)time(NULL));

    if (!INA_SUCCEED(INA_TIME_STOPWATCH_CREATE(&stopwatch, 1, -1))) {
        return EXIT_FAILURE;
    }

    ina_opt_get_int("i", (int*)&iterations);
    
    if (INA_SUCCEED(ina_opt_isset("n"))) {
        
        benchmark = ina_str_new_fromcstr("switch");
       
        INA_TIME_STOPWATCH_START(stopwatch);
        _run_switch(numtpl, iterations);
    }
    else if (INA_SUCCEED(ina_opt_isset("p"))) {

        __eurex_emdi_fast_path_switch[__inafx_fast_eurex_emdi_tid_hash(120)] = __dummy_120;
        __eurex_emdi_fast_path_switch[__inafx_fast_eurex_emdi_tid_hash(109)] = __dummy_109;
        __eurex_emdi_fast_path_switch[__inafx_fast_eurex_emdi_tid_hash(97)] = __dummy_97;
        __eurex_emdi_fast_path_switch[__inafx_fast_eurex_emdi_tid_hash(99)] = __dummy_99;
        __eurex_emdi_fast_path_switch[__inafx_fast_eurex_emdi_tid_hash(98)] = __dummy_98;
        __eurex_emdi_fast_path_switch[__inafx_fast_eurex_emdi_tid_hash(94)] = __dummy_94;
        __eurex_emdi_fast_path_switch[__inafx_fast_eurex_emdi_tid_hash(124)] = __dummy_124;
        __eurex_emdi_fast_path_switch[__inafx_fast_eurex_emdi_tid_hash(95)] = __dummy_95;
        __eurex_emdi_fast_path_switch[__inafx_fast_eurex_emdi_tid_hash(96)] = __dummy_96;
        __eurex_emdi_fast_path_switch[__inafx_fast_eurex_emdi_tid_hash(93)] = __dummy_93;
        __eurex_emdi_fast_path_switch[__inafx_fast_eurex_emdi_tid_hash(512)] = __dummy_512;
        
        benchmark = ina_str_new_fromcstr("mph");
     
        INA_TIME_STOPWATCH_START(stopwatch);
        _run_fast(numtpl, iterations);
    }
    else {
        printf("Invalid benchmark!\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

