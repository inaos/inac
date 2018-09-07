/*
 * Copyright (c) 2012-2018 INAOS GmbH
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
    INA_TEST_ASSERT_SUCCEED(ina_mempool_new(&data->pool, 10*1024,INA_MEM_DYNAMIC, NULL));
    INA_TEST_ASSERT_NOT_NULL(data->pool);
}

INA_TEST_TEARDOWN(string_mempool)
{
    ina_mempool_free(data->pool);
    data->pool = NULL;
}

INA_TEST(string, ina_str_new)
{
    ina_str_t str = ina_str_new(0);
    INA_TEST_ASSERT_NOT_NULL(str);
    INA_TEST_ASSERT_EQUAL_STR("", ina_str_cstr(str));
    ina_str_free(str);
    str = ina_str_new(100);
    INA_TEST_ASSERT_NOT_NULL(str);
    INA_TEST_ASSERT_EQUAL_STR("", ina_str_cstr(str));
    ina_str_free(str);
}

INA_TEST_FIXTURE(string_mempool, ina_str_new_using_pool)
{
    ina_str_t str = ina_str_new_using_pool(0, data->pool);
    INA_TEST_ASSERT_NOT_NULL(str);
    INA_TEST_ASSERT_EQUAL_STR("", ina_str_cstr(str));
    str = ina_str_new_using_pool(100, data->pool);
    INA_TEST_ASSERT_NOT_NULL(str);
    INA_TEST_ASSERT_EQUAL_STR("", ina_str_cstr(str));
}

INA_TEST(string, ina_str_new_fromblk)
{
    ina_str_t str = NULL;
    char blk[] = "USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION";
    str = ina_str_new_fromblk(&blk[5], 4);
    INA_TEST_ASSERT_NOT_NULL(str);
    INA_TEST_ASSERT_EQUAL_STR("DATA", ina_str_cstr(str));
    ina_str_free(str);
}

INA_TEST_FIXTURE(string_mempool, ina_str_new_fromblk_using_pool)
{
    ina_str_t str = NULL;
    char blk[] = "USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION";
    str = ina_str_new_fromblk_using_pool(&blk[5], 4, data->pool);
    INA_TEST_ASSERT_NOT_NULL(str);
    INA_TEST_ASSERT_EQUAL_STR("DATA", ina_str_cstr(str));
}

INA_TEST(string, ina_str_new_fromcstr)
{
    ina_str_t str = NULL;
    const char *cstring = "USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION";
    str = ina_str_new_fromcstr(cstring);
    INA_TEST_ASSERT_NOT_NULL(str);
    INA_TEST_ASSERT_EQUAL_STR(cstring, ina_str_cstr(str));
    ina_str_free(str);
}

INA_TEST_FIXTURE(string_mempool, ina_str_new_fromcstr_using_pool)
{
    ina_str_t str = NULL;
    const char *cstring = "USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION";
    str = ina_str_new_fromcstr_using_pool(cstring, data->pool);
    INA_TEST_ASSERT_NOT_NULL(str);
    INA_TEST_ASSERT_EQUAL_STR(cstring, ina_str_cstr(str));
}


INA_TEST(string, ina_str_free)
{
    ina_str_t str = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_str_free(str));
    str = ina_str_new_fromcstr("test");
    INA_TEST_ASSERT_NOT_NULL(str);
    INA_TEST_ASSERT_SUCCEED(ina_str_free(str));
}

INA_TEST(string, ina_str_dup)
{
    ina_str_t s1 = ina_str_new_fromcstr("a simple string");
    ina_str_t s2;
    INA_TEST_ASSERT_NOT_NULL(s1);
    s2 = ina_str_dup(s1);
    INA_TEST_ASSERT_NOT_NULL(s2);
    INA_TEST_ASSERT_EQUAL_STR(ina_str_cstr(s1), ina_str_cstr(s2));
    INA_TEST_ASSERT_NOT_SAME(s1, s2);
    INA_TEST_ASSERT_SUCCEED(ina_str_free(s1));
    INA_TEST_ASSERT_SUCCEED(ina_str_free(s2));
}

INA_TEST_FIXTURE(string_mempool, ina_str_dup_using_pool)
{
    ina_str_t s1 = ina_str_new_fromcstr("a simple string");
    ina_str_t s2;
    INA_TEST_ASSERT_NOT_NULL(s1);
    s2 = ina_str_dup_using_pool(s1, data->pool);
    INA_TEST_ASSERT_NOT_NULL(s2);
    INA_TEST_ASSERT_EQUAL_STR(ina_str_cstr(s1), ina_str_cstr(s2));
    INA_TEST_ASSERT_NOT_SAME(s1, s2);
}

INA_TEST(string, ina_str_cstr)
{
    const char* c_str;
    ina_str_t str = ina_str_new_fromcstr("an INAC string");
    INA_TEST_ASSERT_NOT_NULL(str);
    c_str = ina_str_cstr(str);
    INA_TEST_ASSERT_NOT_NULL(c_str);
    INA_TEST_ASSERT_TRUE(strcmp("an INAC string", c_str) == 0);
    ina_str_free(str);
}

INA_TEST(string, ina_str_cpy)
{
   ina_str_t src =  NULL;
   ina_str_t dest = NULL;

   src = ina_str_new_fromcstr("a string to copy");
   INA_TEST_ASSERT_NOT_NULL(src);
   dest = ina_str_new(ina_str_len(src));
   dest = ina_str_cpy(dest, src);
   INA_TEST_ASSERT_NOT_NULL(dest);
   INA_TEST_ASSERT_NOT_SAME(src, dest);
   INA_TEST_ASSERT_TRUE(strcmp(ina_str_cstr(src), ina_str_cstr(dest)) == 0);
   ina_str_free(src);
   ina_str_free(dest);
}

INA_TEST(string, issue_355)
{
   ina_str_t src =  NULL;
   ina_str_t dest = NULL;

   src = ina_str_new_fromcstr("a string to copy");
   INA_TEST_ASSERT_NOT_NULL(src);

   dest = ina_str_new(1024);
   INA_TEST_ASSERT_NOT_NULL(dest);
   dest = ina_str_cpy(dest, src);
   INA_TEST_ASSERT_EQUAL_STR("a string to copy", ina_str_cstr(dest));
   INA_TEST_ASSERT_NOT_NULL(dest);
   dest = ina_str_catcstr(dest, "test");
   INA_TEST_ASSERT_NOT_NULL(dest);
   INA_TEST_ASSERT_EQUAL_STR("a string to copytest", ina_str_cstr(dest));
   ina_str_free(src);
   ina_str_free(dest);
}

INA_TEST(string, issue_356)
{
   ina_str_t dest =  NULL;
#define fmt "POST %s%s HTTP/1.1 Host: %s Connection: keep-alive Content-Length: %d Content-Type: application/json Accept: */*"

   dest = ina_str_sprintf(fmt, "1234567890", "1234567890", "host", 123);
   INA_TEST_ASSERT_NOT_NULL(dest);
   INA_TEST_ASSERT_EQUAL_STR("POST 12345678901234567890 HTTP/1.1 Host: host Connection: keep-alive Content-Length: 123 Content-Type: application/json Accept: */*",
                             ina_str_cstr(dest));
   ina_str_free(dest);
}

