/*
 * Copyright (c) 2012, INAOS GmbH
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

INA_TEST(timer,init_destroy)
{
    ina_timer_t *t;

    t = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_timer_init(&t));
    INA_TEST_ASSERT_NOT_NULL(t);
    INA_TEST_ASSERT_SUCCEED(ina_timer_destroy(&t));
    INA_TEST_ASSERT_NULL(t);
}
 
INA_TEST(timer, event)
{
    ina_timer_t *t;
    ina_time_event_t *e1;
    ina_time_event_t *e2;

    t = NULL;
    e1 = NULL;
    e2 = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_timer_init(&t));
    INA_TEST_ASSERT_NOT_NULL(t);
    INA_TEST_ASSERT_SUCCEED(ina_timer_destroy(&t));
    INA_TEST_ASSERT_NULL(t);
    INA_TEST_ASSERT_SUCCEED(ina_timer_init(&t));
    INA_TEST_ASSERT_NOT_NULL(t);
    e1 = ina_timer_create_event(t, 900);
    INA_TEST_ASSERT_SUCCEED(ina_err_peek());
    INA_TEST_ASSERT_NOT_NULL(e1);
    ina_time_sleep(100);
    e2 = ina_timer_next_event(t);
    INA_TEST_ASSERT_SUCCEED(ina_err_peek());
    INA_TEST_ASSERT_NULL(e2);
    ina_time_sleep(1000);
    e2 = ina_timer_next_event(t);
    INA_TEST_ASSERT_SUCCEED(ina_err_peek());
    INA_TEST_ASSERT_NOT_NULL(e2);
    INA_TEST_ASSERT_SAME(e2, e1);
}
 
INA_TEST(timer, event_rdtsc)
{
    ina_timer_t *t;
    ina_time_event_t *e1;
    ina_time_event_t *e2;
    struct timeval tv;
    time_t nowtime;
    struct tm *nowtm;
    char tmbuf[64];


    t = NULL;
    e1 = NULL;
    e2 = NULL;
    #if !defined (INA_OS_WIN32) && !defined(INA_OS_OSX)
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(0, &mask);
    sched_setaffinity(0, sizeof(mask), &mask);
    #endif
    INA_TEST_ASSERT_SUCCEED(ina_timer_init(&t));
    INA_TEST_ASSERT_NOT_NULL(t);
    INA_TEST_ASSERT_SUCCEED(ina_timer_use_rdtsc(t, INA_YES));
    ina_time_sleep(100);
    e1 = ina_timer_create_event(t, 900);
    INA_TEST_ASSERT_SUCCEED(ina_err_peek());
    INA_TEST_ASSERT_NOT_NULL(e1);
    ina_time_sleep(100);
    e2 = ina_timer_next_event(t);
    INA_TEST_ASSERT_SUCCEED(ina_err_peek());
    INA_TEST_ASSERT_NULL(e2);
    ina_time_sleep(2000);
    e2 = ina_timer_next_event(t);
    INA_TEST_ASSERT_SUCCEED(ina_err_peek());
    INA_TEST_ASSERT_NOT_NULL(e2);
    INA_TEST_ASSERT_SAME(e2, e1);

    INA_TEST_ASSERT_SUCCEED(ina_timer_delete_event(t, e1));
    gettimeofday(&tv, NULL);
    nowtime = tv.tv_sec;
    nowtm = localtime(&nowtime);
    strftime(tmbuf, sizeof tmbuf, "Timer event started at %Y-%m-%d %H:%M:%S", nowtm);
    INA_TEST_MSG("%s", tmbuf);
    e1 = ina_timer_create_event(t, 30*1000);
    while (ina_timer_next_event(t) == NULL);
    gettimeofday(&tv, NULL);
    nowtime = tv.tv_sec;
    nowtm = localtime(&nowtime);
    strftime(tmbuf, sizeof tmbuf, "Timer event fired at %Y-%m-%d %H:%M:%S", nowtm);
    INA_TEST_MSG("%s", tmbuf);   
}
