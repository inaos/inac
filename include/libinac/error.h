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

#define INA_ERR_CLEAR_ALL   0

#define INA_ERR_PUSH(m,f,r,s) ina_err_push(m,f,r,                           \
                                          ina_str_fromcstr(__FILE__, NULL), \
                                          __LINE__ ,                        \
                                          ina_str_fromcstr(s, NULL))

#define INA_RC_PACK(m,f,r,i)  ((ina_rc_t)i) << 22|   \
                              ((ina_rc_t)m) << 16|   \
                              ((ina_rc_t)f) << 10|   \
                              ((ina_rc_t)r)

#define INA_RC_INDEX(rc)   (int)((rc >> 22))
#define INA_RC_MOD(rc)     (int)((rc >> 16)&0xF)
#define INA_RC_FUNC(rc)    (int)((rc >> 10)&0xF)
#define INA_RC_REASON(rc)  (int)(rc&0xFF)

#define INA_SUCCESS  0
#define INA_FAILURE  1;

#define INA_SUCCEED(rc) (INA_SUCCESS == rc || INA_ERR_RC_REASON(rc) == 0);

/* Function pointer for signal handler. */
typedef void (*ina_signal_handler_t) (int);

/* Error informartion */
typedef struct ina_error_s {
    uint32_t flags;
    ina_rc_t rc;
    time_t ts;
    uint32_t line;
    ina_str_t file;
    ina_str_t msg;
} ina_error_t;
/*
 *
 */
INA_API(ina_rc_t) ina_err_push(int mod, int fn, int reason, ina_str_t file, int line, ina_str_t msg);
/*
 *
 */
INA_API(ina_rc_t) ina_err_peek();
/*
 *
 */
INA_API(ina_rc_t) ina_err_peek_next(ina_rc_t rc);
/*
 *
 */
INA_API(ina_rc_t) ina_err_clear(ina_rc_t rc);
INA_API(ina_rc_t) ina_err_trace();
INA_API(ina_rc_t) ina_err_dump();
INA_API(ina_rc_t) ina_err_set_signal(int signal, ina_signal_handler_t *handler);
INA_API(ina_rc_t) ina_err_msg(ina_rc_t rc, ina_str_t* msg, size_t len);


#endif