/* Copyright (c) 2013, INAOS GmbH
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

/* 
 * Create a stop watch with an given ID, makes 3 time stamps each 10 ms
 * beetween.
 */
INA_TEST_HELPER(time_ipc, stopwatch_create) {
    int32_t id;
    ina_stopwatch_t *w = NULL;
    char user_data[INA_TIME_MAX_USERDATA_LEN+10];

    INA_TEST_HELPER_CHECK_ARGC(1);
    id = INA_TEST_HELPER_IARG(0);

    if (!INA_SUCCEED(INA_TIME_STOPWATCH_CREATE(&w, id, -1))) {
        INA_TEST_HELPER_SET_RC(ina_err_peek());
        return;
    }

    ina_mem_set(&user_data, 'a', INA_TIME_MAX_USERDATA_LEN+8);
    user_data[INA_TIME_MAX_USERDATA_LEN+9] = '\0';

    INA_TIME_STOPWATCH_START(w);
    ina_time_sleep(10);
    INA_TIME_STOPWATCH_STAMP(w);
    ina_time_sleep(10);
    INA_TIME_STOPWATCH_STAMP1(w, "user_data1");
    ina_time_sleep(10);
    INA_TIME_STOPWATCH_STAMP2(w, "user_data1", "user_data2");
    ina_time_sleep(10);
    INA_TIME_STOPWATCH_STAMP2(w, user_data, user_data);
    
    while (INA_SUCCEED(ina_time_stopwatch_started(w))) {
        ina_time_sleep(100);
    }
    INA_TIME_STOPWATCH_DESTROY(&w);
    INA_TEST_HELPER_SET_RC(INA_SUCCESS);
}
