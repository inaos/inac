/*
 * Copyright INAOS GmbH, Thalwil, 2016-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>
#include <sys/stat.h>
#ifndef INA_OS_WINDOWS
#include <unistd.h>
#else
#include <direct.h>
#define mkdir(path, permission) _mkdir(path)
#endif


static const char* __dir_entries[] = {
    ".",
    "..",
    "test1",
    "test2",
    "test3",
    "test1.txt",
    "test2.txt"
};

INA_TEST_DATA(dir) {
    ina_str_t  tmp_dir;
};

INA_TEST_SETUP(dir) {
    struct stat st;
    ina_mem_set(&st, 0, sizeof(struct stat));

    data->tmp_dir = ina_str_new(2048);
    ina_str_t dir = ina_str_new(2048);
#ifdef INA_OS_WINDOWS
    ina_str_catcstr(data->tmp_dir, getenv("TEMP"));
    ina_str_catcstr(data->tmp_dir, "/inac_test_dir");
#else
    ina_str_catcstr(data->tmp_dir, "/tmp/inac_test_dir");
#endif


    ina_str_cpy(dir, data->tmp_dir);
    if (stat(data->tmp_dir, &st) == -1) {
        mkdir(data->tmp_dir, 0777);
    }
    ina_str_catcstr(dir, "/test1");
    if (stat(dir, &st) == -1) {
        mkdir(dir, 0777);
    }
    ina_str_cpy(dir, data->tmp_dir);
    ina_str_catcstr(dir, "/test2");
    if (stat(dir, &st) == -1) {
        mkdir(dir, 0777);
    }
    ina_str_cpy(dir, data->tmp_dir);
    ina_str_catcstr(dir, "/test3");
    if (stat(dir, &st) == -1) {
        mkdir(dir, 0777);
    }
    ina_str_free(dir);
}

INA_TEST_TEARDOWN(dir) {
    ina_str_free(data->tmp_dir);
}

INA_TEST_FIXTURE(dir, test_new_free)
{
    ina_dir_walker_t *w = NULL;
    ina_dir_sort_order_t sort_order;
    ina_dir_sort_attrib_t sort_attrib;

    INA_TEST_ASSERT_SUCCEED(ina_dir_walker_new(data->tmp_dir, &w));
    INA_TEST_ASSERT_NOT_NULL(w);
    INA_TEST_ASSERT_SUCCEED(ina_dir_walker_get_sort_order(w, &sort_order));
    INA_TEST_ASSERT_EQUAL_INT(INA_DIR_SORT_ORDER_NONE, sort_order);
    INA_TEST_ASSERT_SUCCEED(ina_dir_walker_get_sort_attrib(w, &sort_attrib));
    INA_TEST_ASSERT_EQUAL_INT(INA_DIR_SORT_ATTRIB_DFT, sort_attrib);
    ina_dir_walker_free(&w);
    INA_TEST_ASSERT_NULL(w);
}

INA_TEST_FIXTURE(dir, test_unsorted)
{
    ina_dir_walker_t *w = NULL;
    const ina_dir_entry_t *e;

    INA_TEST_ASSERT_SUCCEED(ina_dir_walker_new(data->tmp_dir, &w));
    INA_TEST_ASSERT_NOT_NULL(w);
    while (INA_SUCCEED(ina_dir_walker_get_next_entry(w, &e))) {
        INA_TEST_MSG("e->name: %s, e->type: %d", ina_str_cstr(e->name), e->type);
    }
    ina_dir_walker_free(&w);
    INA_TEST_ASSERT_NULL(w);
}


INA_TEST_FIXTURE(dir, test_sorted_by_name)
{
    ina_dir_walker_t *w = NULL;
    const ina_dir_entry_t *e;
    size_t i = 0;

    INA_TEST_ASSERT_SUCCEED(ina_dir_walker_new(data->tmp_dir, &w));
    INA_TEST_ASSERT_NOT_NULL(w);
    INA_TEST_ASSERT_SUCCEED(ina_dir_walker_set_sort_order(w, INA_DIR_SORT_ORDER_ASCEND));
    while (INA_SUCCEED(ina_dir_walker_get_next_entry(w, &e)) && i < sizeof(__dir_entries)/sizeof(char*)) {
        INA_TEST_ASSERT_EQUAL_STR(__dir_entries[i], ina_str_cstr(e->name));
        ++i;    /*
     * close files that are still open
     */

}
    ina_dir_walker_free(&w);
    INA_TEST_ASSERT_NULL(w);
}

INA_TEST_FIXTURE(dir, test_sorted_by_name_descend)
{

    ina_dir_walker_t *w = NULL;
    const ina_dir_entry_t *e;
    int i = sizeof(__dir_entries)/sizeof(char*);

    INA_TEST_ASSERT_SUCCEED(ina_dir_walker_new(data->tmp_dir, &w));
    INA_TEST_ASSERT_NOT_NULL(w);
    INA_TEST_ASSERT_SUCCEED(ina_dir_walker_set_sort_order(w, INA_DIR_SORT_ORDER_DESCEND));
    while (ina_dir_walker_get_next_entry(w, &e) && i > 0) {
        INA_TEST_ASSERT_EQUAL_STR(__dir_entries[i-1], ina_str_cstr(e->name));
        --i;
    }
    ina_dir_walker_free(&w);
    INA_TEST_ASSERT_NULL(w);
}

