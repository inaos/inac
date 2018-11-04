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
