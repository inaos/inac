/*
 * Copyright 2013-2020 INAOS GmbH, Thalwil
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _LIBINAC_FSM_H_
#define _LIBINAC_FSM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <libinac/lib.h>

/* FSM action function prototype  */
typedef void (*ina_fsm_action_fn_t)(void *userdata);

/* FSM status, holding current state and last event */
typedef uint16_t ina_fsm_status_t;
/* FSM state type */
typedef uint8_t  ina_fsm_state_t;


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
    ina_fsm_transition_t __##id##_fsm_transitions                                       \
          [id##_MAX_EVENTS][id##_MAX_STATES] = {                                        \
          __VA_ARGS__                                                                   \
    };                                                                                  \
    INA_INLINE id##_fsm_state_t id##_next_fsm_state(ina_fsm_status_t *s, void* u) {     \
        ina_fsm_transition_t *__fsmt = &__##id##_fsm_transitions                        \
            [id##_get_fsm_event(*s)][id##_get_fsm_state(*s)];                           \
        id##_fsm_state_t new_state = (id##_fsm_state_t)__fsmt->next_state;              \
        if (__fsmt->action) __fsmt->action(u);                                          \
        id##_set_fsm_state(s, new_state);                                               \
        return new_state;                                                               \
    }                                                                                   \
    INA_INLINE id##_fsm_state_t id##_fsm_fire_event(ina_fsm_status_t *s,                \
        id##_fsm_event_t e, void* u) {                                                  \
        ina_fsm_transition_t *__fsmt = &__##id##_fsm_transitions                        \
            [e][id##_get_fsm_state(*s)];                                                \
        id##_fsm_state_t new_state = (id##_fsm_state_t)__fsmt->next_state;              \
        if (__fsmt->action) __fsmt->action(u);                                          \
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
    { NULL, state }

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
