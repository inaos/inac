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
#ifndef _LIBINAC_DEBUG_H_
#define _LIBINAC_DEBUG_H_

#include <assert.h>


#ifdef __cplusplus
extern "C" {
#endif

#ifndef INA_TRACE_LEVEL
#define INA_TRACE_LEVEL 1
#endif

/*
 * Trace macros
 *
 */
#ifdef DEBUG
#define INA_TRACE(fmt, ...)  \
    fprintf(stderr,            \
        "%s:%d:%s(): " fmt "\n",\
        __FILE__,             \
        __LINE__,             \
        __FUNCTION__,         \
        __VA_ARGS__           \
        )
#if INA_TRACE_LEVEL>0
#define INA_TRACE1(fmt, ...)  INA_TRACE(fmt, __VA_ARGS__)
#else
#define INA_TRACE1(fmt, ...)
#endif
#if INA_TRACE_LEVEL>1
#define INA_TRACE2(fmt, ...)  INA_TRACE(fmt, __VA_ARGS__)
#else 
#define INA_TRACE2(fmt, ...)
#endif
#if INA_TRACE_LEVEL>2
#define INA_TRACE3(fmt, ...)  INA_TRACE(fmt, __VA_ARGS__)
#else
#define INA_TRACE3(fmt, ...)
#endif
#define INA_TRACE_MSG(msg) INA_TRACE("%s", msg)
#else
#define INA_TRACE(fmt, ...)
#define INA_TRACE1(fmt, ...)
#define INA_TRACE2(fmt, ...)
#define INA_TRACE3(fmt, ...)
#define INA_TRACE_MSG(msg)
#endif 

/*
 * Test macros
 */

#define INA_TEST_RUN(name, pattern) if (strstr(name, pattern)) #name()

#define INA_TEST_SPAWN_BEGIN()      \
 {                                  \
     int _spawn = 0, FILE _fp = 0;

#define INA_TEST_SPAWN_CODE_BEGIN()  \
   if (_spawn) {

#define INA_TEST_SPAWN_CODE_END()    \
   }

#define INA_TEST_SPAWN_END()         \
   fclose(_fp);                      \
   if (_spawn) exit(EXIT_SUCCESS);   \
   }
#define INA_TEST_CMD(cmd) system(cmd)


#ifdef DEBUG
#define INA_NOT_IMPL assert(0)
#define INA_ASSERT(cond) assert(cond)
#define INA_ASSERT_FALSE(v) INA_ASSERT(!v)
#define INA_ASSERT_TRUE(v) INA_ASSERT(v)
#define INA_ASSERT_NULL(v) INA_ASSERT(v == NULL)
#define INA_ASSERT_NOTNULL(v) INA_ASSERT(v != NULL)
#define INA_ASSERT_EQUAL(expected, actual) INA_ASSERT(expected == actual)
#define INA_ASSERT_NOTEQUAL(nexpected, actual) INA_ASSERT(nexpected != actual)
#define INA_ASSERT_SUCCESS(v) INA_ASSERT_EQUAL(INA_SUCCESS, v)
#define INA_ASSERT_FAILURE(v) INA_ASSERT_EQUAL(INA_FAILURE, v)
#define INA_ASSERT_SUCCEED(v) INA_ASSERT_TRUE(INA_SUCCEED(v))
#define INA_ASSERT_NOTSUCCEED(v) INA_ASSERT_FALSE(INA_SUCCEED(v))
#else
#define INA_NOT_IMPL INA_CASSERT(Not_implemented,0)
#define INA_ASSERT(cond)
#define INA_ASSERT_FALSE(v)
#define INA_ASSERT_TRUE(v)
#define INA_ASSERT_NULL(v)
#define INA_ASSERT_NOTNULL(v)
#define INA_ASSERT_EQUAL(expected, actual)
#define INA_ASSERT_NOEQUAL(notexpected, actual)
#define INA_ASSERT_SUCCESS(v)
#define INA_ASSERT_FAILURE(v) 
#define INA_ASSERT_SUCCEED(v)
#define INA_ASSERT_NOTSUCCEED(v)
#endif

#ifdef __cplusplus
}
#endif 

#endif