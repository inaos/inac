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
#include <libinac/lib.h>
#include "test_ullc.h"

static ina_ullc_ctx_t *ullc_ctx = NULL;


static void ina_test_helper_cleanup_producer(int error, int *exitcode) {
    if (ullc_ctx) {
        ina_ullc_producer_destroy(&ullc_ctx);
    }
}

static void ina_test_helper_cleanup_consumer(int error, int *exitcode) {
    if (ullc_ctx) {
        ina_ullc_consumer_destroy(&ullc_ctx);
    }
}

/* Create a single */
INA_TEST_HELPER(ullc, create_producer) {
    const char* name;
    size_t consumers;
    size_t producers;
    size_t slots;
    int16_t version;
 
    ina_set_cleanup_handler(ina_test_helper_cleanup_producer);

    INA_TEST_HELPER_CHECK_ARGC(5);
    version = (int16_t)INA_TEST_HELPER_IARG(0);
    slots = (size_t)INA_TEST_HELPER_IARG(1);
    producers = (size_t)INA_TEST_HELPER_IARG(2);
    consumers = (size_t)INA_TEST_HELPER_IARG(3);
    name = INA_TEST_HELPER_CARG(4);

    if (!INA_SUCCEED(INA_ULLC_PRODUCER_CREATE(ina_test_ullc_t, 
        version, 
        slots, 
        producers, 
        consumers, 
        name, 
        INA_ULLC_WS_SIGNAL_WAIT, 
        &ullc_ctx))) {
        INA_TEST_HELPER_SET_RC(INA_ERR_PUSH_LAST);
    }
 }

/* Create a single */
INA_TEST_HELPER(ullc, create_consumer) {
    const char* name;
    size_t consumers;
    size_t producers;
    size_t slots;
    int16_t version;
    ina_ullc_ctx_t *ullc_ctx = NULL;
    ina_test_ullc_t *v = NULL;

    ina_set_cleanup_handler(ina_test_helper_cleanup_consumer);

    INA_TEST_HELPER_CHECK_ARGC(5);
    version = (int16_t)INA_TEST_HELPER_IARG(0);
    slots = (size_t)INA_TEST_HELPER_IARG(1);
    producers = (size_t)INA_TEST_HELPER_IARG(2);
    consumers = (size_t)INA_TEST_HELPER_IARG(3);
    name = INA_TEST_HELPER_CARG(4);


    if (!INA_SUCCEED(INA_ULLC_CONSUMER_CREATE(ina_test_ullc_t, 
            version, 
            slots, 
            producers, 
            consumers, 
            name, 
            &ullc_ctx))) {
            INA_TEST_HELPER_EXIT(INA_ERR_PUSH_LAST);
    }

   INA_TRACE3("created ullc consumer: version %d, slots:%ld, producers %ld, consumers %ld, name %s",
        version, slots, producers, consumers, name);
 
    while (1) {
        v = INA_ULLC_GET(ina_test_ullc_t, ullc_ctx);
        if (v) {
            INA_TRACE3("consumer %d, v=%d", ullc_ctx->id, v->i3);
            if (v->i3 == -1) {
                break;
            }
        }
        ina_time_sleep(1);
    }
    INA_TRACE3("ullc consumer %d exit", ullc_ctx->id);
    INA_TEST_HELPER_SET_RC(INA_SUCCESS);
}
