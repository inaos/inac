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

void test_timer_event()
{
    ina_timer_t *t;
    ina_time_event_t *e1;
    ina_time_event_t *e2;

    INA_TRACE_MSG("test_timer_event");
    
    t = NULL;
    e1 = NULL;
    e2 = NULL;
    INA_ASSERT_SUCCEED(ina_timer_init(&t));
    INA_ASSERT_NOTNULL(t);
    INA_ASSERT_SUCCEED(ina_timer_destroy(&t));
    INA_ASSERT_NULL(t);
    INA_ASSERT_SUCCEED(ina_timer_init(&t));
    INA_ASSERT_NOTNULL(t);
    e1 = ina_timer_create_event(t, 1000);
    INA_ASSERT_SUCCEED(ina_err_peek());
    INA_ASSERT_NOTNULL(e1);
    ina_time_sleep(1);
    e2 = ina_timer_next_event(t);
    INA_ASSERT_SUCCEED(ina_err_peek());
    INA_ASSERT_NOTNULL(e2);
    INA_ASSERT_EQUAL(e2, e1);
}
void test_timer_init_destroy() 
{
  
    ina_timer_t *t;

    INA_TRACE_MSG("test_timer_init_destroy");
    
    t = NULL;
    INA_ASSERT_SUCCEED(ina_timer_init(&t));
    INA_ASSERT_NOTNULL(t);
    INA_ASSERT_SUCCEED(ina_timer_destroy(&t));
    INA_ASSERT_NULL(t);
}