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
#ifndef _LIBINAC_ERROR_H_
#define _LIBINAC_ERROR_H_

#include <time.h>
#include <errno.h>

#include <libinac/lib.h>

/* Indicate no errors */
#define INA_SUCCESS  0
/* Indicate generic failure */
#define INA_FAILURE  1

/* Module identifiers */
#define INA_MOD_UNKNOWN 0
#define INA_MOD_MEMORY  1
#define INA_MOD_STRING  2
#define INA_MOD_ERROR   3

/* OS function identifiers */
#define INA_OSFN_NONE   0
#define INA_OSFN_FOPEN  1

/* Errors */
#define INA_EMSGLEN 1
#define INA_EMSGFMT 2
#define INA_EALLOC  (3)
#define INA_EPARAM  (4)

/* Mark an handled error (bit 10 of RC) */
#define INA_ERR_FLAG_HANDLED 0x200
/* Mark a fatal error (bit 11 of RC) */
#define INA_ERR_FLAG_FATAL   0x300
/* Used to start an interation  */
#define INA_ERR_PEEK_FIRST    0
/*
 * Push an error to the error state. 
 * 
 * Parameters
 * m    Module identifier (optional)
 * f    OS function identifier (if needed)
 * r    Reason of failure
 * s    Error message
 */
#define INA_ERR_PUSH(r,m,f,s) ina_err_push(m,f,r,                           \
                                          ina_str_fromcstr(__FILE__, NULL), \
                                          __LINE__ ,                        \
                                          ina_str_fromcstr(s, NULL))

/*
 * Push an error to the error state by passing only basic informations like
 * reason of failure and message
 * 
 * Parameters
 * r    Reason of failure
 * s    Error message
 */
#define INA_ERR_PUSH_BASIC(r,s) ina_err_push(INA_MOD_UNKNOWN,               \
                                          INA_OSFN_NONE,                    \
                                          r,                                \
                                          ina_str_fromcstr(__FILE__, NULL), \
                                          __LINE__ ,                        \
                                          ina_str_fromcstr(s, NULL))

/*
 * Push an error to the error state by passing  basic informations like
 * reason of failure, os function indentifier and message 
 * 
 * Parameters
 * r    Reason of failure
 * f    OS function identifier
 * s    Error message
 */ 
#define INA_ERR_PUSH_OSFN(r,f,s) ina_err_push(INA_MOD_UNKNOWN,              \
                                          f,r,                              \
                                          ina_str_fromcstr(__FILE__, NULL), \
                                          __LINE__ ,                        \
                                          ina_str_fromcstr(s, NULL))

/*
 * Pack an RC. 
 * 
 * Parameters
 * m    Module identifier (optional)
 * f    OS function identifier (if needed)
 * r    Reason of failure
 * i    Error identifier
 */
#define INA_RC_PACK(m,f,r,i)  ((ina_rc_t)i) << 22U|   \
                              ((ina_rc_t)m) << 16U|   \
                              ((ina_rc_t)f) << 11U|   \
                              ((ina_rc_t)r)

/* Unpack the error identifier for a given RC */
#define INA_RC_ID(rc)     ((((ina_rc_t)rc)&0xFFC00000U)>>22U)
/* Unpack the module indentifier for a given RC */
#define INA_RC_MOD(rc)    ((((ina_rc_t)rc)&0x3F0000U)>>16U)
/* Unpack the OS function identifier for a given RC */
#define INA_RC_OSFN(rc)    ((((ina_rc_t)rc)&0xF800U)>>11U)
/* Unpack the reason of failuer for a given RC */
#define INA_RC_REASON(rc)  ((((ina_rc_t)rc)&0x1FF))
/* Verify if error is handled */
#define INA_RC_HANDLED(rc) ((ina_rc_t)(rc&INA_ERR_FLAG_HANDLED))
/* Verify if fatal error occurred */
#define INA_RC_FATAL(rc) ((ina_rc_t)(rc&INA_ERR_FLAG_FATAL))
/* Check retuen code if successful or handled */
#define INA_SUCCEED(rc) (INA_SUCCESS == rc ||       \
                         INA_RC_REASON(rc) == 0 ||  \
                         INA_RC_HANDLED(rc))

