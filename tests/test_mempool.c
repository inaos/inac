/*
 * Copyright INAOS GmbH, Thalwil, 2012-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <stdio.h>
#include <libinac/lib.h>


INA_TEST(mempool, create_fixed)
{
    ina_mempool_t *pool;
    ina_mempool_info_t info;
    
    INA_TEST_ASSERT_SUCCEED(ina_mempool_new(4096, NULL, 0, &pool));
    INA_TEST_ASSERT_NOT_NULL(pool);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_info(pool, &info));
    INA_TEST_ASSERT_TRUE(info.cf == INA_MEM_FIXED);
    INA_TEST_ASSERT_EQUAL_SIZE_T(0, info.children);
    INA_TEST_ASSERT_EQUAL_SIZE_T(0, info.used);
    INA_TEST_ASSERT_EQUAL_SIZE_T(4096, info.size);
    ina_mempool_free(&pool);
}

INA_TEST(mempool, create_fixed_bestfit)
{
    ina_mempool_t *pool;
    ina_mempool_info_t info;
    
    INA_TEST_ASSERT_SUCCEED(ina_mempool_new(4096, NULL, INA_MEM_BESTFIT, &pool));
    INA_TEST_ASSERT_NOT_NULL(pool);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_info(pool, &info));
    INA_TEST_ASSERT_TRUE(info.cf&INA_MEM_BESTFIT);
    INA_TEST_ASSERT_EQUAL_SIZE_T(0, info.children);
    INA_TEST_ASSERT_EQUAL_SIZE_T(0, info.used);
    INA_TEST_ASSERT_EQUAL_SIZE_T(4096, info.size);
    ina_mempool_free(&pool);
}

INA_TEST(mempool, bestfit)
{
    ina_mempool_t *pool;
    ina_mempool_info_t info;
    char *buf;

    INA_TEST_ASSERT_SUCCEED(ina_mempool_new(4096, NULL, INA_MEM_BESTFIT, &pool));
    INA_TEST_ASSERT_NOT_NULL(pool);
    buf = ina_mempool_dalloc(pool, 1);
    INA_TEST_ASSERT_NOT_NULL(buf);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_info(pool, &info));
    INA_TEST_ASSERT_EQUAL_SIZE_T(1, info.used);
    buf = ina_mempool_dalloc(pool, 33);
    INA_TEST_ASSERT_NOT_NULL(buf);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_info(pool, &info));
    INA_TEST_ASSERT_EQUAL_SIZE_T(34, info.used);
    ina_mempool_free(&pool);
}


INA_TEST(mempool, aligned)
{
    ina_mempool_t *pool;
    ina_mempool_info_t info;
    char *buf;

    INA_TEST_ASSERT_SUCCEED(ina_mempool_new(4096, NULL, 0, &pool));
    INA_TEST_ASSERT_NOT_NULL(pool);
    buf = ina_mempool_dalloc(pool, 1);
    INA_TEST_ASSERT_NOT_NULL(buf);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_info(pool, &info));
    INA_TEST_ASSERT_EQUAL_SIZE_T(16, info.used);
    buf = ina_mempool_dalloc(pool, 33);
    INA_TEST_ASSERT_NOT_NULL(buf);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_info(pool, &info));
    INA_TEST_ASSERT_EQUAL_SIZE_T(64, info.used);
    ina_mempool_free(&pool);
}

INA_TEST(mempool, nalloc_fixed)
{
    ina_mempool_t *pool;
    ina_mempool_info_t info;
    char *buf1;
    char *buf2;
    char *buf3;
    char *buf4;
    int i;
    
    INA_TEST_ASSERT_SUCCEED(ina_mempool_new(4096, NULL, 0, &pool));
    INA_TEST_ASSERT_NOT_NULL(pool);
    
    /* Allocate a reallocable buffer */
    buf1 = ina_mempool_dalloc(pool, 1024);
    INA_TEST_ASSERT_NOT_NULL(buf1);
    ina_mem_set(buf1, 20, 1024);

    /* Allocate a NON reallocable buffer */
    buf2 = ina_mempool_nalloc(pool, 1024);
    INA_TEST_ASSERT_NOT_NULL(buf2);
    ina_mem_set(buf2, 30, 1024);

    /* we user 2x1024 bytes */
    INA_TEST_ASSERT_SUCCEED(ina_mempool_info(pool, &info));
    INA_TEST_ASSERT_EQUAL_SIZE_T(2048, info.used);

    /* Release pool */
    INA_TEST_ASSERT_SUCCEED(ina_mempool_reset(pool));

    /* After release we still use 1x1024 bytes */
    INA_TEST_ASSERT_SUCCEED(ina_mempool_info(pool, &info));
    INA_TEST_ASSERT_EQUAL_SIZE_T(1024, info.used);

    /* allocate new reallocable buffer friom the fresh pool */
    buf3 = ina_mempool_dalloc(pool, 4);

    /* New pointer same as the old one */
    INA_TEST_ASSERT_SAME(buf3, buf1);

    /* Verify buffer invalidation */
    ina_mem_set(buf3, 40, 4);
    INA_TEST_ASSERT_EQUAL_INT(40, buf1[0]);
    INA_TEST_ASSERT_EQUAL_INT(40, buf1[1]);
    INA_TEST_ASSERT_EQUAL_INT(40, buf1[2]);
    INA_TEST_ASSERT_EQUAL_INT(40, buf1[3]);
    INA_TEST_ASSERT_EQUAL_INT(20, buf1[4]);

    /* allocate new NOT reallocable buffer from the fresh pool */
    buf4 = ina_mempool_nalloc(pool, 5);
    INA_TEST_ASSERT_NOT_NULL(buf4);

    /* New pointer NOT same as the old one */
    INA_TEST_ASSERT_NOT_SAME(buf4, buf2);
    /* Verify buffer, old should be valid */
    ina_mem_set(buf4, 50, 5);
    for (i = 0; i < 1024; i++) {
        INA_TEST_ASSERT_EQUAL_INT(30, buf2[i]);
    }
    INA_TEST_ASSERT_EQUAL_INT(50, buf4[0]);
}

