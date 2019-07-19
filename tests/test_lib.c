/*
 * Copyright INAOS GmbH, Thalwil, 2012-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>

static int __call_count = 0;

static void __cleanup_handler(int error, int *exitcode)
{
    INA_UNUSED(error);
    ++__call_count;
    *exitcode = EXIT_SUCCESS;
}
 
static void __sig_handler(ina_signal_t sig, ina_signal_behavior_t *sb, int *exitcode)
{
    INA_UNUSED(sig);
    INA_UNUSED(sb);
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
    INA_TEST_ASSERT_FAILED(ina_opt_isset(""));
    INA_TEST_ASSERT_FAILED(ina_opt_get_string("", &l_str_value));
    INA_TEST_ASSERT_SUCCEED(ina_opt_isset("long-option"));
    INA_TEST_ASSERT_SUCCEED(ina_opt_get_string("long-option", &l_str_value));
    INA_TEST_ASSERT_EQUAL_STR("long", ina_str_cstr(l_str_value));
    INA_TEST_ASSERT_SUCCEED(ina_opt_get_string("run", &l_str_value));
    INA_TEST_ASSERT_NOT_NULL(l_str_value);
    INA_TEST_ASSERT_SUCCEED(ina_opt_get_string("run", &s_str_value));
    INA_TEST_ASSERT_NOT_NULL(s_str_value);
    INA_TEST_ASSERT_EQUAL_INT(0, ina_str_cmp(l_str_value, s_str_value));
    INA_TEST_ASSERT_SUCCEED(ina_opt_get_int("repeat", &l_int_value));
    INA_TEST_ASSERT_SUCCEED(ina_opt_get_int("x", &s_int_value));
    INA_TEST_ASSERT_EQUAL_INT(s_int_value, l_int_value);
    INA_TEST_ASSERT_SUCCEED(ina_opt_get_int("t", &s_int_value));
    INA_TEST_ASSERT_EQUAL_INT(121, s_int_value);
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
    INA_TEST_ASSERT_FAILED(ina_opt_get_key_value(10, &key, &value));
    INA_TEST_ASSERT_NULL(key);
    INA_TEST_ASSERT_NULL(value);    
}

INA_TEST(lib, appname)
{
    INA_TEST_ASSERT_NOT_NULL(ina_app_get_name());
    INA_TEST_MSG("ina_app_get_name() = %s", ina_app_get_name());
#ifdef INA_OS_WINDOWS
    INA_TEST_ASSERT_EQUAL_INT(0, strcmp("tests.exe", ina_app_get_name()));
#else
    INA_TEST_ASSERT_EQUAL_INT(0, strcmp("tests", ina_app_get_name()));
#endif
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
    INA_TEST_ASSERT_EQUAL_INT(3, INA_MAX(2,3));
    INA_TEST_ASSERT_EQUAL_INT(3, INA_MAX(3,2));
}

INA_TEST(lib, max)
{
    INA_TEST_ASSERT_EQUAL_INT(2, INA_MIN(2,3));
    INA_TEST_ASSERT_EQUAL_INT(2, INA_MIN(3,2));
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
    snprintf(buf, 99, "ui64=%" INA_UINT64_T_FMT, ui64);
    INA_TEST_ASSERT_EQUAL_STR("ui64=90", buf);
    snprintf(buf, 99, "i64=%" INA_INT64_T_FMT, i64);
    /* FIXME */
    INA_TEST_ASSERT_EQUAL_STR("i64=5a", buf);
}

INA_TEST(lib, invalid_arguments)
{
    ina_str_t key = NULL;
    ina_str_t value = NULL;
    float fvalue;
    int ivalue;

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_opt_isset(NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_opt_get_key_value(-1, &key, &value));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_opt_get_key_value(0, NULL, &value));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_opt_get_key_value(0, &key, NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_opt_get_string(NULL, &value));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_opt_get_string("name", NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_opt_get_float(NULL, &fvalue));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_opt_get_float("name", NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_opt_get_int(NULL, &ivalue));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_opt_get_int("name", NULL));




}