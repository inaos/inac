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
#include <setjmp.h>
#include "config.h"

#ifdef INA_OS_OSX
#include <dlfcn.h>
#endif

#define __INA_MSG_SIZE 4096

typedef int (*ina_test_filter_fn_t)(ina_test_testcase_t*);

static size_t      __errorsize;
static char*       __errormsg;
static char        __errorbuffer[__INA_MSG_SIZE];
static jmp_buf     __err;
static const char* __suite_name;

static INA_TEST(suite, test) { }

static int __ina_suite_all(ina_test_testcase_t* t) {
    return 1;
}

static int __ina_suite_filter(ina_test_testcase_t* t) { 
    return strncmp(__suite_name, t->suite_name, strlen(__suite_name)) == 0;
}

#ifdef INA_OS_OSX
static void *__ina_find_symbol(ina_test_testcase_t *test, const char *fname)
{
    size_t len = strlen(test->suite_name) + 1 + strlen(fname);
    char *symbol_name = (char *) malloc(len + 1);
    memset(symbol_name, 0, len + 1);
    snprintf(symbol_name, len + 1, "%s_%s", test->suite_name, fname);

    //fprintf(stderr, ">>>> dlsym: loading %s\n", symbol_name);
    void *symbol = dlsym(RTLD_DEFAULT, symbol_name);
    if (!symbol) {
        //fprintf(stderr, ">>>> ERROR: %s\n", dlerror());
    }
    // returns NULL on error

    free(symbol_name);
    return symbol;
}
#endif


static void __ina_msg_start(const char* color, const char* title) {
    int size;
    size = snprintf(__errormsg, __errorsize, "%s", color);
    __errorsize -= size;
    __errormsg += size;
    size = snprintf(__errormsg, __errorsize, "  %s: ", title);
    __errorsize -= size;
    __errormsg += size;
}

static void __ina_msg_end() {
    int size;
    size = snprintf(__errormsg, __errorsize, INA_CIO_ANSI_NORMAL);
    __errorsize -= size;
    __errormsg += size;
    size = snprintf(__errormsg, __errorsize, "\n");
    __errorsize -= size;
    __errormsg += size;
}

INA_API(ina_rc_t) ina_test_msg(int is_error, char *fmt, ...)
 {
     va_list argp;
     if (is_error != INA_YES) {
         __ina_msg_start(INA_CIO_ANSI_BLUE, "MSG");
     } else {
         __ina_msg_start(INA_CIO_ANSI_YELLOW, "ERR");
     }

     va_start(argp, fmt);
     int size = vsnprintf(__errormsg, __errorsize, fmt, argp);
     __errorsize -= size;
     __errormsg += size;
     va_end(argp);

     __ina_msg_end();
     return INA_SUCCESS;
 }

INA_API(void) ina_test_assert_str(const char* exp, const char*  real, const char* caller, int line) 
{
    if ((exp == NULL && real != NULL) ||
        (exp != NULL && real == NULL) ||
        (exp && real && strcmp(exp, real) != 0)) {
        INA_TEST_ERR("%s:%d  expected '%s', got '%s'", caller, line, exp, real);
        longjmp(__err, 1);
    }
}

INA_API(void) ina_test_assert_data(const unsigned char* exp, int expsize,
                  const unsigned char* real, int realsize,
                  const char* caller, int line) {
    int i;
    if (expsize != realsize) {
        INA_TEST_ERR("%s:%d  expected %d bytes, got %d", caller, line, expsize, realsize);
        longjmp(__err, 1);
    }
    for (i=0; i<expsize; i++) {
        if (exp[i] != real[i]) {
            INA_TEST_ERR("%s:%d expected 0x%02x at offset %d got 0x%02x",
                    caller, line, exp[i], i, real[i]);
            longjmp(__err, 1);
        }
    }
}

INA_API(void) ina_test_assert_equal(long exp, long real, const char *caller, int line) 
{
    if (exp != real) {
        INA_TEST_ERR("%s:%d  expected %ld, got %ld", caller, line, exp, real);
        longjmp(__err, 1);
    }
}

INA_API(void) ina_test_assert_not_equal(long exp, long real, const char *caller, int line) 
{
    if ((exp) == (real)) {
        INA_TEST_ERR("%s:%d  should not be %ld", caller, line, real);
        longjmp(__err, 1);
    }
}

INA_API(void) ina_test_assert_null(const void* real, const char *caller, int line) 
{
    if ((real) != NULL) {
        INA_TEST_ERR("%s:%d  should be NULL", caller, line);
        longjmp(__err, 1);
    }
}

