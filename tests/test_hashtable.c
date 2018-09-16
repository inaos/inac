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

typedef struct ina_htdata_s {
    ina_str_t name;
    int id;
} ina_htdata_t;

typedef struct ina_htdata_u32_s {
    ina_str_t name;
    uint32_t id;
} ina_htdata_u32_t;

typedef struct ina_htdata_i64_s {
    ina_str_t name;
    int64_t id;
} ina_htdata_i64_t;

typedef struct ina_htdata_u64_s {
    ina_str_t name;
    uint64_t id;
} ina_htdata_u64_t;


static ina_rc_t print_data(const void* data)
{
    INA_TEST_MSG("[%d] - %s",
                 ((const ina_htdata_t*)data)->id,
           ((const ina_htdata_t*)data)->name);
    return INA_SUCCESS;
}

static ina_htdata_t* new_data(int id, const char* name)
{
    ina_htdata_t *data;
    data = ina_mem_alloc(sizeof(ina_htdata_t));
    data->id = id;
    data->name = ina_str_new_fromcstr(name);
    return data;
}

static ina_htdata_u32_t* new_data_u32(uint32_t id, const char* name)
{
    ina_htdata_u32_t *data;
    data = ina_mem_alloc(sizeof(ina_htdata_u32_t));
    data->id = id;
    data->name = ina_str_new_fromcstr(name);
    return data;
}

static ina_htdata_u64_t* new_data_u64(uint64_t id, const char* name)
{
    ina_htdata_u64_t *data;
    data = ina_mem_alloc(sizeof(ina_htdata_u64_t));
    data->id = id;
    data->name = ina_str_new_fromcstr(name);
    return data;
}

static ina_htdata_i64_t* new_data_i64(int64_t id, const char* name)
{
    ina_htdata_i64_t *data;
    data = ina_mem_alloc(sizeof(ina_htdata_i64_t));
    data->id = id;
    data->name = ina_str_new_fromcstr(name);
    return data;
}


INA_TEST(hashtable, int_key)
{
    ina_hashtable_t *ht = NULL;
    ina_htdata_t *data;
    int count;
    size_t usage;

    INA_TEST_ASSERT_SUCCEED(ina_hashtable_new(INA_HASHTABLE_INT32_KEY,
                                               INA_HASHTABLE_HASH_DEFAULT,
                                               INA_HASHTABLE_TYPE_DEFAULT,
                                               INA_HASHTABLE_GROW_DEFAULT,
                                               INA_HASHTABLE_SHRINK_DEFAULT,
                                               INA_HASHTABLE_DEFAULT_CAPACITY,
                                               INA_HASHTABLE_CF_DEFAULT, &ht));

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
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_set_i32(ht, data->id, data));
    data = new_data(20, "Name 20");
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_set_i32(ht, data->id, data));
    INA_TEST_ASSERT_SUCCEED(data = new_data(30, "Name 30"));
    ina_hashtable_set_i32(ht, data->id, data);
    data = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_get_i32(ht, 10, (void**)&data));
    INA_TEST_ASSERT_EQUAL_STR("Name 10", data->name);

    INA_TEST_ASSERT_SUCCEED(ina_hashtable_count(ht, &count));
    INA_TEST_ASSERT_EQUAL_INT(4, count);
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_usage(ht, &usage));
    INA_TEST_MSG("usage in bytes: %d", usage);

    INA_TEST_ASSERT_SUCCEED(ina_hashtable_foreach(ht, print_data));
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_free(&ht));
}