INA_TEST(string, ina_str_ncpy)
{
    ina_str_t src =  NULL;
    ina_str_t dest = NULL;

    src = ina_str_new_fromcstr("a string to copy");
    INA_TEST_ASSERT_NOT_NULL(src);
    dest = ina_str_new(ina_str_len(src) + 1);
    dest = ina_str_ncpy(dest, src, 8);
    INA_TEST_ASSERT_NOT_NULL(dest);
    INA_TEST_ASSERT_NOT_SAME(src, dest);
    INA_TEST_ASSERT_TRUE(strcmp("a string", ina_str_cstr(dest)) == 0);

    ina_str_free(src);
    ina_str_free(dest);
}

INA_TEST_SKIP(string, ina_str_cat)
{
    ina_str_t str = ina_str_new(128);
    ina_str_t part1 = ina_str_new_fromcstr("part1");
    ina_str_t part2 = ina_str_new_fromcstr("part2");
    ina_str_t part3 = ina_str_new_fromcstr("part3");
    str = ina_str_cat(str, part1);
    str = ina_str_cat(str, part2);
    str = ina_str_cat(str, part3);
    INA_TEST_ASSERT_TRUE(strcmp("part1part2part3", ina_str_cstr(str)) == 0);
    ina_str_free(str);
    ina_str_free(part1);
    ina_str_free(part2);
    ina_str_free(part3);
    
    str = ina_str_new(0);
    part1 = ina_str_new_fromcstr("part1");
    part2 = ina_str_new_fromcstr("part2");
    part3 = ina_str_new_fromcstr("part3");
    str = ina_str_cat(str, part1);
    str = ina_str_cat(str, part2);
    str = ina_str_cat(str, part3);
    INA_TEST_ASSERT_TRUE(strcmp("part1part2part3", ina_str_cstr(str)) == 0);
    ina_str_free(str);
    ina_str_free(part1);
    ina_str_free(part2);
    ina_str_free(part3);
    
    str = ina_str_new(0);
    str = ina_str_cat(str, ina_str_cat(str, ina_str_cat(str, ina_str_new_fromcstr("test"))));
    INA_TEST_ASSERT_EQUAL_STR("testtesttesttest", ina_str_cstr(str));
    ina_str_free(str);
    
    str = ina_str_cat(ina_str_new(0), ina_str_new(0));
    INA_TEST_ASSERT_EQUAL_STR(str, ina_str_cstr(""));
    ina_str_free(str);
    
    str = ina_str_cat(ina_str_new(0), NULL);
    INA_TEST_ASSERT_EQUAL_STR(str, ina_str_cstr(""));
    ina_str_free(str);
}

INA_TEST_FIXTURE(string_mempool, ina_str_cat)
{
    ina_str_t str = ina_str_new_using_pool(128, data->pool);
    ina_str_t part1 = ina_str_new_fromcstr_using_pool("part1", data->pool);
    ina_str_t part2 = ina_str_new_fromcstr_using_pool("part2", data->pool);
    ina_str_t part3 = ina_str_new_fromcstr_using_pool("part3", data->pool);
    str = ina_str_cat_using_pool(str, part1, data->pool);
    str = ina_str_cat_using_pool(str, part2, data->pool);
    str = ina_str_cat_using_pool(str, part3, data->pool);
    INA_TEST_ASSERT_TRUE(strcmp("part1part2part3", ina_str_cstr(str)) == 0);

    str = ina_str_new_using_pool(0, data->pool);
    part1 = ina_str_new_fromcstr_using_pool("part1", data->pool);
    part2 = ina_str_new_fromcstr_using_pool("part2", data->pool);
    part3 = ina_str_new_fromcstr_using_pool("part3", data->pool);
    str = ina_str_cat_using_pool(str, part1, data->pool);
    str = ina_str_cat_using_pool(str, part2, data->pool);
    str = ina_str_cat_using_pool(str, part3, data->pool);
    INA_TEST_ASSERT_TRUE(strcmp("part1part2part3", ina_str_cstr(str)) == 0);

    str = ina_str_new_using_pool(0, data->pool);
    str = ina_str_cat_using_pool(str, ina_str_cat_using_pool(str, ina_str_cat_using_pool(str, ina_str_new_fromcstr_using_pool("test", data->pool), data->pool), data->pool), data->pool);
    INA_TEST_ASSERT_EQUAL_STR("testtesttesttest", ina_str_cstr(str));

    str = ina_str_cat_using_pool(ina_str_new_using_pool(0, data->pool), ina_str_new_using_pool(0, data->pool), data->pool);
    INA_TEST_ASSERT_EQUAL_STR(str, ina_str_cstr(""));

    str = ina_str_cat_using_pool(ina_str_new_using_pool(0, data->pool), NULL, data->pool);
    INA_TEST_ASSERT_EQUAL_STR(str, ina_str_cstr(""));
}

