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

#define __INA_ERR_STATE_SIZE (32)
#define __INA_ERR_MESSAGE_EXTRALEN (20)

/* Error state */
typedef struct ina_error_state_s {
    size_t c;
    size_t ic;
    ina_error_t errors[__INA_ERR_STATE_SIZE];
} ina_error_state_t;

/* initialized module, returns always INA_SUCCESS */
static ina_rc_t __ina_init(void);
/* pop last error from error state. returns RC of new last error */
static ina_rc_t __ina_pop_error(void);
/* get index of error in the error state for a RC */
static size_t __ina_get_index(ina_rc_t);

/* global error state */
static ina_error_state_t __state;
/* initialization flag */
static int32_t __initialized = 0;

INA_API(ina_rc_t) ina_err_push(int mod, int fn, int reason, const char *file, 
                               int line, const char *msg)
{
    ina_error_t *error;

    INA_ASSERT(__initialized);
    INA_ASSERT(mod <= 64);
    INA_ASSERT(fn <= 32);
    INA_ASSERT(reason <= 1023);
    INA_ASSERT(reason > 0);
    INA_ASSERT_NOTNULL(file);
    INA_ASSERT(line > 0);
    INA_ASSERT_NOTNULL(msg);
    INA_ASSERT_NOTEQUAL(INA_SUCCESS, reason);
    
    if (__state.c == __INA_ERR_STATE_SIZE) {
        __ina_pop_error();
    }
    error = &__state.errors[__state.c++];
    error->rc = INA_RC_PACK(mod, fn, reason, ++__state.ic);
    error->ts = time(NULL); /* FIXME: use own time value */
    strcpy(error->file, file);
    error->line = line;
    strcpy(error->msg, msg);

    return error->rc;
}

INA_API(ina_rc_t) ina_err_repush(ina_rc_t rc, const char *file, int line)
{
    size_t k;

    if (rc == INA_SUCCESS) {
        return INA_SUCCESS;
    }

    k = __ina_get_index(rc);

    return ina_err_push(INA_RC_MOD(rc),
                 INA_RC_OSFN(rc),
                 INA_RC_REASON(rc),
                 file,
                 line,
                 __state.errors[k].msg);
}

INA_API(ina_rc_t) ina_err_succeed(ina_rc_t rc)
{
    if (INA_SUCCESS == rc || INA_RC_REASON(rc) == 0) {
        return 1;
    }
    return 0;
}

