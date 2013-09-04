/*
 * Copyright (c) 2013, INAOS GmbH
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

const static char * __object = "{"
"\"anObject\": {"
    "\"numericProperty\": -122,"
    "\"stringProperty\": An offensive is problematic\","
    "\"nullProperty\": null,"
    "\"booleanProperty\": true,"
    "\"dateProperty\": \"2011-09-23\","
    "\"doubleProperty\": 2.3"
"},"
"\"arrayOfObjects\": ["
    "{"
        "\"item\": 1"
    "},"
    "{"
         "\"item\": 2"
     "},"
     "{"
         "\"item\": 3"
      "}"
   "],"
   "\"arrayOfIntegers\": ["
      "1,"
      "2,"
      "3,"
      "4,"
      "5"
   "]"
"}";

    
INA_TEST(json, init_destroy)
{
    ina_json_ctx_t *ctx = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_json_init(&ctx, 10, 10));
    INA_TEST_ASSERT_NOT_NULL(ctx);
    INA_TEST_ASSERT_SUCCEED(ina_json_destroy(&ctx));
    INA_TEST_ASSERT_NULL(ctx);
}

INA_TEST(json, parser_borrow_release)
{
    ina_json_ctx_t    *ctx = NULL;
    ina_json_parser_t *p1 =  NULL;
    ina_json_parser_t *p2 = NULL;
    ina_json_parser_t *p3 = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_json_init(&ctx, 2, 0));
    INA_TEST_ASSERT_SUCCEED(ina_json_parser_borrow(ctx,  &p1));
    INA_TEST_ASSERT_NOT_NULL(p1);
    INA_TEST_ASSERT_SUCCEED(ina_json_parser_borrow(ctx,  &p2));
    INA_TEST_ASSERT_NOT_NULL(p2);
    INA_TEST_ASSERT_NOT_SAME(p1, p2);
    INA_TEST_ASSERT_NOTSUCCEED(ina_json_parser_borrow(ctx, &p3));
    INA_TEST_ASSERT_SUCCEED(ina_json_parser_release(ctx, &p2));
    INA_TEST_ASSERT_SUCCEED(ina_json_parser_borrow(ctx, &p3));
    INA_TEST_ASSERT_NOT_NULL(p3);
    INA_TEST_ASSERT_SUCCEED(ina_json_parser_release(ctx, &p1));
    INA_TEST_ASSERT_SUCCEED(ina_json_parser_release(ctx, &p3));
    INA_TEST_ASSERT_SUCCEED(ina_json_destroy(&ctx));
    ina_err_reset();
}

INA_TEST(json, parser_execute)
{
    ina_json_ctx_t    *ctx = NULL;
    ina_json_parser_t *p =  NULL;
    INA_TEST_ASSERT_SUCCEED(ina_json_init(&ctx, 1, 1));
    INA_TEST_ASSERT_SUCCEED(ina_json_parser_borrow(ctx,  &p));
        
    INA_TEST_ASSERT_SUCCEED(ina_json_parser_execute(p, 
                                (unsigned char *)__object, 
                                strlen(__object)));

    INA_TEST_ASSERT_SUCCEED(ina_json_parser_release(ctx, &p));
    INA_TEST_ASSERT_SUCCEED(ina_json_destroy(&ctx));                                
}

INA_TEST(json, parser_try_data)
{
    ina_json_ctx_t    *ctx = NULL;
    ina_json_parser_t *p =  NULL;
    ina_json_data_t   *data = NULL;

    INA_TEST_ASSERT_SUCCEED(ina_json_init(&ctx, 1, 0));
    INA_TEST_ASSERT_SUCCEED(ina_json_parser_borrow(ctx,  &p));
        
    INA_TEST_ASSERT_SUCCEED(ina_json_parser_execute(p, 
                                (unsigned char *)__object, 
                                strlen(__object)));

    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);
    INA_TEST_ASSERT_TRUE(data->event == INA_JSON_PARSE_EVENT_START_OBJECT);
    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);
    INA_TEST_ASSERT_TRUE(data->event = INA_JSON_PARSE_EVENT_OBJECT_KEY);
    INA_TEST_ASSERT_EQUAL_STR("anObject", (const char*)data->value.s);
    INA_TEST_ASSERT_TRUE(data->event = INA_JSON_PARSE_EVENT_OBJECT_KEY);
    INA_TEST_ASSERT_EQUAL_STR("numericProperty", (const char*)data->value.s);
}


INA_TEST(json, generator_borrow_release)
{     
    ina_json_ctx_t       *ctx = NULL;
    ina_json_generator_t *g1  = NULL;
    ina_json_generator_t *g2  = NULL;
    ina_json_generator_t *g3  = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_json_init(&ctx, 0, 2));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_borrow(ctx,  &g1));
    INA_TEST_ASSERT_NOT_NULL(g1);
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_borrow(ctx,  &g2));
    INA_TEST_ASSERT_NOT_NULL(g2);
    INA_TEST_ASSERT_NOT_SAME(g1, g2);
    INA_TEST_ASSERT_NOTSUCCEED(ina_json_generator_borrow(ctx, &g3));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_release(ctx, &g2));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_borrow(ctx, &g3));
    INA_TEST_ASSERT_NOT_NULL(g3);
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_release(ctx, &g1));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_release(ctx, &g3));
    INA_TEST_ASSERT_SUCCEED(ina_json_destroy(&ctx));
    ina_err_reset();
}

INA_TEST_SKIP(json, generator)
{
}