INA_TEST(string, ina_str_catcstr)
{
    ina_str_t ref_str = NULL;
    ina_str_t str = ina_str_new(128);
    ref_str = str;
    str = ina_str_catcstr(str, "part1");
    INA_TEST_ASSERT_SAME(ref_str, str);
    str = ina_str_catcstr(str, "part2");
    INA_TEST_ASSERT_SAME(ref_str, str);
    str = ina_str_catcstr(str, "part3");
    INA_TEST_ASSERT_SAME(ref_str, str);
    INA_TEST_ASSERT_TRUE(strcmp("part1part2part3", ina_str_cstr(str)) == 0);
    ina_str_free(str);
}

INA_TEST_FIXTURE(string_mempool, ina_str_catcstr)
{
    ina_str_t ref_str = NULL;
    ina_str_t str = ina_str_new_using_pool(128, data->pool);
    ref_str = str;
    str = ina_str_catcstr_using_pool(str, "part1", data->pool);
    INA_TEST_ASSERT_SAME(ref_str, str);
    str = ina_str_catcstr_using_pool(str, "part2", data->pool);
    INA_TEST_ASSERT_SAME(ref_str, str);
    str = ina_str_catcstr_using_pool(str, "part3", data->pool);
    INA_TEST_ASSERT_SAME(ref_str, str);
    INA_TEST_ASSERT_TRUE(strcmp("part1part2part3", ina_str_cstr(str)) == 0);
}

INA_TEST(string, ina_str_ncat)
{
    ina_str_t ref_str = NULL;
    ina_str_t str = ina_str_new(128);
    ina_str_t part1 = ina_str_new_fromcstr("part1x");
    ina_str_t part2;
    ina_str_t part3;
    ref_str = str;
    INA_TEST_ASSERT_SAME(ref_str, str);
    part2 = ina_str_new_fromcstr("part2x");
    INA_TEST_ASSERT_SAME(ref_str, str);
    part3 = ina_str_new_fromcstr("part3x");
    INA_TEST_ASSERT_SAME(ref_str, str);    
    str = ina_str_ncat(str, part1, 5);
    str = ina_str_ncat(str, part2, 5);
    str = ina_str_ncat(str, part3, 5);
    INA_TEST_ASSERT_TRUE(strcmp("part1part2part3", ina_str_cstr(str)) == 0);
    ina_str_free(str);
    ina_str_free(part1);
    ina_str_free(part2);
    ina_str_free(part3);
}

INA_TEST_FIXTURE(string_mempool, ina_str_ncat)
{
    ina_str_t ref_str = NULL;
    ina_str_t str = ina_str_new_using_pool(128, data->pool);
    ina_str_t part1 = ina_str_new_fromcstr_using_pool("part1x", data->pool);
    ina_str_t part2;
    ina_str_t part3;
    ref_str = str;
    INA_TEST_ASSERT_SAME(ref_str, str);
    part2 = ina_str_new_fromcstr_using_pool("part2x", data->pool);
    INA_TEST_ASSERT_SAME(ref_str, str);
    part3 = ina_str_new_fromcstr_using_pool("part3x", data->pool);
    INA_TEST_ASSERT_SAME(ref_str, str);
    str = ina_str_ncat_using_pool(str, part1, 5, data->pool);
    str = ina_str_ncat_using_pool(str, part2, 5, data->pool);
    str = ina_str_ncat_using_pool(str, part3, 5, data->pool);
    INA_TEST_ASSERT_TRUE(strcmp("part1part2part3", ina_str_cstr(str)) == 0);
}

INA_TEST(string, ina_str_ncatcstr)
{
    ina_str_t str = ina_str_new(128);
    str = ina_str_ncat(str, "part1x", 5);
    str = ina_str_ncat(str, "part2x", 5);
    str = ina_str_ncat(str, "part3x", 5);
    INA_TEST_ASSERT_TRUE(strcmp("part1part2part3", ina_str_cstr(str)) == 0);
    ina_str_free(str);
}

INA_TEST_FIXTURE(string_mempool, ina_str_ncatcstr)
{
    ina_str_t str = ina_str_new_using_pool(128, data->pool);
    str = ina_str_ncat_using_pool(str, "part1x", 5, data->pool);
    str = ina_str_ncat_using_pool(str, "part2x", 5, data->pool);
    str = ina_str_ncat_using_pool(str, "part3x", 5, data->pool);
    INA_TEST_ASSERT_TRUE(strcmp("part1part2part3", ina_str_cstr(str)) == 0);
}

INA_TEST(string, ina_str_len)
{
    ina_str_t str = ina_str_new_fromcstr("an INAC string");
    ina_str_t empty = ina_str_new_fromcstr("");
    INA_TEST_ASSERT_NOT_NULL(str);
    INA_TEST_ASSERT_TRUE(strlen("an INAC string") == ina_str_len(str));
    INA_TEST_ASSERT_EQUAL_SIZE_T(0, ina_str_len(empty));
    INA_TEST_ASSERT_EQUAL_SIZE_T(0, ina_str_len(NULL));
    ina_str_free(str);
    ina_str_free(empty);
    
    str = ina_str_new_fromcstr("an ");
    INA_TEST_ASSERT_NOT_NULL(str);
    INA_TEST_ASSERT_EQUAL_SIZE_T(3, ina_str_len(str));
    str = ina_str_catcstr(str, "1234567890");
    INA_TEST_ASSERT_EQUAL_SIZE_T(13, ina_str_len(str));
    INA_TEST_ASSERT_EQUAL_STR("an 1234567890", ina_str_cstr(str));
    ina_str_free(str);
}

