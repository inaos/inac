/*
 * Copyright (c) 2012-2016, INAOS GmbH
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
#ifndef _LIBINAC_TIMER_H_
#define _LIBINAC_TIMER_H_

#include <libinac/lib.h>
#include <contribs/timerwheel/timeout.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Timer */
typedef struct ina_timer_s ina_timer_t;

/* FIXME: make it opaque */
/* Time event */
typedef struct ina_time_event_s {
    uint64_t id;
    struct timeout *t;
} ina_time_event_t;

/*
 * Creates a new timer.
 *
 * Parameters
 *  timer  Where to store the timer
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_timer_init(ina_timer_t **timer);

/*
 * Destroy a timer.
 *
 * Parameters
 *  timer  Timer to free
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_timer_destroy(ina_timer_t **timer);


/*
 * Creates a new time event for a timer
 *
 * Parameters
 *  timer  Timer
 *  msec   Event interval in milliseconds
 *
 * Return
 *  Pointer to timer event or NULL if an error occurred.
 */
INA_API(ina_time_event_t*) ina_timer_create_event(ina_timer_t *timer,
                                                  time_t msec);

/*
 * Create a new time event for a timer while providing current time.
 *
 * Parameters
 *  timer   Timer
 *  n_msec  Current time in millisecond since epoch
 *  e_msec  Event interval in milliseconds
 *
 * Return
 *  Pointer to timer event or NULL if an error occurred.
 */
INA_API(ina_time_event_t*) ina_timer_create_event_with_time(ina_timer_t *timer,
                                                            time_t n_msec,
                                                            time_t e_msec);
/*
 * Delete a time event from a timer.
 *
 * Parameter
 *  timer  Timer
 *  e      Timer event to delete from timer.
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_timer_delete_event(ina_timer_t *timer,
                                         ina_time_event_t *e);


/*
 * Get the next elapsed time event.
 *
 * Parameters
 *  timer  Timer to query
 *
 * Return
 *  Next elapsed time event or NULL if no events elapsed.
 */
INA_API(ina_time_event_t*) ina_timer_next_event(ina_timer_t *timer);

/*
 * Get the next elapsed time event by providing the milliseconds since epoch.
 *
 * Parameters
 *  timer      Timer to query
 *  now_millis Time since epoch
 *
 * Return
 *  Next elapsed time event or NULL if no events elapsed.
 */
INA_API(ina_time_event_t*) ina_timer_next_event_with_time(ina_timer_t *timer,
                                                          time_t now_millis);

/*
 * Calculate time in milliseconds until the next time event will elapse.
 *
 * Parameters
 *  timer          Timer to query
 *  how_long_msec  Where to store milliseconds when the next event will elapse.
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_timer_time_to_next_event(ina_timer_t *timer,
                                               time_t *how_long_msec);

#ifdef __cplusplus
}
#endif

#endif
