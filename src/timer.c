/*
 * Copyright (c) 2012-2014, INAOS GmbH
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
#include "timer_backend.h"

struct ina_timer_s {
    ina_time_tsc_t *stamp;
    ina_timer_backend_t *be;
    int use_rdtsc;
} ina_timer_s;

static time_t __ina_timer_tsc_to_msec(ina_time_tsc_t *tsc)
{
    time_t now_millis = 0;
    ina_time_tsc_millis(tsc, &now_millis);
    return now_millis;
}

INA_API(ina_rc_t) ina_timer_init(ina_timer_t **timer)
{
    ina_timer_t* t;
    
    INA_ASSERT_NOTNULL(timer);

    *timer = (ina_timer_t*)ina_mem_alloc(sizeof(ina_timer_t));
    if (*timer == NULL) {
        return INA_MEM_EALLOC;
    }
    t = *timer;
    if (!INA_SUCCEED(ina_timer_backend_init(&t->be))) {
        return INA_ERR_PUSH_LAST;
    }
    t->use_rdtsc = INA_NO;
    ina_time_tsc_new(&(*timer)->stamp);

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_timer_destroy(ina_timer_t **timer)
{
    INA_ASSERT_NOTNULL(timer);

    if (*timer == NULL) {
        return INA_SUCCESS;
    }

    if (!INA_SUCCEED(ina_timer_backend_destroy(&(*timer)->be))) {
        return INA_ERR_PUSH_LAST;
    }

    ina_time_tsc_free(&(*timer)->stamp);
    ina_mem_free(*timer);
    *timer = NULL;

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_timer_use_rdtsc(ina_timer_t *timer, int yesno)
{
    INA_ASSERT_NOTNULL(timer);

    if (yesno == INA_YES && timer->use_rdtsc == INA_NO) {
        ina_time_tsc_enable_rdtsc();
    }
    else if (yesno == INA_NO && timer->use_rdtsc == INA_YES) {
        ina_time_tsc_disable_rdtsc();
    }
    return INA_SUCCESS;
}


INA_API(ina_time_event_t*) ina_timer_create_event(ina_timer_t *timer, time_t msec)
{
    time_t now_millis;
    
    INA_ASSERT_NOTNULL(timer);

    ina_time_read_tsc_clock(timer->stamp);
    now_millis = __ina_timer_tsc_to_msec(timer->stamp);

    return ina_timer_create_event_with_time(timer, now_millis, msec);
}

INA_API(ina_time_event_t*) ina_timer_create_event_with_time(ina_timer_t *timer, time_t n_msec, time_t e_msec)
{
    ina_time_event_t *e;
    
    INA_ASSERT_NOTNULL(timer);
    INA_ASSERT(n_msec > 0);
    INA_ASSERT(e_msec > 0);

    e = (ina_time_event_t*)ina_mem_alloc(sizeof(ina_time_event_t));
    if (INA_UNLIKELY(e == NULL)) {
        INA_MEM_EALLOC;
        return NULL;
    }

    if (INA_UNLIKELY(!INA_SUCCEED(ina_timer_backend_create_event(timer->be, e, n_msec, e_msec)))) {
        INA_ERR_PUSH_LAST;
        return NULL;
    }

    return e;
}

INA_API(ina_rc_t) ina_timer_delete_event(ina_timer_t *timer, ina_time_event_t *e)
{
    INA_ASSERT_NOTNULL(timer);
    INA_ASSERT_NOTNULL(e);

    ina_timer_backend_delete_event(timer->be, e);
    ina_mem_free(e);

    return INA_SUCCESS;
}

INA_API(ina_time_event_t*) ina_timer_next_event(ina_timer_t *timer)
{
    time_t now_millis;
    
    INA_ASSERT_NOTNULL(timer);

    ina_time_read_tsc_clock(timer->stamp);
    now_millis = __ina_timer_tsc_to_msec(timer->stamp);

    return ina_timer_next_event_with_time(timer, now_millis);
}

INA_API(ina_time_event_t*) ina_timer_next_event_with_time(ina_timer_t *timer, time_t now_millis)
{
    INA_ASSERT_NOTNULL(timer);

    return ina_timer_backend_next_event_with_time(timer->be, now_millis);   
}

INA_API(ina_rc_t) ina_timer_time_to_next_event(ina_timer_t *timer, time_t *how_long_msec)
{
    time_t now_millis;
    
    INA_ASSERT_NOTNULL(timer);

    ina_time_read_tsc_clock(timer->stamp);
    now_millis = __ina_timer_tsc_to_msec(timer->stamp);

    return ina_timer_backend_time_to_next_event(timer->be, now_millis, how_long_msec);
}
