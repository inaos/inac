/*
 * Copyright (c) 2012-2014, INAOS GmbH
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
#include "config.h"
#ifdef INA_OS_WIN32
static HANDLE __main_thread = NULL;
#endif

/* Internal registry short option */
typedef struct __ina_sopt_s {
    ina_str_t opt;
    ina_str_t value;
    ina_str_t desc;
    ina_opt_type_t type;
    UT_hash_handle hh;
} __ina_sopt_t;

/* Internal registry long option */
typedef struct __ina_lopt_s {
    ina_str_t opt;
    __ina_sopt_t *short_opt;
    UT_hash_handle hh;
} __ina_lopt_t;

/* internal signal handler */
static void __ina_signal_handler(int);
/* internal signal setter */
static void __ina_signal(int, void(*)(int));

/* get command line option */
static __ina_sopt_t *__ina_opt_get(const char*); 
/* display usage */
static void __ina_opt_usage(void);
/* get absolute path */
static ina_rc_t __ina_get_binpath(ina_str_t path);

/* initialization flag, > 0 lib/app initialized */
static int32_t __initialized = 0;
/* function pointer to a custom cleanup routine */
static ina_cleanup_handler_t  __cleanup = NULL;
/* short command line options */
static __ina_sopt_t *__sopt = NULL;
/* long command line options */
static __ina_lopt_t *__lopt = NULL;
/* that's our program name */
static ina_str_t __appname = NULL;
/* That's our app path */
static ina_str_t __apppath = NULL;

#ifdef INA_OS_WIN32
/* internal exception handler for windows */
static LONG WINAPI __ina_windows_exception_handler(EXCEPTION_POINTERS *);
#endif

/* Signal handler map */
static ina_signal_handler_t __signal_handler_map[] = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

INA_API(const char*) ina_app_get_name(void)
{
    return ina_str_cstr(__appname);
}

INA_API(const char*) ina_app_get_path(void)
{
    return ina_str_cstr(__apppath);
}

