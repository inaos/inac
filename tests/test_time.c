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
#include <libinac/lib.h>


void test_time_two_stopwatches()
{
    ina_stopwatch_t *w1 = NULL;
    ina_stopwatch_t *w2 = NULL;
    time_t sec1  = 0;
    time_t sec2  = 0; 
    time_t msec1 = 0;
    time_t msec2 = 0;

    INA_TRACE_MSG("test_time_two_stopwatches");

    INA_ASSERT_SUCCEED(ina_time_stopwatch_create(1, &w1));
    INA_ASSERT_NOTNULL(w1);
    INA_ASSERT_EQUAL(1, w1->id);
    INA_ASSERT_NOTNULL(w1->data);
    INA_ASSERT_EQUAL(1, w1->data->c_ref);
    INA_ASSERT_SUCCEED(ina_time_stopwatch_create(2, &w2));
    INA_ASSERT_NOTNULL(w2);
    INA_ASSERT_EQUAL(2, w2->id);
    INA_ASSERT_NOTNULL(w2->data);
    INA_ASSERT_EQUAL(1, w2->data->c_ref);
    INA_ASSERT_SUCCEED(ina_time_stopwatch_start(w1));
    INA_ASSERT_SUCCEED(ina_time_stopwatch_start(w1));
    INA_ASSERT_SUCCEED(ina_time_stopwatch_stop(w1));
    INA_ASSERT_SUCCEED(ina_time_stopwatch_stop(w2));
    INA_ASSERT_SUCCEED(ina_time_get_seconds(&w1->data->start, &sec1)); 
    INA_ASSERT_SUCCEED(ina_time_get_seconds(&w2->data->start, &sec2));
    INA_ASSERT_EQUAL(sec1, sec2);
    INA_ASSERT_SUCCEED(ina_time_get_milliseconds(&w1->data->start, &msec1)); 
    INA_ASSERT_SUCCEED(ina_time_get_milliseconds(&w2->data->start, &msec2));
    INA_ASSERT_EQUAL(msec1, msec2);
    INA_ASSERT_SUCCEED(ina_time_stopwatch_destroy(&w1));
    INA_ASSERT_SUCCEED(ina_time_stopwatch_destroy(&w2));
} 

/*void test_time_stopwatch() 
{
    struct timeval tv_start;
    struct timeval tv_stop;
    ina_stopwatch_t sw;
    time_t t_start;
    time_t t_stop;

    INA_TRACE_MSG("test_time_stopwatch");

    gettimeofday(&tv_start, NULL);
    INA_ASSERT_SUCCEED(ina_time_stopwatch_start(&sw));
    ina_time_sleep(1);
    INA_ASSERT_SUCCEED(ina_time_stopwatch_stop(&sw));
    gettimeofday(&tv_stop, NULL);

    INA_ASSERT_SUCCEED(ina_time_get_seconds(&sw.start, &t_start));
    INA_ASSERT_EQUAL(tv_start.tv_sec, t_start);
    INA_ASSERT_SUCCEED(ina_time_get_milliseconds(&sw.start, &t_start));
    INA_ASSERT_EQUAL(tv_start.tv_usec/1000, t_start);
     
    INA_ASSERT_SUCCEED(ina_time_get_seconds(&sw.stop, &t_stop));
    INA_ASSERT_EQUAL(tv_stop.tv_sec, t_stop);
    INA_ASSERT_SUCCEED(ina_time_get_milliseconds(&sw.stop, &t_stop));
    INA_ASSERT_EQUAL(tv_stop.tv_usec/1000, t_stop);
}*/
 
/*void test_time_read_clock() 
{
    struct timeval tv;
    ina_time_t t;
    time_t ms;
    time_t secs;
    time_t secs2;

    INA_TRACE_MSG("test_time_read_clock");

    ms = 0;
    secs = 0;
    secs2 = 0;

    gettimeofday(&tv, NULL);
    INA_ASSERT_SUCCEED(ina_time_read_clock(&t));
    INA_ASSERT_SUCCEED(ina_time_get_seconds(&t, &secs));
    INA_ASSERT_SUCCEED(ina_time_get_seconds(&t, &secs2));
    INA_ASSERT(secs > 0);
    INA_ASSERT_EQUAL(secs, secs2);
    INA_ASSERT_EQUAL(tv.tv_sec, secs);
    INA_ASSERT_SUCCEED(ina_time_get_milliseconds(&t, &ms));
    INA_ASSERT(ms > 0);
    INA_TRACE3("tv.tv_usec=%d", tv.tv_usec);
    INA_TRACE3("ms=%ld", ms);
    INA_ASSERT_EQUAL(tv.tv_usec/1000, ms);
}*/
