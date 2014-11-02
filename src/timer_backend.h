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
#ifndef _LIBINAC_TIMER_BACKEND_H_
#define _LIBINAC_TIMER_BACKEND_H_

#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ina_timer_backend_s ina_timer_backend_t;
typedef struct ina_timer_backend_event_s ina_timer_backend_event_t;

/*
 * Initialize timer backend
 */
INA_API(ina_rc_t) ina_timer_backend_init(ina_timer_backend_t **backend);

/*
 * Destroy timer backend
 */
INA_API(ina_rc_t) ina_timer_backend_destroy(ina_timer_backend_t **backend);

/*
 * Create a new backend event for a timer
 */
INA_API(ina_time_event_t*) ina_timer_backend_create_event(ina_timer_backend_t *backend, 
                                                          ina_time_event_t *e,
                                                          time_t n_msec,
                                                          time_t e_msec);
/*
 * Delete a backend event from a timer
 */
INA_API(ina_rc_t) ina_timer_backend_delete_event(ina_timer_backend_t *backend, ina_time_event_t *e);

/*
 * Get the next elapsed time event by providing the milli-seconds since epoch
 */
INA_API(ina_time_event_t*) ina_timer_backend_next_event_with_time(ina_timer_backend_t *backend, time_t now_millis);

/*
 *  Calculate time in msec until the next time event will elapse.
 */
INA_API(ina_rc_t) ina_timer_backend_time_to_next_event(ina_timer_backend_t *backend, time_t now_millis, time_t *how_long_msec);

#ifdef __cplusplus
}
#endif

#endif
