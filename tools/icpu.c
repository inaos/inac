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
 * CAUSED AND ON ANYs THEORY OF LIABILITY, WHETHER IN CONTRACT, 
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */
#include <stdio.h>
#include <libinac/lib.h>

static double __clk = 0;
static INA_VOLATILE int __shutdown = 0;

static void ina_cleanup_handler(int error, int *exitcode)
{
}

static void __ina_interrrupt(ina_signal_t s, ina_signal_behavior_t *b, int *e)
{
    __shutdown = 1;
    *e = EXIT_SUCCESS;
}

static void __ina_measure_freq()
{
    ina_time_t *begints, *endts;
    ina_time_tsc_value_t begin, end;
    time_t bsecs,esecs;
    long bus,eus;
    INA_VOLATILE uint64_t i;
    uint64_t nsecElapsed;
    ina_time_sys_new(&begints);
    ina_time_sys_new(&endts);
    ina_time_read_sys_clock(begints);
    INA_TIME_RDTSC(begin);
    for (i = 0; i < 100000000; i++); /* must be CPU intensive */
    INA_TIME_RDTSC(end);
    ina_time_read_sys_clock(endts);
    ina_time_sys_seconds_micros(endts, &esecs, &eus);
    ina_time_sys_seconds_micros(begints, &bsecs, &bus);
    nsecElapsed = (uint64_t)((esecs * 1000000000ULL + (eus*1000ULL)) - (bsecs * 1000000000ULL + (bus*1000ULL)));
    __clk = (double)(end.uint64 - begin.uint64)/(double)nsecElapsed;
    printf("Clock speed = %1.3f GHz\n", __clk);
    ina_time_sys_free(&begints);
    ina_time_sys_free(&endts);
}

int main(int argc,  char** argv) 
{
    int core;
    int loop = 0;

    INA_OPTS(opt,
        INA_OPT_INT("c", "cpu", NULL, "CPU id to pin"),
        INA_OPT_FLAG("l", "loop", "Loop with 1s pause")
    );

    if (!INA_SUCCEED(ina_app_init(argc, argv, 0, opt))) {
        return EXIT_FAILURE;
    }
    ina_set_cleanup_handler(ina_cleanup_handler);
    ina_register_signal_handler(INA_SIGNAL_INT, __ina_interrrupt);

    ina_opt_get_int("c", &core);
    loop = INA_SUCCEED(ina_opt_isset("l"));

    ina_cpu_pin_to_core(core);

    ina_time_tsc_enable_rdtsc();

    if (loop) {
        while (!__shutdown) {
            __ina_measure_freq();
            ina_time_sleep(1000);
        }
    }
    else {
        __ina_measure_freq();
    }

    return EXIT_SUCCESS;
}