INA_TEST(mempool, dalloc)
{
    ina_mempool_t *pool;
    char *buf;

    ina_err_reset();

    INA_TEST_ASSERT_SUCCEED(ina_mempool_new(1024 * 1024 * 2, NULL, INA_MEM_DYNAMIC, &pool));
    INA_TEST_ASSERT_NOT_NULL(pool);
    buf = ina_mempool_dalloc(pool, 1024*1024*2);
    ina_mempool_reset(pool);
    INA_TEST_ASSERT_NOT_NULL(buf);
    buf = ina_mempool_dalloc(pool, 1024*1024*2);
    ina_mempool_free(&pool);
}

INA_TEST(mempool, clear)
{
    ina_mempool_t *pool;
    ina_mempool_info_t info;

    char *buf;

    ina_err_reset();

    INA_TEST_ASSERT_SUCCEED(ina_mempool_new(2048, NULL, INA_MEM_DYNAMIC, &pool));
    INA_TEST_ASSERT_NOT_NULL(pool);

    buf = ina_mempool_dalloc(pool, 4*1024);
    INA_TEST_ASSERT_NOT_NULL(buf);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_info(pool, &info));
    INA_TEST_ASSERT_EQUAL_SIZE_T(4*1024, info.used);
    INA_TEST_ASSERT_EQUAL_SIZE_T(1, info.children);
    ina_mem_set(buf, 20, 4*1024);


    buf = ina_mempool_dalloc(pool, 2*1024);
    INA_TEST_ASSERT_NOT_NULL(buf);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_info(pool, &info));
    INA_TEST_ASSERT_EQUAL_SIZE_T(6*1024, info.used);
    INA_TEST_ASSERT_EQUAL_SIZE_T(2, info.children);

    ina_mempool_clear(pool);

    buf = ina_mempool_dalloc(pool, 4*1024);
    INA_TEST_ASSERT_NOT_NULL(buf);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_info(pool, &info));
    INA_TEST_ASSERT_EQUAL_SIZE_T(4*1024, info.used);
    INA_TEST_ASSERT_EQUAL_SIZE_T(2, info.children);
    INA_TEST_ASSERT_EQUAL_INT(0, buf[0]);
    INA_TEST_ASSERT_EQUAL_INT(0, buf[100]);
    INA_TEST_ASSERT_EQUAL_INT(0, buf[4095]);

}

