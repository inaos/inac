/*
 * Copyright INAOS GmbH, Thalwil, 2013-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>

/* Stupid signal light */
typedef struct signal_light_s {
    int is_on;
    int is_red;
    int is_orange;
    int is_green;
    int c;
    ina_fsm_status_t fsm_status;
} signal_light_t;

/* Define States */
INA_FSM_STATES(signal_fsm, 
    INA_FSM_STATE(OFF),
    INA_FSM_STATE(ON),
    INA_FSM_STATE(RED),
    INA_FSM_STATE(ORANGE),
    INA_FSM_STATE(GREEN));

/* Define Events */
INA_FSM_EVENTS(signal_fsm, 
    INA_FSM_EVENT(TURN_ON_OFF),
    INA_FSM_EVENT(SWITCH));
    
void do_nothing(void *userdata)
{
    INA_UNUSED(userdata);
    
}

void turn_on(void *userdata)
{
    signal_light_t *sl = (signal_light_t*)userdata;
    INA_TRACE2("turn_on");
    INA_TEST_ASSERT_TRUE(sl->is_on == INA_NO);
    INA_TEST_ASSERT_TRUE(sl->is_red == INA_NO);
    INA_TEST_ASSERT_TRUE(sl->is_orange == INA_NO);
    INA_TEST_ASSERT_TRUE(sl->is_green == INA_NO);    
    INA_TEST_ASSERT_TRUE(sl->c == 0);
    sl->is_on = INA_YES;
    INA_FSM_SET_EVENT(signal_fsm, sl->fsm_status, SWITCH);
}

void turn_off(void *userdata)
{
    signal_light_t *sl = (signal_light_t*)userdata;
    INA_TRACE2("turn_off");
    INA_TEST_ASSERT_TRUE(sl->is_on == INA_YES);
    INA_TEST_ASSERT_TRUE(sl->is_red == INA_NO );
    INA_TEST_ASSERT_TRUE(sl->is_orange == INA_NO);
    INA_TEST_ASSERT_TRUE(sl->is_green == INA_YES);
    INA_TEST_ASSERT_TRUE(sl->c == 3);
    sl->is_on = INA_NO;
    sl->is_green = INA_NO;
}

void turn_red_on(void *userdata)
{
    signal_light_t *sl = (signal_light_t*)userdata;
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

void turn_orange_on(void *userdata)
{
    signal_light_t *sl = (signal_light_t*)userdata;
    INA_TRACE2("turn_orange_on");
    INA_TEST_ASSERT_TRUE(sl->is_on == INA_YES);
    INA_TEST_ASSERT_TRUE(sl->is_red == INA_YES);
    INA_TEST_ASSERT_TRUE(sl->is_orange == INA_NO);
    INA_TEST_ASSERT_TRUE(sl->is_green == INA_NO);
    sl->is_red = INA_NO;
    sl->is_orange = INA_YES;
}

void turn_green_on(void *userdata)
{
    signal_light_t *sl = (signal_light_t*)userdata;
    INA_TRACE2("turn_green_on");
    INA_TEST_ASSERT_TRUE(sl->is_on == INA_YES);
    INA_TEST_ASSERT_TRUE(sl->is_red == INA_NO);
    INA_TEST_ASSERT_TRUE(sl->is_orange == INA_YES);
    INA_TEST_ASSERT_TRUE(sl->is_green == INA_NO);
    sl->is_orange = INA_NO;
    sl->is_green = INA_YES;
    sl->c++;
    if (sl->c == 3) {
        INA_FSM_SET_EVENT(signal_fsm, sl->fsm_status, TURN_ON_OFF);
    }
}

/* Define Transistion Map */
INA_FSM_TRANSITIONS(signal_fsm,
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

INA_TEST(fsm, get_set_state)
{
    signal_light_t sl;
    ina_mem_set(&sl, 0, sizeof(signal_light_t));
    INA_UNUSED(data);

    /* Set initial state */
    INA_FSM_SET_STATE(signal_fsm, sl.fsm_status, ON);
    INA_TEST_ASSERT_TRUE(INA_FSM_GET_STATE(signal_fsm, sl.fsm_status) == ON);
    INA_FSM_SET_STATE(signal_fsm, sl.fsm_status, OFF);
    INA_TEST_ASSERT_TRUE(INA_FSM_GET_STATE(signal_fsm, sl.fsm_status) == OFF);    
}

INA_TEST(fsm, get_set_event)
{
    signal_light_t sl;
    ina_mem_set(&sl, 0, sizeof(signal_light_t));
    INA_UNUSED(data);

    /* Set initial state */
    INA_FSM_SET_EVENT(signal_fsm, sl.fsm_status, SWITCH);
    INA_TEST_ASSERT_TRUE(INA_FSM_GET_EVENT(signal_fsm, sl.fsm_status) == SWITCH);

    INA_FSM_SET_EVENT(signal_fsm, sl.fsm_status, TURN_ON_OFF);
    INA_TEST_ASSERT_TRUE(INA_FSM_GET_EVENT(signal_fsm, sl.fsm_status) == TURN_ON_OFF);
}

INA_TEST(fsm, signal_light)
{
    INA_UNUSED(data);

    signal_light_t sl;
    ina_mem_set(&sl, 0, sizeof(signal_light_t));

    /* Set initial state */
    INA_FSM_SET_STATE(signal_fsm, sl.fsm_status, OFF);
    INA_TEST_ASSERT_TRUE(INA_FSM_GET_STATE(signal_fsm, sl.fsm_status) == OFF);
    
    /* Set start Event */
    INA_FSM_SET_EVENT(signal_fsm, sl.fsm_status, TURN_ON_OFF);
    INA_TEST_ASSERT_TRUE(INA_FSM_GET_EVENT(signal_fsm, sl.fsm_status) == TURN_ON_OFF);

    /* Run until OFF */
    while (INA_FSM_NEXT_STATE(signal_fsm, sl.fsm_status, &sl) != OFF){
        INA_TRACE2("event=%d, state=%d", INA_HIGH(sl.fsm_status), INA_LOW(sl.fsm_status));
    };
    INA_TEST_ASSERT_TRUE(INA_FSM_GET_STATE(signal_fsm, sl.fsm_status) == OFF);
    INA_TEST_ASSERT_TRUE(INA_FSM_GET_EVENT(signal_fsm, sl.fsm_status) == TURN_ON_OFF);
    INA_TEST_ASSERT_EQUAL_INT(3, sl.c);
}

