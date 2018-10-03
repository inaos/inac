/*
 * Copyright INAOS GmbH, Thalwil, 2012-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <stdio.h>
#include <libinac/lib.h>

#define INA_ES_HELLO   (INA_ES_USER_DEFINED+1)
#define INA_ES_WORLD   (INA_ES_USER_DEFINED+2)
#define INA_ES_UNKNOWN (INA_ES_USER_DEFINED+3)

static const char* __ina_get_subject_a(int id)
{
    switch (id) {
        case INA_ES_HELLO:
            return "HELLO A";
        case INA_ES_WORLD:
            return "WORLD A";
        default:
            return "--";
    }
}

static const char* __ina_get_subject_b(int id)
{
    switch (id) {
        case INA_ES_HELLO:
            return "HELLO B";
        case INA_ES_WORLD:
            return "WORLD B";
        default:
            return "XX";
    }
}

INA_TEST(error, error_pack_rc)
{
    ina_rc_t rcc;
    ina_rc_t rc;

    rcc = 0x8000000200058002;
    rc = INA_RC_PACK(INA_ES_ACCESS|INA_ERR_NOT_ALLOWED, 2);
    INA_TEST_ASSERT_EQUAL_UINT64(rcc, rc);

    INA_TEST_MSG("verify INA_RC_PACK with %s", ina_err_strerror(rc));
    INA_TEST_ASSERT_SUCCEED(INA_SUCCESS);
    INA_TEST_ASSERT_FAILED(rc);
;
    /* error indicator */
    INA_TEST_ASSERT_EQUAL_INT(1, INA_RC_EFLAG(rc));

    /* API version information */
    INA_TEST_ASSERT_EQUAL_INT(0, INA_RC_VER(rc));
    INA_TEST_ASSERT_EQUAL_INT(0,  INA_RC_REV(rc));

#undef INA_ERROR_VER
#undef INA_ERROR_REV
#define INA_ERROR_VER 2
#define INA_ERROR_REV 123
    rc = INA_RC_PACK(INA_ES_ACCESS|INA_ERR_NOT_ALLOWED, 2);
    INA_TEST_ASSERT_EQUAL_INT(2, INA_RC_VER(rc));
    INA_TEST_ASSERT_EQUAL_INT(123,  INA_RC_REV(rc));
#undef INA_ERROR_VER
#undef INA_ERROR_REV
#define INA_ERROR_VER INA_MAJOR_VERSION
#define INA_ERROR_REV INA_REVISION_HEX

    /* OS native error */
    INA_TEST_ASSERT_EQUAL_UINT(2, INA_RC_ERRNO(rc));

    /* code */
    INA_TEST_ASSERT_EQUAL_UINT64(INA_ERR_ALLOWED, INA_RC_CODE(rc));
    /* Adjective/Verb */
    INA_TEST_ASSERT_EQUAL_UINT64(5, INA_RC_ADJ(rc));

    /* Negate flag */
    INA_TEST_ASSERT_EQUAL_UINT(1, INA_RC_NFLAG(rc));

    /* subject */
    INA_TEST_ASSERT_EQUAL_INT(INA_ES_ACCESS, INA_RC_SUBJECT(rc));
    INA_TEST_ASSERT_NOT_EQUAL_INT(INA_ES_DEVICE, INA_RC_SUBJECT(rc));
    INA_TEST_ASSERT_NOT_EQUAL_INT(INA_ES_OPERATION, INA_RC_SUBJECT(rc));
    INA_TEST_ASSERT_EQUAL_INT(INA_ERR_NOT_ALLOWED, INA_RC_ERROR(rc));

    /* error message */
    INA_TEST_ASSERT_EQUAL_UINT64((INA_ES_ACCESS|INA_ERR_NOT_ALLOWED), INA_RC_ERRMSG(rc));
}

INA_TEST(error, error_same_as_errmsg)
{
    INA_ERROR(INA_ERR_EMPTY);
    INA_TEST_ASSERT_EQUAL_INT(
            INA_RC_ERRMSG(ina_err_get_last_rc()),
            INA_RC_ERROR(ina_err_get_last_rc()));

}

