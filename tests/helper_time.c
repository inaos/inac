/* Copyright (c) 2013, INAOS GmbH
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
#include <libinac/lib.h>

#if !defined(CLOCK_MONOTONIC_RAW)
    #define CLOCK_MONOTONIC_RAW CLOCK_MONOTONIC
#endif
 
/* 
 * Create a stop watch with an given ID, makes 3 time stamps each 10 ms
 * beetween.
 */
INA_TEST_HELPER(time_ipc, stopwatch_create) {
    int32_t id;
    ina_stopwatch_t *w = NULL;
    char user_data[INA_TIME_MAX_USERDATA_LEN+10];

    INA_TEST_HELPER_CHECK_ARGC(1);
    id = INA_TEST_HELPER_IARG(0);

    if (!INA_SUCCEED(INA_TIME_STOPWATCH_NEW(&w, id, -1))) {
        INA_TEST_HELPER_SET_RC(ina_err_get_last_rc());
        return;
    }

    ina_mem_set(&user_data, 'a', INA_TIME_MAX_USERDATA_LEN+8);
    user_data[INA_TIME_MAX_USERDATA_LEN+9] = '\0';

    INA_TIME_STOPWATCH_START(w);
    ina_time_sleep(10);
    INA_TIME_STOPWATCH_STAMP(w);
    ina_time_sleep(10);
    INA_TIME_STOPWATCH_STAMP1(w, "user_data1");
    ina_time_sleep(10);
    INA_TIME_STOPWATCH_STAMP2(w, "user_data1", "user_data2");
    ina_time_sleep(10);
    INA_TIME_STOPWATCH_STAMP2(w, user_data, user_data);
    
    while (INA_SUCCEED(ina_time_stopwatch_started(w))) {
        ina_time_sleep(100);
    }
    INA_TIME_STOPWATCH_FREE(&w);
    INA_TEST_HELPER_SET_RC(INA_SUCCESS);
}

#if !defined (INA_OS_WIN32) && !defined(INA_OS_OSX)
/* 
 * Create a rdtsc stop watch with an given ID, makes 3 time stamps each 10 ms
 * beetween.
 */
INA_TEST_HELPER(time_ipc_rdtsc, stopwatch_create_rdtsc) {
    int32_t id;
    ina_stopwatch_t *w = NULL;
    ina_time_tsc_t t;

    INA_TEST_HELPER_CHECK_ARGC(1);
    id = INA_TEST_HELPER_IARG(0);

    ina_time_tsc_enable_rdtsc();

    if (!INA_SUCCEED(INA_TIME_STOPWATCH_CREATE(&w, id, -1))) {
        INA_TEST_HELPER_SET_RC(ina_err_get_last_rc());
        return;
    }
 
    ina_time_tsc_t time;
    double msec_duration = 0;
    char user_data2[INA_TIME_MAX_USERDATA_LEN];
    ina_time_read_tsc_clock(&t);    

    INA_TIME_STOPWATCH_START_EX(w, &t);
    ina_time_sleep(1000);
    ina_time_sleep(10);
    clock_gettime(CLOCK_MONOTONIC_RAW, &time.tp);
    msec_duration = (time.tp.tv_sec + time.tp.tv_nsec / 1000000000.0)*1000.0;
    sprintf(user_data2, "%.10f", msec_duration);
    INA_TIME_STOPWATCH_STAMP2(w, "helper", user_data2);
    ina_time_sleep(10);
    clock_gettime(CLOCK_MONOTONIC_RAW, &time.tp);
    msec_duration = (time.tp.tv_sec + time.tp.tv_nsec / 1000000000.0)*1000.0;
    sprintf(user_data2, "%.10f", msec_duration);
    INA_TIME_STOPWATCH_STAMP2(w, "helper", user_data2);
    ina_time_sleep(10);
    clock_gettime(CLOCK_MONOTONIC_RAW, &time.tp);
    msec_duration = (time.tp.tv_sec + time.tp.tv_nsec / 1000000000.0)*1000.0;
    sprintf(user_data2, "%.10f", msec_duration);
    INA_TIME_STOPWATCH_STAMP2(w, "helper", user_data2);
    ina_time_sleep(10);
    clock_gettime(CLOCK_MONOTONIC_RAW, &time.tp);
    msec_duration = (time.tp.tv_sec + time.tp.tv_nsec / 1000000000.0)*1000.0;
    sprintf(user_data2, "%.10f", msec_duration);
    INA_TIME_STOPWATCH_STAMP2(w, "helper", user_data2);
    
    while (INA_SUCCEED(ina_time_stopwatch_started(w))) {
        ina_time_sleep(10);
    }
    INA_TIME_STOPWATCH_DESTROY(&w);
    ina_time_tsc_disable_rdtsc();
    INA_TEST_HELPER_SET_RC(INA_SUCCESS);
}
#endif