INA_TEST(string, ina_str_size)
{
    ina_str_t str = ina_str_new_fromcstr("an INAC string");
    ina_str_t empty = ina_str_new_fromcstr("");
    INA_TEST_ASSERT_NOT_NULL(str);
    INA_TEST_ASSERT_TRUE((strlen("an INAC string")+1) == ina_str_size(str));
    INA_TEST_ASSERT_EQUAL_SIZE_T(1, ina_str_size(empty));
    INA_TEST_ASSERT_EQUAL_SIZE_T(0, ina_str_size(NULL));
    ina_str_free(str);
    ina_str_free(empty);

    str = ina_str_new_fromcstr("an ");
    INA_TEST_ASSERT_NOT_NULL(str);
    INA_TEST_ASSERT_EQUAL_SIZE_T(4, ina_str_size(str));
    str = ina_str_catcstr(str, "1234567890");
    INA_TEST_ASSERT_EQUAL_SIZE_T(14, ina_str_size(str));
    INA_TEST_ASSERT_EQUAL_STR("an 1234567890", ina_str_cstr(str));
    ina_str_free(str);
}

INA_TEST(string, ina_str_available)
{
    ina_str_t str = ina_str_new_fromcstr("1234567890");
    INA_TEST_ASSERT_EQUAL_SIZE_T(0, ina_str_available(str));
    ina_str_free(str);
    str = ina_str_new(100);
    INA_TEST_ASSERT_EQUAL_SIZE_T(100, ina_str_available(str));
    ina_str_catcstr(str, "1234567890");
    INA_TEST_ASSERT_EQUAL_SIZE_T(90, ina_str_available(str));
    ina_str_free(str);
}

INA_TEST(string, ina_str_cmp)
{
    ina_str_t s1 = ina_str_new_fromcstr("abc");
    ina_str_t s2 = ina_str_new_fromcstr("abc");
    ina_str_t s3 = ina_str_new_fromcstr("abC");
    ina_str_t s4 = ina_str_new_fromcstr("abCD");
    int result = 0;

    INA_TEST_ASSERT_NOT_NULL(s1);
    INA_TEST_ASSERT_NOT_NULL(s2);
    INA_TEST_ASSERT_NOT_NULL(s3);
    INA_TEST_ASSERT_NOT_NULL(s4); 
 
    INA_TEST_ASSERT_EQUAL_INT(0, ina_str_casecmp(s1, s2));
    INA_TEST_ASSERT_EQUAL_INT(0, ina_str_casecmp(s2, s1));
    INA_TEST_ASSERT_EQUAL_INT(0, ina_str_casecmp(s2, s3));
    INA_TEST_ASSERT_EQUAL_INT(0, ina_str_casecmp(s3, s3));
    result = ina_str_casecmp(s4, s3);
    INA_TEST_ASSERT_TRUE((result > 0));
    result = ina_str_casecmp(s3, s4);
    INA_TEST_ASSERT_TRUE((result < 0));
    ina_str_free(s1);
    ina_str_free(s2);
    ina_str_free(s3);
    ina_str_free(s4);
}

INA_TEST(string, ina_str_ncmp)
{

}

INA_TEST(string, ina_str_casecmp)
{
    ina_str_t s1 = ina_str_new_fromcstr("abc");
    ina_str_t s2 = ina_str_new_fromcstr("ABC");
    ina_str_t s3 = ina_str_new_fromcstr("abC");
    ina_str_t s4 = ina_str_new_fromcstr("abCD");
    int result = 0;

    INA_TEST_ASSERT_EQUAL_INT(0, ina_str_casecmp(s1, s2));
    INA_TEST_ASSERT_EQUAL_INT(0, ina_str_casecmp(s2, s1));
    INA_TEST_ASSERT_EQUAL_INT(0, ina_str_casecmp(s2, s3));
    INA_TEST_ASSERT_EQUAL_INT(0, ina_str_casecmp(s3, s3));
    result = ina_str_casecmp(s4, s3);
    INA_TEST_ASSERT_TRUE((result > 0));
    result = ina_str_casecmp(s3, s4);
    INA_TEST_ASSERT_TRUE((result < 0));
    ina_str_free(s1);
    ina_str_free(s2);
    ina_str_free(s3);
    ina_str_free(s4);
}

INA_TEST(string, ina_str_str)
{
    ina_str_t str =  ina_str_new_fromcstr("search a substring in a string.");
    ina_str_t substr = ina_str_new_fromcstr("substring");
    ina_str_t x = ina_str_new_fromcstr("x");
    ina_str_t empty = ina_str_new_fromcstr("");
    INA_TEST_ASSERT_EQUAL_STR("substring in a string.",ina_str_str(str,  substr));
    INA_TEST_ASSERT_NULL(ina_str_str(str, x));
    INA_TEST_ASSERT_NULL(ina_str_str(str, empty));
    INA_TEST_ASSERT_NULL(ina_str_str(str, NULL));
    ina_str_free(str);
    ina_str_free(x);
    ina_str_free(empty);
    ina_str_free(substr);
}

INA_TEST(string, ina_str_strcstr)
{
    ina_str_t str =  ina_str_new_fromcstr("search a substring in a string.");
    INA_TEST_ASSERT_EQUAL_STR("substring in a string.", ina_str_strcstr(str, "substring"));
    INA_TEST_ASSERT_NULL(ina_str_strcstr(str, "x"));
    INA_TEST_ASSERT_NULL(ina_str_strcstr(str, ""));
    INA_TEST_ASSERT_NULL(ina_str_strcstr(str, NULL));
    ina_str_free(str);
}

