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

#ifdef INA_OS_WIN32
#define snprintf sprintf_s
#endif

#define __INA_MSG_SIZE 4096

typedef int (*ina_test_filter_fn_t)(ina_test_testcase_t*);

static size_t      __errorsize;
static char*       __errormsg;
static char        __errorbuffer[__INA_MSG_SIZE];
static jmp_buf     __err;
static const char* __suite_name;
static const char* __helper_name;
static const char* __binpath;

INA_TEST(suite, test) { }

static int __ina_suite_all(ina_test_testcase_t* t) {
    return t->is_helper == 0;
}

static int __ina_suite_filter(ina_test_testcase_t* t) { 
    return (strncmp(__suite_name, t->suite_name, strlen(__suite_name)) == 0) &&
        t->is_helper == 0;
}

static int __ina_helper_filter(ina_test_testcase_t* t) { 
    return (strncmp(__suite_name, t->suite_name, strlen(__suite_name)) == 0) &&
        (strncmp(__helper_name, t->test_name, strlen(__helper_name)) == 0) &&
        t->is_helper == 1;
}

#ifdef INA_OS_OSX
static void *__ina_find_symbol(ina_test_testcase_t *test, const char *fname)
{
    size_t len = strlen(test->suite_name) + 1 + strlen(fname);
    char *symbol_name = (char *) malloc(len + 1);
    memset(symbol_name, 0, len + 1);
    snprintf(symbol_name, len + 1, "%s_%s", test->suite_name, fname);
    void *symbol = dlsym(RTLD_DEFAULT, symbol_name);
    if (!symbol) {
        //fprintf(stderr, ">>>> ERROR: %s\n", dlerror());
    }
    // returns NULL on error

    free(symbol_name);
    return symbol;
}
#endif

static void __ina_signal_handler(int sig) {
    longjmp(__err, 1);
}

INA_API(ina_rc_t) ina_test_msg(int is_error, char *fmt, ...)
 {
     int size;
     va_list argp;
     
     if (is_error != INA_YES) {
         size = sprintf(__errormsg, "%s", "     MSG: ");
    } else {
        size = sprintf(__errormsg, "%s", "     ERR: ");
    }
    __errorsize -= size;
    __errormsg += size;
    
     va_start(argp, fmt);
     size = vsnprintf(__errormsg, __errorsize, fmt, argp);
     va_end(argp); 
     __errorsize -= size;
     __errormsg += size;
    
     size = sprintf(__errormsg, "%s", "\n");
     __errorsize -= size;
     __errormsg += size;
     return INA_SUCCESS;
 }

INA_API(void) ina_test_assert_str(const char *exp, const char *real, 
                const char *caller, int line) 
{
    if ((exp == NULL && real != NULL) ||
        (exp != NULL && real == NULL) ||
        (exp && real && strcmp(exp, real) != 0)) {
        INA_TEST_ERR("%s:%d  expected '%s', got '%s'", caller, line, exp, real);
        longjmp(__err, 1);
    }
}

