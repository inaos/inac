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
"\"anObject\":{"
    "\"numericProperty\":-122,"
    "\"stringProperty\":\"An offensive is problematic\","
    "\"nullProperty\":null,"
    "\"booleanProperty\":true,"
    "\"dateProperty\":\"2011-09-23\","
    "\"doubleProperty\":2.3,"
    "\"doublePropertyWithoutFraction\":2.0"
        
"},"
"\"arrayOfObjects\":["
    "{"
        "\"item\":1"
    "},"
    "{"
         "\"item\":2"
     "},"
     "{"
         "\"item\":3"
      "}"
   "],"
   "\"arrayOfIntegers\":["
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
    INA_TEST_ASSERT_NOT_NULL(ctx);
    INA_TEST_ASSERT_SUCCEED(ina_json_parser_borrow(ctx,  &p));
        
    INA_TEST_ASSERT_SUCCEED(ina_json_parser_execute(p, 
                                (unsigned char *)__object, 
                                strlen(__object),
                                INA_YES));

    INA_TEST_ASSERT_SUCCEED(ina_json_parser_release(ctx, &p));
    INA_TEST_ASSERT_SUCCEED(ina_json_destroy(&ctx));                                
}

INA_TEST(json, parser_reset)
{
    ina_json_ctx_t    *ctx = NULL;
    ina_json_parser_t *p =  NULL;
    INA_TEST_ASSERT_SUCCEED(ina_json_init(&ctx, 1, 1));
    INA_TEST_ASSERT_SUCCEED(ina_json_parser_borrow(ctx,  &p));

    INA_TEST_ASSERT_SUCCEED(ina_json_parser_reset(p));
    INA_TEST_ASSERT_SUCCEED(ina_json_parser_execute(p, 
                                (unsigned char *)__object, 
                                strlen(__object),
                                INA_YES));
    INA_TEST_ASSERT_NOTSUCCEED(ina_json_parser_execute(p, 
                                (unsigned char *)__object, 
                                strlen(__object),
                                INA_YES));
    INA_TEST_ASSERT_SUCCEED(ina_json_parser_reset(p));
    INA_TEST_ASSERT_SUCCEED(ina_json_parser_execute(p, 
                                (unsigned char *)__object, 
                                strlen(__object),
                                INA_YES));
    INA_TEST_ASSERT_SUCCEED(ina_json_parser_release(ctx, &p));
    INA_TEST_ASSERT_SUCCEED(ina_json_destroy(&ctx));                                
}

