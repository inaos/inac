/*
 * Copyright INAOS GmbH, Thalwil, 2012-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>
#include "config.h"
#ifdef INA_OS_WINDOWS
static HANDLE __main_thread = NULL;
#endif

/* Internal registry short option */
typedef struct __ina_sopt_s {
    ina_str_t opt;
    ina_str_t value;
    ina_str_t desc;
    ina_opt_type_t type;
    ina_list_node_t node;
} __ina_sopt_t;

/* Internal registry long option */
typedef struct __ina_lopt_s {
    ina_str_t opt;
    __ina_sopt_t *short_opt;
    ina_list_node_t node;
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
/* free short options */
static ina_rc_t __ina_free_sopt(void *data);
/* free long options */
static ina_rc_t __ina_free_lopt(void *data);

/* function pointer to a custom cleanup routine */
static ina_cleanup_handler_t  __cleanup = NULL;
/* short command line options */
static ina_list_t *__sopt = NULL;
/* long command line options */
static ina_list_t *__lopt = NULL;
/* that's our program name */
static ina_str_t __appname = NULL;
/* That's our app path */
static ina_str_t __apppath = NULL;

#ifdef INA_OS_WINDOWS
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

static ina_rc_t __ina_check_options(void *data)
{
    __ina_sopt_t *so = (__ina_sopt_t*)data;
    if (so->type != INA_OPT_TYPE_FLAG && so->value == NULL) {
        INA_ERROR(INA_ES_OPTION | INA_ERR_INVALID);
    }
    return INA_SUCCESS;
}

static int __ina_find_sopt(const void *data, const void *find_arg)
{
    const __ina_sopt_t *d = (__ina_sopt_t*)data;
    const char* name = (const char*)(find_arg);
    return strcmp(d->opt, name);
}

static int __ina_find_lopt(const void *data, const void *find_arg)
{
    const __ina_lopt_t *d = (__ina_lopt_t*)data;
    const char* name = (const char*)(find_arg);
    return strcmp(d->opt, name);
}

INA_API(const char*) ina_app_get_name(void)
{
    return ina_str_cstr(__appname);
}

INA_API(const char*) ina_app_get_path(void)
{
    return ina_str_cstr(__apppath);
}

INA_API(ina_rc_t) ina_app_init(int argc, char** argv, ina_opt_t *opt)
{
    
#ifdef INA_OS_WINDOWS
    _set_abort_behavior(INA_DGBMSG_ASSERT, _WRITE_ABORT_MSG);
    __main_thread = GetCurrentThread();
#endif
    INA_RETURN_IF_FAILED(ina_init());

    INA_INIT_GUARD();

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
        if (INA_FAILED(__ina_get_binpath(__apppath))) {
            __apppath = ina_str_new_fromcstr(argv[0]);
        }
    }

    if (opt != NULL) {
        INA_MUST_SUCCEED(ina_list_new(INA_LIST_CF_NOMALLOC, &__sopt));
        INA_MUST_SUCCEED(ina_list_new(INA_LIST_CF_NOMALLOC, &__lopt));

        while (opt->long_opt) {
            __ina_lopt_t *lo;
            __ina_sopt_t *so = (__ina_sopt_t*)ina_mem_alloc(sizeof(__ina_sopt_t));
            INA_MEM_SET_ZERO(so, __ina_sopt_t);
            so->node.data = so;
            if (so == NULL) {
                return ina_err_get_rc();
            }
            so->opt = ina_str_new_fromcstr(opt->short_opt);
            if (opt->dft != NULL) {
                so->value = ina_str_new_fromcstr(opt->dft);
            }
            so->desc = ina_str_new_fromcstr(opt->desc);
            so->type = opt->type;
            
            if (strlen(so->opt)) {
                ina_list_insert_tail(__sopt, &so->node);
            }

            lo = (__ina_lopt_t*)ina_mem_alloc(sizeof(__ina_lopt_t));
            INA_MEM_SET_ZERO(lo, __ina_lopt_t);
            if (lo == NULL) {
                return ina_err_get_rc();
            }
            lo->opt = ina_str_new_fromcstr(opt->long_opt);
            lo->short_opt = so;
            lo->node.data = lo;
            ina_list_insert_tail(__lopt, &lo->node);
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
                    strncpy(buf, &argv[n][s], (size_t)(e-s));
                    buf[c-s] = 0;
                    INA_TRACE3("opt=%s", buf);
                    so = __ina_opt_get(buf);
                    if (so == NULL) {
                        INA_TRACE2("invalid options %s", buf);
                        __ina_opt_usage();
                        return INA_ERROR(INA_ES_OPTION | INA_ERR_INVALID);
                    }
                    /* Flags don't have any value associated */
                    if (so->type != INA_OPT_TYPE_FLAG) {
                        /* value separated by space? */
                        if (vs == 0) {
                            if (argc > n+1) {
                                ina_str_free(so->value);
                                so->value = ina_str_new_fromcstr(argv[n+1]);
                                n++;
                            }
                        } else {
                            strncpy(buf, &argv[n][vs], strlen(&argv[n][vs]));
                            ina_str_free(so->value);
                            so->value = ina_str_new_fromcstr(buf);
                        }
                    } else {
                        ina_str_free(so->value);
                        so->value = ina_str_new_fromcstr("on");
                    }
                }
            }
            
            /* Validate, any options must have a value except flags */
            if (INA_FAILED(ina_list_foreach(__sopt, __ina_check_options))) {
                __ina_opt_usage();
                return ina_err_get_rc();
            }
        }
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_init(void)
{
#ifdef INA_OS_WINDOWS
    WSADATA wsaData;
#endif

    INA_INIT_GUARD();

    ina_err_init();

    if (atexit(ina_exit) == -1) {
        INA_TRACE("Failed to register exit function!");
        return INA_OS_ERROR(INA_ES_FUNCTION | INA_ERR_NOT_REGISTERED);
    }

    /* Setup signals */
    __ina_signal(SIGABRT, __ina_signal_handler);
    __ina_signal(SIGILL,  __ina_signal_handler);
    __ina_signal(SIGINT,  __ina_signal_handler);
    __ina_signal(SIGTERM, __ina_signal_handler);
#ifndef INA_OS_WINDOWS
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
    /*INA_RETURN_IF_FAILED(ina_mempool_init());*/

#ifdef _LIBINAC_HASHTABLE_H_
    /* initialise hashtable */
    INA_RETURN_IF_FAILED(ina_hashtable_init("hashtable.conf"));
#endif

#ifdef _LIBINAC_CIO_H_
    /* initialise console */
    INA_RETURN_IF_FAILED(ina_cio_init());
#endif


#ifdef INA_OS_WINDOWS
    /* Make sure to use high-accuracy multimedia-timers for windows */
    timeBeginPeriod(1);
    /* Initialize winsock */
    
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        return INA_OS_ERROR(INA_ES_OPERATION|INA_ERR_FAILED);
    }
