/*
 * Copyright (c) 2012-2018, INAOS GmbH
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
#include "config.h"

/* 
 * TODO:
 * - I believe we use the same algorithm as described here: https://www.snellman.net/blog/archive/2016-07-27-ratas-hierarchical-timer-wheel/ 
 *   It could be interesting to compare/benchmark the two implementations, currently it seems there is no need for this.
 * 
 */

struct ina_timer_s {
    ina_time_tsc_t *stamp;
    struct timeouts *timeouts;
    uint64_t next_event_id;
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

INA_API(ina_rc_t) ina_timer_free(ina_timer_t **timer)
{
    INA_VERIFY_NOT_NULL(timer);
    INA_VERIFY_NOT_NULL(*timer);

    if ((*timer)->timeouts != NULL) {
        timeouts_close((*timer)->timeouts);
    }
    if ((*timer)->stamp != NULL) {
        INA_MUST_SUCCEED(ina_time_tsc_free(&(*timer)->stamp));
    }
    ina_mem_free(*timer);
    *timer = NULL;
    return INA_SUCCESS;
}


INA_API(ina_time_event_t*) ina_timer_event_new(ina_timer_t *timer, time_t msec)
{
    time_t now_millis;
    
    INA_VERIFY_NOT_NULL(timer);

    ina_time_read_tsc_clock(timer->stamp);
    now_millis = __ina_timer_tsc_to_msec(timer->stamp);

    return ina_timer_event_new_with_time(timer, now_millis, msec);
}

INA_API(ina_time_event_t*) ina_timer_event_new_with_time(ina_timer_t *timer, time_t n_msec, time_t e_msec)
{
    ina_time_event_t *e;
    
    INA_VERIFY_NOT_NULL(timer);
    INA_VERIFY(n_msec > 0);
    INA_VERIFY(e_msec > 0);

    e = (ina_time_event_t*)ina_mem_alloc(sizeof(ina_time_event_t));
    if (e == NULL) {
        return NULL;
    }

    e->t = (struct timeout*)ina_mem_alloc(sizeof(struct timeout));
    if (e->t != NULL) {
        ina_mem_free(e);
        return NULL;
    }
    e->id = ++timer->next_event_id;

    /* let the timewheel know the current time */
    timeouts_update(timer->timeouts, n_msec);

    e->t = timeout_init(e->t, TIMEOUT_INT);
    e->t->data = e;
    timeouts_add(timer->timeouts, e->t, e_msec);
    return e;
}

INA_API(ina_rc_t) ina_timer_event_free(ina_timer_t *timer, ina_time_event_t *e)
{
    INA_VERIFY_NOT_NULL(timer);
    INA_VERIFY_NOT_NULL(e);

    timeouts_del(timer->timeouts, e->t);
    ina_mem_free(e->t);
    ina_mem_free(e);
    return INA_SUCCESS;
}

INA_API(ina_time_event_t*) ina_timer_next_event(ina_timer_t *timer)
{
    time_t now_millis;
    
    INA_VERIFY_NOT_NULL(timer);

    ina_time_read_tsc_clock(timer->stamp);
    now_millis = __ina_timer_tsc_to_msec(timer->stamp);

    return ina_timer_next_event_with_time(timer, now_millis);
}

INA_API(ina_time_event_t*) ina_timer_next_event_with_time(ina_timer_t *timer, time_t now_millis)
{
    struct timeout *ne;

    INA_VERIFY_NOT_NULL(timer);

    timeouts_update(timer->timeouts, now_millis);
    ne = timeouts_get(timer->timeouts);
    if (ne != NULL) {
        return (ina_time_event_t*)ne->data;
    }
    return NULL;
}

INA_API(ina_rc_t) ina_timer_time_to_next_event(ina_timer_t *timer, time_t *how_long_msec)
{
    INA_VERIFY_NOT_NULL(timer);
    *how_long_msec = timeouts_timeout(timer->timeouts);
    return INA_SUCCESS;
}
