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

INA_TEST(ullc, slow_consumer)
{
    ina_ullc_ctx_t *ullc;
    ina_test_ullc_t *v = NULL;
    ina_test_hid_t hid;
    int old;

    INA_TEST_HELPER_INVOKE(&hid, ullc, create_fast_producer,  
         INA_NUM2STR(1), INA_NUM2STR(128), INA_NUM2STR(2),
         INA_NUM2STR(1), "/ina_ullc_test3", NULL);

    INA_TEST_ASSERT_SUCCEED(INA_ULLC_CONSUMER_NEW(ina_test_ullc_t,
        1, 
        128, 
        2, 
        1, 
        "/ina_ullc_test3",         
        &ullc));

    ina_ullc_overrun_disable(ullc);

    old = -1;
    while (1) {
        v = INA_ULLC_GET(ina_test_ullc_t, ullc);
        if (v) {
            ina_time_sleep(5);
            INA_TRACE3("consumer %d, v=%d", ullc->id, v->i3);
            if (v->i3 == -1) {
                break;
            }
            if (v->i3 != old+1) {
                INA_TEST_MSG("Overrun at %d(last = %d)", v->i3, old);
                INA_TEST_ASSERT_EQUAL_INT(v->i3, old+1);
            }
            old = v->i3;
        }
        ina_time_sleep(5);
    }
    ina_ullc_consumer_free(&ullc);
}

INA_TEST(ullc, multiproducer)
{
    ina_ullc_ctx_t *ullc1;
    ina_ullc_ctx_t *ullc2;
    ina_ullc_ctx_t *ullc3;
    ina_test_ullc_t *v = NULL;
    int c =  0;
    ina_test_hid_t hid1;
    ina_test_hid_t hid2;

    INA_TEST_ASSERT_SUCCEED(INA_ULLC_PRODUCER_NEW(ina_test_ullc_t,
        1, 
        128, 
        3, 
        2, 
        "/ina_ullc_test", 
        INA_ULLC_WS_SIGNAL_WAIT, 
        &ullc1));


    INA_TEST_ASSERT_SUCCEED(INA_ULLC_PRODUCER_NEW(ina_test_ullc_t,
        1, 
        128, 
        3, 
        2, 
        "/ina_ullc_test", 
        INA_ULLC_WS_SIGNAL_WAIT, 
        &ullc2));

    INA_TEST_ASSERT_SUCCEED(INA_ULLC_PRODUCER_NEW(ina_test_ullc_t,
        1, 
        128, 
        3, 
        2, 
        "/ina_ullc_test", 
        INA_ULLC_WS_SIGNAL_WAIT, 
        &ullc3));
 
    ina_ullc_overrun_enable(ullc3);

    INA_TEST_HELPER_INVOKE(&hid1, ullc, create_consumer,  
         INA_NUM2STR(1), INA_NUM2STR(128), INA_NUM2STR(3),
         INA_NUM2STR(2), "/ina_ullc_test", NULL);

    INA_TEST_HELPER_INVOKE(&hid2, ullc, create_consumer,  
         INA_NUM2STR(1), INA_NUM2STR(128), INA_NUM2STR(3),
         INA_NUM2STR(2), "/ina_ullc_test", NULL);

    for (c = 0; c < 1000; c++) {
        v = INA_ULLC_CLAIM(ina_test_ullc_t, ullc1);
        v->d1 += c;
        v->d2 -= c;
        v->i3 = (int32_t)fabs(v->d1*v->d2);
        INA_ULLC_COMMIT(ullc1);
        v = INA_ULLC_CLAIM(ina_test_ullc_t, ullc2);
        v->d1 += c;
        v->d2 -= c;
        v->i3 = (int32_t)fabs(v->d1*v->d2);
        INA_ULLC_COMMIT(ullc2);
        v = INA_ULLC_CLAIM(ina_test_ullc_t, ullc3);
        v->d1 += c;
        v->d2 -= c;
        v->i3 = (int32_t)fabs(v->d1*v->d2);
        INA_ULLC_COMMIT(ullc3);
        ina_time_sleep(1);
    }
    ina_time_sleep(2000);
    v = INA_ULLC_CLAIM(ina_test_ullc_t, ullc1);
    v->i3 = -1;
    INA_ULLC_COMMIT(ullc1);

    ina_ullc_producer_free(&ullc1);
    ina_ullc_producer_free(&ullc2);
    ina_ullc_producer_free(&ullc3);
}

