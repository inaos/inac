/*
 * Copyright (c) 2012-2016, INAOS GmbH
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
#ifndef _LIBINAC_CONIFG_H_
#define _LIBINAC_CONIFG_H_

/* Enabled logging */
#ifndef INA_LOG_ENABLED
#define INA_LOG_ENABLED 1
#endif

/* Define string code/library to use */
#ifndef INA_STRING_DEFINED
#define INA_CSTRING_ENABLED 1
#endif

/* Define time code/library to use */
#ifndef INA_TIME_DEFINED
#define INA_OSTIME_ENABLED 1
#endif

/* Define default sys mem pool size */
#ifndef INA_SYSMEMPOOL_SIZE
#define INA_SYSMEMPOOL_SIZE  8*1024*1024
#endif

/* Define default mem pool size */
#ifndef INA_MEMPOOL_SIZE
#define INA_MEMPOOL_SIZE  8*1024*1024
#endif

/* Define break message on assert for windows plattform */
#ifndef INA_DGBMSG_ASSERT
#define INA_DGBMSG_ASSERT 1
#endif

/* Define memory functions */
#ifndef INA_MEM_MALLOC
#define INA_MEM_MALLOC malloc
#endif
#ifndef INA_MEM_REALLOC
#define INA_MEM_REALLOC realloc
#endif
#ifndef INA_MEM_MEMMOVE
#define INA_MEM_MEMMOVE memmove
#endif
#ifndef INA_MEM_MEMCPY
#define INA_MEM_MEMCPY memcpy
#endif
#ifndef INA_MEM_MEMCMP
#define INA_MEM_MEMCMP memcmp
#endif
#ifndef INA_MEM_MEMCHR
#define INA_MEM_MEMCHR memchr
#endif
#ifndef INA_MEM_MEMSET
#define INA_MEM_MEMSET memset
#endif
#ifndef INA_MEM_FREE
#define INA_MEM_FREE free
#endif

#endif