INA_TEST(json, parser_try_data)
{
    ina_json_ctx_t    *ctx = NULL;
    ina_json_parser_t *p =  NULL;
    const ina_json_data_t   *data = NULL;

    INA_TEST_ASSERT_SUCCEED(ina_json_init(&ctx, 1, 0));
    INA_TEST_ASSERT_SUCCEED(ina_json_parser_borrow(ctx,  &p));
    INA_TEST_ASSERT_NOT_NULL(p);
    INA_TEST_ASSERT_SUCCEED(ina_json_parser_execute(p, 
                                (unsigned char *)__object, 
                                strlen(__object),
                                INA_YES));

    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);
    INA_TEST_ASSERT_TRUE(data->event == INA_JSON_PARSE_EVENT_START_OBJECT);

    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);
    INA_TEST_ASSERT_TRUE(data->event == INA_JSON_PARSE_EVENT_OBJECT_KEY);
    INA_TEST_ASSERT_EQUAL_INTEGER(8, data->size);
    INA_TEST_ASSERT_TRUE(strncmp((const char*)data->value.s, "anObject", 
                                 data->size) == 0);
    
    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);
    INA_TEST_ASSERT_TRUE(data->event == INA_JSON_PARSE_EVENT_START_OBJECT);
    
    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);
    INA_TEST_ASSERT_TRUE(data->event == INA_JSON_PARSE_EVENT_OBJECT_KEY);
    INA_TEST_ASSERT_EQUAL_INTEGER(15, data->size);
    INA_TEST_ASSERT_TRUE(strncmp("numericProperty", (const char*)data->value.s, 
                                 data->size) == 0);

    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);    
    INA_TEST_ASSERT_TRUE(data->event == INA_JSON_PARSE_EVENT_DATA_INT);
    INA_TEST_ASSERT_EQUAL_INTEGER(sizeof(int64_t), data->size);
    INA_TEST_ASSERT_EQUAL_INTEGER(-122, data->value.i);

    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);
    INA_TEST_ASSERT_TRUE(data->event == INA_JSON_PARSE_EVENT_OBJECT_KEY);
    INA_TEST_ASSERT_EQUAL_INTEGER(strlen("stringProperty"), data->size);
    INA_TEST_ASSERT_TRUE(strncmp("stringProperty", (const char*)data->value.s, 
                                 data->size) == 0);

    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);
    INA_TEST_ASSERT_TRUE(data->event == INA_JSON_PARSE_EVENT_DATA_STRING);
    INA_TEST_ASSERT_EQUAL_INTEGER(strlen("An offensive is problematic"), 
                                  data->size);
    INA_TEST_ASSERT_TRUE(strncmp("An offensive is problematic", 
                                 (const char*)data->value.s, data->size) == 0);

    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);
    INA_TEST_ASSERT_TRUE(data->event == INA_JSON_PARSE_EVENT_OBJECT_KEY);
    INA_TEST_ASSERT_EQUAL_INTEGER(strlen("nullProperty"), data->size);
    INA_TEST_ASSERT_TRUE(strncmp("nullProperty", (const char*)data->value.s, 
                                 data->size) == 0);

    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);
    INA_TEST_ASSERT_TRUE(data->event == INA_JSON_PARSE_EVENT_DATA_NULL);
    INA_TEST_ASSERT_EQUAL_INTEGER(0, data->size);

    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);
    INA_TEST_ASSERT_TRUE(data->event == INA_JSON_PARSE_EVENT_OBJECT_KEY);
    INA_TEST_ASSERT_EQUAL_INTEGER(strlen("booleanProperty"), data->size);
    INA_TEST_ASSERT_TRUE(strncmp("booleanProperty", (const char*)data->value.s, 
                                 data->size) == 0);

    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);
    INA_TEST_ASSERT_TRUE(data->event == INA_JSON_PARSE_EVENT_DATA_BOOL);
    INA_TEST_ASSERT_EQUAL_INTEGER(sizeof(int32_t), data->size);
    INA_TEST_ASSERT_EQUAL_FLOATING(INA_YES, data->value.b);


    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);
    INA_TEST_ASSERT_TRUE(data->event == INA_JSON_PARSE_EVENT_OBJECT_KEY);
    INA_TEST_ASSERT_EQUAL_INTEGER(strlen("dateProperty"), data->size);
    INA_TEST_ASSERT_TRUE(strncmp("dateProperty", (const char*)data->value.s, 
                                 data->size) == 0);

    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);
    INA_TEST_ASSERT_TRUE(data->event == INA_JSON_PARSE_EVENT_DATA_STRING);
    INA_TEST_ASSERT_EQUAL_INTEGER(strlen("2011-09-23"), data->size);
    INA_TEST_ASSERT_TRUE(strncmp("2011-09-23", (const char*)data->value.s, 
                                 data->size) == 0);


    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);
    INA_TEST_ASSERT_TRUE(data->event == INA_JSON_PARSE_EVENT_OBJECT_KEY);
    INA_TEST_ASSERT_EQUAL_INTEGER(strlen("doubleProperty"), data->size);
    INA_TEST_ASSERT_TRUE(strncmp("doubleProperty", (const char*)data->value.s, 
                                 data->size) == 0);

    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);
    INA_TEST_ASSERT_TRUE(data->event == INA_JSON_PARSE_EVENT_DATA_DOUBLE);
    INA_TEST_ASSERT_EQUAL_INTEGER(sizeof(double), data->size);
    INA_TEST_ASSERT_EQUAL_FLOATING(2.3, data->value.d);

    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);
    INA_TEST_ASSERT_TRUE(data->event == INA_JSON_PARSE_EVENT_OBJECT_KEY);
    INA_TEST_ASSERT_EQUAL_INTEGER(strlen("doublePropertyWithoutFraction"), 
                                  data->size);
    INA_TEST_ASSERT_TRUE(strncmp("doublePropertyWithoutFraction", 
                                 (const char*)data->value.s, 
                                 data->size) == 0);

    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);
    INA_TEST_ASSERT_TRUE(data->event == INA_JSON_PARSE_EVENT_DATA_DOUBLE);
    INA_TEST_ASSERT_EQUAL_INTEGER(sizeof(double), data->size);
    INA_TEST_ASSERT_EQUAL_FLOATING(2, data->value.d);

    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);
    INA_TEST_ASSERT_TRUE(data->event == INA_JSON_PARSE_EVENT_END_OBJECT);

    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);
    INA_TEST_ASSERT_TRUE(data->event == INA_JSON_PARSE_EVENT_OBJECT_KEY);
    INA_TEST_ASSERT_EQUAL_INTEGER(strlen("arrayOfObjects"), data->size);
    INA_TEST_ASSERT_TRUE(strncmp("arrayOfObjects", (const char*)data->value.s, 
                                 data->size) == 0);

    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);
    INA_TEST_ASSERT_TRUE(data->event == INA_JSON_PARSE_EVENT_START_ARRAY);

    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);
    INA_TEST_ASSERT_TRUE(data->event == INA_JSON_PARSE_EVENT_START_OBJECT);
    INA_TEST_ASSERT_EQUAL_INTEGER(0, data->size);

    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);
    INA_TEST_ASSERT_TRUE(data->event == INA_JSON_PARSE_EVENT_OBJECT_KEY);
    INA_TEST_ASSERT_EQUAL_INTEGER(strlen("item"), data->size);
    INA_TEST_ASSERT_TRUE(strncmp("item", (const char*) data->value.s, 
                                 data->size) == 0);

    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);    
    INA_TEST_ASSERT_TRUE(data->event == INA_JSON_PARSE_EVENT_DATA_INT);
    INA_TEST_ASSERT_EQUAL_INTEGER(sizeof(int64_t), data->size);
    INA_TEST_ASSERT_EQUAL_INTEGER(1, data->value.i);

    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);
    INA_TEST_ASSERT_TRUE(data->event == INA_JSON_PARSE_EVENT_END_OBJECT);
    INA_TEST_ASSERT_EQUAL_INTEGER(0, data->size);


    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);
    INA_TEST_ASSERT_TRUE(data->event == INA_JSON_PARSE_EVENT_START_OBJECT);
    INA_TEST_ASSERT_EQUAL_INTEGER(0, data->size);

    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);
    INA_TEST_ASSERT_TRUE(data->event == INA_JSON_PARSE_EVENT_OBJECT_KEY);
    INA_TEST_ASSERT_EQUAL_INTEGER(strlen("item"), data->size);
    INA_TEST_ASSERT_TRUE(strncmp("item", (const char*) data->value.s, 
                         data->size) == 0);

    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);    
    INA_TEST_ASSERT_TRUE(data->event == INA_JSON_PARSE_EVENT_DATA_INT);
    INA_TEST_ASSERT_EQUAL_INTEGER(sizeof(int64_t), data->size);
    INA_TEST_ASSERT_EQUAL_INTEGER(2, data->value.i);

    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);
    INA_TEST_ASSERT_TRUE(data->event == INA_JSON_PARSE_EVENT_END_OBJECT);
    INA_TEST_ASSERT_EQUAL_INTEGER(0, data->size);


    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);
    INA_TEST_ASSERT_TRUE(data->event == INA_JSON_PARSE_EVENT_START_OBJECT);
    INA_TEST_ASSERT_EQUAL_INTEGER(0, data->size);

    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);
    INA_TEST_ASSERT_TRUE(data->event == INA_JSON_PARSE_EVENT_OBJECT_KEY);
    INA_TEST_ASSERT_EQUAL_INTEGER(strlen("item"), data->size);
    INA_TEST_ASSERT_TRUE(strncmp("item", (const char*) data->value.s, 
                         data->size) == 0);

    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);    
    INA_TEST_ASSERT_TRUE(data->event == INA_JSON_PARSE_EVENT_DATA_INT);
    INA_TEST_ASSERT_EQUAL_INTEGER(sizeof(int64_t), data->size);
    INA_TEST_ASSERT_EQUAL_INTEGER(3, data->value.i);

    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);
    INA_TEST_ASSERT_TRUE(data->event == INA_JSON_PARSE_EVENT_END_OBJECT);
    INA_TEST_ASSERT_EQUAL_INTEGER(0, data->size);

    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);
    INA_TEST_ASSERT_TRUE(data->event == INA_JSON_PARSE_EVENT_END_ARRAY);


    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);
    INA_TEST_ASSERT_TRUE(data->event == INA_JSON_PARSE_EVENT_OBJECT_KEY);
    INA_TEST_ASSERT_EQUAL_INTEGER(strlen("arrayOfIntegers"), data->size);
    INA_TEST_ASSERT_TRUE(strncmp("arrayOfIntegers", (const char*)data->value.s, 
                                 data->size) == 0);

    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);
    INA_TEST_ASSERT_TRUE(data->event == INA_JSON_PARSE_EVENT_START_ARRAY);

    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);    
    INA_TEST_ASSERT_TRUE(data->event == INA_JSON_PARSE_EVENT_DATA_INT);
    INA_TEST_ASSERT_EQUAL_INTEGER(sizeof(int64_t), data->size);
    INA_TEST_ASSERT_EQUAL_INTEGER(1, data->value.i);

    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);    
    INA_TEST_ASSERT_TRUE(data->event == INA_JSON_PARSE_EVENT_DATA_INT);
    INA_TEST_ASSERT_EQUAL_INTEGER(sizeof(int64_t), data->size);
    INA_TEST_ASSERT_EQUAL_INTEGER(2, data->value.i);

    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);    
    INA_TEST_ASSERT_TRUE(data->event == INA_JSON_PARSE_EVENT_DATA_INT);
    INA_TEST_ASSERT_EQUAL_INTEGER(sizeof(int64_t), data->size);
    INA_TEST_ASSERT_EQUAL_INTEGER(3, data->value.i);

    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);    
    INA_TEST_ASSERT_TRUE(data->event == INA_JSON_PARSE_EVENT_DATA_INT);
    INA_TEST_ASSERT_EQUAL_INTEGER(sizeof(int64_t), data->size);
    INA_TEST_ASSERT_EQUAL_INTEGER(4, data->value.i);

    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);    
    INA_TEST_ASSERT_TRUE(data->event == INA_JSON_PARSE_EVENT_DATA_INT);
    INA_TEST_ASSERT_EQUAL_INTEGER(sizeof(int64_t), data->size);
    INA_TEST_ASSERT_EQUAL_INTEGER(5, data->value.i);

    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);
    INA_TEST_ASSERT_TRUE(data->event == INA_JSON_PARSE_EVENT_END_ARRAY);

    INA_TEST_ASSERT_SUCCEED(ina_json_parser_try_data(p, &data));
    INA_TEST_ASSERT_NOT_NULL(data);
    INA_TEST_ASSERT_TRUE(data->event == INA_JSON_PARSE_EVENT_END_OBJECT);
}

