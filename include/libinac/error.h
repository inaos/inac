/*
 * Copyright (c) 2012-2018, INAOS GmbH
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

#ifdef __cplusplus
extern "C" {
#endif

/* Indicate no errors */
#define INA_SUCCESS  0
/* Indicate generic failure */
#define INA_FAILURE  1

#define INA_ERR_AT __FILE__ ":" INA_NUM2STR(__LINE__)


/* Error message length */
#define INA_ERR_MSGLEN  512


/* OS function identifiers */
#define INA_OSFN_NONE    0
#define INA_OSFN_FOPEN   1
#define INA_OSFN_FTRUNC  2
#define INA_OSFN_MMAP    4
#define INA_OSFN_SEMINIT 5

/* Errors */
#define INA_EMSGLEN   1
#define INA_EMSGFMT   2
#define INA_EALLOC    3
#define INA_EPARAM    4
#define INA_EVERSION  5
#define INA_EBADALIGN 6
#define INA_ESEMINIT  7
#define INA_ENET      8
#define INA_ERALLOC   9
#define INA_EINVAL   10
#define INA_ELIMIT   11
#define INA_ESEMOP   12
#define INA_EEXISTS  13
#define INA_EREAD    14
#define INA_EWRITE   15
#define INA_EWAIT    16
#define INA_EEXCALL  17
#define INA_ETIMEOUT 18
#define INA_EOPT     20
#define INA_ETYPE    21
#define INA_EAGAIN   22
#define INA_EINIT    23
#define INA_ELOGIC   24
#define INA_ECAPAC   25
#define INA_EOVRFL   26
#define INA_EEMPTY   27
#define INA_ENYI     28
#define INA_ENOTFND  29
#define INA_ESTATE   30
#define INA_EFS      31

/* Mark an handled error (bit 25 of RC) */
#define INA_ERR_FLAG_HANDLED 0x1000000
/* Mark a fatal error (bit 26 of RC) */
#define INA_ERR_FLAG_FATAL   0x2000000
/* User defined errors base */
#define INA_ERR_USER          (128)

/*
 * Pack an RC.
 *
 * Parameters
 *  f  OS function identifier (if needed)
 *  r  Reason of failure
 */
#define INA_RC_PACK(f,r)  ((ina_rc_t)f) << 26U|((ina_rc_t)r) << 24U

/* Unpack the OS function identifier for a given RC */
#define INA_RC_OSFN(rc)    ((((ina_rc_t)rc)&0xFC000000U)>>26U)
/* Unpack the reason of failure for a given RC */
#define INA_RC_REASON(rc)  ((((ina_rc_t)rc)&0xFFFFFFU))
/* Verify if error is handled */
#define INA_RC_HANDLED(rc) ((ina_rc_t)(rc&INA_ERR_FLAG_HANDLED))
/* Verify if fatal error occurred */
#define INA_RC_FATAL(rc) ((ina_rc_t)(rc&INA_ERR_FLAG_FATAL))
/* Check return code if successful or handled */
#define INA_SUCCEED(rc) (INA_SUCCESS == (rc))
/* Checkpoint must succeed */
#define INA_MUST_SUCCEED(rc) if (INA_UNLIKELY(!INA_SUCCEED(rc))) abort()

/* Set error RC */
#define INA_ERR(r) ina_err_set_rc(r, INA_ERR_AT)

/* Set error RC with a custom message */
#define INA_ERRMSG(r, fmt, ...) ina_err_set_rc_msg(r, INA_ERR_AT, fmt, ##__VA_ARGS__)

/*
 * Set RC
 *
 * Parameters
 *   rc         Return code
 *   location   source location
 *
 * Return
 *   INA_SUCCESS
 */
INA_API(ina_rc_t) ina_err_set_rc(ina_rc_t rc, const char* location);

/*
 * Set error with message
 *
 * Parameters
 *   rc         Return code
 *   location   source location
 *
 * Return
 *   INA_SUCCESS
 */
INA_API(ina_rc_t) ina_err_set_rc_msg(ina_rc_t rc, const char* location, const char* fmt, ...);

/*
 * Query if succeed.
 *
 * Parameters
 *  rc  RC
 *
 * Return
 *  INA_YES if succeed otherwise INA_NO
 */
INA_API(ina_rc_t) ina_err_succeed(ina_rc_t rc);

/*
 * Mark an error as handled. All errors pushed before this one are removed
 * from the state.
 *
 * Parameters
 *  rc  Valid RC to mark as handled. If a error was already maked as handled
 *      no error occurs.
 *
 * Return
 *  Returns INA_SUCCESS when the complete error state was cleared successfully
 *  otherwise returns INA_FAILURE. A marked
 */
INA_API(ina_rc_t) ina_err_clear(ina_rc_t rc);

/*
 * Mark an error as handled.
 *
 * Return
 *  Returns INA_SUCCESS when the complete error state was cleared successfully
 *  otherwise returns INA_FAILURE
 */
INA_API(ina_rc_t) ina_err_reset(void);

/*
 * Makes a backrace to the stderr of the current error state.
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_err_backtrace(void *data);

/*
 * Create a core dump
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_err_coredump(void *data);

/*
 * Format the error message for a given RC.
 *
 * Parameters
 *  rc   Valid RC
 *  str  String buffer to hold the message
 *  len  Max length of the string buffer str
 *
 * Return
 *  INA_SUCCESS if successful, INA_FAILURE if an invalid RC was passed
 */
INA_API(ina_rc_t) ina_err_fmtmsg(ina_rc_t rc, ina_str_t str, size_t len);

/*
 * Return the  error message for the last RC.
 * 
 * Return
 *  Error message or NULL if no errors are occurred.
 */

INA_API(const char*) ina_err_get_last_msg(void);

/*
 * Return the last RC
 *
 * Return
 *  Las RC or INA_SUCCESS of no error occurred
 */
INA_API(ina_rc_t) ina_err_get_last_rc(void);

/*
 * Return the last error code
 *
 * Return
 *  Error code or 0 if no error occurred
 */
INA_API(int) ina_err_get_last_error(void);

/*
 * Return the raw error message for an error rc.
 *
 * Parameters
 *  rc  Valid RC
 *
 * Return
 *  Error message or NULL if rc is invalid.
 */
INA_API(const char*) ina_err_get_msg(ina_rc_t rc);

#ifdef __cplusplus
}
#endif

#endif
