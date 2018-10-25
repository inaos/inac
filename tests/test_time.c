/*
 * Copyright INAOS GmbH, Thalwil, 2012-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
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
    ina_stopwatch_ts_t *ts;

    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_new(3, -1, &w));
    INA_TEST_ASSERT_NOT_NULL(w);
    INA_TEST_ASSERT_FAILED(ina_stopwatch_started(w));
    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_start(w, NULL));
    while (c--) {
        INA_TEST_ASSERT_SUCCEED(ina_stopwatch_stamp(w, "1", "2"));
        ina_time_sleep(100);
    }
    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_stop(w));
    c = 0;
    while (INA_SUCCEED(ina_stopwatch_read_stamp(w, &c, &ts))) {
        INA_TEST_ASSERT_NOT_NULL(ts);
        INA_TEST_ASSERT_TRUE(msec_duration < ts->duration);
        ++c;
    }
    INA_TEST_ASSERT_EQUAL_TIME_T(10, c);
    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_free(&w));
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
    ina_time_tsc_t *ts1;
    ina_time_tsc_t *ts2;

    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_new(1, -1, &w1));
    INA_TEST_ASSERT_NOT_NULL(w1);
    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_new(2, -1, &w2));
    INA_TEST_ASSERT_NOT_NULL(w2);
    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_start(w1, NULL));
    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_start(w2, NULL));
    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_stop(w1));
    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_stop(w2));
    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_start_time(w1, &ts1));
    INA_TEST_ASSERT_SUCCEED(ina_time_tsc_seconds_nanos(ts1, &sec1, &nano1));
    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_start_time(w2, &ts2));
    INA_TEST_ASSERT_SUCCEED(ina_time_tsc_seconds_nanos(ts2, &sec2, &nano2));
    INA_TEST_ASSERT_EQUAL_INT(sec1, sec2);
    INA_TEST_ASSERT_TRUE(nano1< nano2);
    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_free(&w1));
    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_free(&w2));
} 
#endif

INA_TEST(time, stopwatch) 
{
    struct timeval tv_start;
    ina_stopwatch_t *w = NULL;
    double duration;

    gettimeofday(&tv_start, NULL);
    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_new(1, -1, &w));
    INA_TEST_ASSERT_NOT_NULL(w);
    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_start(w, NULL));

    INA_TEST_ASSERT_FAILED(ina_stopwatch_duration(w, &duration));
    INA_TEST_ASSERT_EQUAL_FLOATING(0, duration);
    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_started(w));
    ina_time_sleep(1);
    INA_TEST_ASSERT_FAILED(ina_stopwatch_valid(w));
    ina_time_sleep(1);
    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_stop(w));
    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_valid(w));
    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_free(&w));
    INA_TEST_ASSERT_NULL(w);
}

INA_TEST(time, stopwatch_startime) 
{
    struct timeval tv_start;
    ina_stopwatch_t *w = NULL;
    ina_time_tsc_t start_ts;
    int64_t i = 0;
    double duration = 0;
    ina_stopwatch_ts_t *ts;

    gettimeofday(&tv_start, NULL);
    ina_time_read_tsc_clock(&start_ts);
    ina_time_sleep(200);
    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_new(1, -1, &w));
    INA_TEST_ASSERT_NOT_NULL(w);
    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_start(w, &start_ts));
    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_duration(w, &duration));
    INA_TEST_ASSERT_EQUAL_FLOATING(0, duration);
    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_started(w));
    ina_time_sleep(100);
    INA_TEST_ASSERT_FAILED(ina_stopwatch_valid(w));
    ina_time_sleep(1);
    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_stamp(w, NULL, NULL));
    ina_time_sleep(100);
    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_stamp(w, NULL, NULL));
    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_stop(w));
    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_valid(w));
    while (INA_SUCCEED(ina_stopwatch_read_stamp(w, &i, &ts))) {
	INA_TEST_MSG("Stamp %ld, %.10f", i, ts->duration);
        ++i;
    }
    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_duration(w, &duration));
    INA_TEST_MSG("Duration in secs %.10f", ts->duration);
    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_free(&w));
    INA_TEST_ASSERT_NULL(w);
}


INA_TEST_SKIP(time, stopwatch_startime_rdtsc) 
{
    struct timeval tv_start;
    ina_stopwatch_t *w = NULL;
    ina_time_tsc_t start_ts;
    int64_t i = 0;
    ina_stopwatch_ts_t *ts;
    double duration;

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
    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_new(1, -1, &w));
    INA_TEST_ASSERT_NOT_NULL(w);
    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_start(w, &start_ts));
    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_started(w));
    ina_time_sleep(100);
    INA_TEST_ASSERT_FAILED(ina_stopwatch_valid(w));
    ina_time_sleep(1);
    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_stamp(w, NULL, NULL));
    ina_time_sleep(100);
    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_stamp(w, NULL, NULL));
    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_stop(w));
    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_valid(w));
    while (INA_SUCCEED(ina_stopwatch_read_stamp(w, &i, &ts))) {
    INA_TEST_MSG("Stamp %ld, %.10f", i,ts->duration);
        ++i;
    }
    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_duration(w, &duration));
    INA_TEST_MSG("Duration in secs %.10f", duration);
    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_free(&w));
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
#  ifdef INA_OS_WIN32
    INA_TEST_ASSERT_EQUAL_STR("OS backend: GetSystemTimeAsFileTime()",
                     ina_str_cstr(info.backend_name));
#  else
    INA_TEST_ASSERT_EQUAL_STR("OS backend: gettimeofday()",
                    ina_str_cstr(info.backend_name));
#  endif
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
    INA_TEST_ASSERT_EQUAL_INT64(secs, secs2);
    INA_TEST_ASSERT_EQUAL_INT64(tv.tv_sec, secs);
    ms = us/1000;
    INA_TEST_ASSERT(ms > 0);
    INA_TRACE3("tv.tv_usec=%d", tv.tv_usec);
    INA_TRACE3("ms=%ld", ms);
    INA_TEST_ASSERT_EQUAL_INT64(tv.tv_usec/1000, ms);

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
    INA_TEST_ASSERT_EQUAL_INT64(now_sec, (time_t)now_millis/1000);
    ina_time_tsc_free(&t);
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
 
    INA_TEST_ASSERT_EQUAL_INT(test.tv_sec, sec);
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
        ina_stopwatch_free(&data->w);
    }
}

INA_TEST_FIXTURE(time_ipc, stopwatch_open) {
    int64_t c = 0;
    ina_stopwatch_ts_t *ts;

    /* We need to wait that the heler has done his work */
    ina_time_sleep(500);

    INA_TEST_ASSERT_SUCCEED(INA_STOPWATCH_OPEN(888, &data->w));
    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_started(data->w));

    while (INA_SUCCEED(ina_stopwatch_read_stamp(data->w, &c, &ts))) {
        INA_TEST_ASSERT_NOT_NULL(ts);
        INA_TEST_MSG("stamp %lld: %.10f", c, ts->duration);
        if (c == 0) {
            INA_TEST_ASSERT_EQUAL_STR("", ts->user_data1);
            INA_TEST_ASSERT_EQUAL_STR("", ts->user_data2);
        }
        else if (c == 1) {
            INA_TEST_ASSERT_EQUAL_STR("user_data1", ts->user_data1);
            INA_TEST_ASSERT_EQUAL_STR("", ts->user_data2);
        }
        else if (c == 2) {
            INA_TEST_ASSERT_EQUAL_STR("user_data1", ts->user_data1);
            INA_TEST_ASSERT_EQUAL_STR("user_data2", ts->user_data2);
        }
        else if (c == 3) {
            INA_TEST_ASSERT_EQUAL_STR("", ts->user_data1);
            INA_TEST_ASSERT_EQUAL_STR("", ts->user_data2);
        }
        ++c;
    }
    INA_TEST_ASSERT_EQUAL_INT64(c, 4);

    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_stop(data->w));
    ina_time_sleep(500); /* Wait child is exit */
    INA_TEST_ASSERT_FAILED(ina_stopwatch_started(data->w));
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
        ina_stopwatch_free(&data->w);
    }
}

