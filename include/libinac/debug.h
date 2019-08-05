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
         const char *e = getenv("INAC_TRACE"); \
         const char *s = #cat; \
         size_t i = 0, w = 0, c = 0, match = 0, el = 0; \
         const int always = (strcmp(s, "*") == 0); \
         if (!always && (!e || !strlen(e))) break; \
         match = 1; \
         el = strlen(e); \
         while (i < strlen(e) && !always) { \
             if (e[i] == '*') { w = 1; if (match) { el = i+1; break;} }    \
             if (e[i] == ',')  {w=0;el=i; if (match) break; c = 0; ++i; match=1;continue; }   \
             if (c == strlen(#cat)) c = 0; \
             if (e[i] != s[c]) { match=0; } \
             ++c; ++i; \
         } \
         if (!match || (!always && !w && el < strlen(s))) break; \
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