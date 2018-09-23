/*
 * Copyright INAOS GmbH, Thalwil, 2014-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>

static const char *my_test_string = "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson." \
  "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson." \
  "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson." \
  "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson." \
  "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson." \
  "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson." \
  "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson.";

static void __ina_test_compression(ina_mempool_t *pool, ina_compression_type_t ct, ina_compression_mode_t cm)
{
    ina_compression_state_t *cstate;
    int dest_len;
    int src_len = (int)strlen(my_test_string);
    int wrote_len;
    int read_len;
    unsigned char *dest_buf;
    unsigned char *buf;

    if (pool == NULL) {
        INA_TEST_ASSERT_SUCCEED(ina_compression_new(&cstate, ct, cm));
    }
    else {
        INA_TEST_ASSERT_SUCCEED(ina_compression_new_using_pool(&cstate, ct, cm, pool));
    }

    INA_TEST_ASSERT_SUCCEED(ina_compression_get_destination_len(cstate, src_len, &dest_len));

    if (pool == NULL) {
        dest_buf = (unsigned char*)ina_mem_alloc(sizeof(unsigned char)*dest_len);
    }
    else {
        dest_buf = (unsigned char*)ina_mempool_dalloc(pool, sizeof(unsigned char)*dest_len);
        INA_ASSERT_NOTNULL(dest_buf);
    }

    INA_TEST_ASSERT_SUCCEED(ina_compression_compress_chunk(cstate, (const unsigned char*)my_test_string, 
        src_len, dest_buf, dest_len, &wrote_len, &read_len, INA_NO));

    if (pool == NULL) {
        buf = (unsigned char*)ina_mem_alloc(sizeof(unsigned char)*(src_len+1));
    }
    else {
        buf = (unsigned char*)ina_mempool_dalloc(pool, sizeof(unsigned char)*(src_len+1));
        INA_ASSERT_NOTNULL(buf);
    }

    INA_TEST_ASSERT_SUCCEED(ina_compression_decompress_chunk(cstate, dest_buf, wrote_len, 
        buf, src_len+1, &wrote_len, &read_len, INA_NO));

    buf[wrote_len] = '\0';
    INA_TEST_ASSERT_TRUE(strcmp(my_test_string, (const char*)buf) == 0);
    
    if (pool == NULL) {
        ina_mem_free(dest_buf);
        ina_mem_free(buf);
    }
}

INA_TEST(compression, deflate_string)
{
    __ina_test_compression(NULL, INA_COMPRESSION_TYPE_DEFLATE, INA_COMPRESSION_MODE_TRUSTED_FAST);
}

INA_TEST(compression, deflate_pool_string)
{
    ina_mempool_t *pool;
    
    INA_TEST_ASSERT_SUCCEED(ina_mempool_new(1024 * 1024, NULL, INA_MEM_DYNAMIC, &pool));

    __ina_test_compression(pool, INA_COMPRESSION_TYPE_DEFLATE, INA_COMPRESSION_MODE_TRUSTED_FAST);

    ina_mempool_free(&pool);
}

INA_TEST(compression, lz4_safe_string)
{
    __ina_test_compression(NULL, INA_COMPRESSION_TYPE_LZ4, INA_COMPRESSION_MODE_TRUSTED_SAFE);
}

INA_TEST(compression, lz4_safe_pool_string)
{
    ina_mempool_t *pool;

    INA_TEST_ASSERT_SUCCEED(ina_mempool_new(1024 * 1024, NULL, INA_MEM_DYNAMIC, &pool));

    __ina_test_compression(NULL, INA_COMPRESSION_TYPE_LZ4, INA_COMPRESSION_MODE_TRUSTED_SAFE);

    ina_mempool_free(&pool);
}

INA_TEST(compression, lz4_fast_string)
{
    __ina_test_compression(NULL, INA_COMPRESSION_TYPE_LZ4, INA_COMPRESSION_MODE_TRUSTED_FAST);
}

INA_TEST(compression, lz4_fast_pool_string)
{
    ina_mempool_t *pool;

    INA_TEST_ASSERT_SUCCEED(ina_mempool_new(1024 * 1024, NULL, INA_MEM_DYNAMIC, &pool));

    __ina_test_compression(NULL, INA_COMPRESSION_TYPE_LZ4, INA_COMPRESSION_MODE_TRUSTED_FAST);

    ina_mempool_free(&pool);
}

INA_TEST(compression, lz4hc_safe_string)
{
    __ina_test_compression(NULL, INA_COMPRESSION_TYPE_LZ4HC, INA_COMPRESSION_MODE_TRUSTED_SAFE);
}

INA_TEST(compression, lz4hc_safe_pool_string)
{
    ina_mempool_t *pool;

    INA_TEST_ASSERT_SUCCEED(ina_mempool_new(1024 * 1024, NULL, INA_MEM_DYNAMIC, &pool));

    __ina_test_compression(NULL, INA_COMPRESSION_TYPE_LZ4HC, INA_COMPRESSION_MODE_TRUSTED_SAFE);

    ina_mempool_free(&pool);
}

INA_TEST(compression, lz4hc_fast_string)
{
    __ina_test_compression(NULL, INA_COMPRESSION_TYPE_LZ4HC, INA_COMPRESSION_MODE_TRUSTED_FAST);
}

INA_TEST(compression, lz4hc_fast_pool_string)
{
    ina_mempool_t *pool;

    INA_TEST_ASSERT_SUCCEED(ina_mempool_new(1024 * 1024, NULL, INA_MEM_DYNAMIC, &pool));

    __ina_test_compression(NULL, INA_COMPRESSION_TYPE_LZ4HC, INA_COMPRESSION_MODE_TRUSTED_FAST);

    ina_mempool_free(&pool);
}

