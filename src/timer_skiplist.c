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

#ifdef INA_TIMER_SKIPLIST_ENABLED

#include <contribs/skiplist/skiplist.h>
#include "timer_backend.h"


/* Skip list compare callback */
static int __ina_cmp(const void *, const void *);
/* Get current time and add milliseconds to the time value */
static ina_rc_t __ina_add_millis_to_now(time_t, time_t, time_t*, time_t*);
/* Get current time */
static ina_rc_t __ina_get_time(time_t, time_t*, time_t*);
/* Seach the next event */
static ina_time_event_t* __ina_search_nearest_event(ina_timer_backend_t*);
/* Reschedule time */
static ina_rc_t __ina_reschedule_timer(time_t now, ina_timer_backend_t*, ina_time_event_t*);

struct ina_timer_backend_s {
    skiplist events;
    uint64_t next_event_id;
};

struct ina_timer_backend_event_s {
    time_t msec;
    time_t when_sec;
    time_t when_msec;
};

INA_API(ina_rc_t) ina_timer_backend_init(ina_timer_backend_t **backend)
{
    ina_timer_backend_t *b;
    ina_time_event_t *sentinal;
    ina_timer_backend_event_t *sb;

    sentinal = (ina_time_event_t*)ina_mem_alloc(sizeof(ina_time_event_t));
    if (sentinal == NULL) {
        return INA_MEM_EALLOC;
    }
    sentinal->data = (ina_timer_backend_event_t*)ina_mem_alloc(sizeof(ina_timer_backend_event_t));
    sentinal->id = 0;
    sb = (ina_timer_backend_event_t*)sentinal->data;
    sb->msec = 0;
    sb->when_msec = 0;  
    sb->when_sec = 0;

    *backend = (ina_timer_backend_t*)ina_mem_alloc(sizeof(ina_timer_backend_t));
    if (*backend == NULL) {
        ina_mem_free(sentinal->data);
        ina_mem_free(sentinal);
        return INA_MEM_EALLOC;
    }
    b = *backend;
    b->next_event_id = 0;
    b->events = skiplist_create(__ina_cmp, sentinal);

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_timer_backend_destroy(ina_timer_backend_t **backend)
{
    ina_time_event_t *e;
    skipnode n, ntmp;

    INA_ASSERT_NOTNULL(backend);

    if (*backend == NULL) {
        return INA_SUCCESS;
    }

    SKIPLIST_FOREACH_SAFE((*backend)->events, n, ntmp) {
        e = (ina_time_event_t*)skipnode_item(n);
        ina_mem_free(e);
    }
    skiplist_destroy((*backend)->events);

    ina_mem_free(*backend);

    return INA_SUCCESS;
}

INA_API(ina_time_event_t*) ina_timer_backend_create_event(ina_timer_backend_t *backend, 
                                                          ina_time_event_t *e, 
                                                          time_t n_msec,
                                                          time_t e_msec)
{
    ina_timer_backend_event_t *sb;

    INA_ASSERT_NOTNULL(backend);
    INA_ASSERT_NOTNULL(e);

    e->data = (ina_timer_backend_event_t*)ina_mem_alloc(sizeof(ina_timer_backend_event_t));
    if (e->data == NULL) {
        INA_MEM_EALLOC;
        return NULL;
    }
    sb = (ina_timer_backend_event_t*)e->data;

    e->id = ++backend->next_event_id;
    sb->msec = e_msec;

    __ina_add_millis_to_now(n_msec, e_msec, &sb->when_sec, &sb->when_msec);

    skiplist_insert(backend->events, e);

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_timer_backend_delete_event(ina_timer_backend_t *backend, ina_time_event_t *e)
{
    ina_timer_backend_event_t *sb;

    INA_ASSERT_NOTNULL(backend);
    INA_ASSERT_NOTNULL(e);

    sb = (ina_timer_backend_event_t*)e->data;
    skiplist_delete_gc(backend->events, e);
    ina_mem_free(sb);

    return INA_SUCCESS;
}

INA_API(ina_time_event_t*) ina_timer_backend_next_event_with_time(ina_timer_backend_t *backend, time_t now_millis)
{
    time_t now_sec, now_msec;
    ina_time_event_t *e;
    ina_timer_backend_event_t *sb;

    INA_ASSERT_NOTNULL(backend);

    /* 
	 * We do not need to handle clock-skew!
	 * Since we're using a monotonic increasing clock. 
	 * Check the function calls in __ina_get_time to get 
	 * a better idea.
	 */

    e = __ina_search_nearest_event(backend);
    if (e == NULL) {
        return NULL;
    }
    sb = (ina_timer_backend_event_t*)e->data;
    
    /* check if event should fire if yes return it, if no return null */
    __ina_get_time(now_millis, &now_sec, &now_msec);
    if (now_sec > sb->when_sec || (now_sec == sb->when_sec && now_msec >= sb->when_msec)) {
        __ina_reschedule_timer(now_millis, backend, e);
        return e;
    }
    return NULL;
}

INA_API(ina_rc_t) ina_timer_backend_time_to_next_event(ina_timer_backend_t *backend, time_t now_millis, time_t *how_long_msec)
{
    time_t now_sec, now_msec;
    ina_time_event_t *shortest;
    time_t hl_sec;
    time_t hl_msec;
    ina_timer_backend_event_t *sb;

    INA_ASSERT_NOTNULL(backend);
    INA_ASSERT_NOTNULL(how_long_msec);

    shortest = __ina_search_nearest_event(backend);
    sb = (ina_timer_backend_event_t*)shortest->data;

    /* Calculate the time missing for the nearest timer to fire. */
    __ina_get_time(now_millis, &now_sec, &now_msec);
    hl_sec = sb->when_sec - now_sec;
    hl_msec = sb->when_msec - now_msec;
    if (sb->when_msec < now_msec) {
        hl_sec--;
    }
    if (hl_sec < 0 || hl_msec < 0) {
        /* set to zero */
        *how_long_msec = 0;
    }
    else {
        *how_long_msec = hl_msec;
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
        const ina_timer_backend_event_t *ld = (ina_timer_backend_event_t*)l->data;
        const ina_timer_backend_event_t *ud = (ina_timer_backend_event_t*)u->data;
        if (ld->when_sec > ud->when_sec) {
            return(1);
        }
        else if (ld->when_sec < ud->when_sec) {
            return(-1);
        }
        else {
            if (ld->when_msec > ud->when_msec) {
                return(1);
            }
            else if (ld->when_msec < ud->when_msec) {
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
__ina_add_millis_to_now(time_t now, time_t msec_to_add, time_t *sec, time_t *msec)
{
    time_t cur_sec, cur_msec, when_sec, when_msec;
    INA_ASSERT(msec_to_add > 0);
    INA_ASSERT_NOTNULL(sec);
    INA_ASSERT_NOTNULL(msec);

    __ina_get_time(now, &cur_sec, &cur_msec);
    when_sec = cur_sec + msec_to_add/1000;
    when_msec = cur_msec + msec_to_add%1000;
    if (when_msec >= 1000) {
        when_sec++;
        when_msec -= 1000;
    }
    *sec = when_sec;
    *msec = when_msec;
    return INA_SUCCESS;
}

static ina_rc_t 
__ina_get_time(time_t cur_ms, time_t *sec, time_t *msec)
{   
    INA_ASSERT_NOTNULL(sec);
    INA_ASSERT_NOTNULL(msec);

    *sec = cur_ms/1000;    
	*msec = cur_ms%1000;

    return INA_SUCCESS;
}

static ina_time_event_t*
__ina_search_nearest_event(ina_timer_backend_t *backend)
{
    skipnode i = skiplist_at(backend->events,0);
    ina_time_event_t *e = (ina_time_event_t*)skipnode_item(i);
    return e;
}

static ina_rc_t 
__ina_reschedule_timer(time_t now, ina_timer_backend_t *backend, ina_time_event_t *e)
{
    ina_timer_backend_event_t *sb = (ina_timer_backend_event_t*)e->data;

    skiplist_delete_gc(backend->events, e);
    __ina_add_millis_to_now(now, sb->msec, &sb->when_sec, &sb->when_msec);
    skiplist_insert(backend->events, e);

    return INA_SUCCESS;
}

#endif
