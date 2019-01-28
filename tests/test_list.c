/*
 * Copyright INAOS GmbH, Thalwil, 2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <stdio.h>
#include <libinac/lib.h>

typedef struct ina_data_s {
    int index;
} ina_data_t;

typedef struct ina_node_data_s {
    int index;
    ina_list_node_t node;
} ina_node_data_t;

static ina_rc_t print_data(void *data)
{
    const ina_data_t *d = (ina_data_t*)data;
    INA_TEST_MSG("[%d]", d->index);
    return INA_SUCCESS;
}

static int find_data(const void *data, const void *find_arg)
{
    const ina_data_t *d = (ina_data_t*)data;
    const int index = *(int*)(find_arg);
    if (d->index == index) {
        return 0;
    }
    return (d->index > index);
}

static int sort_desc(const void *lhs, const void *rhs)
{
    const ina_data_t *a = (ina_data_t*)lhs;
    const ina_data_t *b = (ina_data_t*)rhs;
    return (a->index > b->index);
}

INA_TEST(list, new_free)
{
    ina_list_t *list = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_list_new(INA_LIST_CF_DEFAULT, &list));
    INA_TEST_ASSERT_NOT_NULL(list);
    ina_list_free(&list);
    INA_TEST_ASSERT_NULL(list);

    INA_TEST_ASSERT_SUCCEED(ina_list_new(INA_LIST_CF_NOMALLOC, &list));
    INA_TEST_ASSERT_NOT_NULL(list);
    ina_list_free(&list);
    INA_TEST_ASSERT_NULL(list);
}

INA_TEST(list, arbitrary_data)
{
    size_t  count;
    ina_list_t *list;
    ina_list_node_t *node;
    ina_data_t *data1, *data2, *data3, *data4, *data5, *data6, *data7= NULL;
    INA_TEST_ASSERT_SUCCEED(ina_list_new(INA_LIST_CF_DEFAULT, &list));
    data1 = ina_mem_alloc(sizeof(ina_data_t));
    data1->index = 1;
    data2 = ina_mem_alloc(sizeof(ina_data_t));
    data2->index = 2;
    data3 = ina_mem_alloc(sizeof(ina_data_t));
    data3->index = 3;
    data4 = ina_mem_alloc(sizeof(ina_data_t));
    data4->index = 4;
    data5 = ina_mem_alloc(sizeof(ina_data_t));
    data5->index = 5;
    data6 = ina_mem_alloc(sizeof(ina_data_t));
    data6->index = 6;
    data7 = ina_mem_alloc(sizeof(ina_data_t));
    data7->index = 7;

    INA_TEST_ASSERT_SUCCEED(ina_list_insert_tail_data(list, data1));
    INA_TEST_ASSERT_SUCCEED(ina_list_insert_tail_data(list, data2));
    INA_TEST_ASSERT_SUCCEED(ina_list_insert_tail_data(list, data3));
    INA_TEST_ASSERT_SUCCEED(ina_list_insert_tail_data(list, data4));
    INA_TEST_ASSERT_SUCCEED(ina_list_insert_tail_data(list, data5));
    INA_TEST_ASSERT_SUCCEED(ina_list_insert_tail_data(list, data6));

    INA_TEST_ASSERT_SUCCEED(ina_list_count(list, &count));
    INA_TEST_ASSERT_EQUAL_SIZE_T(6, count);
    INA_TEST_ASSERT_FAILED(ina_list_remove_data(list, data7));
    INA_TEST_ASSERT_SUCCEED(ina_list_remove_data(list, data1));
    INA_TEST_ASSERT_SUCCEED(ina_list_count(list, &count));
    INA_TEST_ASSERT_EQUAL_SIZE_T(5, count);

    INA_TEST_ASSERT_SUCCEED(ina_list_find(list, find_data, &data3->index, &node));
    INA_TEST_ASSERT_SUCCEED(ina_list_foreach(list, print_data));
    INA_TEST_ASSERT_SUCCEED(ina_list_sort(list, sort_desc));
    INA_TEST_ASSERT_SUCCEED(ina_list_foreach(list, print_data));

    ina_list_free(&list);
}

#define __INA_CALC_SIZE(nodes, max_recyclable) \
    sizeof(ina_list_node_t)*(nodes) + sizeof(void*)*(max_recyclable)

INA_TEST(list, resize)
{
    int i;
    ina_list_t *list;
    ina_data_t *data;
    size_t usage;


    INA_TEST_ASSERT_SUCCEED(ina_list_new(INA_LIST_CF_DEFAULT, &list));
    INA_TEST_ASSERT_SUCCEED(ina_list_usage(list, &usage));
    INA_TEST_ASSERT_EQUAL_SIZE_T(__INA_CALC_SIZE(INA_LIST_DEFAULT_SIZE, INA_LIST_DEFAULT_SIZE), usage);
    INA_TEST_ASSERT_SUCCEED(ina_list_resize(list, 10000, 1000));

    for (i = 0; i < 5000;++i) {
        data = ina_mem_alloc(sizeof(ina_data_t));
        data->index = i;
        INA_TEST_ASSERT_SUCCEED(ina_list_insert_tail_data(list, data));
    }
    INA_TEST_ASSERT_SUCCEED(ina_list_usage(list, &usage));
    INA_TEST_ASSERT_EQUAL_SIZE_T(__INA_CALC_SIZE(10000, 1000), usage);
    INA_TEST_ASSERT_SUCCEED(ina_list_resize(list, 9000, 0));
    INA_TEST_ASSERT_SUCCEED(ina_list_usage(list, &usage));
    INA_TEST_ASSERT_EQUAL_SIZE_T(__INA_CALC_SIZE(9000, 0), usage);
    INA_TEST_ASSERT_SUCCEED(ina_list_resize(list, 3000, 256));
    INA_TEST_ASSERT_SUCCEED(ina_list_usage(list, &usage));
    INA_TEST_ASSERT_EQUAL_SIZE_T(__INA_CALC_SIZE(8000, 256), usage);
}

