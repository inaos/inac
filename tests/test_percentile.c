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

static int __test_percentile_input[] = {
    43, 54, 56, 61, 62, 66, 68, 69, 69, 70, 71, 72, 77, 78, 79, 85, 87, 88, 89, 93, 95, 96, 98, 99, 99
};

INA_TEST_SKIP(percentile, test_percentile_90)
{
    size_t test_size = sizeof(__test_percentile_input)/sizeof(int);
    ina_percentile_t *p;
    double test_percentile = 0.9;
    size_t i;
    uint16_t actual_percentile = 0;

    INA_TEST_ASSERT_SUCCEED(ina_percentile_new(&p, test_percentile, test_size));
    for (i = 0; i < test_size; i++) {
        INA_TEST_ASSERT_SUCCEED(ina_percentile_add(p, __test_percentile_input[i]));
    }
    INA_TEST_ASSERT_SUCCEED(ina_percentile_get(p, &actual_percentile));
    INA_TEST_ASSERT_EQUAL_INTEGER(98, actual_percentile);
    INA_TEST_ASSERT_SUCCEED(ina_percentile_free(&p));
}

INA_TEST_SKIP(percentile, test_percentile_50)
{
    size_t test_size = sizeof(__test_percentile_input)/sizeof(int);
    ina_percentile_t *p;
    double test_percentile = 0.5;
    size_t i;
    uint16_t actual_percentile = 0;

    INA_TEST_ASSERT_SUCCEED(ina_percentile_new(&p, test_percentile, test_size));
    for (i = 0; i < test_size; i++) {
        INA_TEST_ASSERT_SUCCEED(ina_percentile_add(p, __test_percentile_input[i]));
    }
    INA_TEST_ASSERT_SUCCEED(ina_percentile_get(p, &actual_percentile));
    INA_TEST_ASSERT_EQUAL_INTEGER(77, actual_percentile);
    INA_TEST_ASSERT_SUCCEED(ina_percentile_free(&p));
}


