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
#include <libinac/lib.h>


INA_TEST_DATA(string_mempool)
{
    ina_mempool_t *pool;
};

INA_TEST_SETUP(string_mempool)
{
    ina_err_reset();
    INA_TEST_ASSERT_SUCCEED(ina_mempool_create(&data->pool, 10*1024,INA_MEM_DYNAMIC, NULL));
    INA_TEST_ASSERT_NOT_NULL(data->pool);
}

INA_TEST_TEARDOWN(string_mempool)
{
    ina_mempool_release(data->pool, INA_YES);
    data->pool = NULL;
}

INA_TEST(string, ina_str_newlen)
{
   
}

INA_TEST(string, ina_str_pnewlen)
{
    
}

INA_TEST(string, ina_str_fromblk)
{
    ina_str_t str = NULL;
    char blk[] = "USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION";
    str = ina_str_fromblk(&blk[5], 4);
    INA_TEST_ASSERT_NOT_NULL(str);
    INA_TEST_ASSERT_EQUAL_STR("DATA", ina_str_cstr(str));
}

INA_TEST_FIXTURE(string_mempool, ina_str_pfromblk)
{
    ina_str_t str = NULL;
    char blk[] = "USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION";
    str = ina_str_pfromblk(&blk[5], 4, data->pool);
    INA_TEST_ASSERT_NOT_NULL(str);
    INA_TEST_ASSERT_EQUAL_STR("DATA", ina_str_cstr(str));
}

INA_TEST(string, ina_str_fromcstr)
{
    ina_str_t str = NULL;
    const char *cstring = "USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION";
    str = ina_str_fromcstr(cstring);
    INA_TEST_ASSERT_NOT_NULL(str);
    INA_TEST_ASSERT_EQUAL_STR(cstring, ina_str_cstr(str));  
}

INA_TEST_FIXTURE(string_mempool, ina_str_pfromcstr)
{
    ina_str_t str = NULL;
    const char *cstring = "USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION";
    str = ina_str_pfromcstr(cstring, data->pool);
    INA_TEST_ASSERT_NOT_NULL(str);
    INA_TEST_ASSERT_EQUAL_STR(cstring, ina_str_cstr(str));    
}


INA_TEST(string, ina_str_destroy)
{
    ina_str_t str = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_str_destroy(str));
    str = ina_str_fromcstr("test");
    INA_TEST_ASSERT_NOT_NULL(str);
    INA_TEST_ASSERT_SUCCEED(ina_str_destroy(str));
}

INA_TEST(string, ina_str_dup)
{
    ina_str_t s1 = ina_str_fromcstr("a simple string");
    INA_TEST_ASSERT_NOT_NULL(s1);
    ina_str_t s2 = ina_str_dup(s1);
    INA_TEST_ASSERT_NOT_NULL(s2);
    INA_TEST_ASSERT_EQUAL_STR(ina_str_cstr(s1), ina_str_cstr(s2));
    INA_TEST_ASSERT_NOT_SAME(s1, s2);
    INA_TEST_ASSERT_SUCCEED(ina_str_destroy(s1));
    INA_TEST_ASSERT_SUCCEED(ina_str_destroy(s2));
}

INA_TEST_FIXTURE(string_mempool, ina_str_pdup)
{
    ina_str_t s1 = ina_str_fromcstr("a simple string");
    INA_TEST_ASSERT_NOT_NULL(s1);
    ina_str_t s2 = ina_str_pdup(s1, data->pool);
    INA_TEST_ASSERT_NOT_NULL(s2);
    INA_TEST_ASSERT_EQUAL_STR(ina_str_cstr(s1), ina_str_cstr(s2));
    INA_TEST_ASSERT_NOT_SAME(s1, s2);
    INA_TEST_ASSERT_SUCCEED(ina_str_destroy(s1));
    INA_TEST_ASSERT_SUCCEED(ina_str_destroy(s2));
}

INA_TEST(string, ina_str_cstr)
{
    
}

INA_TEST(string, ina_str_cpy)
{
    
}

INA_TEST(string, ina_str_ncpy)
{
    
}

INA_TEST(string, ina_str_cat)
{
    
}

INA_TEST(string, ina_str_ncat)
{
    
}

INA_TEST(string, ina_str_len)
{
    
}

INA_TEST(string, ina_str_cmp)
{
    
}

INA_TEST(string, ina_str_ncmp)
{
    
}

INA_TEST(string, ina_str_casecmp)
{
    ina_str_t s1 = ina_str_fromcstr("abc");
    ina_str_t s2 = ina_str_fromcstr("ABC");
    ina_str_t s3 = ina_str_fromcstr("abC");
    ina_str_t s4 = ina_str_fromcstr("abCD");
    int result = 0;
    
    INA_TEST_ASSERT_EQUAL_INTEGER(0, ina_str_casecmp(s1, s2));
    INA_TEST_ASSERT_EQUAL_INTEGER(0, ina_str_casecmp(s2, s1));
    INA_TEST_ASSERT_EQUAL_INTEGER(0, ina_str_casecmp(s2, s3));
    INA_TEST_ASSERT_EQUAL_INTEGER(0, ina_str_casecmp(s3, s3));
    result = ina_str_casecmp(s4, s3);
    INA_TEST_ASSERT_TRUE((result > 0));
    result = ina_str_casecmp(s3, s4);
    INA_TEST_ASSERT_TRUE((result < 0));
}

INA_TEST(string, ina_str_str)
{
    
}

INA_TEST(string, ina_str_rchr)
{
    
}

INA_TEST(string, ina_str_vsprintf)
{
    
}

INA_TEST_SKIP(string, ina_str_snprintf)
{
    
}

INA_TEST_SKIP(string, ina_str_vsnprintf)
{
    
}

INA_TEST(string, simple_allocation_with_pool) 
{
    ina_str_t str1;
    ina_str_t str2;
    ina_mempool_t *pool;

    INA_TEST_ASSERT_SUCCEED(ina_mempool_create(&pool, 1024, 0, NULL));

    str1 = ina_str_pfromcstr("hallo", pool);
    INA_TEST_ASSERT_NOT_NULL(str1);
    INA_TEST_ASSERT_EQUAL_FLOATING(strlen("hallo"), ina_str_len(str1));
    str2 = ina_str_pdup(str1, pool);
    INA_TEST_ASSERT_NOT_NULL(str2);
    ina_str_destroy(str1);
    ina_str_destroy(str2);
}

INA_TEST(string, simple_allocation_without_pool)
{
	ina_str_t str1;
    ina_str_t str2;
    
    str1 = ina_str_fromcstr("hallo");
    INA_TEST_ASSERT_NOT_NULL(str1);
    INA_TEST_ASSERT_EQUAL_FLOATING(strlen("hallo"), ina_str_len(str1));
    str2 = ina_str_dup(str1);
    INA_TEST_ASSERT_NOT_NULL(str2);
    ina_str_destroy(str1);
    ina_str_destroy(str2);
}