INA_TEST(json, generator_borrow_release)
{     
    ina_json_ctx_t       *ctx = NULL;
    ina_json_gen_t *g1  = NULL;
    ina_json_gen_t *g2  = NULL;
    ina_json_gen_t *g3  = NULL;
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

INA_TEST(json, generator_get_buffer)
{
    ina_json_ctx_t      *ctx = NULL;
    ina_json_gen_t      *g = NULL;
    const unsigned char *buffer;
    size_t buf_len = 0;

    INA_TEST_ASSERT_SUCCEED(ina_json_init(&ctx, 0, 1));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_borrow(ctx,  &g));
    INA_TEST_ASSERT_NOT_NULL(g);
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_get_buffer(g, &buffer, &buf_len));
    INA_TEST_ASSERT_NULL(buffer);    
    INA_TEST_ASSERT_TRUE(buf_len == 0);
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_start_object(g));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_end_object(g));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_get_buffer(g, &buffer, &buf_len));
    INA_TEST_ASSERT_NOT_NULL(buffer);
    INA_TEST_ASSERT_TRUE(buf_len > 0);
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_release(ctx, &g));
    INA_TEST_ASSERT_SUCCEED(ina_json_destroy(&ctx));
}

INA_TEST(json, generator_get_reset)
{
    ina_json_ctx_t      *ctx = NULL;
    ina_json_gen_t      *g = NULL;
    const unsigned char *buffer;
    size_t buf_len = 0;

    INA_TEST_ASSERT_SUCCEED(ina_json_init(&ctx, 0, 1));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_borrow(ctx,  &g));
    INA_TEST_ASSERT_NOT_NULL(g);
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_start_object(g));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_end_object(g));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_get_buffer(g, &buffer, &buf_len));
    INA_TEST_ASSERT_NOT_NULL(buffer);
    INA_TEST_ASSERT_TRUE(buf_len > 0);
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_reset(g));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_get_buffer(g, &buffer, &buf_len));
    INA_TEST_ASSERT_NULL(buffer);
    INA_TEST_ASSERT_EQUAL_INTEGER(0, buf_len);
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_release(ctx, &g));
    INA_TEST_ASSERT_SUCCEED(ina_json_destroy(&ctx));
    
}