INA_TEST(error, get_set_rc)
{
    INA_TEST_ASSERT_EQUAL_INT64(INA_RC_PACK(INA_ERR_FAILED, 0),
                                  ina_err_set_last_rc(INA_RC_PACK(INA_ERR_FAILED, 0)));
    INA_TEST_ASSERT_EQUAL_INT64(INA_ERROR(INA_ERR_NOT_INITIALIZED),
                                  ina_err_set_last_rc(INA_RC_PACK(INA_ERR_NOT_INITIALIZED, 0)));
}

INA_TEST(error, reset)
{
    INA_TEST_ASSERT_FAILED(INA_ERROR(INA_ERR_NOT_INITIALIZED));
    INA_TEST_ASSERT_FAILED(ina_err_get_last_rc());
    INA_TEST_ASSERT_SUCCEED(ina_err_reset());
    INA_TEST_ASSERT_SUCCEED(ina_err_get_last_rc());
}

INA_TEST(error, strerror)
{
    INA_ERROR(INA_ES_DEVICE|INA_ERR_IN_USE);
    INA_TEST_MSG("%s", ina_err_strerror(ina_err_get_last_rc()));
    INA_TEST_ASSERT_EQUAL_STR("DEVICE IN USE - 0x81000000003b0018 - error=1,ver=1,rev=0,os=0,neg=0,adj=59,subject=24,code=3866624", ina_err_strerror(ina_err_get_last_rc()));
}

INA_TEST(error, register_dict)
{
    INA_TEST_ASSERT_NULL(ina_err_register_dict(__ina_get_subject_a));
    INA_ERROR(INA_ES_HELLO|INA_ERR_FAILED);
    INA_TEST_ASSERT_EQUAL_STR("HELLO A FAILED - 0x81000000002b0401 - error=1,ver=1,rev=0,os=0,neg=0,adj=43,subject=1025,code=2818048", ina_err_strerror(ina_err_get_last_rc()));
    INA_ERROR(INA_ES_WORLD|INA_ERR_NOT_FOUND);
    INA_TEST_ASSERT_EQUAL_STR("WORLD A NOT FOUND - 0x8100000000308402 - error=1,ver=1,rev=0,os=0,neg=1,adj=48,subject=1026,code=3145728", ina_err_strerror(ina_err_get_last_rc()));
    INA_ERROR(INA_ES_UNKNOWN|INA_ERR_NOT_FOUND);
    INA_TEST_ASSERT_EQUAL_STR("-- NOT FOUND - 0x8100000000308403 - error=1,ver=1,rev=0,os=0,neg=1,adj=48,subject=1027,code=3145728", ina_err_strerror(ina_err_get_last_rc()));
    INA_TEST_ASSERT_SAME(__ina_get_subject_a, ina_err_register_dict(__ina_get_subject_b));
    INA_ERROR(INA_ES_HELLO|INA_ERR_FAILED);
    INA_TEST_ASSERT_EQUAL_STR("HELLO B FAILED - 0x81000000002b0401 - error=1,ver=1,rev=0,os=0,neg=0,adj=43,subject=1025,code=2818048", ina_err_strerror(ina_err_get_last_rc()));
    INA_ERROR(INA_ES_WORLD|INA_ERR_NOT_FOUND);
    INA_TEST_ASSERT_EQUAL_STR("WORLD B NOT FOUND - 0x8100000000308402 - error=1,ver=1,rev=0,os=0,neg=1,adj=48,subject=1026,code=3145728", ina_err_strerror(ina_err_get_last_rc()));
    INA_ERROR(INA_ES_UNKNOWN|INA_ERR_NOT_FOUND);

    INA_TEST_ASSERT_EQUAL_STR("XX NOT FOUND - 0x8100000000308403 - error=1,ver=1,rev=0,os=0,neg=1,adj=48,subject=1027,code=3145728", ina_err_strerror(ina_err_get_last_rc()));
}
