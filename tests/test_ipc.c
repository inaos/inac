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

#define F1 0x01
#define F2 0x02
#define F3 0x04
#define F4 0x08

INA_TEST(ipc_flags, new_free)
{
    ina_ipc_flags_t *f1;
    ina_ipc_flags_t *f2;
  
    INA_TEST_ASSERT_SUCCEED(ina_ipc_flags_new("test", F1|F2|F3, &f1));
    INA_TEST_ASSERT_NOT_NULL(f1);
    INA_TEST_ASSERT_SUCCEED(ina_ipc_flags_open("test", &f2));
    INA_TEST_ASSERT_NOT_NULL(f2);
    INA_TEST_ASSERT_SUCCEED(ina_ipc_flags_free(&f1));
    INA_TEST_ASSERT_NULL(f1);
    INA_TEST_ASSERT_SUCCEED(ina_ipc_flags_free(&f2));
    INA_TEST_ASSERT_NULL(f2);
}

INA_TEST(ipc_flags, get_name)
{
    ina_ipc_flags_t *f1;
    ina_ipc_flags_t *f2;
    const char *name1;
    const char *name2;

    INA_TEST_ASSERT_SUCCEED(ina_ipc_flags_new("test_get_name", 0, &f1));
    INA_TEST_ASSERT_NOT_NULL(f1);
    INA_TEST_ASSERT_SUCCEED(ina_ipc_flags_get_name(f1, &name1));
    INA_TEST_ASSERT_EQUAL_STR("test_get_name", name1);
    INA_TEST_ASSERT_SUCCEED(ina_ipc_flags_open("test_get_name", &f2));
    INA_TEST_ASSERT_SUCCEED(ina_ipc_flags_get_name(f2, &name2));
    INA_TEST_ASSERT_NOT_NULL(f2);

    ina_ipc_flags_free(&f1);
    ina_ipc_flags_free(&f2);
 }

INA_TEST(ipc_flags, get)
{
    ina_ipc_flags_t *f;
    uint64_t v;
  
    INA_TEST_ASSERT_SUCCEED(ina_ipc_flags_new("test_get", F1|F2|F3, &f));
    INA_TEST_ASSERT_NOT_NULL(f);
    INA_TEST_ASSERT_SUCCEED(ina_ipc_flags_get(f, &v));
    INA_TEST_ASSERT_TRUE(v == (F1|F2|F3));
    ina_ipc_flags_free(&f);
}

INA_TEST(ipc_flags, set)
{
    ina_ipc_flags_t *f;
    uint64_t v;
  
    INA_TEST_ASSERT_SUCCEED(ina_ipc_flags_new("test_set", F1|F2|F3, &f));
    INA_TEST_ASSERT_NOT_NULL(f);
    INA_TEST_ASSERT_SUCCEED(ina_ipc_flags_set(f, F4));
    INA_TEST_ASSERT_SUCCEED(ina_ipc_flags_get(f, &v));
    INA_TEST_ASSERT_TRUE(v == (F1|F2|F3|F4));
    ina_ipc_flags_free(&f);
}

INA_TEST(ipc_flags, is_set)
{
 
    ina_ipc_flags_t *f;
  
    INA_TEST_ASSERT_SUCCEED(ina_ipc_flags_new("test_is_set", F1|F2|F3, &f));
    INA_TEST_ASSERT_NOT_NULL(f);
    INA_TEST_ASSERT_NOTSUCCEED(ina_ipc_flags_is_set(f, F4));
    INA_TEST_ASSERT_SUCCEED(ina_ipc_flags_set(f, F4));
    INA_TEST_ASSERT_SUCCEED(ina_ipc_flags_is_set(f, F1));
    INA_TEST_ASSERT_SUCCEED(ina_ipc_flags_is_set(f, F2));
    INA_TEST_ASSERT_SUCCEED(ina_ipc_flags_is_set(f, F3));
    INA_TEST_ASSERT_SUCCEED(ina_ipc_flags_is_set(f, F4|F1));
    INA_TEST_ASSERT_SUCCEED(ina_ipc_flags_is_set(f, F4));  
    ina_ipc_flags_free(&f);
 }

