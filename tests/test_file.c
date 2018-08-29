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


INA_TEST(file, test_open_close)
{
    ina_file_ctx_t *ctx;
    ina_file_t *f;
    const char *test_file = "tests.conf";
    
    INA_TEST_ASSERT_SUCCEED(ina_file_init(&ctx, 0));

    INA_TEST_ASSERT_SUCCEED(ina_file_new(ctx, test_file, INA_FILE_ACCESS_MODE_READ, 
        INA_FILE_CREATE_MODE_OPEN, INA_FILE_SHARE_MODE_READ, 0, &f)); 

    INA_TEST_ASSERT_SUCCEED(ina_file_free(ctx, &f));

    INA_TEST_ASSERT_SUCCEED(ina_file_destroy(&ctx));
}

INA_TEST(file, test_stat)
{
    ina_file_ctx_t *ctx;
    ina_file_t *f;
    ina_file_stat_t *stat = NULL;
    uint64_t file_size;

    const char *test_file = "tests.conf";
    INA_TEST_ASSERT_SUCCEED(ina_file_init(&ctx, 0));
    INA_TEST_ASSERT_SUCCEED(ina_file_new(ctx, test_file, INA_FILE_ACCESS_MODE_READ,
                                         INA_FILE_CREATE_MODE_OPEN, INA_FILE_SHARE_MODE_READ, 0, &f));

    INA_TEST_ASSERT_SUCCEED(ina_file_stat_new(test_file, &stat));
    INA_TEST_ASSERT_NOT_NULL(stat);
    INA_TEST_ASSERT_SUCCEED(ina_file_stat_file_size(stat, &file_size));
    INA_TEST_ASSERT_EQUAL_INTEGER(195, file_size);

}