INA_TEST(hashtable, uint32_key)
{
    ina_hashtable_t *ht = NULL;
    ina_htdata_u32_t *data;
    int count;
    size_t usage;

    INA_TEST_ASSERT_SUCCEED(ina_hashtable_new(INA_HASHTABLE_UINT32_KEY,
                                               INA_HASHTABLE_HASH_DEFAULT,
                                               INA_HASHTABLE_TYPE_DEFAULT,
                                               INA_HASHTABLE_GROW_DEFAULT,
                                               INA_HASHTABLE_SHRINK_DEFAULT,
                                               INA_HASHTABLE_DEFAULT_CAPACITY,
                                               INA_HASHTABLE_CF_DEFAULT, &ht));

    INA_TEST_ASSERT_NOT_NULL(ht);

    data = new_data_u32(1, "Name 1");
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_set_u32(ht, data->id, data));
    data = new_data_u32(2, "Name 2");
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_set_u32(ht, data->id, data));


    data = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_get_u32(ht, 1, (void**)&data));
    INA_TEST_ASSERT_EQUAL_STR("Name 1", data->name);
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_get_u32(ht, 2, (void**)&data));
    INA_TEST_ASSERT_EQUAL_STR("Name 2", data->name);
    INA_TEST_ASSERT_FAILED(ina_hashtable_get_u32(ht, 3, (void**)&data));
    data = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_remove_u32(ht, 1, (void**)&data));
    INA_TEST_ASSERT_NOT_NULL(data);
    INA_TEST_ASSERT_EQUAL_STR("Name 1", data->name);
    INA_TEST_ASSERT_FAILED(ina_hashtable_get_u32(ht, 1, (void**)&data));
    data = new_data_u32(10, "Name 10");
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_set_u32(ht, data->id, data));
    data = new_data_u32(20, "Name 20");
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_set_u32(ht, data->id, data));
    data = new_data_u32(30, "Name 30");
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_set_u32(ht, data->id, data));
    data = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_get_u32(ht, 10, (void**)&data));
    INA_TEST_ASSERT_EQUAL_STR("Name 10", data->name);

    INA_TEST_ASSERT_SUCCEED(ina_hashtable_count(ht, &count));
    INA_TEST_ASSERT_EQUAL_INT(4, count);
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_usage(ht, &usage));
    INA_TEST_MSG("usage in bytes: %d", usage);

    INA_TEST_ASSERT_SUCCEED(ina_hashtable_free(&ht));
}

INA_TEST(hashtable, uint64_key)
{
    ina_hashtable_t *ht = NULL;
    ina_htdata_u64_t *data;
    int count;
    size_t usage;

    INA_TEST_ASSERT_SUCCEED(ina_hashtable_new(INA_HASHTABLE_UINT64_KEY,
                                               INA_HASHTABLE_HASH_DEFAULT,
                                               INA_HASHTABLE_TYPE_DEFAULT,
                                               INA_HASHTABLE_GROW_DEFAULT,
                                               INA_HASHTABLE_SHRINK_DEFAULT,
                                               INA_HASHTABLE_DEFAULT_CAPACITY,
                                               INA_HASHTABLE_CF_DEFAULT, &ht));


    INA_TEST_ASSERT_NOT_NULL(ht);

    data = new_data_u64(1, "Name 1");
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_set_u64(ht, data->id, data));
    data = new_data_u64(2, "Name 2");
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_set_u64(ht, data->id, data));


    data = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_get_u64(ht, 1, (void**)&data));
    INA_TEST_ASSERT_EQUAL_STR("Name 1", data->name);
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_get_u64(ht, 2, (void**)&data));
    INA_TEST_ASSERT_EQUAL_STR("Name 2", data->name);
    INA_TEST_ASSERT_FAILED(ina_hashtable_get_u64(ht, 3, (void**)&data));
    data = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_remove_u64(ht, 1, (void**)&data));
    INA_TEST_ASSERT_NOT_NULL(data);
    INA_TEST_ASSERT_EQUAL_STR("Name 1", data->name);
    INA_TEST_ASSERT_FAILED(ina_hashtable_get_u64(ht, 1, (void**)&data));
    data = new_data_u64(10, "Name 10");
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_set_u64(ht, data->id, data));
    data = new_data_u64(20, "Name 20");
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_set_u64(ht, data->id, data));
    data = new_data_u64(30, "Name 30");
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_set_u64(ht, data->id, data));
    data = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_get_u64(ht, 10, (void**)&data));
    INA_TEST_ASSERT_EQUAL_STR("Name 10", data->name);

    INA_TEST_ASSERT_SUCCEED(ina_hashtable_count(ht, &count));
    INA_TEST_ASSERT_EQUAL_INT(4, count);
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_usage(ht, &usage));
    INA_TEST_MSG("usage in bytes: %d", usage);

    INA_TEST_ASSERT_SUCCEED(ina_hashtable_free(&ht));
}

