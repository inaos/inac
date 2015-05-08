/*
 * Copyright (c) 2014, INAOS GmbH
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
    size_t dest_len;
    size_t src_len = strlen(my_test_string);
    size_t wrote_len;
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
    }

    INA_TEST_ASSERT_SUCCEED(ina_compression_compress_chunk(cstate, (const unsigned char*)my_test_string, 
        src_len, dest_buf, dest_len, &wrote_len));

    if (pool == NULL) {
        buf = (unsigned char*)ina_mem_alloc(sizeof(unsigned char)*(src_len+1));
    }
    else {
        buf = (unsigned char*)ina_mempool_dalloc(pool, sizeof(unsigned char)*(src_len+1));
    }

    INA_TEST_ASSERT_SUCCEED(ina_compression_decompress_chunk(cstate, dest_buf, wrote_len, 
        buf, src_len+1, &wrote_len));

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
    
    INA_TEST_ASSERT_SUCCEED(ina_mempool_create(&pool, 1024*1024, INA_MEM_DYNAMIC, NULL));

    __ina_test_compression(pool, INA_COMPRESSION_TYPE_DEFLATE, INA_COMPRESSION_MODE_TRUSTED_FAST);

    INA_TEST_ASSERT_SUCCEED(ina_mempool_release(pool, INA_YES));
}

INA_TEST(compression, lz4_safe_string)
{
    __ina_test_compression(NULL, INA_COMPRESSION_TYPE_LZ4, INA_COMPRESSION_MODE_TRUSTED_SAFE);
}

INA_TEST(compression, lz4_safe_pool_string)
{
    ina_mempool_t *pool;

    INA_TEST_ASSERT_SUCCEED(ina_mempool_create(&pool, 1024*1024, INA_MEM_DYNAMIC, NULL));

    __ina_test_compression(NULL, INA_COMPRESSION_TYPE_LZ4, INA_COMPRESSION_MODE_TRUSTED_SAFE);

    INA_TEST_ASSERT_SUCCEED(ina_mempool_release(pool, INA_YES));
}

INA_TEST(compression, lz4_fast_string)
{
    __ina_test_compression(NULL, INA_COMPRESSION_TYPE_LZ4, INA_COMPRESSION_MODE_TRUSTED_FAST);
}

INA_TEST(compression, lz4_fast_pool_string)
{
    ina_mempool_t *pool;

    INA_TEST_ASSERT_SUCCEED(ina_mempool_create(&pool, 1024*1024, INA_MEM_DYNAMIC, NULL));

    __ina_test_compression(NULL, INA_COMPRESSION_TYPE_LZ4, INA_COMPRESSION_MODE_TRUSTED_FAST);

    INA_TEST_ASSERT_SUCCEED(ina_mempool_release(pool, INA_YES));
}

INA_TEST(compression, lz4hc_safe_string)
{
    __ina_test_compression(NULL, INA_COMPRESSION_TYPE_LZ4HC, INA_COMPRESSION_MODE_TRUSTED_SAFE);
}

INA_TEST(compression, lz4hc_safe_pool_string)
{
    ina_mempool_t *pool;

    INA_TEST_ASSERT_SUCCEED(ina_mempool_create(&pool, 1024*1024, INA_MEM_DYNAMIC, NULL));

    __ina_test_compression(NULL, INA_COMPRESSION_TYPE_LZ4HC, INA_COMPRESSION_MODE_TRUSTED_SAFE);

    INA_TEST_ASSERT_SUCCEED(ina_mempool_release(pool, INA_YES));
}

INA_TEST(compression, lz4hc_fast_string)
{
    __ina_test_compression(NULL, INA_COMPRESSION_TYPE_LZ4HC, INA_COMPRESSION_MODE_TRUSTED_FAST);
}

INA_TEST(compression, lz4hc_fast_pool_string)
{
    ina_mempool_t *pool;

    INA_TEST_ASSERT_SUCCEED(ina_mempool_create(&pool, 1024*1024, INA_MEM_DYNAMIC, NULL));

    __ina_test_compression(NULL, INA_COMPRESSION_TYPE_LZ4HC, INA_COMPRESSION_MODE_TRUSTED_FAST);

    INA_TEST_ASSERT_SUCCEED(ina_mempool_release(pool, INA_YES));
}

