/*
 * Copyright (c) 2012-2013, INAOS GmbH
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
#include <stdio.h>
#include <libinac/lib.h>

/* Align to 2x word size (as GNU libc does). */
#define __INA_ALIGN_SIZE (2 * sizeof(void*))

/* Round up 'n' to a multiple of ALIGN_SIZE. */
#define __INA_MEM_ALIGN(n) ((n+(__INA_ALIGN_SIZE-1)) & (~(__INA_ALIGN_SIZE-1)))

INA_TEST(mempool, fill_zero)
{
    ina_mempool_t *pool;
    unsigned char *buf;
    size_t size = INA_MEM_MIN_POOL_SIZE-100;

    /* clear error state and assure it's clean */
    INA_TEST_ASSERT_SUCCESS(ina_err_reset());
    INA_TEST_ASSERT_SUCCESS(ina_err_peek());

    pool = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_mempool_create(&pool, INA_MEM_MIN_POOL_SIZE, 0, NULL));
    INA_TEST_ASSERT_NOT_NULL(pool);
    buf = (unsigned char*)ina_mempool_dalloc(pool, size);
    INA_TEST_ASSERT_NOT_NULL(pool);
    while (size--) {
        INA_TEST_ASSERT_EQUAL(*(buf++), 0);
    }
}
INA_TEST(mempool, min_allowed_size)
{
    ina_mempool_t *pool;
    ina_mempool_info_t mi;

    /* clear error state and assure it's clean */
    INA_TEST_ASSERT_SUCCESS(ina_err_reset());
    INA_TEST_ASSERT_SUCCESS(ina_err_peek());
    
    pool = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_mempool_create(&pool, INA_MEM_MIN_POOL_SIZE-100, 0, NULL));
    INA_TEST_ASSERT_NOT_NULL(pool);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_getinfo(pool, &mi));
    INA_TEST_ASSERT_EQUAL(INA_MEM_MIN_POOL_SIZE, mi.size);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_create(&pool, INA_MEM_MIN_POOL_SIZE, 0, NULL));
    INA_TEST_ASSERT_NOT_NULL(pool);
}

INA_TEST(test, mempool_bad_dalloc)
{
    void *ptr;
    ina_mempool_t *pool;

    ptr = NULL;
    pool = NULL;

    /* clear error state and assure it's clean */
    INA_TEST_ASSERT_SUCCESS(ina_err_reset());
    INA_TEST_ASSERT_SUCCESS(ina_err_peek());

    /* create a fixed size pool of 1KB and try to allocate 2KB */
    INA_TEST_ASSERT_SUCCESS(ina_mempool_init(0));
    INA_TEST_ASSERT_SUCCESS(ina_mempool_create(&pool, 1024, 0, NULL));
    INA_TEST_ASSERT_NOT_NULL(pool);
    ptr = ina_mempool_dalloc(pool, 2048);
    INA_TEST_ASSERT_NULL(ptr);
    INA_TEST_ASSERT_FALSE(INA_SUCCEED(ina_err_peek()));
    INA_TEST_ASSERT_EQUAL(INA_EALLOC , INA_RC_REASON(ina_err_peek()));
}

INA_TEST(mempool, destroy_syspool_1000_times)
{
    size_t i;
    ina_mempool_info_t mi;

    /* clear error state and assure it's clean */
    INA_TEST_ASSERT_SUCCESS(ina_err_reset());
    INA_TEST_ASSERT_SUCCESS(ina_err_peek());
    
    for (i = 0; i < 1000; ++i) {
        INA_TEST_ASSERT_SUCCESS(ina_mempool_destroy());
        INA_TEST_ASSERT_NOTSUCCEED(ina_mempool_getinfo(NULL, &mi));
    }
}

INA_TEST(mempool,destroy_syspool_1000_times_and_recreate)
{
    size_t i;
    ina_mempool_info_t mi;
    
    /* clear error state and assure it's clean */
    INA_TEST_ASSERT_SUCCESS(ina_err_reset());
    INA_TEST_ASSERT_SUCCESS(ina_err_peek());
    
    for (i = 0; i < 1000; ++i) {
        INA_TEST_ASSERT_SUCCESS(ina_mempool_destroy());
        INA_TEST_ASSERT_SUCCESS(ina_mempool_init(0));
        INA_TEST_ASSERT_SUCCESS(ina_mempool_getinfo(NULL, &mi));
        INA_TEST_ASSERT_EQUAL(0, mi.children);
        INA_TEST_ASSERT_EQUAL(8*1024*1024, mi.size);
    }
}

INA_TEST(mempool,syspool) 
{
    void *p;
    ina_mempool_info_t mi;
    
    /* clear error state and assure it's clean */
    INA_TEST_ASSERT_SUCCESS(ina_err_reset());
    INA_TEST_ASSERT_SUCCESS(ina_err_peek());

    /* destroy all pools and recreate internal pool with default size */
    INA_TEST_ASSERT_SUCCESS(ina_mempool_destroy());
    INA_TEST_ASSERT_SUCCESS(ina_mempool_init(0));
    INA_TEST_ASSERT_SUCCESS(ina_mempool_getinfo(NULL, &mi));
    INA_TEST_ASSERT_EQUAL(0, mi.children);
    INA_TEST_ASSERT_EQUAL(8*1024*1024, mi.size);

    /* destroy all pools and recreate internal 10MB pool */
    INA_TEST_ASSERT_SUCCESS(ina_mempool_destroy());
    INA_TEST_ASSERT_SUCCESS(ina_mempool_init(10*1024*1024));
    INA_TEST_ASSERT_SUCCESS(ina_mempool_getinfo(NULL, &mi));
    INA_TEST_ASSERT_EQUAL(0, mi.children);
    INA_TEST_ASSERT_EQUAL(10*1024*1024, mi.size);

    /* allocate 2MB */    
    p = ina_mem_alloc(2*1024*1024);
    INA_TEST_ASSERT_NOT_NULL(p);
    INA_TEST_ASSERT_SUCCESS(ina_err_peek());
    INA_TEST_ASSERT_SUCCESS(ina_mempool_getinfo(NULL, &mi));
    INA_TEST_ASSERT_EQUAL(0, mi.children);
    INA_TEST_ASSERT_EQUAL((10*1024*1024), mi.size);
    INA_TEST_ASSERT_EQUAL(__INA_MEM_ALIGN(2*1024*1024), mi.used);
}