INA_TEST(string, ina_str_rchr)
{
    ina_str_t str = ina_str_new_fromcstr("search a x in a string with xxx in it.");
    INA_TEST_ASSERT_NOT_NULL(str);
    INA_TEST_ASSERT_EQUAL_STR("x in it.", ina_str_rchr(str, 'x'));
    INA_TEST_ASSERT_NULL(ina_str_rchr(str, 'y'));
    INA_TEST_ASSERT_NULL(ina_str_rchr(str, '\0'));
    ina_str_free(str);
}

INA_TEST(string, ina_str_toupper)
{
    ina_str_t str = ina_str_new_fromcstr("ABabcde123zZ+-=)(/&%+)");
    INA_TEST_ASSERT_NULL(ina_str_toupper(NULL));
    INA_TEST_ASSERT_EQUAL_STR("ABABCDE123ZZ+-=)(/&%+)", ina_str_toupper(str));
    ina_str_free(str);
}

INA_TEST(string, ina_str_tolower)
{
    ina_str_t str = ina_str_new_fromcstr("abABCDE123zZ+-=)(/&%+)");
    INA_TEST_ASSERT_NULL(ina_str_tolower(NULL));
    INA_TEST_ASSERT_EQUAL_STR("ababcde123zz+-=)(/&%+)", ina_str_tolower(str));
    ina_str_free(str);
}

INA_TEST(string, ina_str_truncte_empty_string)
{
    ina_str_t str = ina_str_new_fromcstr("");
    INA_TEST_ASSERT_NOT_NULL(str);
    ina_str_truncate(str, 0);
    INA_TEST_ASSERT_EQUAL_STR("", ina_str_cstr(str));
    INA_TEST_ASSERT_EQUAL_SIZE_T(0, ina_str_len(str));
    ina_str_free(str);
}

INA_TEST(string, ina_str_truncate_zero)
{
    ina_str_t str = ina_str_new_fromcstr("Abc def   ");
    INA_TEST_ASSERT_NOT_NULL(str);
    INA_TEST_ASSERT_EQUAL_STR("Abc def   ", ina_str_cstr(str));
    ina_str_truncate(str, 0);
    INA_TEST_ASSERT_EQUAL_SIZE_T(0, ina_str_len(str));
    INA_TEST_ASSERT_EQUAL_STR("", ina_str_cstr(str));
    INA_TEST_ASSERT_NOT_NULL(str);
    ina_str_free(str);
}

INA_TEST(string, ina_str_truncate_pos)
{
    ina_str_t str = ina_str_new_fromcstr("Abc def   ");
    INA_TEST_ASSERT_NOT_NULL(str);
    INA_TEST_ASSERT_EQUAL_STR("Abc def   ", ina_str_cstr(str));
    ina_str_truncate(str, 3);
    INA_TEST_ASSERT_EQUAL_SIZE_T(3, ina_str_len(str));
    INA_TEST_ASSERT_EQUAL_STR("Abc", ina_str_cstr(str));
    INA_TEST_ASSERT_NOT_NULL(str);
    ina_str_free(str);
}

INA_TEST(string, ina_str_trim)
{
    ina_str_t str = ina_str_new_fromcstr(" test ");
    INA_TEST_ASSERT_EQUAL_STR(" test ", ina_str_trim(str, NULL));
    INA_TEST_ASSERT_EQUAL_STR("test", ina_str_trim(str, " "));
    INA_TEST_ASSERT_EQUAL_STR("es", ina_str_trim(str, "t"));
    INA_TEST_ASSERT_EQUAL_STR("s", ina_str_trim(str, "e"));
    ina_str_free(str);
}

INA_TEST(string, ina_str_split)
{
    ina_str_t *tokens;
    size_t count;
    ina_str_t str = ina_str_new_fromcstr("xx--yy--zz--c-c");
    char *test[] = {"a", "b", "c", "d", "e"};

    INA_TEST_ASSERT_NULL((tokens = ina_str_split(NULL, NULL, &count)));
    INA_TEST_ASSERT_EQUAL_SIZE_T(0, count);
    INA_TEST_ASSERT_SUCCEED(ina_str_split_free_tokens(tokens));
    INA_TEST_ASSERT_NULL((tokens = ina_str_split("", NULL, &count)));
    INA_TEST_ASSERT_EQUAL_SIZE_T(0, count);
    INA_TEST_ASSERT_SUCCEED(ina_str_split_free_tokens(tokens));
    INA_TEST_ASSERT_NULL((tokens = ina_str_split("", "", &count)));
    INA_TEST_ASSERT_EQUAL_SIZE_T(0, count);
    INA_TEST_ASSERT_SUCCEED(ina_str_split_free_tokens(tokens));
    INA_TEST_ASSERT_NULL((tokens = ina_str_split("1-1-1", NULL, &count)));
    INA_TEST_ASSERT_EQUAL_SIZE_T(0, count);
    INA_TEST_ASSERT_SUCCEED(ina_str_split_free_tokens(tokens));
    INA_TEST_ASSERT_NULL((tokens = ina_str_split(NULL, "", &count)));
    INA_TEST_ASSERT_EQUAL_SIZE_T(0, count);
    INA_TEST_ASSERT_SUCCEED(ina_str_split_free_tokens(tokens));
    INA_TEST_ASSERT_NULL((tokens = ina_str_split("1-1-1", "", &count)));
    INA_TEST_ASSERT_EQUAL_SIZE_T(0, count);
    INA_TEST_ASSERT_SUCCEED(ina_str_split_free_tokens(tokens));
    
    
    tokens = ina_str_split(str, "--", &count);
    INA_TEST_ASSERT_NOT_NULL(tokens);
    INA_TEST_ASSERT_EQUAL_SIZE_T(4, count);
    INA_TEST_ASSERT_EQUAL_STR("xx",  tokens[0]);
    INA_TEST_ASSERT_EQUAL_STR("yy",  tokens[1]);
    INA_TEST_ASSERT_EQUAL_STR("zz",  tokens[2]);
    INA_TEST_ASSERT_EQUAL_STR("c-c", tokens[3]);
    
    INA_ASSERT_SUCCEED(ina_str_split_free_tokens(tokens));
    ina_str_free(str);
    
    count = 0;
    tokens = ina_str_split("a b c d e", " ", &count);
    INA_TEST_ASSERT_NOT_NULL(tokens);
    INA_TEST_ASSERT_EQUAL_SIZE_T(5, count);
    while (count--) {
        INA_TEST_ASSERT_EQUAL_STR(test[count], ina_str_cstr(tokens[count]));
    }
    INA_TEST_ASSERT_SUCCEED(ina_str_split_free_tokens(tokens));
}

