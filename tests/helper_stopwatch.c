/*
 * Copyright INAOS GmbH, Thalwil, 2013-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>

#if !defined(CLOCK_MONOTONIC_RAW)
#define CLOCK_MONOTONIC_RAW CLOCK_MONOTONIC
#endif
 
/* 
 * Create a stop watch with an given ID, makes 3 time stamps each 10 ms
 * beetween.
 */
INA_TEST_HELPER(stopwatch_ipc, stopwatch_create) {
    int32_t id;
    ina_stopwatch_t *w = NULL;
    char user_data[INA_STOPWATCH_MAX_STAMPS+10];

    INA_TEST_HELPER_CHECK_ARGC(1);
    id = INA_TEST_HELPER_IARG(0);

    if (!INA_SUCCEED(INA_STOPWATCH_NEW(id, -1, &w))) {
        INA_TEST_HELPER_SET_RC(ina_err_get_rc());
        return;
    }
    ina_mem_set(&user_data, 'a', INA_STOPWATCH_MAX_STAMPS+8);
    user_data[INA_STOPWATCH_MAX_STAMPS+9] = '\0';

    INA_STOPWATCH_START(w);
    ina_time_sleep(10);
    INA_STOPWATCH_STAMP(w);
    ina_time_sleep(10);
    INA_STOPWATCH_STAMP1(w, "user_data1");
    ina_time_sleep(10);
    INA_STOPWATCH_STAMP2(w, "user_data1", "user_data2");
    ina_time_sleep(10);
    INA_STOPWATCH_STAMP2(w, user_data, user_data);
    
    while (INA_SUCCEED(ina_stopwatch_started(w))) {
        ina_time_sleep(100);
    }
    INA_STOPWATCH_FREE(&w);
    INA_TEST_HELPER_SET_RC(INA_SUCCESS);
}

#if !defined (INA_OS_WIN32) && !defined(INA_OS_OSX)
/* 
 * Create a rdtsc stop watch with an given ID, makes 3 time stamps each 10 ms
 * beetween.
 */
INA_TEST_HELPER(stopwatch_ipc_rdtsc, stopwatch_create_rdtsc) {
    int32_t id;
    ina_stopwatch_t *w = NULL;
    ina_time_tsc_t t;

    INA_TEST_HELPER_CHECK_ARGC(1);
    id = INA_TEST_HELPER_IARG(0);

    ina_time_tsc_enable_rdtsc();

    if (!INA_SUCCEED(INA_STOPWATCH_NEW(id, -1, &w))) {
        INA_TEST_HELPER_SET_RC(ina_err_get_rc());
        return;
    }
 
    ina_time_tsc_t time;
    double msec_duration = 0;
    char user_data2[INA_STOPWATCH_MAX_STAMPS];
    ina_time_read_tsc_clock(&t);    

    INA_STOPWATCH_START_EX(w, &t);
    ina_time_sleep(1000);
    ina_time_sleep(10);
    clock_gettime(CLOCK_MONOTONIC_RAW, &time.tp);
    msec_duration = (time.tp.tv_sec + time.tp.tv_nsec / 1000000000.0)*1000.0;
    snprintf(user_data2, INA_STOPWATCH_MAX_USERDATA_LEN-1, "%.10f", msec_duration);
    INA_STOPWATCH_STAMP2(w, "helper", user_data2);
    ina_time_sleep(10);
    clock_gettime(CLOCK_MONOTONIC_RAW, &time.tp);
    msec_duration = (time.tp.tv_sec + time.tp.tv_nsec / 1000000000.0)*1000.0;
    snprintf(user_data2, INA_STOPWATCH_MAX_USERDATA_LEN-1, "%.10f", msec_duration);
    INA_STOPWATCH_STAMP2(w, "helper", user_data2);
    ina_time_sleep(10);
    clock_gettime(CLOCK_MONOTONIC_RAW, &time.tp);
    msec_duration = (time.tp.tv_sec + time.tp.tv_nsec / 1000000000.0)*1000.0;
    snprintf(user_data2, INA_STOPWATCH_MAX_USERDATA_LEN-1, "%.10f", msec_duration);
    INA_STOPWATCH_STAMP2(w, "helper", user_data2);
    ina_time_sleep(10);
    clock_gettime(CLOCK_MONOTONIC_RAW, &time.tp);
    msec_duration = (time.tp.tv_sec + time.tp.tv_nsec / 1000000000.0)*1000.0;
    snprintf(user_data2, INA_STOPWATCH_MAX_USERDATA_LEN-1, "%.10f", msec_duration);
    INA_STOPWATCH_STAMP2(w, "helper", user_data2);
    
    while (INA_SUCCEED(ina_stopwatch_started(w))) {
        ina_time_sleep(10);
    }
    INA_STOPWATCH_FREE(&w);
    ina_time_tsc_disable_rdtsc();
    INA_TEST_HELPER_SET_RC(INA_SUCCESS);
}
#endif
