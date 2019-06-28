/*
 * Copyright INAOS GmbH, Thalwil, 2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>

#ifdef INA_OS_WINDOWS
#define _INA_FILE_TEST_CONF_NAME "tests.exe.conf"
#else
#define _INA_FILE_TEST_CONF_NAME "tests.conf"
#endif

INA_TEST(file, test_open_close)
{
    ina_file_ctx_t *ctx = NULL;
    ina_file_t *f = NULL;
    const char *test_file1 = _INA_FILE_TEST_CONF_NAME;
    const char *test_file2 = "tests2.conf";
    ina_str_t file_path = NULL;
    mode_t mode = 0;

    INA_TEST_ASSERT_SUCCEED(ina_file_ctx_new(&ctx, 0));
    INA_TEST_ASSERT_NOT_NULL(ctx);
    INA_TEST_ASSERT_SUCCEED(ina_file_new(ctx, test_file1,
                                         INA_FILE_ACCESS_MODE_READ,
                                         INA_FILE_CREATE_MODE_OPEN,
                                         INA_FILE_SHARE_MODE_READ,
                                         0,
                                         &f));
    INA_TEST_ASSERT_NOT_NULL(f);
    INA_TEST_ASSERT_SUCCEED(ina_file_get_filepath(f, &file_path));
    INA_TEST_ASSERT_NOT_NULL(file_path);
    INA_TEST_ASSERT_EQUAL_STR(_INA_FILE_TEST_CONF_NAME, file_path);
    ina_str_free(file_path);
#ifdef INA_OS_WINDOWS
    INA_TEST_ASSERT_NOT_NULL(ina_file_os_handle(f));
#else
    INA_TEST_ASSERT_NOT_EQUAL_INT(0, ina_file_os_handle(f));
#endif
    INA_TEST_ASSERT_SUCCEED(ina_file_get_mode(f, &mode));
    INA_TEST_ASSERT_NOT_EQUAL_INT(0, mode);
    INA_TEST_ASSERT_NOT_NULL(ina_file_get_stream(f));
    ina_file_free(&f);
    INA_ASSERT_NULL(f);

    INA_TEST_ASSERT_FAILED(ina_file_new(ctx, test_file2,
                                        INA_FILE_ACCESS_MODE_READ,
                                        INA_FILE_CREATE_MODE_OPEN,
                                        INA_FILE_SHARE_MODE_READ,
                                        0,
                                        &f));
    INA_TEST_ASSERT_NULL(f);

    ina_file_ctx_free(&ctx);
    INA_TEST_ASSERT_NULL(ctx);
}

INA_TEST(file, stat)
{
    ina_file_ctx_t *ctx;
    ina_file_t *f;
    ina_file_stat_t *stat = NULL;
    uint64_t file_size = 0;
    time_t t = 0;

    const char *test_file = _INA_FILE_TEST_CONF_NAME;

    INA_TEST_ASSERT_SUCCEED(ina_file_ctx_new(&ctx, 0));
    INA_TEST_ASSERT_SUCCEED(ina_file_new(ctx, test_file,
                                         INA_FILE_ACCESS_MODE_READ,
                                         INA_FILE_CREATE_MODE_OPEN,
                                         INA_FILE_SHARE_MODE_READ,
                                         0,
                                         &f));
    INA_TEST_ASSERT_SUCCEED(ina_file_stat_new(f, &stat));
    INA_TEST_ASSERT_NOT_NULL(stat);
    INA_TEST_ASSERT_SUCCEED(ina_file_stat_file_size(stat, &file_size));
    INA_TEST_ASSERT_EQUAL_INT64(180LL, file_size);
    INA_TEST_ASSERT_SUCCEED(ina_file_stat_atime(stat, &t));
    INA_TEST_ASSERT_NOT_EQUAL_TIME_T(0, t);
    t = 0;
    INA_TEST_ASSERT_SUCCEED(ina_file_stat_mtime(stat, &t));
    INA_TEST_ASSERT_NOT_EQUAL_TIME_T(0, t);
    INA_TEST_ASSERT_FAILED(ina_file_stat_is_dir(stat));
    ina_file_stat_free(&stat);
    INA_ASSERT_NULL(stat);
    ina_file_free(&f);
    ina_file_ctx_free(&ctx);

}

INA_TEST(file, os_handle)
{
    ina_file_ctx_t *ctx = NULL;
    ina_file_t *f = NULL;
    const char *test_file = _INA_FILE_TEST_CONF_NAME;
    ina_handle_t h;
    char buf[10];
#ifdef INA_OS_WINDOWS
    DWORD nread;
#endif
    INA_TEST_ASSERT_SUCCEED(ina_file_ctx_new(&ctx, 0));
    INA_TEST_ASSERT_NOT_NULL(ctx);
    INA_TEST_ASSERT_SUCCEED(ina_file_new(ctx, test_file,
                                         INA_FILE_ACCESS_MODE_READ,
                                         INA_FILE_CREATE_MODE_OPEN,
                                         INA_FILE_SHARE_MODE_READ,
                                         0,
                                         &f));
    INA_TEST_ASSERT_NOT_NULL(f);

    memset(buf, 0, 10);
    h = ina_file_os_handle(f);
#ifdef INA_OS_WINDOWS
    INA_TEST_ASSERT_NOT_NULL(h);
    INA_TEST_ASSERT_NOT_EQUAL_INT(0, ReadFile(h, buf, 7, &nread, NULL));
#else
    INA_TEST_ASSERT_NOT_EQUAL_INT(0, h);
    read(h, buf, 7);
#endif
    INA_TEST_ASSERT_EQUAL_STR("debug {", buf);
    ina_file_free(&f);
    ina_file_ctx_free(&ctx);
}

INA_TEST(file, stream)
{
    ina_file_ctx_t *ctx = NULL;
    ina_file_t *f = NULL;
    const char *test_file = _INA_FILE_TEST_CONF_NAME;
    FILE *fp;
    char buf[10];

    INA_TEST_ASSERT_SUCCEED(ina_file_ctx_new(&ctx, 0));
    INA_TEST_ASSERT_NOT_NULL(ctx);
    INA_TEST_ASSERT_SUCCEED(ina_file_new(ctx, test_file,
                                         INA_FILE_ACCESS_MODE_READ,
                                         INA_FILE_CREATE_MODE_OPEN,
                                         INA_FILE_SHARE_MODE_READ,
                                         0,
                                         &f));
    INA_TEST_ASSERT_NOT_NULL(f);
    fp = ina_file_get_stream(f);
    INA_TEST_ASSERT_NOT_NULL(fp);
    memset(buf, 0, 10);
    fread(buf, 7, 1, fp);
    INA_TEST_ASSERT_EQUAL_STR("debug {", buf);
    ina_file_free(&f);
    ina_file_ctx_free(&ctx);
}

INA_TEST(file, mode)
{
    ina_file_ctx_t *ctx = NULL;
    ina_file_t *f = NULL;
    const char *test_file = _INA_FILE_TEST_CONF_NAME;

    INA_TEST_ASSERT_SUCCEED(ina_file_ctx_new(&ctx, 0));
    INA_TEST_ASSERT_NOT_NULL(ctx);
    INA_TEST_ASSERT_SUCCEED(ina_file_new(ctx, test_file,
                                         INA_FILE_ACCESS_MODE_READ,
                                         INA_FILE_CREATE_MODE_OPEN,
                                         INA_FILE_SHARE_MODE_READ,
                                         0,
                                         &f));
}
/*
ina_file_get_mode
ina_file_set_mode
ina_file_stat_is_dir
ina_file_read
ina_file_write
ina_file_set_pos
ina_file_get_pos
ina_file_set_eof*/