INA_API(ina_rc_t) ina_err_peek()
{
    if (__state.c > 0) {
        return __state.errors[__state.c-1].rc;
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_err_peek_next(ina_rc_t rc)
{
    size_t k;

    INA_ASSERT(__initialized);
    INA_ASSERT(INA_RC_ID(rc) <= __state.ic);

    if (rc == INA_ERR_PEEK_FIRST) {
        return ina_err_peek();
    }

    k = __ina_get_index(rc);

    if (k < __state.c) {
        return __state.errors[k-1].rc;
    }
    return INA_SUCCESS;
    
}

INA_API(ina_rc_t) ina_err_peek_last()
{
    INA_ASSERT(__initialized);
    if (__state.c > 0) {
        return __state.errors[0].rc;
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_err_clear(ina_rc_t rc)
{
    size_t k;
    ina_rc_t top;
    ina_rc_t ret;

    INA_ASSERT(INA_RC_ID(rc) <= __state.ic);
    ret = INA_SUCCESS;

    k = __ina_get_index(rc);
    
    if (k < __state.c) {
        INA_ASSERT_EQUAL(rc, __state.errors[k].rc);
        __state.errors[k].rc = rc|INA_ERR_FLAG_HANDLED;
        ret = __state.errors[k].rc;
        for (;;) {
            top =  __ina_pop_error();
            if (top == ret) {
                break;
            }
        }
    } else {
        ret = INA_FAILURE;
    }
    return ret;
}

INA_API(ina_rc_t) ina_err_reset(void)
{
    if (__initialized) {
        while (!(INA_SUCCESS == __ina_pop_error()));
    } else {
        __ina_init();
    }
    __state.ic = 0;
    INA_ASSERT(__state.c == 0);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_err_fmtmsg(ina_rc_t rc, char* str, size_t len)
{
    size_t k;
    struct tm *tm;
    ina_error_t *error;
    char tmc[30];
    char outstr[2048];

    INA_ASSERT_NOTNULL(str);
    INA_ASSERT(len > 0);

    if (INA_RC_ID(rc) <= __state.ic) {
        k = __ina_get_index(rc);
        if (k < __state.c) {
            error = &__state.errors[k];
            
            if (len < (strlen(error->msg) +
                       strlen(error->file) +
                       __INA_ERR_MESSAGE_EXTRALEN)) {
                return INA_ERR_EMSGLEN;
            }

            tm = localtime(&error->ts);

            if (strftime(tmc, sizeof(tmc), "%Y-%m-%d %H:%M:%S", tm) > 0) {
                sprintf(outstr, "%s %s:%d - %s (r:%u,f:%u,m:%u,h:%u,i:%d)",
                                            tmc, 
                                            error->file,
                                            error->line,
                                            error->msg,
                                            INA_RC_REASON(error->rc),
                                            INA_RC_OSFN(error->rc),
                                            INA_RC_MOD(error->rc),
                                            INA_RC_HANDLED(error->rc),
                                            INA_RC_ID(error->rc));

                if (strncpy(str, outstr, len) == NULL) {
                    return INA_ERR_EMSGFMT;
                }
                return INA_SUCCESS;
            }
        }
    }
    return INA_FAILURE;
}

INA_API(ina_rc_t) ina_err_trace(void)
{
    ina_rc_t rc = INA_SUCCESS;
    char str[2048];
    int n;

    INA_ASSERT(__initialized);

    if (INA_SUCCEED(ina_err_peek())) {
        return rc;
    }

    fprintf(stderr, "%s\n", "**** UNHANDLED ERROR START ******");

    rc = ina_err_peek();
    n = __state.c;
    while (n--) {
        if (INA_SUCCEED(ina_err_fmtmsg(__state.errors[n].rc, str, 2048))) {
            fprintf(stderr, "%s\n", str);
        } else {
            fprintf(stderr, "%s\n", "**** FATAL ERROR  ******");
            return INA_FAILURE;
        }
    }

    fprintf(stderr, "%s\n", "**** UNHANDLED ERROR END ******");

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_err_backtrace(void)
{
#ifndef INA_OS_WIN32
    void *fnptr[30];
    size_t size;
    
    fprintf(stderr, "%s\n", "**** BACKTRACE START ******");
    size = backtrace(fnptr, 30);
    char** fn = backtrace_symbols(fnptr, size);
    for (int i = 0; i < size; i++) {
        fprintf(stderr, "%s\n", fn[i]);
    }
    free(fn);
    fprintf(stderr, "%s\n", "**** BACKTRACE  END ******");
#endif
}

INA_API(ina_rc_t) ina_err_coredump(void) {
#ifndef INA_OS_WIN32
    char cmd[160];
    sprintf(cmd, "echo 'where\ndetach' | gdb -q %d > %s.dump", getpid(), "test");
    system(cmd);
#endif
    return INA_SUCCESS;
}

static ina_rc_t
__ina_init(void) 
{
    ++__initialized;
    __state.c = 0;
    __state.ic = 0;
 
    return INA_SUCCESS;
}

static size_t
__ina_get_index(ina_rc_t rc)
{
    size_t m;
    size_t k;

    k = INA_RC_ID(rc);
    m = k % __INA_ERR_STATE_SIZE;
    k = m > 0?m-1:k-1;
    return k;
}

static ina_rc_t 
__ina_pop_error(void) 
{
    size_t i;

    INA_ASSERT(__state.c >= 0);

    if (__state.c > 0) {
        for (i = 1; i < __state.c+1; ++i) {
            __state.errors[i-1] = __state.errors[i];
        }
        --__state.c;
        INA_ASSERT(__state.c >= 0);
        if (__state.c > 0) {
            return __state.errors[0].rc;
        }
    }
    return INA_SUCCESS;
}
