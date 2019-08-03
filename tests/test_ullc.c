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

INA_TEST_DATA(ullc){};
INA_TEST(ullc, ina_ullc_producer_new)
{
    ina_ullc_ctx_t *ctx = NULL;
    INA_UNUSED(data);

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ullc_producer_new(0, sizeof(ina_test_ullc_t), 1024, 1, 1, "/ina_ullc_xx1", INA_ULLC_WS_BUSY_WAIT, &ctx));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ullc_producer_new(1, 0, 1024, 1, 1, "/ina_ullc_xx1", INA_ULLC_WS_BUSY_WAIT, &ctx));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ullc_producer_new(1, sizeof(ina_test_ullc_t), 0, 1, 1, "/ina_ullc_xx1", INA_ULLC_WS_BUSY_WAIT, &ctx));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ullc_producer_new(1, sizeof(ina_test_ullc_t), 1024, 0, 1, "/ina_ullc_xx1", INA_ULLC_WS_BUSY_WAIT, &ctx));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ullc_producer_new(1, sizeof(ina_test_ullc_t), 1024, (INA_ULLC_MAX_PRODUCERS+1), 1, "/ina_ullc_xx1", INA_ULLC_WS_BUSY_WAIT, &ctx));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ullc_producer_new(1, sizeof(ina_test_ullc_t), 1024, 1, 0, "/ina_ullc_xx1", INA_ULLC_WS_BUSY_WAIT, &ctx));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ullc_producer_new(1, sizeof(ina_test_ullc_t), 1024, 1, (INA_ULLC_MAX_CONSUMERS+1), "/ina_ullc_xx1", INA_ULLC_WS_BUSY_WAIT, &ctx));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ullc_producer_new(1, sizeof(ina_test_ullc_t), 1024, 1, 1, NULL, INA_ULLC_WS_BUSY_WAIT, &ctx));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ullc_producer_new(1, sizeof(ina_test_ullc_t), 1024, 1, 1, "", INA_ULLC_WS_BUSY_WAIT, &ctx));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ullc_producer_new(1, sizeof(ina_test_ullc_t), 1024, 1, 1, "/ina_ullc_xx1", INA_ULLC_WS_BUSY_WAIT, NULL));

    INA_TEST_ASSERT_SUCCEED(ina_ullc_producer_new(1, sizeof(ina_test_ullc_t), 1024, 1, 1, "/ina_ullc_xx1", INA_ULLC_WS_BUSY_WAIT, &ctx));
    INA_TEST_ASSERT_NOT_NULL(ctx);
    ina_ullc_producer_free(&ctx);
    INA_TEST_ASSERT_NULL(ctx);
}

INA_TEST(ullc, ina_ullc_get_ring_info)
{
    ina_ullc_rb_info_t info;
    ina_ullc_ctx_t *ctx = NULL;
    INA_UNUSED(data);

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ullc_get_ring_info(NULL, &info));

    INA_TEST_ASSERT_SUCCEED(ina_ullc_producer_new(1, sizeof(ina_test_ullc_t), 1024, 1, 1, "/ina_ullc_xx1", INA_ULLC_WS_BUSY_WAIT, &ctx));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ullc_get_ring_info("/ina_ullc_xx1", NULL));

    INA_TEST_ASSERT_SUCCEED(ina_ullc_get_ring_info("/ina_ullc_xx1", &info));
    ina_ullc_producer_free(&ctx);
}


INA_TEST(ullc, ina_ullc_reset_ring)
{
    ina_ullc_ctx_t *ctx = NULL;
    INA_UNUSED(data);

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ullc_reset_ring(NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ullc_reset_ring(""));
    INA_TEST_ASSERT_SUCCEED(ina_ullc_producer_new(1, sizeof(ina_test_ullc_t), 1024, 1, 1, "/ina_ullc_xx1", INA_ULLC_WS_BUSY_WAIT, &ctx));
    INA_TEST_ASSERT_SUCCEED(ina_ullc_reset_ring("/ina_ullc_xx1"));
    ina_ullc_producer_free(&ctx);
}

INA_TEST(ullc, ina_ullc_overrun_disable)
{
    ina_ullc_ctx_t *ctx = NULL;
    INA_UNUSED(data);

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ullc_overrun_disable(NULL));
    INA_TEST_ASSERT_SUCCEED(ina_ullc_producer_new(1, sizeof(ina_test_ullc_t), 1024, 1, 1, "/ina_ullc_xx1", INA_ULLC_WS_BUSY_WAIT, &ctx));
    INA_TEST_ASSERT_SUCCEED(ina_ullc_overrun_disable(ctx));
    ina_ullc_producer_free(&ctx);
}

