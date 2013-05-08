/*
 * Copyright (c) 2012-2013, INAOS GmbH
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
/* get command line option */
static __ina_sopt_t *__ina_opt_get(const char*); 
/* display usage */
static void __ina_opt_usage(void);

/* initialization flag, > 0 lib/app initialized */
static int32_t __initialized = 0;
/* function pointer to a custom cleanup routine */
static ina_cleanup_handler_t  __cleanup = NULL;
/* incemented when a signal is catched */
static int __sig = 0;
/* short command line options */
static __ina_sopt_t *__sopt = NULL;
/* long command line options */
static __ina_lopt_t *__lopt = NULL;
/* that's our program name */
static ina_str_t __appname = NULL;

INA_API(const char*) ina_appname(void)
{
    return ina_str_cstr(__appname);
}

INA_API(ina_rc_t) ina_appinit(const int argc, char** argv, size_t pool_size, ina_opt_t *opt) 
{
    if (!INA_SUCCEED(ina_init(pool_size))) {
        return INA_ERR_PUSH_LAST;
    }
    
    if (argv != NULL) {
        const char* basename = strrchr(argv[0],(int)'/');
        if (basename) {
            basename++;
        }
        __appname = ina_str_fromcstr(basename);
    }

    if (opt != NULL) {
        __ina_sopt_t *so = NULL;
        __ina_sopt_t *tmp_so =  NULL;

        while (opt->short_opt) {
            __ina_lopt_t *lo;
            __ina_sopt_t *so = (__ina_sopt_t*)ina_mem_alloc(sizeof(__ina_sopt_t));
            if (so == NULL) {
                return INA_ERR_PUSH_LAST;
            }
            so->opt = ina_str_fromcstr(opt->short_opt);
            so->value = ina_str_fromcstr(opt->dft);
            so->desc = ina_str_fromcstr(opt->desc);
            so->type = opt->type;
            HASH_ADD_KEYPTR(hh, __sopt, ina_str_cstr(so->opt), ina_str_len(so->opt), so);

            lo = (__ina_lopt_t*)ina_mem_alloc(sizeof(__ina_lopt_t));
            if (lo == NULL) {
                return INA_ERR_PUSH_LAST;
            }
            lo->opt = ina_str_fromcstr(opt->long_opt);
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

                if (s > 0) {
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
                                so->value = ina_str_fromcstr(argv[n+1]);
                                n++;
                            }
                        } else {
                            strcpy(buf, &argv[n][vs]);
                            so->value = ina_str_fromcstr(buf);
                        }
                    } else {
                        so->value = ina_str_fromcstr("on");
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
    if (__initialized++) {
        return INA_SUCCESS;
    }
    if (atexit(ina_exit) != 0) {
        INA_TRACE_MSG("Failed to register exit function!");
        return INA_FAILURE;
    }

    /* Setup signals */
    signal(SIGFPE, __ina_signal_handler);
    signal(SIGABRT, __ina_signal_handler);
    signal(SIGILL, __ina_signal_handler);
    signal(SIGINT, __ina_signal_handler);
    signal(SIGSEGV, __ina_signal_handler);
    signal(SIGTERM, __ina_signal_handler);
#ifndef INA_OS_WIN32
    signal(SIGBUS, __ina_signal_handler);
    signal(SIGHUP, __ina_signal_handler);
    signal(SIGQUIT, __ina_signal_handler);
    signal(SIGKILL, __ina_signal_handler);
    signal(SIGSTOP, __ina_signal_handler);
#endif

   /* initailized console */
    if (!INA_SUCCEED(ina_cio_init())) {
        return INA_ERR_PUSH_LAST;
    }

    /* initalize global memory functions */
    ina_mem_set_fn(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    /* initalize global memory functions for memory pools */
    ina_mempool_set_fn(NULL, NULL, NULL);

    /* initalize error state */
    ina_err_reset();

   /* initialize system memory pool and internal structures */
    if (!INA_SUCCEED(ina_mempool_init(pool_size))) {
        return INA_ERR_PUSH_LAST;
    }
	/* Make sure to use high-accuracy multimedia-timers for windows */
#ifdef INA_OS_WIN32
	timeBeginPeriod(1);
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

    if (__cleanup != NULL) {
        __cleanup(0, 0);
    }

    if (__appname != NULL) {
        ina_str_destroy(__appname);
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
        printf(" -%s | --%s", ina_str_cstr(so->opt), ina_str_cstr(lo->opt));
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
        printf("   -%s | --%s , %s\n", ina_str_cstr(so->opt), ina_str_cstr(lo->opt), ina_str_cstr(so->desc));
    }
}

static void
__ina_signal_handler(int sig)
{
    int exitcode;
    
    if (__sig != 0) {
        return;
    }
    __sig = sig;

    exitcode = 3;
    switch (sig) {
        case SIGABRT:
        return;
        case SIGFPE:
        case SIGILL:
        case SIGSEGV:
            INA_TRACE_MSG("programm error signal received!");
            if (__cleanup) {
                 __cleanup(sig, 0);
            }
            /* Try to trace out the source of error */
            ina_err_trace();
            /* ... then stop */
            abort();
            break;
        case SIGTERM:
        case SIGINT:
#ifndef INA_OS_WIN32
        case SIGHUP:
        case SIGQUIT:
        case SIGSTOP:
        case SIGKILL:
#endif
            INA_TRACE_MSG("termination signal received!");
            exit(exitcode);
            break;
        default:
            INA_TRACE_MSG("unknown singal received!");
    }
    abort();
}