/*
 * Copyright (c) 2012-2018, INAOS GmbH
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

INA_TEST(util, dbl_cmp_abs)
{
    double v1,v2;

    v1 = 10000000.0 + DBL_EPSILON;
    v2 = 10000000.0 + DBL_EPSILON + DBL_EPSILON*0.1;
    INA_TEST_ASSERT_EQUAL_FLOATING(v1, v2);

    v2 = 10000000.0 + DBL_EPSILON + DBL_EPSILON*1.000000000000001;
    INA_TEST_ASSERT_EQUAL_FLOATING(v1, v2);
    INA_TEST_ASSERT_EQUAL_INTEGER(1, ina_util_dbl_cmp_abs(v1, v2));

    v1 = 1.0 + DBL_EPSILON;
    v2 = 1.0 + DBL_EPSILON + DBL_EPSILON*0.1;
    INA_TEST_ASSERT_EQUAL_FLOATING(v1, v2);

    v2 = 1.0 + DBL_EPSILON + DBL_EPSILON*1.000000000000001;
    INA_TEST_ASSERT_NOT_EQUAL_FLOATING(v1, v2);
    INA_TEST_ASSERT_EQUAL_INTEGER(1, ina_util_dbl_cmp_abs(v1, v2));
}

INA_TEST(util, dbl_cmp_rel)
{
    double v1,v2;

    v1 = 0.01 + DBL_EPSILON;
    v2 = 0.01 + DBL_EPSILON + DBL_EPSILON*0.1;
    INA_TEST_ASSERT_NOT_EQUAL_FLOATING(v1, v2);
    INA_TEST_ASSERT_NOT_EQUAL_INTEGER(1, ina_util_dbl_cmp_rel(v1, v2));

    v2 = 0.01 + DBL_EPSILON + DBL_EPSILON*1.000000000000001;
    INA_TEST_ASSERT_NOT_EQUAL_FLOATING(v1, v2);
    INA_TEST_ASSERT_NOT_EQUAL_INTEGER(1, ina_util_dbl_cmp_rel(v1, v2));

    v1 = 0.01 + DBL_EPSILON;
    v2 = 0.01 + DBL_EPSILON + DBL_EPSILON*0.1;
    INA_TEST_ASSERT_NOT_EQUAL_FLOATING(v1, v2);
    INA_TEST_ASSERT_NOT_EQUAL_INTEGER(1, ina_util_dbl_cmp_rel(v1, v2));

    v2 = 0.01 + DBL_EPSILON*0.000000000000000000000000001;
    INA_TEST_ASSERT_NOT_EQUAL_FLOATING(v1, v2);
    INA_TEST_ASSERT_NOT_EQUAL_INTEGER(1, ina_util_dbl_cmp_rel(v1, v2));
}

INA_TEST(util, dbl_cmp_save)
{
   double v1,v2;

    v1 = 0.01 + DBL_EPSILON;
    v2 = 0.01 + DBL_EPSILON + DBL_EPSILON*0.1;
    INA_TEST_ASSERT_NOT_EQUAL_FLOATING(v1, v2);
    INA_TEST_ASSERT_EQUAL_INTEGER(1, ina_util_dbl_cmp_save(v1, v2));

    v2 = 0.01 + DBL_EPSILON + DBL_EPSILON*1.000000000000001;
    INA_TEST_ASSERT_NOT_EQUAL_FLOATING(v1, v2);
    INA_TEST_ASSERT_EQUAL_INTEGER(1, ina_util_dbl_cmp_save(v1, v2));

    v1 = 0.01 + DBL_EPSILON;
    v2 = 0.01 + DBL_EPSILON + DBL_EPSILON*0.1;
    INA_TEST_ASSERT_NOT_EQUAL_FLOATING(v1, v2);
    INA_TEST_ASSERT_EQUAL_INTEGER(1, ina_util_dbl_cmp_save(v1, v2));

    v2 = 0.01 + DBL_EPSILON*0.000000000000000000000000001;
    INA_TEST_ASSERT_NOT_EQUAL_FLOATING(v1, v2);
    INA_TEST_ASSERT_EQUAL_INTEGER(1, ina_util_dbl_cmp_save(v1, v2));

    v1 = 10000000.0 + DBL_EPSILON;
    v2 = 10000000.0 + DBL_EPSILON + DBL_EPSILON*0.1;
    INA_TEST_ASSERT_EQUAL_FLOATING(v1, v2);

    v2 = 10000000.0 + DBL_EPSILON + DBL_EPSILON*1.000000000000001;
    INA_TEST_ASSERT_EQUAL_FLOATING(v1, v2);
    INA_TEST_ASSERT_EQUAL_INTEGER(1, ina_util_dbl_cmp_save(v1, v2));

    v1 = 1.0 + DBL_EPSILON;
    v2 = 1.0 + DBL_EPSILON + DBL_EPSILON*0.1;
    INA_TEST_ASSERT_EQUAL_FLOATING(v1, v2);

    v2 = 1.0 + DBL_EPSILON + DBL_EPSILON*1.000000000000001;
    INA_TEST_ASSERT_NOT_EQUAL_FLOATING(v1, v2);
    INA_TEST_ASSERT_EQUAL_INTEGER(1, ina_util_dbl_cmp_save(v1, v2));
}

INA_TEST(util, base64)
{
#define BUF_LEN 2048
    char *ref_encoded = "QmFzZTY0IGlzIGEgZ2VuZXJpYyB0ZXJtIGZvciBhIG51bWJlciBvZiBzaW1pbGFyIGVuY29kaW5nIHNjaGVtZXMgdGhhdCBlbmNvZGUgYmluYXJ5IGRhdGEgYnkgdHJlYXRpbmcgaXQgbnVtZXJpY2FsbHkgYW5kIHRyYW5zbGF0aW5nIGl0IGludG8gYSBiYXNlIDY0IHJlcHJlc2VudGF0aW9uLiBUaGUgQmFzZTY0IHRlcm0gb3JpZ2luYXRlcyBmcm9tIGEgc3BlY2lmaWMgTUlNRSBjb250ZW50IHRyYW5zZmVyIGVuY29kaW5nLg==";
    char *ref_decoded = "Base64 is a generic term for a number of similar encoding schemes that encode binary data by treating it numerically and translating it into a base 64 representation. The Base64 term originates from a specific MIME content transfer encoding.";
    char buf[BUF_LEN];
    size_t outlen;
    ina_mem_set(&buf, 0, BUF_LEN);

    INA_TEST_ASSERT_SUCCEED(ina_util_base64_decode_chunk(ref_encoded, strlen(ref_encoded), (unsigned char*)buf, BUF_LEN, &outlen));
    INA_TEST_ASSERT_EQUAL_STR(ref_decoded, buf);

    INA_TEST_ASSERT_SUCCEED(ina_util_base64_encode_chunk(ref_decoded, strlen(ref_decoded), buf, 2024));
    INA_TEST_ASSERT_EQUAL_STR(ref_encoded, buf);
}