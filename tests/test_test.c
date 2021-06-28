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

INA_TEST_FIXTURE(test, errmsg_too_big) {
    char buf[17*1024];
    ina_mem_set(&buf[0], 'A', 17*1024);
    buf[17*1024-1] = 0;
    INA_TEST_ASSERT_SUCCEED(INA_TEST_MSG("%s", buf));
    INA_TEST_ASSERT_SUCCEED(INA_TEST_MSG("%s", "OK"));
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


INA_TEST_WINDOWS(test_os, open_file) {
    INA_UNUSED(data);
#ifdef INA_OS_WINDOWS
    INA_TEST_ASSERT_TRUE(1);
#else 
    INA_TEST_ASSERT_TRUE(0);
#endif
}
INA_TEST_OSX(test_os, open_file) {
    INA_UNUSED(data);
#ifdef INA_OS_OSX
    INA_TEST_ASSERT_TRUE(1);
#else 
    INA_TEST_ASSERT_TRUE(0);
#endif
}
INA_TEST_LINUX(test_os, open_file) {
    INA_UNUSED(data);
#ifdef INA_OS_LINUX
    INA_TEST_ASSERT_TRUE(1);
#else 
    INA_TEST_ASSERT_TRUE(0);
#endif
}

INA_TEST_SKIP_WINDOWS(test_os, open_file_skip) {
    INA_UNUSED(data);
    INA_TEST_ASSERT_TRUE(0);
}
INA_TEST_SKIP_OSX(test_os, open_file_skip) {
    INA_UNUSED(data);
    INA_TEST_ASSERT_TRUE(0);
}
INA_TEST_SKIP_LINUX(test_os,  open_file_skip) {
    INA_UNUSED(data);
    INA_TEST_ASSERT_TRUE(0);
}

INA_TEST_DATA_WINDOWS(test_os_fixture) {
    int x;
    int skip;
};

INA_TEST_SETUP_WINDOWS(test_os_fixture) {
    data->x += 1;
}

INA_TEST_TEARDOWN_WINDOWS(test_os_fixture) {
    data->x -= 1;
}

INA_TEST_FIXTURE_WINDOWS(test_os_fixture, open_file) {
    INA_UNUSED(data);
#ifdef INA_OS_WINDOWS
    INA_TEST_ASSERT_TRUE(1);
#else 
    INA_TEST_ASSERT_TRUE(0);
#endif
}

INA_TEST_DATA_OSX(test_os_fixture) {
    int x;
    int skip;
};

INA_TEST_SETUP_OSX(test_os_fixture) {
    data->x += 1;
}

INA_TEST_TEARDOWN_OSX(test_os_fixture) {
    data->x -= 1;
}

INA_TEST_FIXTURE_OSX(test_os_fixture, open_file) {
    INA_UNUSED(data);
#ifdef INA_OS_OSX
    INA_TEST_ASSERT_TRUE(1);
#else 
    INA_TEST_ASSERT_TRUE(0);
#endif
}

INA_TEST_DATA_LINUX(test_os_fixture) {
    int x;
    int skip;
};

INA_TEST_SETUP_LINUX(test_os_fixture) {
    data->x += 1;
}

INA_TEST_TEARDOWN_LINUX(test_os_fixture) {
    data->x -= 1;
}

INA_TEST_FIXTURE_LINUX(test_os_fixture, open_file) {
    INA_UNUSED(data);
#ifdef INA_OS_LINUX
    INA_TEST_ASSERT_TRUE(1);
#else 
    INA_TEST_ASSERT_TRUE(0);
#endif
}

INA_TEST_DATA_WINDOWS(test_os_fixture_skip) {
    int x;
    int skip;
};

INA_TEST_SETUP_WINDOWS(test_os_fixture_skip) {
    data->x += 1;
}

INA_TEST_TEARDOWN_WINDOWS(test_os_fixture_skip) {
    data->x -= 1;
}

INA_TEST_FIXTURE_SKIP_WINDOWS(test_os_fixture_skip, open_file_skip) {
    INA_UNUSED(data);
    INA_TEST_ASSERT_TRUE(0);
}

INA_TEST_DATA_OSX(test_os_fixture_skip) {
    int x;
    int skip;
};

INA_TEST_SETUP_OSX(test_os_fixture_skip) {
    data->x += 1;
}

INA_TEST_TEARDOWN_OSX(test_os_fixture_skip) {
    data->x -= 1;
}

INA_TEST_FIXTURE_SKIP_OSX(test_os_fixture_skip, open_file_skip) {
    INA_UNUSED(data);
    INA_TEST_ASSERT_TRUE(0);
}

INA_TEST_DATA_LINUX(test_os_fixture_skip) {
    int x;
    int skip;
};

INA_TEST_SETUP_LINUX(test_os_fixture_skip) {
    data->x = 2;
}

INA_TEST_TEARDOWN_LINUX(test_os_fixture_skip) {
    data->x -= 1;
}

INA_TEST_FIXTURE_SKIP_LINUX(test_os_fixture_skip, open_file_skip) {
    INA_UNUSED(data);
    INA_TEST_ASSERT_TRUE(0);
}



