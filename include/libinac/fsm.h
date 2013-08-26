/*
 * Copyright (c) 2013, INAOS GmbH
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
typedef void (*ina_fsm_action_fn_t)();

/* FSM transition */
typedef struct ina_fsm_transistion_s {
    ina_fsm_action_fn_t action;   /* Action */
    int32_t next_state;           /* Next state after action */
} ina_fsm_transistion_t;

/* Defines single a FSM state */
#define INA_FSM_STATE(state) state
/* Defines FSM states */
#define INA_FSM_STATES(id, ...)                                              \
    enum __##id##_fsm_states {                                               \
            __VA_ARGS__,                                                     \
            __##id##_MAX_STATES } __##id##_fsm_state

/* Defines a single FSM event */
#define INA_FSM_EVENT(event) event
/* Defines FSM events */
#define INA_FSM_EVENTS(id, ...)                                              \
    enum __##id##_fsm_events {                                               \
            __VA_ARGS__,                                                     \
            __##id##_MAX_EVENTS } __##id##_fsm_event

/* Defines a FSM transition map. For each event all states must be 
 * defined in the map */
#define INA_FSM_TRANSITIONS(id, ...)                                         \
    ina_fsm_transistion_t __##id##_fsm_transitions                           \
          [__##id##_MAX_EVENTS][__##id##_MAX_STATES] = {                     \
          __VA_ARGS__                                                        \
    }
/* Defines transitions for a single event for a FSM */ 
#define INA_FSM_TRANSITION_EVENT(event, ...)                                 \
    { __VA_ARGS__ }
/* Define a state transition in a FSM transition map */
#define INA_FSM_TRANSITION(state, action, next_state)                        \
    { action, next_state }

/* Get the current state of an FSM */
#define INA_FSM_GET_STATE(id)                                                \
    (__##id##_fsm_state)

/* Set the current state of a FSM */
#define INA_FSM_SET_STATE(id, state)                                         \
    __##id##_fsm_state = state                          

/* Set event for a FSM */
#define INA_FSM_SET_EVENT(id, event)                                         \
    __##id##_fsm_event = event

/* Get last set event of a FSM */
#define INA_FSM_GET_EVENT(id)                                                \
    (__##id##_fsm_event)

/* Get the next state for a FSM */
#define INA_FSM_NEXT_STATE(id, context_ptr)                                  \
{                                                                            \
     int32_t new_state = __##id##_fsm_transitions                            \
         [__##id##_fsm_event][__##id##_fsm_state].next_state;                \
    __##id##_fsm_transitions                                                 \
         [__##id##_fsm_event][__##id##_fsm_state].action(context_ptr);       \
    __##id##_fsm_state = new_state; } 

#ifdef __cplusplus
}
#endif
#endif