#if !defined (INA_OS_WIN32) && !defined(INA_OS_OSX)
INA_TEST_FIXTURE(time_ipc_rdtsc, stopwatch_open_rdtsc) {
    int64_t c = 0;
    ina_time_tsc_t time;
    char user_data2[INA_STOPWATCH_MAX_STAMPS];
    double msec_duration = 0;
    double msec_duration2 = 0;
    ina_stopwatch_ts_t *ts;

    /* We need to wait that the helper has done his work */
    ina_time_sleep(1000);

    INA_TEST_ASSERT_SUCCEED(INA_STOPWATCH_OPEN(889, &data->w));
    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_started(data->w));

    clock_gettime(CLOCK_MONOTONIC_RAW, &time.tp);
    msec_duration = (time.tp.tv_sec + time.tp.tv_nsec / 1000000000.0)*1000.0;
    sprintf(user_data2, "%.10f", msec_duration);
    INA_TEST_ASSERT_SUCCEED(INA_STOPWATCH_STAMP2(data->w, "test", user_data2));
    ina_time_sleep(3);
    clock_gettime(CLOCK_MONOTONIC_RAW, &time.tp);
    msec_duration = (time.tp.tv_sec + time.tp.tv_nsec / 1000000000.0)*1000.0;
    sprintf(user_data2, "%.10f", msec_duration);
    INA_TEST_ASSERT_SUCCEED(INA_STOPWATCH_STAMP2(data->w, "test", user_data2));

    while (INA_SUCCEED(ina_stopwatch_read_stamp(data->w, &c, &ts))) {
        INA_TEST_ASSERT_NOT_NULL(ts);
        msec_duration2 = atof(ts->user_data2);
        INA_TEST_MSG("stamp %lld: %.10f ms - %.10f ms = %.10f us (%s)", c, ts->duration,
            msec_duration2-msec_duration, (ts->duration-(msec_duration2-msec_duration))*1000,
             ts->user_data1);
        msec_duration = msec_duration2;
        ++c;
    }
    INA_TEST_ASSERT_EQUAL_INT64(c, 6);
    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_stop(data->w));
    ina_time_sleep(500); /* Wait child is exit */
    INA_TEST_ASSERT_FAILED(ina_stopwatch_started(data->w));
}
#endif
