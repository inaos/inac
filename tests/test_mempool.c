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

INA_TEST(mempool, nalloc)
{
    ina_mempool_t *pool;
    char *buf1;
    char *buf2;
    char *buf3;
    char *buf4;
    int i;
    
    INA_TEST_ASSERT_SUCCEED(ina_mempool_create(&pool, 4096, 0, NULL));
    INA_TEST_ASSERT_NOT_NULL(pool);
    
    /* Allocate a reallocable buffer */
    buf1 = ina_mempool_dalloc(pool, 1024);
    INA_TEST_ASSERT_NOT_NULL(buf1);
    ina_mem_set(buf1, 20, 1024);

    /* Allocate a NON reallocable buffer */
    buf2 = ina_mempool_nalloc(pool, 1024);
    INA_TEST_ASSERT_NOT_NULL(buf1);
    ina_mem_set(buf2, 30, 1024);

    /* Release pool */
    INA_TEST_ASSERT_SUCCEED(ina_mempool_release(pool, INA_NO));

    /* allocate new reallocable buffer friom the fresh pool */
    buf3 = ina_mempool_dalloc(pool, 5);

    /* New pointer same as the old one */
    INA_TEST_ASSERT_SAME(buf3, buf1);

    /* Verify buffer invalidation */
    ina_mem_set(buf3, 40, 5);
    INA_TEST_ASSERT_EQUAL_INTEGER(40, buf1[0]);
    INA_TEST_ASSERT_EQUAL_INTEGER(40, buf1[1]);
    INA_TEST_ASSERT_EQUAL_INTEGER(40, buf1[2]);
    INA_TEST_ASSERT_EQUAL_INTEGER(40, buf1[3]);
    INA_TEST_ASSERT_EQUAL_INTEGER(40, buf1[4]);
    INA_TEST_ASSERT_EQUAL_INTEGER(0, buf1[5]);

    /* allocate new NOT reallocable buffer from the fresh pool */
    buf4 = ina_mempool_nalloc(pool, 5);
    /* New pointer same as the old one */
    INA_TEST_ASSERT_NOT_SAME(buf4, buf2);
    /* Verify buffer */
    ina_mem_set(buf4, 50, 5);
    for (i = 0; i < 1024; i++) {
        INA_TEST_ASSERT_EQUAL_INTEGER(30, buf2[i]);
    }
    INA_TEST_ASSERT_EQUAL_INTEGER(50, buf4[0]);
}


INA_TEST(mempool, realloc_dynamic)
{
    ina_mempool_t *pool;
    ina_mempool_info_t info;

    char *buf;
    char *old_buf;

    ina_err_reset();

    INA_TEST_ASSERT_SUCCEED(ina_mempool_create(&pool, 4096, INA_MEM_DYNAMIC, NULL));
    INA_TEST_ASSERT_NOT_NULL(pool);

    buf = ina_mempool_dalloc(pool, 1024);
    INA_TEST_ASSERT_NOT_NULL(buf);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_getinfo(pool, &info));
    INA_TEST_ASSERT_EQUAL_INTEGER(1024, info.used);

    /* Reallocate up to 2048 bytes. Pointer should be still the same */
    /* we have not sub pools */
    old_buf = buf;
    buf = ina_mempool_ralloc(pool, buf, 1024, 2048);
    INA_TEST_ASSERT_NOT_NULL(buf);
    INA_TEST_ASSERT_SAME(old_buf, buf);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_getinfo(pool, &info));
    INA_TEST_ASSERT_EQUAL_INTEGER(2048, info.used);
    INA_TEST_ASSERT_EQUAL_INTEGER(0, info.children);

    /* Reallocate up to 9216 bytes. Pointer should NOT be the same */
    /* A subpool shold be there */
    buf = ina_mempool_ralloc(pool, buf, 2048, 9216);
    INA_TEST_ASSERT_NOT_NULL(buf);
    INA_TEST_ASSERT_NOT_SAME(old_buf, buf);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_getinfo(pool, &info));
    INA_TEST_ASSERT_EQUAL_INTEGER(9216+2048, info.used);
    INA_TEST_ASSERT_EQUAL_INTEGER(1, info.children);

    ina_mempool_release(pool, INA_YES);
}

