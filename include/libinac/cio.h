/*
 * Copyright (c) 2013, INAOS GmbH
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
#ifndef _LIBINAC_CIO_H_
#define _LIBINAC_CIO_H_

#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ANSI color codes */
#define INA_CIO_ANSI_BLACK    "\033[0;30m"
#define INA_CIO_ANSI_RED      "\033[0;31m"
#define INA_CIO_ANSI_GREEN    "\033[0;32m"
#define INA_CIO_ANSI_YELLOW   "\033[0;33m"
#define INA_CIO_ANSI_BLUE     "\033[0;34m"
#define INA_CIO_ANSI_MAGENTA  "\033[0;35m"
#define INA_CIO_ANSI_CYAN     "\033[0;36m"
#define INA_CIO_ANSI_GREY     "\033[0;37m"
#define INA_CIO_ANSI_DARKGREY "\033[01;30m"
#define INA_CIO_ANSI_BRED     "\033[01;31m"
#define INA_CIO_ANSI_BGREEN   "\033[01;32m"
#define INA_CIO_ANSI_BYELLOW  "\033[01;33m"
#define INA_CIO_ANSI_BBLUE    "\033[01;34m"
#define INA_CIO_ANSI_BMAGENTA "\033[01;35m"
#define INA_CIO_ANSI_BCYAN    "\033[01;36m"
#define INA_CIO_ANSI_WHITE    "\033[01;37m"
#define INA_CIO_ANSI_NORMAL   "\033[0m"


/*
 * Print text
 */
INA_API(ina_rc_t) ina_cio_print(const char *clr, const char* text);


#ifdef __cplusplus
}
#endif
#endif