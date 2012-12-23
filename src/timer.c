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
#include "config.h"
#include <skiplist.h>

struct ina_timer_s {
    skiplist events;
    uint64_t next_event_id;
    time_t last_time;
} ina_timer_s;

/* Skip list compare callback */
static int __ina_cmp(const void *, const void *);
/* Read clock time */
static ina_rc_t __ina_read_clock(ina_time_t*);
/* Get current time */
static ina_rc_t __ina_get_time(time_t*, time_t*);
/* Get current time and add milliseconds to the time value */
static ina_rc_t __ina_add_millis_to_now(time_t, time_t*, time_t*);
/* Seach the next event */
static ina_time_event_t* __ina_search_nearest_event(ina_timer_t*);
/* Reschedule time */
static ina_rc_t __ina_reschedule_timer(ina_timer_t*, ina_time_event_t*);

INA_API(ina_rc_t) ina_timer_init(ina_timer_t **timer)
{
    ina_timer_t* t;
    ina_time_event_t *sentinal; 

    sentinal = (ina_time_event_t*)ina_mem_alloc(sizeof(ina_time_event_t));
    if (sentinal == NULL) {
        return INA_MEM_EALLOC;
    }
    sentinal->id = 0;
    sentinal->millis = 0;
    sentinal->when_ms = 0;  
    sentinal->when_sec = 0;

    *timer = (ina_timer_t*)ina_mem_alloc(sizeof(ina_timer_t));
    if (*timer == NULL) {
        ina_mem_free(sentinal);
        return INA_MEM_EALLOC;
    }
    t = *timer;
    t->next_event_id = 0;
    t->last_time = time(NULL);
    t->events = skiplist_create(__ina_cmp, sentinal);
#ifdef INA_OS_WIN32
    timeBeginPeriod(1);
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_timer_destroy(ina_timer_t **timer)
{
    skipnode n, ntmp;
    ina_time_event_t *e;
    SKIPLIST_FOREACH_SAFE((*timer)->events, n, ntmp) {
        e = (ina_time_event_t*)skipnode_item(n);
        ina_mem_free(e);
    }
    skiplist_destroy((*timer)->events);
    ina_mem_free(*timer);
    *timer = NULL;
#ifdef WIN32
    timeEndPeriod(1);
#endif
    return INA_SUCCESS;
}

INA_API(ina_time_event_t*) ina_timer_create_event(ina_timer_t *timer, time_t milliseconds)
{
    ina_time_event_t *e;
    e = (ina_time_event_t*)ina_mem_alloc(sizeof(ina_time_event_t));
    if (e == NULL) {
        INA_MEM_EALLOC;
        return NULL;
    }

    e->id = ++timer->next_event_id;
    e->millis = milliseconds;

    __ina_add_millis_to_now(milliseconds, &e->when_sec, &e->when_ms);

    skiplist_insert(timer->events, e);
    return e;
}

INA_API(ina_rc_t) ina_timer_delete_event(ina_timer_t *timer, ina_time_event_t *e)
{
    skiplist_delete_gc(timer->events, e);
    ina_mem_free(e);
    return INA_SUCCESS;
}

INA_API(ina_time_event_t*) ina_timer_next(ina_timer_t *timer)
{
    time_t now = time(NULL);
    time_t now_sec, now_ms;
    ina_time_event_t *e;

    /* 
     * If the system clock is moved to the future, and then set back to the
     * right value, time events may be delayed in a random way. Often this
     * means that scheduled operations will not be performed soon enough.
     *
     * Here we try to detect system clock skews, and force all the time
     * events to be processed ASAP when this happens: the idea is that
     * processing events earlier is less dangerous than delaying them
     * indefinitely, and practice suggests it is. 
     */
    if (now < timer->last_time) {
        skipnode n;
        ina_time_event_t *e;
        SKIPLIST_FOREACH(timer->events, n) {
            e = (ina_time_event_t*)skipnode_item(n);
            e->when_sec = 0;
        }
    }
    timer->last_time = now;

    e = __ina_search_nearest_event(timer);
    if (e == NULL) {
        return NULL;
    }
    
    /* check if event should fire if yes return it, if no return null */
    __ina_get_time(&now_sec, &now_ms);
    if (now_sec > e->when_sec || (now_sec == e->when_sec && now_ms >= e->when_ms)) {
        __ina_reschedule_timer(timer, e);
        return e;
    }
    return NULL;
}

INA_API(ina_rc_t) ina_timer_time_to_next(ina_timer_t *timer, time_t *how_long_millis)
{
    time_t now_sec, now_ms;
    ina_time_event_t *shortest;
    time_t hl_sec;
    time_t hl_ms;

    shortest = __ina_search_nearest_event(timer);

    /* Calculate the time missing for the nearest timer to fire. */
    __ina_get_time(&now_sec, &now_ms);
    hl_sec = shortest->when_sec - now_sec;
    hl_ms = shortest->when_ms - now_ms;
    if (shortest->when_ms < now_ms) {
        hl_sec--;
    }
    if (hl_sec < 0 || hl_ms < 0) {
        /* set to zero */
        *how_long_millis = 0;
    }
    else {
        *how_long_millis = hl_ms;
    }
    return INA_SUCCESS;
}

static int
 __ina_cmp(const void *list_value, const void *user_value)
{
    const ina_time_event_t *l = (ina_time_event_t*)list_value;
    const ina_time_event_t *u = (ina_time_event_t*)user_value;

    /* if we compare against the sentinal */
    if (l->id == 0) {
        return(1);
    }

    if (l->id == u->id) {
        return(0);
    }
    else {
        if (l->when_sec > u->when_sec) {
            return(1);
        }
        else if (l->when_sec < u->when_sec) {
            return(-1);
        }
        else {
            if (l->when_ms > u->when_ms) {
                return(1);
            }
            else if (l->when_ms < u->when_ms) {
                return(-1);
            }
            else {
                if (l->id < u->id) {
                    return(-1);
                }
                else {
                    return(1);
                }
            }
        }
    }
}

static ina_rc_t 
__ina_read_clock(ina_time_t *time)
{
#ifdef WIN32
    time->ttp = timeGetTime();
#else
    gettimeofday(&time->tp, NULL);
#endif
    return INA_SUCCESS;
}

static ina_rc_t 
__ina_get_time(time_t *seconds, time_t *millis)
{
    ina_time_t t;

    INA_ASSERT_NOTNULL(seconds);
    INA_ASSERT_NOTNULL(millis);

    __ina_read_clock(&t);

    ina_time_get_seconds(&t, seconds);
    ina_time_get_milliseconds(&t, millis);
    return INA_SUCCESS;
}

static ina_rc_t
__ina_add_millis_to_now(time_t millis, time_t *sec, time_t *ms)
{
    time_t cur_sec, cur_ms, when_sec, when_ms;

    __ina_get_time(&cur_sec, &cur_ms);
    when_sec = cur_sec + millis/1000;
    when_ms = cur_ms + millis%1000;
    if (when_ms >= 1000) {
        when_sec++;
        when_ms -= 1000;
    }
    *sec = when_sec;
    *ms = when_ms;
    return INA_SUCCESS;
}

static ina_time_event_t*
__ina_search_nearest_event(ina_timer_t *timer)
{
    skipnode i = skiplist_at(timer->events,0);
    ina_time_event_t *e = (ina_time_event_t*)skipnode_item(i);
    return e;
}

static ina_rc_t 
__ina_reschedule_timer(ina_timer_t *timer, ina_time_event_t *e)
{
    skiplist_delete_gc(timer->events, e);
    __ina_add_millis_to_now(e->millis, &e->when_sec, &e->when_ms);
    skiplist_insert(timer->events, e);
    return INA_SUCCESS;
}