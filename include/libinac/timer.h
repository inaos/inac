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
#ifndef _LIBINAC_TIMER_H_
#define _LIBINAC_TIMER_H_

#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Timer */
typedef struct ina_timer_s ina_timer_t;

/* Time event */
typedef struct ina_time_event_s {
    uint64_t id;
    time_t msec;
    time_t when_sec;
    time_t when_msec;
} ina_time_event_t;

/*
 * Create an new timer 
 */
INA_API(ina_rc_t) ina_timer_init(ina_timer_t **timer);
/*
 * Destroty a timer
 */
INA_API(ina_rc_t) ina_timer_destroy(ina_timer_t **timer);
/*
 * Enable RDTSC, make sure you understand the caveats 
 * -> check time.h
 * -> Only use on modern processors with Invariant TSC
 * -> Processes must be pinned to CPU
 * Per default RDTSC is disabled
 */
INA_API(ina_rc_t) ina_timer_use_rdtsc(ina_timer_t *timer, int yesno);
/*
 * Create a new time event for a timer
 */
INA_API(ina_time_event_t*) ina_timer_create_event(ina_timer_t *timer, time_t msec);
/*
 * Delete a time event from a timer
 */
INA_API(ina_rc_t) ina_timer_delete_event(ina_timer_t *timer, ina_time_event_t *e);
/*
 * Get the next elapsed time event
 */
INA_API(ina_time_event_t*) ina_timer_next_event(ina_timer_t *timer);
/*
 *  Calculate time in msec until the next time event will elapse.
 */
INA_API(ina_rc_t) ina_timer_time_to_next_event(ina_timer_t *timer, time_t *how_long_msec);
/*
 * Sleep a timer for msec. 
 */
INA_API(ina_rc_t) ina_timer_sleep(ina_timer_t *timer, time_t msec);

#ifdef __cplusplus
}
#endif

#endif
