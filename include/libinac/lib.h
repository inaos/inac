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
#ifndef _LIBINAC_LIB_H_
#define _LIBINAC_LIB_H_

#ifndef _WIN32
#include <sys/types.h>
#include <sys/param.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/sem.h>
#endif

#ifndef _WIN32
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <libinac/portable.h>
#include <libinac/types.h>
#include <libinac/memory.h>
#include <libinac/string.h>
#include <libinac/error.h>
#include <libinac/ullc.h>
#include <libinac/iscp.h>
#include <libinac/util.h>
#include <libinac/debug.h>

#define INA_YES 1
#define INA_NO  0
/*
 * Version
 */
#define INA_MAJOR_VERSION 0
#define INA_MINOR_VERSION 1
#define INA_MICRO_VERSION 0

#define INA_VERSION       "0.1.0"

/* Version as a 3-byte hex number, e.g. 0x010201 == 1.2.1. Use this
 * for numeric comparisons, e.g. #if INA_VERSION_HEX >= ... */
#define INA_VERSION_HEX  ((INA_MAJOR_VERSION << 16) |   \
                          (INA_MINOR_VERSION << 8)  |   \
                          (INA_MICRO_VERSION << 0))

/*
 * Startup application with argc, argv in order to deal with 
 * platform-specific quirks. This must be the first function called for any
 * program.
 *
 * Parameters:
 *  argc  -  argc of main() function
 *  argv  -  Pointer to the argv of main() function
 *
 * Return:
 * INA_SUCCESS  if no error occured
 */
INA_API(ina_rc_t) ina_appinit(const int argc,  char **argv);

/*
 * Initialize all internal data structures. This must be the first function 
 * called for any library.
 *
 * Return:
 * INA_SUCCESS  if no error occured
 */
INA_API(ina_rc_t) ina_libinit(void);

/*
 * Relase and cleanup all internal data structures. This function must be
 * called once before the application terminate.
 */
INA_API(void) ina_exit(void);

#endif
