/*
 * Copyright INAOS GmbH, Thalwil, 2013-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
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
    INA_UNUSED(data);
    abort();
}

INA_TEST_SKIP(test, handle_sigfault) {
    INA_UNUSED(data);
    ina_time_tsc_t *t = NULL;
    t->ref = 0;
}
INA_TEST(test_assert, assert_equal_str) {
    INA_UNUSED(data);
    INA_TEST_ASSERT_EQUAL_STR("test", "test");
}

INA_TEST(test_assert, assert_not_equal_str) {
    INA_UNUSED(data);
    INA_TEST_ASSERT_NOT_EQUAL_STR("test", "test-1");
}

INA_TEST(test_assert, assert_data) {
    const char* exp = "test";
    const char* real = "test";
    INA_UNUSED(data);
    INA_TEST_ASSERT_DATA((const unsigned char*)exp, 4, (const unsigned char*)real, 4);
}

INA_TEST(test_assert, assert_equal_floating) {
    const double exp = 3.3;
    const double real = 3.3;
    INA_UNUSED(data);
    INA_TEST_ASSERT_EQUAL_FLOATING(exp, real);
}

INA_TEST(test_assert, assert_not_equal_floating) {
    const double exp = 3.3;
    const double real = 3.2324;
    INA_UNUSED(data);
    INA_TEST_ASSERT_NOT_EQUAL_FLOATING(exp, real);
}

INA_TEST(test_assert, assert_equal_integer) {
    INA_UNUSED(data);
    INA_TEST_ASSERT_EQUAL_INT(1, 1);
}

INA_TEST(test_assert, assert_not_equal_integer) {
    INA_UNUSED(data);
    INA_TEST_ASSERT_NOT_EQUAL_INT(1, 3);
}

INA_TEST(test_assert, assert_not_null) {
    int32_t c = 1;
    INA_UNUSED(data);
    INA_TEST_ASSERT_NOT_NULL(&c);
}

INA_TEST(test_assert, assert_null) {
    int32_t *c = NULL;
    INA_UNUSED(data);
    INA_TEST_ASSERT_NULL(c);
}

INA_TEST(test_assert, assert_same) {
    int32_t i1 = 2;
    int32_t *i2 = &i1;
    INA_UNUSED(data);
    INA_TEST_ASSERT_SAME(&i1, i2);
}

INA_TEST(test_assert, assert_not_same) {
    int32_t i1 = 2;
    int32_t i2 = 1;
    int32_t *i3 = NULL;
    INA_UNUSED(data);
    INA_TEST_ASSERT_NOT_SAME(&i1, i3);
    INA_TEST_ASSERT_NOT_SAME(&i1, &i2);
    i3 = &i2;
    INA_TEST_ASSERT_NOT_SAME(&i1, &i3);
}

INA_TEST(test_assert, assert_true) {
    INA_UNUSED(data);
    INA_TEST_ASSERT_TRUE(1);
    INA_TEST_ASSERT_TRUE(2);
    INA_TEST_ASSERT_TRUE(0==0);
}

INA_TEST(test_assert, assert_false) {
    INA_UNUSED(data);
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

#ifdef INA_OS_WINDOWS
INA_TEST_FIXTURE_WIN32(test_os_fixture, win32) {
    INA_UNUSED(data);
    INA_TEST_ASSERT_TRUE(1);
}
INA_TEST_FIXTURE_OSX(test_os_fixture, osx) {
    INA_UNUSED(data);
    INA_TEST_ASSERT_TRUE(0);
}
INA_TEST_FIXTURE_LINUX(test_os_fixture, linux) {
    INA_UNUSED(data);
    INA_TEST_ASSERT_TRUE(0);
}
INA_TEST_WIN32(test_os, win32) {
    INA_UNUSED(data);
    INA_TEST_ASSERT_TRUE(1);
}
INA_TEST_OSX(test_os, osx) {
    INA_UNUSED(data);
    INA_TEST_ASSERT_TRUE(0);
}
INA_TEST_LINUX(test_os, linux) {
    INA_UNUSED(data);
    INA_TEST_ASSERT_TRUE(0);
}
#endif

#ifdef INA_OS_OSX
INA_TEST_FIXTURE_WIN32(test_os_fixture, win32) {
    INA_UNUSED(data);
    INA_TEST_ASSERT_TRUE(0);
}
INA_TEST_FIXTURE_OSX(test_os_fixture, osx) {
    INA_UNUSED(data);
    INA_TEST_ASSERT_TRUE(1);
}
INA_TEST_FIXTURE_LINUX(test_os_fixture, linux) {
    INA_UNUSED(data);
    INA_TEST_ASSERT_TRUE(0);
}
INA_TEST_WIN32(test_os, win32) {
    INA_UNUSED(data);
    INA_TEST_ASSERT_TRUE(0);
}
INA_TEST_OSX(test_os, osx) {
    INA_UNUSED(data);
    INA_TEST_ASSERT_TRUE(1);
}
INA_TEST_LINUX(test_os, linux) {
    INA_UNUSED(data);
    INA_TEST_ASSERT_TRUE(0);
}
#endif

#ifdef INA_OS_LINUX
INA_TEST_FIXTURE_WIN32(test_os_fixture, win32) {
    INA_UNUSED(data);
    INA_TEST_ASSERT_TRUE(0);
}
INA_TEST_FIXTURE_OSX(test_os_fixture, osx) {
    INA_UNUSED(data);
    INA_TEST_ASSERT_TRUE(0);
}
INA_TEST_FIXTURE_LINUX(test_os_fixture, linux) {
    INA_UNUSED(data);
    INA_TEST_ASSERT_TRUE(1);
}
INA_TEST_WIN32(test_os, win32) {
    INA_UNUSED(data);
    INA_TEST_ASSERT_TRUE(0);
}
INA_TEST_OSX(test_os, osx) {
    INA_UNUSED(data);
    INA_TEST_ASSERT_TRUE(0);
}
INA_TEST_LINUX(test_os, linux) {
    INA_UNUSED(data);
    INA_TEST_ASSERT_TRUE(1);
}
#endif