INA_TEST(json, generator)
{
    ina_json_ctx_t      *ctx = NULL;
    ina_json_gen_t      *g = NULL;
    ina_json_parser_t   *p = NULL;
    const unsigned char *buffer;
    size_t               buf_len;
    ina_str_t            json_str;

    INA_TEST_ASSERT_SUCCEED(ina_json_init(&ctx, 1, 1));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_borrow(ctx,  &g));
    INA_TEST_ASSERT_NOT_NULL(g);
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_start_object(g));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_add_string(g, 
                                                         "anObject", 
                                                         strlen("anObject")));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_start_object(g));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_add_string(g, 
                                                "numericProperty", 
                                                strlen("numericProperty")));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_add_integer(g, -122));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_add_string(g, 
                                                    "stringProperty", 
                                                    strlen("stringProperty")));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_add_string(g, 
                                        "An offensive is problematic", 
                                        strlen("An offensive is problematic")));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_add_string(g,
                                                    "nullProperty",  
                                                    strlen("nullProperty")));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_add_null(g));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_add_string(g, 
                                                "booleanProperty", 
                                                strlen("booleanProperty")));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_add_boolean(g, INA_YES));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_add_string(g, 
                                                    "dateProperty", 
                                                    strlen("dateProperty")));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_add_string(g, 
                                                    "2011-09-23", 
                                                    strlen("2011-09-23")));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_add_string(g, 
                                                "doubleProperty", 
                                                strlen("doubleProperty")));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_add_double(g, 2.3));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_add_string(g, 
                                    "doublePropertyWithoutFraction", 
                                    strlen("doublePropertyWithoutFraction")));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_add_double(g, 2));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_end_object(g));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_add_string(g, 
                                                "arrayOfObjects", 
                                                strlen("arrayOfObjects")));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_start_array(g));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_start_object(g));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_add_string(g, 
                                                          "item", 
                                                          strlen("item")));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_add_integer(g, 1));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_end_object(g));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_start_object(g));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_add_string(g, 
                                                          "item", 
                                                          strlen("item")));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_add_integer(g, 2));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_end_object(g));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_start_object(g));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_add_string(g, 
                                                          "item", 
                                                          strlen("item")));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_add_integer(g, 3));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_end_object(g));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_end_array(g));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_add_string(g,
                                                "arrayOfIntegers", 
                                                strlen("arrayOfIntegers")));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_start_array(g));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_add_integer(g, 1));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_add_integer(g, 2));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_add_integer(g, 3));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_add_integer(g, 4));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_add_integer(g, 5));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_end_array(g));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_end_object(g));

    INA_TEST_ASSERT_SUCCEED(ina_json_generator_get_buffer(g, &buffer, &buf_len));
    json_str = ina_str_new_fromblk((const char*)buffer, buf_len);

    INA_TEST_ASSERT_SUCCEED(ina_json_parser_borrow(ctx, &p));
    INA_TEST_ASSERT_SUCCEED(ina_json_parser_execute(p, 
                                (const unsigned char*)ina_str_cstr(json_str), 
                                buf_len, 
                                INA_YES));
    INA_TEST_ASSERT_EQUAL_STR(ina_str_cstr(json_str), __object);
    INA_TEST_ASSERT_SUCCEED(ina_json_parser_release(ctx, &p));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_release(ctx, &g));
    INA_TEST_ASSERT_SUCCEED(ina_json_destroy(&ctx));
}