INA_TEST(ipc_flags, unset)
{
    ina_ipc_flags_t *f;
    uint64_t v;
  
    INA_TEST_ASSERT_SUCCEED(ina_ipc_flags_new("test_unset", F1|F2|F3|F4, &f));
    INA_TEST_ASSERT_NOT_NULL(f);
    INA_TEST_ASSERT_SUCCEED(ina_ipc_flags_get(f, &v));
    INA_TEST_ASSERT_TRUE(v == (F1|F2|F3|F4));
    INA_TEST_ASSERT_SUCCEED(ina_ipc_flags_unset(f, F4));
    INA_TEST_ASSERT_SUCCEED(ina_ipc_flags_get(f, &v));
    INA_TEST_ASSERT_TRUE(v == (F1|F2|F3));
    ina_ipc_flags_free(&f);
}

INA_TEST(ipc_flags, ref_count)
{
    ina_ipc_flags_t *f;
    uint64_t v;
  
    INA_TEST_ASSERT_SUCCEED(ina_ipc_flags_new("test_unset", F1|F2|F3|F4, &f));
    INA_TEST_ASSERT_NOT_NULL(f);
    INA_TEST_ASSERT_SUCCEED(ina_ipc_flags_get(f, &v));
    INA_TEST_ASSERT_TRUE(v == (F1|F2|F3|F4));
    INA_TEST_ASSERT_SUCCEED(ina_ipc_flags_set(f, F1|F2));
    INA_TEST_ASSERT_SUCCEED(ina_ipc_flags_unset(f, F4));
    INA_TEST_ASSERT_SUCCEED(ina_ipc_flags_get(f, &v));
    INA_TEST_ASSERT_TRUE(v == (F1|F2|F3));
    INA_TEST_ASSERT_SUCCEED(ina_ipc_flags_unset(f, F1|F2));
    INA_TEST_ASSERT_SUCCEED(ina_ipc_flags_get(f, &v));
    INA_TEST_ASSERT_TRUE(v == (F1|F2|F3));
    INA_TEST_ASSERT_SUCCEED(ina_ipc_flags_unset(f, F1|F2));
    INA_TEST_ASSERT_SUCCEED(ina_ipc_flags_get(f, &v));
    INA_TEST_ASSERT_TRUE(v == (F3));
    ina_ipc_flags_free(&f);
 }

INA_TEST(ipc_flags, wait)
{
    ina_ipc_flags_t *f;

    INA_TEST_ASSERT_SUCCEED(ina_ipc_flags_new("test_wait", F1|F2|F3, &f));
    INA_TEST_ASSERT_NOT_NULL(f);
    INA_TEST_ASSERT_NOTSUCCEED(ina_ipc_flags_wait(f, F1|F2|F3|F4, 200));
    INA_TEST_ASSERT_SUCCEED(ina_ipc_flags_set(f, F4));
    INA_TEST_ASSERT_SUCCEED(ina_ipc_flags_wait(f, F1|F2|F3|F4, 200));
    INA_TEST_ASSERT_SUCCEED(ina_ipc_flags_wait(f, F4, 200));
    ina_ipc_flags_free(&f);
}

INA_TEST(ipc_flags, wait_ipc)
{
    ina_test_hid_t hid;
    ina_ipc_flags_t *f;
    
    INA_TEST_ASSERT_SUCCEED(ina_ipc_flags_new("test_wait_ipc", 0, &f));
    INA_TEST_ASSERT_NOTSUCCEED(ina_ipc_flags_wait(f, INA_IPC_FLAGS_13, 500));
    INA_TEST_HELPER_INVOKE(&hid, ipc, set_unset_flag,  
        "test_wait_ipc", NULL);    
    INA_TEST_ASSERT_SUCCEED(ina_ipc_flags_wait(f, INA_IPC_FLAGS_13, 1000));
    INA_TEST_ASSERT_SUCCEED(ina_ipc_flags_wait(f, 0, 2500));
}

