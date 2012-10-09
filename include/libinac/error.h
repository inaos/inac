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

#include <errno.h>
#include <libinac/lib.h>

#define INA_ERR_STATE_SIZE 32

#define INA_ERROR(e,r, __FILE__, __LINE) ina_err_put(e, r, s)

typedef struct ina_error_s ina_error_t;  

/*
 * Error Codes
 */
#define INA_SUCCESS   0
#define INA_FAILURE  -1


/* set/get rc */
INA_API(ina_rc_t) ina_err_put(ina_rc_t rc);
INA_API(ina_rc_t) ina_err_peek();
INA_API(ina_rc_t) ina_err_getinfo(ina_error_t *info);
INA_API(ina_rc_t) ina_err_clear();

INA_API(ina_rc_t) ina_err_trace();
INA_API(ina_rc_t) ina_err_dump();

/* get human readable error msg */
INA_API(ina_rc_t) ina_err_msg(ina_rc_t rc, ina_str_t* msg, size_t len);


#endif