INA_TEST(mempool, reset)
{
    ina_mempool_t *pool;
    ina_mempool_info_t info;

    char *buf;

    ina_err_reset();

    INA_TEST_ASSERT_SUCCEED(ina_mempool_new(2048, NULL, INA_MEM_DYNAMIC, &pool));
    INA_TEST_ASSERT_NOT_NULL(pool);

    buf = ina_mempool_dalloc(pool, 4*1024);
    INA_TEST_ASSERT_NOT_NULL(buf);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_info(pool, &info));
    INA_TEST_ASSERT_EQUAL_SIZE_T(4*1024, info.used);
    INA_TEST_ASSERT_EQUAL_SIZE_T(1, info.children);
    ina_mem_set(buf, 20, 4*1024);


    buf = ina_mempool_dalloc(pool, 2*1024);
    INA_TEST_ASSERT_NOT_NULL(buf);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_info(pool, &info));
    INA_TEST_ASSERT_EQUAL_SIZE_T(6*1024, info.used);
    INA_TEST_ASSERT_EQUAL_SIZE_T(2, info.children);

    ina_mempool_reset(pool);

    buf = ina_mempool_dalloc(pool, 2*1024);
    INA_TEST_ASSERT_NOT_NULL(buf);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_info(pool, &info));
    INA_TEST_ASSERT_EQUAL_SIZE_T(2*1024, info.used);
    INA_TEST_ASSERT_EQUAL_SIZE_T(2, info.children);
    INA_TEST_ASSERT_EQUAL_INT(0, buf[0]);
    INA_TEST_ASSERT_EQUAL_INT(0, buf[100]);
    INA_TEST_ASSERT_EQUAL_INT(0, buf[2047]);

    buf = ina_mempool_dalloc(pool, 2*1024);
    INA_TEST_ASSERT_NOT_NULL(buf);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_info(pool, &info));
    INA_TEST_ASSERT_EQUAL_SIZE_T(4*1024, info.used);
    INA_TEST_ASSERT_EQUAL_SIZE_T(2, info.children);
    INA_TEST_ASSERT_EQUAL_INT(20, buf[0]);
    INA_TEST_ASSERT_EQUAL_INT(20, buf[100]);
    INA_TEST_ASSERT_EQUAL_INT(20, buf[3071]);
}

INA_TEST(mempool, realloc_dynamic)
{
    ina_mempool_t *pool;
    ina_mempool_info_t info;

    char *buf;
    char *old_buf;

    ina_err_reset();

    INA_TEST_ASSERT_SUCCEED(ina_mempool_new(4096, NULL, INA_MEM_DYNAMIC, &pool));
    INA_TEST_ASSERT_NOT_NULL(pool);

    buf = ina_mempool_dalloc(pool, 1024);
    INA_TEST_ASSERT_NOT_NULL(buf);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_info(pool, &info));
    INA_TEST_ASSERT_EQUAL_SIZE_T(1024, info.used);

    /* Reallocate up to 2048 bytes. Pointer should be still the same */
    /* we have not sub pools */
    old_buf = buf;
    buf = ina_mempool_ralloc(pool, buf, 1024, 2048);
    INA_TEST_ASSERT_NOT_NULL(buf);
    INA_TEST_ASSERT_SAME(old_buf, buf);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_info(pool, &info));
    INA_TEST_ASSERT_EQUAL_SIZE_T(2048, info.used);
    INA_TEST_ASSERT_EQUAL_SIZE_T(0, info.children);

    /* Reallocate up to 9216 bytes. Pointer should NOT be the same */
    /* A subpool shold be there */
    buf = ina_mempool_ralloc(pool, buf, 2048, 9216);
    INA_TEST_ASSERT_NOT_NULL(buf);
    INA_TEST_ASSERT_NOT_SAME(old_buf, buf);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_info(pool, &info));
    INA_TEST_ASSERT_EQUAL_SIZE_T(9216+2048, info.used);
    INA_TEST_ASSERT_EQUAL_SIZE_T(1, info.children);

    ina_mempool_free(&pool);
}

INA_TEST(mempool, realloc_fixed)
{
    ina_mempool_t *pool;
    ina_mempool_info_t info;

    char *buf;
    char *buf2;
    char *old_buf;

    ina_err_reset();

    INA_TEST_ASSERT_SUCCEED(ina_mempool_new(4096, NULL, 0, &pool));
    INA_TEST_ASSERT_NOT_NULL(pool);
    
    buf = ina_mempool_dalloc(pool, 128);
    INA_TEST_ASSERT_NOT_NULL(buf);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_info(pool, &info));
    INA_TEST_ASSERT_EQUAL_SIZE_T(128, info.used);
    
    /* Reallocate up to 512 bytes. Pointer should be still te same */
    old_buf = buf;
    buf = ina_mempool_ralloc(pool, buf, 128, 512);
    INA_TEST_ASSERT_NOT_NULL(buf);
    INA_TEST_ASSERT_SAME(old_buf, buf);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_info(pool, &info));
    INA_TEST_ASSERT_EQUAL_SIZE_T(512, info.used);

   /* Reallocate down to 256 bytes. Pointer should be still te same 
    * Pool shold be shirked */
    buf = ina_mempool_ralloc(pool, buf, 512, 256);
    INA_TEST_ASSERT_NOT_NULL(buf);
    INA_TEST_ASSERT_SAME(old_buf, buf);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_info(pool, &info));
    INA_TEST_ASSERT_EQUAL_SIZE_T(256, info.used);

    /* Allocate new buffer form pool, to break reallocate 
     * optimization */
    buf2 = ina_mempool_dalloc(pool, 128);
    INA_TEST_ASSERT_NOT_NULL(buf2);
    INA_TEST_ASSERT_NOT_SAME(buf, buf2);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_info(pool, &info));
    INA_TEST_ASSERT_EQUAL_SIZE_T(128+256, info.used);

    /* Reallocate up to 512 bytes. Pointer should be still te same 
    * Pool shold be shirked */
    buf = ina_mempool_ralloc(pool, buf, 256, 512);
    INA_TEST_ASSERT_NOT_NULL(buf);
    INA_TEST_ASSERT_NOT_SAME(old_buf, buf);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_info(pool, &info));
    INA_TEST_ASSERT_EQUAL_SIZE_T(128+256+512, info.used);

    ina_mempool_free(&pool);

}