INA_TEST(ipc_counter, new_free)
{
    ina_ipc_counter_t *c1;
    ina_ipc_counter_t *c2;
    uint64_t cval = 0;
  
    INA_TEST_ASSERT_SUCCEED(ina_ipc_counter_new("test", 3, &c1));
    INA_TEST_ASSERT_NOT_NULL(c1);
    INA_TEST_ASSERT_SUCCEED(ina_ipc_counter_open("test", &c2));
    INA_TEST_ASSERT_NOT_NULL(c2);
    INA_TEST_ASSERT_SUCCEED(ina_ipc_counter_get(c1, &cval));
    INA_TEST_ASSERT_EQUAL_INTEGER(3, cval);
    INA_TEST_ASSERT_SUCCEED(ina_ipc_counter_free(&c1));
    INA_TEST_ASSERT_NULL(c1);
    INA_TEST_ASSERT_SUCCEED(ina_ipc_counter_free(&c2));
    INA_TEST_ASSERT_NULL(c2);
}

INA_TEST(ipc_counter, set_get)
{
    ina_ipc_counter_t *c;
    uint64_t cval = 0;

    INA_TEST_ASSERT_SUCCEED(ina_ipc_counter_new("test", 0, &c));
    INA_TEST_ASSERT_NOT_NULL(c);

    INA_TEST_ASSERT_SUCCEED(ina_ipc_counter_get(c, &cval));
    INA_TEST_ASSERT_EQUAL_INTEGER(0, cval);
    INA_TEST_ASSERT_SUCCEED(ina_ipc_counter_set(c, 34LL));
    INA_TEST_ASSERT_SUCCEED(ina_ipc_counter_get(c, &cval));
    INA_TEST_ASSERT_EQUAL_INTEGER(34, cval);

    INA_TEST_ASSERT_SUCCEED(ina_ipc_counter_free(&c));
    INA_TEST_ASSERT_NULL(c);
}

INA_TEST(ipc_counter, inc_get)
{
    ina_ipc_counter_t *c;
    uint64_t cval = 0;

    INA_TEST_ASSERT_SUCCEED(ina_ipc_counter_new("test", 0, &c));
    INA_TEST_ASSERT_NOT_NULL(c);

    INA_TEST_ASSERT_SUCCEED(ina_ipc_counter_set(c, 1LL));
    INA_TEST_ASSERT_SUCCEED(ina_ipc_counter_get(c, &cval));
    INA_TEST_ASSERT_EQUAL_INTEGER(1, cval);
    INA_TEST_ASSERT_SUCCEED(ina_ipc_counter_increment(c, 1));
    INA_TEST_ASSERT_SUCCEED(ina_ipc_counter_get(c, &cval));
    INA_TEST_ASSERT_EQUAL_INTEGER(2, cval);
    INA_TEST_ASSERT_SUCCEED(ina_ipc_counter_increment(c, 3));
    INA_TEST_ASSERT_SUCCEED(ina_ipc_counter_get(c, &cval));
    INA_TEST_ASSERT_EQUAL_INTEGER(5, cval);

    INA_TEST_ASSERT_SUCCEED(ina_ipc_counter_free(&c));
    INA_TEST_ASSERT_NULL(c);
}

INA_TEST(ipc_counter, dec_get)
{
    ina_ipc_counter_t *c;
    uint64_t cval = 0;

    INA_TEST_ASSERT_SUCCEED(ina_ipc_counter_new("test", 0, &c));
    INA_TEST_ASSERT_NOT_NULL(c);

    INA_TEST_ASSERT_SUCCEED(ina_ipc_counter_set(c, 8LL));
    INA_TEST_ASSERT_SUCCEED(ina_ipc_counter_get(c, &cval));
    INA_TEST_ASSERT_EQUAL_INTEGER(8, cval);
    INA_TEST_ASSERT_SUCCEED(ina_ipc_counter_decrement(c, 1));
    INA_TEST_ASSERT_SUCCEED(ina_ipc_counter_get(c, &cval));
    INA_TEST_ASSERT_EQUAL_INTEGER(7, cval);
    INA_TEST_ASSERT_SUCCEED(ina_ipc_counter_decrement(c, 3));
    INA_TEST_ASSERT_SUCCEED(ina_ipc_counter_get(c, &cval));
    INA_TEST_ASSERT_EQUAL_INTEGER(4, cval);

    INA_TEST_ASSERT_SUCCEED(ina_ipc_counter_free(&c));
    INA_TEST_ASSERT_NULL(c);
}
