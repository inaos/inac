/*
 * Copyright (c) 2012-2018, INAOS GmbH
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
#ifndef INA_OS_WIN32
#define _GNU_SOURCE  
#include <sched.h>
#endif
#include <libinac/lib.h>

#if !defined(CLOCK_MONOTONIC_RAW)
    #define CLOCK_MONOTONIC_RAW CLOCK_MONOTONIC
#endif

INA_TEST(time, tsc_strftime)
{
    ina_str_t str = ina_str_new(128);
    ina_time_tsc_t *time = NULL;
    ina_time_tsc_new(&time);
    INA_TEST_ASSERT_NOT_NULL(time);
    INA_TEST_ASSERT_SUCCEED(ina_time_read_tsc_clock(time));
    INA_TEST_ASSERT_SUCCEED(ina_time_tsc_strftime(str, "%H:%M:%S", time, INA_YES));
    INA_TEST_MSG("result of ina_time_tsc_strftime(): %s", ina_str_cstr(str));
    ina_str_free(str);
    ina_time_tsc_free(&time);
}

INA_TEST(time,time_stamp)
{
    ina_stopwatch_t *w = NULL;
    int64_t c = 10;
    double msec_duration = 0;

    INA_TEST_ASSERT_SUCCEED(ina_time_stopwatch_create(&w, 1, -1));
    INA_TEST_ASSERT_NOT_NULL(w);
    INA_TEST_ASSERT_EQUAL_FLOATING(1, w->id);
    INA_TEST_ASSERT_EQUAL_FLOATING(1024, w->tv->max_stamps);
    INA_TEST_ASSERT_NOT_NULL(w->tv);
    INA_TEST_ASSERT_NULL(w->ts);
    INA_TEST_ASSERT_FAILED(ina_time_stopwatch_started(w));
    INA_TEST_ASSERT_SUCCEED(ina_time_stopwatch_start(w, NULL));
    while (c--) {
        INA_TEST_ASSERT_SUCCEED(ina_time_stopwatch_stamp(w, "1", "2"));
        ina_time_sleep(100);
    }
    INA_TEST_ASSERT_SUCCEED(ina_time_stopwatch_stop(w));
    c = 0;
    while (INA_SUCCEED(ina_time_stopwatch_read_stamp(w, &c))) {
        INA_TEST_ASSERT_NOT_NULL(w->ts);
        INA_TEST_ASSERT_TRUE(msec_duration < w->ts->msec_duration);
        ++c;
    }
    INA_TEST_ASSERT_EQUAL_INTEGER(10, c);
    INA_TEST_ASSERT_SUCCEED(ina_time_stopwatch_destroy(&w));
}

#if !defined (INA_OS_WIN32) && !defined(INA_OS_OSX)
INA_TEST(time, two_stopwatches)
{
    ina_stopwatch_t *w1 = NULL;
    ina_stopwatch_t *w2 = NULL;
    time_t sec1  = 0;
    time_t sec2  = 0; 
    long nano1 = 0;
    long nano2 = 0;

    INA_TEST_ASSERT_SUCCEED(ina_time_stopwatch_create(&w1, 1, -1));
    INA_TEST_ASSERT_NOT_NULL(w1);
    INA_TEST_ASSERT_EQUAL_FLOATING(1, w1->id);
    INA_TEST_ASSERT_NOT_NULL(w1->tv);
    INA_TEST_ASSERT_NULL(w1->ts);
    INA_TEST_ASSERT_SUCCEED(ina_time_stopwatch_create(&w2, 2, -1));
    INA_TEST_ASSERT_NOT_NULL(w2);
    INA_TEST_ASSERT_EQUAL_FLOATING(2, w2->id);
    INA_TEST_ASSERT_NOT_NULL(w2->tv);
    INA_TEST_ASSERT_NULL(w2->ts);
    INA_TEST_ASSERT_SUCCEED(ina_time_stopwatch_start(w1, NULL));
    INA_TEST_ASSERT_SUCCEED(ina_time_stopwatch_start(w2, NULL));
    INA_TEST_ASSERT_SUCCEED(ina_time_stopwatch_stop(w1));
    INA_TEST_ASSERT_SUCCEED(ina_time_stopwatch_stop(w2));
    INA_TEST_ASSERT_SUCCEED(ina_time_tsc_seconds_nanos(&w1->tv->start, 
                                &sec1, &nano1)); 
    INA_TEST_ASSERT_SUCCEED(ina_time_tsc_seconds_nanos(&w2->tv->start, 
                                &sec2, &nano2));
    INA_TEST_ASSERT_EQUAL_INTEGER(sec1, sec2);
    INA_TEST_ASSERT_TRUE(nano1< nano2);
    INA_TEST_ASSERT_SUCCEED(ina_time_stopwatch_destroy(&w1));
    INA_TEST_ASSERT_SUCCEED(ina_time_stopwatch_destroy(&w2));
} 
#endif

INA_TEST(time, stopwatch) 
{
    struct timeval tv_start;
    ina_stopwatch_t *w = NULL;

    gettimeofday(&tv_start, NULL);
    INA_TEST_ASSERT_SUCCEED(ina_time_stopwatch_create(&w, 1, -1));
    INA_TEST_ASSERT_NOT_NULL(w);
    INA_TEST_ASSERT_EQUAL_INTEGER(0, w->tv->next_stamp);
    INA_TEST_ASSERT_EQUAL_FLOATING(INA_TIME_MAX_STAMPS, w->tv->max_stamps);
    INA_TEST_ASSERT_SUCCEED(ina_time_stopwatch_start(w, NULL));
    INA_TEST_ASSERT_EQUAL_FLOATING(0, w->tv->sec_duration);
    INA_TEST_ASSERT_NULL(w->ts);
    INA_TEST_ASSERT_SUCCEED(ina_time_stopwatch_started(w));
    ina_time_sleep(1);
    INA_TEST_ASSERT_FAILED(ina_time_stopwatch_valid(w));
    ina_time_sleep(1);
    INA_TEST_ASSERT_SUCCEED(ina_time_stopwatch_stop(w));
    INA_TEST_ASSERT_SUCCEED(ina_time_stopwatch_valid(w));
    INA_TEST_ASSERT_SUCCEED(ina_time_stopwatch_destroy(&w));
    INA_TEST_ASSERT_NULL(w);
}

INA_TEST(time, stopwatch_startime) 
{
    struct timeval tv_start;
    ina_stopwatch_t *w = NULL;
    ina_time_tsc_t start_ts;
    int64_t i = 0;

    gettimeofday(&tv_start, NULL);
    ina_time_read_tsc_clock(&start_ts);
    ina_time_sleep(200);
    INA_TEST_ASSERT_SUCCEED(ina_time_stopwatch_create(&w, 1, -1));
    INA_TEST_ASSERT_NOT_NULL(w);
    INA_TEST_ASSERT_EQUAL_INTEGER(0, w->tv->next_stamp);
    INA_TEST_ASSERT_EQUAL_FLOATING(INA_TIME_MAX_STAMPS, w->tv->max_stamps);
    INA_TEST_ASSERT_SUCCEED(ina_time_stopwatch_start(w, &start_ts));
    INA_TEST_ASSERT_EQUAL_FLOATING(0, w->tv->sec_duration);
    INA_TEST_ASSERT_NULL(w->ts);
    INA_TEST_ASSERT_SUCCEED(ina_time_stopwatch_started(w));
    ina_time_sleep(100);
    INA_TEST_ASSERT_FAILED(ina_time_stopwatch_valid(w));
    ina_time_sleep(1);
    INA_TEST_ASSERT_SUCCEED(ina_time_stopwatch_stamp(w, NULL, NULL));
    ina_time_sleep(100);
    INA_TEST_ASSERT_SUCCEED(ina_time_stopwatch_stamp(w, NULL, NULL));
    INA_TEST_ASSERT_SUCCEED(ina_time_stopwatch_stop(w));
    INA_TEST_ASSERT_SUCCEED(ina_time_stopwatch_valid(w));
    while (INA_SUCCEED(ina_time_stopwatch_read_stamp(w, &i))) {
	INA_TEST_MSG("Stamp %ld, %.10f", i, w->ts->sec_duration);
        ++i;
    }
    INA_TEST_MSG("Duration in secs %.10f", w->tv->sec_duration);
    INA_TEST_ASSERT_SUCCEED(ina_time_stopwatch_destroy(&w));
    INA_TEST_ASSERT_NULL(w);
}


INA_TEST_SKIP(time, stopwatch_startime_rdtsc) 
{
    struct timeval tv_start;
    ina_stopwatch_t *w = NULL;
    ina_time_tsc_t start_ts;
    int64_t i = 0;


    #if !defined (INA_OS_WIN32) && !defined(INA_OS_OSX)
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(0, &mask);
    sched_setaffinity(0, sizeof(mask), &mask);
    #endif

    ina_time_tsc_enable_rdtsc();
    gettimeofday(&tv_start, NULL);
    ina_time_read_tsc_clock(&start_ts);
    ina_time_sleep(200);
    INA_TEST_ASSERT_SUCCEED(ina_time_stopwatch_create(&w, 1, -1));
    INA_TEST_ASSERT_NOT_NULL(w);
    INA_TEST_ASSERT_EQUAL_INTEGER(0, w->tv->next_stamp);
    INA_TEST_ASSERT_EQUAL_FLOATING(INA_TIME_MAX_STAMPS, w->tv->max_stamps);
    INA_TEST_ASSERT_SUCCEED(ina_time_stopwatch_start(w, &start_ts));
    INA_TEST_ASSERT_EQUAL_FLOATING(0, w->tv->sec_duration);
    INA_TEST_ASSERT_NULL(w->ts);
    INA_TEST_ASSERT_SUCCEED(ina_time_stopwatch_started(w));
    ina_time_sleep(100);
    INA_TEST_ASSERT_FAILED(ina_time_stopwatch_valid(w));
    ina_time_sleep(1);
    INA_TEST_ASSERT_SUCCEED(ina_time_stopwatch_stamp(w, NULL, NULL));
    ina_time_sleep(100);
    INA_TEST_ASSERT_SUCCEED(ina_time_stopwatch_stamp(w, NULL, NULL));
    INA_TEST_ASSERT_SUCCEED(ina_time_stopwatch_stop(w));
    INA_TEST_ASSERT_SUCCEED(ina_time_stopwatch_valid(w));
    while (INA_SUCCEED(ina_time_stopwatch_read_stamp(w, &i))) {
	INA_TEST_MSG("Stamp %ld, %.10f", i, w->ts->sec_duration);
        ++i;
    }
    INA_TEST_MSG("Duration in secs %.10f", w->tv->sec_duration);
    INA_TEST_ASSERT_SUCCEED(ina_time_stopwatch_destroy(&w));
    INA_TEST_ASSERT_NULL(w);
    ina_time_tsc_disable_rdtsc();
}


INA_TEST(time,backend) 
{
    ina_time_sys_info_t info;

    INA_TEST_ASSERT_SUCCEED(ina_time_sys_backend_info(&info));
#ifdef INA_MBTIME_ENABLED
    INA_TEST_ASSERT_TRUE(strncmp("HW backend:", ina_str_cstr(info.backend_name), 12) == 0);
#else
    #ifdef INA_OS_WIN32
    INA_TEST_ASSERT_EQUAL_STR("OS backend: GetSystemTimeAsFileTime()",
                     ina_str_cstr(info.backend_name));   
    #else                    
    INA_TEST_ASSERT_EQUAL_STR("OS backend: gettimeofday()",
                    ina_str_cstr(info.backend_name));
    #endif
#endif
}
 
INA_TEST(time,read_clock) 
{
    struct timeval tv;
    ina_time_t *t;
    time_t ms;
    time_t secs;
    time_t secs2;
    long us = 0;
    long us2 = 0;

    INA_TEST_ASSERT_SUCCEED(ina_time_sys_new(&t));

    ms = 0;
    secs = 0;
    secs2 = 0;

    gettimeofday(&tv, NULL);
    INA_TEST_ASSERT_SUCCEED(ina_time_read_sys_clock(t));
	INA_TEST_ASSERT_SUCCEED(ina_time_sys_seconds_micros(t, &secs, &us));
    INA_TEST_ASSERT_SUCCEED(ina_time_sys_seconds_micros(t, &secs2, &us2));
    INA_TEST_ASSERT(secs > 0);
    INA_TEST_ASSERT_EQUAL_INTEGER(secs, secs2);
    INA_TEST_ASSERT_EQUAL_INTEGER(tv.tv_sec, secs);
    ms = us/1000;
    INA_TEST_ASSERT(ms > 0);
    INA_TRACE3("tv.tv_usec=%d", tv.tv_usec);
    INA_TRACE3("ms=%ld", ms);
    INA_TEST_ASSERT_EQUAL_INTEGER(tv.tv_usec/1000, ms);

    INA_TEST_ASSERT_SUCCEED(ina_time_sys_free(&t));
}

INA_TEST(time, tsc_millis)
{
    ina_time_tsc_t *t;
    time_t now_millis = 0;
    time_t now_sec = time(NULL);

    INA_TEST_ASSERT_SUCCEED(ina_time_tsc_new(&t));
    INA_TEST_ASSERT_SUCCEED(ina_time_read_tsc_clock(t));
    INA_TEST_ASSERT_SUCCEED(ina_time_tsc_millis(t, &now_millis));
    INA_TEST_ASSERT_EQUAL_INTEGER(now_sec, (time_t)now_millis/1000);
    INA_TEST_ASSERT_SUCCEED(ina_time_tsc_free(&t));
}

#if !defined (INA_OS_WIN32) && !defined(INA_OS_OSX)
INA_TEST_SKIP(time_tsc,read_tsc)
{
    struct timespec test;
    ina_time_tsc_t t;
    time_t sec;
    long nanos;
    double u1, u2 = 0;
    double d = 0;
    int i;
    const char *msg = "Test may fail, because RDTSC can be different from HPET, "
                      "but should not be more then couple of micro-seconds";

    INA_TEST_MSG("%s", msg);

    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(0, &mask);
    sched_setaffinity(0, sizeof(mask), &mask);
    
    INA_TEST_ASSERT_SUCCEED(ina_time_tsc_enable_rdtsc());
    clock_gettime(CLOCK_REALTIME, &test); 

    INA_TEST_ASSERT_SUCCEED(ina_time_read_tsc_clock(&t));
    INA_TEST_ASSERT_SUCCEED(ina_time_tsc_seconds_nanos(&t, &sec, &nanos));
    u1 = test.tv_nsec / 1000;
    u2 = nanos / 1000;
 
    INA_TEST_ASSERT_EQUAL_INTEGER(test.tv_sec, sec);
    d = u1 - u2;
    if (abs(d) > 1) {
        INA_TEST_ASSERT_SUCCEED(ina_time_tsc_disable_rdtsc());
        INA_TEST_MSG("Difference was %f ms (> +- 1ms)", d); 
    }

    INA_TEST_ASSERT_TRUE(abs(d) <= 1);
    
    for (i = 0; i < 1000; i++) {
        clock_gettime(CLOCK_REALTIME, &test);
        ina_time_read_tsc_clock(&t);
        ina_time_tsc_seconds_nanos(&t, &sec, &nanos);
        u1 = test.tv_nsec / 1000;
        u2 = nanos / 1000;
        d = u1 - u2;
        if (abs(d) > 1) {
            break;
        }
    }
    INA_TEST_ASSERT_SUCCEED(ina_time_tsc_disable_rdtsc());
    if (abs(d) > 1) {
        INA_TEST_MSG("Difference was %f ms (> +- 1ms) at %d cycle", d, i);
    }
    INA_TEST_ASSERT_TRUE(abs(d) <= 1);
}
#endif


INA_TEST_DATA(time_ipc) {
    ina_stopwatch_t *w;
    ina_test_hid_t hid;
};

INA_TEST_SETUP(time_ipc) {
    INA_TEST_HELPER_INVOKE(&data->hid, time_ipc, stopwatch_create, 
        INA_NUM2STR(888),
	NULL);
}

INA_TEST_TEARDOWN(time_ipc) 
{
    INA_TEST_HELPER_TERMINATE(&data->hid);
    if (data->w) {
        ina_time_stopwatch_destroy(&data->w);
    }
}

INA_TEST_FIXTURE(time_ipc, stopwatch_open) {
    int64_t c = 0;

    /* We need to wait that the heler has done his work */
    ina_time_sleep(500);

    INA_TEST_ASSERT_SUCCEED(INA_TIME_STOPWATCH_OPEN(&data->w, 888));
    INA_TEST_ASSERT_SUCCEED(ina_time_stopwatch_started(data->w));

    while (INA_SUCCEED(ina_time_stopwatch_read_stamp(data->w, &c))) {
        INA_TEST_ASSERT_NOT_NULL(data->w->ts);
        INA_TEST_MSG("stamp %lld: %.10f", c, data->w->ts->msec_duration);
        if (c == 0) {
            INA_TEST_ASSERT_EQUAL_STR("", data->w->ts->user_data1);
            INA_TEST_ASSERT_EQUAL_STR("", data->w->ts->user_data2);
        }
        else if (c == 1) {
            INA_TEST_ASSERT_EQUAL_STR("user_data1", data->w->ts->user_data1);
            INA_TEST_ASSERT_EQUAL_STR("", data->w->ts->user_data2);
        }
        else if (c == 2) {
            INA_TEST_ASSERT_EQUAL_STR("user_data1", data->w->ts->user_data1);
            INA_TEST_ASSERT_EQUAL_STR("user_data2", data->w->ts->user_data2);
        }
        else if (c == 3) {
            INA_TEST_ASSERT_EQUAL_STR("", data->w->ts->user_data1);
            INA_TEST_ASSERT_EQUAL_STR("", data->w->ts->user_data2);
        }
        ++c;
    }
    INA_TEST_ASSERT_EQUAL_INTEGER(c, 4);

    INA_TEST_ASSERT_SUCCEED(ina_time_stopwatch_stop(data->w));
    ina_time_sleep(500); /* Wait child is exit */
    INA_TEST_ASSERT_FAILED(ina_time_stopwatch_started(data->w));
}


