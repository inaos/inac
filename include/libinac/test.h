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
#ifndef _LIBINAC_TEST_H_
#define _LIBINAC_TEST_H_

#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef INA_OS_WIN32
typedef struct ina_test_hid_s {
    HANDLE hProcess;
    HANDLE hThread;
} ina_test_hid_t;
#else
typedef struct ina_test_hid_s {
    pid_t pid;
} ina_test_hid_t;
#endif

#define INA_TEST_HELPER_SPAWN(hid, sname, hname, ...)                     \
    INA_TEST_MSG("starting helper %s for suite %s", #sname, #hname);      \
    INA_ASSERT_SUCCEED(ina_test_helper_spawn(hid, #sname, #hname, INA_NO,  __VA_ARGS__)); 

#define INA_TEST_HELPER_WAIT(hid, sname, hname, ...)                     \
    INA_TEST_MSG("starting helper %s for suite %s", #sname, #hname);      \
    INA_ASSERT_SUCCEED(ina_test_helper_spawn(hid, #sname, #hname, INA_YES,  __VA_ARGS__));

#define INA_TEST_HELPER_STOP(id) ina_test_helper_stop(id)

#define INA_TEST_ASSERT(v) INA_TEST_ASSERT_TRUE(v)
#define INA_TEST_ASSERT_SUCCESS(v) INA_TEST_ASSERT_EQUAL_INTEGER(INA_SUCCESS, v)
#define INA_TEST_ASSERT_FAILURE(v) INA_TEST_ASSERT_EQUAL_INTEGER(INA_FAILURE, v)
#define INA_TEST_ASSERT_SUCCEED(v) INA_TEST_ASSERT_TRUE(INA_SUCCEED(v))
#define INA_TEST_ASSERT_NOTSUCCEED(v) INA_TEST_ASSERT_FALSE(INA_SUCCEED(v))
#define INA_TEST_ASSERT_STR(exp, real) \
    ina_test_assert_str(exp, real, __FILE__, __LINE__)
#define INA_TEST_ASSERT_DATA(exp, expsize, real, realsize) \
    ina_test_assert_data(exp, expsize, real, realsize, __FILE__, __LINE__)
#define INA_TEST_ASSERT_EQUAL_INTEGER(exp, real) \
    ina_test_assert_equal_integer(exp, real, __FILE__, __LINE__)
#define INA_TEST_ASSERT_EQUAL_FLOATING(exp, real) \
    ina_test_assert_equal_floating(exp, real, __FILE__, __LINE__)
#define INA_TEST_ASSERT_NOT_EQUAL_INTEGER(exp, real) \
    ina_test_assert_not_equal_integer(exp, real, __FILE__, __LINE__)
#define INA_TEST_ASSERT_NOT_EQUAL_FLOATING(exp, real) \
    ina_test_assert_not_equal_floating(exp, real, __FILE__, __LINE__)
#define INA_TEST_ASSERT_NULL(real) \
    ina_test_assert_null((void*)real, __FILE__, __LINE__)
#define INA_TEST_ASSERT_NOT_NULL(real) \
    ina_test_assert_not_null(real, __FILE__, __LINE__)
#define INA_TEST_ASSERT_SAME(exp, real) \
    ina_test_assert_same(exp, real, __FILE__, __LINE__)
#define INA_TEST_ASSERT_NOT_SAME(exp, real) \
    ina_test_assert_not_same(exp, real, __FILE__, __LINE__)
#define INA_TEST_ASSERT_TRUE(real) \
    ina_test_assert_true(real, __FILE__, __LINE__)
#define INA_TEST_ASSERT_FALSE(real) \
    ina_test_assert_false(real, __FILE__, __LINE__)
#define INA_TEST_ASSERT_FAIL() ina_test_assert_fail(__FILE__, __LINE__)

/* Setup callback */
typedef void (*ina_test_setup_cb_t)(void*);
/* Teardown callback */
typedef void (*ina_test_teardown_cb_t)(void*);

/* Test case */
typedef struct ina_test_testcase_s {
    const char* suite_name;
    const char* test_name; 
    void (*run)();
    int skip;
    int is_helper;
    void *data;
    ina_test_setup_cb_t setup;
    ina_test_teardown_cb_t teardown;
    unsigned int magic;
} ina_test_testcase_t;

/* Test function name. For internal purpose only. */
#define INA_TEST_FNAME(sname, tname) __ina_test_##sname##_##tname##_run
/* Test struct name. For internal purpose only */
#define INA_TEST_TNAME(sname, tname) __ina_test_##sname##_##tname
/* Magic Word: Dead code. For internal purpose only */
#define INA_TEST_MAGIC (0xDEADC0DE)

/* Section holding test cases */
#ifdef INA_OS_OSX
#define INA_TEST_ATTR_WEAK __attribute__ ((weak)) 
#define INA_TEST_SECTION_PUSH
#define INA_TEST_SECTION __attribute__ ((unused,section ("__DATA,.inatest")))
#elif INA_OS_WIN32
#pragma section(".inatest", read)
#define INA_TEST_ATTR_WEAK __declspec(selectany)
#define INA_TEST_SECTION
#define INA_TEST_SECTION_PUSH __declspec(allocate(".inatest"))
#else
#define INA_TEST_ATTR_WEAK __attribute__ ((weak)) 
#define INA_TEST_SECTION_PUSH
#define INA_TEST_SECTION __attribute__ ((unused,section (".inatest")))
#endif

/* Testcase data defines. For internal purpose only */
#define INA_TEST_STRUCT(sname, tname, _skip, __helper, __data, __setup, __teardown) \
    INA_TEST_SECTION_PUSH                                                \
    static ina_test_testcase_t INA_TEST_TNAME(sname, tname) INA_TEST_SECTION = { \
        #sname, \
        #tname, \
        INA_TEST_FNAME(sname, tname),\
        _skip, \
        __helper, \
        __data, \
        (ina_test_setup_cb_t)__setup,\
        (ina_test_teardown_cb_t)__teardown,\
        INA_TEST_MAGIC }

/* Define data for a test suite */
#define INA_TEST_DATA(sname) struct sname##_data
/* Define setup code für a suite */ 
#ifndef INA_OS_WIN32
#define INA_TEST_SETUP(sname) \
    void sname##_setup(struct sname##_data* data)
/* Define teardown code for a suite */
#define INA_TEST_TEARDOWN(sname) \
    void sname##_teardown(struct sname##_data* data)
#else
#define INA_TEST_SETUP(sname) \
    void  sname##_setup(struct sname##_data* data)
/* Define teardown code for a suite */
#define INA_TEST_TEARDOWN(sname) \
    void sname##_teardown(struct sname##_data* data)
#endif
/* Declare test case. For internal purpose only. */
#define INA_TEST_DECL(sname, tname, _skip) \
        void INA_TEST_FNAME(sname, tname)(); \
        INA_TEST_STRUCT(sname, tname, _skip, 0, NULL, NULL, NULL);\
        void INA_TEST_FNAME(sname, tname)()


/* Declare Test case with fixture. For internal purpose only. */
#ifdef INA_OS_OSX
#define INA_SETUP_FNAME(sname) NULL
#define INA_TEARDOWN_FNAME(sname) NULL
#else
#define INA_SETUP_FNAME(sname) sname##_setup
#define INA_TEARDOWN_FNAME(sname) sname##_teardown
#endif
#define INA_TEST_DECL_FIXTURE(sname, tname, _skip) \
    static struct sname##_data  __ina_test_##sname##_data; \
    INA_TEST_SETUP(sname); \
    INA_TEST_TEARDOWN(sname); \
    void INA_TEST_FNAME(sname, tname)(struct sname##_data* data); \
    INA_TEST_STRUCT(sname, tname, _skip, 0, &__ina_test_##sname##_data, INA_SETUP_FNAME(sname), INA_TEARDOWN_FNAME(sname)); \
    void INA_TEST_FNAME(sname, tname)(struct sname##_data* data)

#define INA_HELPER_DECL(sname, hname) \
        void INA_TEST_FNAME(sname, hname)(int *retval, int argc, char **argv); \
        INA_TEST_STRUCT(sname, hname, 1, 1, NULL, NULL, NULL);\
        void INA_TEST_FNAME(sname, hname)(int *retval, int argc, char **argv)

/* Define test case */
#define INA_TEST(sname, tname) INA_TEST_DECL(sname, tname, 0)
/* Skip a text case */
#define INA_TEST_SKIP(sname, tname) INA_TEST_DECL(sname, tname, 1)
/* Define test case unsing fixture features */
#define INA_TEST_FIXTURE(sname, tname) INA_TEST_DECL_FIXTURE(sname, tname, 0)
/* Skip test case with fixture features */
#define INA_TEST_FIXTURE_SKIP(sname, tname) INA_TEST_DECL_FIXTURE(sname, tname, 1)
/* Define helper */
#define INA_TEST_HELPER(sname, hname) INA_HELPER_DECL(sname, hname)
/* Print out message */
#define INA_TEST_MSG(fmt, ...) ina_test_msg(INA_NO, fmt, __VA_ARGS__)
/* Print out a error message */
#define INA_TEST_ERR(fmt, ...) ina_test_msg(INA_YES, fmt, __VA_ARGS__)

/*
 * Printout a message
 */
INA_API(ina_rc_t) ina_test_msg(int is_error, char *fmt, ...);

/*
 *
 */
INA_API(void) ina_test_assert_str(const char *exp, const char *real, 
                                  const char* caller, int line);

/*
 *
 */
INA_API(void) ina_test_assert_data(const unsigned char* exp, int expsize,
                                   const unsigned char* real, int realsize,
                                   const char* caller, int line);
/*
 *
 */
INA_API(void) ina_test_assert_equal_integer(int64_t exp, int64_t real, const char *caller, 
                                    int line);
/*
 *
 */
INA_API(void) ina_test_assert_equal_floating(double exp, double real, const char *caller, 
                                    int line);
/*
 *
 */
INA_API(void) ina_test_assert_not_equal_integer(int64_t exp, int64_t real, 
                                        const char *caller, int line);
/*
 *
 */
INA_API(void) ina_test_assert_not_equal_floating(double exp, double real, 
                                        const char *caller, int line);

/*
 *
 */
INA_API(void) ina_test_assert_null(const void *real, const char *caller, 
                                   int line);

/*
 *
 */
INA_API(void) ina_test_assert_not_null(const void *real, const char *caller, 
                                       int line);

/*
 *
 */
INA_API(void) ina_test_assert_same(const void *exp,  const void *real, 
                                   const char *caller, int line);

/*
 *
 */
INA_API(void) ina_test_assert_not_same(const void *exp,  const void *real, 
                                       const char *caller, int line);

/*
 *
 */
INA_API(void) ina_test_assert_true(int real, const char *caller, int line);

/*
 *
 */
INA_API(void) ina_test_assert_false(int real, const char *caller, int line);

/*
 *
 */
INA_API(void) ina_test_assert_fail(const char *caller, int line);

/*
 *
 */
INA_API(ina_rc_t) ina_test_helper_spawn(ina_test_hid_t *hid, const char *suite_name, const char* helper_name, int32_t wait_msec, ...);

/*
 *
 */
INA_API(ina_rc_t) ina_test_helper_stop(ina_test_hid_t *hid);

/*
 *
 */
INA_API(int) ina_test_helper_run(int argc, char *argv[]);

/*
 * Run tests
 */
int ina_test_run(int argc, char *argv[]);

#ifdef __cplusplus
}
#endif
#endif