INA_API(ina_rc_t) ina_app_init(const int argc, char** argv, size_t pool_size, ina_opt_t *opt) 
{
    
#ifdef INA_OS_WIN32
    _set_abort_behavior(0, _WRITE_ABORT_MSG);
    __main_thread = GetCurrentThread();
#endif
    
    if (!INA_SUCCEED(ina_init(pool_size))) {
        return INA_ERR_PUSH_LAST;
    }
    
    if (argv != NULL) {
        const char* basename = strrchr(argv[0], INA_PATH_SEPARATOR);
        if (basename) {
            basename++;
        } else if (strlen(argv[0])) {
            basename = argv[0];
        }
        if (basename) {
            __appname = ina_str_new_fromcstr(basename);
        }
        /* FIXME: not sure for all platforms */
        __apppath = ina_str_new(256);
        if (!INA_SUCCEED(__ina_get_binpath(__apppath))) {
            __apppath = ina_str_new_fromcstr(argv[0]);
        }
    }

    if (opt != NULL) {
        __ina_sopt_t *so = NULL;
        __ina_sopt_t *tmp_so =  NULL;

        while (opt->long_opt) {
            __ina_lopt_t *lo;
            __ina_sopt_t *so = (__ina_sopt_t*)ina_mem_alloc(sizeof(__ina_sopt_t));
            if (so == NULL) {
                return INA_ERR_PUSH_LAST;
            }
            so->opt = ina_str_new_fromcstr(opt->short_opt);
            if (opt->dft != NULL) {
                so->value = ina_str_new_fromcstr(opt->dft);
            }
            so->desc = ina_str_new_fromcstr(opt->desc);
            so->type = opt->type;
            
            if (strlen(so->opt)) {
                HASH_ADD_KEYPTR(hh, __sopt, ina_str_cstr(so->opt), ina_str_len(so->opt), so);
            }

            lo = (__ina_lopt_t*)ina_mem_alloc(sizeof(__ina_lopt_t));
            if (lo == NULL) {
                return INA_ERR_PUSH_LAST;
            }
            lo->opt = ina_str_new_fromcstr(opt->long_opt);
            lo->short_opt = so;
            HASH_ADD_KEYPTR(hh, __lopt, ina_str_cstr(lo->opt), ina_str_len(lo->opt), lo);
            opt++;
        }
        
        /* Parse arguments, if any */
        if (argv != NULL) {
            int n;
            for (n = 1; n < argc; n++ ) {
                size_t c = 0;
                size_t s = 0;
                size_t e = 0;
                size_t vs = 0;
                __ina_sopt_t *so = NULL; 
                while (argv[n][c]) {
                    switch (tolower(argv[n][c])) {
                        case '-': {
                            if (s == 0) {
                                if (argv[n][c+1] == '-') {
                                    c++;
                                }
                                s=c+1;
                            }
                            break;
                        }
                        case '=': {
                            e = c;
                            vs = c+1;
                            break;
                        }
                        default:
                            break;
                    }
                    if (vs != 0) {
                        break;
                    }
                    c++;
                }
                
                if (e == 0) {
                    e = c;
                }

                if (s > 0 && (e-s) >= 0) {
                    char buf[100];
                    strncpy(buf, &argv[n][s], e-s);
                    buf[c-s] = 0;
                    INA_TRACE3("opt=%s", buf);
                    so = __ina_opt_get(buf);
                    if (so == NULL) {
                        INA_TRACE2("invalid options %s", buf);
                        __ina_opt_usage();
                        return INA_LIB_EOPT;
                    }
                    /* Flags don't have any value associated */
                    if (so->type != INA_OPT_TYPE_FLAG) {
                        /* value separated by space? */
                        if (vs == 0) {
                            if (argc > n+1) {
                                so->value = ina_str_new_fromcstr(argv[n+1]);
                                n++;
                            }
                        } else {
                            strcpy(buf, &argv[n][vs]);
                            so->value = ina_str_new_fromcstr(buf);
                        }
                    } else {
                        so->value = ina_str_new_fromcstr("on");
                    }
                }
            }
            
            /* Validate, any options must have a value except flags */
            HASH_ITER(hh, __sopt, so, tmp_so) {
                if (so->type != INA_OPT_TYPE_FLAG && so->value == NULL) {
                    __ina_opt_usage();
                    return INA_LIB_EOPT;
                }
            }
        }
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_init(size_t pool_size)
{
#ifdef INA_OS_WIN32
    WSADATA wsaData;
#endif

    if (__initialized++) {
        return INA_SUCCESS;
    }
    if (atexit(ina_exit) != 0) {
        INA_TRACE("Failed to register exit function!");
        return INA_FAILURE;
    }

    /* Setup signals */
    __ina_signal(SIGABRT, __ina_signal_handler);
    __ina_signal(SIGILL,  __ina_signal_handler);
    __ina_signal(SIGINT,  __ina_signal_handler);
    __ina_signal(SIGTERM, __ina_signal_handler);
#ifndef INA_OS_WIN32
    __ina_signal(SIGFPE, __ina_signal_handler);
    __ina_signal(SIGSEGV, __ina_signal_handler);
    __ina_signal(SIGBUS,  __ina_signal_handler);
    __ina_signal(SIGHUP,  __ina_signal_handler);
    __ina_signal(SIGQUIT, __ina_signal_handler);
    __ina_signal(SIGKILL, __ina_signal_handler);
    __ina_signal(SIGSTOP, __ina_signal_handler);
    __ina_signal(SIGTTIN, __ina_signal_handler);
    __ina_signal(SIGTTOU, __ina_signal_handler);
#else
    /* Set unhandled exception handler for windows */
    SetUnhandledExceptionFilter(__ina_windows_exception_handler);
#endif

    /* initailized console */
    if (!INA_SUCCEED(ina_cio_init())) {
        return INA_ERR_PUSH_LAST;
    }

    /* initalize global memory functions for memory pools */
    ina_mempool_set_fn(NULL, NULL, NULL);

    /* initalize error state */
    ina_err_reset();

   /* initialize system memory pool and internal structures */
    if (!INA_SUCCEED(ina_mempool_init(pool_size))) {
        return INA_ERR_PUSH_LAST;
    }
#ifdef INA_OS_WIN32
    /* Make sure to use high-accuracy multimedia-timers for windows */
	timeBeginPeriod(1);
    /* Initialize winsock */
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        return INA_NET_ENETINIT;
    }
#endif

    return INA_SUCCESS;
}

INA_API(void) ina_exit(void)
{
    if (!__initialized) {
        return;
    }

    while (__initialized--) {
    }

    /* Reset CIO attributes */
    ina_cio_reset();

    if (__cleanup != NULL) {
        __cleanup(0, 0);
    }

    if (__appname != NULL) {
        ina_str_free(__appname);
    }
    if (__apppath != NULL) {
        ina_str_free(__apppath);
    }

    /* FIXME: Crashes during tests because sys mem pool 
       was destroyed */
    /*if (__lopt != NULL) {
        __ina_lopt_t *lo = NULL;
        __ina_lopt_t *tmp_lo =  NULL;    
        HASH_ITER(hh, __lopt, lo, tmp_lo) {
            HASH_DEL(__lopt, lo);
            ina_mem_free(lo);
        }
    }

    if (__sopt != NULL) {
        __ina_sopt_t *so = NULL;
        __ina_sopt_t *tmp_so =  NULL;    
        HASH_ITER(hh, __sopt, so, tmp_so) {
            HASH_DEL(__sopt, so);
            ina_mem_free(so);
        }
    }*/

    ina_mempool_destroy();

    if (!INA_SUCCEED(ina_err_peek())) {
        ina_err_trace();
    }
    ina_err_reset();

#ifdef INA_OS_WIN32
	timeEndPeriod(1);
    WSACleanup();
#endif
}

INA_API(ina_cleanup_handler_t) ina_set_cleanup_handler(
                                        ina_cleanup_handler_t handler)
{
    ina_cleanup_handler_t old;

    old = __cleanup;
    __cleanup = handler;
    return old;
}

INA_API(ina_signal_handler_t) ina_register_signal_handler(ina_signal_t sig, 
                                            ina_signal_handler_t handler)
{
    ina_signal_handler_t old = __signal_handler_map[sig];
    __signal_handler_map[sig] = handler;
    return old;   
}

INA_API(ina_rc_t) ina_opt_isset(const char *opt) 
{
    __ina_sopt_t *so = __ina_opt_get(opt);
    if (so == NULL) {
        /* FIXME: specific error */
        return INA_FAILURE;
    }
    if (so->type == INA_OPT_TYPE_FLAG && so->value == NULL) {
        return INA_FAILURE;
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_opt_get_key_value(int index,  ina_str_t *key, 
                                         ina_str_t *value)
{
    __ina_lopt_t *lo = NULL;

    INA_ASSERT_TRUE(index >= 0);

    *key = NULL;
    *value = NULL;

    for (lo = __lopt; lo != NULL && index > 0; lo=lo->hh.next) {
        --index;
    }

    if (lo == NULL) {
        return INA_FAILURE;
    }
    *key = lo->opt;
    *value = lo->short_opt->value;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_opt_get_string(const char *opt, ina_str_t *value)
{
    __ina_sopt_t *so = __ina_opt_get(opt);
    if (so == NULL) {
        /* FIXME: specific error */
        return INA_FAILURE;
    }
    *value = ina_str_dup(so->value);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_opt_get_float(const char *opt, float *value)
{
    __ina_sopt_t *so = __ina_opt_get(opt);
    if (so == NULL) {
        /* FIXME: specific error */
        return INA_FAILURE;
    }
    *value = (float)atof(so->value);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_opt_get_int(const char *opt, int *value)
{
    __ina_sopt_t *so = __ina_opt_get(opt);
    if (so == NULL) {
        /* FIXME: specific error */
        return INA_FAILURE;
    }
    *value = atoi(so->value);
    return INA_SUCCESS;
}

static __ina_sopt_t *
__ina_opt_get(const char *opt) 
{
    __ina_sopt_t *so = NULL;

    INA_ASSERT_NOTNULL(opt);

    HASH_FIND_STR(__sopt, opt, so);
    if (so == NULL) {
        __ina_lopt_t *lo = NULL;
        HASH_FIND_STR(__lopt, opt, lo);
        if (lo != NULL) {
            so = lo->short_opt;
        }
    }
    return so;
}

static void 
__ina_opt_usage(void)
{
    __ina_lopt_t *lo = NULL;
    __ina_lopt_t *tmp_lo =  NULL;
    __ina_sopt_t *so = NULL;

    printf("USAGE: %s ", ina_str_cstr(__appname));
    
    HASH_ITER(hh, __lopt, lo, tmp_lo) {
        so = lo->short_opt;
        if (strlen(so->opt) > 0) {
            printf(" -%s | --%s", ina_str_cstr(so->opt), ina_str_cstr(lo->opt));
        } else {
            printf(" --%s", ina_str_cstr(lo->opt));
        }
        if (so->type != INA_OPT_TYPE_FLAG) {
            printf("%s", "= [");
            if (so->type == INA_OPT_TYPE_STRING) {
                printf("%s", "STRING] ");
            } else {
                printf("%s", "INT] ");
            }
        }
    }
    printf("%s", "\n\n");
    HASH_ITER(hh, __lopt, lo, tmp_lo) {
        so = lo->short_opt;
        if (strlen(so->opt) > 0) {
            printf("   -%s | --%s , %s\n", ina_str_cstr(so->opt), 
                ina_str_cstr(lo->opt), 
                ina_str_cstr(so->desc));
        } else {
            printf("        --%s , %s\n",  
                ina_str_cstr(lo->opt), 
                ina_str_cstr(so->desc));           
        }
    }
}

static ina_rc_t 
__ina_get_binpath(ina_str_t path)
{
#ifndef INA_OS_WIN32
    char linkname[64]; /* /proc/<pid>/exe */
    pid_t pid;
    int ret;
    char *buf;

    buf = (char*)ina_str_cstr(path);

    /* Get our PID and build the name of the link in /proc */
    pid = getpid();
    if (snprintf(linkname, sizeof(linkname), "/proc/%i/exe", pid) < 0) {
        abort();
    }

    /* Now read the symbolic link */
    ret = readlink(linkname, buf, ina_str_size(path));

    /* In case of an error, leave the handling up to the caller */
    if (ret == -1)
        return INA_FAILURE;

    /* Report insufficient buffer size */
    if (ret >= ina_str_size(path)) {
        errno = ERANGE;
        return INA_FAILURE;
    }

    /* Ensure proper NUL termination */
    buf[ret] = 0;
#elif INA_OS_WIN32
    HMODULE hMod;
    DWORD ret;
    size_t buf_size = ina_str_size(path);
    char *buf = (char*)ina_str_cstr(path);

    hMod = GetModuleHandle(NULL);
    ret = GetModuleFileName(hMod, buf, buf_size);

    if (ret == ERROR_INSUFFICIENT_BUFFER) {
        return INA_FAILURE;
    }
    else if (ret >= ina_str_size(path)) {
        return INA_FAILURE;
    }

    /* Ensure proper NUL termination */
    buf[ret] = 0;
#endif
    return INA_SUCCESS;   
}


static void
__ina_signal_handler(int sig)
{
    static int exitcode = EXIT_SUCCESS;
    static int signaled = 0;
    ina_signal_t isig;
    ina_signal_handler_t sh = NULL;
    ina_signal_behavior_t sb = INA_SIGNAL_BEHAVIOR_DFT;
    
    if (signaled != 0) {
        return;
    }
    signaled = sig;

    switch (sig) {
        case SIGABRT:
            isig = INA_SIGNAL_ABRT;
            break;        
        case SIGFPE:
            isig = INA_SIGNAL_FPE;
            break;
        case SIGILL:
            isig = INA_SIGNAL_ILL;
            break;
        case SIGSEGV:
            isig = INA_SIGNAL_SEGV;
            break;
        case SIGTERM:
            isig = INA_SIGNAL_TERM;
            break;
        case SIGINT:
            isig = INA_SIGNAL_INT;
            break;
#ifndef INA_OS_WIN32
        case SIGHUP:
            isig = INA_SIGNAL_HUP;
            break;
        case SIGQUIT:
            isig = INA_SIGNAL_QUIT;
            break;
        case SIGSTOP:
            isig = INA_SIGNAL_STOP;
            break;
        case SIGKILL:
            isig = INA_SIGNAL_KILL;
            break; 
        case SIGTTOU:
            isig = INA_SIGNAL_TTOU;
            break;
        case SIGTTIN:
            isig = INA_SIGNAL_TTIN;
            break;
#endif
        default:
            INA_TRACE("Unknown signal received!");
            abort();
    }
    sh = __signal_handler_map[isig];

    if (sh) {
#ifdef INA_OS_WIN32
        WaitForSingleObject(__main_thread, INFINITE);
#endif
        sh(isig, &sb, &exitcode);
    }

    switch (sig) {
        case SIGABRT:
            if (sb != INA_SIGNAL_BEHAVIOR_IGNORE) {
                fprintf(stderr, "Program aborted.\n");
                ina_err_trace();
                ina_err_reset();
#ifndef INA_OS_WIN32
                ina_err_backtrace(NULL);
#endif        
                exit(EXIT_FAILURE);
            }
            break;        
        case SIGFPE:
        case SIGILL:
        case SIGSEGV:
            if (sb != INA_SIGNAL_BEHAVIOR_IGNORE) {
                fprintf(stderr, "Error: signal %d:\n", sig);
                ina_err_trace();
                ina_err_reset();
#ifndef INA_OS_WIN32
                ina_err_backtrace(NULL);
#endif
                exit(EXIT_FAILURE);
                break;
            }
        case SIGTERM:
        case SIGINT:
#ifndef INA_OS_WIN32
        case SIGTTOU:
        case SIGTTIN:
        case SIGHUP:
        case SIGQUIT:
            if (sb != INA_SIGNAL_BEHAVIOR_IGNORE) {
                exit(exitcode);
            }
            break;
        case SIGSTOP:
        case SIGKILL:
#endif
            break;
        default:
            INA_TRACE("Unknown signal received!");
    }
}

void __ina_signal(int sig, void (*handler)(int))
{
#ifdef INA_OS_WIN32
    signal(sig, handler);
#else
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler;
    sigfillset(&sa.sa_mask);
    sigaction(sig, &sa, NULL);
#endif
}

#ifdef INA_OS_WIN32
static LONG WINAPI __ina_windows_exception_handler(EXCEPTION_POINTERS *exception_ptr)
{
    ina_err_coredump(exception_ptr);
    ina_err_backtrace(exception_ptr);
    switch (exception_ptr->ExceptionRecord->ExceptionCode) {
        case EXCEPTION_FLT_DENORMAL_OPERAND:
        case EXCEPTION_FLT_DIVIDE_BY_ZERO:
        case EXCEPTION_FLT_INEXACT_RESULT:
        case EXCEPTION_FLT_INVALID_OPERATION:
        case EXCEPTION_FLT_OVERFLOW:
        case EXCEPTION_FLT_STACK_CHECK:
        case EXCEPTION_FLT_UNDERFLOW:
             __ina_signal_handler(SIGFPE);
             break;
        default:
            __ina_signal_handler(SIGSEGV);
            break;
    }
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif
