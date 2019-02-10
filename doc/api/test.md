
```C
#ifndef _LIBINAC_TEST_H_
```

Copyright INAOS GmbH, Thalwil, 2013-2018. All rights reserved

This software is the confidential and proprietary information of INAOS GmbH
("Confidential Information"). You shall not disclose such Confidential
Information and shall use it only in accordance with the terms of the
license agreement you entered into with INAOS GmbH.

```C
/* Test helper handle */
```

HELPER HANDLING

```C
#define INA_TEST_HELPER_INVOKE(hid, sname, hname, ...)                      \
```
Invoke test helper, returns immediately after starting the helper. If
helper could non be started or returns an error an assertion will fail.


**Parameters**
 - `hid`: Pointer to a helper handle
sname Test suite name
hname Helper name
 - `...`: Arguments to pass to the helper. All arguments must be of type char


```C
#define INA_TEST_HELPER_INVOKE_WAIT(hid, sname, hname, msec, ...)           \
```
Invoke test helper, waits child process to terminate for msec, helper
process is killed before returning.  If helper could non be started or
returns an error an assertion will fail.


**Parameters**
 - `hid`: Pointer to a helper handle
sname Test suite name
hname Helper name
 - `...`: Arguments to pass to the helper. All arguments must be of type char


```C
#define INA_TEST_HELPER_CMD(hid, cmd, ...)                                  \
```
Invoke external test helper, returns immediately after starting the helper.
If helper could non be started or returns an error an assertion will fail.


**Parameters**
 - `hid`: Pointer to a helper handle
 - `cmd`: Absolute path to the executable/command
 - `...`: Arguments to pass to the command. All arguments must be of type char


```C
#define INA_TEST_HELPER_CMD_WAIT(hid, cmd, ...)                             \
```
Invoke external test helper. Waits child process to terminate for msec,
helper process is killed before returning.  If helper could non be started
or returns an error an assertion will fail.


**Parameters**
 - `hid`: Pointer to a helper handle
 - `cmd`: Absolute path to the executable/command
 - `...`: Arguments to pass to the command. All arguments must be of type char


```C
#define INA_TEST_HELPER_TERMINATE(hid)                                      \
```
Stops/Kill helper process.


**Parameters**
 - `hid`: Pointer to a valid helper handle


```C
#define INA_TEST_HELPER_EXIT(rc)                                            \
```
Exit from Helper with return code
```C
#define INA_TEST_HELPER_SET_RC(rc)                                          \
```
Set the return code inside a main function
```C
#define INA_TEST_HELPER_CHECK_ARGC(c)                                       \
```
Check if min argument passed, if not exit with EXIT_FAILURE
```C
#define INA_TEST_HELPER_CARG(n) argv[4+n]
```
Get char argument at n position
```C
#define INA_TEST_HELPER_IARG(n) atoi(argv[4+n])
```
Get int argument at n position
```C
INA_API(ina_rc_t) ina_test_helper_spawn(ina_test_hid_t *hid,
```

Spawn a helper child process. Helper can be an defined as in-site helper
defined with INA_TEST_HELPER or an external executable/command.


**Parameters**
 - `hid`: 
 - `suite_name`: Name od test suite in which the helper is defined. Pass NULL
for external helpers.
helper_name Defined helper name or absolute/relative path to external
helper.
 - `wait_msec`: Milliseconds to wait the child process before killing it. After
wait_msec process will be killed. Pass 0 to not wait or pass
-1 to wait until process terminate.



**Return**

INA_SUCCESS  if no error occurred



```C
INA_API(ina_rc_t) ina_test_helper_terminate(ina_test_hid_t *hid);
```

Terminate a helper child process.


**Parameters**
 - `hid`: Pointer to a valid helper handle



**Return**

INA_SUCCESS


```C
INA_API(int) ina_test_helper_run(int argc, char *argv[]);
```

Invoke test helper.


**Parameters**
 - `argc`: Number of arguments
 - `argv`: Array of arguments



**Return**

Exit code


```C
#define INA_TEST_ASSERT(expr)                                               \
```

ASSERTION MACROS

```C
INA_API(void) ina_test_assert_equal_str(const char *exp,
```

Assert a string to be equal.


**Parameters**
 - `exp`:  Expected string
 - `real`: Real string
 - `caller`: Caller function name calling this assert
 - `line`: Caller line number


```C
INA_API(void) ina_test_assert_not_equal_str(const char *nexp,
```

Assert a string not to be equal.


**Parameters**
 - `exp`:  Not expected string
 - `real`: Real string
 - `caller`: Caller function name calling this assert
 - `line`: Caller line number


```C
INA_API(void) ina_test_assert_data(const unsigned char* exp,
```

Assert data chunk to be equal.


**Parameters**
 - `exp`: 
 - `real`: 
 - `exp_size`: Expected size in bytes
 - `real_size`: Real size in bytes
 - `caller`: Caller function name calling this assert
 - `line`: 


```C
INA_API(void) ina_test_assert_equal_int(int exp,
```

Assert integer value to be equal.


**Parameters**
 - `exp`: Expected value
 - `real`: Real value
 - `caller`: Caller function name calling this assert
 - `line`: Caller line number


```C
INA_API(void) ina_test_assert_equal_uint(unsigned int exp,
```

Assert integer value to be equal.


**Parameters**
 - `exp`: Expected value
 - `real`: Real value
 - `caller`: Caller function name calling this assert
 - `line`: Caller line number


```C
INA_API(void) ina_test_assert_equal_uint64(uint64_t exp,
```

Assert integer value to be equal.


**Parameters**
 - `exp`: Expected value
 - `real`: Real value
 - `caller`: Caller function name calling this assert
 - `line`: Caller line number


```C
INA_API(void) ina_test_assert_equal_int64(int64_t exp,
```

Assert integer value to be equal.


**Parameters**
 - `exp`: Expected value
 - `real`: Real value
 - `caller`: Caller function name calling this assert
 - `line`: Caller line number


```C
INA_API(void) ina_test_assert_equal_floating(double exp,
```

Assert floating value to be equal.


**Parameters**
 - `exp`: Expected value
 - `real`: Real value
 - `caller`: Caller function name calling this assert
 - `line`: Caller line number


```C
INA_API(void) ina_test_assert_not_equal_int(int exp,
```

Assert integer value not to be equal.


**Parameters**
 - `exp`: Expected value
 - `real`: Real value
 - `caller`: Caller function name calling this assert
 - `line`: Caller line number


```C
INA_API(void) ina_test_assert_not_equal_uint(unsigned int exp,
```

Assert integer value not to be equal.


**Parameters**
 - `exp`: Expected value
 - `real`: Real value
 - `caller`: Caller function name calling this assert
 - `line`: Caller line number


```C
INA_API(void) ina_test_assert_not_equal_int64(int64_t exp,
```

Assert integer value not to be equal.


**Parameters**
 - `exp`: Expected value
 - `real`: Real value
 - `caller`: Caller function name calling this assert
 - `line`: Caller line number


```C
INA_API(void) ina_test_assert_not_equal_uint64(uint64_t exp,
```

Assert integer value not to be equal.


**Parameters**
 - `exp`: Expected value
 - `real`: Real value
 - `caller`: Caller function name calling this assert
 - `line`: Caller line number


```C
INA_API(void) ina_test_assert_not_equal_floating(double exp,
```

Assert floating value not to be equal.


**Parameters**
 - `exp`: Expected value
 - `real`: Real value
 - `caller`: Caller function name calling this assert
 - `line`: Caller line number


```C
INA_API(void) ina_test_assert_null(const void *real,
```

Assert a pointer to be NULL.


**Parameters**
 - `real`: Real pointer
 - `caller`: Caller function name calling this assert
 - `line`: Caller line number


```C
INA_API(void) ina_test_assert_not_null(const void *real,
```

Assert pointer not to be NULL.


**Parameters**
 - `real`: Real pointer
 - `caller`: Caller function name calling this assert
 - `line`: Caller line number


```C
INA_API(void) ina_test_assert_same(const void *exp,
```

Assert two pointer ar the same (same address).


**Parameters**
 - `exp`: Expected pointer
 - `real`: Real pointer
 - `caller`: Caller function name calling this assert
 - `line`: Caller line number


```C
INA_API(void) ina_test_assert_not_same(const void *exp,
```

Assert two pointer are not the same (same address).


**Parameters**
 - `exp`: Expected pointer
 - `real`: Real pointer
 - `caller`: Caller function name calling this assert
 - `line`: Caller line number