INA_TEST(json, generator_large)
{

    ina_json_ctx_t      *ctx = NULL;
    ina_json_gen_t      *g = NULL;
    const unsigned char *buffer;
    size_t               buf_len;
    ina_str_t            json_str;
    size_t               i;

    INA_TEST_ASSERT_SUCCEED(ina_json_init(&ctx, 1, 1));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_borrow(ctx,  &g));
    INA_TEST_ASSERT_NOT_NULL(g);
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_start_object(g));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_add_string(g,
                                                "arrayOfStrings", 
                                                strlen("arrayOfStrings")));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_start_array(g));
    for (i=0;i<500000;++i) {
        INA_TEST_ASSERT_SUCCEED(ina_json_generator_add_string(g, 
                                                          "item", 
                                                          strlen("item")));
    }
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_end_array(g));
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_end_object(g));

    INA_TEST_ASSERT_SUCCEED(ina_json_generator_get_buffer(g, &buffer, &buf_len));
    json_str = ina_str_new_fromblk((const char*)buffer, buf_len);
    INA_TEST_MSG("lenght of json_str: %d", ina_str_len(json_str));
  
    INA_TEST_ASSERT_SUCCEED(ina_json_generator_release(ctx, &g));
    INA_TEST_ASSERT_SUCCEED(ina_json_destroy(&ctx));

    ina_str_free(json_str);
}