INA_TEST_FIXTURE_SKIP(dir, test_sorted_by_type)
{
    INA_UNUSED(data);
}

INA_TEST_FIXTURE_SKIP(dir, test_sorted_by_type_descend)
{
    INA_UNUSED(data);
}


INA_TEST_FIXTURE(dir, test_reset)
{
    ina_dir_walker_t *w = NULL;
    const ina_dir_entry_t *e;

    INA_TEST_ASSERT_SUCCEED(ina_dir_walker_new(data->tmp_dir, &w));
    INA_TEST_ASSERT_NOT_NULL(w);
    INA_TEST_ASSERT_SUCCEED(ina_dir_walker_set_sort_order(w, INA_DIR_SORT_ORDER_ASCEND));
    INA_TEST_ASSERT_SUCCEED(ina_dir_walker_get_next_entry(w, &e));
    INA_TEST_ASSERT_SUCCEED(ina_dir_walker_get_next_entry(w, &e));
    INA_TEST_ASSERT_EQUAL_STR("..", e->name);
    INA_TEST_ASSERT_SUCCEED(ina_dir_walker_reset(w));
    INA_TEST_ASSERT_SUCCEED(ina_dir_walker_set_sort_order(w, INA_DIR_SORT_ORDER_DESCEND));
    INA_TEST_ASSERT_SUCCEED(ina_dir_walker_get_next_entry(w, &e));
    INA_TEST_ASSERT_EQUAL_STR(".", e->name);
    ina_dir_walker_free(&w);
    INA_TEST_ASSERT_NULL(w);
}

INA_TEST_FIXTURE(dir, test_reload)
{
    ina_dir_walker_t *w = NULL;
    const ina_dir_entry_t *e;

    INA_TEST_ASSERT_SUCCEED(ina_dir_walker_new(data->tmp_dir, &w));
    INA_TEST_ASSERT_NOT_NULL(w);
    INA_TEST_ASSERT_SUCCEED(ina_dir_walker_set_sort_order(w, INA_DIR_SORT_ORDER_ASCEND));
    INA_TEST_ASSERT_SUCCEED(ina_dir_walker_get_next_entry(w, &e));
    INA_TEST_ASSERT_SUCCEED(ina_dir_walker_get_next_entry(w, &e));
    INA_TEST_ASSERT_EQUAL_STR("..", e->name);
    INA_TEST_ASSERT_SUCCEED(ina_dir_walker_reload(w));
    INA_TEST_ASSERT_SUCCEED(ina_dir_walker_set_sort_order(w, INA_DIR_SORT_ORDER_DESCEND));
    INA_TEST_ASSERT_SUCCEED(ina_dir_walker_get_next_entry(w, &e));
    INA_TEST_ASSERT_EQUAL_STR("test3", e->name);
    ina_dir_walker_free(&w);
    INA_TEST_ASSERT_NULL(w);
}

INA_TEST(dir, invalid_arguments)
{

    ina_dir_walker_t *w;
    ina_dir_sort_attrib_t sa;
    ina_dir_sort_order_t so;
    const ina_dir_entry_t *entry = NULL;
    ina_dir_stat_t *stat = NULL;
    size_t size = 0;
    int pct_used = 0;

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_dir_walker_new(NULL, &w));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_dir_walker_new("test", NULL));

    INA_TEST_ASSERT_SUCCEED(ina_dir_walker_new("./", &w));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_dir_walker_enable_recursive(NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_dir_walker_disable_recursive(NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_dir_walker_get_sort_order(NULL, &so));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_dir_walker_get_sort_order(w, NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_dir_walker_set_sort_order(NULL, so));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_dir_walker_get_sort_attrib(NULL, &sa));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_dir_walker_get_sort_attrib(w, NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_dir_walker_set_sort_attrib(NULL, sa));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_dir_walker_get_next_entry(NULL, &entry));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_dir_walker_get_next_entry(w, NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_dir_walker_reset(NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_dir_walker_reload(NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_dir_stat_new(NULL, &stat));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_dir_stat_new("test", NULL));
    INA_TEST_ASSERT_SUCCEED(ina_dir_stat_new("./", &stat));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_dir_stat_bytes_capacity(NULL, &size));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_dir_stat_bytes_capacity(stat, NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_dir_stat_bytes_free(NULL, &size));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_dir_stat_bytes_free(stat,NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_dir_stat_pct_used(NULL, &pct_used));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_dir_stat_pct_used(stat,NULL));

    ina_dir_walker_free(&w);
    ina_dir_stat_free(&stat);

}

