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

/* Stupid signal light */
typedef struct signal_light_s {
    int is_on;
    int is_red;
    int is_orange;
    int is_green;
    int c;
} signal_light_t;

/* Define States */
INA_FSM_STATES(my_fsm, 
    INA_FSM_STATE(OFF),
    INA_FSM_STATE(ON),
    INA_FSM_STATE(RED),
    INA_FSM_STATE(ORANGE),
    INA_FSM_STATE(GREEN));

/* Define Events */
INA_FSM_EVENTS(my_fsm, 
    INA_FSM_EVENT(TURN_ON_OFF),
    INA_FSM_EVENT(SWITCH));
    
void do_nothing(signal_light_t *sl)
{
    
}

void turn_on(signal_light_t *sl)
{
    INA_TRACE2("turn_on");
    INA_TEST_ASSERT_TRUE(sl->is_on == INA_NO);
    INA_TEST_ASSERT_TRUE(sl->is_red == INA_NO);
    INA_TEST_ASSERT_TRUE(sl->is_orange == INA_NO);
    INA_TEST_ASSERT_TRUE(sl->is_green == INA_NO);    
    INA_TEST_ASSERT_TRUE(sl->c == 0);
    sl->is_on = INA_YES;
    INA_FSM_SET_EVENT(my_fsm, SWITCH);
}

void turn_off(signal_light_t *sl)
{
    INA_TRACE2("turn_off");
    INA_TEST_ASSERT_TRUE(sl->is_on == INA_YES);
    INA_TEST_ASSERT_TRUE(sl->is_red == INA_NO );
    INA_TEST_ASSERT_TRUE(sl->is_orange == INA_NO);
    INA_TEST_ASSERT_TRUE(sl->is_green == INA_YES);
    INA_TEST_ASSERT_TRUE(sl->c == 3);
    sl->is_on = INA_NO;
    sl->is_green = INA_NO;
}

void turn_red_on(signal_light_t *sl)
{
    INA_TRACE2("turn_red_on");
    INA_TEST_ASSERT_TRUE(sl->is_on == INA_YES);
    INA_TEST_ASSERT_TRUE(sl->is_red == INA_NO );
    INA_TEST_ASSERT_TRUE(sl->is_orange == INA_NO);
    if (sl->c > 0) {
        INA_TEST_ASSERT_TRUE(sl->is_green == INA_YES);
    } else {
        INA_TEST_ASSERT_TRUE(sl->is_green == INA_NO);    
    }
    sl->is_green = INA_NO;
    sl->is_red = INA_YES;
}

void turn_orange_on(signal_light_t *sl)
{
    INA_TRACE2("turn_orange_on");
    INA_TEST_ASSERT_TRUE(sl->is_on == INA_YES);
    INA_TEST_ASSERT_TRUE(sl->is_red == INA_YES);
    INA_TEST_ASSERT_TRUE(sl->is_orange == INA_NO);
    INA_TEST_ASSERT_TRUE(sl->is_green == INA_NO);
    sl->is_red = INA_NO;
    sl->is_orange = INA_YES;
}

void turn_green_on(signal_light_t *sl)
{
    INA_TRACE2("turn_green_on");
    INA_TEST_ASSERT_TRUE(sl->is_on == INA_YES);
    INA_TEST_ASSERT_TRUE(sl->is_red == INA_NO);
    INA_TEST_ASSERT_TRUE(sl->is_orange == INA_YES);
    INA_TEST_ASSERT_TRUE(sl->is_green == INA_NO);
    sl->is_orange = INA_NO;
    sl->is_green = INA_YES;
    sl->c++;
    if (sl->c == 3) {
        INA_FSM_SET_EVENT(my_fsm, TURN_ON_OFF);
    }
}

/* Define Transistion Map */
INA_FSM_TRANSITIONS(my_fsm,
    INA_FSM_TRANSITION_EVENT(TURN_ON_OFF,
        INA_FSM_TRANSITION(OFF, turn_on, ON),
        INA_FSM_TRANSITION(ON, turn_off, OFF),
        INA_FSM_TRANSITION(RED, turn_off, OFF),
        INA_FSM_TRANSITION(ORANGE, turn_off, OFF),
        INA_FSM_TRANSITION(GREEN, turn_off, OFF)),
    INA_FSM_TRANSITION_EVENT(SWITCH,
        INA_FSM_TRANSITION(OFF, do_nothing, ON),    
        INA_FSM_TRANSITION(ON, turn_red_on, RED),
        INA_FSM_TRANSITION(RED, turn_orange_on, ORANGE), 
        INA_FSM_TRANSITION(ORANGE, turn_green_on, GREEN),
        INA_FSM_TRANSITION(GREEN, turn_red_on, RED)));

INA_TEST(fsm, signal_light)
{
    signal_light_t sl;
    ina_mem_set(&sl, 0, sizeof(signal_light_t));

    /* Set initial state */
    INA_FSM_SET_STATE(my_fsm, OFF);
    
    /* Set start Event */
    INA_FSM_SET_EVENT(my_fsm, TURN_ON_OFF);
    
    /* Next State */
    INA_FSM_NEXT_STATE(my_fsm, &sl);
    
    /* Run util OFF */
    while (INA_FSM_GET_STATE(my_fsm) != OFF) {
        INA_FSM_NEXT_STATE(my_fsm, &sl);
    }
    INA_TEST_ASSERT_TRUE(sl.c == 3);
}

