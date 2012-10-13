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
/* Indicate genercic failure */
#define INA_FAILURE  1

/* Module identifiers */
#define INA_MOD_UNKNOWN 0
#define INA_MOD_MEMORY  1
#define INA_MOD_STRING  2

/* OS function identifiers */
#define INA_OSFN_NONE   0
#define INA_OSFN_FOPEN  1


/*
 * Used to reset the error state.
 */
#define INA_ERR_CLEAR_ALL   0

/*
 * Push an error to the error state. 
 * 
 * Parameters
 * m    Module identifier (optional)
 * f    OS function identifier (if needed)
 * r    Reason of failure
 * s    Error message
 */
#define INA_ERR_PUSH(m,f,r,s) ina_err_push(m,f,r,                           \
                                          ina_str_fromcstr(__FILE__, NULL), \
                                          __LINE__ ,                        \
                                          ina_str_fromcstr(s, NULL))

/*
 * Push an error to the error state by passing only basic informations like
 * reason of failure and message
 * 
 * Parameters
 * m    Module identifier (optional)
 * s    Error message
 */                                          
#define INA_ERR_PUSH_BASIC(r,s) ina_err_push(INA_MOD_UNKNOWN,              \
                                          INA_OSFN_NONE,                    \
                                          r,                                \
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
#define INA_RC_PACK(m,f,r,i)  ((ina_rc_t)i) << 22|   \
                              ((ina_rc_t)m) << 16|   \
                              ((ina_rc_t)f) << 10|   \
                              ((ina_rc_t)r)

/* Unpack the error identifier for a given RC */
#define INA_RC_ID(rc)      (int)((rc >> 22))
/* Unpack the module indentifier for a given RC */
#define INA_RC_MOD(rc)     (int)((rc >> 16)&0xF)
/* Unpack the OS function identifier for a given RC */
#define INA_RC_OSFN(rc)    (int)((rc >> 10)&0xF)
/* Unpack the reason of failuer for a given RC */
#define INA_RC_REASON(rc)  (int)(rc&0xFF)

#define INA_SUCCEED(rc) (INA_SUCCESS == rc || INA_RC_REASON(rc) == 0);

/* Function pointer for signal handler. */
typedef void (*ina_signal_handler_t) (int);

/* Error information */
typedef struct ina_error_s {
    ina_rc_t rc; 
    time_t ts;  /* FIXME: we should use our proper time value */
    uint32_t line;
    ina_str_t file;
    ina_str_t msg;
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
 * Peek the first unhandled error from the error state.
 *
 * Return Value
 * RC of first unhandled error or INA_SUCCESS if no unhandled errors found 
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
 * Mark an error as handled or clear the entire error state.
 *
 * Parameters
 * rc   Valid RC to mark as handled. If a error is already maked as handled
 *      no error occurs. To clear the complete error state pass INA_SUCCESS
 *      to the function.
 *
 * Return Value
 * RC. 
 * Returns INA_SUCCESS when the complete error state was cleard successfully
 * ohterwise returns INA_FAILURE
 */
INA_API(ina_rc_t) ina_err_clear(ina_rc_t rc);

/*
 * Makes a nice trace to the stdout of the current error state.
 *
 * Return Value
 * INA_SUCCESS
 */
INA_API(ina_rc_t) ina_err_trace(void);

/*
 * Printout a core dump to the stdout.
 *
 * Return Value
 * INA_SUCCESS
 */
INA_API(ina_rc_t) ina_err_dump(void);

/*
 * Set a custom handler for a given signal.
 *
 * Parameters
 * signal   Signal to be handled
 * handler  Function which handle the signal
 *
 * Return Value
 * INA_SUCCESS
 */
INA_API(ina_rc_t) ina_err_set_signal(int signal, ina_signal_handler_t *handler);

/*
 * Format the error message for a given RC.
 *
 * Parameters
 * rc   Valid RC
 * str  String buffer for the message
 * len  Max length of the string buffer str
 *
 * Return Value
 * INA_SUCCESS if successful, INA_FAILURE if an inablid RC was passed
 */
INA_API(ina_rc_t) ina_err_msg(ina_rc_t rc, ina_str_t* str, size_t len);


#endif