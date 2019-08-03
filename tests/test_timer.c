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

INA_TEST(timer,new_free)
{
    ina_timer_t *t;
    INA_UNUSED(data);

    t = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_timer_new(&t));
    INA_TEST_ASSERT_NOT_NULL(t);
    ina_timer_free(&t);
    INA_TEST_ASSERT_NULL(t);
}
 
INA_TEST(timer, event)
{
    ina_timer_t *t;
    ina_timer_event_t *e1;
    ina_timer_event_t *e2;
    INA_UNUSED(data);

    t = NULL;
    e1 = NULL;
    e2 = NULL;
    ina_err_reset();
    INA_TEST_ASSERT_SUCCEED(ina_timer_new(&t));
    INA_TEST_ASSERT_NOT_NULL(t);
    ina_timer_free(&t);
    INA_TEST_ASSERT_NULL(t);
    INA_TEST_ASSERT_SUCCEED(ina_timer_new(&t));
    INA_TEST_ASSERT_NOT_NULL(t);
    INA_TEST_ASSERT_SUCCEED(ina_timer_event_new(t, 900,&e1));
    INA_TEST_ASSERT_NOT_NULL(e1);
    ina_time_sleep(100);
    INA_TEST_ASSERT_FAILED(ina_timer_next_event(t, &e2));
    INA_TEST_ASSERT_NULL(e2);
    ina_time_sleep(1000);
    INA_TEST_ASSERT_SUCCEED(ina_timer_next_event(t, &e2));
    INA_TEST_ASSERT_NOT_NULL(e2);
    INA_TEST_ASSERT_SAME(e2, e1);
    ina_timer_free(&t);
}

INA_TEST(timer, stress_test)
{
    ina_timer_t *t = NULL;
    ina_timer_event_t *e = NULL;
    int c = 0;
    INA_UNUSED(data);

    INA_TEST_ASSERT_SUCCEED(ina_timer_new(&t));
    INA_TEST_ASSERT_NOT_NULL(t);

    ina_timer_event_new(t, 3000, &e);
    for (c = 0; c < 1000000; c++) {
        ina_timer_event_t *ne;
        if (INA_SUCCEED(ina_timer_next_event(t, &ne))) {
            INA_TEST_ASSERT_SAME(e, ne);
        }
    }
    ina_timer_event_free(t, &e);
}

#ifndef INA_OS_WINDOWS
INA_TEST(timer, event_rdtsc)
{
    ina_timer_t *t;
    ina_timer_event_t *e1;
    ina_timer_event_t *e2;
    struct timeval tv;
    time_t nowtime;
    struct tm *nowtm;
    char tmbuf[64];
    INA_UNUSED(data);

    t = NULL;
    e1 = NULL;
    e2 = NULL;
#if !defined (INA_OS_WINDOWS) && !defined(INA_OS_OSX)
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(0, &mask);
    sched_setaffinity(0, sizeof(mask), &mask);
#endif
    INA_TEST_ASSERT_SUCCEED(ina_timer_new(&t));
    INA_TEST_ASSERT_NOT_NULL(t);
    INA_TEST_ASSERT_SUCCEED(ina_time_tsc_enable_rdtsc());
    ina_time_sleep(100);
    INA_TEST_ASSERT_SUCCEED(ina_timer_event_new(t, 900, &e1));
    INA_TEST_ASSERT_NOT_NULL(e1);
    ina_time_sleep(100);
    INA_TEST_ASSERT_FAILED(ina_timer_next_event(t, &e2));
    INA_TEST_ASSERT_NULL(e2);
    ina_time_sleep(2000);
    INA_TEST_ASSERT_SUCCEED(ina_timer_next_event(t, &e2));
    INA_TEST_ASSERT_NOT_NULL(e2);
    INA_TEST_ASSERT_SAME(e2, e1);

    ina_timer_event_free(t, &e1);
    INA_TEST_ASSERT_NULL(e1);
    gettimeofday(&tv, NULL);
    nowtime = tv.tv_sec;
    nowtm = localtime(&nowtime);
    strftime(tmbuf, sizeof tmbuf, "Timer event started at %Y-%m-%d %H:%M:%S", nowtm);
    INA_TEST_MSG("%s", tmbuf);
    INA_TEST_ASSERT_SUCCEED(ina_timer_event_new(t, 30*1000, &e1));
    while (INA_RC_ERROR(ina_timer_next_event(t, &e1)) == INA_ERR_TRY_AGAIN);
    gettimeofday(&tv, NULL);
    nowtime = tv.tv_sec;
    nowtm = localtime(&nowtime);
    strftime(tmbuf, sizeof tmbuf, "Timer event fired at %Y-%m-%d %H:%M:%S", nowtm);
    INA_TEST_MSG("%s", tmbuf);   
}
#endif

INA_TEST(timer, invalid_arguments)
{

    ina_timer_t *timer = NULL;
    ina_timer_event_t *event = NULL;
    time_t msec = 0;
    int id = 0;
    INA_UNUSED(data);

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_timer_new(NULL));

    INA_TEST_ASSERT_SUCCEED(ina_timer_new(&timer));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_timer_event_new(NULL, 100, &event));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_timer_event_new(NULL, 0, &event));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_timer_event_new(timer, 100, NULL));


    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_timer_event_new_with_time(NULL, 1, 1, &event));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_timer_event_new_with_time(timer, 0, 1, &event));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_timer_event_new_with_time(timer, 1, 0, &event));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_timer_event_new_with_time(timer, 1, 1, NULL));

    INA_TEST_ASSERT_SUCCEED(ina_timer_event_new(timer, 1, &event));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_timer_event_get_id(NULL, &id));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_timer_event_get_id(event, NULL));
    ina_timer_event_free(timer, &event);

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_timer_next_event(NULL, &event));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_timer_next_event(timer, NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_timer_next_event_with_time(NULL, 100,  &event));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_timer_next_event_with_time(timer, 100, NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_timer_time_to_next_event(NULL, &msec));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_timer_time_to_next_event(timer, NULL));

}