INA_TEST(string, ina_str_tok)
{
    ina_str_t str = ina_str_new_fromcstr("a b c d   ");
    int c = (int)'a';
    const char *ret;
    char *next_token = NULL;
    
    INA_TEST_ASSERT_NULL(ina_str_tok(str, NULL, &next_token));
    INA_TEST_ASSERT_NULL(ina_str_tok(str, "", &next_token));
    INA_TEST_ASSERT_NULL(ina_str_tok(NULL, " ", &next_token));
    
    ret = ina_str_tok(str, " ", &next_token);
    
    while (ret) {
        INA_TEST_ASSERT_EQUAL_INT(c, (int)*ret);
        ret = ina_str_tok(NULL, " ", &next_token);
        c++;
    }
    INA_TEST_ASSERT_EQUAL_INT(5, c-96);
    ina_str_free(str);
    
    str = ina_str_new_fromcstr("a b-cxd   ");
    ret = ina_str_tok(str, " ", &next_token);
    INA_TEST_ASSERT_EQUAL_STR("a", ret);
    ret = ina_str_tok(NULL, "-", &next_token);
    INA_TEST_ASSERT_EQUAL_STR("b", ret);
    ret = ina_str_tok(NULL, "x", &next_token);
    INA_TEST_ASSERT_EQUAL_STR("c", ret);
    ret = ina_str_tok(NULL, " ", &next_token);
    INA_TEST_ASSERT_EQUAL_STR("d", ret);
    INA_TEST_ASSERT_EQUAL_STR("a", ina_str_cstr(str));
    INA_TEST_ASSERT_EQUAL_SIZE_T(10, ina_str_len(str));
    ina_str_adjust_len(str);
    INA_TEST_ASSERT_EQUAL_SIZE_T(1, ina_str_len(str));
    ina_str_free(str);
}

INA_TEST(string, ina_str_adjust_len)
{
    ina_str_t str = ina_str_new(128);
    str = ina_str_catcstr(str, "12345");
    INA_TEST_ASSERT_EQUAL_SIZE_T(5, ina_str_len(str));
    strcat(str, "67890");
    INA_TEST_ASSERT_EQUAL_STR("1234567890", ina_str_cstr(str));
    INA_TEST_ASSERT_EQUAL_SIZE_T(5, ina_str_len(str));
    ina_str_adjust_len(str);
    INA_TEST_ASSERT_EQUAL_STR("1234567890", ina_str_cstr(str));
    INA_TEST_ASSERT_EQUAL_SIZE_T(10, ina_str_len(str));
    ina_str_free(str);
}

INA_TEST(string, ina_str_substr)
{
    ina_str_t substr = NULL;
    ina_str_t str = ina_str_new_fromcstr("extract a substring from a string");
    INA_TEST_ASSERT_NOT_NULL(str);
    substr = ina_str_substr(str, 10, 18);
    INA_TEST_ASSERT_NOT_NULL(substr);
    INA_TEST_ASSERT_EQUAL_STR("substring", substr);
    ina_str_free(substr);

    substr = ina_str_substr(str, 0, ina_str_len(str));
    INA_TEST_ASSERT_NOT_NULL(substr);
    INA_TEST_ASSERT_EQUAL_STR("extract a substring from a string", substr);
    ina_str_free(substr);

    substr = ina_str_substr(str, 10, ina_str_len(str));
    INA_TEST_ASSERT_NOT_NULL(substr);
    INA_TEST_ASSERT_EQUAL_STR("substring from a string", substr);
    ina_str_free(substr);

    substr = ina_str_substr(str, 4, 2);
    INA_TEST_ASSERT_NOT_NULL(substr);
    INA_TEST_ASSERT_EQUAL_STR("", substr);
    ina_str_free(substr);

    substr = ina_str_substr(str, 0, 0);
    INA_TEST_ASSERT_NOT_NULL(substr);
    INA_TEST_ASSERT_EQUAL_STR("e", substr);
    ina_str_free(substr);

    str = ina_str_new_fromcstr("");
    INA_TEST_ASSERT_NOT_NULL(str);
    substr = ina_str_substr(str, 10, 18);
    INA_TEST_ASSERT_NOT_NULL(substr);
    INA_TEST_ASSERT_EQUAL_STR("", substr);
    ina_str_free(substr);
    ina_str_free(str);

}

INA_TEST_FIXTURE(string_mempool, ina_str_substr)
{
    ina_str_t substr = NULL;
    ina_str_t str = ina_str_new_fromcstr_using_pool("exctrat a substring from a string", data->pool);
    INA_TEST_ASSERT_NOT_NULL(str);
    substr = ina_str_substr_using_pool(str, 10, 18, data->pool);
    INA_TEST_ASSERT_NOT_NULL(substr);
    INA_TEST_ASSERT_EQUAL_STR("substring", substr);
}

INA_TEST(string, ina_str_sprintf)
{
    ina_str_t str = ina_str_sprintf("format:%s", "string");
    INA_TEST_ASSERT_EQUAL_STR("format:string", ina_str_cstr(str));
    INA_TEST_ASSERT_EQUAL_SIZE_T(13, ina_str_len(str));
    ina_str_free(str);
}

