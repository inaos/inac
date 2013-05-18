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
 
static int __handler(const int sig, const int error)
{
    ++__call_count;
    return EXIT_SUCCESS;
}

INA_TEST(lib, opt)
{
    int l_int_value = 1;
    ina_str_t l_str_value = NULL;
    int s_int_value = 2;
    ina_str_t s_str_value = NULL;
    
    INA_TEST_ASSERT_SUCCEED(ina_opt_isset("run"));
    INA_TEST_ASSERT_SUCCEED(ina_opt_isset("r"));
    INA_TEST_ASSERT_SUCCEED(ina_opt_isset("repeat"));
    INA_TEST_ASSERT_SUCCEED(ina_opt_isset("x")); 
    INA_TEST_ASSERT_SUCCEED(ina_opt_get_string("run", &l_str_value));
    INA_TEST_ASSERT_NOT_NULL(l_str_value);
    INA_TEST_ASSERT_SUCCEED(ina_opt_get_string("run", &s_str_value));
    INA_TEST_ASSERT_NOT_NULL(s_str_value);
    INA_TEST_ASSERT_EQUAL_FLOATING(0, ina_str_cmp(l_str_value, s_str_value));
    INA_TEST_ASSERT_SUCCEED(ina_opt_get_int("repeat", &l_int_value));
    INA_TEST_ASSERT_SUCCEED(ina_opt_get_int("x", &s_int_value));
    INA_TEST_ASSERT_EQUAL_FLOATING(s_int_value, l_int_value);
    INA_TEST_ASSERT_SUCCEED(ina_opt_get_int("t", &s_int_value));
    INA_TEST_ASSERT_EQUAL_FLOATING(121, s_int_value);
}
INA_TEST(lib, appname)
{
    INA_TEST_ASSERT_NOT_NULL(ina_app_get_name());
    INA_TEST_ASSERT_EQUAL_INTEGER(0, strcmp("test", ina_app_get_name()));
}

INA_TEST(lib, apppath)
{
    INA_TEST_ASSERT_NOT_NULL(ina_app_get_path());
}

INA_TEST(lib, set_signal_handler)
{
    INA_TEST_ASSERT_NULL(ina_set_cleanup_handler(NULL));
    INA_TEST_ASSERT_NULL(ina_set_cleanup_handler(__handler));
    INA_TEST_ASSERT_SAME(__handler, ina_set_cleanup_handler(__handler));
    INA_TEST_ASSERT_SAME(__handler, ina_set_cleanup_handler(__handler));
    INA_TEST_ASSERT_SAME(__handler, ina_set_cleanup_handler(NULL));
}