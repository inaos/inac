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


INA_TEST_DATA(test) {
    int x;
    int skip;
};

INA_TEST_SETUP(test) {
    data->x += 1;
}

INA_TEST_TEARDOWN(test) {
    data->x -= 1;
}

INA_TEST_FIXTURE(test, fixture_setup) {
    INA_TEST_ASSERT_EQUAL_INT(1, data->x);
}

INA_TEST_FIXTURE(test, fixture_teardown) {
    INA_TEST_ASSERT_EQUAL_INT(1, data->x);
}

INA_TEST_SKIP(test, handle_sigabrt) {
    abort();
}

INA_TEST_SKIP(test, handle_sigfault) {
    ina_time_tsc_t *t = NULL;
    t->ref = 0;
}

INA_TEST(test_asserts, assert_equal_str) {
    INA_TEST_ASSERT_EQUAL_STR("test", "test");
}

INA_TEST(test_assert, assert_not_equal_str) {
    INA_TEST_ASSERT_NOT_EQUAL_STR("test", "test-1");
}

INA_TEST(test_assert, assert_data) {
    const char* exp = "test";
    const char* real = "test";
    INA_TEST_ASSERT_DATA((const unsigned char*)exp, 4, (const unsigned char*)real, 4);
}

INA_TEST(test_assert, assert_equal_floating) {
    const double exp = 3.3;
    const double real = 3.3;
    INA_TEST_ASSERT_EQUAL_FLOATING(exp, real);
}

INA_TEST(test_assert, assert_not_equal_floating) {
    const double exp = 3.3;
    const double real = 3.2324;
    INA_TEST_ASSERT_NOT_EQUAL_FLOATING(exp, real);
}

INA_TEST(test_assert, assert_equal_integer) {
    INA_TEST_ASSERT_EQUAL_INT(1, 1);
}

INA_TEST(test_assert, assert_not_equal_integer) {
    INA_TEST_ASSERT_NOT_EQUAL_INT(1, 3);
}

INA_TEST(test_assert, assert_not_null) {
    int32_t c = 1;
    INA_TEST_ASSERT_NOT_NULL(&c);
}

INA_TEST(test_assert, assert_null) {
    int32_t *c = NULL;
    INA_TEST_ASSERT_NULL(c);
}

INA_TEST(test_assert, assert_same) {
    int32_t i1 = 2;
    int32_t *i2 = &i1;
    INA_TEST_ASSERT_SAME(&i1, i2);
}

INA_TEST(test_assert, assert_not_same) {
    int32_t i1 = 2;
    int32_t i2 = 1;
    int32_t *i3 = NULL;
    INA_TEST_ASSERT_NOT_SAME(&i1, i3);
    INA_TEST_ASSERT_NOT_SAME(&i1, &i2);
    i3 = &i2;
    INA_TEST_ASSERT_NOT_SAME(&i1, &i3);
}

INA_TEST(test_assert, assert_true) {
    INA_TEST_ASSERT_TRUE(1);
    INA_TEST_ASSERT_TRUE(2);
    INA_TEST_ASSERT_TRUE(0==0);
}

INA_TEST(test_assert, assert_false) {
    INA_TEST_ASSERT_FALSE(0);
    INA_TEST_ASSERT_FALSE(2!=2);
}

INA_TEST_DATA(test_os_fixture) {
    int x;
    int skip;
};

INA_TEST_SETUP(test_os_fixture) {
    data->x += 1;
}

INA_TEST_TEARDOWN(test_os_fixture) {
    data->x -= 1;
}

#ifdef INA_OS_WIN32
INA_TEST_FIXTURE_WIN32(test_os_fixture, win32) {
    INA_TEST_ASSERT_TRUE(1);
}
INA_TEST_FIXTURE_OSX(test_os_fixture, osx) {
    INA_TEST_ASSERT_TRUE(0);
}
INA_TEST_FIXTURE_LINUX(test_os_fixture, linux) {
    INA_TEST_ASSERT_TRUE(0);
}
INA_TEST_WIN32(test_os, win32) {
    INA_TEST_ASSERT_TRUE(1);
}
INA_TEST_OSX(test_os, osx) {
    INA_TEST_ASSERT_TRUE(0);
}
INA_TEST_LINUX(test_os, linux) {
    INA_TEST_ASSERT_TRUE(0);
}
#endif

#ifdef INA_OS_OSX
INA_TEST_FIXTURE_WIN32(test_os_fixture, win32) {
    INA_TEST_ASSERT_TRUE(0);
}
INA_TEST_FIXTURE_OSX(test_os_fixture, osx) {
    INA_TEST_ASSERT_TRUE(1);
}
INA_TEST_FIXTURE_LINUX(test_os_fixture, linux) {
    INA_TEST_ASSERT_TRUE(0);
}
INA_TEST_WIN32(test_os, win32) {
    INA_TEST_ASSERT_TRUE(0);
}
INA_TEST_OSX(test_os, osx) {
    INA_TEST_ASSERT_TRUE(1);
}
INA_TEST_LINUX(test_os, linux) {
    INA_TEST_ASSERT_TRUE(0);
}
#endif

#ifdef INA_OS_LINUX
INA_TEST_FIXTURE_WIN32(test_os_fixture, win32) {
    INA_TEST_ASSERT_TRUE(0);
}
INA_TEST_FIXTURE_OSX(test_os_fixture, osx) {
    INA_TEST_ASSERT_TRUE(0);
}
INA_TEST_FIXTURE_LINUX(test_os_fixture, linux) {
    INA_TEST_ASSERT_TRUE(1);
}
INA_TEST_WIN32(test_os, win32) {
    INA_TEST_ASSERT_TRUE(0);
}
INA_TEST_OSX(test_os, osx) {
    INA_TEST_ASSERT_TRUE(0);
}
INA_TEST_LINUX(test_os, linux) {
    INA_TEST_ASSERT_TRUE(1);
}
#endif


