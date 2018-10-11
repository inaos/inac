/*
 * Copyright INAOS GmbH, Thalwil, 2012-2018. All rights reserved
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

#ifndef INA_TRACE_LEVEL
#define INA_TRACE_LEVEL 1
#endif

/*
 * Trace macros
 */
#ifdef INA_DEBUG
#define INA_TRACE(fmt, ...)     \
    fprintf(stderr,             \
        "%s:%d:%s(): " fmt "\n",\
        __FILE__,               \
        __LINE__,               \
        __FUNCTION__,           \
        ##__VA_ARGS__           \
        )
#if INA_TRACE_LEVEL>0
#define INA_TRACE1(fmt, ...)  INA_TRACE(fmt, ##__VA_ARGS__)
#else
#define INA_TRACE1(fmt, ...)
#endif
#if INA_TRACE_LEVEL>1
#define INA_TRACE2(fmt, ...)  INA_TRACE(fmt, ##__VA_ARGS__)
#else 
#define INA_TRACE2(fmt, ...)
#endif
#if INA_TRACE_LEVEL>2
#define INA_TRACE3(fmt, ...)  INA_TRACE(fmt, ##__VA_ARGS__)
#else
#define INA_TRACE3(fmt, ...)
#endif
#else
#define INA_TRACE(fmt, ...)
#define INA_TRACE1(fmt, ...)
#define INA_TRACE2(fmt, ...)
#define INA_TRACE3(fmt, ...)
#endif 


#ifdef INA_DEBUG
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