INA_TEST(string, ina_str_snprintf)
{
    int len;
    ina_str_t str1 = ina_str_new(128);
    ina_str_t str2 = str1;

    len = ina_str_snprintf(&str1, 128, "format:%s", "string");
    INA_TEST_ASSERT_EQUAL_STR("format:string", ina_str_cstr(str1));
    INA_TEST_ASSERT_EQUAL_INT(13, len);
    INA_TEST_ASSERT_SAME(str1, str2);
    ina_str_free(str1);

    str1 = ina_str_new(5);
    str2 = str1; 
    len = ina_str_snprintf(&str1, 5, "format:%s", "string");
    INA_TEST_ASSERT_EQUAL_STR("format:string", ina_str_cstr(str1));
    INA_TEST_ASSERT_EQUAL_INT(13, len);
    INA_TEST_ASSERT_EQUAL_SIZE_T(13, ina_str_len(str1));
    INA_TEST_ASSERT_NOT_SAME(str1, str2);
    ina_str_free(str1);
}

INA_TEST(string, simple_allocation_with_pool)
{
    ina_str_t str1;
    ina_str_t str2;
    ina_mempool_t *pool;

    INA_TEST_ASSERT_SUCCEED(ina_mempool_new(&pool, 1024, 0, NULL));

    str1 = ina_str_new_fromcstr_using_pool("hallo", pool);
    INA_TEST_ASSERT_NOT_NULL(str1);
    INA_TEST_ASSERT_EQUAL_SIZE_T(strlen("hallo"), ina_str_len(str1));
    str2 = ina_str_dup_using_pool(str1, pool);
    INA_TEST_ASSERT_NOT_NULL(str2);
}

INA_TEST(string, simple_allocation_without_pool)
{
    ina_str_t str1;
    ina_str_t str2;

    str1 = ina_str_new_fromcstr("hallo");
    INA_TEST_ASSERT_NOT_NULL(str1);
    INA_TEST_ASSERT_EQUAL_SIZE_T(strlen("hallo"), ina_str_len(str1));
    str2 = ina_str_dup(str1);
    INA_TEST_ASSERT_NOT_NULL(str2);
    ina_str_free(str1);
    ina_str_free(str2);
}