INA_TEST(ullc, consumer_get_set_pos)
{
    ina_ullc_ctx_t *consumer1;
    ina_ullc_ctx_t *consumer2;
    ina_ullc_ctx_t *producer;
    ina_test_ullc_t *v = NULL;
    size_t c;
    int64_t pos;
 
    INA_TEST_ASSERT_SUCCEED(INA_ULLC_PRODUCER_NEW(ina_test_ullc_t,
        1, 
        1000, 
        1, 
        2, 
        "/ina_ullc_test2", 
        INA_ULLC_WS_SIGNAL_WAIT, 
        &producer));

    INA_TEST_ASSERT_SUCCEED(INA_ULLC_CONSUMER_NEW(ina_test_ullc_t,
        1, 
        1000, 
        1, 
        2, 
        "/ina_ullc_test2", 
        &consumer1));

    INA_TEST_ASSERT_SUCCEED(INA_ULLC_CONSUMER_NEW(ina_test_ullc_t,
        1, 
        1000, 
        1, 
        2, 
        "/ina_ullc_test2", 
        &consumer2));

    ina_ullc_overrun_enable(producer);

    INA_TEST_ASSERT_SUCCEED(ina_ullc_producer_get_pos(producer, &pos));
    v = INA_ULLC_GET(ina_test_ullc_t, consumer1);
    INA_TEST_ASSERT_NULL(v);
    
    INA_TEST_ASSERT_SUCCEED(ina_ullc_consumer_set_pos(consumer1, -1));
    v = INA_ULLC_GET(ina_test_ullc_t, consumer1);
    INA_TEST_ASSERT_NULL(v);
    INA_TEST_ASSERT_EQUAL_INT64(0, pos);

    for (c = 0; c < 125; c++) {
        v = INA_ULLC_CLAIM(ina_test_ullc_t, producer);
        v->d1 += c;
        v->d2 -= c;
        v->i3 = (int32_t )fabs(v->d1*v->d2);
        INA_ULLC_COMMIT(producer);
    }

    INA_TEST_ASSERT_SUCCEED(ina_ullc_consumer_get_pos(consumer1, &pos));
    INA_TEST_ASSERT_EQUAL_INT64(0, pos);
    v = INA_ULLC_GET(ina_test_ullc_t, consumer1);
    INA_TEST_ASSERT_NOT_NULL(v);
    INA_TEST_ASSERT_EQUAL_FLOATING(0.0, v->d1);
    INA_TEST_ASSERT_SUCCEED(ina_ullc_consumer_get_pos(consumer1, &pos));
    INA_TEST_ASSERT_EQUAL_INT64(1, pos);
    v = INA_ULLC_GET(ina_test_ullc_t, consumer1);
    INA_TEST_ASSERT_NOT_NULL(v);
    INA_TEST_ASSERT_EQUAL_FLOATING(1.0, v->d1);
    INA_TEST_ASSERT_SUCCEED(ina_ullc_consumer_set_pos(consumer1, 102));
    v = INA_ULLC_GET(ina_test_ullc_t, consumer1);
    INA_TEST_ASSERT_NOT_NULL(v);
    INA_TEST_ASSERT_EQUAL_FLOATING(102.0, v->d1);
    INA_TEST_ASSERT_SUCCEED(ina_ullc_consumer_get_pos(consumer1, &pos));
    INA_TEST_ASSERT_EQUAL_INT64(103, pos);
    INA_TEST_ASSERT_SUCCEED(ina_ullc_consumer_set_pos(consumer1, 1024));
    INA_TEST_ASSERT_SUCCEED(ina_ullc_consumer_get_pos(consumer1, &pos));
    INA_TEST_ASSERT_EQUAL_INT64(125, pos);
    v = INA_ULLC_GET(ina_test_ullc_t, consumer1);
    INA_TEST_ASSERT_NULL(v);
    v = INA_ULLC_CLAIM(ina_test_ullc_t, producer);
    v->d1 = 9999;
    v->d2 = 1111;
    v->i3 = (int32_t )fabs(v->d1*v->d2);
    INA_ULLC_COMMIT(producer); 
    v = INA_ULLC_GET(ina_test_ullc_t, consumer1);
    INA_TEST_ASSERT_NOT_NULL(v);
 
    INA_TEST_ASSERT_SUCCEED(ina_ullc_consumer_set_pos(consumer2, -1));
    INA_TEST_ASSERT_SUCCEED(ina_ullc_consumer_get_pos(consumer2, &pos));
    INA_TEST_ASSERT_EQUAL_INT64(126, pos);
    v = INA_ULLC_GET(ina_test_ullc_t, consumer2);
    INA_TEST_ASSERT_NULL(v);

    ina_ullc_producer_free(&producer);
    ina_ullc_consumer_free(&consumer1);
    ina_ullc_consumer_free(&consumer2);
}

