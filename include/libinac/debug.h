/*
 * Copyright INAOS GmbH, Thalwil, 2012-2019. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#ifndef _LIBINAC_DEBUG_H_
#define _LIBINAC_DEBUG_H_

#include <assert.h>


#ifdef __cplusplus
extern "C" {
#endif

#if !defined(INA_TRACE_ENABLE) && defined(INA_DEBUG)
#define INA_TRACE_ENABLED 1
#endif

#ifndef INA_TRACE_LEVEL
#define INA_TRACE_LEVEL 1
#endif

#ifndef INA_TRACE_TARGET
#define INA_TRACE_TARGET stdout
#endif

/*
 * Trace macros
 */
#ifdef INA_TRACE_ENABLED
#define INA_TRACE_TO_FILE(fh, cat, fmt, ...)     \
    do { \
         const char *__e = getenv("INAC_TRACE"); \
         const char *__s = #cat; \
         size_t __i = 0, __w = 0, __c = 0, __match = 0, __el = 0; \
         const int __always = (strcmp(__s, "*") == 0); \
         if (!__always && (!__e || !strlen(__e))) break; \
         __match = 1; \
         __el = strlen(__e); \
         while (__i < strlen(__e) && !__always) { \
             if (__e[__i] == '*') { __w = 1; if (__match) { __el = __i+1; break;} }    \
             if (__e[__i] == ',')  {__w=0;__el=__i; if (__match) break; __c = 0; ++__i; __match=1;continue; }   \
             if (__c == strlen(#cat)) __c = 0; \
             if (__e[__i] != __s[__c]) { __match=0; } \
             ++__c; ++__i; \
         } \
         if (!__match || (!__always && !__w && __el < strlen(__s))) break; \
         fprintf(fh,                \
            "[%s] - " fmt "\n", \
            #cat, \
            ##__VA_ARGS__ \
        );} while(0)

#define INA_TRACE(cat, fmt, ...) INA_TRACE_TO_FILE(INA_TRACE_TARGET, cat, fmt, ##__VA_ARGS__)

#if INA_TRACE_LEVEL>0
#define INA_TRACE1(cat, fmt, ...)  INA_TRACE(cat, fmt, ##__VA_ARGS__)
#define INA_TRACE1_TO_FILE(fh, cat, fmt, ...)  INA_TRACE_TO_FILE(fh, cat, fmt, ##__VA_ARGS__)
#else
#define INA_TRACE1(cat, fmt, ...)
#define INA_TRACE1_TO_FILE(fh, cat, fmt, ...)
#endif
#if INA_TRACE_LEVEL>1
#define INA_TRACE2(cat, fmt, ...)  INA_TRACE(cat, fmt, ##__VA_ARGS__)
#define INA_TRACE2_TO_FILE(fh, cat, fmt, ...)  INA_TRACE_TO_FILE(fh, cat, fmt, ##__VA_ARGS__)
#else 
#define INA_TRACE2(cat, fmt, ...)
#define INA_TRACE2_TO_FILE(fh, cat, fmt, ...)
#endif
#if INA_TRACE_LEVEL>2
#define INA_TRACE3(cat, fmt, ...)  INA_TRACE(cat, fmt, ##__VA_ARGS__)
#define INA_TRACE3_TO_FILE(fh, cat, fmt, ...)  INA_TRACE_TO_FILE(fh, cat, fmt, ##__VA_ARGS__)
#else
#define INA_TRACE3(cat, fmt, ...)
#define INA_TRACE3_TO_FILE(fh, cat, fmt, ...)
#endif
#else
#define INA_TRACE(cat, fmt, ...)
#define INA_TRACE1(cat, fmt, ...)
#define INA_TRACE2(cat, fmt, ...)
#define INA_TRACE3(cat, fmt, ...)
#define INA_TRACE_TO_FILE(fh, cat, fmt, ...)
#define INA_TRACE1_TO_FILE(fh, cat, fmt, ...)
#define INA_TRACE2_TO_FILE(fh, cat, fmt, ...)
#define INA_TRACE3_TO_FILE(fh,cat, fmt, ...)
#endif

#if !defined(INA_USE_ASSERTS) && defined(INA_DEBUG)
#define INA_USE_ASSERTS 1
#endif

#ifdef INA_USE_ASSERTS
#define INA_USED_BY_ASSERT(x) 
#define INA_NOT_IMPL assert(0)
#define INA_ASSERT(cond) assert(cond)
#define INA_ASSERT_FALSE(v) INA_ASSERT(!(v))
#define INA_ASSERT_TRUE(v) INA_ASSERT(v)
#define INA_ASSERT_NULL(v) INA_ASSERT(v == NULL)
#define INA_ASSERT_NOT_NULL(v) INA_ASSERT(v != NULL)
#define INA_ASSERT_EQUAL(expected, actual) INA_ASSERT(expected == actual)
#define INA_ASSERT_NOTEQUAL(nexpected, actual) INA_ASSERT(nexpected != actual)
#define INA_ASSERT_SUCCESS(v) INA_ASSERT_EQUAL(INA_SUCCESS, v)
#define INA_ASSERT_FAILURE(v) INA_ASSERT_EQUAL(INA_FAILURE, v)
#define INA_ASSERT_SUCCEED(v) INA_ASSERT_TRUE(INA_SUCCEED(v))
#define INA_ASSERT_NOTSUCCEED(v) INA_ASSERT_FALSE(INA_SUCCEED(v))
#else
#define INA_USED_BY_ASSERT(x) INA_UNUSED(x)
#define INA_NOT_IMPL INA_CASSERT(Not_implemented,0)
#define INA_ASSERT(cond)
#define INA_ASSERT_FALSE(v)
#define INA_ASSERT_TRUE(v)
#define INA_ASSERT_NULL(v)
#ifdef INA_ASSERT_NOT_NULL_ENABLED
#define INA_ASSERT_NOT_NULL(v) assert(v != NULL)
#else
#define INA_ASSERT_NOT_NULL(v)
#endif    
#define INA_ASSERT_EQUAL(expected, actual)
#define INA_ASSERT_NOTEQUAL(notexpected, actual)
#define INA_ASSERT_SUCCESS(v)
#define INA_ASSERT_FAILURE(v) 
#define INA_ASSERT_SUCCEED(v)
#define INA_ASSERT_NOTSUCCEED(v)
#endif

#ifdef __cplusplus
}
#endif 

#endif