/* Error-Module errors */
#define INA_ERR_ERROR(r,s) INA_ERR_PUSH(r, INA_MOD_ERROR,INA_OSFN_NONE, s)
#define INA_ERR_EMSGLEN INA_ERR_ERROR(INA_EMSGLEN, "Message size")
#define INA_ERR_EMSGFMT INA_ERR_ERROR(INA_EMSGFMT, "Message format")

/* String-Module errors */
#define INA_STR_ERROR(r,s) INA_ERR_PUSH(r, INA_MOD_STRING,INA_OSFN_NONE, s)
#define INA_STR_EALLOC INA_STR_ERROR(INA_EALLOC, "Bad string alloc")

/* String-Module errors */
#define INA_MEM_ERROR(r,s) INA_ERR_PUSH(r, INA_MOD_MEMORY,INA_OSFN_NONE, s)
#define INA_MEM_EALLOC INA_MEM_ERROR(INA_EALLOC, "Bad memory alloc")


/* Function pointer cleanup handler. */
typedef int (*ina_cleanup_handler_t) (const int, const int);

/* Error information */
typedef struct ina_error_s {
    ina_rc_t rc; 
    time_t ts;  /* FIXME: we should use our proper time value */
    uint32_t line;
    ina_str_t file;
    ina_str_t msg;
    void *data;
} ina_error_t;

/*
 * Push an error to the error state.
 * 
 * Parameters
 * mod      Module identifier
 * osfn     OS function intentifier
 * reason   Reason of failure
 * file     filename
 * line     line
 * msg      Error message
 *
 * Return Value
 * RC
 */
INA_API(ina_rc_t) ina_err_push(int mod, int osfn, int reason, ina_str_t file, 
                               int line, 
                               ina_str_t msg);

/*
 * Peek the first pushed error from the error state.
 *
 * Return Value
 * RC of first pushed error or INA_SUCCESS if error state is clean 
 */
INA_API(ina_rc_t) ina_err_peek_last(void);

/*
 * Peek the first unhandled error from the error state.
 *
 * Return Value
 * RC of first unhandled error or INA_SUCCESS  if error state is clean 
 */
INA_API(ina_rc_t) ina_err_peek(void);

/*
 * Peek the next (handled or unhandled) error from the error state.
 *
 * Parameters
 * rc   Previous RC
 *
 * Return Value
 * RC or INA_SUCCESS if no more errors found.
 */
INA_API(ina_rc_t) ina_err_peek_next(ina_rc_t rc);

/*
 * Mark an error as handled. All errors pushed before this one are removed
 * from the state.
 *
 * Parameters
 * rc   Valid RC to mark as handled. If a error was already maked as handled
 *      no error occurs.
 *
 * Return Value
 * Returns INA_SUCCESS when the complete error state was cleared successfully
 * otherwise returns INA_FAILURE. A marked 
 */
INA_API(ina_rc_t) ina_err_clear(ina_rc_t rc);

/*
 * Mark an error as handled.
 *
 * Return Value
 * Returns INA_SUCCESS when the complete error state was cleared successfully
 * otherwise returns INA_FAILURE
 */
INA_API(ina_rc_t) ina_err_reset(void);

/*
 * Makes a trace to the stdout of the current error state.
 *
 * Return Value
 * INA_SUCCESS
 */
INA_API(ina_rc_t) ina_err_trace(void);

/*
 * Set a custom cleanup routine to call in case of an programm error or
 * a terminiation signal. The purpose of such a routine is to give consumers
 * a last chance to cleanup before the program exits.
 *
 * Parameters
 * handler  Cleanup routine. A cleanup should return EXIT_SUCCESS or 
 *          EXIT_FAILURE depending on type of signal. On a programm error
 *          the return of cleanup routines will be ignored. 
 *
 * Return Value
 * Previously defined handler
 */
INA_API(ina_cleanup_handler_t) ina_err_set_cleanup_handler(
                                        ina_cleanup_handler_t handler);

/*
 * Format the error message for a given RC.
 *
 * Parameters
 * rc   Valid RC
 * str  String buffer to hold the message
 * len  Max length of the string buffer str
 *
 * Return Value
 * INA_SUCCESS if successful, INA_FAILURE if an invalid RC was passed
 */
INA_API(ina_rc_t) ina_err_fmtmsg(ina_rc_t rc, ina_str_t str, size_t len);
#endif