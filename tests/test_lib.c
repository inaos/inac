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

static int __call_count = 0;

static void __cleanup_handler(int error, int *exitcode)
{
    ++__call_count;
    *exitcode = EXIT_SUCCESS;
}
 
static void __sig_handler(ina_signal_t sig, ina_signal_behavior_t *sb, int *exitcode)
{
    ++__call_count;
    *exitcode = EXIT_SUCCESS;
}

INA_TEST(lib, opt)
{
    int l_int_value = 1;
    ina_str_t l_str_value = NULL;
    int s_int_value = 2;
    ina_str_t s_str_value = NULL;
    float l_float_value = 0.0;
    float s_float_value = 0;
    
    INA_TEST_ASSERT_SUCCEED(ina_opt_isset("run"));
    INA_TEST_ASSERT_SUCCEED(ina_opt_isset("r"));
    INA_TEST_ASSERT_SUCCEED(ina_opt_isset("repeat"));
    INA_TEST_ASSERT_SUCCEED(ina_opt_isset("x")); 
    INA_TEST_ASSERT_SUCCEED(ina_opt_isset("f"));
    INA_TEST_ASSERT_SUCCEED(ina_opt_isset("float"));
    INA_TEST_ASSERT_NOTSUCCEED(ina_opt_isset(""));
    INA_TEST_ASSERT_NOTSUCCEED(ina_opt_get_string("", &l_str_value));
    INA_TEST_ASSERT_SUCCEED(ina_opt_isset("long-option"));
    INA_TEST_ASSERT_SUCCEED(ina_opt_get_string("long-option", &l_str_value));
    INA_TEST_ASSERT_EQUAL_STR("long", ina_str_cstr(l_str_value));
    INA_TEST_ASSERT_SUCCEED(ina_opt_get_string("run", &l_str_value));
    INA_TEST_ASSERT_NOT_NULL(l_str_value);
    INA_TEST_ASSERT_SUCCEED(ina_opt_get_string("run", &s_str_value));
    INA_TEST_ASSERT_NOT_NULL(s_str_value);
    INA_TEST_ASSERT_EQUAL_INTEGER(0, ina_str_cmp(l_str_value, s_str_value));
    INA_TEST_ASSERT_SUCCEED(ina_opt_get_int("repeat", &l_int_value));
    INA_TEST_ASSERT_SUCCEED(ina_opt_get_int("x", &s_int_value));
    INA_TEST_ASSERT_EQUAL_INTEGER(s_int_value, l_int_value);
    INA_TEST_ASSERT_SUCCEED(ina_opt_get_int("t", &s_int_value));
    INA_TEST_ASSERT_EQUAL_INTEGER(121, s_int_value);
    INA_TEST_ASSERT_SUCCEED(ina_opt_get_float("f", &s_float_value));
    INA_TEST_ASSERT_SUCCEED(ina_opt_get_float("float", &l_float_value));
    INA_TEST_ASSERT_EQUAL_FLOATING(l_float_value, s_float_value);
    INA_TEST_ASSERT_EQUAL_FLOATING(l_float_value, (float)1.02);
}

INA_TEST(lib, opt_get_key_value)
{
    ina_str_t key = NULL;
    ina_str_t value = NULL;

    INA_TEST_ASSERT_SUCCEED(ina_opt_get_key_value(1, &key, &value));
    INA_TEST_MSG("ina_opt_get_key_value() at index 0: key=%s, value=%s",
        ina_str_cstr(key),
        ina_str_cstr(value));
    INA_TEST_ASSERT_NOT_NULL(key);
    INA_TEST_ASSERT_NOT_NULL(value);
    INA_TEST_ASSERT_NOTSUCCEED(ina_opt_get_key_value(10, &key, &value));
    INA_TEST_ASSERT_NULL(key);
    INA_TEST_ASSERT_NULL(value);    
}

INA_TEST(lib, appname)
{
    INA_TEST_ASSERT_NOT_NULL(ina_app_get_name());
    INA_TEST_ASSERT_EQUAL_INTEGER(0, strcmp("test", ina_app_get_name()));
}

INA_TEST(lib, apppath)
{
    INA_TEST_ASSERT_NOT_NULL(ina_app_get_path());
    INA_TEST_MSG("ina_app_get_path(): %s", ina_app_get_path());
}

INA_TEST(lib, set_cleanup_handler)
{
    INA_TEST_ASSERT_NULL(ina_set_cleanup_handler(NULL));
    INA_TEST_ASSERT_NULL(ina_set_cleanup_handler(__cleanup_handler));
    INA_TEST_ASSERT_SAME(__cleanup_handler, ina_set_cleanup_handler(__cleanup_handler));
    INA_TEST_ASSERT_SAME(__cleanup_handler, ina_set_cleanup_handler(__cleanup_handler));
    INA_TEST_ASSERT_SAME(__cleanup_handler, ina_set_cleanup_handler(NULL));
}

INA_TEST(lib, set_signal_handler)
{
    INA_TEST_ASSERT_NULL(ina_register_signal_handler(INA_SIGNAL_INT, NULL));    
    INA_TEST_ASSERT_NULL(ina_register_signal_handler(INA_SIGNAL_INT, __sig_handler));
    INA_TEST_ASSERT_SAME(__sig_handler, ina_register_signal_handler(INA_SIGNAL_INT, __sig_handler));
    INA_TEST_ASSERT_SAME(__sig_handler, ina_register_signal_handler(INA_SIGNAL_INT, __sig_handler));
    INA_TEST_ASSERT_SAME(__sig_handler, ina_register_signal_handler(INA_SIGNAL_INT, NULL));
}

INA_TEST(lib, min)
{
    INA_TEST_ASSERT_EQUAL_INTEGER(3, INA_MAX(2,3));
    INA_TEST_ASSERT_EQUAL_INTEGER(3, INA_MAX(3,2));
}

INA_TEST(lib, max)
{
    INA_TEST_ASSERT_EQUAL_INTEGER(2, INA_MIN(2,3));
    INA_TEST_ASSERT_EQUAL_INTEGER(2, INA_MIN(3,2));
}

INA_TEST(lib, high_low_toword) 
{
    uint8_t low = 4;
    uint8_t high = 1;
    uint8_t low2 = 0;
    uint8_t high2 = 0;
    uint16_t word = 0;
    
    word = INA_TOWORD(high, low);
    high2 = INA_HIGH(word);
    INA_TEST_ASSERT_TRUE(high == high2);
    low2 = INA_LOW(word);
    INA_TEST_ASSERT_TRUE(low == low2);
}

INA_TEST(lib, format_specifiers)
{
    char buf[100];
    uint64_t ui64 = 90;
    int64_t i64 = 90;
    sprintf(buf, "ui64=%" INA_UINT64_T_FMT, ui64);
    INA_TEST_ASSERT_EQUAL_STR("ui64=90", buf);
    sprintf(buf, "i64=%" INA_INT64_T_FMT, i64);
    INA_TEST_ASSERT_EQUAL_STR("i64=90", buf);
}