INA_API(void) ina_test_assert_data(const unsigned char *exp, int expsize,
                  const unsigned char *real, int realsize,
                  const char *caller, int line) 
{
    int i;
    if (expsize != realsize) {
        INA_TEST_ERR("%s:%d  expected %d bytes, got %d", 
                        caller, 
                        line, 
                        expsize, 
                        realsize);
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

INA_API(void) ina_test_assert_equal_integer(int64_t exp, int64_t real, const char *caller, 
                int line) 
{
    if (exp != real) {
        INA_TEST_ERR("%s:%d  expected %ld, got %ld", caller, line, exp, real);
        longjmp(__err, 1);
    }
}

INA_API(void) ina_test_assert_equal_floating(double exp, double real, const char *caller, 
                int line) 
{
    if (exp != real) {
        INA_TEST_ERR("%s:%d  expected %ld, got %ld", caller, line, exp, real);
        longjmp(__err, 1);
    }
}

INA_API(void) ina_test_assert_not_equal_integer(int64_t exp, int64_t real, const char *caller, 
                int line) 
{
    if ((exp) == (real)) {
        INA_TEST_ERR("%s:%d  should not be %ld", caller, line, real);
        longjmp(__err, 1);
    }
}

INA_API(void) ina_test_assert_not_equal_floating(double exp, double real, const char *caller, 
                int line) 
{
    if ((exp) == (real)) {
        INA_TEST_ERR("%s:%d  should not be %ld", caller, line, real);
        longjmp(__err, 1);
    }
}

INA_API(void) ina_test_assert_null(const void* real, const char *caller, 
                int line) 
{
    if ((real) != NULL) {
        INA_TEST_ERR("%s:%d  should be NULL", caller, line);
        longjmp(__err, 1);
    }
}

INA_API(void) ina_test_assert_not_null(const void *real, const char *caller,
                int line) 
{
    if (real == NULL) {
        INA_TEST_ERR("%s:%d  should not be NULL", caller, line);
        longjmp(__err, 1);
    }
}

INA_API(void) ina_test_assert_same(const void *exp, const void *real,
                    const char *caller, int line)
{
    if (&real != &exp) {
        INA_TEST_ERR("%s:%d  should be SAME", caller, line);
        longjmp(__err, 1);
    }
}

INA_API(void) ina_test_assert_not_same(const void *exp, const void *real, 
                    const char *caller, int line) 
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

INA_API(ina_rc_t) ina_test_helper_spawn(ina_test_hid_t *hid, const char *suite_name, const char* helper_name, int32_t wait_msec, ...) {
#ifndef INA_OS_WIN32
    char* args[16];
    int n;

    INA_ASSERT_NOTNULL(hid);

    pid_t pid = fork();
    
    if (pid < 0) {
         perror("fork");
         return INA_FAILURE;
     }
     
     if (pid == 0) {
       /* child */
       n = 0;

       args[n] = __binpath;
       args[n++] = "-h";
       args[n++] = (char*)suite_name;
       args[n++] = (char*)helper_name;
       /*va_start(ap, wait);
       while (*wait) {
           args[n++] = va_arg(ap, char *);
       }
       va_end(ap);*/
       args[n++] = NULL;
       execvp(args[0], args);
       perror("execvp()");
       _exit(127);
    }
    hid->pid = pid;
    ina_time_sleep(500);
    return INA_SUCCESS;
#else
    PROCESS_INFORMATION pi;
    STARTUPINFOA si;
    DWORD dwExitCode;
    char cmdline[256];
    char exepath[MAX_PATH];
    
    INA_ASSERT_NOTNULL(hid);

    GetModuleFileName(NULL, exepath, MAX_PATH-1);
    sprintf(cmdline, "\"%s\" -h %s %s", exepath, suite_name, helper_name);
    ina_mem_set(&si, 0, sizeof(si));
    ina_mem_set(&pi, 0, sizeof(pi));
    si.cb = sizeof(si);

    if (CreateProcess(NULL, cmdline, 0, 0, FALSE, 
            CREATE_DEFAULT_ERROR_MODE, 0, 0,
            &si, &pi) != FALSE) {
        if (wait_msec < 0) {
            wait_msec = INFINITE;
        }
        if (wait_msec > 0) {
            dwExitCode = WaitForSingleObject(pi.hProcess, wait_msec);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            hid->hProcess = NULL;
            hid->hThread = NULL;
        } else {
            hid->hProcess = pi.hProcess;
            hid->hThread = pi.hThread;
        }
        return INA_SUCCESS;
    }
    return INA_FAILURE;
#endif
}

INA_API(ina_rc_t) ina_test_helper_stop(ina_test_hid_t *hid)
{
    INA_ASSERT_NOTNULL(hid);
#ifdef INA_OS_WIN32
    TerminateProcess(hid->hProcess, 0);
    CloseHandle(hid->hProcess);
    CloseHandle(hid->hThread);
    hid->hProcess = NULL;
    hid->hThread = NULL;
#else
    kill(hid->pid, SIGKILL);
#endif
    return INA_SUCCESS;
}

/*
 * Start a Helper
 */
INA_API(int) ina_test_helper_run(int argc, char *argv[])
{
    static ina_test_filter_fn_t filter = __ina_suite_all;
    static ina_test_testcase_t* test;
    ina_test_testcase_t* begin;
    ina_test_testcase_t* end;
    static int retval = EXIT_FAILURE;

    if (argc < 3) {
        return retval;
    }
    __suite_name = argv[2];
    __helper_name = argv[3];
    filter = __ina_helper_filter;
 
    begin = &INA_TEST_TNAME(suite, test);
    end = &INA_TEST_TNAME(suite, test);
 
    while (1) {
        ina_test_testcase_t* t = begin-1;
        if (t->magic != INA_TEST_MAGIC) {
            break;
        }
        begin--;
    }
    while (1) {
        ina_test_testcase_t* t = end+1;
        if (t->magic != INA_TEST_MAGIC) {
            break;
        }
        end++;
    }
    end++;

     for (test = begin; test != end; test++) {
        if (test == &__ina_test_suite_test) {
            continue;
        }
        if (filter(test)) {
            test->run(&retval, argc, argv);
        }
    }
    return retval;
}

INA_API(int) ina_test_run(int argc, char *argv[])
{
    static int total = 0;
    static int num_ok = 0;
    static int num_fail = 0;
    static int num_skip = 0;
    static int index = 1;
    static ina_test_filter_fn_t filter = __ina_suite_all;
    static ina_test_testcase_t* test;
    ina_test_testcase_t* begin;
    ina_test_testcase_t* end;
    ina_cio_color_t color;

    __binpath = argv[0];

    if (argc > 2) {
        if (strcmp(argv[1], "-h")==0) {
            return ina_test_helper_run(argc, argv);
        }
    }

    if (argc == 2) {
        __suite_name = argv[1];
        filter = __ina_suite_filter;
    }
 
    begin = &INA_TEST_TNAME(suite, test);
    end = &INA_TEST_TNAME(suite, test);
 
    while (1) {
        ina_test_testcase_t* t = begin-1;
        if (t->magic != INA_TEST_MAGIC) {
            break;
        }
        begin--;
    }
    while (1) {
        ina_test_testcase_t* t = end+1;
        if (t->magic != INA_TEST_MAGIC) {
            break;
        }
        end++;
    }
    end++;

    for (test = begin; test != end; test++) {
        if (test == &__ina_test_suite_test) {
            continue;
        }
        if (filter(test)) {
            total++;
        }
    }

    for (test = begin; test != end; test++) {
        if (test == &__ina_test_suite_test) {
            continue;
        }
        if (filter(test)) {
            __errorbuffer[0] = 0;
            __errorsize = __INA_MSG_SIZE-1;
            __errormsg = __errorbuffer;
            printf("TEST %d/%d %s:%s ", index, total, test->suite_name, test->test_name);
            fflush(stdout);
            if (test->skip) {
                ina_cio_printf(-1,-1, INA_CIO_COLOR_YELLOW, 
                        INA_CIO_COLOR_UNDEFINED, 
                        "[SKIPPED]\n");

                num_skip++;
            } else {
                 void* old_sigabrt_handler = NULL;
                 void* old_sigsegv_handler = NULL;
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
                    old_sigabrt_handler = signal(SIGABRT, 
                           __ina_signal_handler);
                    old_sigsegv_handler = signal(SIGSEGV, 
                    __ina_signal_handler);

                if (setjmp(__err) == 0) {
                    if (test->data) {
                        test->run(test->data);
                    } else {
                        test->run();
                    }
                    ina_cio_printf(-1,-1, INA_CIO_COLOR_GREEN, 
                            INA_CIO_COLOR_UNDEFINED, 
                            "[OK]\n");
                    num_ok++;
                } else {
                    ina_cio_printf(-1,-1, INA_CIO_COLOR_RED, 
                            INA_CIO_COLOR_UNDEFINED, 
                            "[FAIL]\n");
                    num_fail++;
                }
                signal(SIGABRT, old_sigabrt_handler);
                signal(SIGSEGV, old_sigsegv_handler);
                if (test->teardown) {
                    test->teardown(test->data);
                }

                if (__errorsize != __INA_MSG_SIZE-1) {
                    printf("%s", __errorbuffer);
                }
            }
            index++;
        }
    }

    color = (num_fail) ? INA_CIO_COLOR_RED : INA_CIO_COLOR_GREEN;
    ina_cio_printf(-1,-1, color, INA_CIO_COLOR_UNDEFINED, 
                "RESULTS: %d tests (%d ok, %d failed, %d skipped)\n", 
                total, 
                num_ok, 
                num_fail, 
                num_skip);
    return num_fail;
}