INA_TEST(mempool, realloc_fixed)
{
    ina_mempool_t *pool;
    ina_mempool_info_t info;

    char *buf;
    char *buf2;
    char *old_buf;

    ina_err_reset();

    INA_TEST_ASSERT_SUCCEED(ina_mempool_create(&pool, 4096, 0, NULL));
    INA_TEST_ASSERT_NOT_NULL(pool);
    
    buf = ina_mempool_dalloc(pool, 128);
    INA_TEST_ASSERT_NOT_NULL(buf);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_getinfo(pool, &info));
    INA_TEST_ASSERT_EQUAL_INTEGER(128, info.used);
    
    /* Reallocate up to 512 bytes. Pointer should be still te same */
    old_buf = buf;
    buf = ina_mempool_ralloc(pool, buf, 128, 512);
    INA_TEST_ASSERT_NOT_NULL(buf);
    INA_TEST_ASSERT_SAME(old_buf, buf);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_getinfo(pool, &info));
    INA_TEST_ASSERT_EQUAL_INTEGER(512, info.used);

   /* Reallocate down to 256 bytes. Pointer should be still te same 
    * Pool shold be shirked */
    buf = ina_mempool_ralloc(pool, buf, 512, 256);
    INA_TEST_ASSERT_NOT_NULL(buf);
    INA_TEST_ASSERT_SAME(old_buf, buf);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_getinfo(pool, &info));
    INA_TEST_ASSERT_EQUAL_INTEGER(256, info.used); 

    /* Allocate new buffer form pool, to break reallocate 
     * optimization */
    buf2 = ina_mempool_dalloc(pool, 128);
    INA_TEST_ASSERT_NOT_NULL(buf2);
    INA_TEST_ASSERT_NOT_SAME(buf, buf2);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_getinfo(pool, &info));
    INA_TEST_ASSERT_EQUAL_INTEGER(128+256, info.used);

    /* Reallocate up to 512 bytes. Pointer should be still te same 
    * Pool shold be shirked */
    buf = ina_mempool_ralloc(pool, buf, 256, 512);
    INA_TEST_ASSERT_NOT_NULL(buf);
    INA_TEST_ASSERT_NOT_SAME(old_buf, buf);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_getinfo(pool, &info));
    INA_TEST_ASSERT_EQUAL_INTEGER(128+256+512, info.used); 

    ina_mempool_release(pool, INA_YES);

}

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
        INA_TEST_ASSERT_EQUAL_FLOATING(*(buf++), 0);
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
    INA_TEST_ASSERT_EQUAL_FLOATING(INA_MEM_MIN_POOL_SIZE, mi.size);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_create(&pool, INA_MEM_MIN_POOL_SIZE, 0, NULL));
    INA_TEST_ASSERT_NOT_NULL(pool);
}

INA_TEST(mempool, auto_resize) {

    ina_mempool_t *pool;
    ina_mempool_info_t mi;
    unsigned char *buffer;

    /* clear error state and assure it's clean */
    INA_TEST_ASSERT_SUCCESS(ina_err_reset());
    INA_TEST_ASSERT_SUCCESS(ina_err_peek());

    /* Allocate pool with initial site 2KB dynamic + auto size */
    INA_TEST_ASSERT_SUCCEED(ina_mempool_create(&pool, 2048, INA_MEM_DYNAMIC|INA_MEM_AUTOSIZE, NULL));
    buffer = ina_mempool_dalloc(pool, 1024);
    INA_TEST_ASSERT_NOT_NULL(buffer);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_getinfo(pool, &mi));
    INA_TEST_ASSERT_EQUAL_INTEGER(2048, mi.size);
    INA_TEST_ASSERT_EQUAL_INTEGER(1024, mi.used);

    buffer = ina_mempool_dalloc(pool, 1024);
    INA_TEST_ASSERT_NOT_NULL(buffer);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_getinfo(pool, &mi));
    INA_TEST_ASSERT_EQUAL_INTEGER(2048, mi.size);
    INA_TEST_ASSERT_EQUAL_INTEGER(2048, mi.used);

    buffer = ina_mempool_dalloc(pool, 1024);
    INA_TEST_ASSERT_NOT_NULL(buffer);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_getinfo(pool, &mi));
    INA_TEST_ASSERT_EQUAL_INTEGER(3072, mi.size);
    INA_TEST_ASSERT_EQUAL_INTEGER(3072, mi.used);

    buffer = ina_mempool_dalloc(pool, 4096);
    INA_TEST_ASSERT_NOT_NULL(buffer);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_getinfo(pool, &mi));
    INA_TEST_ASSERT_EQUAL_INTEGER(7168, mi.size);
    INA_TEST_ASSERT_EQUAL_INTEGER(7168, mi.used);

    INA_TEST_ASSERT_SUCCEED(ina_mempool_release(pool, INA_YES));
    
    /* Allocate pool with initial site 2KB dynamic */
    INA_TEST_ASSERT_SUCCEED(ina_mempool_create(&pool, 2048, INA_MEM_DYNAMIC, NULL));
    buffer = ina_mempool_dalloc(pool, 1024);
    INA_TEST_ASSERT_NOT_NULL(buffer);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_getinfo(pool, &mi));
    INA_TEST_ASSERT_EQUAL_INTEGER(2048, mi.size);
    INA_TEST_ASSERT_EQUAL_INTEGER(1024, mi.used);

    buffer = ina_mempool_dalloc(pool, 1024);
    INA_TEST_ASSERT_NOT_NULL(buffer);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_getinfo(pool, &mi));
    INA_TEST_ASSERT_EQUAL_INTEGER(2048, mi.size);
    INA_TEST_ASSERT_EQUAL_INTEGER(2048, mi.used);

    buffer = ina_mempool_dalloc(pool, 1024);
    INA_TEST_ASSERT_NOT_NULL(buffer);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_getinfo(pool, &mi));
    INA_TEST_ASSERT_EQUAL_INTEGER(4096, mi.size);
    INA_TEST_ASSERT_EQUAL_INTEGER(3072, mi.used);

    buffer = ina_mempool_dalloc(pool, 3096);
    INA_TEST_ASSERT_NOT_NULL(buffer);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_getinfo(pool, &mi));