INA_TEST(hashtable, int64_key)
{
    ina_hashtable_t *ht = NULL;
    ina_htdata_i64_t *data;
    int count;
    size_t usage;

    INA_TEST_ASSERT_SUCCEED(ina_hashtable_new(INA_HASHTABLE_INT64_KEY,
                                               INA_HASHTABLE_HASH_DEFAULT,
                                               INA_HASHTABLE_TYPE_DEFAULT,
                                               INA_HASHTABLE_GROW_DEFAULT,
                                               INA_HASHTABLE_SHRINK_DEFAULT,
                                               INA_HASHTABLE_DEFAULT_CAPACITY,
                                               INA_HASHTABLE_CF_DEFAULT, &ht));

    INA_TEST_ASSERT_NOT_NULL(ht);

    data = new_data_i64(1, "Name 1");
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_set_i64(ht, data->id, data));
    data = new_data_i64(2, "Name 2");
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_set_i64(ht, data->id, data));


    data = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_get_i64(ht, 1, (void**)&data));
    INA_TEST_ASSERT_EQUAL_STR("Name 1", data->name);
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_get_i64(ht, 2, (void**)&data));
    INA_TEST_ASSERT_EQUAL_STR("Name 2", data->name);
    INA_TEST_ASSERT_FAILED(ina_hashtable_get_i64(ht, 3, (void**)&data));
    data = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_remove_i64(ht, 1, (void**)&data));
    INA_TEST_ASSERT_NOT_NULL(data);
    INA_TEST_ASSERT_EQUAL_STR("Name 1", data->name);
    INA_TEST_ASSERT_FAILED(ina_hashtable_get_i64(ht, 1, (void**)&data));
    data = new_data_i64(10, "Name 10");
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_set_i64(ht, data->id, data));
    data = new_data_i64(20, "Name 20");
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_set_u64(ht, data->id, data));
    data = new_data_i64(30, "Name 30");
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_set_u64(ht, data->id, data));
    data = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_get_i64(ht, 10, (void**)&data));
    INA_TEST_ASSERT_EQUAL_STR("Name 10", data->name);

    INA_TEST_ASSERT_SUCCEED(ina_hashtable_count(ht, &count));
    INA_TEST_ASSERT_EQUAL_INT(4, count);
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_usage(ht, &usage));
    INA_TEST_MSG("usage in bytes: %d", usage);

    INA_TEST_ASSERT_SUCCEED(ina_hashtable_free(&ht));
}


INA_TEST(hashtable, ptr_key)
{
    ina_hashtable_t *ht = NULL;
    ina_htdata_t *data1, *data2, *data3, *data;
    int count;

    INA_TEST_ASSERT_SUCCEED(ina_hashtable_new(INA_HASHTABLE_PTR_KEY,
                                               INA_HASHTABLE_HASH_DEFAULT,
                                               INA_HASHTABLE_TYPE_DEFAULT,
                                               INA_HASHTABLE_GROW_DEFAULT,
                                               INA_HASHTABLE_SHRINK_DEFAULT,
                                               INA_HASHTABLE_DEFAULT_CAPACITY,
                                               INA_HASHTABLE_CF_DEFAULT, &ht));
    INA_TEST_ASSERT_NOT_NULL(ht);

    data1 = new_data(1, "Name 1");
    data2 = new_data(2, "Name 2");
    data3 = new_data(2, "Name 3");
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_set_ptr(ht, data1, data2));
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_set_ptr(ht, data2, data1));


    INA_TEST_ASSERT_SUCCEED(ina_hashtable_get_ptr(ht, data1, (void**)&data));
    INA_TEST_ASSERT_EQUAL_STR("Name 2", data->name);
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_get_ptr(ht, data2, (void**)&data));
    INA_TEST_ASSERT_EQUAL_STR("Name 1", data->name);
    INA_TEST_ASSERT_FAILED(ina_hashtable_get_ptr(ht, data3, (void**)&data));
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_count(ht, &count));
    INA_TEST_ASSERT_EQUAL_INT(2, count);
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_remove_ptr(ht, data1, (void**)&data));
    INA_TEST_ASSERT_NOT_NULL(data);
    INA_TEST_ASSERT_EQUAL_STR("Name 2", data->name);
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_count(ht, &count));
    INA_TEST_ASSERT_EQUAL_INT(1, count);

    INA_TEST_ASSERT_SUCCEED(ina_hashtable_free(&ht));
}

INA_TEST(hashtable, str_key)
{
    ina_hashtable_t *ht = NULL;
    ina_htdata_t *data1, *data2, *data;
    int count;


    INA_TEST_ASSERT_SUCCEED(ina_hashtable_new(INA_HASHTABLE_STR_KEY,
                                               INA_HASHTABLE_HASH32_SPOOKY,
                                               INA_HASHTABLE_TYPE_DEFAULT,
                                               INA_HASHTABLE_GROW_DEFAULT,
                                               INA_HASHTABLE_SHRINK_DEFAULT,
                                               INA_HASHTABLE_DEFAULT_CAPACITY,
                                               INA_HASHTABLE_CF_STAT, &ht));
    INA_TEST_ASSERT_NOT_NULL(ht);
    /*sleep(10);*/
    data1 = new_data(1, "Name 1");
    data2 = new_data(2, "Name 2");
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_set_str(ht, "n1", data1));
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_set_str(ht, "n2", data2));


    INA_TEST_ASSERT_SUCCEED(ina_hashtable_get_str(ht, "n2", (void**)&data));
    INA_TEST_ASSERT_EQUAL_STR("Name 2", data->name);
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_get_str(ht, "n1", (void**)&data));
    INA_TEST_ASSERT_EQUAL_STR("Name 1", data->name);
    INA_TEST_ASSERT_FAILED(ina_hashtable_get_str(ht, "n3", (void**)&data));
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_count(ht, &count));
    INA_TEST_ASSERT_EQUAL_INT(2, count);
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_remove_str(ht, "n2", (void**)&data));
    INA_TEST_ASSERT_NOT_NULL(data);
    INA_TEST_ASSERT_EQUAL_STR("Name 2", data->name);
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_count(ht, &count));
    INA_TEST_ASSERT_EQUAL_INT(1, count);

    INA_TEST_ASSERT_SUCCEED(ina_hashtable_free(&ht));
}



