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
#include <stdio.h>
#include <libinac/lib.h>

#define INA_NN_HELLO   INA_NN_USER_DEFINED+1
#define INA_NN_WORLD   INA_NN_USER_DEFINED+2
#define INA_NN_UNKNOWN INA_NN_USER_DEFINED+3

static const char* __ina_get_noun_a(int id)
{
    switch (id) {
        case INA_NN_HELLO:
            return "HELLO A";
        case INA_NN_WORLD:
            return "WORLD A";
        default:
            return "--";
    }
}

static const char* __ina_get_noun_b(int id)
{
    switch (id) {
        case INA_NN_HELLO:
            return "HELLO B";
        case INA_NN_WORLD:
            return "WORLD B";
        default:
            return "XX";
    }
}

INA_TEST(error, get_set_rc)
{
    INA_TEST_ASSERT_EQUAL_INT64(INA_RC_PACK(INA_ERR_FAILED, 0),
                                  ina_err_set_last_rc(INA_RC_PACK(INA_ERR_FAILED, 0), ""));
    INA_TEST_ASSERT_EQUAL_INT64(INA_ERROR(INA_ERR_NOT_INITIALIZED),
                                  ina_err_set_last_rc(INA_RC_PACK(INA_ERR_NOT_INITIALIZED, 0), ""));
}

INA_TEST(error, clear)
{
    INA_TEST_ASSERT_FAILED(INA_ERROR(INA_ERR_NOT_INITIALIZED));
    INA_TEST_ASSERT_FAILED(ina_err_get_last_rc());
    INA_TEST_ASSERT_SUCCEED(ina_err_reset());
    INA_TEST_ASSERT_SUCCEED(ina_err_get_last_rc());

}
INA_TEST(error, strerror)
{
    char msg[INA_ERROR_MSGLEN];

    INA_TEST_ASSERT_SUCCEED(ina_err_reset());
    INA_TEST_ASSERT_SUCCEED(ina_err_get_last_rc());
    INA_ERROR(INA_NN_DEVICE|INA_ERR_IN_USE);
    INA_TEST_MSG("%s", ina_err_strerror(ina_err_get_last_rc(), msg));
}

INA_TEST(error, error_pack_rc) 
{
    char msg[INA_ERROR_MSGLEN];
    ina_rc_t rcc;
    ina_rc_t rc;

    rcc = -9223372036812668926;
    rc = INA_RC_PACK(INA_NN_ACCESS|INA_ERR_NOT_ALLOWED, 2);


    INA_TRACE3("rc = %u", rc);
    INA_TRACE3("reason = %u", INA_RC_ATTRIB(rc, 0));
    INA_TEST_MSG("%s", ina_err_strerror(rc, msg));
    
    INA_TEST_ASSERT_EQUAL_INT64(rcc,  rc);
    INA_TEST_ASSERT_EQUAL_INT(INA_NN_ACCESS, INA_RC_USERNN(rc));
    INA_TEST_ASSERT_NOT_EQUAL_INT(INA_NN_DEVICE, INA_RC_USERNN(rc));
    INA_TEST_ASSERT_NOT_EQUAL_INT(INA_NN_OPERATION, INA_RC_USERNN(rc));
    INA_TEST_ASSERT_EQUAL_INT(INA_ERR_NOT_ALLOWED, INA_RC_ERROR(rc));
    INA_TEST_ASSERT_EQUAL_INT(0, INA_RC_VER(rc));
    INA_TEST_ASSERT_EQUAL_INT(0,  INA_RC_REV(rc));
    INA_TEST_ASSERT_EQUAL_INT(INA_ERR_NOT_ALLOWED, INA_RC_ERROR(rc));
    INA_TEST_ASSERT_EQUAL_INT64(rcc, ina_err_set_last_rc(rc, INA_AT));
    INA_TEST_ASSERT_EQUAL_INT64(rcc,   ina_err_get_last_rc());
#undef INA_ERROR_VER
#undef INA_ERROR_REV
#define INA_ERROR_VER 2
#define INA_ERROR_REV 123
    rc = INA_RC_PACK(INA_NN_ACCESS|INA_ERR_NOT_ALLOWED, 2);
    INA_TEST_ASSERT_EQUAL_INT(2, INA_RC_VER(rc));
    INA_TEST_ASSERT_EQUAL_INT(123,  INA_RC_REV(rc));
#undef INA_ERROR_VER
#undef INA_ERROR_REV
#define INA_ERROR_VER INA_MAJOR_VERSION
#define INA_ERROR_REV INA_REVISION_HEX
}


INA_TEST(error, register_dict)
{
    char msg[INA_ERROR_MSGLEN];
    INA_TEST_ASSERT_NULL(ina_err_register_dict(__ina_get_noun_a));
    INA_ERROR(INA_NN_HELLO|INA_ERR_FAILED);
    INA_TEST_ASSERT_EQUAL_STR("HELLO A FAILED - 0x8009000000158401 - error=1,ver=0,rev=2304,os=0,neg=0,attr=43,noun=1025", ina_err_strerror(ina_err_get_last_rc(), msg));
    INA_ERROR(INA_NN_WORLD|INA_ERR_NOT_FOUND);
    INA_TEST_ASSERT_EQUAL_STR("WORLD A NOT FOUND - 0x8009000000980402 - error=1,ver=0,rev=2304,os=0,neg=1,attr=48,noun=1026", ina_err_strerror(ina_err_get_last_rc(), msg));
    INA_ERROR(INA_NN_UNKNOWN|INA_ERR_NOT_FOUND);
    INA_TEST_ASSERT_EQUAL_STR("-- NOT FOUND - 0x8009000000980403 - error=1,ver=0,rev=2304,os=0,neg=1,attr=48,noun=1027", ina_err_strerror(ina_err_get_last_rc(), msg));
    INA_TEST_ASSERT_SAME(__ina_get_noun_a, ina_err_register_dict(__ina_get_noun_b));
    INA_ERROR(INA_NN_HELLO|INA_ERR_FAILED);
    INA_TEST_ASSERT_EQUAL_STR("HELLO B FAILED - 0x8009000000158401 - error=1,ver=0,rev=2304,os=0,neg=0,attr=43,noun=1025", ina_err_strerror(ina_err_get_last_rc(), msg));
    INA_ERROR(INA_NN_WORLD|INA_ERR_NOT_FOUND);
    INA_TEST_ASSERT_EQUAL_STR("WORLD B NOT FOUND - 0x8009000000980402 - error=1,ver=0,rev=2304,os=0,neg=1,attr=48,noun=1026", ina_err_strerror(ina_err_get_last_rc(), msg));
    INA_ERROR(INA_NN_UNKNOWN|INA_ERR_NOT_FOUND);
    INA_TEST_ASSERT_EQUAL_STR("XX NOT FOUND - 0x8009000000980403 - error=1,ver=0,rev=2304,os=0,neg=1,attr=48,noun=1027", ina_err_strerror(ina_err_get_last_rc(), msg));
}
