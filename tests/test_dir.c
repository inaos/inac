/*
 * Copyright (c) 2016-2017, INAOS GmbH
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
#include <sys/stat.h>
#ifndef INA_OS_WIN32
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
    char tmp_dir[2024];
};

INA_TEST_SETUP(dir) {
    struct stat st = {0};
    char dir[128];
#ifdef INA_OS_WIN32
    strcpy(data->tmp_dir, getenv("TEMP"));
    strcat(data->tmp_dir, "/inac_test_dir");
#else
    strcpy(data->tmp_dir, "/tmp/inac_test_dir");
#endif


    strcpy(dir, data->tmp_dir);
    if (stat(data->tmp_dir, &st) == -1) {
        mkdir(data->tmp_dir, 0777);
    }
    strcpy(dir, data->tmp_dir);
    strcat(dir, "/test1");
    if (stat(dir, &st) == -1) {
        mkdir(dir, 0777);
    }
    strcpy(dir, data->tmp_dir);
    strcat(dir, "/test2");
    if (stat(dir, &st) == -1) {
        mkdir(dir, 0777);
    }
    strcpy(dir, data->tmp_dir);
    strcat(dir, "/test3");
    if (stat(dir, &st) == -1) {
        mkdir(dir, 0777);
    }
}

INA_TEST_TEARDOWN(dir) {

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

}

INA_TEST_FIXTURE_SKIP(dir, test_sorted_by_type_descend)
{
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