INA_TEST(mempool, fill_zero)
{
    ina_mempool_t *pool;
    unsigned char *buf;
    size_t size = INA_MEM_MIN_POOL_SIZE-100;

    /* clear error state and assure it's clean */
    INA_TEST_ASSERT_SUCCEED(ina_err_reset());
    INA_TEST_ASSERT_SUCCEED(ina_err_get_rc());

    pool = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_mempool_new(INA_MEM_MIN_POOL_SIZE, NULL, 0, &pool));
    INA_TEST_ASSERT_NOT_NULL(pool);
    buf = (unsigned char*)ina_mempool_dalloc(pool, size);
    INA_TEST_ASSERT_NOT_NULL(buf);
    while (size--) {
        INA_TEST_ASSERT_EQUAL_UINT(*(buf++), 0);
    }
}
INA_TEST(mempool, min_allowed_size)
{
    ina_mempool_t *pool;
    ina_mempool_info_t mi;

    /* clear error state and assure it's clean */
    INA_TEST_ASSERT_SUCCEED(ina_err_reset());
    INA_TEST_ASSERT_SUCCEED(ina_err_get_rc());
    
    pool = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_mempool_new(INA_MEM_MIN_POOL_SIZE - 100, NULL, 0, &pool));
    INA_TEST_ASSERT_NOT_NULL(pool);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_info(pool, &mi));
    INA_TEST_ASSERT_EQUAL_SIZE_T(INA_MEM_MIN_POOL_SIZE, mi.size);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_new(INA_MEM_MIN_POOL_SIZE, NULL, 0, &pool));
    INA_TEST_ASSERT_NOT_NULL(pool);
}

INA_TEST(mempool, merge)
{
    ina_mempool_t *pool_a, *pool_b;
    ina_mempool_info_t mi;

    INA_TEST_ASSERT_SUCCEED(ina_mempool_new(4096, NULL, 0, &pool_a));
    INA_TEST_ASSERT_SUCCEED(ina_mempool_info(pool_a, &mi));
    INA_TEST_ASSERT_EQUAL_SIZE_T(0, mi.children);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_new(4096, NULL, 0, &pool_b));
    INA_TEST_ASSERT_SUCCEED(ina_mempool_info(pool_b, &mi));
    INA_TEST_ASSERT_EQUAL_SIZE_T(0, mi.children);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_merge(pool_a, pool_b));
    INA_TEST_ASSERT_SUCCEED(ina_mempool_info(pool_a, &mi));
    INA_TEST_ASSERT_EQUAL_SIZE_T(1, mi.children);

    ina_mempool_free(&pool_a);
}

