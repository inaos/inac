/*
 * Copyright (c) 2018, INAOS GmbH
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

typedef struct data {
    ina_str_t name;
    int id;
} ina_data_t;

static ina_rc_t print_data(const void* data)
{
    INA_TEST_MSG("[%d] - %s",
            ((const ina_data_t*)data)->id,
           ((const ina_data_t*)data)->name);
    return INA_SUCCESS;
}

static ina_data_t* new_data(int id, const char* name)
{
    ina_data_t *data;
    data = ina_mem_alloc(sizeof(ina_data_t));
    data->id = id;
    data->name = ina_str_new_fromcstr(name);
    return data;
}

INA_TEST(hashtable, simple)
{
    ina_hashtable_ctx_t *ctx = NULL;
    ina_hashtable_t *ht = NULL;
    ina_data_t *data;
    int count;
    size_t usage;

    INA_TEST_ASSERT_SUCCEED(ina_hashtable_init(INA_HASHTABL_INT32_KEY,
            INA_HASHTABLE_HASH32_SPOOKY,
            INA_HASHTABLE_TYPE_CHAINED,
            INA_HASHTABLE_GROW_LINEAR, 0, &ctx));
    INA_TEST_ASSERT_NOT_NULL(ctx);


    INA_TEST_ASSERT_SUCCEED(ina_hashtable_new(ctx, 256, 0, &ht));
    INA_TEST_ASSERT_NOT_NULL(ht);

    data = new_data(1, "Name 1");
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_set_i32(ht, data->id, data));
    data = new_data(2, "Name 2");
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_set_i32(ht, data->id, data));


    data = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_get_i32(ht, 1, (void**)&data));
    INA_TEST_ASSERT_EQUAL_STR("Name 1", data->name);
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_get_i32(ht, 2, (void**)&data));
    INA_TEST_ASSERT_EQUAL_STR("Name 2", data->name);
    INA_TEST_ASSERT_FAILED(ina_hashtable_get_i32(ht, 3, (void**)&data));
    data = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_remove_i32(ht, 1, (void**)&data));
    INA_TEST_ASSERT_NOT_NULL(data);
    INA_TEST_ASSERT_EQUAL_STR("Name 1", data->name);
    INA_TEST_ASSERT_FAILED(ina_hashtable_get_i32(ht, 1, (void**)&data));
    data = new_data(10, "Name 10");
    ina_hashtable_set_i32(ht, data->id, data);
    data = new_data(20, "Name 20");
    ina_hashtable_set_i32(ht, data->id, data);
    data = new_data(30, "Name 30");
    ina_hashtable_set_i32(ht, data->id, data);
    data = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_get_i32(ht, 10, (void**)&data));
    INA_TEST_ASSERT_EQUAL_STR("Name 10", data->name);

    INA_TEST_ASSERT_SUCCEED(ina_hashtable_count(ht, &count));
    INA_TEST_MSG("count: %d", count);
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_usage(ht, &usage));
    INA_TEST_MSG("usage in bytes: %d", usage);

    ina_hashtable_foreach(ht, print_data);
    ina_hashtable_free(&ht);
    ina_hashtable_destroy(&ctx);
}

INA_TEST(hashtable, ptr_key)
{
    ina_hashtable_ctx_t *ctx = NULL;
    ina_hashtable_t *ht = NULL;
    ina_data_t *data1, *data2, *data3, *data;

    INA_TEST_ASSERT_SUCCEED(ina_hashtable_init(INA_HASHTABLE_PTR_KEY,
                                               INA_HASHTABLE_HASH32_SPOOKY,
                                               INA_HASHTABLE_TYPE_CHAINED,
                                               INA_HASHTABLE_GROW_LINEAR, 0, &ctx));
    INA_TEST_ASSERT_NOT_NULL(ctx);


    INA_TEST_ASSERT_SUCCEED(ina_hashtable_new(ctx, 256, 0, &ht));
    INA_TEST_ASSERT_NOT_NULL(ht);

    data1 = new_data(1, "Name 1");
    data2 = new_data(2, "Name 2");
    data3 = new_data(2, "Name 3");
    ina_hashtable_set_ptr(ht, data1, data2);
    ina_hashtable_set_ptr(ht, data2, data1);


    INA_TEST_ASSERT_SUCCEED(ina_hashtable_get_ptr(ht, data1, (void**)&data));
    INA_TEST_ASSERT_EQUAL_STR("Name 2", data->name);
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_get_ptr(ht, data2, (void**)&data));
    INA_TEST_ASSERT_EQUAL_STR("Name 1", data->name);
    INA_TEST_ASSERT_FAILED(ina_hashtable_get_ptr(ht, data3, (void**)&data));

    ina_hashtable_foreach(ht, print_data);
    ina_hashtable_free(&ht);
    ina_hashtable_destroy(&ctx);
}

INA_TEST(hashtable, str_key)
{
    ina_hashtable_ctx_t *ctx = NULL;
    ina_hashtable_t *ht = NULL;
    ina_data_t *data1, *data2, *data;


    INA_TEST_ASSERT_SUCCEED(ina_hashtable_init(INA_HASHTABLE_STR_KEY,
                                               INA_HASHTABLE_HASH32_SPOOKY,
                                               INA_HASHTABLE_TYPE_CHAINED,
                                               INA_HASHTABLE_GROW_LINEAR,
                                               INA_HASHTABLE_CF_STAT, &ctx));
    INA_TEST_ASSERT_NOT_NULL(ctx);
    sleep(10);

    INA_TEST_ASSERT_SUCCEED(ina_hashtable_new(ctx, 256, INA_HASHTABLE_CF_STAT, &ht));
    INA_TEST_ASSERT_NOT_NULL(ht);

    data1 = new_data(1, "Name 1");
    data2 = new_data(2, "Name 2");
    ina_hashtable_set_str(ht, "n1", data1);
    ina_hashtable_set_str(ht, "n2", data2);


    INA_TEST_ASSERT_SUCCEED(ina_hashtable_get_str(ht, "n2", (void**)&data));
    INA_TEST_ASSERT_EQUAL_STR("Name 2", data->name);
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_get_str(ht, "n1", (void**)&data));
    INA_TEST_ASSERT_EQUAL_STR("Name 1", data->name);
    INA_TEST_ASSERT_FAILED(ina_hashtable_get_str(ht, "n3", (void**)&data));

    ina_hashtable_foreach(ht, print_data);
    ina_hashtable_free(&ht);
    ina_hashtable_destroy(&ctx);
}

INA_TEST(hashtable, iter)
{
    ina_hashtable_ctx_t *ctx = NULL;
    ina_hashtable_t *ht = NULL;
    ina_data_t *data;
    void *d = NULL;
    ina_hashtable_iter_t *iter = NULL;

    INA_TEST_ASSERT_SUCCEED(ina_hashtable_init(INA_HASHTABLE_STR_KEY,
                                               INA_HASHTABLE_HASH32_SPOOKY,
                                               INA_HASHTABLE_TYPE_CHAINED,
                                               INA_HASHTABLE_GROW_LINEAR, 0, &ctx));
    INA_TEST_ASSERT_NOT_NULL(ctx);


    INA_TEST_ASSERT_SUCCEED(ina_hashtable_new(ctx, 256, 0, &ht));
    INA_TEST_ASSERT_NOT_NULL(ht);

    ina_hashtable_set_str(ht, "n1", new_data(1, "Name 1"));
    ina_hashtable_set_str(ht, "n2", new_data(2, "Name 2"));
    ina_hashtable_set_str(ht, "n3", new_data(3, "Name 3"));
    ina_hashtable_set_str(ht, "n4", new_data(4, "Name 4"));

    INA_TEST_ASSERT_SUCCEED(ina_hashtable_iter_new(ht, &iter));
    INA_TEST_ASSERT_NOT_NULL(ht);
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_iter_next(iter, &d));
    INA_TEST_ASSERT_NOT_NULL(&d);
    d = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_iter_next(iter, &d));
    INA_TEST_ASSERT_NOT_NULL(&d);
    d = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_iter_next(iter, &d));
    INA_TEST_ASSERT_NOT_NULL(&d);
    d = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_iter_next(iter, &d));
    INA_TEST_ASSERT_NOT_NULL(&d);
    d = NULL;
    INA_TEST_ASSERT_FAILED(ina_hashtable_iter_next(iter, &d));

    INA_TEST_ASSERT_SUCCEED(ina_hashtable_iter_reset(iter));

    INA_TEST_ASSERT_SUCCEED(ina_hashtable_iter_next(iter, &d));
    INA_TEST_ASSERT_NOT_NULL(&d);
    d = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_iter_next(iter, &d));
    INA_TEST_ASSERT_NOT_NULL(&d);
    d = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_iter_next(iter, &d));
    INA_TEST_ASSERT_NOT_NULL(&d);
    d = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_iter_next(iter, &d));
    INA_TEST_ASSERT_NOT_NULL(&d);
    d = NULL;
    INA_TEST_ASSERT_FAILED(ina_hashtable_iter_next(iter, &d));

    INA_TEST_ASSERT_SUCCEED(ina_hashtable_iter_reset(iter));


    d = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_iter_next(iter, &d));
    INA_TEST_ASSERT_NOT_NULL(&d);

    ina_hashtable_set_str(ht, "n5", new_data(4, "Name 5"));

    d = NULL;
    INA_TEST_ASSERT_FAILED(ina_hashtable_iter_next(iter, &d));

    INA_TEST_ASSERT_SUCCEED(ina_hashtable_iter_reset(iter));

    INA_TEST_ASSERT_SUCCEED(ina_hashtable_iter_next(iter, &d));
    INA_TEST_ASSERT_NOT_NULL(&d);
    d = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_iter_next(iter, &d));
    INA_TEST_ASSERT_NOT_NULL(&d);
    d = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_iter_next(iter, &d));
    INA_TEST_ASSERT_NOT_NULL(&d);
    d = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_iter_next(iter, &d));
    INA_TEST_ASSERT_NOT_NULL(&d);
    d = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_iter_next(iter, &d));
    INA_TEST_ASSERT_NOT_NULL(&d);
    d = NULL;
    INA_TEST_ASSERT_FAILED(ina_hashtable_iter_next(iter, &d));

    ina_hashtable_foreach(ht, print_data);
    ina_hashtable_iter_free(&iter);
    ina_hashtable_free(&ht);
    ina_hashtable_destroy(&ctx);
}

