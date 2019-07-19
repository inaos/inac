/*
 * Copyright INAOS GmbH, Thalwil, 2012-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>
#include <contribs/timerwheel/timeout.h>
#include "config.h"

/* 
 * TODO:
 * - I believe we use the same algorithm as described here: https://www.snellman.net/blog/archive/2016-07-27-ratas-hierarchical-timer-wheel/ 
 *   It could be interesting to compare/benchmark the two implementations, currently it seems there is no need for this.
 * 
 */

/* Time event */
typedef struct ina_timer_event_s {
    int id;
    struct timeout *t;
} ina_timer_event_s;

struct ina_timer_s {
    ina_time_tsc_t *stamp;
    struct timeouts *timeouts;
    int next_event_id;
} ina_timer_s;

static time_t __ina_timer_tsc_to_msec(ina_time_tsc_t *tsc)
{
    time_t now_millis = 0;
    ina_time_tsc_millis(tsc, &now_millis);
    return now_millis;
}

INA_API(ina_rc_t) ina_timer_new(ina_timer_t **timer)
{
    ina_timer_t* t;
    int err;
    
    INA_VERIFY_NOT_NULL(timer);

    *timer = (ina_timer_t*)ina_mem_alloc(sizeof(ina_timer_t));
    INA_RETURN_IF_NULL(*timer);

    t = *timer;
    t->timeouts = timeouts_open(0, &err, ina_mem_alloc, ina_mem_free);
    t->next_event_id = 0;
    return ina_time_tsc_new(&(*timer)->stamp);
}

INA_API(void) ina_timer_free(ina_timer_t **timer)
{
    INA_VERIFY_FREE(timer);
    if ((*timer)->timeouts != NULL) {
        timeouts_close((*timer)->timeouts);
    }
    ina_time_tsc_free(&(*timer)->stamp);
    INA_MEM_FREE_SAFE(*timer);
}


INA_API(ina_rc_t) ina_timer_event_new(ina_timer_t *timer, time_t msec, ina_timer_event_t **event)
{
    time_t now_millis;
    
    INA_VERIFY_NOT_NULL(timer);
    INA_VERIFY_NOT_NULL(event);

    ina_time_read_tsc_clock(timer->stamp);
    now_millis = __ina_timer_tsc_to_msec(timer->stamp);

    return ina_timer_event_new_with_time(timer, now_millis, msec, event);
}

INA_API(ina_rc_t) ina_timer_event_new_with_time(ina_timer_t *timer, time_t n_msec, time_t e_msec, ina_timer_event_t **event)
{
    INA_VERIFY_NOT_NULL(timer);
    INA_VERIFY(n_msec > 0);
    INA_VERIFY(e_msec > 0);
    INA_VERIFY_NOT_NULL(event);

    *event = (ina_timer_event_t*)ina_mem_alloc(sizeof(ina_timer_event_t));
    INA_RETURN_IF_NULL(*event);

    (*event)->t = (struct timeout*)ina_mem_alloc(sizeof(struct timeout));
    if ((*event)->t == NULL) {
        ina_mem_free(*event);
        *event = NULL;
        return ina_err_get_rc();
    }
    (*event)->id = ++timer->next_event_id;

    /* let the timewheel know the current time */
    timeouts_update(timer->timeouts, n_msec);

    (*event)->t = timeout_init((*event)->t, TIMEOUT_INT);
    (*event)->t->data = *event;
    timeouts_add(timer->timeouts, (*event)->t, e_msec);
    return INA_SUCCESS;
}

INA_API(void) ina_timer_event_free(ina_timer_t *timer, ina_timer_event_t **e)
{
    INA_ASSERT_NOT_NULL(timer);
    INA_VERIFY_FREE(e);

    timeouts_del(timer->timeouts, (*e)->t);
    INA_MEM_FREE_SAFE((*e)->t);
    INA_MEM_FREE_SAFE(*e);
}

INA_API(ina_rc_t) ina_timer_event_get_id(const ina_timer_event_t *event, int *id)
{
    INA_VERIFY_NOT_NULL(event);
    INA_VERIFY_NOT_NULL(id);
    *id = event->id;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_timer_next_event(const ina_timer_t *timer, ina_timer_event_t **event)
{
    time_t now_millis;
    
    INA_VERIFY_NOT_NULL(timer);

    ina_time_read_tsc_clock(timer->stamp);
    now_millis = __ina_timer_tsc_to_msec(timer->stamp);

    return ina_timer_next_event_with_time(timer, now_millis, event);
}

INA_API(ina_rc_t) ina_timer_next_event_with_time(const ina_timer_t *timer, time_t now_millis, ina_timer_event_t **event)
{
    struct timeout *ne;

    INA_VERIFY_NOT_NULL(timer);
    INA_VERIFY_NOT_NULL(event);

    timeouts_update(timer->timeouts, now_millis);
    ne = timeouts_get(timer->timeouts);
    if (ne != NULL) {
        *event = (ina_timer_event_t*)ne->data;
        return INA_SUCCESS;
    }
    return INA_ERROR(INA_ERR_TRY_AGAIN);
}

INA_API(ina_rc_t) ina_timer_time_to_next_event(const ina_timer_t *timer, time_t *how_long_msec)
{
    INA_VERIFY_NOT_NULL(timer);
    INA_VERIFY_NOT_NULL(how_long_msec);
    *how_long_msec = timeouts_timeout(timer->timeouts);
    return INA_SUCCESS;
}