#endif

#ifdef _LIBINAC_CPU_H_
    /* initialise CPU module */
    INA_RETURN_IF_FAILED(ina_cpu_init());
#endif

    return INA_SUCCESS;
}

INA_API(void) ina_exit(void)
{
    INA_DESTROY_GUARD();

    /* call cleanup handler if any */
    if (__cleanup != NULL) {
        __cleanup(0, 0);
    }

    /* Reset CIO attributes */
#ifdef _LIBINAC_CIO_H_
    ina_cio_reset();
#endif

#ifdef _LIBINAC_CPU_H_
    /* destroy cpu module */
    ina_cpu_destroy();
#endif

    /* free allocated memory  */
    if (__lopt != NULL) {
        ina_list_foreach(__sopt, __ina_free_sopt);
        ina_list_free(&__sopt);
    }
    if (__sopt != NULL) {
        ina_list_foreach(__lopt, __ina_free_lopt);
        ina_list_free(&__lopt);
    }
    ina_str_free(__appname);
    ina_str_free(__apppath);

#ifdef _LIBINAC_HASHTABLE_H_
    ina_hashtable_destroy();
#endif
#ifdef _LIBINAC_LOG_H_
    ina_log_destroy();
#endif
    /*ina_mempool_destroy();*/
    ina_err_destroy();

#ifdef INA_OS_WINDOWS
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
    __ina_sopt_t *so;
    INA_VERIFY_NOT_NULL(opt);

    so = __ina_opt_get(opt);
    if (so == NULL) {
        return INA_ERROR(INA_ES_OPTION | INA_ERR_NOT_EXISTS);
    }
    if (so->type == INA_OPT_TYPE_FLAG && so->value == NULL) {
        return INA_ERROR(INA_ES_OPTION | INA_ERR_NOT_EXISTS);
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_opt_get_key_value(int index,  ina_str_t *key, 
                                         ina_str_t *value)
{
    __ina_lopt_t *lo = NULL;
    ina_list_node_t *next = NULL;

    INA_VERIFY_NOT_NULL(key);
    INA_VERIFY_NOT_NULL(value);
    INA_VERIFY(index >= 0);

    *key = NULL;
    *value = NULL;

    if (INA_SUCCEED(ina_list_head(__lopt, &next))) {
        while (next && index > 0) {
            --index;
            next = next->next;
        }
    }
    if (index < 0 || next == NULL) {
        return INA_ERROR(INA_ES_OPTION | INA_ERR_NOT_EXISTS);
    }
    lo = (__ina_lopt_t*)next->data;
    *key = lo->opt;
    *value = lo->short_opt->value;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_opt_get_string(const char *opt, ina_str_t *value)
{
    __ina_sopt_t *so;

    INA_VERIFY_NOT_NULL(opt);
    INA_VERIFY_NOT_NULL(value);

    so = __ina_opt_get(opt);
    if (so == NULL) {
        *value = NULL;
        return INA_ERROR(INA_ES_OPTION | INA_ERR_NOT_EXISTS);
    }
    *value = ina_str_dup(so->value);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_opt_get_float(const char *opt, float *value)
{
    __ina_sopt_t *so;
    INA_VERIFY_NOT_NULL(opt);
    INA_VERIFY_NOT_NULL(value);
    *value = 0.0;
    so = __ina_opt_get(opt);
    if (so == NULL) {
        return INA_ERROR(INA_ES_OPTION | INA_ERR_NOT_EXISTS);
    }
    *value = (float)atof(so->value);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_opt_get_int(const char *opt, int *value)
{
    __ina_sopt_t *so;
    INA_VERIFY_NOT_NULL(opt);
    INA_VERIFY_NOT_NULL(value);
    *value = 0;
    so = __ina_opt_get(opt);
    if (so == NULL) {
        return INA_ERROR(INA_ES_OPTION | INA_ERR_NOT_EXISTS);
    }
    *value = atoi(so->value);
    return INA_SUCCESS;
}

static __ina_sopt_t *
__ina_opt_get(const char *opt) 
{
    ina_list_node_t *node = NULL;

    INA_ASSERT_NOT_NULL(opt);

    if (INA_SUCCEED(ina_list_find(__sopt, __ina_find_sopt, opt, &node))) {
        return (__ina_sopt_t *) node->data;
    }

    if (INA_SUCCEED(ina_list_find(__lopt, __ina_find_lopt, opt, &node))) {
        __ina_lopt_t *lo = (__ina_lopt_t*)node->data;
        return lo->short_opt;
    }
    return NULL;
}

static void 
__ina_opt_usage(void) {
    ina_list_node_t *next;
    __ina_lopt_t *lo = NULL;
    __ina_sopt_t *so = NULL;

    printf("USAGE: %s ", __appname);

    if (INA_SUCCEED(ina_list_head(__lopt, &next))) {
        while (next) {
            lo = (__ina_lopt_t *) next->data;
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
            next = next->next;
        }
    }
    printf("%s", "\n\n");

    if (INA_SUCCEED(ina_list_head(__lopt, &next))) {
        while (next) {
            lo = (__ina_lopt_t *) next->data;
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
            next = next->next;
        }
    }
}

static ina_rc_t 
__ina_get_binpath(ina_str_t path)
{
#ifndef INA_OS_WINDOWS
    char linkname[64]; /* /proc/<pid>/exe */
    pid_t pid;
    ssize_t ret;
    char *buf;

    buf = (char*)ina_str_cstr(path);

    /* Get our PID and build the name of the link in /proc */
    pid = getpid();
    if (snprintf(linkname, sizeof(linkname), "/proc/%i/exe", pid) < 0) {
        INA_OS_ERROR(INA_ERR_NOT_FATAL);
    }

    /* Now read the symbolic link */
    ret = readlink(linkname, buf, ina_str_size(path));
    /* In case of an error, leave the handling up to the caller */
    if (ret == -1) {
        return INA_OS_ERROR(INA_ES_OPERATION | INA_ERR_FAILED);
    }

    /* Report insufficient buffer size */
    if (ret >= (int)ina_str_size(path)) {
        return INA_ERROR(INA_ES_BUFFER | INA_ERR_TOO_SMALL);
    }

    /* Ensure proper NUL termination */
    buf[ret] = 0;
#elif INA_OS_WINDOWS
    HMODULE hMod;
    DWORD ret;
    DWORD buf_size = (DWORD)ina_str_size(path); /* Lenght of a path has to fix in a DWORD */
    char *buf = (char*)ina_str_cstr(path);

    hMod = GetModuleHandle(NULL);
    ret = GetModuleFileName(hMod, buf, buf_size);

    if (ret == ERROR_INSUFFICIENT_BUFFER) {
        return INA_OS_ERROR(INA_ES_BUFFER|INA_ERR_TOO_SMALL);
    }
    else if (ret >= ina_str_size(path)) {
        return INA_OS_ERROR(INA_ES_BUFFER|INA_ERR_TOO_SMALL);
    }

    /* Ensure proper NUL termination */
    buf[ret] = 0;
#endif
    return INA_SUCCESS;   
}

static ina_rc_t
__ina_free_sopt(void *data)
{
    __ina_sopt_t *opt = (__ina_sopt_t*)data;
    ina_str_free(opt->desc);
    ina_str_free(opt->opt);
    ina_str_free(opt->value);
    ina_mem_free(opt);
    return INA_SUCCESS;
}

static ina_rc_t
__ina_free_lopt(void *data)
{
    __ina_lopt_t *opt = (__ina_lopt_t*)data;
    ina_str_free(opt->opt);
    ina_mem_free(opt);
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
#ifndef INA_OS_WINDOWS
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
            INA_TRACE("Unknown signal %d received!", sig);
            abort();
    }
    sh = __signal_handler_map[isig];

    if (sh) {
        sh(isig, &sb, &exitcode);
    }
#ifdef INA_OS_WINDOWS
    WaitForSingleObject(__main_thread, INFINITE);
#endif

    switch (sig) {
        case SIGABRT:
            if (sb != INA_SIGNAL_BEHAVIOR_IGNORE) {
                exit(EXIT_FAILURE);
            }
            break;        
        case SIGFPE:
        case SIGILL:
        case SIGSEGV:
            if (sb != INA_SIGNAL_BEHAVIOR_IGNORE) {
                exit(EXIT_FAILURE);
            }
            break;
        case SIGTERM:
        case SIGINT:
#ifndef INA_OS_WINDOWS
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
    }
}

void __ina_signal(int sig, void (*handler)(int))
{
#ifdef INA_OS_WINDOWS
    signal(sig, handler);
#else
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler;
    sigfillset(&sa.sa_mask);
    sigaction(sig, &sa, NULL);
#endif
}

#ifdef INA_OS_WINDOWS
static LONG WINAPI __ina_windows_exception_handler(EXCEPTION_POINTERS *exception_ptr)
{
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
