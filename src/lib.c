/*
 * Copyright (c) 2012, INAOS GmbH
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

static int32_t __initialized = 0;

/* function pointer to a custom cleanup routine */
static ina_cleanup_handler_t  __cleanup = NULL;
static int __sig = 0;

/* internal signal handler */
static void __ina_signal_handler(int);


INA_API(ina_rc_t) ina_appinit(const int argc,  char** argv, size_t pool_size) 
{
    return ina_init(pool_size);
}

INA_API(ina_rc_t) ina_init(size_t pool_size)
{
    if (__initialized++) {
        return INA_SUCCESS;
    }
    if (atexit(ina_exit) != 0) {
        INA_TRACE_MSG("Failed to regsiter exit fucntion!");
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

    /* initalize global memory functions */
    ina_mem_set_fn(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
     /* initalize global memory functions for memory pools */
    ina_mempool_set_fn(NULL, NULL, NULL);

    /* initalize error state */
    ina_err_reset();

   /* initialize system memory pool and internal structures */
    if (!INA_SUCCEED(ina_mempool_init(pool_size))) {
        return ina_err_peek();
    }
    return INA_SUCCESS;
}

INA_API(void) ina_exit(void)
{
    while (__initialized--) {
    }
    
    if (__cleanup != NULL) {
        __cleanup(0, 0);
    }
    ina_mempool_destroy();

    if (!INA_SUCCEED(ina_err_peek())) {
        ina_err_trace();
    }
    ina_err_reset();

#ifdef INA_OS_WIN32
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
        case SIGFPE:
        case SIGILL:
        case SIGSEGV:
        case SIGABRT:
            INA_TRACE_MSG("programm error signal received!");
            if (__cleanup) {
                 __cleanup(sig, 0);
            }
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