/*
 * Copyright INAOS GmbH, Thalwil, 2014-2019. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>


INA_TEST_SKIP(file_cursor, test_file_cursor)
{
}

INA_TEST(file_cursor, invalid_arguments)
{
    ina_file_t *file = NULL;
    ina_file_cursor_t* cursor = NULL;
    ina_mempool_t *pool = NULL;
    ina_mmap_ctx_t *mmap_ctx = NULL;
    uint64_t size;
    ina_file_cursor_mode_t mode;
    ina_file_cursor_type_t type;

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT,
            ina_file_cursor_new_using_pool(NULL, INA_FILE_CURSOR_TYPE_FILEIO, INA_FILE_CURSOR_MODE_READ_TEXT_CHUNK, 10, &cursor, mmap_ctx, pool));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT,
            ina_file_cursor_new_using_pool(file, INA_FILE_CURSOR_TYPE_FILEIO, INA_FILE_CURSOR_MODE_READ_TEXT_CHUNK, 10, NULL, mmap_ctx, pool));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT,
            ina_file_cursor_new_using_pool(file, INA_FILE_CURSOR_TYPE_FILEIO, INA_FILE_CURSOR_MODE_READ_TEXT_CHUNK, 10, &cursor, NULL, pool));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT,
            ina_file_cursor_new_using_pool(file, INA_FILE_CURSOR_TYPE_FILEIO, INA_FILE_CURSOR_MODE_READ_TEXT_CHUNK, 10, &cursor, mmap_ctx, NULL));


    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT,
                           ina_file_cursor_new(NULL, INA_FILE_CURSOR_TYPE_FILEIO, INA_FILE_CURSOR_MODE_READ_TEXT_CHUNK, 10, &cursor, mmap_ctx));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT,
                           ina_file_cursor_new(file, INA_FILE_CURSOR_TYPE_FILEIO, INA_FILE_CURSOR_MODE_READ_TEXT_CHUNK, 10, NULL, mmap_ctx));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT,
                           ina_file_cursor_new(file, INA_FILE_CURSOR_TYPE_FILEIO, INA_FILE_CURSOR_MODE_READ_TEXT_CHUNK, 10, &cursor, NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT,
                           ina_file_cursor_new(file, INA_FILE_CURSOR_TYPE_FILEIO, INA_FILE_CURSOR_MODE_READ_TEXT_CHUNK, 10, &cursor, mmap_ctx));


    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT,
                           ina_file_cursor_get_file(NULL, &file));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT,
                           ina_file_cursor_get_file(cursor, NULL));


    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT,
                           ina_file_cursor_get_buffer_size(NULL, &size));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT,
                           ina_file_cursor_get_buffer_size(cursor, NULL));


    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT,
                           ina_file_cursor_get_mode(NULL, &mode));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT,
                           ina_file_cursor_get_mode(cursor, NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT,
                           ina_file_cursor_get_type(NULL, &type));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT,
                           ina_file_cursor_get_type(cursor, NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT,
                           ina_file_cursor_get_pos(NULL, &size));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT,
                           ina_file_cursor_get_pos(cursor, NULL));


    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT,
                           ina_file_cursor_set_pos(NULL, 10));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT,
            ina_file_cursor_set_bof(NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT,
                           ina_file_cursor_set_eof(NULL));



}