INA_TEST(string, ina_str_wildcard_match)
{
    ina_str_t ts;

#define _INA_TEST_STRING_WILDCARD_TEST_OK(teme, wildcard)                  \
    ts = ina_str_new_fromcstr(teme);                                       \
    INA_TEST_ASSERT_SUCCEED(ina_str_wildcard_match(ts, wildcard));         \
    ina_str_free(ts);

#define _INA_TEST_STRING_WILDCARD_TEST_NOK(teme, wildcard)                 \
    ts = ina_str_new_fromcstr(teme);                                       \
    INA_TEST_ASSERT_FAILED(ina_str_wildcard_match(ts, wildcard));      \
    ina_str_free(ts);

    /* Cases with repeating character sequences. */
    _INA_TEST_STRING_WILDCARD_TEST_OK("abcccd", "*ccd");
    _INA_TEST_STRING_WILDCARD_TEST_OK("mississipissippi", "*issip*ss*");
    _INA_TEST_STRING_WILDCARD_TEST_NOK("xxxx*zzzzzzzzy*f", "xxxx*zzy*fffff");
    _INA_TEST_STRING_WILDCARD_TEST_OK("xxxx*zzzzzzzzy*f", "xxx*zzy*f");
    _INA_TEST_STRING_WILDCARD_TEST_NOK("xxxxzzzzzzzzyf", "xxxx*zzy*fffff");
    _INA_TEST_STRING_WILDCARD_TEST_OK("xxxxzzzzzzzzyf", "xxxx*zzy*f");
    _INA_TEST_STRING_WILDCARD_TEST_OK("xyxyxyzyxyz", "xy*z*xyz");
    _INA_TEST_STRING_WILDCARD_TEST_OK("mississippi", "*sip*");
    _INA_TEST_STRING_WILDCARD_TEST_OK("xyxyxyxyz", "xy*xyz");
    _INA_TEST_STRING_WILDCARD_TEST_OK("mississippi", "mi*sip*");
    _INA_TEST_STRING_WILDCARD_TEST_OK("ababac", "*abac*");
    _INA_TEST_STRING_WILDCARD_TEST_OK("ababac", "*abac*");
    _INA_TEST_STRING_WILDCARD_TEST_OK("aaazz", "a*zz*");
    _INA_TEST_STRING_WILDCARD_TEST_NOK("a12b12", "*12*23");
    _INA_TEST_STRING_WILDCARD_TEST_NOK("a12b12", "a12b");
    _INA_TEST_STRING_WILDCARD_TEST_OK("a12b12", "*12*12*");

    /* Additional cases where the '*' char appears in the tame string. */
    _INA_TEST_STRING_WILDCARD_TEST_OK("*", "*");
    _INA_TEST_STRING_WILDCARD_TEST_OK("a*abab", "a*b");
    _INA_TEST_STRING_WILDCARD_TEST_OK("a*r", "a*");
    _INA_TEST_STRING_WILDCARD_TEST_NOK("a*ar", "a*aar");

    /* More double wildcard scenarios. */
    _INA_TEST_STRING_WILDCARD_TEST_OK("XYXYXYZYXYz", "XY*Z*XYz");
    _INA_TEST_STRING_WILDCARD_TEST_OK("missisSIPpi", "*SIP*");
    _INA_TEST_STRING_WILDCARD_TEST_OK("mississipPI", "*issip*PI");
    _INA_TEST_STRING_WILDCARD_TEST_OK("xyxyxyxyz", "xy*xyz");
    _INA_TEST_STRING_WILDCARD_TEST_OK("miSsissippi", "mi*sip*");
    _INA_TEST_STRING_WILDCARD_TEST_NOK("miSsissippi", "mi*Sip*");
    _INA_TEST_STRING_WILDCARD_TEST_OK("abAbac", "*Abac*");
    _INA_TEST_STRING_WILDCARD_TEST_OK("abAbac", "*Abac*");
    _INA_TEST_STRING_WILDCARD_TEST_OK("aAazz", "a*zz*");
    _INA_TEST_STRING_WILDCARD_TEST_NOK("A12b12", "*12*23");
    _INA_TEST_STRING_WILDCARD_TEST_OK("a12B12", "*12*12*");
    _INA_TEST_STRING_WILDCARD_TEST_OK("oWn", "*oWn*");

    /* Completely tame (no wildcards) cases. */
    _INA_TEST_STRING_WILDCARD_TEST_OK("bLah", "bLah");
    _INA_TEST_STRING_WILDCARD_TEST_NOK("bLah", "bLaH");

    /* Simple mixed wildcard tests suggested by IBMer Marlin Deckert. */
    _INA_TEST_STRING_WILDCARD_TEST_OK("a", "*?");
    _INA_TEST_STRING_WILDCARD_TEST_OK("ab", "*?");
    _INA_TEST_STRING_WILDCARD_TEST_OK("abc", "*?");

    /* More mixed wildcard tests including coverage for false positives. */
    _INA_TEST_STRING_WILDCARD_TEST_NOK("a", "??");
    _INA_TEST_STRING_WILDCARD_TEST_OK("ab", "?*?");
    _INA_TEST_STRING_WILDCARD_TEST_OK("ab", "*?*?*");
    _INA_TEST_STRING_WILDCARD_TEST_OK("abc", "?**?*?");
    _INA_TEST_STRING_WILDCARD_TEST_NOK("abc", "?**?*&?");
    _INA_TEST_STRING_WILDCARD_TEST_OK("abcd", "?b*??");
    _INA_TEST_STRING_WILDCARD_TEST_NOK("abcd", "?a*??");
    _INA_TEST_STRING_WILDCARD_TEST_OK("abcd", "?**?c?");
    _INA_TEST_STRING_WILDCARD_TEST_NOK("abcd", "?**?d?");
    _INA_TEST_STRING_WILDCARD_TEST_OK("abcde", "?*b*?*d*?");

    /* Single-character-match cases. */
    _INA_TEST_STRING_WILDCARD_TEST_OK("bLah", "bL?h");
    _INA_TEST_STRING_WILDCARD_TEST_NOK("bLaaa", "bLa?");
    _INA_TEST_STRING_WILDCARD_TEST_OK("bLah", "bLa?");
    _INA_TEST_STRING_WILDCARD_TEST_NOK("bLaH", "?Lah");
    _INA_TEST_STRING_WILDCARD_TEST_OK("bLaH", "?LaH");

    /* Many-wildcard scenarios. */
    _INA_TEST_STRING_WILDCARD_TEST_OK("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
    aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaab", "a*a*a*a*a*a*aa*aaa*a*a*b");
    _INA_TEST_STRING_WILDCARD_TEST_OK("abababababababababababababababababababaacacacacaca\
    cacadaeafagahaiajakalaaaaaaaaaaaaaaaaaffafagaagggagaaaaaaaab", "*a*b*ba*ca*a*aa*aaa*fa*ga*b*");
    _INA_TEST_STRING_WILDCARD_TEST_NOK("abababababababababababababababababababaacacacacaca\
    cacadaeafagahaiajakalaaaaaaaaaaaaaaaaaffafagaagggagaaaaaaaab", "*a*b*ba*ca*a*x*aaa*fa*ga*b*");
    _INA_TEST_STRING_WILDCARD_TEST_NOK("abababababababababababababababababababaacacacacaca\
    cacadaeafagahaiajakalaaaaaaaaaaaaaaaaaffafagaagggagaaaaaaaab", "*a*b*ba*ca*aaaa*fa*ga*gggg*b*");
    _INA_TEST_STRING_WILDCARD_TEST_OK("abababababababababababababababababababaacacacacaca\
    cacadaeafagahaiajakalaaaaaaaaaaaaaaaaaffafagaagggagaaaaaaaab", "*a*b*ba*ca*aaaa*fa*ga*ggg*b*");
    _INA_TEST_STRING_WILDCARD_TEST_OK("aaabbaabbaab", "*aabbaa*a*");
    _INA_TEST_STRING_WILDCARD_TEST_OK("a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*", "a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*");
    _INA_TEST_STRING_WILDCARD_TEST_OK("aaaaaaaaaaaaaaaaa", "*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*");
    _INA_TEST_STRING_WILDCARD_TEST_NOK("aaaaaaaaaaaaaaaa", "*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*");
    _INA_TEST_STRING_WILDCARD_TEST_NOK("abc*abcd*abcde*abcdef*abcdefg*abcdefgh*abcdefghi*a\
    bcdefghij*abcdefghijk*abcdefghijkl*abcdefghijklm*abcdefghijklmn", 
    "abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*a\
                bc*");
    _INA_TEST_STRING_WILDCARD_TEST_NOK("abc*abcd*abcd*abc*abcd", "abc*abc*abc*abc*abc");
    _INA_TEST_STRING_WILDCARD_TEST_OK("abc*abcd*abcd*abc*abcd*abcd*abc*abcd*abc*abc*abcd", "abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abcd");
    _INA_TEST_STRING_WILDCARD_TEST_OK("abc", "********a********b********c********");
    _INA_TEST_STRING_WILDCARD_TEST_NOK("********a********b********c********", "abc");
    _INA_TEST_STRING_WILDCARD_TEST_NOK("abc", "********a********b********b********");
    _INA_TEST_STRING_WILDCARD_TEST_OK("*abc*", "***a*b*c***");
}