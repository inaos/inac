/*
 * Copyright INAOS GmbH, Thalwil, 2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <stdlib.h>
#include <libinac/lib.h>

static ina_timer_event_t *e1 = NULL;
static ina_timer_event_t *e2 = NULL;
static ina_timer_event_t *e3 = NULL;
static ina_timer_event_t *e4 = NULL;
static ina_timer_event_t *e5 = NULL;
static ina_timer_event_t *e6 = NULL;
static ina_timer_event_t *e7 = NULL;
static ina_timer_event_t *e8 = NULL;
static ina_timer_event_t *e9 = NULL;
static ina_timer_event_t *e10 = NULL;
static ina_timer_event_t *e11 = NULL;
static ina_timer_event_t *e12 = NULL;
static ina_timer_event_t *e13 = NULL;
static ina_timer_event_t *e14 = NULL;
static ina_timer_event_t *e15 = NULL;
static ina_timer_event_t *e16 = NULL;
static ina_timer_event_t *e17 = NULL;
static ina_timer_event_t *stop_event = NULL;

static ina_rc_t __ina_timer_bench_create_events(ina_timer_t *timer)
{
    ina_timer_event_new(timer, 50, &e1);
    ina_timer_event_new(timer, 500, &e2);
    ina_timer_event_new(timer, 100, &e3);
    ina_timer_event_new(timer, 200 ,&e4);
    ina_timer_event_new(timer, 70,  &e5);
    ina_timer_event_new(timer, 100, &e6);
    ina_timer_event_new(timer, 1000, &e7);
    ina_timer_event_new(timer, 150, &e8);
    ina_timer_event_new(timer, 555, &e9);
    ina_timer_event_new(timer, 2000, &e10);
    ina_timer_event_new(timer, 1500, &e11);
    ina_timer_event_new(timer, 30, &e12);
    ina_timer_event_new(timer, 8, &e13);
    ina_timer_event_new(timer, 22, &e14);
    ina_timer_event_new(timer, 111, &e15);
    ina_timer_event_new(timer, 222, &e16);
    ina_timer_event_new(timer, 333, &e17);
    ina_timer_event_new(timer, 20000, &stop_event);
    return INA_SUCCESS;
}

INA_BENCH_DATA(timer) {
    int dummy;
};

INA_BENCH_SETUP(timer){
    INA_UNUSED(data);
    ina_bench_set_precision(0);
}
INA_BENCH_TEARDOWN(timer){ INA_UNUSED(data);}
INA_BENCH_SCALE(timer) {
    INA_UNUSED(data);
    ina_bench_set_scale(1);
}
INA_BENCH_BEGIN(timer, create_rdtsc) { INA_UNUSED(data);}
INA_BENCH_END(timer, create_rdtsc) { INA_UNUSED(data);}
INA_BENCH(timer, create_rdtsc) {
    ina_timer_t *timer;
    INA_UNUSED(data);
    INA_MUST_SUCCEED(ina_timer_new(&timer));

    ina_time_tsc_enable_rdtsc();
    ina_bench_stopwatch_start();
    __ina_timer_bench_create_events(timer);
    ina_bench_set_value((double)ina_bench_stopwatch_stop());
    ina_time_tsc_disable_rdtsc();

    ina_timer_free(&timer);
}

INA_BENCH_BEGIN(timer, create) { INA_UNUSED(data);}
INA_BENCH_END(timer, create) { INA_UNUSED(data);}
INA_BENCH(timer, create) {
    ina_timer_t *timer;
    INA_UNUSED(data);
    INA_MUST_SUCCEED(ina_timer_new(&timer));

    ina_bench_stopwatch_start();
    __ina_timer_bench_create_events(timer);
    ina_bench_set_value((double)ina_bench_stopwatch_stop());

    ina_timer_free(&timer);
}


INA_BENCH_BEGIN(timer, exec) { INA_UNUSED(data);}
INA_BENCH_END(timer, exec) { INA_UNUSED(data); }
INA_BENCH(timer, exec) {
    ina_timer_t *timer;
    int64_t total = 0;
    uint64_t id;
    uint64_t stop_id;
    INA_UNUSED(data);
    INA_MUST_SUCCEED(ina_timer_new(&timer));

    __ina_timer_bench_create_events(timer);
    INA_TEST_ASSERT_SUCCEED(ina_timer_event_get_id(stop_event, &stop_id));
    for (;;) {
        ina_timer_event_t *e;
        ina_bench_stopwatch_start();
        ina_timer_next_event(timer, &e);
        total += ina_bench_stopwatch_stop();
        if (e != NULL && INA_SUCCEED(ina_timer_event_get_id(e, &id)) && id == stop_id) {
                break;
        }
        ina_time_sleep(1);
    }
    ina_bench_set_value((double)total);
    ina_timer_free(&timer);
}


INA_BENCH_BEGIN(timer, exec_rdtsc) { INA_UNUSED(data);}
INA_BENCH_END(timer, exec_rdtsc) { INA_UNUSED(data);}
INA_BENCH(timer, exec_rdtsc) {
    ina_timer_t *timer;
    int64_t total = 0;
    uint64_t id;
    uint64_t stop_id;
    INA_UNUSED(data);
    ina_time_tsc_enable_rdtsc();

    INA_MUST_SUCCEED(ina_timer_new(&timer));

    __ina_timer_bench_create_events(timer);
    INA_TEST_ASSERT_SUCCEED(ina_timer_event_get_id(stop_event, &stop_id));
    for (;;) {
        ina_timer_event_t *e;
        ina_bench_stopwatch_start();
        ina_timer_next_event(timer, &e);
        total += ina_bench_stopwatch_stop();
        if (e != NULL && INA_SUCCEED(ina_timer_event_get_id(e, &id)) && id == stop_id) {
            break;
        }
        ina_time_sleep(1);
    }
    ina_time_tsc_disable_rdtsc();
    ina_bench_set_value((double)total);
    ina_timer_free(&timer);
}