INA_API(void) ina_test_assert_not_null(const void *real, const char *caller, int line) 
{
    if (real == NULL) {
        INA_TEST_ERR("%s:%d  should not be NULL", caller, line);
        longjmp(__err, 1);
    }
}

INA_API(void) ina_test_assert_same(const void *exp, const void *real, const char *caller, int line) 
{
    if (&real != &exp) {
        INA_TEST_ERR("%s:%d  should be SAME", caller, line);
        longjmp(__err, 1);
    }
}

INA_API(void) ina_test_assert_not_same(const void *exp, const void *real, const char *caller, int line) 
{
    if (&real == &exp) {
        INA_TEST_ERR("%s:%d  should not be SAME", caller, line);
        longjmp(__err, 1);
    }
}

INA_API(void) ina_test_assert_true(int real, const char *caller, int line) 
{
    if ((real) == 0) {
        INA_TEST_ERR("%s:%d  should be true", caller, line);
        longjmp(__err, 1);
    }
}

INA_API(void) ina_test_assert_false(int real, const char *caller, int line) 
{
    if ((real) != 0) {
        INA_TEST_ERR("%s:%d  should be false", caller, line);
        longjmp(__err, 1);
    }
}

INA_API(void) ina_test_assert_fail(const char *caller, int line) 
{ 
    INA_TEST_ERR("%s:%d  shouldn't come here", caller, line);
    longjmp(__err, 1);
}

/*
 * Start a Helper
 */
INA_API(ina_rc_t) ina_test_runhelper(const char* cmd)
{
    return INA_SUCCESS;
}

INA_API(int) ina_test_run(int argc, char *argv[])
{
    static int total = 0;
    static int num_ok = 0;
    static int num_fail = 0;
    static int num_skip = 0;
    static int index = 1;
    static ina_test_filter_fn_t filter = __ina_suite_all;

    if (argc == 2) {
        __suite_name = argv[1];
        filter = __ina_suite_filter;
    }
 
    ina_test_testcase_t* begin = &INA_TEST_TNAME(suite, test);
    ina_test_testcase_t* end = &INA_TEST_TNAME(suite, test);
 
    while (1) {
        ina_test_testcase_t* t = begin-1;
        if (t->magic != INA_TEST_MAGIC) break;
        begin--;
    }
    while (1) {
        ina_test_testcase_t* t = end+1;
        if (t->magic != INA_TEST_MAGIC) break;
        end++;
    }
    end++;

    static ina_test_testcase_t* test;
    for (test = begin; test != end; test++) {
        if (test == &__ina_test_suite_test) continue;
        if (filter(test)) total++;
    }

    for (test = begin; test != end; test++) {
        if (test == &__ina_test_suite_test) continue;
        if (filter(test)) {
            __errorbuffer[0] = 0;
            __errorsize = __INA_MSG_SIZE-1;
            __errormsg = __errorbuffer;
            printf("TEST %d/%d %s:%s ", index, total, test->suite_name, test->test_name);
            fflush(stdout);
            if (test->skip) {
                ina_cio_print(INA_CIO_ANSI_BYELLOW, "[SKIPPED]");
                num_skip++;
            } else {
                int result = setjmp(__err);
                if (result == 0) {
#ifdef INA_OS_OSX
                    if (!test->setup) {
                        test->setup = __ina_find_symbol(test, "setup");
                    }
                    if (!test->teardown) {
                        test->teardown = __ina_find_symbol(test, "teardown");
                    }
#endif

                    if (test->setup) {
                        test->setup(test->data);
                    }
                    if (test->data) {
                        test->run(test->data);
                    }
                    test->run();
                    
                    if (test->teardown) {
                        test->teardown(test->data);
                    }
                    ina_cio_print(INA_CIO_ANSI_BGREEN, "[OK]");
                    num_ok++;
                } else {
                    ina_cio_print(INA_CIO_ANSI_BRED, "[FAIL]");
                    num_fail++;
                }
                if (__errorsize != __INA_MSG_SIZE-1) printf("%s", __errorbuffer);
            }
            index++;
        }
    }

    const char* color = (num_fail) ? INA_CIO_ANSI_BRED : INA_CIO_ANSI_GREEN;
    char results[80];
    sprintf(results, "RESULTS: %d tests (%d ok, %d failed, %d skipped)", total, num_ok, num_fail, num_skip);
    ina_cio_print(color, results);
    return num_fail;
}