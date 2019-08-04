/*
 * Copyright INAOS GmbH, Thalwil, 2015-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <stdio.h>
#include <libinac/lib.h>

static double __clk = 0;
static INA_VOLATILE int __shutdown = 0;

static void ina_cleanup_handler(int error, int *exitcode)
{
	INA_UNUSED(error);
	*exitcode = 0;
}

static void __ina_interrrupt(ina_signal_t s, ina_signal_behavior_t *b, int *e)
{
	INA_UNUSED(s);
	INA_UNUSED(b);
    __shutdown = 1;
    *e = EXIT_SUCCESS;
}

static void __show_cpu_flags(void)
{
	ina_str_t sflags;
	ina_cpu_feature_t flags;
	ina_cpu_get_features(&flags);

	/* FIXME: create macro to reduce duplication */

	sflags = ina_str_new(1024);
	if (flags & INA_CPU_FEATURE_FPU) {
		sflags = ina_str_catcstr(sflags, "FPU: YES\n");
	}
	else {
		sflags = ina_str_catcstr(sflags, "FPU: NO\n");
	}
	if (flags & INA_CPU_FEATURE_TSC) {
		sflags = ina_str_catcstr(sflags, "TSC: YES\n");
	}
	else {
		sflags = ina_str_catcstr(sflags, "TSC: NO\n");
	}
	if (flags & INA_CPU_FEATURE_CX8) {
		sflags = ina_str_catcstr(sflags, "CX8: YES\n");
	}
	else {
		sflags = ina_str_catcstr(sflags, "CX8: NO\n");
	}
	if (flags & INA_CPU_FEATURE_CMOV) {
		sflags = ina_str_catcstr(sflags, "CMOV: YES\n");
	}
	else {
		sflags = ina_str_catcstr(sflags, "CMOV: NO\n");
	}
	if (flags & INA_CPU_FEATURE_MMX) {
		sflags = ina_str_catcstr(sflags, "MMX: YES\n");
	}
	else {
		sflags = ina_str_catcstr(sflags, "MMX: NO\n");
	}
	if (flags & INA_CPU_FEATURE_SSE) {
		sflags = ina_str_catcstr(sflags, "SSE: YES\n");
	}
	else {
		sflags = ina_str_catcstr(sflags, "SSE: NO\n");
	}
	if (flags & INA_CPU_FEATURE_SSE3) {
		sflags = ina_str_catcstr(sflags, "SSE3: YES\n");
	}
	else {
		sflags = ina_str_catcstr(sflags, "SSE3: NO\n");
	}
	if (flags & INA_CPU_FEATURE_SSSE3) {
		sflags = ina_str_catcstr(sflags, "SSSE3: YES\n");
	}
	else {
		sflags = ina_str_catcstr(sflags, "SSSE3: NO\n");
	}
	if (flags & INA_CPU_FEATURE_HTT) {
		sflags = ina_str_catcstr(sflags, "HTT: YES\n");
	}
	else {
		sflags = ina_str_catcstr(sflags, "HTT: NO\n");
	}
	if (flags & INA_CPU_FEATURE_EST) {
		sflags = ina_str_catcstr(sflags, "EST: YES\n");
	}
	else {
		sflags = ina_str_catcstr(sflags, "EST: NO\n");
	}
	if (flags & INA_CPU_FEATURE_FMA) {
		sflags = ina_str_catcstr(sflags, "FMA: YES\n");
	}
	else {
		sflags = ina_str_catcstr(sflags, "FMA: NO\n");
	}
	if (flags & INA_CPU_FEATURE_DCA) {
		sflags = ina_str_catcstr(sflags, "DCA: YES\n");
	}
	else {
		sflags = ina_str_catcstr(sflags, "DCA: NO\n");
	}
	if (flags & INA_CPU_FEATURE_SSE41) {
		sflags = ina_str_catcstr(sflags, "SSE41: YES\n");
	}
	else {
		sflags = ina_str_catcstr(sflags, "SSE41: NO\n");
	}
	if (flags & INA_CPU_FEATURE_SSE42) {
		sflags = ina_str_catcstr(sflags, "SSE42: YES\n");
	}
	else {
		sflags = ina_str_catcstr(sflags, "SSE42: NO\n");
	}
	if (flags & INA_CPU_FEATURE_AVX) {
		sflags = ina_str_catcstr(sflags, "AVX: YES\n");
	}
	else {
		sflags = ina_str_catcstr(sflags, "AVX: NO\n");
	}
	if (flags & INA_CPU_FEATURE_AES) {
		sflags = ina_str_catcstr(sflags, "AES: YES\n");
	}
	else {
		sflags = ina_str_catcstr(sflags, "AES: NO\n");
	}
	if (flags & INA_CPU_FEATURE_RDRND) {
		sflags = ina_str_catcstr(sflags, "RDRND: YES\n");
	}
	else {
		sflags = ina_str_catcstr(sflags, "RDRND: NO\n");
	}
	if (flags & INA_CPU_FEATURE_POPCNT) {
		sflags = ina_str_catcstr(sflags, "POPCNT: YES\n");
	}
	else {
		sflags = ina_str_catcstr(sflags, "POPCNT: NO\n");
	}
	printf("%s", ina_str_cstr(sflags));
	ina_str_free(sflags);
}

static void __ina_measure_freq(void)
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

    if (!INA_SUCCEED(ina_app_init(argc, argv, opt))) {
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

	__show_cpu_flags();

    return EXIT_SUCCESS;
}
