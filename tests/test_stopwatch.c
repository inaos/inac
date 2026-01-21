/*
 * Copyright INAOS GmbH, Thalwil, 2012-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>

#ifndef INA_OS_WINDOWS
#define _GNU_SOURCE  
#include <sched.h>
#endif

#if !defined(CLOCK_MONOTONIC_RAW)
#define CLOCK_MONOTONIC_RAW CLOCK_MONOTONIC
#endif

INA_TEST(stopwatch ,time_stamp)
{
    ina_stopwatch_t *w = NULL;
    int64_t c = 10;
    double msec_duration = 0;
    ina_stopwatch_ts_t *ts;
    INA_UNUSED(data);

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
    ina_stopwatch_free(&w);
    INA_TEST_ASSERT_NULL(w);
}

#if !defined (INA_OS_WINDOWS) && !defined(INA_OS_OSX)
INA_TEST(stopwatch, two_stopwatches)
{
    ina_stopwatch_t *w1 = NULL;
    ina_stopwatch_t *w2 = NULL;
    time_t sec1  = 0;
    time_t sec2  = 0; 
    long nano1 = 0;
    long nano2 = 0;
    ina_time_tsc_t *ts1;
    ina_time_tsc_t *ts2;
    INA_UNUSED(data);

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
    ina_stopwatch_free(&w1);
    INA_ASSERT_NULL(w1);
    ina_stopwatch_free(&w2);
    INA_ASSERT_NULL(w2);
} 
#endif

INA_TEST(stopwatch, stopwatch)
{
    struct timeval tv_start;
    ina_stopwatch_t *w = NULL;
    double duration = 0;
    INA_UNUSED(data);

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
    ina_stopwatch_free(&w);
    INA_TEST_ASSERT_NULL(w);
}

INA_TEST(stopwatch, stopwatch_startime)
{
    struct timeval tv_start;
    ina_stopwatch_t *w = NULL;
    ina_time_tsc_t start_ts;
    int64_t i = 0;
    double duration = 0;
    ina_stopwatch_ts_t *ts;
    INA_UNUSED(data);

    gettimeofday(&tv_start, NULL);
    ina_time_read_tsc_clock(&start_ts);
    ina_time_sleep(200);
    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_new(1, -1, &w));
    INA_TEST_ASSERT_NOT_NULL(w);
    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_start(w, &start_ts));
    INA_TEST_ASSERT_FAILED(ina_stopwatch_duration(w, &duration));
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
    ina_stopwatch_free(&w);
    INA_TEST_ASSERT_NULL(w);
}


INA_TEST_SKIP(stopwatch, stopwatch_startime_rdtsc)
{
    struct timeval tv_start;
    ina_stopwatch_t *w = NULL;
    ina_time_tsc_t start_ts;
    int64_t i = 0;
    ina_stopwatch_ts_t *ts;
    double duration;
    INA_UNUSED(data);

#if !defined (INA_OS_WINDOWS) && !defined(INA_OS_OSX)
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
    ina_stopwatch_free(&w);
    INA_TEST_ASSERT_NULL(w);
    ina_time_tsc_disable_rdtsc();
}



INA_TEST_DATA(stopwatch_ipc) {
    ina_stopwatch_t *w;
    ina_test_hid_t hid;
};

INA_TEST_SETUP(stopwatch_ipc) {
    INA_TEST_HELPER_INVOKE(&data->hid, stopwatch_ipc, stopwatch_create,
        INA_NUM2STR(888),
        NULL);
}

INA_TEST_TEARDOWN(stopwatch_ipc)
{
    INA_TEST_HELPER_TERMINATE(&data->hid);
    ina_stopwatch_free(&data->w);
}

INA_TEST_FIXTURE(stopwatch_ipc, stopwatch_open) {
    int64_t c = 0;
    ina_stopwatch_ts_t *ts;

    /* We need to wait that the helper has done his work */
    ina_time_sleep(2000);

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
            INA_TEST_ASSERT_EQUAL_STR("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", ts->user_data1);
            INA_TEST_ASSERT_EQUAL_STR("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", ts->user_data2);
        }
        ++c;
    }
    INA_TEST_ASSERT_EQUAL_INT64(c, 4);

    INA_TEST_ASSERT_SUCCEED(ina_stopwatch_stop(data->w));
    ina_time_sleep(500); /* Wait child is exit */
    INA_TEST_ASSERT_FAILED(ina_stopwatch_started(data->w));
}


INA_TEST_DATA(stopwatch_ipc_rdtsc) {
    ina_stopwatch_t *w;
    ina_test_hid_t hid;
};

INA_TEST_SETUP(stopwatch_ipc_rdtsc) {
#if !defined (INA_OS_WINDOWS) && !defined(INA_OS_OSX)
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(0, &mask);
    sched_setaffinity(0, sizeof(mask), &mask);
#endif
    ina_time_tsc_enable_rdtsc();
    ina_time_sleep(3000);
    INA_TEST_HELPER_INVOKE(&data->hid, stopwatch_ipc_rdtsc, stopwatch_create_rdtsc, 
        INA_NUM2STR(889),
    NULL);
}

INA_TEST_TEARDOWN(stopwatch_ipc_rdtsc)
{
    INA_TEST_HELPER_TERMINATE(&data->hid);
    ina_time_tsc_disable_rdtsc();
    ina_stopwatch_free(&data->w);
}

#if !defined (INA_OS_WINDOWS) && !defined(INA_OS_OSX)
INA_TEST_FIXTURE(stopwatch_ipc_rdtsc, stopwatch_open_rdtsc) {
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
    snprintf(user_data2, INA_STOPWATCH_MAX_USERDATA_LEN-1,"%.10f", msec_duration);
    INA_TEST_ASSERT_SUCCEED(INA_STOPWATCH_STAMP2(data->w, "test", user_data2));
    ina_time_sleep(3);
    clock_gettime(CLOCK_MONOTONIC_RAW, &time.tp);
    msec_duration = (time.tp.tv_sec + time.tp.tv_nsec / 1000000000.0)*1000.0;
    snprintf(user_data2, INA_STOPWATCH_MAX_USERDATA_LEN-1, "%.10f", msec_duration);
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

static ina_rc_t foreach(void *data) {
    INA_UNUSED(data);
    return INA_SUCCESS;
}

INA_TEST(stopwatch, invalid_arguments)
{
    ina_stopwatch_t *w = NULL;
    ina_time_tsc_t *t = NULL;
    ina_stopwatch_ts_t *ts = NULL;
    int fake = 0;
    double duration = 0;
    int64_t index = 0;
    INA_UNUSED(data);

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_stopwatch_new(1, -1, NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_stopwatch_open(1, NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_stopwatch_started(NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_stopwatch_valid(NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_stopwatch_start(NULL, NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_stopwatch_stop(NULL));

    w = (ina_stopwatch_t*)&fake;
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_stopwatch_start_time(NULL, &t));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_stopwatch_start_time(w, NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_stopwatch_stop_time(NULL, &t));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_stopwatch_stop_time(w, NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_stopwatch_duration(NULL, &duration));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_stopwatch_duration(w, NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_stopwatch_read_stamp(NULL, &index, &ts));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_stopwatch_read_stamp(w, NULL, &ts));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_stopwatch_read_stamp(w, &index, NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_stopwatch_foreach_stamp(NULL, foreach));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_stopwatch_foreach_stamp(w, NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_stopwatch_stamp(NULL, "u1", "u2"));
}
