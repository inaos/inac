/*
 * Copyright (c) 2014, INAOS GmbH
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

#ifdef INA_TIMER_BACKEND_WHEEL_ENABLED

#include <contribs/timerwheel/timeout.h>
#include "timer_backend.h"

struct ina_timer_backend_s {
    struct timeouts *timeouts;
    uint64_t next_event_id;
};

struct ina_timer_backend_event_s {
    struct timeout *t;
};

INA_API(ina_rc_t) ina_timer_backend_init(ina_timer_backend_t **backend)
{
    ina_timer_backend_t *b;
    int err;
    *backend = (ina_timer_backend_t*)ina_mem_alloc(sizeof(ina_timer_backend_t));
    if (*backend == NULL) {
        return INA_MEM_EALLOC;
    }
    b = *backend;
    b->timeouts = timeouts_open(0, &err, ina_mem_alloc, ina_mem_free);
    b->next_event_id = 0;

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_timer_backend_destroy(ina_timer_backend_t **backend)
{
    INA_ASSERT_NOTNULL(backend);

    if (*backend == NULL) {
        return INA_SUCCESS;
    }

    timeouts_close((*backend)->timeouts);

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
    if (INA_UNLIKELY(e->data == NULL)) {
        INA_MEM_EALLOC;
        return NULL;
    }
    sb = (ina_timer_backend_event_t*)e->data;
    sb->t = (struct timeout*)ina_mem_alloc(sizeof(struct timeout));
    if (INA_UNLIKELY(sb->t == NULL)) {
        ina_mem_free(e->data);
        INA_MEM_EALLOC;
        return NULL;
    }

    e->id = ++backend->next_event_id;

    /* let the timewheel know the current time */
    timeouts_update(backend->timeouts, n_msec);

    sb->t = timeout_init(sb->t, TIMEOUT_INT);
    sb->t->data = e;
    timeouts_add(backend->timeouts, sb->t, e_msec);

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_timer_backend_delete_event(ina_timer_backend_t *backend, ina_time_event_t *e)
{
    ina_timer_backend_event_t *sb;

    INA_ASSERT_NOTNULL(backend);
    INA_ASSERT_NOTNULL(e);

    sb = (ina_timer_backend_event_t*)e->data;
    timeouts_del(backend->timeouts, sb->t);
    ina_mem_free(sb->t);
    ina_mem_free(sb);

    return INA_SUCCESS;
}

INA_API(ina_time_event_t*) ina_timer_backend_next_event_with_time(ina_timer_backend_t *backend, time_t now_millis)
{
    struct timeout *ne;

    timeouts_update(backend->timeouts, now_millis);
    ne = timeouts_get(backend->timeouts);
    if (ne != NULL) {
        return (ina_time_event_t*)ne->data;
    }

    return NULL;
}

INA_API(ina_rc_t) ina_timer_backend_time_to_next_event(ina_timer_backend_t *backend, time_t now_millis, time_t *how_long_msec)
{
    *how_long_msec = timeouts_timeout(backend->timeouts);
    return INA_SUCCESS;
}


#endif