INA_TEST(ullc, ina_ullc_overrun_enable)
{
    ina_ullc_ctx_t *ctx = NULL;
    INA_UNUSED(data);

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ullc_overrun_enable(NULL));
    INA_TEST_ASSERT_SUCCEED(ina_ullc_producer_new(1, sizeof(ina_test_ullc_t), 1024, 1, 1, "/ina_ullc_xx1", INA_ULLC_WS_BUSY_WAIT, &ctx));
    INA_TEST_ASSERT_SUCCEED(ina_ullc_overrun_disable(ctx));
    INA_TEST_ASSERT_SUCCEED(ina_ullc_overrun_enable(ctx));
    ina_ullc_producer_free(&ctx);
}

INA_TEST(ullc, ina_ullc_producer_commit) {
    ina_ullc_ctx_t *pctx = NULL;
    ina_ullc_ctx_t *cctx = NULL;
    INA_UNUSED(data);

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ullc_producer_commit(NULL));
    INA_TEST_ASSERT_SUCCEED(
            ina_ullc_producer_new(1, sizeof(ina_test_ullc_t), 1024, 1, 1, "/ina_ullc_xx1", INA_ULLC_WS_BUSY_WAIT,
                                  &pctx));
    INA_TEST_ASSERT_SUCCEED(ina_ullc_consumer_new(1, sizeof(ina_test_ullc_t), 1024, 1, 1, "/ina_ullc_xx1", &cctx));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ullc_producer_commit(cctx));
    INA_TEST_ASSERT_SUCCEED(ina_ullc_producer_commit(pctx));
    ina_ullc_consumer_free(&cctx);
    ina_ullc_producer_free(&pctx);
}

INA_TEST(ullc, ina_ullc_producer_get_pos)
{
    ina_ullc_ctx_t *pctx = NULL;
    ina_ullc_ctx_t *cctx = NULL;
    int64_t pos  = -1;
    INA_UNUSED(data);

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ullc_producer_get_pos(NULL, &pos));
    INA_TEST_ASSERT_SUCCEED(ina_ullc_producer_new(1, sizeof(ina_test_ullc_t), 1024, 1, 1, "/ina_ullc_xx1", INA_ULLC_WS_BUSY_WAIT, &pctx));
    INA_TEST_ASSERT_SUCCEED(ina_ullc_consumer_new(1, sizeof(ina_test_ullc_t), 1024, 1, 1, "/ina_ullc_xx1", &cctx));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ullc_producer_get_pos(NULL, &pos));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ullc_producer_get_pos(pctx, NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ullc_producer_get_pos(cctx, &pos));

    INA_TEST_ASSERT_SUCCEED(ina_ullc_producer_get_pos(pctx, &pos));
    INA_TEST_ASSERT_EQUAL_INT64(0, pos);
    INA_TEST_ASSERT_SUCCEED(ina_ullc_producer_commit(pctx));
    INA_TEST_ASSERT_SUCCEED(ina_ullc_producer_get_pos(pctx, &pos));
    INA_TEST_ASSERT_EQUAL_INT64(1, pos);
    ina_ullc_consumer_free(&cctx);
    ina_ullc_producer_free(&pctx);
}

INA_TEST(ullc, ina_ullc_consumer_new)
{
    ina_ullc_ctx_t *cctx = NULL;
    ina_ullc_ctx_t *pctx = NULL;
    INA_UNUSED(data);

    INA_TEST_ASSERT_SUCCEED(ina_ullc_producer_new(1, sizeof(ina_test_ullc_t), 1024, 1, 1, "/ina_ullc_xx1", INA_ULLC_WS_BUSY_WAIT, &pctx));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ullc_consumer_new(0, sizeof(ina_test_ullc_t), 1024, 1, 1, "/ina_ullc_xx1", &cctx));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ullc_consumer_new(1, 0, 1024, 1, 1, "/ina_ullc_xx1", &cctx));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ullc_consumer_new(1, sizeof(ina_test_ullc_t), 0, 1, 1, "/ina_ullc_xx1", &cctx));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ullc_consumer_new(1, sizeof(ina_test_ullc_t), 1024, 0, 1, "/ina_ullc_xx1", &cctx));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ullc_consumer_new(1, sizeof(ina_test_ullc_t), 1024, (INA_ULLC_MAX_PRODUCERS+1), 1, "/ina_ullc_xx1", &cctx));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ullc_consumer_new(1, sizeof(ina_test_ullc_t), 1024, 1, 0, "/ina_ullc_xx1", &cctx));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ullc_consumer_new(1, sizeof(ina_test_ullc_t), 1024, 1, (INA_ULLC_MAX_CONSUMERS+1), "/ina_ullc_xx1", &cctx));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ullc_consumer_new(1, sizeof(ina_test_ullc_t), 1024, 1, 1, NULL, &cctx));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ullc_consumer_new(1, sizeof(ina_test_ullc_t), 1024, 1, 1, "", &cctx));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ullc_consumer_new(1, sizeof(ina_test_ullc_t), 1024, 1, 1, "/ina_ullc_xx1", NULL));

    INA_TEST_ASSERT_SUCCEED(ina_ullc_consumer_new(1, sizeof(ina_test_ullc_t), 1024, 1, 1, "/ina_ullc_xx1", &cctx));
    INA_TEST_ASSERT_NOT_NULL(cctx);
    ina_ullc_consumer_free(&cctx);
    INA_TEST_ASSERT_NULL(cctx);
    ina_ullc_producer_free(&pctx);
}

