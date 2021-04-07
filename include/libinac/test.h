/*
 * Copyright 2013-2020 INAOS GmbH, Thalwil
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _LIBINAC_TEST_H_
#define _LIBINAC_TEST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <libinac/lib.h>

/* 
 * HELPER HANDLING 
 */

/* Test helper handle */
typedef struct ina_test_hid_s ina_test_hid_t;

#ifdef INA_OS_WINDOWS
struct ina_test_hid_s {
    HANDLE hProcess;
    HANDLE hThread;
};
#else
struct ina_test_hid_s {
    pid_t pid;
};
#endif

/* Invoke test helper, returns immediately after starting the helper. If
 * helper could non be started or returns an error an assertion will fail.    
 *
 * Parameters
 *  hid   Pointer to a helper handle
 *  sname Test suite name
 *  hname Helper name
 *  ...   Arguments to pass to the helper. All arguments must be of type char*
 */
#define INA_TEST_HELPER_INVOKE(hid, sname, hname, ...)                      \
    INA_TEST_MSG("starting helper %s", #hname);                             \
    INA_TEST_ASSERT_SUCCEED(ina_test_helper_spawn(hid, #sname, #hname, 0,   \
                              __VA_ARGS__)); 

/* Invoke test helper, waits child process to terminate for msec, helper 
 * process is killed before returning.  If helper could non be started or 
 * returns an error an assertion will fail.
 *
 * Parameters
 *  hid   Pointer to a helper handle
 *  sname Test suite name
 *  hname Helper name
 *  ...   Arguments to pass to the helper. All arguments must be of type char*
 */
#define INA_TEST_HELPER_INVOKE_WAIT(hid, sname, hname, msec, ...)           \
    INA_TEST_MSG("starting helper %s", #hname);                             \
    INA_TEST_ASSERT_SUCCEED(ina_test_helper_spawn(hid, #sname, #hname, msec,\
                              __VA_ARGS__));

/* Invoke external test helper, returns immediately after starting the helper.
 * If helper could non be started or returns an error an assertion will fail.
 *
 * Parameters
 *  hid   Pointer to a helper handle
 *  cmd   Absolute path to the executable/command
 *  ...   Arguments to pass to the command. All arguments must be of type char*
 */
#define INA_TEST_HELPER_CMD(hid, cmd, ...)                                  \
    INA_TEST_HELPER_INVOKE(hid, NULL, cmd, ...)

/* Invoke external test helper. Waits child process to terminate for msec, 
 * helper process is killed before returning.  If helper could non be started 
 * or returns an error an assertion will fail.    
 *
 * Parameters
 *  hid   Pointer to a helper handle
 *  cmd   Absolute path to the executable/command
 *  ...   Arguments to pass to the command. All arguments must be of type char*
 */
#define INA_TEST_HELPER_CMD_WAIT(hid, cmd, ...)                             \
    INA_TEST_HELPER_INVOKE_WAIT(hid, NULL, cmd, msec...)

/* Stops/Kill helper process.
 * 
 * Parameters
 *  hid  Pointer to a valid helper handle
 */         
#define INA_TEST_HELPER_TERMINATE(hid)                                      \
    ina_test_helper_terminate(hid)

/* Exit from Helper with return code */
#define INA_TEST_HELPER_EXIT(rc)                                            \
    INA_TEST_HELPER_SET_RC(rc); return

/* Set the return code inside a main function */
#define INA_TEST_HELPER_SET_RC(rc)                                          \
    *retval = INA_RC_ERROR(rc)

/* Check if min argument passed, if not exit with EXIT_FAILURE */
#define INA_TEST_HELPER_CHECK_ARGC(c)                                       \
    if (argc<(3+c)) { INA_TEST_HELPER_EXIT(EXIT_FAILURE); }

/* Get char* argument at n position */ 
#define INA_TEST_HELPER_CARG(n) argv[4+n]

/* Get int argument at n position */
#define INA_TEST_HELPER_IARG(n) atoi(argv[4+n])
        
/*
 * Spawn a helper child process. Helper can be an defined as in-site helper
 * defined with INA_TEST_HELPER or an external executable/command.
 *
 * Parameters
 *  hid         Pointer to a helper handle.
 *  suite_name  Name od test suite in which the helper is defined. Pass NULL
 *              for external helpers.
 *  helper_name Defined helper name or absolute/relative path to external
 *              helper.
 *  wait_msec   Milliseconds to wait the child process before killing it. After
 *              wait_msec process will be killed. Pass 0 to not wait or pass
 *              -1 to wait until process terminate.
 * 
 * Return
 *  INA_SUCCESS  if no error occurred
 *
 */
INA_API(ina_rc_t) ina_test_helper_spawn(ina_test_hid_t *hid, 
                       const char *suite_name, 
                       const char *helper_name, 
                       int32_t wait_msec, ...);
/*
 * Terminate a helper child process.
 *
 * Parameters
 *  hid  Pointer to a valid helper handle
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_test_helper_terminate(ina_test_hid_t *hid);

/*
 * Invoke test helper.
 *
 * Parameters
 *  argc  Number of arguments
 *  argv  Array of arguments
 *
 * Return
 *  Exit code
 */
INA_API(int) ina_test_helper_run(int argc, char *argv[]);

/* 
 * ASSERTION MACROS
 */

#define INA_TEST_ASSERT(expr)                                               \
    INA_TEST_ASSERT_TRUE(expr)
#define INA_TEST_ASSERT_SUCCESS(expr)                                       \
    INA_TEST_ASSERT_EQUAL_UINT64(INA_SUCCESS, expr)
#define INA_TEST_ASSERT_SUCCEED(expr)                                       \
    ina_test_assert_succeed((expr), __FILE__, __LINE__)
#define INA_TEST_ASSERT_FAILED(expr)                                        \
    ina_test_assert_failed((expr), __FILE__, __LINE__)
#define INA_TEST_ASSERT_ERRMSG(exp, real)                                   \
    ina_test_assert_equal_uint((exp), INA_RC_ERRMSG((real)), __FILE__, __LINE__)
#define INA_TEST_ASSERT_EQUAL_STR(exp, real)                                \
    ina_test_assert_equal_str(exp, real, __FILE__, __LINE__)
#define INA_TEST_ASSERT_NOT_EQUAL_STR(exp, real)                            \
    ina_test_assert_not_equal_str(exp, real, __FILE__, __LINE__)
#define INA_TEST_ASSERT_DATA(exp, expsize, real, realsize)                  \
    ina_test_assert_data(exp, expsize, real, realsize, __FILE__, __LINE__)
#define INA_TEST_ASSERT_EQUAL_INT(exp, real)                                \
    ina_test_assert_equal_int(exp, real, __FILE__, __LINE__)
#define INA_TEST_ASSERT_EQUAL_UINT(exp, real)                               \
    ina_test_assert_equal_uint(exp, real, __FILE__, __LINE__)
#define INA_TEST_ASSERT_EQUAL_INT64(exp, real)                              \
    ina_test_assert_equal_int64(exp, real, __FILE__, __LINE__)
#define INA_TEST_ASSERT_EQUAL_UINT64(exp, real)                             \
    ina_test_assert_equal_uint64(exp, real, __FILE__, __LINE__)
#define INA_TEST_ASSERT_EQUAL_FLOATING(exp, real)                           \
    ina_test_assert_equal_floating(exp, real, __FILE__, __LINE__)
#define INA_TEST_ASSERT_NOT_EQUAL_INT(exp, real)                            \
    ina_test_assert_not_equal_int(exp, real, __FILE__, __LINE__)
#define INA_TEST_ASSERT_NOT_EQUAL_UINT(exp, real)                           \
    ina_test_assert_not_equal_uint(exp, real, __FILE__, __LINE__)
#define INA_TEST_ASSERT_NOT_EQUAL_INT64(exp, real)                          \
    ina_test_assert_not_equal_int64(exp, real, __FILE__, __LINE__)
#define INA_TEST_ASSERT_NOT_EQUAL_UINT64(exp, real)                         \
    ina_test_assert_not_equal_uint64(exp, real, __FILE__, __LINE__)
#define INA_TEST_ASSERT_EQUAL_TIME_T(exp, real)                             \
    ina_test_assert_equal_uint64(exp, real, __FILE__, __LINE__)
#define INA_TEST_ASSERT_NOT_EQUAL_TIME_T(exp, real)                         \
    ina_test_assert_not_equal_uint64(exp, real, __FILE__, __LINE__)

#ifdef INA_CPU_X86_64
#define INA_TEST_ASSERT_EQUAL_SIZE_T(exp, real)                             \
    ina_test_assert_equal_uint64(exp, real, __FILE__, __LINE__)
#define INA_TEST_ASSERT_NOT_EQUAL_SIZE_T(exp, real)                         \
    ina_test_assert_not_equal_uint64(exp, real, __FILE__, __LINE__)
#else
#define INA_TEST_ASSERT_EQUAL_SIZE_T(exp, real)                             \
    ina_test_assert_equal_uint(exp, real, __FILE__, __LINE__)
#define INA_TEST_ASSERT_NOT_EQUAL_SIZE_T(exp, real)                         \
    ina_test_assert_not_equal_uint(exp, real, __FILE__, __LINE__)
#endif
#define INA_TEST_ASSERT_NOT_EQUAL_FLOATING(exp, real)                       \
    ina_test_assert_not_equal_floating(exp, real, __FILE__, __LINE__)
#define INA_TEST_ASSERT_NULL(real)                                          \
    ina_test_assert_null((void*)real, __FILE__, __LINE__)
#define INA_TEST_ASSERT_NOT_NULL(real)                                      \
    ina_test_assert_not_null(real, __FILE__, __LINE__)
#define INA_TEST_ASSERT_SAME(exp, real)                                     \
    ina_test_assert_same(exp, real, __FILE__, __LINE__)
#define INA_TEST_ASSERT_NOT_SAME(exp, real)                                 \
    ina_test_assert_not_same(exp, real, __FILE__, __LINE__)
#define INA_TEST_ASSERT_TRUE(real)                                          \
    ina_test_assert_true(real, __FILE__, __LINE__)
#define INA_TEST_ASSERT_FALSE(real)                                         \
    ina_test_assert_false(real, __FILE__, __LINE__)
#define INA_TEST_ASSERT_FAIL()                                              \
    ina_test_assert_fail(__FILE__, __LINE__)
#define INA_TEST_ASSERT_SIGNAL(sig)                                         \
    ina_test_assert_signal(__FILE__, __LINE__)

/*
 * Assert a string to be equal.
 *
 * Parameters
 *  exp      Expected string
 *  real     Real string
 *  caller   Caller function name calling this assert
 *  line     Caller line number
 */
INA_API(void) ina_test_assert_equal_str(const char *exp,
                                        const char *real,
                                        const char *caller,
                                        int line);

/*
 * Assert a string not to be equal.
 *
 * Parameters
 *  exp      Not expected string
 *  real     Real string
 *  caller   Caller function name calling this assert
 *  line     Caller line number
 */
INA_API(void) ina_test_assert_not_equal_str(const char *nexp,
                                            const char *real,
                                            const char *caller,
                                            int line);

/*
 * Assert data chunk to be equal.
 *
 * Parameters
 *  exp        Pointer to expected data
 *  real       Pointer to real data
 *  exp_size   Expected size in bytes
 *  real_size  Real size in bytes
 *  caller     Caller function name calling this assert
 *  line       Caller line number
 */
INA_API(void) ina_test_assert_data(const unsigned char* exp,
                                   size_t exp_size,
                                   const unsigned char* real,
                                   size_t real_size,
                                   const char *caller,
                                   int line);

/*
 * Assert integer value to be equal.
 *
 * Parameters
 *  exp     Expected value
 *  real    Real value
 *  caller  Caller function name calling this assert
 *  line    Caller line number
 */
INA_API(void) ina_test_assert_equal_int(int exp,
                                        int real,
                                        const char *caller,
                                        int line);

/*
 * Assert integer value to be equal.
 *
 * Parameters
 *  exp     Expected value
 *  real    Real value
 *  caller  Caller function name calling this assert
 *  line    Caller line number
 */
INA_API(void) ina_test_assert_equal_uint(unsigned int exp,
                                         unsigned int real,
                                        const char *caller,
                                        int line);

/*
 * Assert integer value to be equal.
 *
 * Parameters
 *  exp     Expected value
 *  real    Real value
 *  caller  Caller function name calling this assert
 *  line    Caller line number
 */
INA_API(void) ina_test_assert_equal_uint64(uint64_t exp,
                                           uint64_t real,
                                           const char *caller,
                                           int line);

/*
 * Assert integer value to be equal.
 *
 * Parameters
 *  exp     Expected value
 *  real    Real value
 *  caller  Caller function name calling this assert
 *  line    Caller line number
 */
INA_API(void) ina_test_assert_equal_int64(int64_t exp,
                                          int64_t real,
                                           const char *caller,
                                           int line);
/*
 * Assert floating value to be equal.
 *
 * Parameters
 *  exp     Expected value
 *  real    Real value
 *  caller  Caller function name calling this assert
 *  line    Caller line number
 */
INA_API(void) ina_test_assert_equal_floating(double exp,
                                             double real,
                                             const char *caller,
                                             int line);

/*
 * Assert integer value not to be equal.
 *
 * Parameters
 *  exp     Expected value
 *  real    Real value
 *  caller  Caller function name calling this assert
 *  line    Caller line number
 */
INA_API(void) ina_test_assert_not_equal_int(int exp,
                                            int real,
                                            const char *caller,
                                            int line);

/*
 * Assert integer value not to be equal.
 *
 * Parameters
 *  exp     Expected value
 *  real    Real value
 *  caller  Caller function name calling this assert
 *  line    Caller line number
 */
INA_API(void) ina_test_assert_not_equal_uint(unsigned int exp,
                                            unsigned int real,
                                            const char *caller,
                                            int line);
/*
 * Assert integer value not to be equal.
 *
 * Parameters
 *  exp     Expected value
 *  real    Real value
 *  caller  Caller function name calling this assert
 *  line    Caller line number
 */
INA_API(void) ina_test_assert_not_equal_int64(int64_t exp,
                                              int64_t real,
                                                const char *caller,
                                                int line);

/*
 * Assert integer value not to be equal.
 *
 * Parameters
 *  exp     Expected value
 *  real    Real value
 *  caller  Caller function name calling this assert
 *  line    Caller line number
 */
INA_API(void) ina_test_assert_not_equal_uint64(uint64_t exp,
                                              uint64_t real,
                                              const char *caller,
                                              int line);
/*
 * Assert floating value not to be equal.
 *
 * Parameters
 *  exp     Expected value
 *  real    Real value
 *  caller  Caller function name calling this assert
 *  line    Caller line number
 */
INA_API(void) ina_test_assert_not_equal_floating(double exp,
                                                 double real,
                                                 const char *caller,
                                                 int line);

/*
 * Assert a pointer to be NULL.
 *
 * Parameters
 *  real    Real pointer
 *  caller  Caller function name calling this assert
 *  line    Caller line number
 */
INA_API(void) ina_test_assert_null(const void *real,
                                   const char *caller,
                                   int line);

/*
 * Assert pointer not to be NULL.
 *
 * Parameters
 *  real    Real pointer
 *  caller  Caller function name calling this assert
 *  line    Caller line number
 */
INA_API(void) ina_test_assert_not_null(const void *real,
                                       const char *caller,
                                       int line);

/*
 * Assert two pointer ar the same (same address).
 *
 * Parameters
 *  exp     Expected pointer
 *  real    Real pointer
 *  caller  Caller function name calling this assert
 *  line    Caller line number
 */
INA_API(void) ina_test_assert_same(const void *exp,
                                   const void *real,
                                   const char *caller, int line);

/*
 * Assert two pointer are not the same (same address).
 *
 * Parameters
 *  exp     Expected pointer
 *  real    Real pointer
 *  caller  Caller function name calling this assert
 *  line    Caller line number
 */
INA_API(void) ina_test_assert_not_same(const void *exp,
                                       const void *real,
                                       const char *caller,
                                       int line);

/*
 * Assert true expression
 *
 * Parameters
 *  real    Real value
 *  caller  Caller function name calling this assert
 *  line    Caller line number
 */
INA_API(void) ina_test_assert_true(int real, const char *caller, int line);

/*
 * Assert false expression
 *
 * Parameters
 *  real    Real value
 *  caller  Caller function name calling this assert
 *  line    Caller line number
 */
INA_API(void) ina_test_assert_false(int real, const char *caller, int line);

/*
 * Assert succeed expression
 *
 * Parameters
 *  real    Real value
 *  caller  Caller function name calling this assert
 *  line    Caller line number
 */
INA_API(void) ina_test_assert_succeed(ina_rc_t real, const char *caller, int line);

/*
 * Assert failed expression
 *
 * Parameters
 *  real    Real value
 *  caller  Caller function name calling this assert
 *  line    Caller line number
 */
INA_API(void) ina_test_assert_failed(ina_rc_t real, const char *caller, int line);

/*
 * Assert
 *
 * Parameters
 *  caller  Caller function name calling this assert
 *  line    Caller line number
 */
INA_API(void) ina_test_assert_fail(const char *caller, int line);

/*
 * Assert signal
 *
 * Parameters
 *  sig     Signal
 *  caller  Caller function name calling this assert
 *  line    Caller line number
 */
INA_API(void) ina_test_assert_signal(int sig, const char *caller, int line);


/*
 * TEST HANDLING
 */

/* Setup callback */
typedef void (*ina_test_setup_cb_t)(void*);
/* Teardown callback */
typedef void (*ina_test_teardown_cb_t)(void*);

/* Test case */
typedef struct ina_test_testcase_s {
    const char* suite_name;
    const char* test_name;
    INA_DISABLE_WARNING_CLANG(strict-prototypes)
    INA_DISABLE_WARNING_GCC(strict-prototypes)
    void (*run)();
    INA_ENABLE_WARNING_CLANG(strict-prototypes)
    INA_ENABLE_WARNING_GCC(strict-prototypes)
    int skip;
    int is_helper;
    void *data;
    ina_test_setup_cb_t setup;
    ina_test_teardown_cb_t teardown;
    unsigned int magic;
} ina_test_testcase_t;

/* Magic. For internal purpose only. */
#define INA_TEST_MAGIC (0xDEADC0DE)
/* Test function name. For internal purpose only. */
#define INA_TEST_FNAME(sname, tname) __ina_test_##sname##_##tname##_run
/* Test struct name. For internal purpose only */
#define INA_TEST_TNAME(sname, tname) __ina_test_##sname##_##tname

/* Section holding test cases */
#ifdef INA_OS_OSX
#define INA_TEST_SECTION __attribute__ ((unused,section ("__DATA, .inatest")))
#define INA_TEST_SECTION_PUSH
#elif INA_OS_WINDOWS
#pragma section(".inatest", read)
#define INA_TEST_SECTION
#define INA_TEST_SECTION_PUSH __declspec(allocate(".inatest"))
#else
#define INA_TEST_SECTION __attribute__ ((unused,section (".inatest")))
#define INA_TEST_SECTION_PUSH
#endif

/* Testcase data defines. For internal purpose only */
#define INA_TEST_STRUCT(sname, tname, _skip, __helper, __data, __setup,     \
                            __teardown)                                     \
    INA_TEST_SECTION_PUSH ina_test_testcase_t INA_TEST_TNAME(sname, tname) INA_TEST_SECTION = {   \
        #sname,                                                             \
        #tname,                                                             \
        INA_TEST_FNAME(sname, tname),                                       \
        _skip,                                                              \
        __helper,                                                           \
        __data,                                                             \
        (ina_test_setup_cb_t)__setup,                                       \
        (ina_test_teardown_cb_t)__teardown,                                 \
        INA_TEST_MAGIC }

/* Define data struct obfor a test suite */
#define INA_TEST_DATA(sname) struct sname##_data
/* Internal. Define empty data struct for a test suite */
#define INA_TEST_NO_DATA(sname, tname) struct sname##tname##_data { int _x; }

/* Define setup code für a suite */
#ifndef INA_OS_WINDOWS
#define INA_TEST_SETUP(sname)                                               \
    void sname##_setup(struct sname##_data* data)
/* Define teardown code for a suite */
#define INA_TEST_TEARDOWN(sname)                                            \
    void sname##_teardown(struct sname##_data* data)
#else
#define INA_TEST_SETUP(sname)                                               \
    void  sname##_setup(struct sname##_data* data)
/* Define teardown code for a suite */
#define INA_TEST_TEARDOWN(sname)                                            \
    void sname##_teardown(struct sname##_data* data)
#endif
/* Declare test case. For internal purpose only. */
#define INA_TEST_DECL(sname, tname, _skip)                                   \
    INA_TEST_NO_DATA(sname, tname);                                                  \
    void INA_TEST_FNAME(sname, tname) (struct sname##tname##_data* data);            \
    INA_TEST_STRUCT(sname, tname, _skip, 0, NULL, NULL, NULL);              \
    void INA_TEST_FNAME(sname, tname) (struct sname##tname##_data* data)

/* Declare Test case with fixture. For internal purpose only. */
#ifdef INA_OS_OSX
#define INA_SETUP_FNAME(sname) NULL
#define INA_TEARDOWN_FNAME(sname) NULL
#else
#define INA_SETUP_FNAME(sname) sname##_setup
#define INA_TEARDOWN_FNAME(sname) sname##_teardown
#endif
#define INA_TEST_DECL_FIXTURE(sname, tname, _skip)                          \
    static struct sname##_data  __ina_test_##sname##_data;                  \
    INA_TEST_SETUP(sname);                                                  \
    INA_TEST_TEARDOWN(sname);                                               \
    void INA_TEST_FNAME(sname, tname)(struct sname##_data* data);           \
    INA_TEST_STRUCT(sname, tname, _skip, 0, &__ina_test_##sname##_data,     \
        INA_SETUP_FNAME(sname), INA_TEARDOWN_FNAME(sname));                 \
    void INA_TEST_FNAME(sname, tname)(struct sname##_data* data)
#define INA_HELPER_DECL(sname, hname)                                       \
    void INA_TEST_FNAME(sname, hname)(int *retval, int argc, char **argv);  \
    INA_TEST_STRUCT(sname, hname, 1, 1, NULL, NULL, NULL);                  \
    void INA_TEST_FNAME(sname, hname)(int *retval, int argc, char **argv)

/* Define test case */
#ifdef INA_OS_WINDOWS
#define INA_TEST_DATA_WINDOWS(sname) INA_TEST_DATA(sname##_win)
#define INA_TEST_SETUP_WINDOWS(sname) INA_TEST_SETUP(sname##_win)
#define INA_TEST_TEARDOWN_WINDOWS(sname) INA_TEST_TEARDOWN(sname##_win)
#define INA_TEST_WINDOWS(sname, tname) INA_TEST(sname##_win, tname)
#define INA_TEST_SKIP_WINDOWS(sname, tname) INA_TEST_SKIP(sname##_win, tname)
#define INA_TEST_FIXTURE_WINDOWS(sname, tname) INA_TEST_FIXTURE(sname##_win, tname)
#define INA_TEST_FIXTURE_SKIP_WINDOWS(sname, tname) INA_TEST_FIXTURE_SKIP(sname##_win, tname)
#else
#define INA_TEST_DATA_WINDOWS(sname) INA_TEST_DATA(sname##_win)
#define INA_TEST_SETUP_WINDOWS(sname) INA_TEST_SETUP(sname##_win)
#define INA_TEST_TEARDOWN_WINDOWS(sname) INA_TEST_TEARDOWN(sname##_win)
#define INA_TEST_WINDOWS(sname, tname) INA_TEST_NO_DATA(sname##_win, tname);void x__ina_test_win32_##sname##_##tname(struct sname##_win##tname##_data* data)
#define INA_TEST_SKIP_WINDOWS(sname, tname) INA_TEST_WINDOWS(sname, tname)
#define INA_TEST_FIXTURE_WINDOWS(sname, tname) INA_TEST_WINDOWS(sname, tname)
#define INA_TEST_FIXTURE_SKIP_WINDOWS(sname, tname) INA_TEST_WINDOWS(sname, tname)
#endif

#ifdef INA_OS_OSX
#define INA_TEST_DATA_OSX(sname) INA_TEST_DATA(sname##_osx)
#define INA_TEST_SETUP_OSX(sname) INA_TEST_SETUP(sname##_osx)
#define INA_TEST_TEARDOWN_OSX(sname) INA_TEST_TEARDOWN(sname##_osx)
#define INA_TEST_OSX(sname, tname) INA_TEST(sname##_osx, tname)
#define INA_TEST_SKIP_OSX(sname, tname) INA_TEST_SKIP(sname##_osx, tname)
#define INA_TEST_FIXTURE_OSX(sname, tname) INA_TEST_FIXTURE(sname##_osx, tname)
#define INA_TEST_FIXTURE_SKIP_OSX(sname, tname) INA_TEST_FIXTURE_SKIP(sname##_osx, tname)
#else
#define INA_TEST_DATA_OSX(sname) INA_TEST_DATA(sname##_osx)
#define INA_TEST_SETUP_OSX(sname) INA_TEST_SETUP(sname##_osx)
#define INA_TEST_TEARDOWN_OSX(sname) INA_TEST_TEARDOWN(sname##_osx)
#define INA_TEST_OSX(sname, tname) INA_TEST_NO_DATA(sname##_osx, tname);void x__ina_test_osx_##sname##_##tname(struct sname##_osx##tname##_data* data)
#define INA_TEST_SKIP_OSX(sname, tname) INA_TEST_OSX(sname, tname)
#define INA_TEST_FIXTURE_OSX(sname, tname) INA_TEST_OSX(sname, tname)
#define INA_TEST_FIXTURE_SKIP_OSX(sname, tname) INA_TEST_OSX(sname, tname)
#endif

#ifdef INA_OS_LINUX
#define INA_TEST_DATA_LINUX(sname) INA_TEST_DATA(sname##_linux)
#define INA_TEST_SETUP_LINUX(sname) INA_TEST_SETUP(sname##_linux)
#define INA_TEST_TEARDOWN_LINUX(sname) INA_TEST_TEARDOWN(sname##_linux)
#define INA_TEST_LINUX(sname, tname) INA_TEST(sname##_linux, tname)
#define INA_TEST_SKIP_LINUX(sname, tname) INA_TEST_SKIP(sname##_linux, tname)
#define INA_TEST_FIXTURE_LINUX(sname, tname) INA_TEST_FIXTURE(sname##_linux, tname)
#define INA_TEST_FIXTURE_SKIP_LINUX(sname, tname) INA_TEST_FIXTURE_SKIP(sname##_linux, tname)
#else
#define INA_TEST_DATA_LINUX(sname) INA_TEST_DATA(sname##_linux)
#define INA_TEST_SETUP_LINUX(sname) INA_TEST_SETUP(sname##_linux)
#define INA_TEST_TEARDOWN_LINUX(sname) INA_TEST_TEARDOWN(sname##_linux)
#define INA_TEST_LINUX(sname, tname) INA_TEST_NO_DATA(sname##_linux, tname);void x__ina_test_linux_##sname##_##tname(struct sname##_linux##tname##_data* data)
#define INA_TEST_SKIP_LINUX(sname, tname) INA_TEST_LINUX(sname, tname)
#define INA_TEST_FIXTURE_LINUX(sname, tname) INA_TEST_LINUX(sname, tname)
#define INA_TEST_FIXTURE_SKIP_LINUX(sname, tname) INA_TEST_LINUX(sname, tname)

#endif

#define INA_TEST(sname, tname) INA_TEST_DECL(sname, tname, 0)
/* Skip a test case */
#define INA_TEST_SKIP(sname, tname) INA_TEST_DECL(sname, tname, 1)

/* Define test case using fixture features */
#define INA_TEST_FIXTURE(sname, tname) \
    INA_TEST_DECL_FIXTURE(sname, tname, 0)

/* Skip test case with fixture features */
#define INA_TEST_FIXTURE_SKIP(sname, tname) \
    INA_TEST_DECL_FIXTURE(sname, tname, 1)

/* Define helper */
#define INA_TEST_HELPER(sname, hname) INA_HELPER_DECL(sname, hname)

/* Print out message */
#define INA_TEST_MSG(fmt, ...) ina_test_msg(INA_NO, fmt, __VA_ARGS__)
/* Print out a error message */
#define INA_TEST_ERR(fmt, ...) ina_test_msg(INA_YES, fmt, __VA_ARGS__)

/*
 * Run test suites.
 *
 * Parameters
 *  argc  Argument count
 *  argv  Array of arguments
 *  ctx   LuaJIT context. Optional. only needed if lua test are included
 *
 * Return
 *  Exit code
 */
int ina_test_run(int argc, char *argv[], ina_ljit_ctx_t *ctx);

/*
 * Printout a message.
 *
 * Parameters
 *  is_error  INA_YES to print a error message
 *  fmt       Message format
 *  ...       Message arguments
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_test_msg(int is_error, const char *fmt, ...);


#ifdef __cplusplus
}
#endif
#endif