INA_TEST(hashtable, iter)
{
    ina_hashtable_t *ht = NULL;
    void *d = NULL;
    ina_hashtable_iter_t *iter = NULL;
    int count;

    INA_TEST_ASSERT_SUCCEED(ina_hashtable_new(INA_HASHTABLE_STR_KEY,
                                               INA_HASHTABLE_HASH_DEFAULT,
                                               INA_HASHTABLE_TYPE_DEFAULT,
                                               INA_HASHTABLE_GROW_DEFAULT,
                                               INA_HASHTABLE_SHRINK_DEFAULT,
                                               INA_HASHTABLE_DEFAULT_CAPACITY,
                                               INA_HASHTABLE_CF_DEFAULT, &ht));
    INA_TEST_ASSERT_NOT_NULL(ht);


    INA_TEST_ASSERT_SUCCEED(ina_hashtable_set_str(ht, "n1", new_data(1, "Name 1")));
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_set_str(ht, "n2", new_data(2, "Name 2")));
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_set_str(ht, "n3", new_data(3, "Name 3")));
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_set_str(ht, "n4", new_data(4, "Name 4")));
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_count(ht, &count));
    INA_TEST_ASSERT_EQUAL_INT(4, count);

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

    INA_TEST_ASSERT_SUCCEED(ina_hashtable_set_str(ht, "n5", new_data(4, "Name 5")));

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

    INA_TEST_ASSERT_SUCCEED(ina_hashtable_foreach(ht, print_data));
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_iter_free(&iter));
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_foreach(ht, print_data));
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_free(&ht));
}

INA_TEST(hashtable, clear)
{
    ina_hashtable_t *ht = NULL;
    ina_htdata_t *data;
    int count;
    size_t usage;

    INA_TEST_ASSERT_SUCCEED(ina_hashtable_new(INA_HASHTABLE_INT32_KEY,
                                               INA_HASHTABLE_HASH_DEFAULT,
                                               INA_HASHTABLE_TYPE_DEFAULT,
                                               INA_HASHTABLE_GROW_DEFAULT,
                                               INA_HASHTABLE_SHRINK_DEFAULT,
                                               INA_HASHTABLE_DEFAULT_CAPACITY,
                                               INA_HASHTABLE_CF_DEFAULT, &ht));


    INA_TEST_ASSERT_NOT_NULL(ht);

    data = new_data(1, "Name 1");
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_set_i32(ht, data->id, data));
    data = new_data(2, "Name 2");
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_set_i32(ht, data->id, data));
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_count(ht, &count));
    INA_TEST_ASSERT_EQUAL_INT(2, count);
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_clear(ht));
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_count(ht, &count));
    INA_TEST_ASSERT_EQUAL_INT(0, count);
    INA_TEST_ASSERT_SUCCEED(ina_hashtable_free(&ht));
}

INA_TEST(hashtable, new_from_cfg)
{
    ina_hashtable_t *ht;
    const char* names[] = {"h1", "h2", "h3", NULL};
    int i = -1;
    int count = 0;
    const int data = 1;

    while (names[++i] != NULL) {
        int j;
        INA_TEST_ASSERT_SUCCEED(ina_hashtable_new_from_cfg(INA_HASHTABLE_INT32_KEY, names[i], &ht));
        INA_TEST_ASSERT_NOT_NULL(ht);
        for (j=0;j<100*i;j++) {
            INA_TEST_ASSERT_SUCCEED(ina_hashtable_set_i32(ht, j, &data));
        }
        ina_hashtable_count(ht, &count);
        INA_TEST_ASSERT_EQUAL_INT(100*i, count);
        INA_TEST_ASSERT_SUCCEED(ina_hashtable_free(&ht));
        INA_TEST_ASSERT_NULL(ht);
    }
}