#ifdef INA_CPU_X86_64
    INA_TEST_ASSERT_EQUAL_INTEGER(7200, mi.size);
    INA_TEST_ASSERT_EQUAL_INTEGER(6176, mi.used);
#else
    INA_TEST_ASSERT_EQUAL_INTEGER(7192, mi.size);
    INA_TEST_ASSERT_EQUAL_INTEGER(6168, mi.used);
#endif


}

INA_TEST(mempool, bad_dalloc)
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
    INA_TEST_ASSERT_EQUAL_FLOATING(INA_EALLOC , INA_RC_REASON(ina_err_peek()));
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
        INA_TEST_ASSERT_EQUAL_FLOATING(0, mi.children);
        INA_TEST_ASSERT_EQUAL_FLOATING(8*1024*1024, mi.size);
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
    INA_TEST_ASSERT_EQUAL_FLOATING(0, mi.children);
    INA_TEST_ASSERT_EQUAL_FLOATING(8*1024*1024, mi.size);

    /* destroy all pools and recreate internal 10MB pool */
    INA_TEST_ASSERT_SUCCESS(ina_mempool_destroy());
    INA_TEST_ASSERT_SUCCESS(ina_mempool_init(10*1024*1024));
    INA_TEST_ASSERT_SUCCESS(ina_mempool_getinfo(NULL, &mi));
    INA_TEST_ASSERT_EQUAL_FLOATING(0, mi.children);
    INA_TEST_ASSERT_EQUAL_FLOATING(10*1024*1024, mi.size);

    /* allocate 2MB */    
    p = ina_mem_alloc(2*1024*1024);
    INA_TEST_ASSERT_NOT_NULL(p);
    INA_TEST_ASSERT_SUCCESS(ina_err_peek());
    INA_TEST_ASSERT_SUCCESS(ina_mempool_getinfo(NULL, &mi));
    INA_TEST_ASSERT_EQUAL_FLOATING(0, mi.children);
    INA_TEST_ASSERT_EQUAL_FLOATING((10*1024*1024), mi.size);
    INA_TEST_ASSERT_EQUAL_FLOATING(__INA_MEM_ALIGN(2*1024*1024), mi.used);
}

INA_TEST_DATA(mempool_ipc) {
    ina_test_hid_t hid;
    ina_mempool_t *mp;
};

INA_TEST_SETUP(mempool_ipc)
{
    ina_mem_set(&data->hid, 0, sizeof(ina_test_hid_t));
    INA_TEST_HELPER_INVOKE(&data->hid, mempool_ipc, 
        mempool_create_and_fill_int32_values,
        "/ina_test", 
        INA_NUM2STR(4096), /*FIXME: 1024 * sizeof(int32_t)*/
        NULL);
    data->mp = NULL;
}

INA_TEST_TEARDOWN(mempool_ipc)
{
    INA_TEST_HELPER_TERMINATE(&data->hid);
    ina_mempool_release(data->mp, INA_YES);
}

INA_TEST_FIXTURE(mempool_ipc, mempool_create)
{
    int32_t *v = NULL;
    int32_t c = 0;
 
    ina_time_sleep(1000);

    INA_TEST_ASSERT_SUCCEED(ina_mempool_create(&data->mp, 
        1024*sizeof(int32_t),
        INA_MEM_SHARED,
        "/ina_test"));
    
    v = (int32_t*)ina_mempool_dalloc(data->mp, 1024*sizeof(int32_t));
    while (c < 1024) {
        INA_TEST_ASSERT_EQUAL_INTEGER(c, v[c]);
        c++;
    }
}
