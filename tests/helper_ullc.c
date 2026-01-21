/*
 * Copyright INAOS GmbH, Thalwil, 2014-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>
#include "test_ullc.h"

static ina_ullc_ctx_t *ullc_ctx = NULL;


static void ina_test_helper_cleanup_producer(int error, int *exitcode) {
    INA_UNUSED(error);
    INA_UNUSED(exitcode);
    if (ullc_ctx) {
        ina_ullc_producer_free(&ullc_ctx);
    }
}

static void ina_test_helper_cleanup_consumer(int error, int *exitcode) {
    INA_UNUSED(error);
    INA_UNUSED(exitcode);
    if (ullc_ctx) {
        ina_ullc_consumer_free(&ullc_ctx);
    }
}

/* Create a single */
INA_TEST_HELPER(ullc, create_fast_producer) {
    const char* name;
    int consumers;
    int producers;
    size_t slots;
    int16_t version;
    ina_test_ullc_t *v = NULL;
    int c;
 
    ina_set_cleanup_handler(ina_test_helper_cleanup_producer);

    INA_TEST_HELPER_CHECK_ARGC(5);
    version = (int16_t)INA_TEST_HELPER_IARG(0);
    slots = (size_t)INA_TEST_HELPER_IARG(1);
    producers = INA_TEST_HELPER_IARG(2);
    consumers = INA_TEST_HELPER_IARG(3);
    name = INA_TEST_HELPER_CARG(4);

  
    if (!INA_SUCCEED(INA_ULLC_PRODUCER_NEW(ina_test_ullc_t,
        version, 
        slots, 
        producers, 
        consumers, 
        name, 
        INA_ULLC_WS_SIGNAL_WAIT, 
        &ullc_ctx))) {
        INA_TEST_HELPER_SET_RC(ina_err_get_rc());
    }


   INA_TRACE3(inac.test.ullc, "created ullc producer: version %d, slots:%zu, producers %d, consumers %d, name %s",
        version, slots, producers, consumers, name);

    ina_time_sleep(2000);

    for (c = 0; c < 1000; c++) {
        v = INA_ULLC_CLAIM(ina_test_ullc_t, ullc_ctx);
            v->i3 = c;
            INA_ULLC_COMMIT(ullc_ctx);
            ina_time_sleep(1);
    }

    v = INA_ULLC_CLAIM(ina_test_ullc_t, ullc_ctx);
    v->i3 = -1;
    INA_ULLC_COMMIT(ullc_ctx);
    INA_TRACE3(inac.test.ullc, "ullc producer %p exit", ullc_ctx);
 }

/* Create a single */
INA_TEST_HELPER(ullc, create_consumer) {
    const char* name;
    int consumers;
    int producers;
    size_t slots;
    int16_t version;
    ina_ullc_ctx_t *ctx = NULL;
    ina_test_ullc_t *v = NULL;

    ina_set_cleanup_handler(ina_test_helper_cleanup_consumer);

    INA_TEST_HELPER_CHECK_ARGC(5);
    version = (int16_t)INA_TEST_HELPER_IARG(0);
    slots = (size_t)INA_TEST_HELPER_IARG(1);
    producers = INA_TEST_HELPER_IARG(2);
    consumers = INA_TEST_HELPER_IARG(3);
    name = INA_TEST_HELPER_CARG(4);


    if (!INA_SUCCEED(INA_ULLC_CONSUMER_NEW(ina_test_ullc_t,
            version, 
            slots, 
            producers, 
            consumers, 
            name, 
            &ctx))) {
            INA_TEST_HELPER_EXIT(ina_err_get_rc());
    }

   INA_TRACE3(inac.test.ullc, "created ullc consumer: version %d, slots:%zu, producers %d, consumers %d, name %s",
        version, slots, producers, consumers, name);
 
    while (1) {
        v = INA_ULLC_GET(ina_test_ullc_t, ctx);
        if (v) {
            INA_TRACE3(inac.test.ullc, "consumer %p, v=%d", ctx, v->i3);
            if (v->i3 == -1) {
                break;
            }
        }
        ina_time_sleep(1);
    }
    INA_TRACE3(inac.test.ullc, "ullc consumer %p exit", ctx);
    INA_TEST_HELPER_SET_RC(INA_SUCCESS);
}
