/*
 * Copyright INAOS GmbH, Thalwil, 2012-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>

INA_TEST(util, dbl_cmp_abs)
{
    double v1,v2;
    INA_UNUSED(data);

    v1 = 10000000.0 + DBL_EPSILON;
    v2 = 10000000.0 + DBL_EPSILON + DBL_EPSILON*0.1;
    INA_TEST_ASSERT_EQUAL_FLOATING(v1, v2);

    v2 = 10000000.0 + DBL_EPSILON + DBL_EPSILON*1.000000000000001;
    INA_TEST_ASSERT_EQUAL_FLOATING(v1, v2);
    INA_TEST_ASSERT_EQUAL_INT(1, ina_util_dbl_cmp_abs(v1, v2));

    v1 = 1.0 + DBL_EPSILON;
    v2 = 1.0 + DBL_EPSILON + DBL_EPSILON*0.1;
    INA_TEST_ASSERT_EQUAL_FLOATING(v1, v2);

    v2 = 1.0 + DBL_EPSILON + DBL_EPSILON*1.000000000000001;
    INA_TEST_ASSERT_NOT_EQUAL_FLOATING(v1, v2);
    INA_TEST_ASSERT_EQUAL_INT(1, ina_util_dbl_cmp_abs(v1, v2));
}

INA_TEST(util, dbl_cmp_rel)
{
    double v1,v2;
    INA_UNUSED(data);

    v1 = 0.01 + DBL_EPSILON;
    v2 = 0.01 + DBL_EPSILON + DBL_EPSILON*0.1;
    INA_TEST_ASSERT_NOT_EQUAL_FLOATING(v1, v2);
    INA_TEST_ASSERT_NOT_EQUAL_INT(1, ina_util_dbl_cmp_rel(v1, v2));

    v2 = 0.01 + DBL_EPSILON + DBL_EPSILON*1.000000000000001;
    INA_TEST_ASSERT_NOT_EQUAL_FLOATING(v1, v2);
    INA_TEST_ASSERT_NOT_EQUAL_INT(1, ina_util_dbl_cmp_rel(v1, v2));

    v1 = 0.01 + DBL_EPSILON;
    v2 = 0.01 + DBL_EPSILON + DBL_EPSILON*0.1;
    INA_TEST_ASSERT_NOT_EQUAL_FLOATING(v1, v2);
    INA_TEST_ASSERT_NOT_EQUAL_INT(1, ina_util_dbl_cmp_rel(v1, v2));

    v2 = 0.01 + DBL_EPSILON*0.000000000000000000000000001;
    INA_TEST_ASSERT_NOT_EQUAL_FLOATING(v1, v2);
    INA_TEST_ASSERT_NOT_EQUAL_INT(1, ina_util_dbl_cmp_rel(v1, v2));
}

INA_TEST(util, dbl_cmp_save)
{
   double v1,v2;
    INA_UNUSED(data);

    v1 = 0.01 + DBL_EPSILON;
    v2 = 0.01 + DBL_EPSILON + DBL_EPSILON*0.1;
    INA_TEST_ASSERT_NOT_EQUAL_FLOATING(v1, v2);
    INA_TEST_ASSERT_EQUAL_INT(1, ina_util_dbl_cmp_save(v1, v2));

    v2 = 0.01 + DBL_EPSILON + DBL_EPSILON*1.000000000000001;
    INA_TEST_ASSERT_NOT_EQUAL_FLOATING(v1, v2);
    INA_TEST_ASSERT_EQUAL_INT(1, ina_util_dbl_cmp_save(v1, v2));

    v1 = 0.01 + DBL_EPSILON;
    v2 = 0.01 + DBL_EPSILON + DBL_EPSILON*0.1;
    INA_TEST_ASSERT_NOT_EQUAL_FLOATING(v1, v2);
    INA_TEST_ASSERT_EQUAL_INT(1, ina_util_dbl_cmp_save(v1, v2));

    v2 = 0.01 + DBL_EPSILON*0.000000000000000000000000001;
    INA_TEST_ASSERT_NOT_EQUAL_FLOATING(v1, v2);
    INA_TEST_ASSERT_EQUAL_INT(1, ina_util_dbl_cmp_save(v1, v2));

    v1 = 10000000.0 + DBL_EPSILON;
    v2 = 10000000.0 + DBL_EPSILON + DBL_EPSILON*0.1;
    INA_TEST_ASSERT_EQUAL_FLOATING(v1, v2);

    v2 = 10000000.0 + DBL_EPSILON + DBL_EPSILON*1.000000000000001;
    INA_TEST_ASSERT_EQUAL_FLOATING(v1, v2);
    INA_TEST_ASSERT_EQUAL_INT(1, ina_util_dbl_cmp_save(v1, v2));

    v1 = 1.0 + DBL_EPSILON;
    v2 = 1.0 + DBL_EPSILON + DBL_EPSILON*0.1;
    INA_TEST_ASSERT_EQUAL_FLOATING(v1, v2);

    v2 = 1.0 + DBL_EPSILON + DBL_EPSILON*1.000000000000001;
    INA_TEST_ASSERT_NOT_EQUAL_FLOATING(v1, v2);
    INA_TEST_ASSERT_EQUAL_INT(1, ina_util_dbl_cmp_save(v1, v2));
}

INA_TEST(util, base64)
{
#define BUF_LEN 2048
    char *ref_encoded = "QmFzZTY0IGlzIGEgZ2VuZXJpYyB0ZXJtIGZvciBhIG51bWJlciBvZiBzaW1pbGFyIGVuY29kaW5nIHNjaGVtZXMgdGhhdCBlbmNvZGUgYmluYXJ5IGRhdGEgYnkgdHJlYXRpbmcgaXQgbnVtZXJpY2FsbHkgYW5kIHRyYW5zbGF0aW5nIGl0IGludG8gYSBiYXNlIDY0IHJlcHJlc2VudGF0aW9uLiBUaGUgQmFzZTY0IHRlcm0gb3JpZ2luYXRlcyBmcm9tIGEgc3BlY2lmaWMgTUlNRSBjb250ZW50IHRyYW5zZmVyIGVuY29kaW5nLg==";
    char *ref_decoded = "Base64 is a generic term for a number of similar encoding schemes that encode binary data by treating it numerically and translating it into a base 64 representation. The Base64 term originates from a specific MIME content transfer encoding.";
    char buf[BUF_LEN];
    size_t outlen;
    INA_UNUSED(data);

    ina_mem_set(&buf, 0, BUF_LEN);

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_util_base64_decode_chunk(NULL, strlen(ref_encoded), (unsigned char*)buf, BUF_LEN, &outlen));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_util_base64_decode_chunk(ref_encoded, strlen(ref_encoded), NULL, BUF_LEN, &outlen));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_util_base64_decode_chunk(ref_encoded, strlen(ref_encoded), (unsigned char*)buf, BUF_LEN, NULL));

    INA_TEST_ASSERT_SUCCEED(ina_util_base64_decode_chunk(ref_encoded, strlen(ref_encoded), (unsigned char*)buf, BUF_LEN, &outlen));
    INA_TEST_ASSERT_EQUAL_STR(ref_decoded, buf);

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_util_base64_encode_chunk(NULL, strlen(ref_encoded), buf, 2024));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_util_base64_encode_chunk(ref_encoded, strlen(ref_encoded), NULL, 2024));

    INA_TEST_ASSERT_SUCCEED(ina_util_base64_encode_chunk(ref_decoded, strlen(ref_decoded), buf, 2024));
    INA_TEST_ASSERT_EQUAL_STR(ref_encoded, buf);

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_util_base64_encode_length(100, 0, &outlen));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_util_base64_encode_length(800, 80, NULL));
}