/*
 * Copyright (c) 2013-2014, INAOS GmbH
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
#ifndef _LIBINAC_FSM_H_
#define _LIBINAC_FSM_H_

#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* FSM action function prototype  */
typedef void (*ina_fsm_action_fn_t)(void *userdata);

/* FSM status, holding current state and last event */
typedef uint16_t ina_fsm_status_t;
/* FSM state type */
typedef uint8_t  ina_fsm_state_t;
/* FSM event type */
typedef uint8_t  ina_fsm_event_t;

/* FSM transition */
typedef struct ina_fsm_transition_s {
    ina_fsm_action_fn_t action;   /* Action */
    ina_fsm_state_t next_state;   /* Next state after action */
} ina_fsm_transition_t;

/* Defines single a FSM state */
#define INA_FSM_STATE(state) state
/* Defines FSM states */
#define INA_FSM_STATES(id, ...)                                              \
    typedef enum id##_fsm_state_e {                                          \
            __VA_ARGS__,                                                     \
            id##_MAX_STATES } id##_fsm_state_t;                              \
    INA_INLINE id##_fsm_state_t id##_get_fsm_state(ina_fsm_status_t s) {     \
        return (id##_fsm_state_t)INA_LOW(s); }                               \
    INA_INLINE void id##_set_fsm_state(ina_fsm_status_t *s, id##_fsm_state_t ns) \
        { *s = INA_TOWORD(INA_HIGH(*s), (uint8_t)ns);} 

/* Defines a single FSM event */
#define INA_FSM_EVENT(event) event
/* Defines FSM events */
#define INA_FSM_EVENTS(id, ...)                                              \
    typedef enum id##_fsm_event_e {                                          \
            __VA_ARGS__,                                                     \
            id##_MAX_EVENTS }  id##_fsm_event_t;                             \
    INA_INLINE id##_fsm_event_t id##_get_fsm_event(ina_fsm_status_t s) {     \
            return (id##_fsm_event_t)INA_HIGH(s); }                          \
    INA_INLINE void id##_set_fsm_event(ina_fsm_status_t *s, id##_fsm_event_t e) \
            { *s = INA_TOWORD((uint8_t)e, INA_LOW(*s));}

/* Defines a FSM transition map. For each event all states must be 
 * defined in the map */
#define INA_FSM_TRANSITIONS(id, ...)                                                    \
    static void __ina_fsm_noop(void *userdata) {}                                       \
    ina_fsm_transition_t __##id##_fsm_transitions                                       \
          [id##_MAX_EVENTS][id##_MAX_STATES] = {                                        \
          __VA_ARGS__                                                                   \
    };                                                                                  \
    INA_INLINE id##_fsm_state_t id##_next_fsm_state(ina_fsm_status_t *s, void* u) {     \
        ina_fsm_transition_t *__fsmt = &__##id##_fsm_transitions                        \
            [id##_get_fsm_event(*s)][id##_get_fsm_state(*s)];                           \
        id##_fsm_state_t new_state = (id##_fsm_state_t)__fsmt->next_state;              \
        id##_set_fsm_state(s, new_state);                                               \
        return new_state;                                                               \
    }                                                                                   \
    INA_INLINE id##_fsm_state_t id##_fsm_fire_event(ina_fsm_status_t *s,                \
        id##_fsm_event_t e, void* u) {                                                  \
        ina_fsm_transition_t *__fsmt = &__##id##_fsm_transitions                        \
            [e][id##_get_fsm_state(*s)];                                                \
        id##_fsm_state_t new_state = (id##_fsm_state_t)__fsmt->next_state;              \
        id##_set_fsm_state(s, new_state);                                               \
        return new_state;                                                               \
    }

/* Defines transitions for a single event for a FSM */ 
#define INA_FSM_TRANSITION_EVENT(event, ...)                                 \
    { __VA_ARGS__ }
/* Define a state transition in a FSM transition map */
#define INA_FSM_TRANSITION(state, action, next_state)                        \
    { action, next_state }
/* Define a non state transition in a FSM transition map */
#define INA_FSM_NO_TRANSITION(state)                                         \
    { __ina_fsm_noop, 0 }

/* Get the current state of an FSM */
#define INA_FSM_GET_STATE(id, status)                                        \
    id##_get_fsm_state(status)
        
/* Set the current state of a FSM */
#define INA_FSM_SET_STATE(id, status, new_state)                             \
    id##_set_fsm_state(&status, new_state)                         

/* Set event for a FSM */
#define INA_FSM_SET_EVENT(id, status, new_event)                             \
    id##_set_fsm_event(&status, new_event);
 
/* Get last set event of a FSM */
#define INA_FSM_GET_EVENT(id, status)                                        \
    id##_get_fsm_event(status)

/* Get the next state for a FSM */
#define INA_FSM_NEXT_STATE(id, status, userdata)                             \
  id##_next_fsm_state(&status, userdata)                                     \

/* Fire event */
#define INA_FSM_FIRE_EVENT(id, status, event, userdata)                      \
  id##_fsm_fire_event(&status, event, userdata)                              \

#ifdef __cplusplus
}
#endif
#endif