```C
INA_API(void) ina_test_assert_true(int real, const char *caller, int line);
```

Assert true expression


**Parameters**
 - `real`: Real value
 - `caller`: Caller function name calling this assert
 - `line`: Caller line number


```C
INA_API(void) ina_test_assert_false(int real, const char *caller, int line);
```

Assert false expression


**Parameters**
 - `real`: Real value
 - `caller`: Caller function name calling this assert
 - `line`: Caller line number


```C
INA_API(void) ina_test_assert_succeed(ina_rc_t real, const char *caller, int line);
```

Assert succeed expression


**Parameters**
 - `real`: Real value
 - `caller`: Caller function name calling this assert
 - `line`: Caller line number


```C
INA_API(void) ina_test_assert_failed(ina_rc_t real, const char *caller, int line);
```

Assert failed expression


**Parameters**
 - `real`: Real value
 - `caller`: Caller function name calling this assert
 - `line`: Caller line number


```C
INA_API(void) ina_test_assert_fail(const char *caller, int line);
```

Assert


**Parameters**
 - `caller`: Caller function name calling this assert
 - `line`: Caller line number


```C
INA_API(void) ina_test_assert_signal(int sig, const char *caller, int line);
```

Assert signal


**Parameters**
 - `sig`: Signal
 - `caller`: Caller function name calling this assert
 - `line`: Caller line number


```C
/* Setup callback */
```

TEST HANDLING

```C
typedef void (*ina_test_teardown_cb_t)(void*);
```
Teardown callback
```C
typedef struct ina_test_testcase_s {
```
Test case
```C
#define INA_TEST_MAGIC (0xDEADC0DE)
```
Magic. For internal purpose only.
```C
#define INA_TEST_FNAME(sname, tname) __ina_test_##sname##_##tname##_run
```
Test function name. For internal purpose only.
```C
#define INA_TEST_TNAME(sname, tname) __ina_test_##sname##_##tname
```
Test struct name. For internal purpose only
```C
#ifdef INA_OS_OSX
```
Section holding test cases
```C
#define INA_TEST_STRUCT(sname, tname, _skip, __helper, __data, __setup,     \
```
Testcase data defines. For internal purpose only
```C
#define INA_TEST_DATA(sname) struct sname##_data
```
Define data for a test suite
```C
#ifndef INA_OS_WIN32
```
Define setup code für a suite
```C
#define INA_TEST_TEARDOWN(sname)                                            \
```
Define teardown code for a suite
```C
#define INA_TEST_TEARDOWN(sname)                                            \
```
Define teardown code for a suite
```C
#define INA_TEST_DECL(sname, tname, _skip)                                  \
```
Declare test case. For internal purpose only.
```C
#ifdef INA_OS_OSX
```
Declare Test case with fixture. For internal purpose only.
```C
#ifdef INA_OS_WIN32
```
Define test case
```C
#define INA_TEST_SKIP(sname, tname) INA_TEST_DECL(sname, tname, 1)
```
Skip a test case
```C
#define INA_TEST_FIXTURE(sname, tname) \
```
Define test case using fixture features
```C
#define INA_TEST_FIXTURE_SKIP(sname, tname) \
```
Skip test case with fixture features
```C
#define INA_TEST_HELPER(sname, hname) INA_HELPER_DECL(sname, hname)
```
Define helper
```C
#define INA_TEST_MSG(fmt, ...) ina_test_msg(INA_NO, fmt, __VA_ARGS__)
```
Print out message
```C
#define INA_TEST_ERR(fmt, ...) ina_test_msg(INA_YES, fmt, __VA_ARGS__)
```
Print out a error message
```C
int ina_test_run(int argc, char *argv[], ina_ljit_ctx_t *ctx);
```

Run test suites.


**Parameters**
 - `argc`: Argument count
 - `argv`: Array of arguments
 - `ctx`: LuaJIT context. Optional. only needed if lua test are included



**Return**

Exit code


```C
INA_API(ina_rc_t) ina_test_msg(int is_error, const char *fmt, ...);
```

Printout a message.


**Parameters**
 - `is_error`: INA_YES to print a error message
 - `fmt`: 
 - `...`: 



**Return**

INA_SUCCESS