INA_TEST(mempool, auto_resize) {

    ina_mempool_t *pool;
    ina_mempool_info_t mi;
    unsigned char *buffer;

    /* clear error state and assure it's clean */
    INA_TEST_ASSERT_SUCCEED(ina_err_reset());
    INA_TEST_ASSERT_SUCCEED(ina_err_get_rc());

    /* Allocate pool with initial site 2KB dynamic + auto size */
    INA_TEST_ASSERT_SUCCEED(ina_mempool_new(2048, NULL, INA_MEM_DYNAMIC | INA_MEM_AUTOSIZE, &pool));
    buffer = ina_mempool_dalloc(pool, 1024);
    INA_TEST_ASSERT_NOT_NULL(buffer);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_info(pool, &mi));
    INA_TEST_ASSERT_EQUAL_SIZE_T(2048, mi.size);
    INA_TEST_ASSERT_EQUAL_SIZE_T(1024, mi.used);

    buffer = ina_mempool_dalloc(pool, 1024);
    INA_TEST_ASSERT_NOT_NULL(buffer);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_info(pool, &mi));
    INA_TEST_ASSERT_EQUAL_SIZE_T(2048, mi.size);
    INA_TEST_ASSERT_EQUAL_SIZE_T(2048, mi.used);

    buffer = ina_mempool_dalloc(pool, 1024);
    INA_TEST_ASSERT_NOT_NULL(buffer);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_info(pool, &mi));
    INA_TEST_ASSERT_EQUAL_SIZE_T(3072, mi.size);
    INA_TEST_ASSERT_EQUAL_SIZE_T(3072, mi.used);

    buffer = ina_mempool_dalloc(pool, 4096);
    INA_TEST_ASSERT_NOT_NULL(buffer);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_info(pool, &mi));
    INA_TEST_ASSERT_EQUAL_SIZE_T(7168, mi.size);
    INA_TEST_ASSERT_EQUAL_SIZE_T(7168, mi.used);

    ina_mempool_free(&pool);
    
    /* Allocate pool with initial site 2KB dynamic */
    INA_TEST_ASSERT_SUCCEED(ina_mempool_new(2048, NULL, INA_MEM_DYNAMIC, &pool));
    buffer = ina_mempool_dalloc(pool, 1024);
    INA_TEST_ASSERT_NOT_NULL(buffer);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_info(pool, &mi));
    INA_TEST_ASSERT_EQUAL_SIZE_T(2048, mi.size);
    INA_TEST_ASSERT_EQUAL_SIZE_T(1024, mi.used);

    buffer = ina_mempool_dalloc(pool, 1024);
    INA_TEST_ASSERT_NOT_NULL(buffer);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_info(pool, &mi));
    INA_TEST_ASSERT_EQUAL_SIZE_T(2048, mi.size);
    INA_TEST_ASSERT_EQUAL_SIZE_T(2048, mi.used);

    buffer = ina_mempool_dalloc(pool, 1024);
    INA_TEST_ASSERT_NOT_NULL(buffer);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_info(pool, &mi));
    INA_TEST_ASSERT_EQUAL_SIZE_T(4096, mi.size);
    INA_TEST_ASSERT_EQUAL_SIZE_T(3072, mi.used);

    buffer = ina_mempool_dalloc(pool, 3096);
    INA_TEST_ASSERT_NOT_NULL(buffer);
    INA_TEST_ASSERT_SUCCEED(ina_mempool_info(pool, &mi));
#ifdef INA_CPU_X86_64
    INA_TEST_ASSERT_EQUAL_SIZE_T(7200, mi.size);
    INA_TEST_ASSERT_EQUAL_SIZE_T(6176, mi.used);
#else
    INA_TEST_ASSERT_EQUAL_SIZE_T(7192, mi.size);
    INA_TEST_ASSERT_EQUAL_SIZE_T(6168, mi.used);
#endif


}

INA_TEST(mempool, bad_dalloc)
{
    void *ptr;
    ina_mempool_t *pool;

    ptr = NULL;
    pool = NULL;

    /* clear error state and assure it's clean */
    INA_TEST_ASSERT_SUCCEED(ina_err_reset());
    INA_TEST_ASSERT_SUCCEED(ina_err_get_rc());

    /* create a fixed size pool of 1KB and try to allocate 2KB */
    INA_TEST_ASSERT_SUCCEED(ina_mempool_new(1024, NULL, 0, &pool));
    INA_TEST_ASSERT_NOT_NULL(pool);
    ptr = ina_mempool_dalloc(pool, 2048);
    INA_TEST_ASSERT_NULL(ptr);
    INA_TEST_ASSERT_FALSE(INA_SUCCEED(ina_err_get_rc()));
    INA_TEST_ASSERT_EQUAL_INT64(INA_ERR_FULL , INA_RC_ERROR(ina_err_get_rc()));
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
    ina_mempool_free(&data->mp);
}

INA_TEST_FIXTURE(mempool_ipc, mempool_create)
{
    int32_t *v = NULL;
    int32_t c = 0;
 
    ina_time_sleep(1000);

    INA_TEST_ASSERT_SUCCEED(ina_mempool_new(
            1024 * sizeof(int32_t),
            "/ina_test",
            INA_MEM_SHARED, &data->mp));
    
    v = (int32_t*)ina_mempool_dalloc(data->mp, 1024*sizeof(int32_t));
    INA_TEST_ASSERT_NOT_NULL(v);
    while (c < 1024) {
        INA_TEST_ASSERT_EQUAL_INT(c, v[c]);
        c++;
    }
}