INA_TEST_DATA(time_ipc_rdtsc) {
    ina_stopwatch_t *w;
    ina_test_hid_t hid;
};

INA_TEST_SETUP(time_ipc_rdtsc) {
    #if !defined (INA_OS_WIN32) && !defined(INA_OS_OSX)
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(0, &mask);
    sched_setaffinity(0, sizeof(mask), &mask);
    #endif
    ina_time_tsc_enable_rdtsc();
    ina_time_sleep(3000);
    INA_TEST_HELPER_INVOKE(&data->hid, time_ipc_rdtsc, stopwatch_create_rdtsc, 
        INA_NUM2STR(889),
    NULL);
}

INA_TEST_TEARDOWN(time_ipc_rdtsc) 
{
    INA_TEST_HELPER_TERMINATE(&data->hid);
    ina_time_tsc_disable_rdtsc();
    if (data->w) {
        ina_time_stopwatch_destroy(&data->w);   
    }
}

#if !defined (INA_OS_WIN32) && !defined(INA_OS_OSX)
INA_TEST_FIXTURE(time_ipc_rdtsc, stopwatch_open_rdtsc) {
    int64_t c = 0;
    ina_time_tsc_t time;
    char user_data2[INA_TIME_MAX_USERDATA_LEN];
    double msec_duration = 0;
    double msec_duration2 = 0;

    /* We need to wait that the helper has done his work */
    ina_time_sleep(1000);

    INA_TEST_ASSERT_SUCCEED(INA_TIME_STOPWATCH_OPEN(&data->w, 889));
    INA_TEST_ASSERT_SUCCEED(ina_time_stopwatch_started(data->w));   

    clock_gettime(CLOCK_MONOTONIC_RAW, &time.tp);
    msec_duration = (time.tp.tv_sec + time.tp.tv_nsec / 1000000000.0)*1000.0;
    sprintf(user_data2, "%.10f", msec_duration);
    INA_TEST_ASSERT_SUCCEED(INA_TIME_STOPWATCH_STAMP2(data->w, "test", user_data2));
    ina_time_sleep(3);
    clock_gettime(CLOCK_MONOTONIC_RAW, &time.tp);
    msec_duration = (time.tp.tv_sec + time.tp.tv_nsec / 1000000000.0)*1000.0;
    sprintf(user_data2, "%.10f", msec_duration);
    INA_TEST_ASSERT_SUCCEED(INA_TIME_STOPWATCH_STAMP2(data->w, "test", user_data2));

    while (INA_SUCCEED(ina_time_stopwatch_read_stamp(data->w, &c))) {
        INA_TEST_ASSERT_NOT_NULL(data->w->ts);
        msec_duration2 = atof(data->w->ts->user_data2);
        INA_TEST_MSG("stamp %lld: %.10f ms - %.10f ms = %.10f us (%s)", c, data->w->ts->msec_duration, 
            msec_duration2-msec_duration, (data->w->ts->msec_duration-(msec_duration2-msec_duration))*1000,
             data->w->ts->user_data1);
        msec_duration = msec_duration2;
        ++c;
    }
    INA_TEST_ASSERT_EQUAL_INTEGER(c, 6);
    INA_TEST_ASSERT_SUCCEED(ina_time_stopwatch_stop(data->w));
    ina_time_sleep(500); /* Wait child is exit */
    INA_TEST_ASSERT_FAILED(ina_time_stopwatch_started(data->w));
}
#endif
