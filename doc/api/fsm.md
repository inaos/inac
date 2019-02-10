
```C
#ifndef _LIBINAC_FSM_H_
```

Copyright INAOS GmbH, Thalwil, 2013-2018. All rights reserved

This software is the confidential and proprietary information of INAOS GmbH
("Confidential Information"). You shall not disclose such Confidential
Information and shall use it only in accordance with the terms of the
license agreement you entered into with INAOS GmbH.

```C
typedef void (*ina_fsm_action_fn_t)(void *userdata);
```
FSM action function prototype
```C
typedef uint16_t ina_fsm_status_t;/* FSM state type */
```
FSM status, holding current state and last event
```C
typedef struct ina_fsm_transition_s {
```
FSM transition
```C
#define INA_FSM_STATE(state) state
```
Defines single a FSM state
```C
#define INA_FSM_STATES(id, ...)                                              \
```
Defines FSM states
```C
#define INA_FSM_EVENT(event) event
```
Defines a single FSM event
```C
#define INA_FSM_EVENTS(id, ...)                                              \
```
Defines FSM events
```C
#define INA_FSM_TRANSITIONS(id, ...)                                                    \
```
Defines a FSM transition map. For each event all states must be
defined in the map
```C
#define INA_FSM_TRANSITION_EVENT(event, ...)                                 \
```
Defines transitions for a single event for a FSM
```C
#define INA_FSM_TRANSITION(state, action, next_state)                        \
```
Define a state transition in a FSM transition map
```C
#define INA_FSM_NO_TRANSITION(state)                                         \
```
Define a non state transition in a FSM transition map
```C
#define INA_FSM_GET_STATE(id, status)                                        \
```
Get the current state of an FSM
```C
#define INA_FSM_SET_STATE(id, status, new_state)                             \
```
Set the current state of a FSM
```C
#define INA_FSM_SET_EVENT(id, status, new_event)                             \
```
Set event for a FSM
```C
#define INA_FSM_GET_EVENT(id, status)                                        \
```
Get last set event of a FSM
```C
#define INA_FSM_NEXT_STATE(id, status, userdata)                             \
```
Get the next state for a FSM
```C
#define INA_FSM_FIRE_EVENT(id, status, event, userdata)                      \
```
Fire event