INA_TEST(ullc, ina_ullc_consumer_get_pos)
{
    ina_ullc_ctx_t *pctx = NULL;
    ina_ullc_ctx_t *cctx = NULL;
    int64_t pos  = -1;
    INA_UNUSED(data);

    INA_TEST_ASSERT_SUCCEED(ina_ullc_producer_new(1, sizeof(ina_test_ullc_t), 1024, 1, 1, "/ina_ullc_xx1", INA_ULLC_WS_BUSY_WAIT, &pctx));
    INA_TEST_ASSERT_SUCCEED(ina_ullc_consumer_new(1, sizeof(ina_test_ullc_t), 1024, 1, 1, "/ina_ullc_xx1", &cctx));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ullc_consumer_get_pos(NULL, &pos));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ullc_consumer_get_pos(cctx, NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ullc_consumer_get_pos(pctx, &pos));


    INA_TEST_ASSERT_SUCCEED(ina_ullc_producer_commit(pctx));
    INA_TEST_ASSERT_SUCCEED(ina_ullc_producer_commit(pctx));
    INA_TEST_ASSERT_SUCCEED(ina_ullc_consumer_get_pos(cctx, &pos));
    INA_TEST_ASSERT_EQUAL_INT64(0, pos);
    ina_ullc_consumer_get(cctx);
    INA_TEST_ASSERT_SUCCEED(ina_ullc_consumer_get_pos(cctx, &pos));
    INA_TEST_ASSERT_EQUAL_INT64(1, pos);
    ina_ullc_consumer_get(cctx);
    INA_TEST_ASSERT_SUCCEED(ina_ullc_consumer_get_pos(cctx, &pos));
    INA_TEST_ASSERT_EQUAL_INT64(2, pos);
    ina_ullc_consumer_get(cctx);
    INA_TEST_ASSERT_SUCCEED(ina_ullc_consumer_get_pos(cctx, &pos));
    INA_TEST_ASSERT_EQUAL_INT64(2, pos);
    INA_TEST_ASSERT_SUCCEED(ina_ullc_producer_commit(pctx));
    ina_ullc_consumer_get(cctx);
    INA_TEST_ASSERT_SUCCEED(ina_ullc_consumer_get_pos(cctx, &pos));
    INA_TEST_ASSERT_EQUAL_INT64(3, pos);

    ina_ullc_consumer_free(&cctx);
    ina_ullc_producer_free(&pctx);
}

INA_TEST(ullc, ina_ullc_consumer_set_pos)
{
    ina_ullc_ctx_t *pctx = NULL;
    ina_ullc_ctx_t *cctx = NULL;
    int64_t pos = 0;
    INA_UNUSED(data);

    INA_TEST_ASSERT_SUCCEED(ina_ullc_producer_new(1, sizeof(ina_test_ullc_t), 1024, 1, 1, "/ina_ullc_xx1", INA_ULLC_WS_BUSY_WAIT, &pctx));
    INA_TEST_ASSERT_SUCCEED(ina_ullc_consumer_new(1, sizeof(ina_test_ullc_t), 1024, 1, 1, "/ina_ullc_xx1", &cctx));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ullc_consumer_set_pos(NULL, 0));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_ullc_consumer_set_pos(pctx, 0));
    INA_TEST_ASSERT_SUCCEED(ina_ullc_producer_commit(pctx));
    INA_TEST_ASSERT_SUCCEED(ina_ullc_producer_commit(pctx));
    INA_TEST_ASSERT_SUCCEED(ina_ullc_producer_commit(pctx));

    INA_TEST_ASSERT_SUCCEED(ina_ullc_consumer_set_pos(cctx, 0));
    INA_TEST_ASSERT_SUCCEED(ina_ullc_consumer_get_pos(cctx, &pos));
    INA_TEST_ASSERT_EQUAL_INT64(0, pos);

    INA_TEST_ASSERT_SUCCEED(ina_ullc_consumer_set_pos(cctx, 2));
    INA_TEST_ASSERT_SUCCEED(ina_ullc_consumer_get_pos(cctx, &pos));
    INA_TEST_ASSERT_EQUAL_INT64(2, pos);

    pos = 0;
    INA_TEST_ASSERT_SUCCEED(ina_ullc_consumer_set_pos(cctx, 10));
    INA_TEST_ASSERT_SUCCEED(ina_ullc_consumer_get_pos(cctx, &pos));
    INA_TEST_ASSERT_EQUAL_INT64(3, pos);

    ina_ullc_consumer_free(&cctx);
    ina_ullc_producer_free(&pctx);
}

INA_TEST(ullc, slow_consumer)
{
    ina_ullc_ctx_t *ullc;
    ina_test_ullc_t *v = NULL;
    ina_test_hid_t hid;
    int old;
    INA_UNUSED(data);

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
    INA_UNUSED(data);

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
    INA_UNUSED(data);

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

