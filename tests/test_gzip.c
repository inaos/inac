/*
 * Copyright (c) 2015, INAOS GmbH
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

static const char *my_test_string = "EUR/USD,GBP/USD,USD/JPY,AUD/USD,EUR/JPY,USD/CAD,EUR/GBP,USD/CHF," \
  "USD/MXN,NZD/USD,EUR/CHF,USD/RUB,USD/ZAR,USD/SGD,USD/TRY,EUR/SEK,GBP/JPY," \
  "EUR/AUD,EUR/NOK,EUR/PLN,USD/SEK,USD/NOK,USD/INR,USD/PLN," \
  "USD/CNY,USD/BRL,GBP/AUD,EUR/CAD,GBP/CHF,GBP/CAD";

INA_TEST(gzip, test_gzip)
{
    ina_gzip_file_t *gzf;
    size_t read = 0;
    unsigned char *chunk;
    size_t tot_len = 0;
    unsigned char buf[4*1024];

    ina_mem_set(buf, 0, 4*1024);
    chunk = buf;
    INA_TEST_MSG("size of size_t: %d", sizeof(size_t));
    INA_TEST_MSG("size of uint64_t: %d", sizeof(uint64_t));
    INA_TEST_MSG("out_bytes: %d", strlen(my_test_string));
    INA_TEST_ASSERT_SUCCEED(ina_gzip_open("test_gzip.gz", 4*1024, &gzf));
    
    while (INA_SUCCEED(ina_gzip_read_next_block(gzf, 4*1024, &read, &chunk)) && read > 0) {
        tot_len += read;
    }
    INA_TEST_MSG("tot_len: %d", tot_len);
    INA_TEST_MSG("buf: %s", buf);
    INA_TEST_ASSERT_TRUE(strcmp(my_test_string, (const char*)buf) == 0);
    INA_TEST_ASSERT_SUCCEED(ina_gzip_close(&gzf));
}

INA_TEST(gzip, test_requested)
{
    ina_gzip_file_t *gzf;
    size_t read = 0;
    unsigned char *chunk;
    unsigned char buf[4*1024];

    ina_mem_set(buf, 0, 4*1024);
    chunk = buf;
    INA_TEST_ASSERT_SUCCEED(ina_gzip_open("test_gzip.gz", 4*1024, &gzf));
    
    INA_TEST_ASSERT_SUCCEED(ina_gzip_read_next_block(gzf, 8, &read, &chunk));
    INA_TEST_MSG("read: %d", read);
    INA_TEST_MSG("buf: %s", buf);
    INA_TEST_ASSERT_EQUAL_INTEGER(8, read);
    INA_TEST_ASSERT_TRUE(strcmp("EUR/USD,", (const char*)buf) == 0);
    INA_TEST_ASSERT_SUCCEED(ina_gzip_close(&gzf));
}


